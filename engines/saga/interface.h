/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Game interface module private header file

#ifndef SAGA_INTERFACE_H__
#define SAGA_INTERFACE_H__

#include "common/savefile.h"

#include "saga/sprite.h"
#include "saga/script.h"

namespace Saga {

enum InterfaceUpdateFlags {
	UPDATE_MOUSEMOVE = 1,
	UPDATE_LEFTBUTTONCLICK = 2,
	UPDATE_RIGHTBUTTONCLICK = 4,
	UPDATE_MOUSECLICK = UPDATE_LEFTBUTTONCLICK | UPDATE_RIGHTBUTTONCLICK,
	UPDATE_WHEELUP = 8,
	UPDATE_WHEELDOWN = 16
};

#define CONVERSE_MAX_TEXTS      64
#define CONVERSE_MAX_WORK_STRING      128

#define ITE_INVENTORY_SIZE 24

#define VERB_STRLIMIT 32

#define STATUS_TEXT_LEN 128
#define STATUS_TEXT_INPUT_MAX 256

// Converse-specific stuff

enum PanelModes {
	kPanelNull,
	kPanelMain,
	kPanelOption,
	kPanelSave, //ex- kPanelTextBox,
	kPanelQuit,
	kPanelError,
	kPanelLoad,
	kPanelConverse,
	kPanelProtect,
	kPanelPlacard,
	kPanelMap,
	kPanelSceneSubstitute,
	kPanelChapterSelection,
	kPanelCutaway,
	kPanelVideo,
	kPanelBoss
//	kPanelInventory
};

enum FadeModes {
	kNoFade = 0,
	kFadeIn,
	kFadeOut
};

struct InterfacePanel {
	int x;
	int y;
	byte *image;
	size_t imageLength;
	int imageWidth;
	int imageHeight;

	PanelButton *currentButton;
	int buttonsCount;
	PanelButton *buttons;
	SpriteList sprites;

	InterfacePanel() {
		x = y = 0;
		image = NULL;
		imageLength = 0;
		imageWidth = imageHeight = 0;
		currentButton = NULL;
		buttonsCount = 0;
		buttons = NULL;
	}

	PanelButton *getButton(int index) {
		if ((index >= 0) && (index < buttonsCount)) {
			return &buttons[index];
		}
		return NULL;
	}

	void getRect(Rect &rect) {
		rect.left = x;
		rect.top = y;
		rect.setWidth(imageWidth);
		rect.setHeight(imageHeight);
	}

	void calcPanelButtonRect(const PanelButton* panelButton, Rect &rect) {
		rect.left = x + panelButton->xOffset;
		rect.right = rect.left + panelButton->width;
		rect.top = y + panelButton->yOffset;
		rect.bottom = rect.top + panelButton->height;
	}

	PanelButton *hitTest(const Point& mousePoint, int buttonType) {
		PanelButton *panelButton;
		Rect rect;
		int i;
		for (i = 0; i < buttonsCount; i++) {
			panelButton = &buttons[i];
			if (panelButton != NULL) {
				if ((panelButton->type & buttonType) > 0) {
					calcPanelButtonRect(panelButton, rect);
					if (rect.contains(mousePoint)) {
						return panelButton;
					}
				}
			}
		}
		return NULL;
	}

	void zeroAllButtonState() {
		int i;
		for (i = 0; i < buttonsCount; i++) {
			buttons[i].state = 0;
		}
	}


};

struct Converse {
	char *text;
	int stringNum;
	int textNum;
	int replyId;
	int replyFlags;
	int replyBit;
};


enum StatusTextInputState {
	kStatusTextInputFirstRun,
	kStatusTextInputEntered,
	kStatusTextInputAborted
};

class Interface {
public:
	Interface(SagaEngine *vm);
	~Interface(void);

	int activate();
	int deactivate();
	void setSaveReminderState(int state) {
		_saveReminderState = state;
		draw();
	}
	int getSaveReminderState() {
		return _saveReminderState;
	}
	bool isActive() { return _active; }
	void setMode(int mode);
	int getMode(void) const { return _panelMode; }
	void setFadeMode(int fadeMode) {
		_fadeMode = fadeMode;
		draw();
	}
	int getFadeMode() const {
		return _fadeMode;
	}
	void rememberMode();
	void restoreMode();
	bool isInMainMode() { return _inMainMode; }
	void setStatusText(const char *text, int statusColor = -1);
	void loadScenePortraits(int resourceId);
	void setLeftPortrait(int portrait) {
		_leftPortrait = portrait;
		draw();
	}
	void setRightPortrait(int portrait) {
		_rightPortrait = portrait;
		draw();
	}
	void setPortraitBgColor(int red, int green, int blue) {
		_portraitBgColor.red = red;
		_portraitBgColor.green = green;
		_portraitBgColor.blue = blue;
	}

	void draw();
	void drawOption();
	void drawQuit();
	void drawLoad();
	void drawSave();
	void drawProtect();
	void update(const Point& mousePoint, int updateFlag);
	void drawStatusBar();
	void setVerbState(int verb, int state);

	bool processAscii(uint16 ascii, bool synthetic = false);
	void processKeyUp(uint16 ascii);

	void keyBoss();
	void keyBossExit();

	void disableAbortSpeeches(bool d) { _disableAbortSpeeches = d; }

	bool _textInput;

	bool _statusTextInput;
	StatusTextInputState _statusTextInputState;
	char _statusTextInputString[STATUS_TEXT_INPUT_MAX];
	void enterStatusString() {
		_statusTextInput = true;
		_statusTextInputPos = 0;
		_statusTextInputString[0] = 0;
		setStatusText(_statusTextInputString);
	}

private:
	static void textInputRepeatCallback(void *refCon);

	void drawInventory(Surface *backBuffer);
	void updateInventory(int pos);
	void inventoryChangePos(int chg);
	void inventorySetPos(int key);

public:
	void refreshInventory() {
		updateInventory(_inventoryCount);
		draw();
	}
	void addToInventory(int objectId);
	void removeFromInventory(int objectId);
	void clearInventory();
	int inventoryItemPosition(int objectId);
	int getInventoryContentByPanelButton(PanelButton * panelButton) {
		int cell = _inventoryStart + panelButton->id;
		if (cell >= _inventoryCount) {
			return 0;
		}
		return _inventory[cell];
	}

	PanelButton *inventoryHitTest(const Point& mousePoint) {
		return _mainPanel.hitTest(mousePoint, kPanelButtonInventory);
	}
	PanelButton *verbHitTest(const Point& mousePoint){
		return _mainPanel.hitTest(mousePoint, kPanelButtonVerb);
	}
	void saveState(Common::OutSaveFile *out);
	void loadState(Common::InSaveFile *in);

	void mapPanelDrawCrossHair();

	int32 getProtectHash() { return _protectHash; }

private:
	void handleMainUpdate(const Point& mousePoint);					// main panel update
	void handleMainClick(const Point& mousePoint);					// main panel click

	PanelButton *converseHitTest(const Point& mousePoint) {
		return _conversePanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleConverseUpdate(const Point& mousePoint);				// converse panel update
	void handleConverseClick(const Point& mousePoint);				// converse panel click

	PanelButton *optionHitTest(const Point& mousePoint) {
		return _optionPanel.hitTest(mousePoint, kPanelButtonOptionSaveFiles | kPanelButtonOption | kPanelButtonOptionSlider);
	}
	void handleOptionUpdate(const Point& mousePoint);				// option panel update
	void handleOptionClick(const Point& mousePoint);				// option panel click

	PanelButton *quitHitTest(const Point& mousePoint) {
		return _quitPanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleQuitUpdate(const Point& mousePoint);					// quit panel update
	void handleQuitClick(const Point& mousePoint);					// quit panel click

	PanelButton *loadHitTest(const Point& mousePoint) {
		return _loadPanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleLoadUpdate(const Point& mousePoint);					// load panel update
	void handleLoadClick(const Point& mousePoint);					// load panel click

	PanelButton *saveHitTest(const Point& mousePoint) {
		return _savePanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleSaveUpdate(const Point& mousePoint);					// save panel update
	void handleSaveClick(const Point& mousePoint);					// save panel click

	void handleChapterSelectionUpdate(const Point& mousePoint);
	void handleChapterSelectionClick(const Point& mousePoint);

	void mapPanelShow();
	void mapPanelClean();

	void lockMode() { _lockedMode = _panelMode; }
	void unlockMode() { _panelMode = _lockedMode; }

	void setOption(PanelButton *panelButton);
	void setQuit(PanelButton *panelButton);
	void setLoad(PanelButton *panelButton);
	void setSave(PanelButton *panelButton);

	void drawTextInput(Surface *ds, InterfacePanel *panel, PanelButton *panelButton);
	void drawPanelText(Surface *ds, InterfacePanel *panel, PanelButton *panelButton);
	void drawPanelButtonText(Surface *ds, InterfacePanel *panel, PanelButton *panelButton);
	enum ButtonKind {
		kButton,
		kSlider,
		kEdit
	};
	void drawButtonBox(Surface *ds, const Rect &rect, ButtonKind kind, bool down);
	void drawPanelButtonArrow(Surface *ds, InterfacePanel *panel, PanelButton *panelButton);
	void drawVerbPanelText(Surface *ds, PanelButton *panelButton, KnownColor textKnownColor, KnownColor textShadowKnownColor);
	void drawVerbPanel(Surface *backBuffer, PanelButton* panelButton);
	void calcOptionSaveSlider();
	bool processTextInput(uint16 ascii);
	void processStatusTextInput(uint16 ascii);
	void textInputStartRepeat(uint16 ascii);
	void textInputRepeat(void);

public:
	void converseInit(void);
	void converseClear(void);
	bool converseAddText(const char *text, int replyId, byte replyFlags, int replyBit);
	void converseDisplayText();
	void converseSetTextLines(int row);
	void converseChangePos(int chg);
	void converseSetPos(int key);

private:
	void converseDisplayTextLines(Surface *ds);
	PanelButton *getPanelButtonByVerbType(int verb) {
		if ((verb < 0) || (verb >= kVerbTypeIdsMax)) {
			error("Interface::getPanelButtonByVerbType wrong verb");
		}
		return _verbTypeToPanelButton[verb];
	}

	void validateOptionButtons() {
		if (!_vm->isSaveListFull() && (_optionSaveFileTitleNumber == 0) && (_optionPanel.currentButton != NULL)) {
			if (_optionPanel.currentButton->id == kTextLoad) {
				_optionPanel.currentButton = NULL;
			}
		}
	}
	void validateSaveButtons() {
		if ((_textInputStringLength == 0) && (_savePanel.currentButton != NULL)) {
			if (_savePanel.currentButton->id == kTextSave) {
				_savePanel.currentButton = NULL;
			}
		}
	}

public:
	SpriteList _defPortraits;

private:
	SagaEngine *_vm;

	ResourceContext *_interfaceContext;
	InterfacePanel _mainPanel;
	PanelButton *_inventoryUpButton;
	PanelButton *_inventoryDownButton;
	InterfacePanel _conversePanel;
	PanelButton *_converseUpButton;
	PanelButton *_converseDownButton;
	SpriteList _scenePortraits;
	PanelButton *_verbTypeToPanelButton[kVerbTypeIdsMax];
	InterfacePanel _optionPanel;
	PanelButton * _optionSaveFileSlider;
	PanelButton * _optionSaveFilePanel;
	InterfacePanel _quitPanel;
	InterfacePanel _loadPanel;
	InterfacePanel _savePanel;
	PanelButton * _saveEdit;
	InterfacePanel _protectPanel;
	PanelButton * _protectEdit;

	bool _disableAbortSpeeches;

	int _saveReminderState;
	bool _active;
	int _fadeMode;
	int _panelMode;
	int _savedMode;
	int _lockedMode;
	int _bossMode;
	bool _inMainMode;
	char _statusText[STATUS_TEXT_LEN];
	int _statusOnceColor;
	int _leftPortrait;
	int _rightPortrait;
	PalEntry _portraitBgColor;

	Point _lastMousePoint;

	uint16 *_inventory;
	int _inventorySize;
	int _inventoryStart;
	int _inventoryEnd;
	int _inventoryPos;
	int _inventoryBox;
	int _inventoryCount;

	char _converseWorkString[CONVERSE_MAX_WORK_STRING];
	Converse _converseText[CONVERSE_MAX_TEXTS];
	int _converseTextCount;
	int _converseStrCount;
	int _converseStartPos;
	int _converseEndPos;
	int _conversePos;

	uint _optionSaveFileTop;
	uint _optionSaveFileTitleNumber;
	int16 _optionSaveFileMouseOff;
	Rect _optionSaveRectTop;
	Rect _optionSaveRectSlider;
	Rect _optionSaveRectBottom;

	char _textInputString[SAVE_TITLE_SIZE];
	uint _textInputStringLength;
	uint _textInputPos;
	uint _textInputMaxWidth;

	uint _statusTextInputPos;

	int _textInputRepeatPhase;
	uint16 _textInputRepeatChar;

	PalEntry _mapSavedPal[PAL_ENTRIES];
	bool _mapPanelCrossHairState;

	int32 _protectHash;
};

} // End of namespace Saga

#endif				/* INTERFACE_H__ */
