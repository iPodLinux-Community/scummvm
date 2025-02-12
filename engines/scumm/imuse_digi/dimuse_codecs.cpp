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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"

namespace Scumm {

namespace BundleCodecs {

uint32 decode12BitsSample(const byte *src, byte **dst, uint32 size) {
	uint32 loop_size = size / 3;
	uint32 s_size = loop_size * 4;
	byte *ptr = *dst = (byte *)malloc(s_size);
	assert(ptr);

	uint32 tmp;
	while (loop_size--) {
		byte v1 = *src++;
		byte v2 = *src++;
		byte v3 = *src++;
		tmp = ((((v2 & 0x0f) << 8) | v1) << 4) - 0x8000;
		WRITE_BE_UINT16(ptr, tmp); ptr += 2;
		tmp = ((((v2 & 0xf0) << 4) | v3) << 4) - 0x8000;
		WRITE_BE_UINT16(ptr, tmp); ptr += 2;
	}
	return s_size;
}

/*
 * The "IMC" codec below (see cases 13 & 15 in decompressCodec) is actually a
 * variant of the IMA codec, see also
 *   <http://www.multimedia.cx/simpleaudio.html>
 *
 * It is somewhat different, though: the standard ADPCM codecs use a fixed
 * size for their data packets (4 bits), while the codec implemented here
 * varies the size of each "packet" between 2 and 7 bits.
 */

static byte _imcTableEntryBitCount[89];

#ifdef PALMOS_68K
static const int16 *imcTable;
#else
static const int16 imcTable[89] = {
		7,	  8,	9,	 10,   11,	 12,   13,	 14,
	   16,	 17,   19,	 21,   23,	 25,   28,	 31,
	   34,	 37,   41,	 45,   50,	 55,   60,	 66,
	   73,	 80,   88,	 97,  107,	118,  130,	143,
	  157,	173,  190,	209,  230,	253,  279,	307,
	  337,	371,  408,	449,  494,	544,  598,	658,
	  724,	796,  876,	963, 1060, 1166, 1282, 1411,
	 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	 7132, 7845, 8630, 9493,10442,11487,12635,13899,
	15289,16818,18500,20350,22385,24623,27086,29794,
	32767
};
#endif

static const byte imxOtherTable[6][64] = {
	{
		0xFF,
		4
	},

	{
		0xFF, 0xFF,
		   2,    8
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    4,    6
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    4,    6,    8,   12,   16,   32
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    4,    6,    8,   10,   12,   14,
		  16,   18,   20,   22,   24,   26,   28,   32
	},

	{
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		   1,    2,    3,    4,    5,    6,    7,    8,
		   9,   10,   11,   12,   13,   14,   15,   16,
		  17,   18,   19,   20,   21,   22,   23,   24,
		  25,   26,   27,   28,   29,   30,   31,   32
	}
};

void initializeImcTables() {
	int pos;

	for (pos = 0; pos < ARRAYSIZE(imcTable); ++pos) {
		byte put = 0;
		int32 tableValue = ((imcTable[pos] * 4) / 7) / 2;
		while (tableValue != 0) {
			tableValue /= 2;
			put++;
		}
		if (put < 2) {
			put = 2;
		}
		if (put > 7) {
			put = 7;
		}
		_imcTableEntryBitCount[pos] = put;
	}
}

#define NextBit                            \
	do {                                   \
		bit = mask & 1;                    \
		mask >>= 1;                        \
		if (!--bitsleft) {                 \
			mask = READ_LE_UINT16(srcptr); \
			srcptr += 2;                   \
			bitsleft = 16;                 \
		}                                  \
	} while (0)

static int32 compDecode(byte *src, byte *dst) {
	byte *result, *srcptr = src, *dstptr = dst;
	int data, size, bit, bitsleft = 16, mask = READ_LE_UINT16(srcptr);
	srcptr += 2;

	for (;;) {
		NextBit;
		if (bit) {
			*dstptr++ = *srcptr++;
		} else {
			NextBit;
			if (!bit) {
				NextBit;
				size = bit << 1;
				NextBit;
				size = (size | bit) + 3;
				data = *srcptr++ | 0xffffff00;
			} else {
				data = *srcptr++;
				size = *srcptr++;

				data |= 0xfffff000 + ((size & 0xf0) << 4);
				size = (size & 0x0f) + 3;

				if (size == 3)
					if (((*srcptr++) + 1) == 1)
						return dstptr - dst;
			}
			result = dstptr + data;
			while (size--)
				*dstptr++ = *result++;
		}
	}
}
#undef NextBit

int32 decompressCodec(int32 codec, byte *comp_input, byte *comp_output, int32 input_size) {
	int32 output_size, channels;
	int32 offset1, offset2, offset3, length, k, c, s, j, r, t, z;
	byte *src, *t_table, *p, *ptr;
	byte t_tmp1, t_tmp2;

	switch (codec) {
	case 0:
		memcpy(comp_output, comp_input, input_size);
		output_size = input_size;
		break;

	case 1:
		output_size = compDecode(comp_input, comp_output);
		break;

	case 2:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];
		break;

	case 3:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];
		break;

	case 4:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		assert(t_table);

		src = comp_output;
		length = (output_size << 3) / 12;
		k = 0;
		if (length > 0) {
			c = -12;
			s = 0;
			j = 0;
			do {
				ptr = src + length + (k >> 1);
				t_tmp2 = src[j];
				if (k & 1) {
					r = c >> 3;
					t_table[r + 2] = ((t_tmp2 & 0x0f) << 4) | (ptr[1] >> 4);
					t_table[r + 1] = (t_tmp2 & 0xf0) | (t_table[r + 1]);
				} else {
					r = s >> 3;
					t_table[r + 0] = ((t_tmp2 & 0x0f) << 4) | (ptr[0] & 0x0f);
					t_table[r + 1] = t_tmp2 >> 4;
				}
				s += 12;
				c += 12;
				k++;
				j++;
			} while (k < length);
		}
		offset1 = ((length - 1) * 3) >> 1;
		t_table[offset1 + 1] = (t_table[offset1 + 1]) | (src[length - 1] & 0xf0);
		memcpy(src, t_table, output_size);
		free(t_table);
		break;

	case 5:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		assert(t_table);

		src = comp_output;
		length = (output_size << 3) / 12;
		k = 1;
		c = 0;
		s = 12;
		t_table[0] = src[length] >> 4;
		t = length + k;
		j = 1;
		if (t > k) {
			do {
				t_tmp1 = *(src + length + (k >> 1));
				t_tmp2 = src[j - 1];
				if (k & 1) {
					r = c >> 3;
					t_table[r + 0] = (t_tmp2 & 0xf0) | t_table[r];
					t_table[r + 1] = ((t_tmp2 & 0x0f) << 4) | (t_tmp1 & 0x0f);
				} else {
					r = s >> 3;
					t_table[r + 0] = t_tmp2 >> 4;
					t_table[r - 1] = ((t_tmp2 & 0x0f) << 4) | (t_tmp1 >> 4);
				}
				s += 12;
				c += 12;
				k++;
				j++;
			} while (k < t);
		}
		memcpy(src, t_table, output_size);
		free(t_table);
		break;

	case 6:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		assert(t_table);

		src = comp_output;
		length = (output_size << 3) / 12;
		k = 0;
		c = 0;
		j = 0;
		s = -12;
		t_table[0] = src[output_size - 1];
		t_table[output_size - 1] = src[length - 1];
		t = length - 1;
		if (t > 0) {
			do {
				t_tmp1 = *(src + length + (k >> 1));
				t_tmp2 = src[j];
				if (k & 1) {
					r = s >> 3;
					t_table[r + 2] = (t_tmp2 & 0xf0) | t_table[r + 2];
					t_table[r + 3] = ((t_tmp2 & 0x0f) << 4) | (t_tmp1 >> 4);
				} else {
					r = c >> 3;
					t_table[r + 2] = t_tmp2 >> 4;
					t_table[r + 1] = ((t_tmp2 & 0x0f) << 4) | (t_tmp1 & 0x0f);
				}
				s += 12;
				c += 12;
				k++;
				j++;
			} while (k < t);
		}
		memcpy(src, t_table, output_size);
		free(t_table);
		break;

	case 10:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		assert(t_table);
		memcpy(t_table, p, output_size);

		offset1 = output_size / 3;
		offset2 = offset1 << 1;
		offset3 = offset2;
		src = comp_output;

		while (offset1--) {
			offset2 -= 2;
			offset3--;
			t_table[offset2 + 0] = src[offset1];
			t_table[offset2 + 1] = src[offset3];
		}

		src = comp_output;
		length = (output_size << 3) / 12;
		k = 0;
		if (length > 0) {
			c = -12;
			s = 0;
			do {
				j = length + (k >> 1);
				t_tmp1 = t_table[k];
				if (k & 1) {
					r = c >> 3;
					t_tmp2 = t_table[j + 1];
					src[r + 2] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 >> 4);
					src[r + 1] = (src[r + 1]) | (t_tmp1 & 0xf0);
				} else {
					r = s >> 3;
					t_tmp2 = t_table[j];
					src[r + 0] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
					src[r + 1] = t_tmp1 >> 4;
				}
				s += 12;
				c += 12;
				k++;
			} while (k < length);
		}
		offset1 = ((length - 1) * 3) >> 1;
		src[offset1 + 1] = (t_table[length] & 0xf0) | src[offset1 + 1];
		free(t_table);
		break;

	case 11:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		assert(t_table);
		memcpy(t_table, p, output_size);

		offset1 = output_size / 3;
		offset2 = offset1 << 1;
		offset3 = offset2;
		src = comp_output;

		while (offset1--) {
			offset2 -= 2;
			offset3--;
			t_table[offset2 + 0] = src[offset1];
			t_table[offset2 + 1] = src[offset3];
		}

		src = comp_output;
		length = (output_size << 3) / 12;
		k = 1;
		c = 0;
		s = 12;
		t_tmp1 = t_table[length] >> 4;
		src[0] = t_tmp1;
		t = length + k;
		if (t > k) {
			do {
				j = length + (k >> 1);
				t_tmp1 = t_table[k - 1];
				t_tmp2 = t_table[j];
				if (k & 1) {
					r = c >> 3;
					src[r + 0] = (src[r]) | (t_tmp1 & 0xf0);
					src[r + 1] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
				} else {
					r = s >> 3;
					src[r + 0] = t_tmp1 >> 4;
					src[r - 1] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 >> 4);
				}
				s += 12;
				c += 12;
				k++;
			} while (k < t);
		}
		free(t_table);
		break;

	case 12:
		output_size = compDecode(comp_input, comp_output);
		p = comp_output;
		for (z = 2; z < output_size; z++)
			p[z] += p[z - 1];
		for (z = 1; z < output_size; z++)
			p[z] += p[z - 1];

		t_table = (byte *)malloc(output_size);
		assert(t_table);
		memcpy(t_table, p, output_size);

		offset1 = output_size / 3;
		offset2 = offset1 << 1;
		offset3 = offset2;
		src = comp_output;

		while (offset1--) {
			offset2 -= 2;
			offset3--;
			t_table[offset2 + 0] = src[offset1];
			t_table[offset2 + 1] = src[offset3];
		}

		src = comp_output;
		length = (output_size << 3) / 12;
		k = 0;
		c = 0;
		s = -12;
		src[0] = t_table[output_size - 1];
		src[output_size - 1] = t_table[length - 1];
		t = length - 1;
		if (t > 0) {
			do {
				j = length + (k >> 1);
				t_tmp1 = t_table[k];
				t_tmp2 = t_table[j];
				if (k & 1) {
					r = s >> 3;
					src[r + 2] = (src[r + 2]) | (t_tmp1 & 0xf0);
					src[r + 3] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 >> 4);
				} else {
					r = c >> 3;
					src[r + 2] = t_tmp1 >> 4;
					src[r + 1] = ((t_tmp1 & 0x0f) << 4) | (t_tmp2 & 0x0f);
				}
				s += 12;
				c += 12;
				k++;
			} while (k < t);
		}
		free(t_table);
		break;

	case 13:
	case 15:
		if (codec == 13) {
			channels = 1;
		} else {
			channels = 2;
		}

		{
			// Decoder for the the IMA ADPCM variants used in COMI.
			// Contrary to regular IMA ADPCM, this codec uses a variable
			// bitsize for the encoded data.

			const int MAX_CHANNELS = 2;
			int32 outputSamplesLeft;
			int32 destPos;
			int16 firstWord;
			byte initialTablePos[MAX_CHANNELS] = {0, 0};
			int32 initialimcTableEntry[MAX_CHANNELS] = {7, 7};
			int32 initialOutputWord[MAX_CHANNELS] = {0, 0};
			int32 totalBitOffset, curTablePos, outputWord;
			byte *dst;
			int i;

			// We only support mono and stereo
			assert(channels == 1 || channels == 2);

			src = comp_input;
			dst = comp_output;
			output_size = 0x2000;
			outputSamplesLeft = 0x1000;

			// Every data packet contains 0x2000 bytes of audio data
			// when extracted. In order to encode bigger data sets,
			// one has to split the data into multiple blocks.
			//
			// Every block starts with a 2 byte word. If that word is
			// non-zero, it indicates the size of a block of raw audio
			// data (not encoded) following it. That data we simply copy
			// to the output buffer and the proceed by decoding the
			// remaining data.
			//
			// If on the other hand the word is zero, then what follows
			// are 7*channels bytes containing seed data for the decoder.
			firstWord = READ_BE_UINT16(src);
			src += 2;
			if (firstWord != 0) {
				// Copy raw data
				memcpy(dst, src, firstWord);
				dst += firstWord;
				src += firstWord;
				assert((firstWord & 1) == 0);
				outputSamplesLeft -= firstWord / 2;
			} else {
				// Read the seed values for the decoder.
				for (i = 0; i < channels; i++) {
					initialTablePos[i] = *src;
					src += 1;
					initialimcTableEntry[i] = READ_BE_UINT32(src);
					src += 4;
					initialOutputWord[i] = READ_BE_UINT32(src);
					src += 4;
				}
			}

			totalBitOffset = 0;
			// The channels are encoded separately.
			for (int chan = 0; chan < channels; chan++) {
				// Read initial state (this makes it possible for the data stream
				// to be split & spread across multiple data chunks.
				curTablePos = initialTablePos[chan];
				//imcTableEntry = initialimcTableEntry[chan];
				outputWord = initialOutputWord[chan];

				// We need to interleave the channels in the output; we achieve
				// that by using a variables dest offset:
				destPos = chan * 2;

				const int bound = (channels == 1)
									? outputSamplesLeft
									: ((chan == 0)
										? (outputSamplesLeft+1) / 2
										: outputSamplesLeft / 2);
				for (i = 0; i < bound; ++i) {
					// Determine the size (in bits) of the next data packet
					const int32 curTableEntryBitCount = _imcTableEntryBitCount[curTablePos];
					assert(2 <= curTableEntryBitCount && curTableEntryBitCount <= 7);

					// Read the next data packet
					const byte *readPos = src + (totalBitOffset >> 3);
					const uint16 readWord = (uint16)(READ_BE_UINT16(readPos) << (totalBitOffset & 7));
					const byte packet = (byte)(readWord >> (16 - curTableEntryBitCount));

					// Advance read position to the next data packet
					totalBitOffset += curTableEntryBitCount;

					// Decode the data packet into a delta value for the output signal.
					const byte signBitMask = (1 << (curTableEntryBitCount - 1));
					const byte dataBitMask = (signBitMask - 1);
					const byte data = (packet & dataBitMask);

					int32 delta = imcTable[curTablePos] * (2 * data + 1) >> (curTableEntryBitCount - 1);

					// The topmost bit in the data packet tells is a sign bit
					if ((packet & signBitMask) != 0) {
						delta = -delta;
					}

					// Accumulate the delta onto the output data
					outputWord += delta;

					// Clip outputWord to 16 bit signed, and write it into the destination stream
					if (outputWord > 0x7fff)
						outputWord = 0x7fff;
					if (outputWord < -0x8000)
						outputWord = -0x8000;
					WRITE_BE_UINT16(dst + destPos, outputWord);
					destPos += channels << 1;

					// Adjust the curTablePos
					curTablePos += (int8)imxOtherTable[curTableEntryBitCount - 2][data];
					if (curTablePos < 0)
						curTablePos = 0;
					else if (curTablePos >= ARRAYSIZE(imcTable))
						curTablePos = ARRAYSIZE(imcTable) - 1;
				}
			}
		}
		break;

	default:
		error("BundleCodecs::decompressCodec() Unknown codec %d!", (int)codec);
		output_size = 0;
		break;
	}

	return output_size;
}

} // End of namespace BundleCodecs

} // End of namespace Scumm

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(DimuseCodecs)
_GSETPTR(Scumm::BundleCodecs::imcTable, GBVARS_IMCTABLE_INDEX, int16, GBVARS_SCUMM)
_GEND

_GRELEASE(DimuseCodecs)
_GRELEASEPTR(GBVARS_IMCTABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
