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
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/costume.h"
#include "scumm/sound.h"
#include "scumm/util.h"

namespace Scumm {

#ifdef PALMOS_68K
const byte *smallCostumeScaleTable;
#else
const byte smallCostumeScaleTable[256] = {
	0xFF, 0xFD, 0x7D, 0xBD, 0x3D, 0xDD, 0x5D, 0x9D,
	0x1D, 0xED, 0x6D, 0xAD, 0x2D, 0xCD, 0x4D, 0x8D,
	0x0D, 0xF5, 0x75, 0xB5, 0x35, 0xD5, 0x55, 0x95,
	0x15, 0xE5, 0x65, 0xA5, 0x25, 0xC5, 0x45, 0x85,
	0x05, 0xF9, 0x79, 0xB9, 0x39, 0xD9, 0x59, 0x99,
	0x19, 0xE9, 0x69, 0xA9, 0x29, 0xC9, 0x49, 0x89,
	0x09, 0xF1, 0x71, 0xB1, 0x31, 0xD1, 0x51, 0x91,
	0x11, 0xE1, 0x61, 0xA1, 0x21, 0xC1, 0x41, 0x81,
	0x01, 0xFB, 0x7B, 0xBB, 0x3B, 0xDB, 0x5B, 0x9B,
	0x1B, 0xEB, 0x6B, 0xAB, 0x2B, 0xCB, 0x4B, 0x8B,
	0x0B, 0xF3, 0x73, 0xB3, 0x33, 0xD3, 0x53, 0x93,
	0x13, 0xE3, 0x63, 0xA3, 0x23, 0xC3, 0x43, 0x83,
	0x03, 0xF7, 0x77, 0xB7, 0x37, 0xD7, 0x57, 0x97,
	0x17, 0xE7, 0x67, 0xA7, 0x27, 0xC7, 0x47, 0x87,
	0x07, 0xEF, 0x6F, 0xAF, 0x2F, 0xCF, 0x4F, 0x8F,
	0x0F, 0xDF, 0x5F, 0x9F, 0x1F, 0xBF, 0x3F, 0x7F,
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE
};
#endif

static const int v1MMNESLookup[25] = {
	0x00, 0x03, 0x01, 0x06, 0x08,
	0x02, 0x00, 0x07, 0x0C, 0x04,
	0x09, 0x0A, 0x12, 0x0B, 0x14,
	0x0D, 0x11, 0x0F, 0x0E, 0x10,
	0x17, 0x00, 0x01, 0x05, 0x16 
};


byte ClassicCostumeRenderer::mainRoutine(int xmoveCur, int ymoveCur) {
	int i, skip = 0;
	byte drawFlag = 1;
	bool use_scaling;
	byte startScaleIndexX;
	int ex1, ex2;
	Common::Rect rect;
	int step;
	Codec1 v1;

	
	const int scaletableSize = 128;
	const bool newAmiCost = (_vm->_game.version == 5) && (_vm->_game.platform == Common::kPlatformAmiga);

	CHECK_HEAP

	v1.scaletable = smallCostumeScaleTable;

	if (_loaded._numColors == 32) {
		v1.mask = 7;
		v1.shr = 3;
	} else {
		v1.mask = 15;
		v1.shr = 4;
	}

	switch (_loaded._format) {
	case 0x60:
	case 0x61:
		// This format is used e.g. in the Sam&Max intro
		ex1 = _srcptr[0];
		ex2 = _srcptr[1];
		_srcptr += 2;
		if (ex1 != 0xFF || ex2 != 0xFF) {
			ex1 = READ_LE_UINT16(_loaded._frameOffsets + ex1 * 2);
			_srcptr = _loaded._baseptr + READ_LE_UINT16(_loaded._baseptr + ex1 + ex2 * 2) + 14;
		}
	}

	use_scaling = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	v1.x = _actorX;
	v1.y = _actorY;

	if (use_scaling) {

		/* Scale direction */
		v1.scaleXstep = -1;
		if (xmoveCur < 0) {
			xmoveCur = -xmoveCur;
			v1.scaleXstep = 1;
		}

		if (_mirror) {
			/* Adjust X position */
			startScaleIndexX = _scaleIndexX = scaletableSize - xmoveCur;
			for (i = 0; i < xmoveCur; i++) {
				if (v1.scaletable[_scaleIndexX++] < _scaleX)
					v1.x -= v1.scaleXstep;
			}

			rect.left = rect.right = v1.x;

			_scaleIndexX = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (rect.right < 0) {
					skip++;
					startScaleIndexX = _scaleIndexX;
				}
				if (v1.scaletable[_scaleIndexX++] < _scaleX)
					rect.right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			startScaleIndexX = _scaleIndexX = xmoveCur + scaletableSize;
			for (i = 0; i < xmoveCur; i++) {
				if (v1.scaletable[_scaleIndexX--] < _scaleX)
					v1.x += v1.scaleXstep;
			}

			rect.left = rect.right = v1.x;

			_scaleIndexX = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (rect.left >= _out.w) {
					startScaleIndexX = _scaleIndexX;
					skip++;
				}
				if (v1.scaletable[_scaleIndexX--] < _scaleX)
					rect.left--;
			}
		}
		_scaleIndexX = startScaleIndexX;

		if (skip)
			skip--;

		step = -1;
		if (ymoveCur < 0) {
			ymoveCur = -ymoveCur;
			step = 1;
		}

		_scaleIndexY = scaletableSize - ymoveCur;
		for (i = 0; i < ymoveCur; i++) {
			if (v1.scaletable[_scaleIndexY++] < _scaleY)
				v1.y -= step;
		}

		rect.top = rect.bottom = v1.y;
		_scaleIndexY = scaletableSize - ymoveCur;
		for (i = 0; i < _height; i++) {
			if (v1.scaletable[_scaleIndexY++] < _scaleY)
				rect.bottom++;
		}

		_scaleIndexY = scaletableSize - ymoveCur;
	} else {
		if (!_mirror)
			xmoveCur = -xmoveCur;

		v1.x += xmoveCur;
		v1.y += ymoveCur;

		if (_mirror) {
			rect.left = v1.x;
			rect.right = v1.x + _width;
		} else {
			rect.left = v1.x - _width;
			rect.right = v1.x;
		}

		rect.top = v1.y;
		rect.bottom = rect.top + _height;

	}

	v1.skip_width = _width;
	v1.scaleXstep = _mirror ? 1 : -1;

	if (_vm->_game.version == 1)
		// V1 games uses 8 x 8 pixels for actors
		_vm->markRectAsDirty(kMainVirtScreen, rect.left, rect.right + 8, rect.top, rect.bottom, _actorID);
	else
		_vm->markRectAsDirty(kMainVirtScreen, rect.left, rect.right + 1, rect.top, rect.bottom, _actorID);

	if (rect.top >= _out.h || rect.bottom <= 0)
		return 0;

	if (rect.left >= _out.w || rect.right <= 0)
		return 0;

	v1.replen = 0;

	if (_mirror) {
		if (!use_scaling)
			skip = -v1.x;
		if (skip > 0) {
			if (!newAmiCost && _loaded._format != 0x57) {
				v1.skip_width -= skip;
				codec1_ignorePakCols(v1, skip);
				v1.x = 0;
			}
		} else {
			skip = rect.right - _out.w;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	} else {
		if (!use_scaling)
			skip = rect.right - _out.w;
		if (skip > 0) {
			if (!newAmiCost && _loaded._format != 0x57) {
				v1.skip_width -= skip;
				codec1_ignorePakCols(v1, skip);
				v1.x = _out.w - 1;
			}
		} else {
			// V1 games uses 8 x 8 pixels for actors
			if (_loaded._format == 0x57)
				skip = -8 - rect.left;
			else
				skip = -1 - rect.left;
			if (skip <= 0)
				drawFlag = 2;
			else
				v1.skip_width -= skip;
		}
	}

	if (v1.skip_width <= 0)
		return 0;

	if (rect.left < 0)
		rect.left = 0;

	if (rect.top < 0)
		rect.top = 0;

	if (rect.top > _out.h)
		rect.top = _out.h;

	if (rect.bottom > _out.h)
		rect.bottom = _out.h;

	if (_draw_top > rect.top)
		_draw_top = rect.top;
	if (_draw_bottom < rect.bottom)
		_draw_bottom = rect.bottom;

	if (_height + rect.top >= 256) {
		CHECK_HEAP
		return 2;
	}

	v1.destptr = (byte *)_out.pixels + v1.y * _out.pitch + v1.x;

	v1.mask_ptr = _vm->getMaskBuffer(0, v1.y, _zbuf);

	CHECK_HEAP

	if (_loaded._format == 0x57) {
		// The v1 costume renderer needs the actor number, which is
		// the same thing as the costume renderer's _actorID.
		procC64(v1, _actorID);
	} else if (newAmiCost)
		proc3_ami(v1);
	else
		proc3(v1);

	CHECK_HEAP
	return drawFlag;
}

static const int v1MMActorPalatte1[25] = {
	8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};
static const int v1MMActorPalatte2[25] = {
	0, 7, 2, 6, 9, 1, 3, 7, 7, 1, 1, 9, 1, 4, 5, 5, 4, 1, 0, 5, 4, 2, 2, 7, 7
};

#define MASK_AT(xoff) \
	(mask && (mask[((v1.x + xoff) / 8)] & revBitMask((v1.x + xoff) & 7)))
#define LINE(c,p) \
	pcolor = (color >> c) & 3; \
	if (pcolor) { \
		if (!MASK_AT(p)) \
			dst[p] = palette[pcolor]; \
		if (!MASK_AT(p + 1)) \
			dst[p + 1] = palette[pcolor]; \
	}

void ClassicCostumeRenderer::procC64(Codec1 &v1, int actor) {
	const byte *mask, *src;
	byte *dst;
	byte len;
	int y;
	uint height;
	byte color, pcolor;
	bool rep;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	v1.skip_width /= 8;

	// Set up the palette data
	byte palette[4] = { 0, 0, 0, 0 };
	if (_vm->getCurrentLights() & LIGHTMODE_actor_use_colors) {
		if (_vm->_game.id == GID_MANIAC) {
			palette[1] = v1MMActorPalatte1[actor];
			palette[2] = v1MMActorPalatte2[actor];
		} else {
			// Adjust for C64 version of Zak McKracken
			palette[1] = (_vm->_game.platform == Common::kPlatformC64) ? 10 : 8;
			palette[2] = _palette[actor];
		}
	} else {
		palette[2] = 11;
		palette[3] = 11;
	}
	mask = v1.mask_ptr;

	if (len)
		goto StartPos;

	do {
		len = *src++;
		if (len & 0x80)
			color = *src++;
	StartPos:;
		rep = (len & 0x80) != 0;
		len &= 0x7f;
		while (len--) {
			if (!rep)
				color = *src++;
			
			if (0 <= y && y < _out.h && 0 <= v1.x && v1.x < _out.w) {
				if (!_mirror) {
					LINE(0, 0); LINE(2, 2); LINE(4, 4); LINE(6, 6);
				} else {
					LINE(6, 0); LINE(4, 2); LINE(2, 4); LINE(0, 6);
				}
			}
			dst += _out.pitch;
			y++;
			mask += _numStrips;
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;
				v1.x += 8 * v1.scaleXstep;
				if (v1.x < 0 || v1.x >= _out.w)
					return;
				mask = v1.mask_ptr;
				v1.destptr += 8 * v1.scaleXstep;
				dst = v1.destptr;
			}
		}
	} while (1);
}

#undef LINE
#undef MASK_AT

void ClassicCostumeRenderer::proc3(Codec1 &v1) {
	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	int y;
	uint color, height, pcolor;
	const byte *scaleytab;
	bool masked;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[_scaleIndexY];
	maskbit = revBitMask(v1.x & 7);
	mask = v1.mask_ptr + v1.x / 8;

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || *scaleytab++ < _scaleY) {
				masked = (y < 0 || y >= _out.h) || (v1.mask_ptr && (mask[0] & maskbit));
				
				if (color && !masked) {
					if (_shadow_mode & 0x20) {
						pcolor = _shadow_table[*dst];
					} else {
						pcolor = _palette[color];
						if (pcolor == 13 && _shadow_table)
							pcolor = _shadow_table[*dst];
					}
					*dst = pcolor;
				}
				dst += _out.pitch;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[_scaleIndexY];

				if (_scaleX == 255 || v1.scaletable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _out.w)
						return;
					maskbit = revBitMask(v1.x & 7);
					v1.destptr += v1.scaleXstep;
				}
				_scaleIndexX += v1.scaleXstep;
				dst = v1.destptr;
				mask = v1.mask_ptr + v1.x / 8;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void ClassicCostumeRenderer::proc3_ami(Codec1 &v1) {
	const byte *mask, *src;
	byte *dst;
	byte maskbit, len, height, width;
	int color;
	int y;
	bool masked;
	int oldXpos, oldScaleIndexX;

	mask = v1.mask_ptr + v1.x / 8;
	dst = v1.destptr;
	height = _height;
	width = _width;
	src = _srcptr;
	maskbit = revBitMask(v1.x & 7);
	y = v1.y;
	oldXpos = v1.x;
	oldScaleIndexX = _scaleIndexX;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;
		do {
			if (_scaleY == 255 || v1.scaletable[_scaleIndexY] < _scaleY) {
				masked = (y < 0 || y >= _out.h) || (v1.mask_ptr && (mask[0] & maskbit));
				
				if (color && v1.x >= 0 && v1.x < _out.w && !masked) {
					*dst = _palette[color];
				}

				if (_scaleX == 255 || v1.scaletable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					dst += v1.scaleXstep;
					maskbit = revBitMask(v1.x & 7);
				}
				_scaleIndexX += v1.scaleXstep;
				mask = v1.mask_ptr + v1.x / 8;
			}
			if (!--width) {
				if (!--height)
					return;

				if (y >= _out.h)
					return;

				if (v1.x != oldXpos) {
					dst += _out.pitch - (v1.x - oldXpos);
					v1.mask_ptr += _numStrips;
					mask = v1.mask_ptr + oldXpos / 8;
					maskbit = revBitMask(oldXpos & 7);
					y++;
				}
				width = _width;
				v1.x = oldXpos;
				_scaleIndexX = oldScaleIndexX;
				_scaleIndexY++;
			}
		} while (--len);
	} while (1);
}

void ClassicCostumeLoader::loadCostume(int id) {
	_id = id;
	byte *ptr = _vm->getResourceAddress(rtCostume, id);

	if (_vm->_game.version >= 6)
		ptr += 8;
	else if (_vm->_game.features & GF_OLD_BUNDLE)
		ptr += -2;
	else if (_vm->_game.features & GF_SMALL_HEADER)
		ptr += 0;
	else
		ptr += 2;

	_baseptr = ptr;

	_numAnim = ptr[6];
	_format = ptr[7] & 0x7F;
	_mirror = (ptr[7] & 0x80) != 0;
	_palette = ptr + 8;
	switch (_format) {
	case 0x57:				// Only used in V1 games
		_numColors = 0;
		break;
	case 0x58:
		_numColors = 16;
		break;
	case 0x59:
		_numColors = 32;
		break;
	case 0x60:				// New since version 6
		_numColors = 16;
		break;
	case 0x61:				// New since version 6
		_numColors = 32;
		break;
	default:
		error("Costume %d with format 0x%X is invalid", id, _format);
	}

	
	// In GF_OLD_BUNDLE games, there is no actual palette, just a single color byte. 
	// Don't forget, these games were designed around a fixed 16 color HW palette :-)
	// In addition, all offsets are shifted by 2; we accomodate that via a separate
	// _baseptr value (instead of adding tons of if's throughout the code).
	if (_vm->_game.features & GF_OLD_BUNDLE) {
		_numColors = (_format == 0x57) ? 0 : 1;
		_baseptr += 2;
	}
	ptr += 8 + _numColors;
	_frameOffsets = ptr + 2;
	if (_format == 0x57) {
		_dataOffsets = ptr + 18;
		_baseptr += 4;
	} else {
		_dataOffsets = ptr + 34;
	}
	_animCmds = _baseptr + READ_LE_UINT16(ptr);
}

byte NESCostumeRenderer::drawLimb(const Actor *a, int limb) {
	const byte darkpalette[16] = {0x00,0x00,0x2D,0x3D,0x00,0x00,0x2D,0x3D,0x00,0x00,0x2D,0x3D,0x00,0x00,0x2D,0x3D};
	const CostumeData &cost = a->_cost;
	const byte *palette, *src, *sprdata;
	int anim, frameNum, frame, offset, numSprites;

	// If the specified limb is stopped or not existing, do nothing.
	if (cost.curpos[limb] == 0xFFFF)
		return 0;

	if (_vm->getCurrentLights() & LIGHTMODE_actor_use_base_palette)
		palette = _vm->_NESPalette[1];
	else
		palette = darkpalette;

	src = _loaded._dataOffsets;
	anim = 4 * cost.frame[limb] + newDirToOldDir(a->getFacing());
	frameNum = cost.curpos[limb];
	frame = src[src[2 * anim] + frameNum];

	offset = READ_LE_UINT16(_vm->_NEScostdesc + v1MMNESLookup[_loaded._id] * 2);
	numSprites = _vm->_NEScostlens[offset + frame] + 1;
	sprdata = _vm->_NEScostdata + READ_LE_UINT16(_vm->_NEScostoffs + 2 * (offset + frame)) + numSprites * 3;

	bool flipped = (newDirToOldDir(a->getFacing()) == 1);
	int left = 239, right = 0, top = 239, bottom = 0;
	byte *maskBuf = _vm->getMaskBuffer(0, 0, 1);

	for (int spr = 0; spr < numSprites; spr++) {
		byte mask, tile, sprpal;
		int8 y, x;

		sprdata -= 3;

		mask = (sprdata[0] & 0x80) ? 0x01 : 0x80;
		y = sprdata[0] << 1;
		y >>= 1;

		tile = sprdata[1];

		sprpal = (sprdata[2] & 0x03) << 2;
		x = sprdata[2];
		x >>= 2;

		if (flipped) {
			mask = (mask == 0x80) ? 0x01 : 0x80;
			x = -x;
		}

		if ((_actorX + x < 0) || (_actorX + x + 8 >= _out.w))
			continue;
		if ((_actorY + y < 0) || (_actorY + y + 8 >= _out.h))
			continue;

		for (int ty = 0; ty < 8; ty++) {
			byte c1 = _vm->_NESPatTable[0][tile * 16 + ty];
			byte c2 = _vm->_NESPatTable[0][tile * 16 + ty + 8];

			for (int tx = 0; tx < 8; tx++) {
				unsigned char c = ((c1 & mask) ? 1 : 0) | ((c2 & mask) ? 2 : 0) | sprpal;
				if (mask == 0x01) {
					c1 >>= 1;
					c2 >>= 1;
				} else {
					c1 <<= 1;
					c2 <<= 1;
				}
				if (!(c & 3))
					continue;
				int my = _actorY + y + ty;
				int mx = _actorX + x + tx;
				if (!(_zbuf && (maskBuf[my * _numStrips + mx / 8] & revBitMask(mx & 7))))
					*((byte *)_out.pixels + my * _out.pitch + mx) = palette[c];
			}
		}
		left = MIN(left, _actorX + x);
		right = MAX(right, _actorX + x + 8);
		top = MIN(top, _actorY + y);
		bottom = MAX(bottom, _actorY + y + 8);
	}

	_draw_top = top;
	_draw_bottom = bottom;

	_vm->markRectAsDirty(kMainVirtScreen, left, right, top, bottom, _actorID);

	return 0;
}

byte ClassicCostumeRenderer::drawLimb(const Actor *a, int limb) {
	int i;
	int code;
	const byte *frameptr;
	const CostumeData &cost = a->_cost;

	// If the specified limb is stopped or not existing, do nothing.
	if (cost.curpos[limb] == 0xFFFF || cost.stopped & (1 << limb))
		return 0;

	// Determine the position the limb is at
	i = cost.curpos[limb] & 0x7FFF;
	
	// Get the frame pointer for that limb
	frameptr = _loaded._baseptr + READ_LE_UINT16(_loaded._frameOffsets + limb * 2);

	// Determine the offset to the costume data for the limb at position i
	code = _loaded._animCmds[i] & 0x7F;

	// Code 0x7B indicates a limb for which there is nothing to draw
	if (code != 0x7B) {
		_srcptr = _loaded._baseptr + READ_LE_UINT16(frameptr + code * 2);

		if (!(_vm->_game.features & GF_OLD256) || code < 0x79) {
			const CostumeInfo *costumeInfo;
			int xmoveCur, ymoveCur;

			if (_loaded._format == 0x57) {
				_width = _srcptr[0] * 8;
				_height = _srcptr[1];
				xmoveCur = _xmove + (int8)_srcptr[2] * 8;
				ymoveCur = _ymove - (int8)_srcptr[3];
				_xmove += (int8)_srcptr[4] * 8;
				_ymove -= (int8)_srcptr[5];
				_srcptr += 6;
			} else {
				costumeInfo = (const CostumeInfo *)_srcptr;
				_width = READ_LE_UINT16(&costumeInfo->width);
				_height = READ_LE_UINT16(&costumeInfo->height);
				xmoveCur = _xmove + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
				ymoveCur = _ymove + (int16)READ_LE_UINT16(&costumeInfo->rel_y);
				_xmove += (int16)READ_LE_UINT16(&costumeInfo->move_x);
				_ymove -= (int16)READ_LE_UINT16(&costumeInfo->move_y);
				_srcptr += 12;
			}

			return mainRoutine(xmoveCur, ymoveCur);
		}
	}

	return 0;

}

void NESCostumeRenderer::setPalette(byte *palette) {
	// TODO
}

void NESCostumeRenderer::setFacing(const Actor *a) {
	// TODO
	//_mirror = newDirToOldDir(a->getFacing()) != 0 || _loaded._mirror;
}

void NESCostumeRenderer::setCostume(int costume, int shadow) {
	_loaded.loadCostume(costume);
}

void ClassicCostumeLoader::costumeDecodeData(Actor *a, int frame, uint usemask) {
	const byte *r;
	uint mask, j;
	int i;
	byte extra, cmd;
	int anim;

	loadCostume(a->_costume);

	anim = newDirToOldDir(a->getFacing()) + frame * 4;

	if (anim > _numAnim) {
		return;
	}

	r = _baseptr + READ_LE_UINT16(_dataOffsets + anim * 2);

	if (r == _baseptr) {
		return;
	}

	if (_vm->_game.version == 1) {
		mask = *r++ << 8;
	} else {
		mask = READ_LE_UINT16(r);
		r += 2;
	}
	i = 0;
	do {
		if (mask & 0x8000) {
			if (_vm->_game.version <= 3) {
				j = *r++;

				if (j == 0xFF)
					j = 0xFFFF;
			} else {
				j = READ_LE_UINT16(r);
				r += 2;
			}
			if (usemask & 0x8000) {
				if (j == 0xFFFF) {
					a->_cost.curpos[i] = 0xFFFF;
					a->_cost.start[i] = 0;
					a->_cost.frame[i] = frame;
				} else {
					extra = *r++;
					cmd = _animCmds[j];
					if (cmd == 0x7A) {
						a->_cost.stopped &= ~(1 << i);
					} else if (cmd == 0x79) {
						a->_cost.stopped |= (1 << i);
					} else {
						a->_cost.curpos[i] = a->_cost.start[i] = j;
						a->_cost.end[i] = j + (extra & 0x7F);
						if (extra & 0x80)
							a->_cost.curpos[i] |= 0x8000;
						a->_cost.frame[i] = frame;
					}
				}
			} else {
				if (j != 0xFFFF)
					r++;
			}
		}
		i++;
		usemask <<= 1;
		mask <<= 1;
	} while (mask&0xFFFF);
}

void ClassicCostumeRenderer::setPalette(byte *palette) {
	int i;
	byte color;

	if (_loaded._format == 0x57) {
		memcpy(_palette, palette, 13);
	} else if (_vm->_game.features & GF_OLD_BUNDLE) {
		if (_vm->getCurrentLights() & LIGHTMODE_actor_use_colors) {
			memcpy(_palette, palette, 16);
		} else {
			memset(_palette, 8, 16);
			_palette[12] = 0;
		}
		_palette[_loaded._palette[0]] = _palette[0];
	} else {
		if (_vm->getCurrentLights() & LIGHTMODE_actor_use_colors) {
			for (i = 0; i < _loaded._numColors; i++) {
				color = palette[i];
				if (color == 255)
					color = _loaded._palette[i];
				_palette[i] = color;
			}
		} else {
			memset(_palette, 8, _loaded._numColors);
			_palette[12] = 0;
		}
	}
}

void ClassicCostumeRenderer::setFacing(const Actor *a) {
	_mirror = newDirToOldDir(a->getFacing()) != 0 || _loaded._mirror;
}

void ClassicCostumeRenderer::setCostume(int costume, int shadow) {
	_loaded.loadCostume(costume);
}

byte ClassicCostumeLoader::increaseAnims(Actor *a) {
	int i;
	byte r = 0;

	for (i = 0; i != 16; i++) {
		if (a->_cost.curpos[i] != 0xFFFF)
			r += increaseAnim(a, i);
	}
	return r;
}

byte ClassicCostumeLoader::increaseAnim(Actor *a, int slot) {
	int highflag;
	int i, end;
	byte code, nc;

	if (a->_cost.curpos[slot] == 0xFFFF)
		return 0;

	highflag = a->_cost.curpos[slot] & 0x8000;
	i = a->_cost.curpos[slot] & 0x7FFF;
	end = a->_cost.end[slot];
	code = _animCmds[i] & 0x7F;
	
	if (_vm->_game.version <= 3) {
		if (_animCmds[i] & 0x80)
			a->_cost.soundCounter++;
	}
	
	do {
		if (!highflag) {
			if (i++ >= end)
				i = a->_cost.start[slot];
		} else {
			if (i != end)
				i++;
		}
		nc = _animCmds[i];

		if (nc == 0x7C) {
			a->_cost.animCounter++;
			if (a->_cost.start[slot] != end)
				continue;
		} else {
			if (_vm->_game.version >= 6) {
				if (nc >= 0x71 && nc <= 0x78) {
					uint sound = (_vm->_game.heversion == 60) ? 0x78 - nc : nc - 0x71;
					_vm->_sound->addSoundToQueue2(a->_sound[sound]);
					if (a->_cost.start[slot] != end)
						continue;
				}
			} else {
				if (nc == 0x78) {
					a->_cost.soundCounter++;
					if (a->_cost.start[slot] != end)
						continue;
				}
			}
		}

		a->_cost.curpos[slot] = i | highflag;
		return (_animCmds[i] & 0x7F) != code;
	} while (1);
}

/**
 * costume ID -> v1MMNESLookup[] -> desc -> lens & offs -> data -> Gfx & pal
 */
void NESCostumeLoader::loadCostume(int id) {
	_id = id;
	_baseptr = _vm->getResourceAddress(rtCostume, id);
	_dataOffsets = _baseptr + 2;
	_numAnim = 0x17;
}

void NESCostumeLoader::costumeDecodeData(Actor *a, int frame, uint usemask) {
	int anim;

	loadCostume(a->_costume);

	anim = 4 * frame + newDirToOldDir(a->getFacing());

	if (anim > _numAnim) {
		return;
	}

	a->_cost.curpos[0] = 0;
	a->_cost.start[0] = 0;
	a->_cost.end[0] = _dataOffsets[2 * anim + 1];
	a->_cost.frame[0] = frame;
}

byte NESCostumeLoader::increaseAnims(Actor *a) {
	int i;
	byte r = 0;

	for (i = 0; i != 16; i++) {
		if (a->_cost.curpos[i] != 0xFFFF)
			r += increaseAnim(a, i);
	}
	return r;
}

byte NESCostumeLoader::increaseAnim(Actor *a, int slot) {
	int oldframe = a->_cost.curpos[slot]++;
	if (a->_cost.curpos[slot] >= a->_cost.end[slot])
		a->_cost.curpos[slot] = a->_cost.start[slot];
	return (a->_cost.curpos[slot] != oldframe);
}

static const byte actorColorsMMC64[25] = {
	0, 7, 2, 6, 9, 1, 3, 7, 7, 1, 1, 9, 1, 4, 5, 5, 4, 1, 0, 5, 4, 2, 2, 7, 7
};

#define MASK_AT(xoff) \
	(mask && (mask[((destX + xoff) / 8)] & revBitMask((destX + xoff) & 7)))
#define LINE(c,p) \
	pcolor = (color >> c) & 3; \
	if (pcolor) { \
		if (!MASK_AT(p)) \
			dst[p] = palette[pcolor]; \
		if (!MASK_AT(p + 1)) \
			dst[p + 1] = palette[pcolor]; \
	}

byte C64CostumeRenderer::drawLimb(const Actor *a, int limb) {
	if (limb >= 8)
		return 0;

	if (limb == 0) {
		_draw_top = 200;
		_draw_bottom = 0;
	}

	// TODO:
	// get out how animations are handled
	byte state = a->_moving != 0 ? 0 : 1;
	byte unk1 = (_loaded._animCmds + (state*32) + newDirToOldDir(a->getFacing()) * 8)[limb];
	byte unk2 = _loaded._frameOffsets[_loaded._frameOffsets[limb] + (unk1 & 0x7f)];
	bool flipped = (unk1 & 0x80) != 0;

	byte p1 = _loaded._frameOffsets[unk2];
	byte temp1 = _loaded._baseptr[p1];
	byte temp2 = temp1 + _loaded._dataOffsets[4];
	int offL = _loaded._baseptr[temp1 + 2];
	int offH = _loaded._baseptr[temp2];
	int off = (offH << 8) + offL;

	const byte *data = _loaded._baseptr + off;

	// Set up the palette data
	byte palette[4] = { 0, 0, 0, 0 };
	if (_vm->getCurrentLights() & LIGHTMODE_actor_use_colors) {
		palette[1] = 10;
		palette[2] = actorColorsMMC64[_actorID];
	} else {
		palette[2] = 11;
		palette[3] = 11;
	}

	int width = *data++;
	int height = *data++;
	int offsetX = *data++;
	int offsetY = *data++;
	// these two fields seems to be most times zero
	// byte6 was one time 255 in one costume I tried
//	int byte5 = *data++;
//	int byte6 = *data++;
//	debug(3, "byte5: %d", byte5);
//	debug(3, "byte6: %d", byte6);
	data += 2;

	if (!width || !height)
		return 0;

	int xpos = 0;
	int ypos = _loaded._maxHeight - offsetY;
	
	if (flipped) {
		if (offsetX)
			xpos += (offsetX-1) * 8;
	} else {
		xpos += offsetX * 8;
	}

	// + 4 could be commented, because maybe the _actorX position is
	// wrong, I looked at the scumm-c64 interpreter by lloyd
	// and there Bernhard is directly on the right in the intro
	// but here in ScummVM he is 4 pixel left of the other position.
	xpos += _actorX - (a->_width / 2) + 4;
	ypos += _actorY - _loaded._maxHeight;

	// This code is very similar to procC64()
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			byte color = data[y * width + x];
			byte pcolor;
			int realX = 0;
			if (flipped) {
				if (offsetX == 0||offsetX == 1) {
					realX = width-(x+1);
				} else if (offsetX == 2) {
					realX = width-(x+2);
				}
			} else {
				realX = x;
			}

			int destY = y + ypos;
			int destX = realX * 8 + xpos;
			if (destY >= 0 && destY < _out.h && destX >= 0 && destX < _out.w) {
				byte *dst = (byte *)_out.pixels + destY * _out.pitch + destX;
				byte *mask = _vm->getMaskBuffer(0, destY, _zbuf);
				if (flipped) {
					LINE(0, 0); LINE(2, 2); LINE(4, 4); LINE(6, 6);
				} else {
					LINE(6, 0); LINE(4, 2); LINE(2, 4); LINE(0, 6);
				}
			}
		}
	}

	_draw_top = MIN(_draw_top, ypos);
	_draw_bottom = MAX(_draw_bottom, ypos+height);
	// if +4 above is NOT commented, here "+(flipped ? 4 : 0)" can be commented out
	// and other way round
	_vm->markRectAsDirty(kMainVirtScreen, xpos, xpos+(width*8)/*+(flipped ? 4 : 0)*/, ypos, ypos+height, _actorID);

	return 0;
}

#undef LINE
#undef MASK_AT

void C64CostumeRenderer::setCostume(int costume, int shadow) {
	_loaded.loadCostume(costume);
}

void C64CostumeLoader::loadCostume(int id) {
	const byte *ptr = _vm->getResourceAddress(rtCostume, id);
	_id = id;
	_baseptr = ptr + 9;

	_format = 0x57;
	_numColors = 0;
	_numAnim = 0;
	_mirror = 0;
	_palette = &actorColorsMMC64[id];

	_frameOffsets = _baseptr + READ_LE_UINT16(ptr + 5);
	_dataOffsets = ptr;
	_animCmds = _baseptr + READ_LE_UINT16(ptr + 7);

	_maxHeight = 0;
	for (int i = 0; i < 8; ++i) {
		int pid = _frameOffsets[_frameOffsets[i]];
		byte p1 = _frameOffsets[pid];
		byte b = _baseptr[p1];
		byte c = b + _dataOffsets[4];
		int offL = _baseptr[b + 2];
		int offH = _baseptr[c];
		int off = (offH << 8) + offL;
		const byte *data = _baseptr + off;

		if (data[3] > _maxHeight) {
			_maxHeight = data[3];		// data[3] is libs's Y offset
		}
	}
	++_maxHeight;
}

void C64CostumeLoader::costumeDecodeData(Actor *a, int frame, uint usemask) {
}

byte C64CostumeLoader::increaseAnims(Actor *a) {
	return 0;
}

byte C64CostumeLoader::increaseAnim(Actor *a, int slot) {
	return 0;
}


} // End of namespace Scumm

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Costume)
_GSETPTR(Scumm::smallCostumeScaleTable, GBVARS_SMALLSCALETABLE_INDEX, byte, GBVARS_SCUMM)
_GEND

_GRELEASE(Costume)
_GRELEASEPTR(GBVARS_SMALLSCALETABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
