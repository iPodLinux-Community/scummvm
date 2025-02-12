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

#ifndef SCUMM_SOUND_H
#define SCUMM_SOUND_H

#include "common/scummsys.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "scumm/saveload.h"

namespace Common {
	class File;
}

namespace Scumm {

class ScummEngine;
class BaseScummFile;

struct MP3OffsetTable;
struct SaveLoadEntry;

enum {
	kTalkSoundID = 10000
};

// TODO: Consider splitting Sound into even more subclasses.
// E.g. for v1-v4, v5, v6+, ...
class Sound : public Serializable {
public:
	enum SoundMode {
		kVOCMode,
		kMP3Mode,
		kVorbisMode,
		kFlacMode
	};

protected:
	ScummEngine *_vm;

	int16 _soundQuePos, _soundQue[0x100];
	int16 _soundQue2Pos;

	struct {
		int16 sound;
		int32 offset;
		int16 channel;
		int16 flags;
	} _soundQue2[10];

	BaseScummFile *_sfxFile;
	SoundMode _soundMode;
	MP3OffsetTable *_offsetTable;	// For compressed audio
	int _numSoundEffects;		// For compressed audio

	uint32 _talk_sound_a1, _talk_sound_a2, _talk_sound_b1, _talk_sound_b2;
	byte _talk_sound_mode, _talk_sound_channel;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[64];
	uint _curSoundPos;

	int _overrideFreq;

	int16 _currentCDSound;
	int16 _currentMusic;

public:
	Audio::SoundHandle _talkChannelHandle;	// Handle of mixer channel actor is talking on

	bool _soundsPaused;
	byte _sfxMode;

public:
	Sound(ScummEngine *parent);
	virtual ~Sound();
	virtual void addSoundToQueue(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0);
	virtual void addSoundToQueue2(int sound, int heOffset = 0, int heChannel = 0, int heFlags = 0);
	void processSound();

	void playSound(int soundID);
	void startTalkSound(uint32 offset, uint32 b, int mode, Audio::SoundHandle *handle = NULL);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	virtual int isSoundRunning(int sound) const;
	bool isSoundInUse(int sound) const;
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	void soundKludge(int *list, int num);
	void talkSound(uint32 a, uint32 b, int mode, int channel = 0);
	virtual void setupSound();
	void pauseSounds(bool pause);

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int numLoops, int startFrame, int duration);
	void stopCD();
	int pollCD() const;
	void updateCD();
	int getCurrentCDSound() const { return _currentCDSound; }

	// Used by the save/load system:
	void saveLoadWithSerializer(Serializer *ser);

protected:
	BaseScummFile *openSfxFile();
	bool isSfxFinished() const;
	void processSfxQueues();

	bool isSoundInQueue(int sound) const;

	virtual void processSoundQueues();
};


} // End of namespace Scumm

#endif
