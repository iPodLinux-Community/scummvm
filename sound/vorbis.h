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

#ifndef SOUND_VORBIS_H
#define SOUND_VORBIS_H

#include "common/stdafx.h"
#include "common/scummsys.h"

#ifdef USE_VORBIS

namespace Common {
	class File;
}

namespace Audio {

class AudioStream;
class DigitalTrackInfo;

DigitalTrackInfo *getVorbisTrack(int track);

/**
 * Create a new AudioStream from the Vorbis data in the given
 * file. If you only want to play part of that file, then seek
 * to the start position in file before passing it to this
 * factory function, and specify the appropriate size.
 */
AudioStream *makeVorbisStream(Common::File *file, uint32 size);

} // End of namespace Audio

#endif // #ifdef USE_VORBIS
#endif // #ifndef SOUND_VORBIS_H
