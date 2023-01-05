#include <iostream>
#include <algorithm>
#include <vector>
#include "Types.h"
#include "encodeInputText.h"

size_t encodeInputText(std::vector<byte>* datastream, size_t inter_len)
{
	size_t arlen = datastream->size(); // get textstream size
	size_t bitlen = arlen * 8 + 320 + 144; // and convert to bits, then add 40*8 bits for EOM and 144 bits padding for the FEC buffer

	size_t additionalEl = 0;

	if (inter_len != 0)
	{
		while ((bitlen + additionalEl) % inter_len != 0) // add additional padding to make the stream evenly divisible by the interleaver
		{
			additionalEl++;
		}
	}

	/*
		Because integer division returns the floor of the result (because integers), the actual bit length
		may actually extend a byte past where the division tells you it does. The modulo operator will return
		non-zero if that is the case (this is actually how get/setBitVal selects with bit granularity as well).

		You must add and individual bits before division (as below) because of these rounding mechanics.
	*/
	size_t outlen = ((bitlen + additionalEl) / 8) + (((bitlen + additionalEl) % 8 != 0) ? 1 : 0);
	bitlen += additionalEl;

	datastream->insert(datastream->end(), &EOM[0], &EOM[39]); // insert the EOM stream right at the end of the text

	datastream->resize(outlen); // and all the empty padding should be all zeros, which vector resizing does automagically.

	return bitlen;
}