#include <iostream>
#include <vector>
#include "mgdDecode.h"
#include "Types.h"

byte mgdDecode(std::vector<byte>* datastream, size_t baud, size_t bitlen)
{
	byte temp;
	size_t idx = 0, numGroups = 0;
	byte idxInc;

	switch (baud)
	{
	case 75:
	case 1200:
		idxInc = 2;
		break;

	case 2400:
	case 4800:
		idxInc = 3;
		break;

	default:
		idxInc = 1;
		goto normalize;
	}
	
	while ((idx + idxInc) < bitlen)
	{
		temp = (getBitVal(*datastream, idx)) << (idxInc - 1);

		if (idxInc == 3)
		{
			temp |= (getBitVal(*datastream, idx + 1) << 1);
		}

		temp |= getBitVal(*datastream, idx + (idxInc - 1));

		switch (temp)
		{
		case 0:
		case 1:
			break;

		case 2:
			temp = 3;
			break;

		case 3:
			temp = 2;
			break;

		case 4:
			temp = 7;
			break;

		case 5:
			temp = 6;
			break;

		case 6:
			temp = 4;
			break;

		case 7:
			temp = 5;
			break;

		default:
			std::cerr << "Invalid symbol detected in stream. This shouldn't be possible.\n\n";
			exit(1);
		}

		setBitVal(datastream, idx, (temp & (1 << (idxInc - 1))));

		if (idxInc == 3)
		{
			setBitVal(datastream, idx + 1, ((temp & 2) >> 1));
		}

		setBitVal(datastream, idx + idxInc - 1, temp & 1);

		idx += idxInc;
		numGroups++;
	}

	normalize:
	/* The plan here is to convert all the bitgroups, regardless of their length
		above, to three bits because the scrambler module needs them regardless
		of mode. */
		if (idxInc == 3) return bitlen;
		if (idxInc == 1)
		{
			numGroups = bitlen;
		}
		
		std::vector<byte> mgd_out(((numGroups * 3) / 8) + (((numGroups * 3) % 8 != 0) ? 1 : 0));
		
		for (size_t i = 0, optr = 0; i < bitlen, optr < (numGroups * 3); i += idxInc, optr += 3)
		{
			temp = (idxInc == 2) ? (getBitVal(*datastream, i) << 1) : getBitVal(*datastream, i);
			
			if (idxInc == 2) temp |= getBitVal(*datastream, i + 1);
			
			setBitVal(&mgd_out, optr, 0);
			setBitVal(&mgd_out, optr + 1, temp >> 1);
			setBitVal(&mgd_out, optr + 2, temp & 1);
		}
		
		datastream->clear();
		datastream->resize(mgd_out. size());
		std::copy(mgd_out. begin(), mgd_out.end(), datastream->begin());
		
		bitlen = numGroups * 3;
		
	return bitlen;
}