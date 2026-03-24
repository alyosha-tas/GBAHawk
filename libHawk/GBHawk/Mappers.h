#ifndef MAPPERS_H
#define MAPPERS_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "../Common/Savestate.h"

using namespace std;

namespace GBHawk
{
	class Mappers
	{
	public:

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

		virtual void Set_RTC(int32_t val, uint32_t param) { }

		virtual void Dispose() { }

		virtual void Reset() { }

		virtual void Mapper_Tick() { }

		virtual uint8_t* SaveState(uint8_t* saver)
		{

			return saver;
		}

		virtual uint8_t* LoadState(uint8_t* loader)
		{

			return loader;
		}
	};
}

#endif
