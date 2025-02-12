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

#include "common/stdafx.h"
#include "common/system.h"
#include "common/util.h"

#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/util.h"

namespace Scumm {

void ScummEngine::resetPalette() {
	if (_game.version <= 1) {
		if (_game.platform == Common::kPlatformC64) {
			setC64Palette();
		} else if (_game.platform == Common::kPlatformNES) {
			setNESPalette();
		} else {
			setV1Palette();
		}
	} else if (_game.features & GF_16COLOR) {
		switch (_renderMode) {
		case Common::kRenderEGA:
			setEGAPalette();
			break;

		case Common::kRenderAmiga:
			setAmigaPalette();
			break;

		case Common::kRenderCGA:
			setCGAPalette();
			break;

		case Common::kRenderHercA:
		case Common::kRenderHercG:
			setHercPalette();
			break;

		default:
			if ((_game.platform == Common::kPlatformAmiga) || (_game.platform == Common::kPlatformAtariST))
				setAmigaPalette();
			else
				setEGAPalette();
		}
	} else
		setDirtyColors(0, 255);
}

void ScummEngine::setC64Palette() {
	setPalColor( 0, 0x00, 0x00, 0x00);
	setPalColor( 1, 0xFD, 0xFE, 0xFC);
	setPalColor( 2, 0xBE, 0x1A, 0x24);
	setPalColor( 3, 0x30, 0xE6, 0xC6);
	setPalColor( 4, 0xB4, 0x1A, 0xE2);
	setPalColor( 5, 0x1F, 0xD2, 0x1E);
	setPalColor( 6, 0x21, 0x1B, 0xAE);
	setPalColor( 7, 0xDF, 0xF6, 0x0A);
	setPalColor( 8, 0xB8, 0x41, 0x04);
	setPalColor( 9, 0x6A, 0x33, 0x04);
	setPalColor(10, 0xFE, 0x4A, 0x57);
	setPalColor(11, 0x42, 0x45, 0x40);
	setPalColor(12, 0x70, 0x74, 0x6F);
	setPalColor(13, 0x59, 0xFE, 0x59);
	setPalColor(14, 0x5F, 0x53, 0xFE);
	setPalColor(15, 0xA4, 0xA7, 0xA2);

	// Use 17 color table for v1 games to allow correct color for inventory and
	// sentence line Original games used some kind of dynamic color table
	// remapping between rooms.
	setPalColor(16, 255,  85, 255);
}

void ScummEngine::setNESPalette() {
	setPalColor(0x00,0x24,0x24,0x24); // 0x1D
	setPalColor(0x01,0x00,0x24,0x92);
	setPalColor(0x02,0x00,0x00,0xDB);
	setPalColor(0x03,0x6D,0x49,0xDB);
	setPalColor(0x04,0x92,0x00,0x6D);
	setPalColor(0x05,0xB6,0x00,0x6D);
	setPalColor(0x06,0xB6,0x24,0x00);
	setPalColor(0x07,0x92,0x49,0x00);
	setPalColor(0x08,0x6D,0x49,0x00);
	setPalColor(0x09,0x24,0x49,0x00);
	setPalColor(0x0A,0x00,0x6D,0x24);
	setPalColor(0x0B,0x00,0x92,0x00);
	setPalColor(0x0C,0x00,0x49,0x49);
	setPalColor(0x0D,0x00,0x00,0x00);
	setPalColor(0x0E,0x00,0x00,0x00);
	setPalColor(0x0F,0x00,0x00,0x00);

	setPalColor(0x10,0xB6,0xB6,0xB6);
	setPalColor(0x11,0x00,0x6D,0xDB);
	setPalColor(0x12,0x00,0x49,0xFF);
	setPalColor(0x13,0x92,0x00,0xFF);
	setPalColor(0x14,0xB6,0x00,0xFF);
	setPalColor(0x15,0xFF,0x00,0x92);
	setPalColor(0x16,0xFF,0x00,0x00);
	setPalColor(0x17,0xDB,0x6D,0x00);
	setPalColor(0x18,0x92,0x6D,0x00);
	setPalColor(0x19,0x24,0x92,0x00);
	setPalColor(0x1A,0x00,0x92,0x00);
	setPalColor(0x1B,0x00,0xB6,0x6D);
	setPalColor(0x1C,0x00,0x92,0x92);
	setPalColor(0x1D,0x6D,0x6D,0x6D); // 0x00
	setPalColor(0x1E,0x00,0x00,0x00);
	setPalColor(0x1F,0x00,0x00,0x00);

	setPalColor(0x20,0xFF,0xFF,0xFF);
	setPalColor(0x21,0x6D,0xB6,0xFF);
	setPalColor(0x22,0x92,0x92,0xFF);
	setPalColor(0x23,0xDB,0x6D,0xFF);
	setPalColor(0x24,0xFF,0x00,0xFF);
	setPalColor(0x25,0xFF,0x6D,0xFF);
	setPalColor(0x26,0xFF,0x92,0x00);
	setPalColor(0x27,0xFF,0xB6,0x00);
	setPalColor(0x28,0xDB,0xDB,0x00);
	setPalColor(0x29,0x6D,0xDB,0x00);
	setPalColor(0x2A,0x00,0xFF,0x00);
	setPalColor(0x2B,0x49,0xFF,0xDB);
	setPalColor(0x2C,0x00,0xFF,0xFF);
	setPalColor(0x2D,0x49,0x49,0x49);
	setPalColor(0x2E,0x00,0x00,0x00);
	setPalColor(0x2F,0x00,0x00,0x00);

	setPalColor(0x30,0xFF,0xFF,0xFF);
	setPalColor(0x31,0xB6,0xDB,0xFF);
	setPalColor(0x32,0xDB,0xB6,0xFF);
	setPalColor(0x33,0xFF,0xB6,0xFF);
	setPalColor(0x34,0xFF,0x92,0xFF);
	setPalColor(0x35,0xFF,0xB6,0xB6);
	setPalColor(0x36,0xFF,0xDB,0x92);
	setPalColor(0x37,0xFF,0xFF,0x49);
	setPalColor(0x38,0xFF,0xFF,0x6D);
	setPalColor(0x39,0xB6,0xFF,0x49);
	setPalColor(0x3A,0x92,0xFF,0x6D);
	setPalColor(0x3B,0x49,0xFF,0xDB);
	setPalColor(0x3C,0x92,0xDB,0xFF);
	setPalColor(0x3D,0x92,0x92,0x92);
	setPalColor(0x3E,0x00,0x00,0x00);
	setPalColor(0x3F,0x00,0x00,0x00);
}

void ScummEngine::setAmigaPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0,   0, 187);
	setPalColor( 2,   0, 187,   0);
	setPalColor( 3,   0, 187, 187);
	setPalColor( 4, 187,   0,   0);
	setPalColor( 5, 187,   0, 187);
	setPalColor( 6, 187, 119,   0);
	setPalColor( 7, 187, 187, 187);
	setPalColor( 8, 119, 119, 119);
	setPalColor( 9, 119, 119, 255);
	setPalColor(10,   0, 255,   0);
	setPalColor(11,   0, 255, 255);
	setPalColor(12, 255, 136, 136);
	setPalColor(13, 255,   0, 255);
	setPalColor(14, 255, 255,   0);
	setPalColor(15, 255, 255, 255);
}

void ScummEngine::setHercPalette() {
	setPalColor( 0,   0,   0,   0);

	if (_renderMode == Common::kRenderHercA)
		setPalColor( 1, 0xAE, 0x69, 0x38);
	else
		setPalColor( 1, 0x00, 0xFF, 0x00);

	// Setup cursor palette
	setPalColor( 7, 170, 170, 170);
	setPalColor( 8,  85,  85,  85);
	setPalColor(15, 255, 255, 255);
}

void ScummEngine::setCGAPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0, 168, 168);
	setPalColor( 2, 168,   0, 168);
	setPalColor( 3, 168, 168, 168);

	// Setup cursor palette
	setPalColor( 7, 170, 170, 170);
	setPalColor( 8,  85,  85,  85);
	setPalColor(15, 255, 255, 255);
}

void ScummEngine::setEGAPalette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1,   0,   0, 170);
	setPalColor( 2,   0, 170,   0);
	setPalColor( 3,   0, 170, 170);
	setPalColor( 4, 170,   0,   0);
	setPalColor( 5, 170,   0, 170);
	setPalColor( 6, 170,  85,   0);
	setPalColor( 7, 170, 170, 170);
	setPalColor( 8,  85,  85,  85);
	setPalColor( 9,  85,  85, 255);
	setPalColor(10,  85, 255,  85);
	setPalColor(11,  85, 255, 255);
	setPalColor(12, 255,  85,  85);
	setPalColor(13, 255,  85, 255);
	setPalColor(14, 255, 255,  85);
	setPalColor(15, 255, 255, 255);
}

void ScummEngine::setV1Palette() {
	setPalColor( 0,   0,   0,   0);
	setPalColor( 1, 255, 255, 255);
	setPalColor( 2, 170,   0,   0);
	setPalColor( 3,   0, 170, 170);
	setPalColor( 4, 170,   0, 170);
	setPalColor( 5,   0, 170,   0);
	setPalColor( 6,   0,   0, 170);
	setPalColor( 7, 255, 255,  85);
	setPalColor( 8, 255,  85,  85);
	setPalColor( 9, 170,  85,   0);
	setPalColor(10, 255,  85,  85);
	setPalColor(11,  85,  85,  85);
	setPalColor(12, 170, 170, 170);
	setPalColor(13,  85, 255,  85);
	setPalColor(14,  85,  85, 255);

	if (_game.id == GID_ZAK)
		setPalColor(15, 170, 170, 170);
	else
		setPalColor(15,  85,  85,  85);

	setPalColor(16, 255,  85, 255);
}

void ScummEngine::setPaletteFromPtr(const byte *ptr, int numcolor) {
	int i;
	byte *dest, r, g, b;

	if (numcolor < 0) {
		if (_game.features & GF_SMALL_HEADER) {
			if (_game.features & GF_OLD256)
				numcolor = READ_LE_UINT16(ptr);
			else
				numcolor = READ_LE_UINT16(ptr) / 3;
			ptr += 2;
		} else {
			numcolor = getResourceDataSize(ptr) / 3;
		}
	}

	checkRange(256, 0, numcolor, "Too many colors (%d) in Palette");

	dest = _currentPalette;

	for (i = 0; i < numcolor; i++) {
		r = *ptr++;
		g = *ptr++;
		b = *ptr++;

		// Only SCUMM 5/6 games use 6/6/6 style palettes
		if (_game.version >= 5 && _game.version <= 6) {
			if ((_game.heversion <= 73 && i < 15) || i == 15 || r < 252 || g < 252 || b < 252) {
				*dest++ = r;
				*dest++ = g;
				*dest++ = b;
			} else {
				dest += 3;
			}
		} else {
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		}
	}

	if (_game.heversion >= 90 || _game.version == 8) {
		memcpy(_darkenPalette, _currentPalette, 768);
	}

	setDirtyColors(0, numcolor - 1);
}

void ScummEngine::setDirtyColors(int min, int max) {
	if (_palDirtyMin > min)
		_palDirtyMin = min;
	if (_palDirtyMax < max)
		_palDirtyMax = max;
}

void ScummEngine::initCycl(const byte *ptr) {
	int j;
	ColorCycle *cycl;

	memset(_colorCycle, 0, sizeof(_colorCycle));

	if (_game.features & GF_SMALL_HEADER) {
		cycl = _colorCycle;
		for (j = 0; j < 16; ++j, ++cycl) {
			uint16 delay = READ_BE_UINT16(ptr);
			ptr += 2;
			byte start = *ptr++;
			byte end = *ptr++;

			if (!delay || delay == 0x0aaa || start >= end)
				continue;

			cycl->counter = 0;
			cycl->delay = 16384 / delay;
			cycl->flags = 2;
			cycl->start = start;
			cycl->end = end;
		}
	} else {
		memset(_colorUsedByCycle, 0, sizeof(_colorUsedByCycle));
		while ((j = *ptr++) != 0) {
			if (j < 1 || j > 16) {
				error("Invalid color cycle index %d", j);
			}
			cycl = &_colorCycle[j - 1];

			ptr += 2;
			cycl->counter = 0;
			cycl->delay = 16384 / READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->flags = READ_BE_UINT16(ptr);
			ptr += 2;
			cycl->start = *ptr++;
			cycl->end = *ptr++;

			for (int i = cycl->start; i <= cycl->end; ++i) {
				_colorUsedByCycle[i] = 1;
			}
		}
	}
}

void ScummEngine::stopCycle(int i) {
	ColorCycle *cycl;

	checkRange(16, 0, i, "Stop Cycle %d Out Of Range");
	if (i != 0) {
		_colorCycle[i - 1].delay = 0;
		return;
	}

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++)
		cycl->delay = 0;
}

/**
 * Cycle the colors in the given palette in the intervael [cycleStart, cycleEnd]
 * either one step forward or backward.
 */
static void doCyclePalette(byte *palette, int cycleStart, int cycleEnd, int size, bool forward) {
	byte *start = palette + cycleStart * size;
	byte *end = palette + cycleEnd * size;
	int num = cycleEnd - cycleStart;
	byte tmp[6];

	assert(size <= 6);

	if (forward) {
		memmove(tmp, end, size);
		memmove(start + size, start, num * size);
		memmove(start, tmp, size);
	} else {
		memmove(tmp, start, size);
		memmove(start, start + size, num * size);
		memmove(end, tmp, size);
	}
}

/**
 * Adjust an 'indirect' color palette for the color cycling performed on its
 * master palette. An indirect palette is a palette which contains indices
 * pointing into another palette - it provides a level of indirection to map
 * palette colors to other colors. Now when the target palette is cycled, the
 * indirect palette suddenly point at the wrong color(s). This function takes
 * care of adjusting an indirect palette by searching through it and replacing
 * all indices that are in the cycle range by the new (cycled) index.
 *
 * Finally, the palette entries still have to be cycled normally.
 */
static void doCycleIndirectPalette(byte *palette, int cycleStart, int cycleEnd, bool forward) {
	int num = cycleEnd - cycleStart + 1;
	int i;
	int offset = forward ? 1 : num - 1;

	for (i = 0; i < 256; i++) {
		if (cycleStart <= palette[i] && palette[i] <= cycleEnd) {
			palette[i] = (palette[i] - cycleStart + offset) % num + cycleStart;
		}
	}

	doCyclePalette(palette, cycleStart, cycleEnd, 1, forward);
}


void ScummEngine::cyclePalette() {
	ColorCycle *cycl;
	int valueToAdd;
	int i, j;

	valueToAdd = VAR(VAR_TIMER);
	if (valueToAdd < VAR(VAR_TIMER_NEXT))
		valueToAdd = VAR(VAR_TIMER_NEXT);

	for (i = 0, cycl = _colorCycle; i < 16; i++, cycl++) {
		if (!cycl->delay || cycl->start > cycl->end)
			continue;
		cycl->counter += valueToAdd;
		if (cycl->counter >= cycl->delay) {
			cycl->counter %= cycl->delay;

			setDirtyColors(cycl->start, cycl->end);
			moveMemInPalRes(cycl->start, cycl->end, cycl->flags & 2);

			doCyclePalette(_currentPalette, cycl->start, cycl->end, 3, !(cycl->flags & 2));

			if (_shadowPalette) {
				if (_game.version >= 7) {
					for (j = 0; j < NUM_SHADOW_PALETTE; j++)
						doCycleIndirectPalette(_shadowPalette + j * 256, cycl->start, cycl->end, !(cycl->flags & 2));
				} else {
					doCycleIndirectPalette(_shadowPalette, cycl->start, cycl->end, !(cycl->flags & 2));
				}
			}
		}
	}
}

/**
 * Perform color cycling on the palManipulate data, too, otherwise
 * color cycling will be disturbed by the palette fade.
 */
void ScummEngine::moveMemInPalRes(int start, int end, byte direction) {
	if (!_palManipCounter)
		return;

	doCyclePalette(_palManipPalette, start, end, 3, !direction);
	doCyclePalette(_palManipIntermediatePal, start, end, 6, !direction);
}

void ScummEngine::palManipulateInit(int resID, int start, int end, int time) {
	byte *pal, *target, *between;
	byte *string1, *string2, *string3;
	int i;

	string1 = getStringAddress(resID);
	string2 = getStringAddress(resID + 1);
	string3 = getStringAddress(resID + 2);
	if (!string1 || !string2 || !string3) {
		error("palManipulateInit(%d,%d,%d,%d): Cannot obtain string resources %d, %d and %d",
				resID, start, end, time, resID, resID + 1, resID + 2);
		return;
	}

	string1 += start;
	string2 += start;
	string3 += start;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;

	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	pal = _currentPalette + start * 3;
	target = _palManipPalette + start * 3;
	between = _palManipIntermediatePal + start * 6;

	for (i = start; i < end; ++i) {
		*target++ = *string1++;
		*target++ = *string2++;
		*target++ = *string3++;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
	}

	_palManipCounter = time;
}

void ScummEngine_v6::palManipulateInit(int resID, int start, int end, int time) {
	byte *pal, *target, *between;
	const byte *new_pal;
	int i;

	new_pal = getPalettePtr(resID, _roomResource);

	new_pal += start*3;

	_palManipStart = start;
	_palManipEnd = end;
	_palManipCounter = 0;

	if (!_palManipPalette)
		_palManipPalette = (byte *)calloc(0x300, 1);
	if (!_palManipIntermediatePal)
		_palManipIntermediatePal = (byte *)calloc(0x600, 1);

	pal = _currentPalette + start * 3;
	target = _palManipPalette + start * 3;
	between = _palManipIntermediatePal + start * 6;

	for (i = start; i < end; ++i) {
		*target++ = *new_pal++;
		*target++ = *new_pal++;
		*target++ = *new_pal++;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
		*(uint16 *)between = ((uint16) *pal++) << 8;
		between += 2;
	}

	_palManipCounter = time;
}


void ScummEngine::palManipulate() {
	byte *target, *pal, *between;
	int i, j;

	if (!_palManipCounter || !_palManipPalette || !_palManipIntermediatePal)
		return;

	target = _palManipPalette + _palManipStart * 3;
	pal = _currentPalette + _palManipStart * 3;
	between = _palManipIntermediatePal + _palManipStart * 6;

	for (i = _palManipStart; i < _palManipEnd; ++i) {
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
		j = (*((uint16 *)between) += ((*target++ << 8) - *((uint16 *)between)) / _palManipCounter);
		*pal++ = j >> 8;
		between += 2;
	}
	setDirtyColors(_palManipStart, _palManipEnd);
	_palManipCounter--;
}

void ScummEngine::setShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	byte *table;
	int i;
	byte *curpal;

	if (slot < 0 || slot >= NUM_SHADOW_PALETTE)
		error("setShadowPalette: invalid slot %d", slot);

	if (startColor < 0 || startColor > 255 || endColor < 0 || startColor > 255 || endColor < startColor)
		error("setShadowPalette: invalid range from %d to %d", startColor, endColor);

	table = _shadowPalette + slot * 256;
	for (i = 0; i < 256; i++)
		table[i] = i;

	table += startColor;
	curpal = _currentPalette + startColor * 3;
	for (i = startColor; i <= endColor; i++) {
		*table++ = remapPaletteColor((curpal[0] * redScale) >> 8,
									 (curpal[1] * greenScale) >> 8,
									 (curpal[2] * blueScale) >> 8,
									 -1);
		curpal += 3;
	}
}

static inline uint colorWeight(int red, int green, int blue) {
	return 3 * red * red + 6 * green * green + 2 * blue * blue;
}

void ScummEngine::setShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor, int start, int end) {
	const byte *basepal = getPalettePtr(_curPalIndex, _roomResource);
	const byte *compareptr;
	const byte *pal = basepal + start * 3;
	byte *table = _shadowPalette + start;
	int i;

	// This is an implementation based on the original games code.
	//
	// The four known rooms where setShadowPalette is used in atlantis are:
	//
	// 1) FOA Room 53: subway departing Knossos for Atlantis.
	// 2) FOA Room 48: subway crashing into the Atlantis entrance area
	// 3) FOA Room 82: boat/sub shadows while diving near Thera
	// 4) FOA Room 23: the big machine room inside Atlantis
	//
	// There seems to be no explanation for why this function is called
	// from within Room 23 (the big machine), as it has no shadow effects
	// and thus doesn't result in any visual differences.

	if (_game.id == GID_SAMNMAX) {
		for (i = 0; i < 256; i++)
			_shadowPalette[i] = i;
	}

	for (i = start; i < end; i++) {
		int r = (int) ((pal[0] >> 2) * redScale) >> 8;
		int g = (int) ((pal[1] >> 2) * greenScale) >> 8;
		int b = (int) ((pal[2] >> 2) * blueScale) >> 8;
		pal += 3;

		uint8 bestitem = 0;
		uint bestsum = 32000;

		compareptr = basepal + startColor * 3;
		for (int j = startColor; j <= endColor; j++, compareptr += 3) {
			int ar = compareptr[0] >> 2;
			int ag = compareptr[1] >> 2;
			int ab = compareptr[2] >> 2;

			uint sum = ABS(ar - r) + ABS(ag - g) + ABS(ab - b);

			if (sum < bestsum) {
				bestsum = sum;
				bestitem = j;
			}
		}
		*table++ = bestitem;
	}
}

void ScummEngine::darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	int max;
	if (_game.version >= 5 && _game.version <= 6 && _game.heversion <= 60) {
		max = 252;
	} else {
		max = 255;
	}

	if (startColor <= endColor) {
		const byte *cptr;
		const byte *palptr;
		int color, idx, j;

		if (_game.heversion >= 90 || _game.version == 8) {
			palptr = _darkenPalette;
		} else {
			palptr = getPalettePtr(_curPalIndex, _roomResource);
		}
		for (j = startColor; j <= endColor; j++) {
			idx = (_game.heversion == 70) ? _HEV7ActorPalette[j] : j;
			cptr = palptr + idx * 3;

			if (_game.heversion == 70)
				setDirtyColors(idx, idx);

			color = *cptr++;
			color = color * redScale / 0xFF;
			if (color > max)
				color = max;
			_currentPalette[idx * 3 + 0] = color;

			color = *cptr++;
			color = color * greenScale / 0xFF;
			if (color > max)
				color = max;
			_currentPalette[idx * 3 + 1] = color;

			color = *cptr++;
			color = color * blueScale / 0xFF;
			if (color > max)
				color = max;
			_currentPalette[idx * 3 + 2] = color;
		}
		if (_game.heversion != 70)
			setDirtyColors(startColor, endColor);
	}
}

#ifndef DISABLE_SCUMM_7_8
static int HSL2RGBHelper(int n1, int n2, int hue) {
	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		return n1 + (n2 - n1) * hue / 60;
	if (hue < 180)
		return n2;
	if (hue < 240)
		return n1 + (n2 - n1) * (240 - hue) / 60;
	return n1;
}

/**
 * This function scales the HSL (Hue, Saturation and Lightness)
 * components of the palette colors. It's used in CMI when Guybrush
 * walks from the beach towards the swamp.
 */
void ScummEngine_v8::desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor) {

	if (startColor <= endColor) {
		const byte *cptr;
		byte *cur;
		int j;

		cptr = _darkenPalette + startColor * 3;
		cur = _currentPalette + startColor * 3;

		for (j = startColor; j <= endColor; j++) {
			int R = *cptr++;
			int G = *cptr++;
			int B = *cptr++;

			// RGB to HLS (Foley and VanDam)

			const int min = MIN(R, MIN(G, B));
			const int max = MAX(R, MAX(G, B));
			const int diff = (max - min);
			const int sum = (max + min);

			if (diff != 0) {
				int H, S, L;

				if (sum <= 255)
					S = 255 * diff / sum;
				else
					S = 255 * diff / (255 * 2 - sum);

				if (R == max)
					H = 60 * (G - B) / diff;
				else if (G == max)
					H = 120 + 60 * (B - R) / diff;
				else
					H = 240 + 60 * (R - G) / diff;

				if (H < 0)
					H = H + 360;

				// Scale the result

				H = (H * hueScale) / 255;
				S = (S * satScale) / 255;
				L = (sum * lightScale) / 255;

				// HLS to RGB (Foley and VanDam)

				int m1, m2;
				if (L <= 255)
					m2 = L * (255 + S) / (255 * 2);
				else
					m2 = L * (255 - S) / (255 * 2) + S;

				m1 = L - m2;

				R = HSL2RGBHelper(m1, m2, H + 120);
				G = HSL2RGBHelper(m1, m2, H);
				B = HSL2RGBHelper(m1, m2, H - 120);
			} else {
				// Maximal color = minimal color -> R=G=B -> it's a grayscale.
				R = G = B = (R * lightScale) / 255;
			}

			*cur++ = R;
			*cur++ = G;
			*cur++ = B;
		}

		setDirtyColors(startColor, endColor);
	}
}
#endif


int ScummEngine::remapPaletteColor(int r, int g, int b, int threshold) {
	int i;
	int ar, ag, ab;
	uint sum, bestsum, bestitem = 0;

	int startColor = (_game.version == 8) ? 24 : 1;
	byte *pal = _currentPalette + startColor * 3;

	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;

	bestsum = 0x7FFFFFFF;

	r &= ~3;
	g &= ~3;
	b &= ~3;

	for (i = startColor; i < 255; i++, pal += 3) {
		if (_game.version == 7 && _colorUsedByCycle[i])
			continue;

		ar = pal[0] & ~3;
		ag = pal[1] & ~3;
		ab = pal[2] & ~3;
		if (ar == r && ag == g && ab == b)
			return i;

		sum = colorWeight(ar - r, ag - g, ab - b);

		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
	}

	if (threshold != -1 && bestsum > colorWeight(threshold, threshold, threshold)) {
		// Best match exceeded threshold. Try to find an unused palette entry and
		// use it for our purpose.
		pal = _currentPalette + (256 - 2) * 3;
		for (i = 254; i > 48; i--, pal -= 3) {
			if (pal[0] >= 252 && pal[1] >= 252 && pal[2] >= 252) {
				setPalColor(i, r, g, b);
				return i;
			}
		}
	}

	return bestitem;
}

void ScummEngine::swapPalColors(int a, int b) {
	byte *ap, *bp;
	byte t;

	if ((uint) a >= 256 || (uint) b >= 256)
		error("swapPalColors: invalid values, %d, %d", a, b);

	ap = &_currentPalette[a * 3];
	bp = &_currentPalette[b * 3];

	t = ap[0];
	ap[0] = bp[0];
	bp[0] = t;
	t = ap[1];
	ap[1] = bp[1];
	bp[1] = t;
	t = ap[2];
	ap[2] = bp[2];
	bp[2] = t;

	setDirtyColors(a, a);
	setDirtyColors(b, b);
}

void ScummEngine::copyPalColor(int dst, int src) {
	byte *dp, *sp;

	if ((uint) dst >= 256 || (uint) src >= 256)
		error("copyPalColor: invalid values, %d, %d", dst, src);

	dp = &_currentPalette[dst * 3];
	sp = &_currentPalette[src * 3];

	dp[0] = sp[0];
	dp[1] = sp[1];
	dp[2] = sp[2];

	setDirtyColors(dst, dst);
}

void ScummEngine::setPalColor(int idx, int r, int g, int b) {
	if (_game.heversion == 70)
		idx = _HEV7ActorPalette[idx];

	_currentPalette[idx * 3 + 0] = r;
	_currentPalette[idx * 3 + 1] = g;
	_currentPalette[idx * 3 + 2] = b;
	if (_game.version == 8) {
		_darkenPalette[idx * 3 + 0] = r;
		_darkenPalette[idx * 3 + 1] = g;
		_darkenPalette[idx * 3 + 2] = b;
	}
	setDirtyColors(idx, idx);
}

void ScummEngine::setPalette(int palindex) {
	const byte *pals;

	_curPalIndex = palindex;
	pals = getPalettePtr(_curPalIndex, _roomResource);
	setPaletteFromPtr(pals);
}

void ScummEngine::setRoomPalette(int palindex, int room) {
	const byte *roomptr = getResourceAddress(rtRoom, room);
	assert(roomptr);
	const byte *pals = findResource(MKID_BE('PALS'), roomptr);
	assert(pals);
	const byte *rgbs = findPalInPals(pals, palindex);
	assert(rgbs);
	setPaletteFromPtr(rgbs);
}

const byte *ScummEngine::findPalInPals(const byte *pal, int idx) {
	const byte *offs;
	uint32 size;

	pal = findResource(MKID_BE('WRAP'), pal);
	if (pal == NULL)
		return NULL;

	offs = findResourceData(MKID_BE('OFFS'), pal);
	if (offs == NULL)
		return NULL;

	size = getResourceDataSize(offs) / 4;
	if ((uint32)idx >= (uint32)size)
		return NULL;

	return offs + READ_LE_UINT32(offs + idx * sizeof(uint32));
}

const byte *ScummEngine::getPalettePtr(int palindex, int room) {
	const byte *cptr;

	cptr = getResourceAddress(rtRoom, room);
	assert(cptr);
	if (_CLUT_offs) {
		cptr += _CLUT_offs;
	} else {
		cptr = findPalInPals(cptr + _PALS_offs, palindex);
		assert(cptr);
	}
	return cptr;
}

void ScummEngine::updatePalette() {
	if (_palDirtyMax == -1)
		return;

	bool noir_mode = (_game.id == GID_SAMNMAX && readVar(0x8000));
	int first = _palDirtyMin;
	int num = _palDirtyMax - first + 1;
	int i;

	byte palette_colors[1024];
	byte *p = palette_colors;

	for (i = _palDirtyMin; i <= _palDirtyMax; i++) {
		byte *data;

		if (_game.features & GF_SMALL_HEADER && _game.version > 2)
			data = _currentPalette + _shadowPalette[i] * 3;
		else
			data = _currentPalette + i * 3;

		// Sam & Max film noir mode. Convert the colours to grayscale
		// before uploading them to the backend.

		if (noir_mode) {
			int r, g, b;
			byte brightness;

			r = data[0];
			g = data[1];
			b = data[2];

			brightness = (byte)((0.299 * r + 0.587 * g + 0.114 * b) + 0.5);

			*p++ = brightness;
			*p++ = brightness;
			*p++ = brightness;
			*p++ = 0;
		} else {
			*p++ = data[0];
			*p++ = data[1];
			*p++ = data[2];
			*p++ = 0;
		}
	}

	_system->setPalette(palette_colors, first, num);

	_palDirtyMax = -1;
	_palDirtyMin = 256;
}

} // End of namespace Scumm
