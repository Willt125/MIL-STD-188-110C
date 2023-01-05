#ifndef _TYPES_H
#define _TYPES_H

typedef unsigned char byte;


/*
	get/setBitVal() take in a bitstream and returns/sets
	a single bit with bit granularity. Takes/returns byte
	instead of bool just because.
*/
byte getBitVal(std::vector<byte> stream, size_t idx);
void setBitVal(std::vector<byte>* stream, size_t idx, byte val);

/*
	unaryXOR() takes a single byte and returns its parity.
	Also returns byte instead of bool just because.
*/
byte unaryXOR(byte val);

#endif