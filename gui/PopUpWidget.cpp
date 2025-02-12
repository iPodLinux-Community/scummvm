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
#include "common/system.h"
#include "gui/dialog.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/PopUpWidget.h"
#include "base/engine.h"

namespace GUI {

//
// PopUpDialog
//

class PopUpDialog : public Dialog {
protected:
	PopUpWidget	*_popUpBoss;
	int			_clickX, _clickY;
	byte		*_buffer;
	int			_selection;
	uint32		_openTime;
	bool		_twoColumns;
	int			_entriesPerColumn;

	int			_leftPadding;
	int			_rightPadding;

public:
	PopUpDialog(PopUpWidget *boss, int clickX, int clickY);

	void drawDialog();

	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseWheel(int x, int y, int direction);	// Scroll through entries with scroll wheel
	void handleMouseMoved(int x, int y, int button);	// Redraw selections depending on mouse position
	void handleKeyDown(uint16 ascii, int keycode, int modifiers);	// Scroll through entries with arrow keys etc.

protected:
	void drawMenuEntry(int entry, bool hilite);

	int findItem(int x, int y) const;
	void setSelection(int item);
	bool isMouseDown();

	void moveUp();
	void moveDown();
};

PopUpDialog::PopUpDialog(PopUpWidget *boss, int clickX, int clickY)
	: Dialog(0, 0, 16, 16, false),
	_popUpBoss(boss) {

	// Copy the selection index
	_selection = _popUpBoss->_selectedItem;

	// Calculate real popup dimensions
	_x = _popUpBoss->getAbsX() + _popUpBoss->_labelWidth + _popUpBoss->_labelSpacing;
	_y = _popUpBoss->getAbsY() - _popUpBoss->_selectedItem * kLineHeight;
	_h = _popUpBoss->_entries.size() * kLineHeight + 2;
	_w = _popUpBoss->_w - kLineHeight + 2 - _popUpBoss->_labelWidth - _popUpBoss->_labelSpacing;

	_leftPadding = _popUpBoss->_leftPadding;
	_rightPadding = _popUpBoss->_rightPadding;

	// Perform clipping / switch to scrolling mode if we don't fit on the screen
	// FIXME - OSystem should send out notification messages when the screen
	// resolution changes... we could generalize CommandReceiver and CommandSender.

	const int screenH = g_system->getOverlayHeight();

	// HACK: For now, we do not do scrolling. Instead, we draw the dialog
	// in two columns if it's too tall.

	if (_h >= screenH) {
		const int screenW = g_system->getOverlayWidth();

		_twoColumns = true;
		_entriesPerColumn = _popUpBoss->_entries.size() / 2;

		if (_popUpBoss->_entries.size() & 1)
			_entriesPerColumn++;

		_h = _entriesPerColumn * kLineHeight + 2;
		_w = 0;

		for (uint i = 0; i < _popUpBoss->_entries.size(); i++) {
			int width = g_gui.getStringWidth(_popUpBoss->_entries[i].name);

			if (width > _w)
				_w = width;
		}

		_w = 2 * _w + 10;

		if (!(_w & 1))
			_w++;

		if (_popUpBoss->_selectedItem >= _entriesPerColumn) {
			_x -= _w / 2;
			_y = _popUpBoss->getAbsY() - (_popUpBoss->_selectedItem - _entriesPerColumn) * kLineHeight;
		}

		if (_w >= screenW)
			_w = screenW - 1;
		if (_x < 0)
			_x = 0;
		if (_x + _w >= screenW)
			_x = screenW - 1 - _w;
	} else
		_twoColumns = false;

	if (_h >= screenH)
		_h = screenH - 1;
	if (_y < 0)
		_y = 0;
	else if (_y + _h >= screenH)
		_y = screenH - 1 - _h;

	// TODO - implement scrolling if we had to move the menu, or if there are too many entries

	// Remember original mouse position
	_clickX = clickX - _x;
	_clickY = clickY - _y;

	// Time the popup was opened
	_openTime = getMillis();
}

void PopUpDialog::drawDialog() {
	// Draw the menu border
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x+_w, _y+_h), THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND | THEME_HINT_USE_SHADOW);

	/*if (_twoColumns)
		g_gui.vLine(_x + _w / 2, _y, _y + _h - 2, g_gui._color);*/

	// Draw the entries
	int count = _popUpBoss->_entries.size();
	for (int i = 0; i < count; i++) {
		drawMenuEntry(i, i == _selection);
	}

	// The last entry may be empty. Fill it with black.
	/*if (_twoColumns && (count & 1)) {
		g_gui.fillRect(_x + 1 + _w / 2, _y + 1 + kLineHeight * (_entriesPerColumn - 1), _w / 2 - 1, kLineHeight, g_gui._bgcolor);
	}*/
}

void PopUpDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	// Mouse was released. If it wasn't moved much since the original mouse down,
	// let the popup stay open. If it did move, assume the user made his selection.
	int dist = (_clickX - x) * (_clickX - x) + (_clickY - y) * (_clickY - y);
	if (dist > 3 * 3 || getMillis() - _openTime > 300) {
		setResult(_selection);
		close();
	}
	_clickX = -1;
	_clickY = -1;
	_openTime = (uint32)-1;
}

void PopUpDialog::handleMouseWheel(int x, int y, int direction) {
	if (direction < 0)
		moveUp();
	else if (direction > 0)
		moveDown();
}

void PopUpDialog::handleMouseMoved(int x, int y, int button) {
	// Compute over which item the mouse is...
	int item = findItem(x, y);

	if (item >= 0 && _popUpBoss->_entries[item].name.size() == 0)
		item = -1;

	if (item == -1 && !isMouseDown())
		return;

	// ...and update the selection accordingly
	setSelection(item);
}

void PopUpDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (keycode == 27) {	// escape
		close();
		return;
	}

	if (isMouseDown())
		return;

	switch (keycode) {
	case '\n':		// enter/return
	case '\r':
		setResult(_selection);
		close();
		break;
	case 256+17:	// up arrow
		moveUp();
		break;
	case 256+18:	// down arrow
		moveDown();
		break;
	case 256+22:	// home
		setSelection(0);
		break;
	case 256+23:	// end
		setSelection(_popUpBoss->_entries.size()-1);
		break;
	}
}

int PopUpDialog::findItem(int x, int y) const {
	if (x >= 0 && x < _w && y >= 0 && y < _h) {
		if (_twoColumns) {
			uint entry = (y - 2) / kLineHeight;
			if (x > _w / 2) {
				entry += _entriesPerColumn;

				if (entry >= _popUpBoss->_entries.size())
					return -1;
			}
			return entry;
		}
		return (y - 2) / kLineHeight;
	}
	return -1;
}

void PopUpDialog::setSelection(int item) {
	if (item != _selection) {
		// Undraw old selection
		if (_selection >= 0)
			drawMenuEntry(_selection, false);

		// Change selection
		_selection = item;

		// Draw new selection
		if (item >= 0)
			drawMenuEntry(item, true);
	}
}

bool PopUpDialog::isMouseDown() {
	// TODO/FIXME - need a way to determine whether any mouse buttons are pressed or not.
	// Sure, we could just count mouse button up/down events, but that is cumbersome and
	// error prone. Would be much nicer to add an API to OSystem for this...

	return false;
}

void PopUpDialog::moveUp() {
	if (_selection < 0) {
		setSelection(_popUpBoss->_entries.size() - 1);
	} else if (_selection > 0) {
		int item = _selection;
		do {
			item--;
		} while (item >= 0 && _popUpBoss->_entries[item].name.size() == 0);
		if (item >= 0)
			setSelection(item);
	}
}

void PopUpDialog::moveDown() {
	int lastItem = _popUpBoss->_entries.size() - 1;

	if (_selection < 0) {
		setSelection(0);
	} else if (_selection < lastItem) {
		int item = _selection;
		do {
			item++;
		} while (item <= lastItem && _popUpBoss->_entries[item].name.size() == 0);
		if (item <= lastItem)
			setSelection(item);
	}
}

void PopUpDialog::drawMenuEntry(int entry, bool hilite) {
	// Draw one entry of the popup menu, including selection
	assert(entry >= 0);
	int x, y, w;

	if (_twoColumns) {
		int n = _popUpBoss->_entries.size() / 2;

		if (_popUpBoss->_entries.size() & 1)
			n++;

		if (entry >= n) {
			x = _x + 1 + _w / 2;
			y = _y + 1 + kLineHeight * (entry - n);
		} else {
			x = _x + 1;
			y = _y + 1 + kLineHeight * entry;
		}

		w = _w / 2 - 1;
	} else {
		x = _x + 1;
		y = _y + 1 + kLineHeight * entry;
		w = _w - 2;
	}

	Common::String &name(_popUpBoss->_entries[entry].name);

	if (name.size() == 0) {
		// Draw a separator
		g_gui.theme()->drawLineSeparator(Common::Rect(x, y, x+w, y+kLineHeight));
	} else {
		g_gui.theme()->drawText(Common::Rect(x+1, y+2, x+w, y+2+kLineHeight), name,	hilite ? Theme::kStateHighlight : Theme::kStateEnabled,
								Theme::kTextAlignLeft, false, _leftPadding);
	}
}


#pragma mark -

//
// PopUpWidget
//

PopUpWidget::PopUpWidget(GuiObject *boss, const String &name, const String &label, uint labelWidth)
	: Widget(boss, name), CommandSender(boss), _label(label), _labelWidth(labelWidth) {
	handleScreenChanged();

	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS;
	setHints(THEME_HINT_SAVE_BACKGROUND);
	_type = kPopUpWidget;

	_selectedItem = -1;

	if (!_label.empty() && _labelWidth == 0)
		_labelWidth = g_gui.getStringWidth(_label);
}

void PopUpWidget::handleMouseDown(int x, int y, int button, int clickCount) {

	if (isEnabled()) {
		PopUpDialog popupDialog(this, x + getAbsX(), y + getAbsY());
		int newSel = popupDialog.runModal();
		if (newSel != -1 && _selectedItem != newSel) {
			_selectedItem = newSel;
			sendCommand(kPopUpItemSelectedCmd, _entries[_selectedItem].tag);
		}
		g_gui.clearDragWidget();
	}
}

void PopUpWidget::handleScreenChanged() {
	_leftPadding = g_gui.evaluator()->getVar("PopUpWidget.leftPadding", 0);
	_rightPadding = g_gui.evaluator()->getVar("PopUpWidget.rightPadding", 0);
	_labelSpacing = g_gui.evaluator()->getVar("PopUpWidget.labelSpacing", 0);

	Widget::handleScreenChanged();
}

void PopUpWidget::appendEntry(const String &entry, uint32 tag) {
	Entry e;
	e.name = entry;
	e.tag = tag;
	_entries.push_back(e);
}

void PopUpWidget::clearEntries() {
	_entries.clear();
	_selectedItem = -1;
}

void PopUpWidget::setSelected(int item) {
	// FIXME
	if (item != _selectedItem) {
		if (item >= 0 && item < (int)_entries.size()) {
			_selectedItem = item;
		} else {
			_selectedItem = -1;
		}
	}
}

void PopUpWidget::setSelectedTag(uint32 tag) {
	uint item;
	for (item = 0; item < _entries.size(); ++item) {
		if (_entries[item].tag == tag) {
			setSelected(item);
			return;
		}
	}
}

void PopUpWidget::drawWidget(bool hilite) {
	int x = _x + _labelWidth + _labelSpacing;
	int w = _w - _labelWidth - _labelSpacing;

	// Draw the label, if any
	if (_labelWidth > 0)
		g_gui.theme()->drawText(Common::Rect(_x+2,_y+3,_x+2+_labelWidth, _y+3+g_gui.theme()->getFontHeight()), _label,
								isEnabled() ? Theme::kStateEnabled : Theme::kStateDisabled, Theme::kTextAlignRight);

	Common::String sel;
	if (_selectedItem >= 0)
		sel = _entries[_selectedItem].name;
	g_gui.theme()->drawPopUpWidget(Common::Rect(x, _y, x+w, _y+_h), sel, _leftPadding, isEnabled() ? (hilite ? Theme::kStateHighlight : Theme::kStateEnabled) : Theme::kStateDisabled, g_gui.theme()->convertAligment(kTextAlignLeft));
}

} // End of namespace GUI
