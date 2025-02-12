/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "gmchannel.h"
#include "common/util.h"
#include "sound/mididrv.h"

namespace Sky {

GmChannel::GmChannel(uint8 *pMusicData, uint16 startOfData, MidiDriver *pMidiDrv, const byte *pInstMap, const byte *veloTab) {

	_musicData = pMusicData;
	_midiDrv = pMidiDrv;
	_channelData.midiChannelNumber = 0;
	_channelData.startOfData = startOfData;
	_channelData.eventDataPtr = startOfData;
	_channelData.channelActive = 1;
	_channelData.nextEventTime = getNextEventTime();
	_instMap = pInstMap;
	_veloTab = veloTab;

	_musicVolume = 0x7F;
	_lastVolume = 0xFF;
}

bool GmChannel::isActive(void) {

	return _channelData.channelActive != 0;
}

void GmChannel::updateVolume(uint16 pVolume) {

	_musicVolume = pVolume;
	if (_musicVolume > 0)
		_musicVolume = (_musicVolume * 2) / 3 + 43;
	if (_lastVolume < 0xFF) {
		uint8 newVol = (_lastVolume * _musicVolume) >> 7;
		_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x700 | (newVol << 16));
	}
}

void GmChannel::stopNote(void) {

	// All Notes Off
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x7B00 | 0 | 0x79000000);
	// Reset the Pitch Wheel. See bug #1016556.
	_midiDrv->send((0xE0 | _channelData.midiChannelNumber) | 0x400000);
}

int32 GmChannel::getNextEventTime(void) {

	int32 retV = 0;
	uint8 cnt, lVal = 0;
	for (cnt = 0; cnt < 4; cnt++) {
		lVal = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		retV = (retV << 7) | (lVal & 0x7F);
		if (!(lVal & 0x80)) break;
	}
	if (lVal & 0x80) { // should never happen
		return -1;
	} else return retV;

}

uint8 GmChannel::process(uint16 aktTime) {

	if (!_channelData.channelActive)
		return 0;

	uint8 returnVal = 0;

	_channelData.nextEventTime -= aktTime;
	uint8 opcode;

	while ((_channelData.nextEventTime < 0) && (_channelData.channelActive)) {
		opcode = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		if (opcode&0x80) {
			if (opcode == 0xFF) {
				// dummy opcode
			} else if (opcode >= 0x90) {
				switch (opcode&0xF) {
				case 0: com90_caseNoteOff(); break;
				case 1: com90_stopChannel(); break;
				case 2: com90_setupInstrument(); break;
				case 3:
					returnVal = com90_updateTempo();
					break;
				case 5: com90_getPitch(); break;
				case 6: com90_getChannelVolume(); break;
				case 8: com90_rewindMusic(); break;
				case 9: com90_keyOff(); break;
				case 11: com90_getChannelPanValue(); break;
				case 12: com90_setStartOfData(); break;
				case 13: com90_getChannelControl(); break;
				case 4: //com90_dummy();
				case 7: //com90_skipTremoVibro();
				case 10: //com90_error();
					error("Channel: dummy music routine 0x%02X was called",opcode);
					_channelData.channelActive = 0;
					break;
				default:
					// these opcodes aren't implemented in original music driver
					error("Channel: Not existent routine 0x%02X was called",opcode);
					_channelData.channelActive = 0;
					break;
				}
			} else {
				// new midi channel assignment
				_channelData.midiChannelNumber = opcode&0xF;
			}
		} else {
			_channelData.note = opcode;
			byte velocity = _musicData[_channelData.eventDataPtr];
			if (_veloTab)
				velocity = _veloTab[velocity];
			_channelData.eventDataPtr++;
			_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (opcode << 8) | (velocity << 16));
		}
		if (_channelData.channelActive)
			_channelData.nextEventTime += getNextEventTime();
	}
	return returnVal;
}

//- command 90h routines

void GmChannel::com90_caseNoteOff(void) {

	_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (_musicData[_channelData.eventDataPtr] << 8));
	_channelData.eventDataPtr++;
}

void GmChannel::com90_stopChannel(void) {

	stopNote();
	_channelData.channelActive = 0;
}

void GmChannel::com90_setupInstrument(void) {
	byte instrument = _musicData[_channelData.eventDataPtr];
	if (_instMap)
		instrument = _instMap[instrument];
	_midiDrv->send((0xC0 | _channelData.midiChannelNumber) | (instrument << 8));
	_channelData.eventDataPtr++;
}

uint8 GmChannel::com90_updateTempo(void) {

	uint8 retV = _musicData[_channelData.eventDataPtr];
	_channelData.eventDataPtr++;
	return retV;
}

void GmChannel::com90_getPitch(void) {

	_midiDrv->send((0xE0 | _channelData.midiChannelNumber) | 0 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void GmChannel::com90_getChannelVolume(void) {

	_lastVolume = _musicData[_channelData.eventDataPtr];
	uint8 newVol = (uint8)((_musicData[_channelData.eventDataPtr++] * _musicVolume) >> 7);
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x700 | (newVol << 16));
}

void GmChannel::com90_rewindMusic(void) {

	_channelData.eventDataPtr = _channelData.startOfData;
}

void GmChannel::com90_keyOff(void) {

	_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (_channelData.note << 8) | 0);
}

void GmChannel::com90_setStartOfData(void) {

	_channelData.startOfData = _channelData.eventDataPtr;
}

void GmChannel::com90_getChannelPanValue(void) {

	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x0A00 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void GmChannel::com90_getChannelControl(void) {

	uint8 conNum = _musicData[_channelData.eventDataPtr];
	uint8 conDat = _musicData[_channelData.eventDataPtr + 1];
	_channelData.eventDataPtr += 2;
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | (conNum << 8) | (conDat << 16));
}

} // End of namespace Sky
