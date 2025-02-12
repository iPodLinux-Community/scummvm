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

// Font management and font drawing header file

#ifndef SAGA_FONT_H__
#define SAGA_FONT_H__

#include "saga/list.h"
#include "saga/gfx.h"

namespace Saga {

#define FONT_SHOWUNDEFINED 1	// Define to draw undefined characters * as ?'s

// The first defined character (!) is the only one that may
// have a valid offset of '0'
#define FONT_FIRSTCHAR 33

#define FONT_CH_SPACE 32
#define FONT_CH_QMARK 63

// Minimum font header size without font data
// (6 + 512 + 256 + 256 + 256 )
#define FONT_DESCSIZE 1286

#define FONT_CHARCOUNT 256
#define FONT_CHARMASK 0xFFU

#define SAGA_FONT_HEADER_LEN 6

#define TEXT_CENTERLIMIT 50
#define TEXT_MARGIN 10
#define TEXT_LINESPACING 2

enum FontEffectFlags {
	kFontNormal   = 0,
	kFontOutline  = 1 << 0,
	kFontShadow   = 1 << 1,
	kFontBold     = 1 << 2,
	kFontCentered = 1 << 3,
	kFontDontmap  = 1 << 4
};

enum KnownFont {
	kKnownFontSmall,
	kKnownFontMedium,
	kKnownFontBig,

	kKnownFontPause,
	kKnownFontScript,
	kKnownFontVerb
};

struct TextListEntry {
	bool display;
	bool useRect;
	Common::Point point;
	Common::Rect rect;
	KnownColor knownColor;
	KnownColor effectKnownColor;
	FontEffectFlags flags;
	KnownFont font;
	const char *text;
	TextListEntry() {
		memset(this, 0, sizeof(*this));
	}
};

class TextList: public SortedList<TextListEntry> {
public:

	TextListEntry *addEntry(const TextListEntry &entry) {
		return pushBack(entry).operator->();
	}
};

struct FontHeader {
	int charHeight;
	int charWidth;
	int rowLength;
};

struct FontCharEntry {
	int index;
	int byteWidth;
	int width;
	int flag;
	int tracking;
};

struct FontStyle {
	FontHeader header;
	FontCharEntry fontCharEntry[256];
	byte *font;
};

struct FontData {
	FontStyle normal;
	FontStyle outline;
};

class Font {
 public:
	Font(SagaEngine *vm);
	~Font(void);
	int getStringWidth(KnownFont font, const char *text, size_t count, FontEffectFlags flags) {
		return getStringWidth(knownFont2FontIdx(font), text, count, flags);
	}
	int getHeight(KnownFont font) {
		return getHeight(knownFont2FontIdx(font));
	}
	int getHeight(KnownFont font, const char *text, int width, FontEffectFlags flags) {
		return getHeight(knownFont2FontIdx(font), text, width, flags);
	}
	void textDraw(KnownFont font, Surface *ds, const char *string, const Common::Point &point, int color, int effectColor, FontEffectFlags flags) {
		textDraw(knownFont2FontIdx(font), ds, string, point, color, effectColor, flags);
	}
	void textDrawRect(KnownFont font, Surface *ds, const char *text, const Common::Rect &rect, int color, int effectColor, FontEffectFlags flags) {
		textDrawRect(knownFont2FontIdx(font), ds, text, rect, color, effectColor, flags);
	}
	
 private:
	 enum FontId {
		 kSmallFont,
		 kMediumFont,
		 kBigFont,
		 kIHNMUnknown,
		 kIHNMFont8,
		 kIHNMUnknown2,
		 kIHNMMainFont
	 };

	 Font::FontId knownFont2FontIdx(KnownFont font);

	 int getStringWidth(FontId fontId, const char *text, size_t count, FontEffectFlags flags);
	 int getHeight(FontId fontId, const char *text, int width, FontEffectFlags flags);
	 void textDrawRect(FontId fontId, Surface *ds, const char *text, const Common::Rect &rect, int color, int effectColor, FontEffectFlags flags);
	 void textDraw(FontId fontId, Surface *ds, const char *string, const Common::Point &point, int color, int effectColor, FontEffectFlags flags);

	 void loadFont(uint32 fontResourceId);
	 void createOutline(FontData *font);
	 void draw(FontId fontId, Surface *ds, const char *text, size_t count, const Common::Point &point, int color, int effectColor, FontEffectFlags flags);
	 void outFont(const FontStyle &drawFont, Surface *ds, const char *text, size_t count, const Common::Point &point, int color, FontEffectFlags flags);
	 
	 FontData *getFont(FontId fontId) {
		 validate(fontId);
		 return _fonts[fontId];
	 }

	int getHeight(FontId fontId) {
		 return getFont(fontId)->normal.header.charHeight;
	 }

	 void validate(FontId fontId) {
		 if (!valid(fontId)) {
			 error("Font::validate: Invalid font id.");
		 }
	 }
	 bool valid(FontId fontId) {
		 return ((fontId >= 0) && (fontId < _loadedFonts));
	 }
	 int getByteLen(int numBits) const {
		 int byteLength = numBits / 8;

		 if (numBits % 8) {
			 byteLength++;
		 }

		 return byteLength;
	 }

	static const int _charMap[256];
	SagaEngine *_vm;

	bool _initialized;

	int _loadedFonts;
	FontData **_fonts;
};

} // End of namespace Saga

#endif
