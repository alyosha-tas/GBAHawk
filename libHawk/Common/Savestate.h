#ifndef SAVESTATES_H
#define SAVESTATES_H

#pragma once

// Functions for saving and loading states, one variable at a time.
// Must save and load in the same order.

inline uint8_t* bool_saver(bool to_save, uint8_t* saver)
{
	*saver = (uint8_t)(to_save ? 1 : 0); saver++;

	return saver;
}

inline uint8_t* byte_saver(uint8_t to_save, uint8_t* saver)
{
	*saver = to_save; saver++;

	return saver;
}

inline uint8_t* short_saver(uint16_t to_save, uint8_t* saver)
{
	*saver = (uint8_t)(to_save & 0xFF); saver++;
	*saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;

	return saver;
}

inline uint8_t* int_saver(uint32_t to_save, uint8_t* saver)
{
	*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
	*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;

	return saver;
}

inline uint8_t* long_saver(uint64_t to_save, uint8_t* saver)
{
	*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
	*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;
	*saver = (uint8_t)((to_save >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 40) & 0xFF); saver++;
	*saver = (uint8_t)((to_save >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 56) & 0xFF); saver++;

	return saver;
}

inline uint8_t* bool_array_saver(bool* to_save, uint8_t* saver, int length)
{
	for (int i = 0; i < length; i++) { *saver = (uint8_t)(to_save[i] ? 1 : 0); saver++; }

	return saver;
}

inline uint8_t* byte_array_saver(uint8_t* to_save, uint8_t* saver, int length)
{
	for (int i = 0; i < length; i++) { *saver = to_save[i]; saver++; }

	return saver;
}

inline uint8_t* short_array_saver(uint16_t* to_save, uint8_t* saver, int length)
{
	for (int i = 0; i < length; i++)
	{ 
		*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
	}

	return saver;
}

inline uint8_t* int_array_saver(uint32_t* to_save, uint8_t* saver, int length)
{
	for (int i = 0; i < length; i++)
	{
		*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
		*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
	}

	return saver;
}

inline uint8_t* sint_array_saver(int32_t* to_save, uint8_t* saver, uint32_t length)
{
	for (uint32_t i = 0; i < length; i++)
	{
		*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
		*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
	}

	return saver;
}

inline uint8_t* long_array_saver(uint64_t* to_save, uint8_t* saver, int length)
{
	for (int i = 0; i < length; i++)
	{
		*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
		*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
		*saver = (uint8_t)((to_save[i] >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 40) & 0xFF); saver++;
		*saver = (uint8_t)((to_save[i] >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 56) & 0xFF); saver++;
	}

	return saver;
}

inline uint8_t* bool_loader(bool* to_load, uint8_t* loader)
{
	to_load[0] = *loader == 1; loader++;

	return loader;
}

inline uint8_t* byte_loader(uint8_t* to_load, uint8_t* loader)
{
	to_load[0] = *loader; loader++;

	return loader;
}

inline uint8_t* short_loader(uint16_t* to_load, uint8_t* loader)
{
	to_load[0] = *loader; loader++;
	to_load[0] |= ((uint16_t)(*loader) << 8); loader++;

	return loader;
}

inline uint8_t* int_loader(uint32_t* to_load, uint8_t* loader)
{
	to_load[0] = *loader; loader++; to_load[0] |= ((uint32_t)(*loader) << 8); loader++;
	to_load[0] |= ((uint32_t)(*loader) << 16); loader++; to_load[0] |= ((uint32_t)(*loader) << 24); loader++;

	return loader;
}

inline uint8_t* sint_loader(int32_t* to_load, uint8_t* loader)
{
	to_load[0] = *loader; loader++; to_load[0] |= ((int32_t)(*loader) << 8); loader++;
	to_load[0] |= ((int32_t)(*loader) << 16); loader++; to_load[0] |= ((int32_t)(*loader) << 24); loader++;

	return loader;
}

inline uint8_t* long_loader(uint64_t* to_load, uint8_t* loader)
{
	to_load[0] = *loader; loader++; to_load[0] |= (uint64_t)(* loader) << 8; loader++;
	to_load[0] |= (uint64_t)(*loader) << 16; loader++; to_load[0] |= (uint64_t)(*loader) << 24; loader++;
	to_load[0] |= (uint64_t)(*loader) << 32; loader++; to_load[0] |= (uint64_t)(*loader) << 40; loader++;
	to_load[0] |= (uint64_t)(*loader) << 48; loader++; to_load[0] |= (uint64_t)(*loader) << 56; loader++;

	return loader;
}

inline uint8_t* slong_loader(int64_t* to_load, uint8_t* loader)
{
	to_load[0] = *loader; loader++; to_load[0] |= (int64_t)(*loader) << 8; loader++;
	to_load[0] |= (int64_t)(*loader) << 16; loader++; to_load[0] |= (int64_t)(*loader) << 24; loader++;
	to_load[0] |= (int64_t)(*loader) << 32; loader++; to_load[0] |= (int64_t)(*loader) << 40; loader++;
	to_load[0] |= (int64_t)(*loader) << 48; loader++; to_load[0] |= (int64_t)(*loader) << 56; loader++;

	return loader;
}

inline uint8_t* bool_array_loader(bool* to_load, uint8_t* loader, int length)
{
	for (int i = 0; i < length; i++) { to_load[i] = *loader == 1; loader++; }

	return loader;
}

inline uint8_t* byte_array_loader(uint8_t* to_load, uint8_t* loader, int length)
{
	for (int i = 0; i < length; i++) { to_load[i] = *loader; loader++; }

	return loader;
}

inline uint8_t* short_array_loader(uint16_t* to_load, uint8_t* loader, int length)
{
	for (int i = 0; i < length; i++)
	{
		to_load[i] = *loader; loader++; to_load[i] |= ((uint16_t)(*loader) << 8); loader++;
	}

	return loader;
}

inline uint8_t* int_array_loader(uint32_t* to_load, uint8_t* loader, int length)
{
	for (int i = 0; i < length; i++)
	{
		to_load[i] = *loader; loader++; to_load[i] |= ((uint32_t)(*loader) << 8); loader++;
		to_load[i] |= ((uint32_t)(*loader) << 16); loader++; to_load[i] |= ((uint32_t)(*loader) << 24); loader++;
	}

	return loader;
}

inline uint8_t* sint_array_loader(int32_t* to_load, uint8_t* loader, uint32_t length)
{
	for (uint32_t i = 0; i < length; i++)
	{
		to_load[i] = *loader; loader++; to_load[i] |= ((int32_t)(*loader) << 8); loader++;
		to_load[i] |= ((int32_t)(*loader) << 16); loader++; to_load[i] |= ((int32_t)(*loader) << 24); loader++;
	}

	return loader;
}

inline uint8_t* long_array_loader(uint64_t* to_load, uint8_t* loader, int length)
{
	for (int i = 0; i < length; i++)
	{
		to_load[i] = *loader; loader++; to_load[i] |= (uint64_t)(*loader) << 8; loader++;
		to_load[i] |= (uint64_t)(*loader) << 16; loader++; to_load[i] |= (uint64_t)(*loader) << 24; loader++;
		to_load[i] |= (uint64_t)(*loader) << 32; loader++; to_load[i] |= (uint64_t)(*loader) << 40; loader++;
		to_load[i] |= (uint64_t)(*loader) << 48; loader++; to_load[i] |= (uint64_t)(*loader) << 56; loader++;
	}

	return loader;
}

#endif