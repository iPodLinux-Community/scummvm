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

// Sprite management module private header file

#ifndef SAGA_SPRITE_H__
#define SAGA_SPRITE_H__

namespace Saga {

#define SPRITE_ZMAX  16
#define SPRITE_ZMASK 0x0F

#define DECODE_BUF_LEN 64000

struct SpriteInfo {
	byte *decodedBuffer;
	int width;
	int height;
	int xAlign;
	int yAlign;
};

struct SpriteList {
	int spriteListResourceId;
	int spriteCount;
	SpriteInfo *infoList;

	void freeMem() {
		int i;
		for (i = 0; i < spriteCount; i++) {
			free(infoList[i].decodedBuffer);
		}
		free(infoList);
		memset(this, 0, sizeof(*this));
	}

	SpriteList() {
		memset(this, 0, sizeof(*this));
	}
};


class Sprite {
public:
	SpriteList _mainSprites;
	SpriteList _saveReminderSprites;
	SpriteList _arrowSprites;
	SpriteList _inventorySprites;

	Sprite(SagaEngine *vm);
	~Sprite(void);

	// draw scaled sprite using background scene mask
	void drawOccluded(Surface *ds, const Rect &clipRect, SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, int depth);

	// draw scaled sprite using background scene mask
	void draw(Surface *ds, const Rect &clipRect, SpriteList &spriteList, int32 spriteNumber, const Point &screenCoord, int scale);

	// main function
	void drawClip(Surface *ds, const Rect &clipRect, const Point &spritePointer, int width, int height, const byte *spriteBuffer);

	void draw(Surface *ds, const Rect &clipRect, SpriteList &spriteList, int32 spriteNumber, const Rect &screenRect, int scale);

	void loadList(int resourceId, SpriteList &spriteList); // load or append spriteList
	bool hitTest(SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, const Point &testPoint);
	void getScaledSpriteBuffer(SpriteList &spriteList, int spriteNumber, int scale, int &width, int &height, int &xAlign, int &yAlign, const byte *&buffer);

private:
	void decodeRLEBuffer(const byte *inputBuffer, size_t inLength, size_t outLength);
	void scaleBuffer(const byte *src, int width, int height, int scale);

	SagaEngine *_vm;
	ResourceContext *_spriteContext;
	byte *_decodeBuf;
	size_t _decodeBufLen;
};

} // End of namespace Saga

#endif
