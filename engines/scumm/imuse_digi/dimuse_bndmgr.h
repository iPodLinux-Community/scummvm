/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef BUNDLE_MGR_H
#define BUNDLE_MGR_H

#include "common/scummsys.h"
#include "common/file.h"

namespace Scumm {

class BaseScummFile;

class BundleDirCache {
public:
	struct AudioTable {
		char filename[24];
		int32 offset;
		int32 size;
	};

	struct IndexNode {
		char filename[24];
		int32 index;
	};

private:

	struct FileDirCache {
		char fileName[20];
		AudioTable *bundleTable;
		int32 numFiles;
		bool compressedBun;
		IndexNode *indexTable;
	} _budleDirCache[4];

public:
	BundleDirCache();
	~BundleDirCache();

	int matchFile(const char *filename);
	AudioTable *getTable(int slot);
	IndexNode *getIndexTable(int slot);
	int32 getNumFiles(int slot);
	bool isCompressed(int slot);
};

class BundleMgr {

private:

	struct CompTable {
		int32 offset;
		int32 size;
		int32 codec;
	};

	BundleDirCache *_cache;
	BundleDirCache::AudioTable *_bundleTable;
	BundleDirCache::IndexNode *_indexTable;
	CompTable *_compTable;
	int _numFiles;
	int _numCompItems;
	int _curSample;
	BaseScummFile *_file;
	bool _compTableLoaded;
	int _fileBundleId;
	byte _compOutput[0x2000];
	byte *_compInput;
	int _outputSize;
	int _lastBlock;

	bool loadCompTable(int32 index);

public:

	BundleMgr(BundleDirCache *_cache);
	~BundleMgr();

	bool open(const char *filename, bool &compressed, bool errorFlag=false);
	void close();
	Common::File *getFile(const char *filename, int32 &offset, int32 &size);
	int32 decompressSampleByName(const char *name, int32 offset, int32 size, byte **comp_final, bool header_outside);
	int32 decompressSampleByIndex(int32 index, int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside);
	int32 decompressSampleByCurIndex(int32 offset, int32 size, byte **comp_final, int header_size, bool header_outside);
};

namespace BundleCodecs {

uint32 decode12BitsSample(const byte *src, byte **dst, uint32 size);
void initializeImcTables();
int32 decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 input_size);

} // End of namespace BundleCodecs

} // End of namespace Scumm

#endif
