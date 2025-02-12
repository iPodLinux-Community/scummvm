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

#ifdef MACOSX

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "sound/mpu401.h"

#include <CoreMIDI/CoreMIDI.h>



/*
For information on how to unify the CoreMidi and MusicDevice code:

http://lists.apple.com/archives/Coreaudio-api/2005/Jun/msg00194.html
http://lists.apple.com/archives/coreaudio-api/2003/Mar/msg00248.html
http://lists.apple.com/archives/coreaudio-api/2003/Jul/msg00137.html

*/


/* CoreMIDI MIDI driver
 * By Max Horn
 */
class MidiDriver_CoreMIDI : public MidiDriver_MPU401 {
public:
	MidiDriver_CoreMIDI();
	~MidiDriver_CoreMIDI();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	MIDIClientRef	mClient;
	MIDIPortRef		mOutPort;
	MIDIEndpointRef	mDest;
};

MidiDriver_CoreMIDI::MidiDriver_CoreMIDI()
	: mClient(0), mOutPort(0), mDest(0) {

	OSStatus err;
	err = MIDIClientCreate(CFSTR("ScummVM MIDI Driver for OS X"), NULL, NULL, &mClient);
}

MidiDriver_CoreMIDI::~MidiDriver_CoreMIDI() {
	if (mClient)
		MIDIClientDispose(mClient);
	mClient = 0;
}

int MidiDriver_CoreMIDI::open() {
	if (mDest)
		return MERR_ALREADY_OPEN;

	OSStatus err = noErr;

	mOutPort = 0;

	int dests = MIDIGetNumberOfDestinations();
	if (dests > 0 && mClient) {
		mDest = MIDIGetDestination(0);
		err = MIDIOutputPortCreate( mClient,
									CFSTR("scummvm_output_port"),
									&mOutPort);
	} else {
		return MERR_DEVICE_NOT_AVAILABLE;
	}
	
	if (err != noErr)	
		return MERR_CANNOT_CONNECT;

	return 0;
}

void MidiDriver_CoreMIDI::close() {
	MidiDriver_MPU401::close();

	if (mOutPort && mDest) {
		MIDIPortDispose(mOutPort);
		mOutPort = 0;
		mDest = 0;
	}
}

void MidiDriver_CoreMIDI::send(uint32 b) {
	assert(mOutPort != NULL);
	assert(mDest != NULL);

	// Extract the MIDI data
	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;

	// Generate a single MIDI packet with that data
	MIDIPacketList packetList;
	MIDIPacket *packet = &packetList.packet[0];

	packetList.numPackets = 1;

	packet->timeStamp = 0;
	packet->data[0] = status_byte;
	packet->data[1] = first_byte;
	packet->data[2] = second_byte;

	// Compute the correct length of the MIDI command. This is important,
	// else things may screw up badly...
	switch (status_byte & 0xF0) {
	case 0x80:	// Note Off
	case 0x90:	// Note On
	case 0xA0:	// Polyphonic Aftertouch
	case 0xB0:	// Controller Change
	case 0xE0:	// Pitch Bending
		packet->length = 3;
		break;
	case 0xC0:	// Programm Change
	case 0xD0:	// Monophonic Aftertouch
		packet->length = 2;
		break;
	default:
		warning("CoreMIDI driver encountered unsupported status byte: 0x%02x", status_byte);
		packet->length = 3;
		break;
	}

	// Finally send it out to the synthesizer.
	MIDISend(mOutPort, mDest, &packetList);
}

void MidiDriver_CoreMIDI::sysEx(const byte *msg, uint16 length) {
	assert(mOutPort != NULL);
	assert(mDest != NULL);

	byte buf[384];
	MIDIPacketList *packetList = (MIDIPacketList *)buf;
	MIDIPacket *packet = packetList->packet;

	assert(sizeof(buf) >= sizeof(UInt32) + sizeof(MIDITimeStamp) + sizeof(UInt16) + length + 2);

	packetList->numPackets = 1;

	packet->timeStamp = 0;

	// Add SysEx frame if missing
	if (*msg != 0xF0) {
		packet->length = length + 2;
		packet->data[0] = 0xF0;
		memcpy(packet->data + 1, msg, length);
		packet->data[length + 1] = 0xF7;
	} else {
		packet->length = length;
		memcpy(packet->data, msg, length);
	}

	MIDISend(mOutPort, mDest, packetList);
}

MidiDriver *MidiDriver_CoreMIDI_create() {
	return new MidiDriver_CoreMIDI();
}

#endif // MACOSX
