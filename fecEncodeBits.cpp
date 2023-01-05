#include <iostream>
#include <vector>
#include "fecEncodeBits.h"
#include "Types.h"

size_t fecEncodeBits(std::vector<byte>* datastream, size_t baud, size_t bitlen)
{
	byte fec_buffer = 0, fec_mask = 0x7f;
	byte valbuf;
	byte p1mask = 0x5b;
	byte p2mask = 0x79;
	byte p1tmp, p2tmp, p1out, p2out;
	size_t outsize = 1;

	switch (baud) // The encoding method is slightly different depending on the mode selected.
	{
	case 75:
	case 600:
	case 1200:
	case 2400:
	{
		outsize = 2;
		break;
	}

	case 150:
	{
		outsize = 8;
		break;
	}

	case 300:
	{
		outsize = 4;
		break;
	}

	case 4800:
	{
		return bitlen; // 4800-baud mode does not change the input stream.
	}
	}

	std::vector<byte> fec_out(((bitlen * outsize) / 8) + (((bitlen * outsize) % 8 != 0) ? 1 : 0)); // we will swap pointers with the input stream when we're done, making it look in-place. < You can't do that you idiot, the stack gets torn down

	valbuf = datastream->at(0); // sneaky extra buffer to help load the actual buffer

	/*
					HERE BE DRAGONS
					
					This is going to be some funky shit, because I don't know what I'm doing.
					I am attempting to iterate by single bits with byte granularity with zero
					education, foresight, or wisdom. Proceed with caution.
	*/

	fec_buffer |= (((valbuf & 0x80) >> 1) | 1); // The FEC buffer is only seven bits long, and the LSB is always set. The new bit gets inserted directly into the 7th (MSB) spot.
	valbuf <<= 1; // and move the sneaky buffer for next round

	for (size_t bitidx = 1, optr = 0; bitidx < bitlen; bitidx++)
	{
		switch (baud)
		{
		case 150: // I'm abusing case fallthrough here to achieve the different encodings, don't worry it's on purpose
			p1tmp = fec_buffer & p1mask;
			p2tmp = fec_buffer & p2mask;

			p1out = unaryXOR(p1tmp);
			p2out = unaryXOR(p2tmp);

			setBitVal(&fec_out, optr, p1out);
			setBitVal(&fec_out, optr + 1, p2out);
			setBitVal(&fec_out, optr + 2, p1out);
			setBitVal(&fec_out, optr + 3, p2out);
			optr += 4;

		case 300:

			p1tmp = fec_buffer & p1mask;
			p2tmp = fec_buffer & p2mask;

			p1out = unaryXOR(p1tmp);
			p2out = unaryXOR(p2tmp);

			setBitVal(&fec_out, optr, p1out);
			setBitVal(&fec_out, optr + 1, p2out);
			optr += 2;

		case 75:
		case 600:
		case 1200:
		case 2400:

			p1tmp = fec_buffer & p1mask;
			p2tmp = fec_buffer & p2mask;

			p1out = unaryXOR(p1tmp);
			p2out = unaryXOR(p2tmp);

			setBitVal(&fec_out, optr, p1out);
			setBitVal(&fec_out, optr + 1, p2out);
			optr += 2;
		}

		fec_buffer = (fec_buffer >> 1) | 1; // loading the next bit into the buffer
		fec_buffer |= ((valbuf & 0x80) >> 1) & fec_mask;
		valbuf <<= 1;
		


		if ((bitidx % 8) == 7 && (bitidx / 8 == datastream->size() - 1)) // every 8 rounds, reload the sneaky buffer from the input stream.
		{
			valbuf = datastream->at(bitidx / 8);
		}
		else if ((bitidx % 8) == 7) // the if statement is needed because of the shenanigans we use to index with bit granularity. More research needed to make sure it's bug-free.
		{
			valbuf = datastream->at(bitidx / 8 + 1);
		}
	}

	datastream->clear();
	datastream->resize(fec_out.size());

	std::copy(fec_out.begin(), fec_out.end(), datastream->begin()); // move the changed data back
	bitlen *= outsize; // update bitlen for the rest of the chain
	return bitlen;
}