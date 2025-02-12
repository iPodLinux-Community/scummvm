/* ScummVM - Scumm Interpreter
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
#include "ItemSwitch.h"

namespace CEGUI {

	void ItemSwitch::init(WORD referenceTrue, WORD referenceFalse) {
		_backgroundTrue = _background;
		_backgroundFalse = new SDL_ImageResource();
		if (!_backgroundFalse->load(referenceFalse)) {
			delete _backgroundFalse;
			delete _background;
			_background = NULL;
			_backgroundFalse = NULL;
		}
	}

	ItemSwitch::ItemSwitch(WORD referenceTrue, WORD referenceFalse, bool *item) :
	PanelItem(referenceTrue) {
		init(referenceTrue, referenceFalse);
		_item = item;
		_itemmax = -1;
		if (!*_item)
			_background = _backgroundFalse;
	}

	ItemSwitch::ItemSwitch(WORD referenceTrue, WORD referenceFalse, int *item, int max) :
	PanelItem(referenceTrue) {
		 init(referenceTrue, referenceFalse);
		_itemmultiple = item;
		_itemmax = max;
		if (!*item)
			_background = _backgroundFalse;
	}

	ItemSwitch::~ItemSwitch() {
		if (_backgroundFalse)
			delete _backgroundFalse;
	}

	bool ItemSwitch::action(int x, int y, bool pushed) {

		if (checkInside(x, y) && _visible && pushed) {
			if (_itemmax <= 0) {
				*_item = !*_item;
				if (*_item)
					_background = _backgroundTrue;
				else
					_background = _backgroundFalse;

				if (_panel)
					_panel->forceRedraw();

				return true;
			} else {
				*_itemmultiple = *_itemmultiple + 1;
				if (*_itemmultiple > _itemmax)
					*_itemmultiple = 0;
				if (*_itemmultiple)
					_background = _backgroundTrue;
				else
					_background = _backgroundFalse;

				if (_panel)
					_panel->forceRedraw();

				return true;
			}
		} else
			return false;
	}
}
