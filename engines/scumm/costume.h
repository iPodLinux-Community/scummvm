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

#ifndef COSTUME_H
#define COSTUME_H

#include "scumm/base-costume.h"

namespace Scumm {

class ClassicCostumeLoader : public BaseCostumeLoader {
public:
	int _id;
	const byte *_baseptr;
	const byte *_animCmds;
	const byte *_dataOffsets;
	const byte *_palette;
	const byte *_frameOffsets;
	byte _numColors;
	byte _numAnim;
	byte _format;
	bool _mirror;

	ClassicCostumeLoader(ScummEngine *vm) :
		BaseCostumeLoader(vm),
		_id(-1), _baseptr(0), _animCmds(0), _dataOffsets(0), _palette(0),
		_frameOffsets(0), _numColors(0), _numAnim(0), _format(0), _mirror(false) {}

	void loadCostume(int id);
	void costumeDecodeData(Actor *a, int frame, uint usemask);
	byte increaseAnims(Actor *a);

protected:
	byte increaseAnim(Actor *a, int slot);
};

class NESCostumeLoader : public BaseCostumeLoader {
public:
	int _id;
	const byte *_baseptr;
	const byte *_dataOffsets;
	byte _numAnim;

	NESCostumeLoader(ScummEngine *vm) : BaseCostumeLoader(vm) {}
	void loadCostume(int id);
	void costumeDecodeData(Actor *a, int frame, uint usemask);
	byte increaseAnims(Actor *a);

protected:
	byte increaseAnim(Actor *a, int slot);
};

class C64CostumeLoader : public ClassicCostumeLoader {
public:
	C64CostumeLoader(ScummEngine *vm) : ClassicCostumeLoader(vm) {}
	void loadCostume(int id);
	void costumeDecodeData(Actor *a, int frame, uint usemask);
	byte increaseAnims(Actor *a);

	int _maxHeight;
protected:
	byte increaseAnim(Actor *a, int slot);
};

class ClassicCostumeRenderer : public BaseCostumeRenderer {
protected:
	ClassicCostumeLoader _loaded;

	byte _scaleIndexX;						/* must wrap at 256 */
	byte _scaleIndexY;
	byte _palette[32];

public:
	ClassicCostumeRenderer(ScummEngine *vm) : BaseCostumeRenderer(vm), _loaded(vm) {}

	void setPalette(byte *palette);
	void setFacing(const Actor *a);
	void setCostume(int costume, int shadow);

protected:
	byte drawLimb(const Actor *a, int limb);

	void proc3(Codec1 &v1);
	void proc3_ami(Codec1 &v1);

	void procC64(Codec1 &v1, int actor);

	byte mainRoutine(int xmoveCur, int ymoveCur);
};

class NESCostumeRenderer : public BaseCostumeRenderer {
protected:
	NESCostumeLoader _loaded;

public:
	NESCostumeRenderer(ScummEngine *vm) : BaseCostumeRenderer(vm), _loaded(vm) {}

	void setPalette(byte *palette);
	void setFacing(const Actor *a);
	void setCostume(int costume, int shadow);

protected:
	byte drawLimb(const Actor *a, int limb);
};

class C64CostumeRenderer : public BaseCostumeRenderer {
protected:
	C64CostumeLoader _loaded;

public:
	C64CostumeRenderer(ScummEngine *vm) : BaseCostumeRenderer(vm), _loaded(vm) {}

	void setPalette(byte *palette) {}
	void setFacing(const Actor *a) {}
	void setCostume(int costume, int shadow);

protected:
	byte drawLimb(const Actor *a, int limb);
};

} // End of namespace Scumm

#endif
