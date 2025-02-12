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

#ifndef BSSOUND_H
#define BSSOUND_H

#include "sword1/object.h"
#include "sword1/sworddefs.h"
#include "common/file.h"
#include "sound/mixer.h"
#include "common/util.h"

namespace Audio {
	class Mixer;
}

namespace Sword1 {

#define	TOTAL_FX_PER_ROOM	7		// total loop & random fx per room (see fx_list.c)
#define	MAX_ROOMS_PER_FX	7		// max no. of rooms in the fx's room,vol list
#define	MAX_FXQ_LENGTH		32		// max length of sound queue - ie. max number of fx that can be stored up/playing together

#define FX_SPOT 1
#define FX_LOOP 2
#define FX_RANDOM 3

struct QueueElement {
	uint32 id, delay;
	Audio::SoundHandle handle;
};

struct RoomVol {
	int32 roomNo, leftVol, rightVol;
};

struct FxDef {
	uint32 sampleId, type, delay;
	RoomVol roomVolList[MAX_ROOMS_PER_FX];
};

class ResMan;
#define WAVE_VOL_TAB_LENGTH 480
#define WAVE_VOL_THRESHOLD 190000 //120000

enum CowMode {
	CowWave = 0,
	CowMp3,
	CowVorbis,
	CowDemo
};

class Sound {
public:
	Sound(const char *searchPath, Audio::Mixer *mixer, ResMan *pResMan);
	~Sound(void);
	void setSpeechVol(uint8 volL, uint8 volR) { _speechVolL = volL; _speechVolR = volR; };
	void setSfxVol(uint8 volL, uint8 volR) { _sfxVolL = volL; _sfxVolR = volR; };
	void giveSpeechVol(uint8 *volL, uint8 *volR) { *volL = _speechVolL; *volR = _speechVolR; };
	void giveSfxVol(uint8 *volL, uint8 *volR) { *volL = _sfxVolL; *volR = _sfxVolR; };
	void newScreen(uint32 screen);
	void quitScreen(void);
	void closeCowSystem(void);

	bool startSpeech(uint16 roomNo, uint16 localNo);
	bool speechFinished(void);
	void stopSpeech();
	bool amISpeaking(void);

	void fnStopFx(int32 fxNo);
	int addToQueue(int32 fxNo);

	void engine(void);

private:
	uint8 _sfxVolL, _sfxVolR, _speechVolL, _speechVolR;
	void playSample(QueueElement *elem);
	void initCowSystem(void);

	int16 *uncompressSpeech(uint32 index, uint32 cSize, uint32 *size);
	void calcWaveVolume(int16 *data, uint32 length);
	bool _waveVolume[WAVE_VOL_TAB_LENGTH];
	uint16 _waveVolPos;
	Common::File _cowFile;
	uint32		 *_cowHeader;
	uint32		 _cowHeaderSize;
	uint8		 _currentCowFile;
	CowMode		 _cowMode;
	Audio::SoundHandle _speechHandle, _fxHandle;
	Common::RandomSource _rnd;

	QueueElement _fxQueue[MAX_FXQ_LENGTH];
	uint8		 _endOfQueue;
	Audio::Mixer *_mixer;
	ResMan *_resMan;
	char _filePath[100];
	static const char _musicList[270];
	static const uint16 _roomsFixedFx[TOTAL_ROOMS][TOTAL_FX_PER_ROOM];
#ifdef PALMOS_68K
public:
	static const FxDef *_fxList;
#else
	static const FxDef _fxList[312];
#endif
};

} // End of namespace Sword1

#endif //BSSOUND_H
