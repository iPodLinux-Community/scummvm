/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#ifndef __IRXBOOT_H__
#define __IRXBOOT_H__

#include "common/scummsys.h"

enum IrxFlags {
	BIOS = 0,
	SYSTEM = 1,
	USB = 2,
	HDD = 3,
	TYPEMASK = 3,

	OPTIONAL = 4,
	DEPENDANCY = 8,
	NOT_HOST = 16
};

enum IrxPurpose {
	NOTHING,
	HDD_DRIVER,
	USB_DRIVER,
	MOUSE_DRIVER,
	KBD_DRIVER,
	MASS_DRIVER
};

enum IrxLocation {
	IRX_BUFFER,
	IRX_FILE
};

enum BootDevice {
	HOST = 0,
	CDROM,
	OTHER,
	UNKNOWN
};

struct IrxFile {
	const char *name;
	int flags;
	IrxPurpose purpose;
	const char *args;
	int argSize;
};

struct IrxReference {
	IrxFile *fileRef;
	IrxLocation loc;
	char *path;
	void *buffer;
	uint32 size;
	uint32 argSize;
	const char *args;
	int errorCode;
};

BootDevice detectBootPath(const char *elfPath, char *bootPath);
int loadIrxModules(int device, const char *irxPath, IrxReference **modules);

#endif // __IRXBOOT_H__

