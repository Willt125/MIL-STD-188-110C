#ifndef _FECENCODEBITS_H
#define _FECENCODEBITS_H

#include <vector>
#include "Types.h"

/*
	fecEncodeBits() runs a Forward Error-Correction algorithm on the input stream (pseudo) in-place.
	Only a very specific algorithm will do for this chain (because it was custom made), so I guess
	I'm doing it myself.

	This step adds resiliency to the encoded signal.
*/
size_t fecEncodeBits(std::vector<byte>* datastream, size_t baud, size_t bitlen);

#endif