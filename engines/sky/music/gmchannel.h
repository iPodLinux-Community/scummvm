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

#ifndef SKYGMCHANNEL_H
#define SKYGMCHANNEL_H

#include "sky/music/musicbase.h"

class MidiDriver;

namespace Sky {

typedef struct {
	uint16 eventDataPtr;
	int32 nextEventTime;
	uint16 startOfData;
	uint8 midiChannelNumber;
	uint8 note;
	uint8 channelActive;
} MidiChannelType;

class GmChannel : public ChannelBase {
public:
	GmChannel(uint8 *pMusicData, uint16 startOfData, MidiDriver *pMidiDrv, const byte *pInstMap, const byte *veloTab);
	virtual void stopNote(void);
	virtual uint8 process(uint16 aktTime);
	virtual void updateVolume(uint16 pVolume);
	virtual bool isActive(void);
private:
	const byte *_instMap;
	const byte *_veloTab;
	MidiDriver *_midiDrv;
	uint8 *_musicData;
	uint16 _musicVolume;
	MidiChannelType _channelData;
	uint8 _lastVolume;
	//-                          normal subs
	void setRegister(uint8 regNum, uint8 value);
	int32 getNextEventTime(void);
	uint16 getNextNote(uint8 param);
	void adlibSetupInstrument(void);
	void setupInstrument(uint8 opcode);
	void setupChannelVolume(uint8 volume);
	//-                          Streamfunctions from Command90hTable
	void com90_caseNoteOff(void);        // 0
	void com90_stopChannel(void);        // 1
	void com90_setupInstrument(void);    // 2
	uint8 com90_updateTempo(void);       // 3
	//void com90_dummy(void);            // 4
	void com90_getPitch(void);           // 5
	void com90_getChannelVolume(void);   // 6
	//void com90_skipTremoVibro(void);   // 7
	void com90_rewindMusic(void);        // 8
	void com90_keyOff(void);             // 9
	//void com90_error(void);            // 10
	void com90_getChannelPanValue(void); // 11
	void com90_setStartOfData(void);     // 12
	void com90_getChannelControl(void);     // 13
};

} // End of namespace Sky

#endif //SKYGMCHANNEL_H
