#ifndef _MGD_DECODE_H
#define _MGD_DECODE_H

#include <iostream>
#include <vector>
#include "Types.h"

void mgdDecode(std::vector<byte>* datastream, size_t baud, size_t bitlen);

#endif