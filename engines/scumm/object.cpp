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
#include "scumm/bomp.h"
#include "scumm/intern.h"
#ifndef DISABLE_HE
#include "scumm/he/intern_he.h"
#endif
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"

namespace Scumm {

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct BompHeader {			/* Bomp header */
	union {
		struct {
			uint16 unk;
			uint16 width, height;
		} GCC_PACK old;

		struct {
			uint32 width, height;
		} GCC_PACK v8;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif


bool ScummEngine::getClass(int obj, int cls) const {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getClass");
	cls &= 0x7F;
	checkRange(32, 1, cls, "Class %d out of range in getClass");

	if (_game.features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
		case kObjectClassUntouchable:
			cls = 24;
			break;
		case kObjectClassPlayer:
			cls = 23;
			break;
		case kObjectClassXFlip:
			cls = 19;
			break;
		case kObjectClassYFlip:
			cls = 18;
			break;
		}
	}

	return (_classData[obj] & (1 << (cls - 1))) != 0;
}

void ScummEngine::putClass(int obj, int cls, bool set) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putClass");
	cls &= 0x7F;
	checkRange(32, 1, cls, "Class %d out of range in putClass");

	if (_game.features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
		case kObjectClassUntouchable:
			cls = 24;
			break;
		case kObjectClassPlayer:
			cls = 23;
			break;
		case kObjectClassXFlip:
			cls = 19;
			break;
		case kObjectClassYFlip:
			cls = 18;
			break;
		}
	}

	if (set)
		_classData[obj] |= (1 << (cls - 1));
	else
		_classData[obj] &= ~(1 << (cls - 1));

	if (_game.version <= 4 && obj >= 1 && obj < _numActors) {
		_actors[obj].classChanged(cls, set);
	}
}

int ScummEngine::getOwner(int obj) const {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getOwner");
	return _objectOwnerTable[obj];
}

void ScummEngine::putOwner(int obj, int owner) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putOwner");
	checkRange(0xFF, 0, owner, "Owner %d out of range in putOwner");
	_objectOwnerTable[obj] = owner;
}

int ScummEngine::getState(int obj) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getState");

	if (!_copyProtection) {
		// I knew LucasArts sold cracked copies of the original Maniac Mansion,
		// at least as part of Day of the Tentacle. Apparently they also sold
		// cracked versions of the enhanced version. At least in Germany.
		//
		// This will keep the security door open at all times. I can only
		// assume that 182 and 193 each correspond to one particular side of
		// the it. Fortunately it does not prevent frustrated players from
		// blowing up the mansion, should they feel the urge to.

		if (_game.id == GID_MANIAC && (obj == 182 || obj == 193))
			_objectStateTable[obj] |= 0x08;
	}

	return _objectStateTable[obj];
}

void ScummEngine::putState(int obj, int state) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putState");
	checkRange(0xFF, 0, state, "State %d out of range in putState");
	_objectStateTable[obj] = state;
}

int ScummEngine::getObjectRoom(int obj) const {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getObjectRoom");
	return _objectRoomTable[obj];
}

int ScummEngine::getObjectIndex(int object) const {
	int i;

	if (object < 1)
		return -1;

	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object)
			return i;
	}
	return -1;
}

int ScummEngine::whereIsObject(int object) const {
	int i;

	if (object >= _numGlobalObjects)
		return WIO_NOT_FOUND;

	if (object < 1)
		return WIO_NOT_FOUND;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM) {
		for (i = 0; i < _numInventory; i++)
			if (_inventory[i] == object)
				return WIO_INVENTORY;
		return WIO_NOT_FOUND;
	}

	for (i = (_numLocalObjects-1); i > 0; i--)
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index)
				return WIO_FLOBJECT;
			return WIO_ROOM;
		}

	return WIO_NOT_FOUND;
}

int ScummEngine::getObjectOrActorXY(int object, int &x, int &y) {
	if (object < _numActors) {
		Actor *act = derefActorSafe(object, "getObjectOrActorXY");
		if (act)
			return act->getActorXYPos(x, y);
		else
			return -1;
	}

	switch (whereIsObject(object)) {
	case WIO_NOT_FOUND:
		return -1;
	case WIO_INVENTORY:
		if (_objectOwnerTable[object] < _numActors)
			return derefActor(_objectOwnerTable[object], "getObjectOrActorXY(2)")->getActorXYPos(x, y);
		else
			return -1;
	}
	getObjectXYPos(object, x, y);
	return 0;
}

/**
 * Return the position of an object.
 * Returns X, Y and direction in angles
 */
void ScummEngine::getObjectXYPos(int object, int &x, int &y, int &dir) {
	int idx = getObjectIndex(object);
	assert(idx >= 0);
	ObjectData &od = _objs[idx];
	int state;
	const byte *ptr;
	const ImageHeader *imhd;

	if (_game.version >= 6) {
		state = getState(object) - 1;
		if (state < 0)
			state = 0;

		ptr = getOBIMFromObjectData(od);
		if (!ptr) {
			// FIXME: We used to assert here, but it seems that in the nexus
			// in The Dig, this can happen, at least with old savegames, and
			// it's safe to continue...
			debug(0, "getObjectXYPos: Can't find object %d", object);
			return;
		}
		imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), ptr);
		assert(imhd);
		if (_game.version == 8) {
			switch (FROM_LE_32(imhd->v8.version)) {
			case 800:
				x = od.x_pos + (int32)READ_LE_UINT32((const byte *)imhd + 8 * state + 0x44);
				y = od.y_pos + (int32)READ_LE_UINT32((const byte *)imhd + 8 * state + 0x48);
				break;
			case 801:
				x = od.x_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].x);
				y = od.y_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].y);
				break;
			default:
				error("Unsupported image header version %d\n", FROM_LE_32(imhd->v8.version));
			}
		} else if (_game.version == 7) {
			x = od.x_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].x);
			y = od.y_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].y);
		} else {
			x = od.x_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].x);
			y = od.y_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].y);
		}
	} else {
		x = od.walk_x;
		y = od.walk_y;
	}
	if (_game.version == 8)
		dir = fromSimpleDir(1, od.actordir);
	else
		dir = oldDirToNewDir(od.actordir & 3);
}

static int getDist(int x, int y, int x2, int y2) {
	int a = ABS(y - y2);
	int b = ABS(x - x2);
	return MAX(a, b);
}

int ScummEngine::getObjActToObjActDist(int a, int b) {
	int x, y, x2, y2;
	Actor *acta = NULL;
	Actor *actb = NULL;

	if (a < _numActors)
		acta = derefActorSafe(a, "getObjActToObjActDist");

	if (b < _numActors)
		actb = derefActorSafe(b, "getObjActToObjActDist(2)");

	if (acta && actb && acta->getRoom() == actb->getRoom() && acta->getRoom() && !acta->isInCurrentRoom())
		return 0;

	if (getObjectOrActorXY(a, x, y) == -1)
		return 0xFF;

	if (getObjectOrActorXY(b, x2, y2) == -1)
		return 0xFF;

	// Perform adjustXYToBeInBox() *only* if the first item is an
	// actor and the second is an object. This used to not check
	// whether the second item is a non-actor, which caused bug
	// #853874).
	if (acta && !actb) {
		AdjustBoxResult r = acta->adjustXYToBeInBox(x2, y2);
		x2 = r.x;
		y2 = r.y;
	}

	// Now compute the distance between the two points
	if (_game.version <= 2) {
		// For V1/V2 games, distances are measured in the original "character"
		// based coordinate system, instead of pixels. Otherwise various scripts
		// will break. See bugs #853874, #774529
		x /= 8;
		y /= 2;
		x2 /= 8;
		y2 /= 2;
	}

	return getDist(x, y, x2, y2);
}

int ScummEngine::findObject(int x, int y) {
	int i, b;
	byte a;
	const int mask = (_game.version <= 2) ? 0x8 : 0xF;

	for (i = 1; i < _numLocalObjects; i++) {
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;

		if (_game.version == 0) {
			if (_objs[i].flags == 0 && _objs[i].state & 0x2)
				continue;
		} else {
			if (_game.version <= 2 && _objs[i].state & 0x2)
				continue;
		}

		b = i;
		do {
			a = _objs[b].parentstate;
			b = _objs[b].parent;
			if (b == 0) {
#ifndef DISABLE_HE
				if (_game.heversion >= 70) {
					if (((ScummEngine_v70he *)this)->_wiz->polygonHit(_objs[i].obj_nr, x, y))
						return _objs[i].obj_nr;
				}
#endif
				if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
				    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					return _objs[i].obj_nr;
				break;
			}
		} while ((_objs[b].state & mask) == a);
	}

	return 0;
}

void ScummEngine::drawRoomObject(int i, int arg) {
	ObjectData *od;
	byte a;
	const int mask = (_game.version <= 2) ? 0x8 : 0xF;

	od = &_objs[i];
	if ((i < 1) || (od->obj_nr < 1) || !od->state)
		return;

	do {
		a = od->parentstate;
		if (!od->parent) {
			if (_game.version <= 6 || od->fl_object_index == 0)
				drawObject(i, arg);
			break;
		}
		od = &_objs[od->parent];
	} while ((od->state & mask) == a);
}

void ScummEngine::drawRoomObjects(int arg) {
	int i;
	const int mask = (_game.version <= 2) ? 0x8 : 0xF;

	if (_game.heversion >= 60) {
		// In HE games, normal objects are drawn, followed by FlObjects.
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask) && _objs[i].fl_object_index == 0)
				drawRoomObject(i, arg);
		}
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask) && _objs[i].fl_object_index != 0)
				drawRoomObject(i, arg);
		}
	} else if (_game.id == GID_SAMNMAX) {
		// In Sam & Max, objects are drawn in reverse order.
		for (i = 1; i < _numLocalObjects; i++)
			if (_objs[i].obj_nr > 0)
				drawRoomObject(i, arg);
	} else {
		for (i = (_numLocalObjects-1); i > 0; i--)
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask)) {
				drawRoomObject(i, arg);
			}
	}
}

void ScummEngine::drawObject(int obj, int arg) {
	if (_skipDrawObject)
		return;

	ObjectData &od = _objs[obj];
	int height, width;
	const byte *ptr;
	int x, a, numstrip;
	int tmp;

	if (_bgNeedsRedraw)
		arg = 0;

	if (od.obj_nr == 0)
		return;

	checkRange(_numGlobalObjects - 1, 0, od.obj_nr, "Object %d out of range in drawObject");

	const int xpos = od.x_pos / 8;
	const int ypos = od.y_pos;

	width = od.width / 8;
	height = od.height &= 0xFFFFFFF8;	// Mask out last 3 bits

	// Short circuit for objects which aren't visible at all.
	if (width == 0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;

	ptr = getOBIMFromObjectData(od);

	if (_game.features & GF_OLD_BUNDLE)
		ptr += 0;
	else
		ptr = getObjectImage(ptr, getState(od.obj_nr));

	if (!ptr)
		return;

	x = 0xFFFF;

	for (a = numstrip = 0; a < width; a++) {
		tmp = xpos + a;
		if (tmp < _screenStartStrip || _screenEndStrip < tmp)
			continue;
		if (arg > 0 && _screenStartStrip + arg <= tmp)
			continue;
		if (arg < 0 && tmp <= _screenEndStrip + arg)
			continue;
		setGfxUsageBit(tmp, USAGE_BIT_DIRTY);
		if (tmp < x)
			x = tmp;
		numstrip++;
	}

	if (numstrip != 0) {
		byte flags = od.flags | Gdi::dbObjectMode;

		// Sam & Max needs this to fix object-layering problems with
		// the inventory and conversation icons.
		if ((_game.id == GID_SAMNMAX && getClass(od.obj_nr, kObjectClassIgnoreBoxes)) ||
		    (_game.id == GID_FT && getClass(od.obj_nr, kObjectClassPlayer)))
			flags |= Gdi::dbDrawMaskOnAll;

		if (_game.heversion >= 70 && findResource(MKID_BE('SMAP'), ptr) == NULL)
			gdi.drawBMAPObject(ptr, &virtscr[0], obj, od.x_pos, od.y_pos, od.width, od.height);
		else
			gdi.drawBitmap(ptr, &virtscr[0], x, ypos, width * 8, height, x - xpos, numstrip, flags);
	}
}

void ScummEngine::clearRoomObjects() {
	int i;

	if (_game.features & GF_SMALL_HEADER) {
		for (i = 0; i < _numLocalObjects; i++) {
			_objs[i].obj_nr = 0;
		}
	} else {
		storeFlObject(-1);

		for (i = 0; i < _numLocalObjects; i++) {
			if (_objs[i].obj_nr < 1)	// Optimise codepath
				continue;

			// Nuke all non-flObjects (flObjects are nuked in script.cpp)
			if (_objs[i].fl_object_index == 0) {
				_objs[i].obj_nr = 0;
			} else {
				// Nuke all unlocked flObjects
				if (!res.isLocked(rtFlObject, _objs[i].fl_object_index)) {
					res.nukeResource(rtFlObject, _objs[i].fl_object_index);
					_objs[i].obj_nr = 0;
					_objs[i].fl_object_index = 0;
				} else if (_game.heversion >= 70) {
					storeFlObject(i);
					_objs[i].obj_nr = 0;
					_objs[i].fl_object_index = 0;
				}
			}
		}
	}
}

void ScummEngine::storeFlObject(int slot) {
	if (slot == -1) {
		_numStoredFlObjects = 0;
	} else {
		memcpy(&_storedFlObjects[_numStoredFlObjects], &_objs[slot], sizeof(_objs[slot]));
		_numStoredFlObjects++;
		if (_numStoredFlObjects > 100)
			error("Too many flobjects saved on room transition.");
	}
}

void ScummEngine::restoreFlObjects() {
	if (!_numStoredFlObjects)
		return;

	int i, slot;

	for (i = 0; i < _numStoredFlObjects; i++) {
		slot = findLocalObjectSlot();
		memcpy(&_objs[slot], &_storedFlObjects[i], sizeof(_objs[slot]));
	}

	_numStoredFlObjects = 0;
}

void ScummEngine::resetRoomObjects() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room, *searchptr, *rootptr;
	const CodeHeader *cdhd;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_game.version == 8)
		searchptr = rootptr = getResourceAddress(rtRoomScripts, _roomResource);
	else
		searchptr = rootptr = room;
	assert(searchptr);

	// Load in new room objects
	ResourceIterator	obcds(searchptr, false);
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = obcds.findNext(MKID_BE('OBCD'));
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - rootptr;
		cdhd = (const CodeHeader *)findResourceData(MKID_BE('CDHD'), ptr);

		if (_game.version >= 7)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));
		else if (_game.version == 6)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));
		else
			od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			ptr = findResource(MKID_BE('VERB'), ptr);
			dumpResource(buf, od->obj_nr, ptr);
		}

	}

	searchptr = room;
	ResourceIterator	obims(room, false);
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = obims.findNext(MKID_BE('OBIM'));
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		obim_id = getObjectIdFromOBIM(ptr);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index)
			resetRoomObject(&_objs[i], room);
	}

	CHECK_HEAP
}

void ScummEngine_v3old::resetRoomObjects() {
	int i;
	ObjectData *od;
	const byte *room, *ptr;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_game.version <= 2)
		ptr = room + 28;
	else
		ptr = room + 29;

	// Default pointer of objects without image, in C64 verison of Maniac Mansion
	int defaultPtr = READ_LE_UINT16(ptr + 2 * _numObjectsInRoom);

	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		if (_game.version == 0 && READ_LE_UINT16(ptr) == defaultPtr)
			od->OBIMoffset = 0;
		else
			od->OBIMoffset = READ_LE_UINT16(ptr);

		od->OBCDoffset = READ_LE_UINT16(ptr + 2 * _numObjectsInRoom);
		resetRoomObject(od, room);

		ptr += 2;

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, room + od->OBCDoffset);
		}
	}

	CHECK_HEAP
}

void ScummEngine_v4::resetRoomObjects() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	ResourceIterator	obcds(room, true);
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = obcds.findNext(MKID_BE('OBCD'));
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - room;
		od->obj_nr = READ_LE_UINT16(ptr + 6);
		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		}
	}

	ResourceIterator	obims(room, true);
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = obims.findNext(MKID_BE('OBIM'));
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		obim_id = READ_LE_UINT16(ptr + 6);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index) {
			resetRoomObject(&_objs[i], room);
		}
	}

	CHECK_HEAP
}

void ScummEngine_c64::resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	assert(room);
	const byte *ptr = room + od->OBCDoffset;
	ptr -= 2;

	od->obj_nr = *(ptr + 6);
	od->flags = *(ptr + 7);

	od->x_pos = *(ptr + 8) * 8;
	od->y_pos = ((*(ptr + 9)) & 0x7F) * 8;

	od->parentstate = (*(ptr + 9) & 0x80) ? 1 : 0;
	od->parentstate *= 8;

	od->width = *(ptr + 10) * 8;

	od->parent = *(ptr + 11);

	od->walk_x = *(ptr + 12) * 8;
	od->walk_y = (*(ptr + 13) & 0x1f) * 8;
	od->actordir = (*(ptr + 14)) & 7;
	od->height = *(ptr + 14) & 0xf8;
}

void ScummEngine_v4::resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	assert(room);
	const byte *ptr = room + od->OBCDoffset;

	if (_game.features & GF_OLD_BUNDLE)
		ptr -= 2;

	od->obj_nr = READ_LE_UINT16(ptr + 6);

	od->x_pos = *(ptr + 9) * 8;
	od->y_pos = ((*(ptr + 10)) & 0x7F) * 8;

	od->parentstate = (*(ptr + 10) & 0x80) ? 1 : 0;
	if (_game.version <= 2)
		od->parentstate *= 8;

	od->width = *(ptr + 11) * 8;

	od->parent = *(ptr + 12);

	if (_game.version <= 2) {
		od->walk_x = *(ptr + 13) * 8;
		od->walk_y = (*(ptr + 14) & 0x1f) * 8;
		od->actordir = (*(ptr + 15)) & 7;
		od->height = *(ptr + 15) & 0xf8;
	} else {
		od->walk_x = READ_LE_UINT16(ptr + 13);
		od->walk_y = READ_LE_UINT16(ptr + 15);
		od->actordir = (*(ptr + 17)) & 7;
		od->height = *(ptr + 17) & 0xf8;
	}
}

void ScummEngine::resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	const CodeHeader *cdhd = NULL;
	const ImageHeader *imhd = NULL;

	assert(room);

	if (searchptr == NULL) {
		if (_game.version == 8)
			searchptr = getResourceAddress(rtRoomScripts, _roomResource);
		else
			searchptr = room;
	}

	cdhd = (const CodeHeader *)findResourceData(MKID_BE('CDHD'), searchptr + od->OBCDoffset);
	if (cdhd == NULL)
		error("Room %d missing CDHD blocks(s)", _roomResource);
	if (od->OBIMoffset)
		imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), room + od->OBIMoffset);

	od->flags = Gdi::dbAllowMaskOr;

	if (_game.version == 8) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		od->x_pos = (int)READ_LE_UINT32(&imhd->v8.x_pos);
		od->y_pos = (int)READ_LE_UINT32(&imhd->v8.y_pos);
		od->width = (uint)READ_LE_UINT32(&imhd->v8.width);
		od->height = (uint)READ_LE_UINT32(&imhd->v8.height);
		// HACK: This is done since an angle doesn't fit into a byte (360 > 256)
		od->actordir = toSimpleDir(1, READ_LE_UINT32(&imhd->v8.actordir));
		if (FROM_LE_32(imhd->v8.version) == 801)
			od->flags = ((((byte)READ_LE_UINT32(&imhd->v8.flags)) & 16) == 0) ? Gdi::dbAllowMaskOr : 0;

	} else if (_game.version == 7) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		od->x_pos = READ_LE_UINT16(&imhd->v7.x_pos);
		od->y_pos = READ_LE_UINT16(&imhd->v7.y_pos);
		od->width = READ_LE_UINT16(&imhd->v7.width);
		od->height = READ_LE_UINT16(&imhd->v7.height);
		od->actordir = (byte)READ_LE_UINT16(&imhd->v7.actordir);

	} else if (_game.version == 6) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));

		od->width = READ_LE_UINT16(&cdhd->v6.w);
		od->height = READ_LE_UINT16(&cdhd->v6.h);
		od->x_pos = ((int16)READ_LE_UINT16(&cdhd->v6.x));
		od->y_pos = ((int16)READ_LE_UINT16(&cdhd->v6.y));
		if (cdhd->v6.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v6.flags & 0xF);
		}
		od->parent = cdhd->v6.parent;
		od->actordir = cdhd->v6.actordir;

		if (_game.heversion >= 60 && imhd)
			od->flags = ((imhd->old.flags & 1) != 0) ? Gdi::dbAllowMaskOr : 0;

	} else {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		od->width = cdhd->v5.w * 8;
		od->height = cdhd->v5.h * 8;
		od->x_pos = cdhd->v5.x * 8;
		od->y_pos = cdhd->v5.y * 8;
		if (cdhd->v5.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v5.flags & 0xF);
		}
		od->parent = cdhd->v5.parent;
		od->walk_x = READ_LE_UINT16(&cdhd->v5.walk_x);
		od->walk_y = READ_LE_UINT16(&cdhd->v5.walk_y);
		od->actordir = cdhd->v5.actordir;
	}

	od->fl_object_index = 0;
}

void ScummEngine::updateObjectStates() {
	int i;
	ObjectData *od = &_objs[1];
	for (i = 1; i < _numLocalObjects; i++, od++) {
		if (od->obj_nr > 0)
			od->state = getState(od->obj_nr);
	}
}

void ScummEngine::processDrawQue() {
	int i, j;
	for (i = 0; i < _drawObjectQueNr; i++) {
		j = _drawObjectQue[i];
		if (j)
			drawObject(j, 0);
	}
	_drawObjectQueNr = 0;
}

void ScummEngine::addObjectToDrawQue(int object) {
	if ((unsigned int)_drawObjectQueNr >= ARRAYSIZE(_drawObjectQue))
		error("Draw Object Que overflow");
	_drawObjectQue[_drawObjectQueNr++] = object;
}

void ScummEngine::removeObjectFromDrawQue(int object) {
	if (_drawObjectQueNr <= 0)
		return;

	int i;
	for (i = 0; i < _drawObjectQueNr; i++) {
		if (_drawObjectQue[i] == object)
			_drawObjectQue[i] = 0;
	}
}

void ScummEngine::clearDrawObjectQueue() {
	_drawObjectQueNr = 0;
}

void ScummEngine::clearDrawQueues() {
	clearDrawObjectQueue();
}

void ScummEngine_v6::clearDrawQueues() {
	ScummEngine::clearDrawQueues();

	_blastObjectQueuePos = 0;
}

#ifndef DISABLE_HE
void ScummEngine_v70he::clearDrawQueues() {
	ScummEngine_v6::clearDrawQueues();

	_wiz->polygonClear();
}

void ScummEngine_v80he::clearDrawQueues() {
	ScummEngine_v70he::clearDrawQueues();

	_wiz->clearWizBuffer();
}
#endif

void ScummEngine::clearOwnerOf(int obj) {
	int i, j;
	uint16 *a;

	stopObjectScript(obj);

	if (getOwner(obj) == OF_OWNER_ROOM) {
		i = 0;
		do {
			if (_objs[i].obj_nr == obj) {
				if (!_objs[i].fl_object_index)
					return;
				res.nukeResource(rtFlObject, _objs[i].fl_object_index);
				_objs[i].obj_nr = 0;
				_objs[i].fl_object_index = 0;
			}
		} while (++i < _numLocalObjects);
		return;
	}

	for (i = 0; i < _numInventory; i++) {
		if (_inventory[i] == obj) {
			j = whereIsObject(obj);
			if (j == WIO_INVENTORY) {
				res.nukeResource(rtInventory, i);
				_inventory[i] = 0;
			}
			a = _inventory;
			for (i = 0; i < _numInventory - 1; i++, a++) {
				if (!a[0] && a[1]) {
					a[0] = a[1];
					a[1] = 0;
					res.address[rtInventory][i] = res.address[rtInventory][i + 1];
					res.address[rtInventory][i + 1] = NULL;
				}
			}
			return;
		}
	}
}

/**
 * Mark the rectangle covered by the given object as dirty, thus eventually
 * ensuring a redraw of that area. This function is typically invoked when an
 * object gets removed from the current room, or when its state changed.
 */
void ScummEngine::markObjectRectAsDirty(int obj) {
	int i, strip;

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr == (uint16)obj) {
			if (_objs[i].width != 0) {
				const int minStrip = MAX(_screenStartStrip, _objs[i].x_pos / 8);
				const int maxStrip = MIN(_screenEndStrip+1, _objs[i].x_pos / 8 + _objs[i].width / 8);
				for (strip = minStrip; strip < maxStrip; strip++) {
					setGfxUsageBit(strip, USAGE_BIT_DIRTY);
				}
			}
			_bgNeedsRedraw = true;
			return;
		}
	}
}

const byte *ScummEngine::getObjOrActorName(int obj) {
	byte *objptr;
	int i;

	if (obj < _numActors && _game.version >= 1)
		return derefActor(obj, "getObjOrActorName")->getActorName();

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == obj) {
			debug(5, "Found new name for object %d at _newNames[%d]", obj, i);
			return getResourceAddress(rtObjectName, i);
		}
	}

	objptr = getOBCDFromObject(obj);
	if (objptr == NULL)
		return NULL;

	if (_game.features & GF_SMALL_HEADER) {
		byte offset = 0;

		if (_game.version == 0)
			offset = *(objptr + 13);
		else if (_game.version <= 2)
			offset = *(objptr + 14);
		else if (_game.features & GF_OLD_BUNDLE)
			offset = *(objptr + 16);
		else
			offset = *(objptr + 18);

		return (objptr + offset);
	}

	return findResourceData(MKID_BE('OBNA'), objptr);
}

void ScummEngine::setObjectName(int obj) {
	int i;

	if (obj < _numActors)
		error("Can't set actor %d name with new-name-of", obj);

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == obj) {
			res.nukeResource(rtObjectName, i);
			_newNames[i] = 0;
			break;
		}
	}

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == 0) {
			loadPtrToResource(rtObjectName, i, NULL);
			_newNames[i] = obj;
			runInventoryScript(0);
			return;
		}
	}

	error("New name of %d overflows name table (max = %d)", obj, _numNewNames);
}

uint32 ScummEngine::getOBCDOffs(int object) const {
	int i;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM)
		return 0;
	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index != 0)
				return 8;
			return _objs[i].OBCDoffset;
		}
	}
	return 0;
}

byte *ScummEngine::getOBCDFromObject(int obj) {
	int i;
	byte *ptr;

	if (_objectOwnerTable[obj] != OF_OWNER_ROOM) {
		for (i = 0; i < _numInventory; i++) {
			if (_inventory[i] == obj)
				return getResourceAddress(rtInventory, i);
		}
	} else {
		for (i = (_numLocalObjects-1); i > 0; --i) {
			if (_objs[i].obj_nr == obj) {
				if (_objs[i].fl_object_index) {
					assert(_objs[i].OBCDoffset == 8);
					ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index);
				} else if (_game.version == 8)
					ptr = getResourceAddress(rtRoomScripts, _roomResource);
				else
					ptr = getResourceAddress(rtRoom, _roomResource);
				assert(ptr);
				return ptr + _objs[i].OBCDoffset;
			}
		}
	}
	return 0;
}

const byte *ScummEngine::getOBIMFromObjectData(const ObjectData &od) {
	const byte *ptr;

	// For objects without image in C64 version of Maniac Mansion
	if (_game.version == 0 && od.OBIMoffset == 0)
		return NULL;

	if (od.fl_object_index) {
		ptr = getResourceAddress(rtFlObject, od.fl_object_index);
		ptr = findResource(MKID_BE('OBIM'), ptr);
	} else {
		ptr = getResourceAddress(rtRoom, _roomResource);
		if (ptr)
			ptr += od.OBIMoffset;
	}
	return ptr;
}

static const uint32 IMxx_tags[] = {
	MKID_BE('IM00'),
	MKID_BE('IM01'),
	MKID_BE('IM02'),
	MKID_BE('IM03'),
	MKID_BE('IM04'),
	MKID_BE('IM05'),
	MKID_BE('IM06'),
	MKID_BE('IM07'),
	MKID_BE('IM08'),
	MKID_BE('IM09'),
	MKID_BE('IM0A'),
	MKID_BE('IM0B'),
	MKID_BE('IM0C'),
	MKID_BE('IM0D'),
	MKID_BE('IM0E'),
	MKID_BE('IM0F'),
	MKID_BE('IM10')
};

const byte *ScummEngine::getObjectImage(const byte *ptr, int state) {
	assert(ptr);
	if (_game.features & GF_OLD_BUNDLE)
		ptr += 0;
	else if (_game.features & GF_SMALL_HEADER) {
		ptr += 8;
	} else if (_game.version == 8) {
		// The OBIM contains an IMAG, which in turn contains a WRAP, which contains
		// an OFFS chunk and multiple BOMP/SMAP chunks. To find the right BOMP/SMAP,
		// we use the offsets in the OFFS chunk,
		ptr = findResource(MKID_BE('IMAG'), ptr);
		if (!ptr)
			return 0;

		ptr = findResource(MKID_BE('WRAP'), ptr);
		if (!ptr)
			return 0;

		ptr = findResource(MKID_BE('OFFS'), ptr);
		if (!ptr)
			return 0;

		// Get the address of the specified SMAP (corresponding to IMxx)
		ptr += READ_LE_UINT32(ptr + 4 + 4*state);
	} else {
		ptr = findResource(IMxx_tags[state], ptr);
	}

	return ptr;
}

int ScummEngine::getObjectImageCount(int object) {
	const byte *ptr;
	const ImageHeader *imhd;
	int objnum;

	objnum = getObjectIndex(object);
	if (objnum == -1)
		return 0;

	ptr = getOBIMFromObjectData(_objs[objnum]);
	imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), ptr);
	if (!imhd)
		return 0;

	if (_game.version == 8) {
		return (READ_LE_UINT32(&imhd->v8.image_count));
	} else if (_game.version == 7) {
		return(READ_LE_UINT16(&imhd->v7.image_count));
	} else {
		return (READ_LE_UINT16(&imhd->old.image_count));
	}
}

#ifndef DISABLE_SCUMM_7_8
int ScummEngine_v8::getObjectIdFromOBIM(const byte *obim) {
	// In V8, IMHD has no obj_id, but rather a name string. We map the name
	// back to an object id using a table derived from the DOBJ resource.
	const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), obim);
	ObjectNameId *found = (ObjectNameId *)bsearch(imhd->v8.name, _objectIDMap, _objectIDMapSize,
					sizeof(ObjectNameId), (int (*)(const void*, const void*))strcmp);
	assert(found);
	return found->id;
}

int ScummEngine_v7::getObjectIdFromOBIM(const byte *obim) {
	const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), obim);
	return READ_LE_UINT16(&imhd->v7.obj_id);
}
#endif

int ScummEngine::getObjectIdFromOBIM(const byte *obim) {
	if (_game.features & GF_SMALL_HEADER)
		return READ_LE_UINT16(obim + 6);

	const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), obim);
	return READ_LE_UINT16(&imhd->old.obj_id);
}

void ScummEngine::addObjectToInventory(uint obj, uint room) {
	int idx, slot;
	uint32 size;
	const byte *ptr;
	byte *dst;
	FindObjectInRoom foir;

	debug(1, "Adding object %d from room %d into inventory", obj, room);

	CHECK_HEAP
	if (whereIsObject(obj) == WIO_FLOBJECT) {
		idx = getObjectIndex(obj);
		assert(idx >= 0);
		ptr = getResourceAddress(rtFlObject, _objs[idx].fl_object_index) + 8;
		size = READ_BE_UINT32(ptr + 4);
	} else {
		findObjectInRoom(&foir, foCodeHeader, obj, room);
		if (_game.features & GF_OLD_BUNDLE)
			size = READ_LE_UINT16(foir.obcd);
		else if (_game.features & GF_SMALL_HEADER)
			size = READ_LE_UINT32(foir.obcd);
		else
			size = READ_BE_UINT32(foir.obcd + 4);
		ptr = foir.obcd;
	}

	slot = getInventorySlot();
	_inventory[slot] = obj;
	dst = res.createResource(rtInventory, slot, size);
	assert(dst);
	memcpy(dst, ptr, size);

	CHECK_HEAP
}

void ScummEngine::findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint id, uint room) {

	const CodeHeader *cdhd;
	int i, numobj;
	const byte *roomptr, *obcdptr, *obimptr, *searchptr;
	int id2;
	int obim_id;

	id2 = getObjectIndex(id);
	if (findWhat & foCheckAlreadyLoaded && id2 != -1) {
		assert(_game.version >= 6);
		if (findWhat & foCodeHeader) {
			fo->obcd = obcdptr = getOBCDFromObject(id);
			assert(obcdptr);
			fo->cdhd = (const CodeHeader *)findResourceData(MKID_BE('CDHD'), obcdptr);
		}
		if (findWhat & foImageHeader) {
			fo->obim = obimptr = getOBIMFromObjectData(_objs[id2]);
			assert(obimptr);
		}
		return;
	}

	fo->roomptr = roomptr = getResourceAddress(rtRoom, room);
	if (!roomptr)
		error("findObjectInRoom: failed getting roomptr to %d", room);

	if (_game.features & GF_OLD_BUNDLE) {
		numobj = roomptr[20];
	} else {
		const RoomHeader *roomhdr = (const RoomHeader *)findResourceData(MKID_BE('RMHD'), roomptr);

		if (_game.version == 8)
			numobj = READ_LE_UINT32(&(roomhdr->v8.numObjects));
		else if (_game.version == 7)
			numobj = READ_LE_UINT16(&(roomhdr->v7.numObjects));
		else
			numobj = READ_LE_UINT16(&(roomhdr->old.numObjects));
	}

	if (numobj == 0)
		error("findObjectInRoom: No object found in room %d", room);
	if (numobj > _numLocalObjects)
		error("findObjectInRoom: More (%d) than %d objects in room %d", numobj, _numLocalObjects, room);

	if (_game.features & GF_OLD_BUNDLE) {
		if (_game.version <= 2)
			searchptr = roomptr + 28;
		else
			searchptr = roomptr + 29;

		for (i = 0; i < numobj; i++) {
			obimptr = roomptr + READ_LE_UINT16(searchptr);
			obcdptr = roomptr + READ_LE_UINT16(searchptr + 2 * numobj);
			id2 = READ_LE_UINT16(obcdptr + 4);

			if (id2 == (uint16)id) {
				if (findWhat & foCodeHeader) {
					fo->obcd = obcdptr;
					fo->cdhd = (const CodeHeader *)(obcdptr + 10);	// TODO - FIXME
				}
				if (findWhat & foImageHeader) {
					fo->obim = obimptr;
				}
				break;
			}
			searchptr += 2;
		}
		return;
	}

	if (findWhat & foCodeHeader) {
		if (_game.version == 8)
			searchptr = getResourceAddress(rtRoomScripts, room);
		else
			searchptr = roomptr;
		assert(searchptr);
		ResourceIterator	obcds(searchptr, (_game.features & GF_SMALL_HEADER) != 0);
		for (i = 0; i < numobj; i++) {
			obcdptr = obcds.findNext(MKID_BE('OBCD'));
			if (obcdptr == NULL)
				error("findObjectInRoom: Not enough code blocks in room %d", room);
			cdhd = (const CodeHeader *)findResourceData(MKID_BE('CDHD'), obcdptr);

			if (_game.features & GF_SMALL_HEADER)
				id2 = READ_LE_UINT16(obcdptr + 6);
			else if (_game.version >= 7)
				id2 = READ_LE_UINT16(&(cdhd->v7.obj_id));
			else if (_game.version == 6)
				id2 = READ_LE_UINT16(&(cdhd->v6.obj_id));
			else
				id2 = READ_LE_UINT16(&(cdhd->v5.obj_id));

			if (id2 == (uint16)id) {
				fo->obcd = obcdptr;
				fo->cdhd = cdhd;
				break;
			}
		}
		if (i == numobj)
			error("findObjectInRoom: Object %d not found in room %d", id, room);
	}

	roomptr = fo->roomptr;
	if (findWhat & foImageHeader) {
		ResourceIterator	obims(roomptr, (_game.features & GF_SMALL_HEADER) != 0);
		for (i = 0; i < numobj; i++) {
			obimptr = obims.findNext(MKID_BE('OBIM'));
			if (obimptr == NULL)
				error("findObjectInRoom: Not enough image blocks in room %d", room);
			obim_id = getObjectIdFromOBIM(obimptr);

			if (obim_id == (uint16)id) {
				fo->obim = obimptr;
				break;
			}
		}
		if (i == numobj)
			error("findObjectInRoom: Object %d image not found in room %d", id, room);
	}
}

int ScummEngine::getInventorySlot() {
	int i;
	for (i = 0; i < _numInventory; i++) {
		if (_inventory[i] == 0)
			return i;
	}
	error("Inventory full, %d max items", _numInventory);
	return -1;
}

void ScummEngine::setOwnerOf(int obj, int owner) {
	ScriptSlot *ss;

	// In Sam & Max this is necessary, or you won't get your stuff back
	// from the Lost and Found tent after riding the Cone of Tragedy. But
	// it probably applies to all V6+ games. See bugs #493153 and #907113.
	// FT disassembly is checked, behaviour is correct. [sev]

	int arg = (_game.version >= 6) ? obj : 0;

	if (owner == 0) {
		clearOwnerOf(obj);
		ss = &vm.slot[_currentScript];
		if (ss->where == WIO_INVENTORY && _inventory[ss->number] == obj) {
			putOwner(obj, 0);
			runInventoryScript(arg);
			stopObjectCode();
			return;
		}
	}

	putOwner(obj, owner);
	runInventoryScript(arg);
}

int ScummEngine::getObjX(int obj) {
	if (obj < _numActors) {
		if (obj < 1)
			return 0;									/* fix for indy4's map */
		return derefActor(obj, "getObjX")->_pos.x;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return x;
	}
}

int ScummEngine::getObjY(int obj) {
	if (obj < _numActors) {
		if (obj < 1)
			return 0;									/* fix for indy4's map */
		return derefActor(obj, "getObjY")->_pos.y;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return y;
	}
}

int ScummEngine::getObjOldDir(int obj) {
	return newDirToOldDir(getObjNewDir(obj));
}

int ScummEngine::getObjNewDir(int obj) {
	int dir;
	if (obj < _numActors) {
		dir = derefActor(obj, "getObjNewDir")->getFacing();
	} else {
		int x, y;
		getObjectXYPos(obj, x, y, dir);
	}
	return dir;
}

int ScummEngine::findInventory(int owner, int idx) {
	int count = 1, i, obj;
	for (i = 0; i < _numInventory; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner && count++ == idx)
			return obj;
	}
	return 0;
}

int ScummEngine::getInventoryCount(int owner) {
	int i, obj;
	int count = 0;
	for (i = 0; i < _numInventory; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner)
			count++;
	}
	return count;
}

void ScummEngine::setObjectState(int obj, int state, int x, int y) {
	int i;

	i = getObjectIndex(obj);
	if (i == -1) {
		debug(0, "setObjectState: no such object %d", obj);
		return;
	}

	if (x != -1 && x != 0x7FFFFFFF) {
		_objs[i].x_pos = x * 8;
		_objs[i].y_pos = y * 8;
	}

	addObjectToDrawQue(i);
	if (_game.version >= 7) {
		int imagecount;
		if (state == 0xFF) {
			state = getState(obj);
			imagecount = getObjectImageCount(obj);

			if (state < imagecount)
				state++;
			else
				state = 1;
		}

		if (state == 0xFE)
			state = _rnd.getRandomNumber(getObjectImageCount(obj));
	}
	putState(obj, state);
}

int ScummEngine::getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f) {
	int i, j;
	int x, y;
	int x2, y2;

	j = i = 0xFF;

	if (is_obj_1) {
		if (getObjectOrActorXY(b, x, y) == -1)
			return -1;
		if (b < _numActors)
			i = derefActor(b, "getDistanceBetween_is_obj_1")->_scalex;
	} else {
		x = b;
		y = c;
	}

	if (is_obj_2) {
		if (getObjectOrActorXY(e, x2, y2) == -1)
			return -1;
		if (e < _numActors)
			j = derefActor(e, "getDistanceBetween_is_obj_2")->_scalex;
	} else {
		x2 = e;
		y2 = f;
	}

	return getDist(x, y, x2, y2) * 0xFF / ((i + j) / 2);
}

void ScummEngine::nukeFlObjects(int min, int max) {
	ObjectData *od;
	int i;

	debug(0, "nukeFlObjects(%d,%d)", min, max);

	for (i = (_numLocalObjects-1), od = _objs; --i >= 0; od++)
		if (od->fl_object_index && od->obj_nr >= min && od->obj_nr <= max) {
			res.nukeResource(rtFlObject, od->fl_object_index);
			od->obj_nr = 0;
			od->fl_object_index = 0;
		}
}

void ScummEngine_v6::enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
								int objectHeight, int scaleX, int scaleY, int image, int mode) {
	BlastObject *eo;

	if (_blastObjectQueuePos >= (int)ARRAYSIZE(_blastObjectQueue)) {
		error("enqueueObject: overflow");
	}

	int idx = getObjectIndex(objectNumber);
	assert(idx >= 0);

	eo = &_blastObjectQueue[_blastObjectQueuePos++];
	eo->number = objectNumber;
	eo->rect.left = objectX;
	eo->rect.top = objectY + _screenTop;
	if (objectWidth == 0) {
		eo->rect.right = eo->rect.left + _objs[idx].width;
	} else {
		eo->rect.right = eo->rect.left + objectWidth;
	}
	if (objectHeight == 0) {
		eo->rect.bottom = eo->rect.top + _objs[idx].height;
	} else {
		eo->rect.bottom = eo->rect.top + objectHeight;
	}

	eo->scaleX = scaleX;
	eo->scaleY = scaleY;
	eo->image = image;

	eo->mode = mode;
}

void ScummEngine_v6::drawBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		drawBlastObject(eo);
	}
}

void ScummEngine_v6::drawBlastObject(BlastObject *eo) {
	VirtScreen *vs;
	const byte *bomp, *ptr;
	int objnum;
	BompDrawData bdd;

	vs = &virtscr[0];

	checkRange(_numGlobalObjects - 1, 30, eo->number, "Illegal Blast object %d");

	objnum = getObjectIndex(eo->number);
	if (objnum == -1)
		error("drawBlastObject: getObjectIndex on BlastObject %d failed", eo->number);

	ptr = getOBIMFromObjectData(_objs[objnum]);
	if (!ptr)
		error("BlastObject object %d image not found", eo->number);

	const byte *img = getObjectImage(ptr, eo->image);
	if (_game.version == 8) {
		assert(img);
		bomp = img + 8;
	} else {
		if (!img)
			img = getObjectImage(ptr, 1);	// Backward compatibility with samnmax blast objects
		assert(img);
		bomp = findResourceData(MKID_BE('BOMP'), img);
	}

	if (!bomp)
		error("object %d is not a blast object", eo->number);

	if (_game.version == 8) {
		bdd.srcwidth = READ_LE_UINT32(&((const BompHeader *)bomp)->v8.width);
		bdd.srcheight = READ_LE_UINT32(&((const BompHeader *)bomp)->v8.height);
	} else {
		bdd.srcwidth = READ_LE_UINT16(&((const BompHeader *)bomp)->old.width);
		bdd.srcheight = READ_LE_UINT16(&((const BompHeader *)bomp)->old.height);
	}

	bdd.dst = *vs;
	bdd.dst.pixels = vs->getPixels(0, 0);
	// Skip the bomp header
	if (_game.version == 8) {
		bdd.dataptr = bomp + 8;
	} else {
		bdd.dataptr = bomp + 10;
	}
	bdd.x = eo->rect.left;
	bdd.y = eo->rect.top;
	bdd.scale_x = (byte)eo->scaleX;
	bdd.scale_y = (byte)eo->scaleY;
	bdd.maskPtr = NULL;

	if ((bdd.scale_x != 255) || (bdd.scale_y != 255)) {
		bdd.shadowMode = 0;
	} else {
		bdd.shadowMode = eo->mode;
	}
	drawBomp(bdd, false);

	markRectAsDirty(vs->number, bdd.x, bdd.x + bdd.srcwidth, bdd.y, bdd.y + bdd.srcheight);
}

void ScummEngine_v6::removeBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		removeBlastObject(eo);
	}
	_blastObjectQueuePos = 0;
}

void ScummEngine_v6::removeBlastObject(BlastObject *eo) {
	VirtScreen *vs = &virtscr[0];

	Common::Rect r;
	int left_strip, right_strip;
	int i;

	r = eo->rect;

	r.clip(Common::Rect(vs->w, vs->h));

	if (r.width() <= 0 || r.height() <= 0)
		return;

	left_strip = r.left / 8;
	right_strip = (r.right + (vs->xstart % 8)) / 8;

	if (left_strip < 0)
		left_strip = 0;
	if (right_strip > gdi._numStrips - 1)
		right_strip = gdi._numStrips - 1;
	for (i = left_strip; i <= right_strip; i++)
		gdi.resetBackground(r.top, r.bottom, i);

	markRectAsDirty(kMainVirtScreen, r, USAGE_BIT_RESTORED);
}

int ScummEngine::findLocalObjectSlot() {
	int i;

	for (i = 1; i < _numLocalObjects; i++) {
		if (!_objs[i].obj_nr) {
			memset(&_objs[i], 0, sizeof(_objs[i]));
			return i;
		}
	}

	return -1;
}

int ScummEngine::findFlObjectSlot() {
	int i;
	for (i = 1; i < _numFlObject; i++) {
		if (res.address[rtFlObject][i] == NULL)
			return i;
	}
	error("findFlObjectSlot: Out of FLObject slots");
	return -1;
}

void ScummEngine::loadFlObject(uint object, uint room) {
	FindObjectInRoom foir;
	int i, slot, objslot;
	ObjectData *od;
	byte *flob;
	uint32 obcd_size, obim_size, flob_size;
	bool isRoomLocked, isRoomScriptsLocked;

	// Don't load an already loaded object
	if (getObjectIndex(object) != -1)
		return;

	// Don't load an already stored object
	for (i = 0; i < _numStoredFlObjects; i++) {
		if (_storedFlObjects[i].obj_nr == object)
			return;
	}

	// Locate the object in the room resource
	findObjectInRoom(&foir, foImageHeader | foCodeHeader, object, room);

	// Add an entry for the new floating object in the local object table
	objslot = findLocalObjectSlot();
	if (objslot == -1)
		error("loadFlObject: Local Object Table overflow");

	od = &_objs[objslot];

	// Dump object script
	if (_dumpScripts) {
		char buf[32];
		const byte *ptr = foir.obcd;
		sprintf(buf, "roomobj-%d-", room);
		ptr = findResource(MKID_BE('VERB'), ptr);
		dumpResource(buf, object, ptr);
	}

	// Setup sizes
	obcd_size = READ_BE_UINT32(foir.obcd + 4);
	od->OBCDoffset = 8;
	od->OBIMoffset = obcd_size + 8;
	obim_size = READ_BE_UINT32(foir.obim + 4);
	flob_size = obcd_size + obim_size + 8;

	// Lock room/roomScripts for the given room. They contains the OBCD/OBIM
	// data, and a call to createResource might expire them, hence we lock them.
	isRoomLocked = res.isLocked(rtRoom, room);
	isRoomScriptsLocked = res.isLocked(rtRoomScripts, room);
	if (!isRoomLocked)
		res.lock(rtRoom, room);
	if (_game.version == 8 && !isRoomScriptsLocked)
		res.lock(rtRoomScripts, room);

	// Allocate slot & memory for floating object
	slot = findFlObjectSlot();
	flob = res.createResource(rtFlObject, slot, flob_size);
	assert(flob);

	// Copy object code + object image to floating object
	((uint32 *)flob)[0] = MKID_BE('FLOB');
	((uint32 *)flob)[1] = TO_BE_32(flob_size);

	memcpy(flob + 8, foir.obcd, obcd_size);
	memcpy(flob + 8 + obcd_size, foir.obim, obim_size);

	// Unlock room/roomScripts
	if (!isRoomLocked)
		res.unlock(rtRoom, room);
	if (_game.version == 8 && !isRoomScriptsLocked)
		res.unlock(rtRoomScripts, room);

	// Setup local object flags
	resetRoomObject(od, flob, flob);

	od->fl_object_index = slot;
}

} // End of namespace Scumm
