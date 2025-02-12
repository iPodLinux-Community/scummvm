/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#ifndef GRAPHICS_PALETTEMAN_H
#define GRAPHICS_PALETTEMAN_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/stack.h"
#include "common/singleton.h"

namespace Graphics {

class PaletteManager : public Common::Singleton<PaletteManager> {
public:
	/**
	 * Enable/Disable the current cursor palette.
	 *
	 * @param disable
	 */
	void disableCursorPalette(bool disable);

	/**
	 * Push a new cursor palette onto the stack, and set it in the backend.
	 * The palette entries from 'start' till (start+num-1) will be replaced
	 * so a full palette updated is accomplished via start=0, num=256.
	 *
	 * The palette data is specified in the same interleaved RGBA format as
	 * used by all backends.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note If num is zero, the cursor palette is disabled.
	 */
	void pushCursorPalette(const byte *colors, uint start, uint num);

	/**
	 * Pop a cursor palette from the stack, and restore the previous one to
	 * the backend. If there is no previous palette, the cursor palette is
	 * disabled instead.
	 */
	void popCursorPalette();

	/**
	 * Replace the current cursor palette on the stack. If the stack is
	 * empty, the palette is pushed instead. It's a slightly more optimized
	 * way of popping the old palette before pushing the new one.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note If num is zero, the cursor palette is disabled.
	 */
	void replaceCursorPalette(const byte *colors, uint start, uint num);

private:
	friend class Common::Singleton<SingletonBaseType>;
	PaletteManager();

	struct Palette {
		byte *_data;
		uint _start;
		uint _num;
		uint _size;

		bool _disabled;

		Palette(const byte *colors, uint start, uint num) {
			_start = start;
			_num = num;
			_size = 4 * num;

			if (num) {
				_data = new byte[_size];
				memcpy(_data, colors, _size);
			} else {
				_data = NULL;
			}

			_disabled = false;
		}

		~Palette() {
			delete [] _data;
		}
	};

	Common::Stack<Palette *> _cursorPaletteStack;
};


} // End of namespace Graphics

/** Shortcut for accessing the palette manager. */
#define PaletteMan	(Graphics::PaletteManager::instance())

#endif
