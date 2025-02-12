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

#include "common/util.h"
#include "scumm/imuse/imuse_internal.h"
#include "scumm/saveload.h"

namespace Scumm {

////////////////////////////////////////
//
//  IMuse Part implementation
//
////////////////////////////////////////

Part::Part() {
	_slot = 0;
	_next = 0;
	_prev = 0;
	_mc = 0;
	_player = 0;
	_pitchbend = 0;
	_pitchbend_factor = 0;
	_transpose = 0;
	_transpose_eff = 0;
	_vol = 0;
	_vol_eff = 0;
	_detune = 0;
	_detune_eff = 0;
	_pan = 0;
	_pan_eff = 0;
	_on = false;
	_modwheel = 0;
	_pedal = false;
	_pri = 0;
	_pri_eff = 0;
	_chan = 0;
	_effect_level = 0;
	_chorus = 0;
	_percussion = 0;
	_bank = 0;
	_unassigned_instrument = false;
}

void Part::saveLoadWithSerializer(Serializer *ser) {
	const SaveLoadEntry partEntries[] = {
		MKLINE(Part, _pitchbend, sleInt16, VER(8)),
		MKLINE(Part, _pitchbend_factor, sleUint8, VER(8)),
		MKLINE(Part, _transpose, sleInt8, VER(8)),
		MKLINE(Part, _vol, sleUint8, VER(8)),
		MKLINE(Part, _detune, sleInt8, VER(8)),
		MKLINE(Part, _pan, sleInt8, VER(8)),
		MKLINE(Part, _on, sleUint8, VER(8)),
		MKLINE(Part, _modwheel, sleUint8, VER(8)),
		MKLINE(Part, _pedal, sleUint8, VER(8)),
		MK_OBSOLETE(Part, _program, sleUint8, VER(8), VER(16)),
		MKLINE(Part, _pri, sleUint8, VER(8)),
		MKLINE(Part, _chan, sleUint8, VER(8)),
		MKLINE(Part, _effect_level, sleUint8, VER(8)),
		MKLINE(Part, _chorus, sleUint8, VER(8)),
		MKLINE(Part, _percussion, sleUint8, VER(8)),
		MKLINE(Part, _bank, sleUint8, VER(8)),
		MKEND()
	};

	int num;
	if (ser->isSaving()) {
		num = (_next ? (_next - _se->_parts + 1) : 0);
		ser->saveUint16(num);

		num = (_prev ? (_prev - _se->_parts + 1) : 0);
		ser->saveUint16(num);

		num = (_player ? (_player - _se->_players + 1) : 0);
		ser->saveUint16(num);
	} else {
		num = ser->loadUint16();
		_next = (num ? &_se->_parts[num - 1] : 0);

		num = ser->loadUint16();
		_prev = (num ? &_se->_parts[num - 1] : 0);

		num = ser->loadUint16();
		_player = (num ? &_se->_players[num - 1] : 0);
	}
	ser->saveLoadEntries(this, partEntries);
}

void Part::set_detune(int8 detune) {
	_detune_eff = clamp((_detune = detune) + _player->getDetune(), -128, 127);
	if (_mc)
		sendPitchBend();
}

void Part::pitchBend(int16 value) {
	_pitchbend = value;
	if (_mc)
		sendPitchBend();
}

void Part::volume(byte value) {
	_vol_eff = ((_vol = value) + 1) * _player->getEffectiveVolume() >> 7;
	if (_mc)
		_mc->volume(_vol_eff);
}

void Part::set_pri(int8 pri) {
	_pri_eff = clamp((_pri = pri) + _player->getPriority(), 0, 255);
	if (_mc)
		_mc->priority(_pri_eff);
}

void Part::set_pan(int8 pan) {
	_pan_eff = clamp((_pan = pan) + _player->getPan(), -64, 63);
	if (_mc)
		_mc->panPosition(_pan_eff + 0x40);
}

void Part::set_transpose(int8 transpose) {
	_transpose_eff = transpose_clamp((_transpose = transpose) + _player->getTranspose(), -24, 24);
	if (_mc)
		sendPitchBend();
}

void Part::sustain(bool value) {
	_pedal = value;
	if (_mc)
		_mc->sustain(value);
}

void Part::modulationWheel(byte value) {
	_modwheel = value;
	if (_mc)
		_mc->modulationWheel(value);
}

void Part::chorusLevel(byte value) {
	_chorus = value;
	if (_mc)
		_mc->chorusLevel(value);
}

void Part::effectLevel(byte value)
{
	_effect_level = value;
	if (_mc)
		_mc->effectLevel(value);
}

void Part::fix_after_load() {
	set_transpose(_transpose);
	volume(_vol);
	set_detune(_detune);
	set_pri(_pri);
	set_pan(_pan);
	sendAll();
}

void Part::pitchBendFactor(byte value) {
	if (value > 12)
		return;
	pitchBend(0);
	_pitchbend_factor = value;
	if (_mc)
		_mc->pitchBendFactor(value);
}

void Part::set_onoff(bool on) {
	if (_on != on) {
		_on = on;
		if (!on)
			off();
		if (!_percussion)
			_player->_se->reallocateMidiChannels(_player->getMidiDriver());
	}
}

void Part::set_instrument(byte * data) {
	_instrument.adlib(data);
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::load_global_instrument(byte slot) {
	_player->_se->copyGlobalAdlibInstrument(slot, &_instrument);
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::noteOn(byte note, byte velocity) {
	if (!_on)
		return;

	MidiChannel *mc = _mc;

	// DEBUG
	if (_unassigned_instrument && !_percussion) {
		_unassigned_instrument = false;
		if (!_instrument.isValid()) {
			debug(0, "[%02d] No instrument specified", (int)_chan);
			return;
		}
	}

	if (mc && _instrument.isValid()) {
		mc->noteOn(note, velocity);
	} else if (_percussion) {
		mc = _player->getMidiDriver()->getPercussionChannel();
		if (!mc)
			return;
		static byte prev_vol_eff = 128;
		if (_vol_eff != prev_vol_eff){
			mc->volume(_vol_eff);
			prev_vol_eff = _vol_eff;
		}
		if ((note < 35) && (!_player->_se->isNativeMT32()))
			note = Instrument::_gmRhythmMap[note];

		mc->noteOn(note, velocity);
	}
}

void Part::noteOff(byte note) {
	if (!_on)
		return;

	MidiChannel *mc = _mc;
	if (mc) {
		mc->noteOff(note);
	} else if (_percussion) {
		mc = _player->getMidiDriver()->getPercussionChannel();
		if (mc)
			mc->noteOff(note);
	}
}

void Part::init() {
	_player = NULL;
	_next = NULL;
	_prev = NULL;
	_mc = NULL;
}

void Part::setup(Player *player) {
	_player = player;

	_percussion = (player->isMIDI() && _chan == 9); // true;
	_on = true;
	_pri_eff = player->getPriority();
	_pri = 0;
	_vol = 127;
	_vol_eff = player->getEffectiveVolume();
	_pan = clamp(player->getPan(), -64, 63);
	_transpose_eff = player->getTranspose();
	_transpose = 0;
	_detune = 0;
	_detune_eff = player->getDetune();
	_pitchbend_factor = 2;
	_pitchbend = 0;
	_effect_level = 64;
	_instrument.clear();
	_unassigned_instrument = true;
	_chorus = 0;
	_modwheel = 0;
	_bank = 0;
	_pedal = false;
	_mc = NULL;
}

void Part::uninit() {
	if (!_player)
		return;
	off();
	_player->removePart(this);
	_player = NULL;
}

void Part::off() {
	if (_mc) {
		_mc->allNotesOff();
		_mc->release();
		_mc = NULL;
	}
}

bool Part::clearToTransmit() {
	if (_mc)
		return true;
	if (_instrument.isValid())
		_player->_se->reallocateMidiChannels(_player->getMidiDriver());
	return false;
}

void Part::sendAll() {
	if (!clearToTransmit())
		return;
	_mc->pitchBendFactor(_pitchbend_factor);
	sendPitchBend();
	_mc->volume(_vol_eff);
	_mc->sustain(_pedal);
	_mc->modulationWheel(_modwheel);
	_mc->panPosition(_pan_eff + 0x40);
	_mc->effectLevel(_effect_level);
	if (_instrument.isValid())
		_instrument.send(_mc);
	_mc->chorusLevel(_chorus);
	_mc->priority(_pri_eff);
}

void Part::sendPitchBend() {
	int16 bend = _pitchbend;
	// RPN-based pitchbend range doesn't work for the MT32,
	// so we'll do the scaling ourselves.
	if (_player->_se->isNativeMT32())
		bend = bend * _pitchbend_factor / 12;
	_mc->pitchBend(clamp(bend + (_detune_eff * 64 / 12) + (_transpose_eff * 8192 / 12), -8192, 8191));
}

void Part::programChange(byte value) {
	_bank = 0;
	_instrument.program(value, _player->isMT32());
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::set_instrument(uint b) {
	_bank = (byte)(b >> 8);
	if (_bank)
		error("Non-zero instrument bank selection. Please report this");
	_instrument.program((byte)b, _player->isMT32());
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::allNotesOff() {
	if (!_mc)
		return;
	_mc->allNotesOff();
}

} // End of namespace Scumm
