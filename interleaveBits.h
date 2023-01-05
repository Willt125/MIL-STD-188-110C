#ifndef _INTERLEAVE_BITS_H
#define _INTERLEAVE_BITS_H

#include <vector>
#include "Types.h"

// I plan on using this as part of a mode selector
enum interleave_len
{
	_zero,
	_short,
	_long
};

/*
	interleave_data() does all the management for interleave_chunk(), which does all the dirty work interleaving the input stream.

	The interleaver works thus:
	you pass in the input stream (and bit size) and mode parameters to interleave_data(), which sets up the size and interleave pattern
	used by interleave_chunk().

	They then go back and forth slicing the input stream, and in-place interleaving it. This module is parallelizable, probably embarassingly so.
*/
void interleave_data(std::vector<byte>* datastream, size_t baud, interleave_len chunklen, size_t bitlen);
void interleave_chunk(std::vector<byte>* chunk, size_t numrows, size_t numcols, byte offset);

#endif