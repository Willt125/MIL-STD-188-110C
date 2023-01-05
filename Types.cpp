#include <iostream>
#include <vector>
#include "Types.h"

byte getBitVal(std::vector<byte> stream, size_t idx)
{
	if (idx < 0 || idx >= (stream.size() * 8)) // Make sure the bit index exists
	{
		std::cerr << "Stream index out of bounds. (getBitVal)\n\n";
		std::cerr << "stream:\n";
		for (size_t i = 0; i < stream.size(); i++)
		{
			std::cerr << stream[i];
		}
		std::cerr << "\n\n";

		std::cerr << "idx: " << idx << "\n\n";
		exit(1);
	}

	int byteidx = idx / 8; // floor(idx / 8). Selects the byte the wanted bit is in.
	byte bitidx = idx % 8; // and now the bit itself.

	byte tmp = stream[byteidx]; // make a copy for data safety

	byte result = tmp & (0x80 >> bitidx); // lsr to set the mask

	return (result >> (7 - bitidx)); // move the returned bit all the way to the right, could've replaced with bool
}

void setBitVal(std::vector<byte>* stream, size_t idx, byte val)
{
	if ((idx < 0) || (idx >= stream->size() * 8)) // make sure the bit index exists
	{
		std::cerr << "Stream index out of bounds. (setBitVal)\n\n";
		std::cerr << "stream:\n";
		for (size_t i = 0; i < stream->size(); i++)
		{
			std::cerr << stream->at(i);
		}
		std::cerr << "\n\n";

		std::cerr << "idx: " << idx << "\n\n";
		exit(1);
	}

	int byteidx = idx / 8; // select the byte the bit  will reside in
	byte bitidx = idx % 8; // and the fine position

	byte tmp = stream->at(byteidx); // make a copy!

	byte setmask = ~(0x80 >> bitidx); // this mask will let you set a 0

	val <<= (7 - bitidx); // move the bit in place (needed because it's currently just a bool)

	if (val == 0) // now finally set the bit
		tmp &= setmask;
	else
		tmp |= val;

	stream->at(byteidx) = tmp; // and put the byte back in the stream

	return;
}

byte unaryXOR(byte val)
{
	byte result = 0;
	while (val)
	{
		result ^= val & 1;
		val >>= 1;
	}

	return result;
}