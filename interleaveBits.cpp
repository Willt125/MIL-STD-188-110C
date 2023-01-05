#include <algorithm>
#include <iostream>
#include <vector>
#include "Types.h"
#include "interleaveBits.h"

void interleave_chunk(std::vector<byte>* chunk, size_t numrows, size_t numcols, byte offset)
{
	if (chunk->size() != ((numcols * numrows) / 8) + (((numcols * numrows) % 8 == 0) ? 0 : 1)) // bounds check!
	{
		std::cerr << "The input chunk doesn't fit in the interleave matrix. How did you do this?\n\n";
		exit(1);
	}

	size_t i = 0, rownum = 0, colnum = 0;
	std::vector<byte> temp(chunk->size()); // temporary buffer to hold the matrix itself

	while (i < (numrows * numcols)) // load the matrix from the input
	{
		size_t count = 0;

		while (count < numrows)
		{
			setBitVal(&temp, (colnum + numcols * rownum), getBitVal(*chunk, i + offset));
			rownum = (rownum + 7) % numrows;
			count++; i++;
		}

		rownum = 0;
		colnum++;
	}

	i = 0;
	rownum = 0;
	colnum = 0;
	size_t lastcolnum;

	while (i < (numcols * numrows)) // and then unload it back in a different order
	{
		lastcolnum = colnum;

		while (rownum < numrows)
		{
			setBitVal(chunk, i + offset, getBitVal(temp, (colnum + numcols * rownum)));
			rownum++;
			colnum = (colnum - 7) % numcols;
			i++;
		}

		rownum = 0;
		colnum = lastcolnum + 1;
	}
	return;
}

void interleave_data(std::vector<byte>* datastream, size_t baud, interleave_len chunklen, size_t bitlen)
{
	size_t numrows, numcols;
	byte tempmask[2];
	byte boffset, eoffset;

	switch (baud) // the interleave pattern changes depending on the selected parameters.
	{
	case 150:
	case 300:
	case 600:
	{
		switch (chunklen)
		{
		case _zero:
			return;

		case _short:
			numcols = 18;
			numrows = 40;
			break;

		case _long:
			numcols = 144;
			numrows = 40;
			break;
		}
		break;
	}

	case 75:
	{
		switch (chunklen)
		{
		case _zero:
			return;

		case _short:
			numcols = 9;
			numrows = 10;
			break;

		case _long:
			numcols = 36;
			numrows = 20;
			break;
		}
		break;
	}

	case 1200:
	{
		switch (chunklen)
		{
		case _zero:
			return;

		case _short:
			numcols = 36;
			numrows = 40;
			break;

		case _long:
			numcols = 288;
			numrows = 40;
			break;
		}
		break;
	}

	case 2400:
	{
		switch (chunklen)
		{
		case _zero:
			return;

		case _short:
			numcols = 72;
			numrows = 40;
			break;

		case _long:
			numcols = 576;
			numrows = 40;
			break;
		}
		break;
	}

	default: // 4800 baud doesn't get interleaved
		return;
	}

	size_t z = numcols * numrows;

	if ((bitlen % z) != 0) // bounds check
	{
		std::cerr << "The input data does not fit evenly into the interleaver. Check upstream! (interleave_data)\n\n";
		exit(1);
	}

	std::vector<byte> temp((z / 8) + ((z % 8 != 0) ? 1 : 0)); // Set up the interleave matrix

	/*
	We're ready to go, but it's not that simple. Unfortunately, the interleave matrix may not be
	an even number of bytes long, which means we may end up stuck between bytes.

	This means we'll need to do a few things before and after we interleave each chunk, that being:

	1) Slice the needed chunk into a separate buffer called the matrix (from above), to protect the rest
	of the stream.

	2) If we're in the middle of a byte (whether at the beginning or the end of the slice), we need to mask
	the data we don't want, setting it to zero. That's where those funky ternary conditions come in:
	If we're aligned on the byte, (index modulo 8 = 0), return the full byte. Otherwise, we whip up a mask
	to erase the bits before the index.
	Same concept at the tail; if we're aligned on the byte at the tail end of the buffer (index + length of
	the buffer modulo 8 = 0), that means we didn't need to add any stray bits in an extra byte, just return
	the whole thing. Otherwise, whip up a mask to only return the stray bits in the MSB.

	3) Call interleave_chunk() on the tailored chunk.

	4) Now we need to do the same thing we just did in step 2, but in reverse, because we still need to preserve
	the bits around the chunk, if we're not aligned on a byte boundary.

	NOTE!!!

	This is looking really wonky, and MIGHT work.
	*/

	for (size_t i = 0; i < bitlen; i += z)
	{
		std::copy(datastream->begin() + ((i / 8) + ((i % 8 != 0) ? 1 : 0)), datastream->begin() + (((i + z) / 8) + (((i + z) % 8 != 0) ? 1 : 0)), temp.begin());
		boffset = (i % 8);
		eoffset = ((i + z) % 8);

		temp[0] &= ((boffset == 0) ? 0xff : ~((char)0x80 >> (boffset - 1)));
		temp[(z / 8) + ((z % 8 == 0) ? -1 : 0)] &= ((eoffset == 0) ? 0xff : ((char)0x80 >> (eoffset - 1))); // We're not allowed to have any simple things in life

		interleave_chunk(&temp, numrows, numcols, boffset);

		tempmask[0] = datastream->at((i / 8)) & ((boffset == 0) ? 0 : ((char)0x80 >> (boffset - 1)));
		tempmask[1] = datastream->at((((i + z) / 8) + (((i + z) % 8 != 0) ? 1 : 0) + (((i + z) / 8 == datastream->size()) ? -1 : 0))) & ((eoffset == 0) ? 0 : ~((char)0x80 >> (eoffset - 1))); // ouch

		temp[0] |= tempmask[0];
		temp[(z / 8) + ((z % 8 == 0) ? -1 : 0)] |= tempmask[1]; // ever

		std::copy(std::begin(temp), std::end(temp), std::begin(*datastream) + (i / 8));
	}

	return;
}