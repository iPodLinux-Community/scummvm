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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "backends/timer/default/default-timer.h"
#include "common/util.h"
#include "common/system.h"

namespace Common {
// FIXME: Hack: This global variable shouldn't be declared here; in fact it 
// probably shouldn't be declared at all but rather a different method to
// query the TimerManager object should be invented.
TimerManager *g_timer = NULL;
}

DefaultTimerManager::DefaultTimerManager(OSystem *system) :
	_system(system),
	_timerHandler(0),
	_lastTime(0) {

	Common::g_timer = this;

	for (int i = 0; i < MAX_TIMERS; i++) {
		_timerSlots[i].procedure = NULL;
		_timerSlots[i].interval = 0;
		_timerSlots[i].counter = 0;
	}

	_thisTime = _system->getMillis();

	// Set the timer last, after everything has been initialised
	_system->setTimerCallback(&timer_handler, 10);

}

DefaultTimerManager::~DefaultTimerManager() {
	// Remove the timer callback.
	// Note: backends *must* gurantee that after this method call returns,
	// the handler is not in use anymore; else race condtions could occur.
	_system->setTimerCallback(0, 0);

	{
		Common::StackLock lock(_mutex);
		for (int i = 0; i < MAX_TIMERS; i++) {
			_timerSlots[i].procedure = NULL;
			_timerSlots[i].interval = 0;
			_timerSlots[i].counter = 0;
		}
	}
}

int DefaultTimerManager::timer_handler(int t) {
	if (Common::g_timer)
		return ((DefaultTimerManager *)Common::g_timer)->handler(t);
	return 0;
}

int DefaultTimerManager::handler(int t) {
	Common::StackLock lock(_mutex);
	uint32 interval, l;

	_lastTime = _thisTime;
	_thisTime = _system->getMillis();
	interval = 1000 * (_thisTime - _lastTime);

	for (l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure && _timerSlots[l].interval > 0) {
			_timerSlots[l].counter -= interval;
			while (_timerSlots[l].counter <= 0) {
				// A small paranoia check which catches the case where
				// a timer removes itself (which it never should do).
				assert(_timerSlots[l].procedure && _timerSlots[l].interval > 0);
				_timerSlots[l].counter += _timerSlots[l].interval;
				_timerSlots[l].procedure(_timerSlots[l].refCon);
			}
		}
	}

	return t;
}

bool DefaultTimerManager::installTimerProc(TimerProc procedure, int32 interval, void *refCon) {
	assert(interval > 0);
	Common::StackLock lock(_mutex);

	for (int l = 0; l < MAX_TIMERS; l++) {
		if (!_timerSlots[l].procedure) {
			_timerSlots[l].procedure = procedure;
			_timerSlots[l].interval = interval;
			_timerSlots[l].counter = interval;
			_timerSlots[l].refCon = refCon;
			return true;
		}
	}

	warning("Couldn't find free timer slot");
	return false;
}

void DefaultTimerManager::removeTimerProc(TimerProc procedure) {
	Common::StackLock lock(_mutex);

	for (int l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure == procedure) {
			_timerSlots[l].procedure = 0;
			_timerSlots[l].interval = 0;
			_timerSlots[l].counter = 1;	// Work around a problem when a timer proc removes itself
			_timerSlots[l].refCon = 0;
		}
	}
}
