/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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

#ifndef BSCONTROL_H
#define BSCONTROL_H

#include "common/scummsys.h"
#include "sword1/sworddefs.h"

class OSystem;
namespace Common {
	class SaveFileManager;
}

namespace Sword1 {

class ObjectMan;
class ResMan;
class Mouse;
class Music;
class Sound;

#define MAX_BUTTONS 16

#define CONTROL_NOTHING_DONE 0
#define CONTROL_GAME_RESTORED 1
#define CONTROL_RESTART_GAME 2

class ControlButton {
public:
	ControlButton(uint16 x, uint16 y, uint32 resId, uint8 id, uint8 flag, ResMan *pResMan, uint8 *screenBuf, OSystem *system);
	~ControlButton(void);
	void draw(void);
	bool wasClicked(uint16 mouseX, uint16 mouseY);
	void setSelected(uint8 selected);
	bool isSaveslot(void);
	uint8 _id;
	uint8 _flag;
private:
	int _frameIdx;
	uint16 _x, _y;
	uint16 _width, _height;
	uint32 _resId;
	ResMan *_resMan;
	uint8 *_dstBuf;
	OSystem *_system;
};

enum {
	kButtonOk = 1,
	kButtonCancel = 2
};

struct ButtonInfo {
	uint16 x, y;
	uint32 resId, id;
	uint8 flag;
};

class Control {
public:
	Control(Common::SaveFileManager *saveFileMan, ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, Mouse *pMouse, Sound *pSound, Music *pMusic);
	uint8 runPanel(void);
	void doRestore(void);
	void askForCd(void);
	bool savegamesExist(void);
	bool restoreGameFromFile(uint8 slot);
private:
	int displayMessage(const char *altButton, const char *message, ...);

	void saveGameToFile(uint8 slot);
	void readSavegameDescriptions(void);
	void writeSavegameDescriptions(void);
	void showSavegameNames(void);
	void deselectSaveslots(void);
	uint8 *_restoreBuf;
	uint8 _saveFiles;
	uint8 _saveScrollPos;
	uint8 _selectedSavegame;
	uint8 _saveNames[64][32];
	uint8 _oldName[32];
	uint8 _cursorTick;
	bool _cursorVisible;

	uint8 getClicks(uint8 mode, uint8 *retVal);
	uint8 handleButtonClick(uint8 id, uint8 mode, uint8 *retVal);
	void handleVolumeClicks(void);
	void changeVolume(uint8 id, uint8 action);

	void setupMainPanel(void);
	void setupSaveRestorePanel(bool saving);
	void setupVolumePanel(void);
	bool getConfirm(const uint8 *title);

	void saveNameScroll(uint8 scroll, bool saving);
	void saveNameSelect(uint8 id, bool saving);
	bool saveToFile(void);
	bool restoreFromFile(void);
	bool keyAccepted(uint8 key);
	void handleSaveKey(uint8 key);

	void renderVolumeBar(uint8 id, uint8 volL, uint8 volR);
	uint16 getTextWidth(const uint8 *str);
	void renderText(const uint8 *str, uint16 x, uint16 y, uint8 mode);
	uint8 _numButtons;
	uint8 _selectedButton;
	void createButtons(const ButtonInfo *buttons, uint8 num);
	void destroyButtons(void);
	ControlButton *_buttons[MAX_BUTTONS];
	static const ButtonInfo _deathButtons[3], _panelButtons[7], _saveButtons[16], _volumeButtons[4];
	static const uint8 _languageStrings[8 * 20][43];
	const uint8 (*_lStrings)[43];
	Common::SaveFileManager *_saveFileMan;
	ObjectMan *_objMan;
	ResMan *_resMan;
	OSystem *_system;
	Mouse *_mouse;
	Music *_music;
	Sound *_sound;
	uint8 *_font, *_redFont;
	uint8 *_screenBuf;
	uint8 _keyPressed;
	uint8 _keyRepeat;
	uint32 _keyRepeatTime;
	void delay(uint32 msecs);
	uint16 _mouseX, _mouseY, _mouseState;
	bool _mouseDown;
};

} // End of namespace Sword1

#endif //BSCONTROL_H

