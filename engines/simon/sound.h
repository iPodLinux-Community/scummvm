/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SIMON_SOUND_H
#define SIMON_SOUND_H

#include "sound/mixer.h"
#include "simon/intern.h"
#include "common/str.h"

namespace Simon {

class BaseSound;

class SimonEngine;

class Sound {
private:
	SimonEngine *_vm;

	Audio::Mixer *_mixer;

	BaseSound *_voice;
	BaseSound *_effects;

	bool _effectsPaused;
	bool _ambientPaused;
	bool _sfx5Paused;

	uint16 *_filenums;
	uint32 *_offsets;
	uint16 _lastVoiceFile;

	Audio::SoundHandle _voiceHandle;
	Audio::SoundHandle _effectsHandle;
	Audio::SoundHandle _ambientHandle;
	Audio::SoundHandle _sfx5Handle;

	bool _hasEffectsFile;
	bool _hasVoiceFile;
	uint _ambientPlaying;

public:
	Sound(SimonEngine *vm, const GameSpecificSettings *gss, Audio::Mixer *mixer);
	~Sound();

	void loadVoiceFile(const GameSpecificSettings *gss);
	void loadSfxFile(const GameSpecificSettings *gss);

	void readSfxFile(const char *filename);
	void loadSfxTable(Common::File *gameFile, uint32 base);
	void readVoiceFile(const char *filename);

	void playVoice(uint sound);
	void playEffects(uint sound);
	void playAmbient(uint sound);

	// Feeble Files specific
	void playAmbientData(byte *soundData, uint sound, uint pan, uint vol);
	void playSfxData(byte *soundData, uint sound, uint pan, uint vol);
	void playSfx5Data(byte *soundData, uint sound, uint pan, uint vol);
	void playSoundData(Audio::SoundHandle *handle, byte *soundData, uint sound, int pan = 0, int vol = 0, bool loop = false);
	void playVoiceData(byte *soundData, uint sound);
	void switchVoiceFile(const GameSpecificSettings *gss, uint disc);

	bool hasVoice() const;
	bool isVoiceActive() const;
	void stopSfx5();
	void stopVoice();
	void stopAll();
	void effectsPause(bool b);
	void ambientPause(bool b);
};

} // End of namespace Simon

#endif
