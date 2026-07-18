#ifndef MAPPERS_H
#define MAPPERS_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "../Common/Savestate.h"

using namespace std;

namespace SNESHawk
{
	class Mappers
	{
	public:
	#pragma region mapper base

		// not stated, initialized with power on
		bool Reset_RTC;
		bool RTC_Functional;
		bool Solar_Functional;
		uint8_t ROM_C4, ROM_C5, ROM_C6, ROM_C7, ROM_C8, ROM_C9;
		int16_t EEPROM_Offset;
		int16_t Flash_Write_Offset;
		int32_t Flash_Sector_Erase_Offset;
		int32_t Flash_Chip_Erase_Offset;

		uint16_t Flash_Type_64_Value;
		uint16_t Flash_Type_128_Value;

		// stated
		bool Command_Mode;
		bool Command_Mode_Switch;
		bool RTC_Clock_Low;
		bool RTC_Clock_High;
		bool RTC_Read;
		bool RTC_24_Hour;

		bool Ready_Flag;
		bool ADC_Ready_X, ADC_Ready_Y;
		bool Swapped_Out;
		bool Erase_Command;
		bool Erase_4k;
		bool Erase_All;
		bool Force_Bit_6;

		bool Chip_Select;
		bool Solar_Clock;
		bool Solar_Flag;
		bool Solar_Reset;
		bool Gyro_Clock;
		bool Gyro_Reset;
		bool Gyro_Bit;
		bool Rumble_Bit;
		bool Commit_Write_To_Reg;

		uint8_t Port_State;
		uint8_t Port_Dir;
		uint8_t Ports_RW;
		uint8_t CTRL_Reg;
		uint8_t CTRL_Reg_Commit;
		uint8_t CTRL_Reg_Latch;

		uint8_t Command_Byte;
		uint8_t RTC_SIO;
		uint8_t RTC_SIO_Old;
		uint8_t RTC_Latched_SIO;
		uint8_t Command_Bit, Command_Bit_Count;
		uint8_t Reg_Bit, Reg_Bit_Count;
		uint8_t Reg_Access;
		uint8_t Write_Value;
		uint8_t Write_Count; // for Atmel devices only

		uint8_t Reg_Year, Reg_Month, Reg_Day, Reg_Week, Reg_Hour, Reg_Minute, Reg_Second;

		uint16_t Solar_Clock_Count, Gyro_Clock_Count;

		uint32_t Size_Mask;
		uint32_t Bit_Offset, Bit_Read;
		uint32_t Access_Address;

		// 0 = ready for command
		// 2 = writing
		// 3 = reading
		// 5 = get address for write
		// 6 = get address for read
		uint32_t Current_State;

		// 0 - Ready
		// 1 - First comand write dne
		// 2 - Second command write done
		// 3 - Write a byte
		// 4 - Change Bank
		uint32_t Chip_Mode;
		uint32_t Next_State;
		uint32_t Bank_State;
		uint32_t Next_Mode;
		uint32_t Erase_4k_Addr;

		// video mapper variables
		uint32_t Video_Bank_Start_Address;
		uint32_t Video_ROM_Space_Address;
		uint32_t Video_Banks_to_Map;
		uint32_t Video_Command;

		uint64_t Next_Ready_Cycle;

		uint64_t RTC_Temp_Write;

		uint64_t* Core_Cycle_Count = nullptr;

		uint64_t* Core_Clock_Update_Cycle = nullptr;

		uint16_t* Core_Acc_X = nullptr;

		uint16_t* Core_Acc_Y = nullptr;

		uint8_t* Core_Solar = nullptr;

		uint8_t* Cart_RAM = nullptr;

		uint8_t* Core_Pointers_Read[512] = { };

		uint8_t* Core_Pointers_Write[512] = { };

		string* Core_Message_String = nullptr;

		void (*RumbleCallback)(bool);

		void (*MessageCallback)(int);

		Mappers()
		{
			Reset();
		}

		virtual uint8_t Read_Memory_High(uint32_t addr)
		{
			return 0;
		}

		virtual uint8_t Read_Memory_Low(uint32_t addr)
		{
			return 0;
		}

		virtual uint8_t Peek_Memory_High(uint32_t addr)
		{
			return 0;
		}

		virtual uint8_t Peek_Memory_Low(uint32_t addr)
		{
			return 0;
		}

		virtual void Write_Memory_High(uint32_t addr, uint8_t value)
		{
		}

		virtual void Write_Memory_Low(uint32_t addr, uint8_t value)
		{
		}

		virtual void Dispose()
		{
		}

		virtual void Reset()
		{
		}

		virtual void Update_State()
		{
		}

	#pragma endregion

	#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Command_Mode, saver);
			saver = bool_saver(Command_Mode_Switch, saver);
			saver = bool_saver(RTC_Clock_Low, saver);
			saver = bool_saver(RTC_Clock_High, saver);
			saver = bool_saver(RTC_Read, saver);
			saver = bool_saver(RTC_24_Hour, saver);

			saver = bool_saver(Ready_Flag, saver);
			saver = bool_saver(ADC_Ready_X, saver);
			saver = bool_saver(ADC_Ready_Y, saver);
			saver = bool_saver(Swapped_Out, saver);
			saver = bool_saver(Erase_Command, saver);
			saver = bool_saver(Erase_4k, saver);
			saver = bool_saver(Erase_All, saver);
			saver = bool_saver(Force_Bit_6, saver);

			saver = bool_saver(Chip_Select, saver);
			saver = bool_saver(Solar_Clock, saver);
			saver = bool_saver(Solar_Flag, saver);
			saver = bool_saver(Solar_Reset, saver);
			saver = bool_saver(Gyro_Clock, saver);
			saver = bool_saver(Gyro_Reset, saver);
			saver = bool_saver(Gyro_Bit, saver);
			saver = bool_saver(Rumble_Bit, saver);
			saver = bool_saver(Commit_Write_To_Reg, saver);

			saver = byte_saver(Port_State, saver);
			saver = byte_saver(Port_Dir, saver);
			saver = byte_saver(Ports_RW, saver);

			saver = byte_saver(Command_Byte, saver);
			saver = byte_saver(RTC_SIO, saver);
			saver = byte_saver(RTC_SIO_Old, saver);
			saver = byte_saver(RTC_Latched_SIO, saver);
			saver = byte_saver(Command_Bit, saver);
			saver = byte_saver(Command_Bit_Count, saver);
			saver = byte_saver(Reg_Bit, saver);
			saver = byte_saver(Reg_Bit_Count, saver);
			saver = byte_saver(Reg_Access, saver);
			saver = byte_saver(CTRL_Reg, saver);
			saver = byte_saver(Write_Value, saver);
			saver = byte_saver(Write_Count, saver);

			saver = byte_saver(Reg_Year, saver);
			saver = byte_saver(Reg_Month, saver);
			saver = byte_saver(Reg_Day, saver);
			saver = byte_saver(Reg_Week, saver);
			saver = byte_saver(Reg_Hour, saver);
			saver = byte_saver(Reg_Minute, saver);
			saver = byte_saver(Reg_Second, saver);
			saver = byte_saver(CTRL_Reg_Commit, saver);
			saver = byte_saver(CTRL_Reg_Latch, saver);

			saver = short_saver(Solar_Clock_Count, saver);
			saver = short_saver(Gyro_Clock_Count, saver);

			saver = int_saver(Size_Mask, saver);
			saver = int_saver(Bit_Offset, saver);
			saver = int_saver(Bit_Read, saver);
			saver = int_saver(Access_Address, saver);
			saver = int_saver(Current_State, saver);
			saver = int_saver(Chip_Mode, saver);
			saver = int_saver(Next_State, saver);
			saver = int_saver(Bank_State, saver);
			saver = int_saver(Next_Mode, saver);
			saver = int_saver(Erase_4k_Addr, saver);
			saver = int_saver(Video_Bank_Start_Address, saver);
			saver = int_saver(Video_ROM_Space_Address, saver);
			saver = int_saver(Video_Banks_to_Map, saver);
			saver = int_saver(Video_Command, saver);

			saver = long_saver(Next_Ready_Cycle, saver);
			saver = long_saver(RTC_Temp_Write, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Command_Mode, loader);
			loader = bool_loader(&Command_Mode_Switch, loader);
			loader = bool_loader(&RTC_Clock_Low, loader);
			loader = bool_loader(&RTC_Clock_High, loader);
			loader = bool_loader(&RTC_Read, loader);
			loader = bool_loader(&RTC_24_Hour, loader);

			loader = bool_loader(&Ready_Flag, loader);
			loader = bool_loader(&ADC_Ready_X, loader);
			loader = bool_loader(&ADC_Ready_Y, loader);
			loader = bool_loader(&Swapped_Out, loader);
			loader = bool_loader(&Erase_Command, loader);
			loader = bool_loader(&Erase_4k, loader);
			loader = bool_loader(&Erase_All, loader);
			loader = bool_loader(&Force_Bit_6, loader);

			loader = bool_loader(&Chip_Select, loader);
			loader = bool_loader(&Solar_Clock, loader);
			loader = bool_loader(&Solar_Flag, loader);
			loader = bool_loader(&Solar_Reset, loader);
			loader = bool_loader(&Gyro_Clock, loader);
			loader = bool_loader(&Gyro_Reset, loader);
			loader = bool_loader(&Gyro_Bit, loader);
			loader = bool_loader(&Rumble_Bit, loader);
			loader = bool_loader(&Commit_Write_To_Reg, loader);

			loader = byte_loader(&Port_State, loader);
			loader = byte_loader(&Port_Dir, loader);
			loader = byte_loader(&Ports_RW, loader);

			loader = byte_loader(&Command_Byte, loader);
			loader = byte_loader(&RTC_SIO, loader);
			loader = byte_loader(&RTC_SIO_Old, loader);
			loader = byte_loader(&RTC_Latched_SIO, loader);
			loader = byte_loader(&Command_Bit, loader);
			loader = byte_loader(&Command_Bit_Count, loader);
			loader = byte_loader(&Reg_Bit, loader);
			loader = byte_loader(&Reg_Bit_Count, loader);
			loader = byte_loader(&Reg_Access, loader);
			loader = byte_loader(&CTRL_Reg, loader);
			loader = byte_loader(&Write_Value, loader);
			loader = byte_loader(&Write_Count, loader);

			loader = byte_loader(&Reg_Year, loader);
			loader = byte_loader(&Reg_Month, loader);
			loader = byte_loader(&Reg_Day, loader);
			loader = byte_loader(&Reg_Week, loader);
			loader = byte_loader(&Reg_Hour, loader);
			loader = byte_loader(&Reg_Minute, loader);
			loader = byte_loader(&Reg_Second, loader);
			loader = byte_loader(&CTRL_Reg_Commit, loader);
			loader = byte_loader(&CTRL_Reg_Latch, loader);

			loader = short_loader(&Solar_Clock_Count, loader);
			loader = short_loader(&Gyro_Clock_Count, loader);

			loader = int_loader(&Size_Mask, loader);
			loader = int_loader(&Bit_Offset, loader);
			loader = int_loader(&Bit_Read, loader);
			loader = int_loader(&Access_Address, loader);
			loader = int_loader(&Current_State, loader);
			loader = int_loader(&Chip_Mode, loader);
			loader = int_loader(&Next_State, loader);
			loader = int_loader(&Bank_State, loader);
			loader = int_loader(&Next_Mode, loader);
			loader = int_loader(&Erase_4k_Addr, loader);
			loader = int_loader(&Video_Bank_Start_Address, loader);
			loader = int_loader(&Video_ROM_Space_Address, loader);
			loader = int_loader(&Video_Banks_to_Map, loader);
			loader = int_loader(&Video_Command, loader);

			loader = long_loader(&Next_Ready_Cycle, loader);
			loader = long_loader(&RTC_Temp_Write, loader);

			return loader;
		}

	#pragma endregion

	};

	#pragma region Default Lo ROM

	class Mapper_Lo_ROM : public Mappers
	{
	public:

		void Reset()
		{
			// nothing to initialize
		}

		uint8_t Read_Memory_High(uint32_t addr)
		{	
			return Core_Pointers_Read[(addr >> 15) & 0x1FF][addr & 0x7FFF];
		}

		void Write_Memory_High(uint32_t addr, uint8_t value)
		{
			Core_Pointers_Write[(addr >> 15) & 0x1FF][addr & 0x7FFF] = value;
		}

		uint8_t Peek_Memory_High(uint32_t addr)
		{
			return Read_Memory_High(addr);
		}

		uint8_t Read_Memory_Low(uint32_t addr)
		{
			uint8_t base = (addr >> 16) & 0xFF;
			
			if ((base >= 0x7E) && (base < 0x80))
			{
				// RAM
				return Core_Pointers_Read[(addr >> 15) & 0x1FF][addr & 0x7FFF];
			}
			else if ((base >= 0x70) && (base < 0x7E))
			{
				// SRAM if applicable
				return 0;
			}
			else
			{
				return 0;
			}
		}

		void Write_Memory_Low(uint32_t addr, uint8_t value)
		{
			uint8_t base = (addr >> 16) & 0xFF;

			if ((base >= 0x7E) && (base < 0x80))
			{
				// RAM
				Core_Pointers_Write[(addr >> 15) & 0x1FF][addr & 0x7FFF] = value;
			}
			else if ((base >= 0x70) && (base < 0x7E))
			{
				// SRAM if applicable

			}
		}

		uint8_t Peek_Memory_Low(uint32_t addr)
		{
			return Read_Memory_Low(addr);
		}
	};

#pragma endregion

	#pragma region Default Hi ROM

	class Mapper_Hi_ROM : public Mappers
	{
	public:

		void Reset()
		{
			// nothing to initialize
		}

		uint8_t Read_Memory_High(uint32_t addr)
		{
			return Core_Pointers_Read[(addr >> 15) & 0x1FF][addr & 0x7FFF];
		}

		void Write_Memory_High(uint32_t addr, uint8_t value)
		{
			Core_Pointers_Write[(addr >> 15) & 0x1FF][addr & 0x7FFF] = value;
		}

		uint8_t Peek_Memory_High(uint32_t addr)
		{
			return Read_Memory_High(addr);
		}

		uint8_t Read_Memory_Low(uint32_t addr)
		{
			uint8_t base = (addr >> 16) & 0xFF;

			if ((base >= 0x7E) && (base < 0x80))
			{
				// RAM
				return Core_Pointers_Read[(addr >> 15) & 0x1FF][addr & 0x7FFF];
			}
			else if ((base >= 0x70) && (base < 0x7E))
			{
				// SRAM if applicable
				return 0;
			}
			else
			{
				return 0;
			}
		}

		void Write_Memory_Low(uint32_t addr, uint8_t value)
		{
			uint8_t base = (addr >> 16) & 0xFF;

			if ((base >= 0x7E) && (base < 0x80))
			{
				// RAM
				Core_Pointers_Write[(addr >> 15) & 0x1FF][addr & 0x7FFF] = value;
			}
			else if ((base >= 0x70) && (base < 0x7E))
			{
				// SRAM if applicable

			}
		}

		uint8_t Peek_Memory_Low(uint32_t addr)
		{
			return Read_Memory_Low(addr);
		}
	};

	#pragma endregion

	#pragma region Default Ex Hi ROM

	class Mapper_Ex_Hi_ROM : public Mappers
	{
	public:

		void Reset()
		{
			// nothing to initialize
		}

		uint8_t Read_Memory_High(uint32_t addr)
		{
			return Core_Pointers_Read[(addr >> 15) & 0x1FF][addr & 0x7FFF];
		}

		void Write_Memory_High(uint32_t addr, uint8_t value)
		{
			Core_Pointers_Write[(addr >> 15) & 0x1FF][addr & 0x7FFF] = value;
		}

		uint8_t Peek_Memory_High(uint32_t addr)
		{
			return Read_Memory_High(addr);
		}

		uint8_t Read_Memory_Low(uint32_t addr)
		{
			uint8_t base = (addr >> 16) & 0xFF;

			if ((base >= 0x7E) && (base < 0x80))
			{
				// RAM
				return Core_Pointers_Read[(addr >> 15) & 0x1FF][addr & 0x7FFF];
			}
			else if ((base >= 0x70) && (base < 0x7E))
			{
				// SRAM if applicable
				return 0;
			}
			else
			{
				return 0;
			}
		}

		void Write_Memory_Low(uint32_t addr, uint8_t value)
		{
			uint8_t base = (addr >> 16) & 0xFF;

			if ((base >= 0x7E) && (base < 0x80))
			{
				// RAM
				Core_Pointers_Write[(addr >> 15) & 0x1FF][addr & 0x7FFF] = value;
			}
			else if ((base >= 0x70) && (base < 0x7E))
			{
				// SRAM if applicable

			}
		}

		uint8_t Peek_Memory_Low(uint32_t addr)
		{
			return Read_Memory_Low(addr);
		}
	};

	#pragma endregion

}

#endif
