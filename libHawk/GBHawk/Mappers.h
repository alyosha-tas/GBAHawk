#ifndef MAPPERS_H
#define MAPPERS_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace std;

namespace GBHawk
{
	class Mappers
	{
	public:
	#pragma region mapper base
 
		uint16_t* Core_Acc_X_State = nullptr;

		uint16_t* Core_Acc_Y_State = nullptr;

		uint16_t* Core_Addr_Access = nullptr;

		uint8_t* Core_ROM = nullptr;

		uint32_t* Core_ROM_Length = nullptr;

		uint8_t* Core_Bus_Value = nullptr;

		uint8_t* Core_Cart_RAM = nullptr;

		uint32_t* Core_Cart_RAM_Length = nullptr;

		uint64_t* Core_Cycle_Count = nullptr;

		uint64_t* Core_Bus_Access_Time = nullptr;

		string* Core_Message_String = nullptr;

		void (*RumbleCallback)(bool);

		void (*MessageCallback)(int);
		
		Mappers()
		{
			Reset();
		}

		inline bool Get_Bit(uint8_t val, uint8_t bit)
		{
			return ((val & (1 << bit)) == (1 << bit));
		}

		virtual uint8_t ReadMemoryLow(uint16_t addr)
		{
			return 0;
		}

		virtual uint8_t ReadMemoryHigh(uint16_t addr)
		{
			return 0;
		}

		virtual uint8_t PeekMemoryLow(uint16_t addr)
		{
			return 0;
		}

		virtual uint8_t PeekMemoryHigh(uint16_t addr)
		{
			return 0;
		}

		virtual void WriteMemory(uint16_t addr, uint8_t value) { }

		virtual void PokeMemory(uint16_t addr, uint8_t value) { }

		virtual void Set_RTC(int32_t val, uint32_t param);

		virtual void Dispose() { }

		virtual void Reset() { }

		virtual void Mapper_Tick() { }

		virtual void RTC_Get(uint32_t value, uint32_t index) {}

	#pragma endregion

	#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{

			return loader;
		}

		uint8_t* bool_saver(bool to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save ? 1 : 0); saver++;

			return saver;
		}

		uint8_t* byte_saver(uint8_t to_save, uint8_t* saver)
		{
			*saver = to_save; saver++;

			return saver;
		}

		uint8_t* short_saver(uint16_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;

			return saver;
		}

		uint8_t* int_saver(uint32_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;

			return saver;
		}

		uint8_t* long_saver(uint64_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 40) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 56) & 0xFF); saver++;

			return saver;
		}

		uint8_t* bool_loader(bool* to_load, uint8_t* loader)
		{
			to_load[0] = *loader == 1; loader++;

			return loader;
		}

		uint8_t* byte_loader(uint8_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;

			return loader;
		}

		uint8_t* short_loader(uint16_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;
			to_load[0] |= ((uint16_t)(*loader) << 8); loader++;

			return loader;
		}

		uint8_t* int_loader(uint32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (uint32_t)(*loader << 8); loader++;
			to_load[0] |= (uint32_t)(*loader << 16); loader++; to_load[0] |= (uint32_t)(*loader << 24); loader++;

			return loader;
		}

		uint8_t* sint_loader(int32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= ((int32_t)(*loader) << 8); loader++;
			to_load[0] |= ((int32_t)(*loader) << 16); loader++; to_load[0] |= ((int32_t)(*loader) << 24); loader++;

			return loader;
		}

		uint8_t* long_loader(uint64_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (uint64_t)(*loader) << 8; loader++;
			to_load[0] |= (uint64_t)(*loader) << 16; loader++; to_load[0] |= (uint64_t)(*loader) << 24; loader++;
			to_load[0] |= (uint64_t)(*loader) << 32; loader++; to_load[0] |= (uint64_t)(*loader) << 40; loader++;
			to_load[0] |= (uint64_t)(*loader) << 48; loader++; to_load[0] |= (uint64_t)(*loader) << 56; loader++;

			return loader;
		}

		uint8_t* byte_array_saver(uint8_t* to_save, uint8_t* saver, uint32_t length)
		{
			for (uint32_t i = 0; i < length; i++) { *saver = to_save[i]; saver++; }

			return saver;
		}

		uint8_t* byte_array_loader(uint8_t* to_load, uint8_t* loader, uint32_t length)
		{
			for (uint32_t i = 0; i < length; i++) { to_load[i] = *loader; loader++; }

			return loader;
		}

	#pragma endregion

	};
}

#endif
