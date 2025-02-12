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

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/util.h"

#include "sword1/objectman.h"
#include "sword1/sworddefs.h"
#include "sword1/swordres.h"
#include "sword1/sword1.h"

namespace Sword1 {

ObjectMan::ObjectMan(ResMan *pResourceMan) {
	_resMan = pResourceMan;
}

void ObjectMan::initialize(void) {
	uint16 cnt;
	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		_liveList[cnt] = 0; // we don't need to close the files here. When this routine is
							// called, the memory was flushed() anyways, so these resources
							// already *are* closed.

	_liveList[128] = _liveList[129] = _liveList[130] = _liveList[131] = _liveList[133] =
		_liveList[134] = _liveList[145] = _liveList[146] = _liveList[TEXT_sect] = 1;

	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++) {
		if (_liveList[cnt])
			_cptData[cnt] = (uint8*)_resMan->cptResOpen(_objectList[cnt]) + sizeof(Header);
		else
			_cptData[cnt] = NULL;
	}
}

ObjectMan::~ObjectMan(void) {
	for (uint16 cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		if (_liveList[cnt])
			_resMan->resClose(_objectList[cnt]);
}

bool ObjectMan::sectionAlive(uint16 section) {
	return (_liveList[section] > 0);
}

void ObjectMan::megaEntering(uint16 section) {
	_liveList[section]++;
	if (_liveList[section] == 1)
		_cptData[section] = ((uint8*)_resMan->cptResOpen(_objectList[section])) + sizeof(Header);
}

void ObjectMan::megaLeaving(uint16 section, int id) {
	if (_liveList[section] == 0)
		error("mega %d is leaving empty section %d", id, section);
	_liveList[section]--;
	if ((_liveList[section] == 0) && (id != PLAYER)) {
		_resMan->resClose(_objectList[section]);
		_cptData[section] = NULL;
	}
	/* if the player is leaving the section then we have to close the resources after
	   mainloop ends, because the screen will still need the resources*/
}

uint8 ObjectMan::fnCheckForTextLine(uint32 textId) {
	uint8 retVal = 0;
	if (!_textList[textId / ITM_PER_SEC][0])
		return 0; // section does not exist

	uint8 lang = SwordEngine::_systemVars.language;
	uint32 *textData = (uint32*)((uint8*)_resMan->openFetchRes(_textList[textId / ITM_PER_SEC][lang]) + sizeof(Header));
	if ((textId & ITM_ID) < READ_LE_UINT32(textData)) {
		textData++;
		if (textData[textId & ITM_ID])
			retVal = 1;
	}
	_resMan->resClose(_textList[textId / ITM_PER_SEC][lang]);
	return retVal;
}

char *ObjectMan::lockText(uint32 textId) {
	uint8 lang = SwordEngine::_systemVars.language;
	char *addr = (char*)_resMan->openFetchRes(_textList[textId / ITM_PER_SEC][lang]) + sizeof(Header);
	if ((textId & ITM_ID) >= READ_LE_UINT32(addr)) {
		warning("ObjectMan::lockText(%d): only %d texts in file", textId & ITM_ID, READ_LE_UINT32(addr));
		textId = 0; // get first line instead
	}
	uint32 offset = READ_LE_UINT32(addr + ((textId & ITM_ID) + 1)* 4);
	if (offset == 0) {
		warning("ObjectMan::lockText(%d): text number has no text lines", textId);
		return _errorStr;
	}
	return addr + offset;
}

void ObjectMan::unlockText(uint32 textId) {
	_resMan->resClose(_textList[textId / ITM_PER_SEC][SwordEngine::_systemVars.language]);
}

uint32 ObjectMan::lastTextNumber(int section) {
	uint8 *data = (uint8*)_resMan->openFetchRes(_textList[section][SwordEngine::_systemVars.language]) + sizeof(Header);
	uint32 result = READ_LE_UINT32(data) - 1;
	_resMan->resClose(_textList[section][SwordEngine::_systemVars.language]);
	return result;
}

Object *ObjectMan::fetchObject(uint32 id) {
	uint8 *addr = _cptData[id / ITM_PER_SEC];
	if (!addr)
		error("fetchObject: section %d is not open!", id / ITM_PER_SEC);
	id &= ITM_ID;
	// DON'T do endian conversion here. it's already done.
	return (Object*)(addr + *(uint32*)(addr + (id + 1)*4));
}

uint32 ObjectMan::fetchNoObjects(int section) {
	if (_cptData[section] == NULL)
		error("fetchNoObjects: section %d is not open!", section);
	return *(uint32*)_cptData[section];
}

void ObjectMan::closeSection(uint32 screen) {
	if (_liveList[screen] == 0)	 // close the section that PLAYER has just left, if it's empty now
		_resMan->resClose(_objectList[screen]);
}

void ObjectMan::loadLiveList(uint16 *src) {
	for (uint16 cnt = 0; cnt < TOTAL_SECTIONS; cnt++) {
		if (_liveList[cnt]) {
			_resMan->resClose(_objectList[cnt]);
			_cptData[cnt] = NULL;
		}
		_liveList[cnt] = src[cnt];
		if (_liveList[cnt])
			_cptData[cnt] = ((uint8*)_resMan->cptResOpen(_objectList[cnt])) + sizeof(Header);
	}
}

void ObjectMan::saveLiveList(uint16 *dest) {
	memcpy(dest, _liveList, TOTAL_SECTIONS * sizeof(uint16));
}

char ObjectMan::_errorStr[] = "Error: Text not found.";

} // End of namespace Sword1
