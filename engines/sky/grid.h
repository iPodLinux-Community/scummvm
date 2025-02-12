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

#ifndef SKYGRID_H
#define SKYGRID_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "skydefs.h"

namespace Sky {

struct Compact;
class Disk;
class SkyCompact;

class Grid {
public:
	Grid(Disk *pDisk, SkyCompact *skyCompact);
	~Grid(void);

	// grid.asm routines
	void loadGrids(void);
	void removeObjectFromWalk(Compact *cpt);
	void objectToWalk(Compact *cpt);

	// function.asm
	// note that this routine does the same as objectToWalk, it just doesn't get
	// its x, y, width parameters from cpt.
	void plotGrid(uint32 x, uint32 y, uint32 width, Compact *cpt);
	// same here, it's basically the same as removeObjectFromWalk
	void removeGrid(uint32 x, uint32 y, uint32 width, Compact *cpt);
	uint8 *giveGrid(uint32 pScreen);

private:
	void objectToWalk(uint8 gridIdx, uint32 bitNum, uint32 width);
	void removeObjectFromWalk(uint8 gridIdx, uint32 bitNum, uint32 width);
	bool getGridValues(Compact *cpt, uint8 *resGrid, uint32 *resBitNum, uint32 *resWidth);
	bool getGridValues(uint32 x, uint32 y, uint32 width, Compact *cpt, uint8 *resGrid, uint32 *resBitNum, uint32 *resWidth);

	static int8 _gridConvertTable[];
	uint8 *_gameGrids[TOT_NO_GRIDS];
	Disk *_skyDisk;
	SkyCompact *_skyCompact;
};

} // End of namespace Sky

#endif //SKYGRID_H
