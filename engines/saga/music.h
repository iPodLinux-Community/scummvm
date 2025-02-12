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

// Music class

#ifndef SAGA_MUSIC_H_
#define SAGA_MUSIC_H_

#include "sound/audiocd.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"

namespace Saga {

enum MusicFlags {
	MUSIC_NORMAL = 0,
	MUSIC_LOOP = 0x0001,
	MUSIC_DEFAULT = 0xffff
};

class MusicPlayer : public MidiDriver {
public:
	MusicPlayer(MidiDriver *driver);
	~MusicPlayer();

	bool isPlaying() { return _isPlaying; }

	void setVolume(int volume);
	int getVolume() { return _masterVolume; }

	void setNativeMT32(bool b) { _nativeMT32 = b; }
	bool hasNativeMT32() { return _nativeMT32; }
	void playMusic();
	void stopMusic();
	void setLoop(bool loop) { _looping = loop; }
	void setPassThrough(bool b) { _passThrough = b; }

	void setGM(bool isGM) { _isGM = isGM; }

	//MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo(void)	{ return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel()		{ return 0; }
	MidiChannel *getPercussionChannel()	{ return 0; }

	MidiParser *_parser;

protected:

	static void onTimer(void *data);

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	byte _channelVolume[16];
	bool _nativeMT32;
	bool _isGM;
	bool _passThrough;

	bool _isPlaying;
	bool _looping;
	bool _randomLoop;
	byte _masterVolume;

	byte *_musicData;
	uint16 *_buf;
	size_t _musicDataSize;
};

class Music {
public:

	Music(SagaEngine *vm, Audio::Mixer *mixer, MidiDriver *driver, int enabled);
	~Music(void);
	void setNativeMT32(bool b)	{ _player->setNativeMT32(b); }
	bool hasNativeMT32()		{ return _player->hasNativeMT32(); }
	void setAdlib(bool b)		{ _adlib = b; }
	bool hasAdlib()			{ return _adlib; }
	void setPassThrough(bool b)	{ _player->setPassThrough(b); }
	bool isPlaying(void);

	void play(uint32 resourceId, MusicFlags flags = MUSIC_DEFAULT);
	void pause(void);
	void resume(void);
	void stop(void);

	void setVolume(int volume, int time = 1);
	int getVolume() { return _currentVolume; }

	int32 *_songTable;
	int _songTableLen;

private:
	SagaEngine *_vm;
	Audio::Mixer *_mixer;

	MusicPlayer *_player;
	Audio::SoundHandle _musicHandle;
	uint32 _trackNumber;

	int _enabled;
	bool _adlib;

	int _targetVolume;
	int _currentVolume;
	int _currentVolumePercent;

	ResourceContext *_musicContext;
	MidiParser *xmidiParser;
	MidiParser *smfParser;

	Audio::DigitalTrackInfo *_track;

	byte *_midiMusicData;

	static void musicVolumeGaugeCallback(void *refCon);
	void musicVolumeGauge(void);
};

} // End of namespace Saga

#endif
