#ifndef COMMON_OPS_H
#define COMMON_OPS_H

#pragma once

// Common operations across cores

inline bool Bit_Test(uint8_t val, uint32_t bit)
{
	return ((val >> bit) & 1) == 1;
}

inline bool Bit_Test(uint16_t val, uint32_t bit)
{
	return ((val >> bit) & 1) == 1;
}

inline bool Bit_Test(uint32_t val, uint32_t bit)
{
	return ((val >> bit) & 1) == 1;
}

inline bool Bit_Test(int32_t val, uint32_t bit)
{
	return ((val >> bit) & 1) == 1;
}

inline uint8_t BitReverse(uint8_t b)
{
	uint8_t ret = (uint8_t)(b >> 7);

	uint8_t bit_0 = (uint8_t)(b & 1);
	uint8_t bit_1 = (uint8_t)(b & 2);
	uint8_t bit_2 = (uint8_t)(b & 4);
	uint8_t bit_3 = (uint8_t)(b & 8);
	uint8_t bit_4 = (uint8_t)(b & 0x10);
	uint8_t bit_5 = (uint8_t)(b & 0x20);
	uint8_t bit_6 = (uint8_t)(b & 0x40);

	ret |= (uint8_t)(bit_0 << 7);
	ret |= (uint8_t)(bit_1 << 5);
	ret |= (uint8_t)(bit_2 << 3);
	ret |= (uint8_t)(bit_3 << 1);
	ret |= (uint8_t)(bit_4 >> 1);
	ret |= (uint8_t)(bit_5 >> 3);
	ret |= (uint8_t)(bit_6 >> 5);

	return ret;
}


#endif