/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "gui/console.h"
#include "gui/ScrollBarWidget.h"
#include "gui/eval.h"

#include "base/engine.h"
#include "base/version.h"

#include "common/system.h"

#include "graphics/font.h"

namespace GUI {

#define kConsoleCharWidth	(_font->getMaxCharWidth())
#define kConsoleLineHeight	(_font->getFontHeight() + 2)

enum {
	kConsoleSlideDownDuration = 200	// Time in milliseconds
};


#define PROMPT	") "

/* TODO:
 * - it is very inefficient to redraw the full thingy when just one char is added/removed.
 *   Instead, we could just copy the GFX of the blank console (i.e. after the transparent
 *   background is drawn, before any text is drawn). Then using that, it becomes trivial
 *   to erase a single character, do scrolling etc.
 * - a *lot* of others things, this code is in no way complete and heavily under progress
 */
ConsoleDialog::ConsoleDialog(float widthPercent, float heightPercent)
	: Dialog(0, 0, 1, 1),
	_widthPercent(widthPercent), _heightPercent(heightPercent) {

	// Reset the line buffer
	memset(_buffer, ' ', kBufferSize);

	// Dummy
	_scrollBar = new ScrollBarWidget(this, 0, 0, 5, 10);
	_scrollBar->setTarget(this);

	init();

	_currentPos = 0;
	_scrollLine = _linesPerPage - 1;
	_firstLineInBuffer = 0;

	_caretVisible = false;
	_caretTime = 0;

	_slideMode = kNoSlideMode;
	_slideTime = 0;

	_promptStartPos = _promptEndPos = -1;

	// Init callback
	_callbackProc = 0;
	_callbackRefCon = 0;

	// Init History
	_historyIndex = 0;
	_historyLine = 0;
	_historySize = 0;
	for (int i = 0; i < kHistorySize; i++)
		_history[i][0] = '\0';

	// Display greetings & prompt
	print(gScummVMFullVersion);
	print("\nConsole is ready\n");
}

void ConsoleDialog::init() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();
	int f = g_gui.evaluator()->getVar("Console.font");

	if (f == EVAL_UNDEF_VAR)
		_font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	else
		_font = g_gui.theme()->getFont((Theme::FontStyle)f);

	_leftPadding = g_gui.evaluator()->getVar("Console.leftPadding", 0);
	_rightPadding = g_gui.evaluator()->getVar("Console.rightPadding", 0);
	_topPadding = g_gui.evaluator()->getVar("Console.topPadding", 0);
	_bottomPadding = g_gui.evaluator()->getVar("Console.bottomPadding", 0);

	// Calculate the real width/height (rounded to char/line multiples)
	_w = (uint16)(_widthPercent * screenW);
	_h = (uint16)((_heightPercent * screenH - 2) / kConsoleLineHeight);
	_h = _h * kConsoleLineHeight + 2;

	// Set scrollbar dimensions
	int scrollBarWidth;
	if (g_gui.getWidgetSize() == kBigWidgetSize)
		scrollBarWidth = kBigScrollBarWidth;
	else
		scrollBarWidth = kNormalScrollBarWidth;
	_scrollBar->resize(_w - scrollBarWidth - 1, 0, scrollBarWidth, _h);

	_drawingHints = THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND;

	_pageWidth = (_w - scrollBarWidth - 2 - _leftPadding - _topPadding - scrollBarWidth) / kConsoleCharWidth;
	_linesPerPage = (_h - 2 - _topPadding - _bottomPadding) / kConsoleLineHeight;
	_linesInBuffer = kBufferSize / kLineWidth;
}

void ConsoleDialog::slideUpAndClose() {
	if (_slideMode == kNoSlideMode) {
		_slideTime = g_system->getMillis();
		_slideMode = kUpSlideMode;
	}
}

void ConsoleDialog::open() {
	// TODO: find a new way to do this
	// Initiate sliding the console down. We do a very simple trick to achieve
	// this effect: we simply move the console dialog just above (outside) the
	// visible screen area, then shift it down in handleTickle() over a
	// certain period of time.
	
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();
	
	// Calculate the real width/height (rounded to char/line multiples)
	uint16 w = (uint16)(_widthPercent * screenW);
	uint16 h = (uint16)((_heightPercent * screenH - 2) / kConsoleLineHeight);
	h = h * kConsoleLineHeight + 2;

	if (_w != w || _h != h)
		init();

	_drawingHints |= THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND;

	_y = -_h;
	_slideTime = g_system->getMillis();
	_slideMode = kDownSlideMode;

	Dialog::open();
	if (_promptStartPos == -1) {
		print(PROMPT);
		_promptStartPos = _promptEndPos = _currentPos;
	}
}

void ConsoleDialog::close() {
	Dialog::close();
}

void ConsoleDialog::drawDialog() {
	g_gui.theme()->drawDialogBackground(Common::Rect(_x, _y, _x+_w, _y+_h), _drawingHints);
	// FIXME: for the old theme the frame around the console vanishes
	// when any action is processed if we enable this
	// _drawingHints &= ~THEME_HINT_FIRST_DRAW;

	for (int line = 0; line < _linesPerPage; line++)
		drawLine(line, false);

	// Draw the scrollbar
	_scrollBar->draw();
}

void ConsoleDialog::drawLine(int line, bool restoreBg) {
	int x = _x + 1 + _leftPadding;
	int start = _scrollLine - _linesPerPage + 1;
	int y = _y + 2 + _topPadding;
	int limit = MIN(_pageWidth, (int)kLineWidth);

	y += line * kConsoleLineHeight;

	if (restoreBg) {
		Common::Rect r(_x, y - 2, _x + _pageWidth * kConsoleCharWidth, y+kConsoleLineHeight);
		g_gui.theme()->restoreBackground(r);
		g_gui.theme()->addDirtyRect(r);
	}

	for (int column = 0; column < limit; column++) {
#if 0
		int l = (start + line) % _linesInBuffer;
		byte c = buffer(l * kLineWidth + column);
#else
		byte c = buffer((start + line) * kLineWidth + column);
#endif
		g_gui.theme()->drawChar(Common::Rect(x, y, x+kConsoleCharWidth, y+kConsoleLineHeight), c, _font);
		x += kConsoleCharWidth;
	}
}

void ConsoleDialog::handleScreenChanged() {
	init();

	_scrollLine = _promptEndPos / kLineWidth;
	if (_scrollLine < _linesPerPage - 1)
		_scrollLine = _linesPerPage - 1;
	updateScrollBuffer();

	Dialog::handleScreenChanged();
	draw();
}

void ConsoleDialog::handleTickle() {
	uint32 time = g_system->getMillis();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}

	// Perform the "slide animation".
	if (_slideMode != kNoSlideMode) {
		_drawingHints = THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND;
		const float tmp = (float)(g_system->getMillis() - _slideTime) / kConsoleSlideDownDuration;
		if (_slideMode == kUpSlideMode) {
			_y = (int)(_h * (0.0 - tmp));
		} else {
			_y = (int)(_h * (tmp - 1.0));
		}

		if (_slideMode == kDownSlideMode && _y > 0) {
			// End the slide
			_slideMode = kNoSlideMode;
			_y = 0;
			draw();
		} else if (_slideMode == kUpSlideMode && _y <= -_h) {
			// End the slide
			//_slideMode = kNoSlideMode;
			close();
		} else
			draw();
	}
}

void ConsoleDialog::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}

void ConsoleDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	int i;

	if (_slideMode != kNoSlideMode)
		return;

	switch (keycode) {
	case '\n':	// enter/return
	case '\r': {
		if (_caretVisible)
			drawCaret(true);

		nextLine();

		assert(_promptEndPos >= _promptStartPos);
		int len = _promptEndPos - _promptStartPos;
		bool keepRunning = true;


		if (len > 0) {

			// We have to allocate the string buffer with new, since VC++ sadly does not
			// comply to the C++ standard, so we can't use a dynamic sized stack array.
			char *str = new char[len + 1];

			// Copy the user input to str
			for (i = 0; i < len; i++)
				str[i] = buffer(_promptStartPos + i);
			str[len] = '\0';

			// Add the input to the history
			addToHistory(str);

			// Pass it to the input callback, if any
			if (_callbackProc)
				keepRunning = (*_callbackProc)(this, str, _callbackRefCon);

				// Get rid of the string buffer
			delete [] str;
		}

		print(PROMPT);
		_promptStartPos = _promptEndPos = _currentPos;

		draw();
		if (!keepRunning)
			slideUpAndClose();
		break;
		}
	case 27:	// escape
		slideUpAndClose();
		break;
	case 8:		// backspace
		if (_caretVisible)
			drawCaret(true);

		if (_currentPos > _promptStartPos) {
			_currentPos--;
			killChar();
		}
		scrollToCurrent();
		drawLine(pos2line(_currentPos));
		break;
	case 9: // tab
	{
		if (_completionCallbackProc) {
			int len = _currentPos - _promptStartPos;
			assert(len >= 0);
			char *str = new char[len + 1];

			// Copy the user input to str
			for (i = 0; i < len; i++)
				str[i] = buffer(_promptStartPos + i);
			str[len] = '\0';

			char *completion = 0;
			if ((*_completionCallbackProc)(this, str, completion, _callbackRefCon)) {
				if (_caretVisible)
					drawCaret(true);
				insertIntoPrompt(completion);
				scrollToCurrent();
				drawLine(pos2line(_currentPos));
				delete[] completion;
			}
			delete[] str;
		}
		break;
	}
	case 127:
		killChar();
		drawLine(pos2line(_currentPos));
		break;
	case 256 + 24:	// pageup
		if (modifiers == OSystem::KBD_SHIFT) {
			_scrollLine -= _linesPerPage - 1;
			if (_scrollLine < _firstLineInBuffer + _linesPerPage - 1)
				_scrollLine = _firstLineInBuffer + _linesPerPage - 1;
			updateScrollBuffer();
			draw();
		}
		break;
	case 256 + 25:	// pagedown
		if (modifiers == OSystem::KBD_SHIFT) {
			_scrollLine += _linesPerPage - 1;
			if (_scrollLine > _promptEndPos / kLineWidth) {
				_scrollLine = _promptEndPos / kLineWidth;
				if (_scrollLine < _firstLineInBuffer + _linesPerPage - 1)
					_scrollLine = _firstLineInBuffer + _linesPerPage - 1;
			}
			updateScrollBuffer();
			draw();
		}
		break;
	case 256 + 22:	// home
		if (modifiers == OSystem::KBD_SHIFT) {
			_scrollLine = _firstLineInBuffer + _linesPerPage - 1;
			updateScrollBuffer();
		} else {
			_currentPos = _promptStartPos;
		}
		draw();
		break;
	case 256 + 23:	// end
		if (modifiers == OSystem::KBD_SHIFT) {
			_scrollLine = _promptEndPos / kLineWidth;
			if (_scrollLine < _linesPerPage - 1)
				_scrollLine = _linesPerPage - 1;
			updateScrollBuffer();
		} else {
			_currentPos = _promptEndPos;
		}
		draw();
		break;
	case 273:	// cursor up
		historyScroll(+1);
		break;
	case 274:	// cursor down
		historyScroll(-1);
		break;
	case 275:	// cursor right
		if (_currentPos < _promptEndPos)
			_currentPos++;
		drawLine(pos2line(_currentPos));
		break;
	case 276:	// cursor left
		if (_currentPos > _promptStartPos)
			_currentPos--;
		drawLine(pos2line(_currentPos));
		break;
	default:
		if (ascii == '~' || ascii == '#') {
			slideUpAndClose();
		} else if (modifiers == OSystem::KBD_CTRL) {
			specialKeys(keycode);
		} else if ((ascii >= 32 && ascii <= 127) || (ascii >= 160 && ascii <= 255)) {
			for (i = _promptEndPos - 1; i >= _currentPos; i--)
				buffer(i + 1) = buffer(i);
			_promptEndPos++;
			putchar((byte)ascii);
			scrollToCurrent();
		}
	}
}

void ConsoleDialog::insertIntoPrompt(const char* str) {
	unsigned int l = strlen(str);
	for (int i = _promptEndPos - 1; i >= _currentPos; i--)
		buffer(i + l) = buffer(i);
	for (unsigned int j = 0; j < l; ++j) {
		_promptEndPos++;
		putcharIntern(str[j]);
	}
}

void ConsoleDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		int newPos = (int)data + _linesPerPage - 1 + _firstLineInBuffer;
		if (newPos != _scrollLine) {
			_scrollLine = newPos;
			draw();
		}
		break;
	}
}

void ConsoleDialog::specialKeys(int keycode) {
	switch (keycode) {
	case 'a':
		_currentPos = _promptStartPos;
		draw();
		break;
	case 'd':
		if (_currentPos < _promptEndPos) {
			killChar();
			draw();
		}
		break;
	case 'e':
		_currentPos = _promptEndPos;
		draw();
		break;
	case 'k':
		killLine();
		draw();
		break;
	case 'w':
		killLastWord();
		draw();
		break;
	}
}

void ConsoleDialog::killChar() {
	for (int i = _currentPos; i < _promptEndPos; i++)
		buffer(i) = buffer(i + 1);
	buffer(_promptEndPos) = ' ';
	_promptEndPos--;
}

void ConsoleDialog::killLine() {
	for (int i = _currentPos; i < _promptEndPos; i++)
		buffer(i) = ' ';
	_promptEndPos = _currentPos;
}

void ConsoleDialog::killLastWord() {
	int cnt = 0;
	bool space = true;
	while (_currentPos > _promptStartPos) {
		if (buffer(_currentPos - 1) == ' ') {
			if (!space)
				break;
		} else
			space = false;
		_currentPos--;
		cnt++;
	}

	for (int i = _currentPos; i < _promptEndPos; i++)
		buffer(i) = buffer(i + cnt);
	buffer(_promptEndPos) = ' ';
	_promptEndPos -= cnt;
}

void ConsoleDialog::addToHistory(const char *str) {
	strcpy(_history[_historyIndex], str);
	_historyIndex = (_historyIndex + 1) % kHistorySize;
	_historyLine = 0;
	if (_historySize < kHistorySize)
		_historySize++;
}

void ConsoleDialog::historyScroll(int direction) {
	if (_historySize == 0)
		return;

	if (_historyLine == 0 && direction > 0) {
		int i;
		for (i = 0; i < _promptEndPos - _promptStartPos; i++)
			_history[_historyIndex][i] = buffer(_promptStartPos + i);
		_history[_historyIndex][i] = '\0';
	}

	// Advance to the next line in the history
	int line = _historyLine + direction;
	if ((direction < 0 && line < 0) || (direction > 0 && line > _historySize))
		return;
	_historyLine = line;

	// Hide caret if visible
	if (_caretVisible)
		drawCaret(true);

	// Remove the current user text
	_currentPos = _promptStartPos;
	killLine();

	// ... and ensure the prompt is visible
	scrollToCurrent();

	// Print the text from the history
	int idx;
	if (_historyLine > 0)
		idx = (_historyIndex - _historyLine + _historySize) % _historySize;
	else
		idx = _historyIndex;
	for (int i = 0; i < kLineBufferSize && _history[idx][i] != '\0'; i++)
		putcharIntern(_history[idx][i]);
	_promptEndPos = _currentPos;

	// Ensure once more the caret is visible (in case of very long history entries)
	scrollToCurrent();

	draw();
}

void ConsoleDialog::nextLine() {
	int line = _currentPos / kLineWidth;
	if (line == _scrollLine)
		_scrollLine++;
	_currentPos = (line + 1) * kLineWidth;

	updateScrollBuffer();
}


// Call this (at least) when the current line changes or when
// a new line is added
void ConsoleDialog::updateScrollBuffer() {
	int lastchar = MAX(_promptEndPos, _currentPos);
	int line = lastchar / kLineWidth;
	int numlines = (line < _linesInBuffer) ? line + 1 : _linesInBuffer;
	int firstline = line - numlines + 1;
	if (firstline > _firstLineInBuffer) {
		// clear old line from buffer
		for (int i = lastchar; i < (line+1) * kLineWidth; ++i)
			buffer(i) = ' ';
		_firstLineInBuffer = firstline;
	}

	_scrollBar->_numEntries = numlines;
	_scrollBar->_currentPos = _scrollBar->_numEntries - (line - _scrollLine + _linesPerPage);
	_scrollBar->_entriesPerPage = _linesPerPage;
	_scrollBar->recalc();
}

int ConsoleDialog::printf(const char *format, ...) {
	va_list	argptr;

	va_start(argptr, format);
	int count = this->vprintf(format, argptr);
	va_end (argptr);
	return count;
}

int ConsoleDialog::vprintf(const char *format, va_list argptr) {
#ifdef PALMOS_MODE
	char	buf[256];
#else
	char	buf[2048];
#endif

#if defined(WIN32)
	int count = _vsnprintf(buf, sizeof(buf), format, argptr);
#elif defined(__SYMBIAN32__)
	int count = vsprintf(buf, format, argptr);
#else
	int count = vsnprintf(buf, sizeof(buf), format, argptr);
#endif
	print(buf);
	return count;
}

void ConsoleDialog::putchar(int c) {
	if (_caretVisible)
		drawCaret(true);

	putcharIntern(c);
	drawLine(pos2line(_currentPos));
}

void ConsoleDialog::putcharIntern(int c) {
	if (c == '\n')
		nextLine();
	else {
		buffer(_currentPos) = (char)c;
		_currentPos++;
		if ((_scrollLine + 1) * kLineWidth == _currentPos) {
			_scrollLine++;
			updateScrollBuffer();
		}
	}
}

void ConsoleDialog::print(const char *str) {
	if (_caretVisible)
		drawCaret(true);

	while (*str)
		putcharIntern(*str++);

	draw();
}

void ConsoleDialog::drawCaret(bool erase) {
	// TODO: use code from EditableWidget::drawCaret here
	int line = _currentPos / kLineWidth;
	int displayLine = line - _scrollLine + _linesPerPage - 1;

	// Only draw caret if visible
	if (!isVisible() || displayLine < 0 || displayLine >= _linesPerPage) {
		_caretVisible = false;
		return;
	}

	int x = _x + 1 + _leftPadding + (_currentPos % kLineWidth) * kConsoleCharWidth;
	int y = _y + _topPadding + displayLine * kConsoleLineHeight;

	_caretVisible = !erase;
	g_gui.theme()->drawCaret(Common::Rect(x, y, x+1, y+kConsoleLineHeight), erase);
}

void ConsoleDialog::scrollToCurrent() {
	int line = _promptEndPos / kLineWidth;

	if (line + _linesPerPage <= _scrollLine) {
		// TODO - this should only occur for loong edit lines, though
	} else if (line > _scrollLine) {
		_scrollLine = line;
		updateScrollBuffer();
	}
}

} // End of namespace GUI
