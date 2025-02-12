/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
#ifndef GOB_SOUND_H
#define GOB_SOUND_H

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Gob {

class Snd {
public:
	struct SoundDesc {
		Audio::SoundHandle handle;
		char *data;
		int32 size;
		int16 repCount;
		int16 timerTicks;
		int16 inClocks;
		int16 frequency;
		int16 flag;
		SoundDesc() : data(0), size(0), repCount(0), timerTicks(0),
					  inClocks(0), frequency(0), flag(0) {}
	};

	typedef void (*CleanupFuncPtr) (int16);

	SoundDesc *_loopingSounds[10]; // Should be enough
	char _playingSound;
	CleanupFuncPtr _cleanupFunc;

	Snd(GobEngine *vm);
	void speakerOn(int16 frequency, int32 length);
	void speakerOff(void);
	SoundDesc *loadSoundData(const char *path);
	void stopSound(int16 arg){return;}
	void loopSounds(void);
	void playSample(SoundDesc *sndDesc, int16 repCount, int16 frequency);
	void playComposition(Snd::SoundDesc ** samples, int16 *composit, int16 freqVal) {;}
	void waitEndPlay(void) {;}

	// This deletes sndDesc and stops playing the sample.
	// If freedata is set, it also delete[]s the sample data.
	void freeSoundDesc(SoundDesc *sndDesc, bool freedata=true);

protected:
	// TODO: This is a very primitive square wave generator. The only thing is
	//       has in common with the PC speaker is that it sounds terrible.
	class SquareWaveStream : public Audio::AudioStream {
	private:
		uint _rate;
		bool _beepForever;
		uint32 _periodLength;
		uint32 _periodSamples;
		uint32 _remainingSamples;
		int16 _sampleValue;

	public:
		SquareWaveStream() {}
		~SquareWaveStream() {}

		void playNote(int freq, int32 ms, uint rate);

		int readBuffer(int16 *buffer, const int numSamples);

		bool endOfData() const	{ return _remainingSamples == 0; }
		bool isStereo() const	{ return false; }
		int getRate() const	{ return _rate; }
	};

	SquareWaveStream _speakerStream;
	Audio::SoundHandle _speakerHandle;

	GobEngine *_vm;

	void cleanupFuncCallback() {;}
	int16 checkProAudio(void) {return 0;}
	int16 checkAdlib(void) {return 0;}
	int16 checkBlaster(void) {return 0;}

	void writeAdlib(int16 port, int16 data);
	void setBlasterPort(int16 port);
	void setResetTimerFlag(char flag){return;}
};

}				// End of namespace Gob

#endif
