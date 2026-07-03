#ifndef Flash_MAPPERS_H
#define Flash_MAPPERS_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace GBAHawk
{

	#pragma region FLASH

	class Mapper_FLASH : public Mappers
	{
	public:

		void Reset()
		{
			Write_Value = 0;
			Access_Address = 0;

			Bank_State = 0;
			Chip_Mode = 0;
			Next_Mode = 0;
			Erase_4k_Addr = 0;

			Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;

			Swapped_Out = false;
			Erase_Command = false;
			Erase_4k = false;
			Erase_All = false;
			Force_Bit_6 = false;
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			Update_State();

			uint8_t ret_value = 0;

			if (Swapped_Out)
			{
				if ((addr & 0xFFFF) > 1)
				{
					ret_value = Cart_RAM[(addr & 0xFFFF) + Bank_State];
				}
				else if ((addr & 0xFFFF) == 1)
				{
					if ((Size_Mask + 1) == 0x10000)
					{
						ret_value = (uint8_t)((Flash_Type_64_Value >> 8) & 0xFF);
					}
					else
					{
						ret_value = (uint8_t)((Flash_Type_128_Value >> 8) & 0xFF);
					}
				}
				else
				{
					if ((Size_Mask + 1) == 0x10000)
					{
						ret_value = (uint8_t)(Flash_Type_64_Value & 0xFF);
					}
					else
					{
						ret_value = (uint8_t)(Flash_Type_128_Value & 0xFF);
					}
				}
			}
			else
			{
				ret_value = Cart_RAM[(addr & 0xFFFF) + Bank_State];
			}

			// according to data sheet, upper bit returns 0 when an operation is in progress
			// this is important to ex Sonic Advance
			if (Next_Ready_Cycle != 0xFFFFFFFFFFFFFFFF)
			{
				if ((Size_Mask + 1) == 0x10000)
				{
					// according to data sheet, upper bit returns 0 when an operation is in progress
					// and the 6th bit alternates
					// this is important to ex Sonic Advance
					ret_value &= 0x7F;

					ret_value &= 0xBF;

					if (Force_Bit_6)
					{
						ret_value |= 0x40;
					}

					Force_Bit_6 ^= true;
				}
				else
				{
					// for the larger chips, it seems a status register is activated on writes, which
					// returns 0 until done (for our purposes since operations always succeed.)

					ret_value = 0;
				}
			}

			return ret_value;
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			// 8 bit bus only
			uint16_t ret = Read_Memory_8(addr & 0xFFFE);

			ret = (uint16_t)(ret | (ret << 8));
			return ret;
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			// 8 bit bus only
			uint32_t ret = Read_Memory_8(addr & 0xFFFC);

			ret = (uint32_t)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		void Write_Memory_8(uint32_t addr, uint8_t value)
		{
			Update_State();

			if (Chip_Mode == 3)
			{
				if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
				{
					Access_Address = (addr & 0xFFFF) + Bank_State;
					Write_Value = value;

					Next_Ready_Cycle = Core_Cycle_Count[0] + 325 + (uint64_t)Flash_Write_Offset;
				}

				Chip_Mode = 0;
			}
			else if ((addr & 0xFFFF) == 0x5555)
			{
				if (Chip_Mode == 0)
				{
					if (value == 0xAA)
					{
						Chip_Mode = 1;
					}
				}
				if (Chip_Mode == 2)
				{
					if (value == 0x10)
					{
						if (Erase_Command)
						{
							if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
							{
								Next_Ready_Cycle = Core_Cycle_Count[0] + 3 * 430000 + (uint64_t)Flash_Chip_Erase_Offset;

								Erase_All = true;

								Force_Bit_6 = false;
							}

							Erase_Command = false;

							Chip_Mode = 0;
						}
					}
					else if (value == 0x80)
					{
						Erase_Command = true;
						Chip_Mode = 0;
					}
					else if (value == 0x90)
					{
						Swapped_Out = true;
						Chip_Mode = 0;
					}
					else if (value == 0xA0)
					{
						Chip_Mode = 3;
					}
					else if (value == 0xB0)
					{
						if ((Size_Mask + 1) == 0x20000)
						{
							Chip_Mode = 4;
						}
					}
					else if (value == 0xF0)
					{
						Swapped_Out = false;
						Chip_Mode = 0;
					}
				}
			}
			else if ((addr & 0xFFFF) == 0x2AAA)
			{
				if (Chip_Mode == 1)
				{
					if (value == 0x55)
					{
						Chip_Mode = 2;
					}
				}
			}
			else if ((addr & 0xFFF) == 0)
			{
				if ((Chip_Mode == 2) && Erase_Command)
				{
					if (value == 0x30)
					{
						if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
						{
							Next_Ready_Cycle = Core_Cycle_Count[0] + 460000 + (uint64_t)Flash_Sector_Erase_Offset;

							Erase_4k = true;

							Erase_4k_Addr = (uint32_t)(addr & 0xF000);

							Force_Bit_6 = false;
						}

						Erase_Command = false;

						Chip_Mode = 0;
					}
				}

				if ((addr & 0xFFFF) == 0)
				{
					if (Chip_Mode == 4)
					{
						if (value == 1)
						{
							Bank_State = 0x10000;
						}
						else
						{
							Bank_State = 0;
						}

						Chip_Mode = 0;
					}
				}
			}
		}

		void Write_Memory_16(uint32_t addr, uint16_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)value);
			}
			else
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 8) & 0xFF));
			}
		}

		void Write_Memory_32(uint32_t addr, uint32_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)value);
			}
			else if ((addr & 3) == 1)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 8) & 0xFF));
			}
			else if ((addr & 3) == 2)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 16) & 0xFF));
			}
			else
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 24) & 0xFF));
			}
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Cart_RAM[(addr & 0xFFFF) + Bank_State];
		}

		void Update_State()
		{
			if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
			{
				if (Erase_4k)
				{
					for (uint32_t i = 0; i < 0x1000; i++)
					{
						Cart_RAM[i + Erase_4k_Addr + Bank_State] = 0xFF;
					}

					Erase_4k = false;
				}
				else if (Erase_All)
				{
					for (uint32_t i = 0; i < (Size_Mask + 1); i++)
					{
						Cart_RAM[i] = 0xFF;
					}

					Erase_All = false;
				}
				else
				{
					Cart_RAM[Access_Address] = Write_Value;
				}

				Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;
			}
		}
	};
	#pragma endregion

	#pragma region FLASH RTC

	class Mapper_FLASH_RTC : public Mappers
	{
	public:

		void Reset()
		{
			Bank_State = 0;
			Chip_Mode = 0;
			Next_Mode = 0;
			Erase_4k_Addr = 0;

			Access_Address = 0;
			Write_Value = 0;

			Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;

			Swapped_Out = false;
			Erase_Command = false;
			Erase_4k = false;
			Erase_All = false;
			Force_Bit_6 = false;

			// set up initial variables for IO
			Chip_Select = false;

			Port_State = 0;
			Port_Dir = 0;
			Ports_RW = 0;

			Current_C4 = ROM_C4;
			Current_C5 = ROM_C5;
			Current_C6 = ROM_C6;
			Current_C7 = ROM_C7;
			Current_C8 = ROM_C8;
			Current_C9 = ROM_C9;

			Core_ROM[0xC4] = Current_C4;
			Core_ROM[0xC5] = Current_C5;
			Core_ROM[0xC6] = Current_C6;
			Core_ROM[0xC7] = Current_C7;
			Core_ROM[0xC8] = Current_C8;
			Core_ROM[0xC9] = Current_C9;

			// set up initial variables for RTC
			Command_Mode = true;
			Command_Mode_Switch = false;
			Commit_Write_To_Reg = false;
			RTC_Clock_Low = false;
			RTC_Clock_High = false;
			RTC_Read = false;

			Command_Byte = 0;
			RTC_SIO = 0;
			RTC_SIO_Old = 0;
			RTC_Latched_SIO = 0;
			Command_Bit = Command_Bit_Count = 0;
			Reg_Bit = Reg_Bit_Count = 0;
			Reg_Access = 0;

			RTC_Temp_Write = 0;
			CTRL_Reg_Commit = 0;
			CTRL_Reg_Latch = 0;

			if (Reset_RTC)
			{
				RTC_24_Hour = false;
				Reg_Year = Reg_Week = Reg_Hour = Reg_Minute = Reg_Second = 0;
				Reg_Day = Reg_Month = 1;

				CTRL_Reg = 0;
			}
		}

		void Write_ROM_8(uint32_t addr, uint8_t value)
		{
			Write_IO_RTC(addr, value);
		}

		void Write_ROM_16(uint32_t addr, uint16_t value)
		{
			Write_ROM_8(addr, (uint8_t)value);
			Write_ROM_8((addr + 1), (uint8_t)(value >> 8));
		}

		void Write_ROM_32(uint32_t addr, uint32_t value)
		{
			Write_ROM_8(addr, (uint8_t)value);
			Write_ROM_8((addr + 1), (uint8_t)(value >> 8));
			Write_ROM_8((addr + 2), (uint8_t)(value >> 16));
			Write_ROM_8((addr + 3), (uint8_t)(value >> 24));
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			Update_State();

			uint8_t ret_value = 0;

			if (Swapped_Out)
			{
				if ((addr & 0xFFFF) > 1)
				{
					ret_value = Cart_RAM[(addr & 0xFFFF) + Bank_State];
				}
				else if ((addr & 0xFFFF) == 1)
				{
					if ((Size_Mask + 1) == 0x10000)
					{
						ret_value = (uint8_t)((Flash_Type_64_Value >> 8) & 0xFF);
					}
					else
					{
						ret_value = (uint8_t)((Flash_Type_128_Value >> 8) & 0xFF);
					}
				}
				else
				{
					if ((Size_Mask + 1) == 0x10000)
					{
						ret_value = (uint8_t)(Flash_Type_64_Value & 0xFF);
					}
					else
					{
						ret_value = (uint8_t)(Flash_Type_128_Value & 0xFF);
					}
				}
			}
			else
			{
				ret_value = Cart_RAM[(addr & 0xFFFF) + Bank_State];
			}

			// according to data sheet, upper bit returns 0 when an operation is in progress
			// this is important to ex Sonic Advance
			if (Next_Ready_Cycle != 0xFFFFFFFFFFFFFFFF)
			{
				if ((Size_Mask + 1) == 0x10000)
				{
					// according to data sheet, upper bit returns 0 when an operation is in progress
					// and the 6th bit alternates
					// this is important to ex Sonic Advance
					ret_value &= 0x7F;

					ret_value &= 0xBF;

					if (Force_Bit_6)
					{
						ret_value |= 0x40;
					}

					Force_Bit_6 ^= true;
				}
				else
				{
					// for the larger chips, it seems a status register is activated on writes, which
					// returns 0 until done (for our purposes since operations always succeed.)

					ret_value = 0;
				}
			}

			return ret_value;
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			// 8 bit bus only
			uint16_t ret = Read_Memory_8(addr & 0xFFFE);

			ret = (uint16_t)(ret | (ret << 8));
			return ret;
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			// 8 bit bus only
			uint32_t ret = Read_Memory_8(addr & 0xFFFC);

			ret = (uint32_t)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		void Write_Memory_8(uint32_t addr, uint8_t value)
		{
			Update_State();

			if (Chip_Mode == 3)
			{
				if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
				{
					Access_Address = (addr & 0xFFFF) + Bank_State;
					Write_Value = value;

					Next_Ready_Cycle = Core_Cycle_Count[0] + 325 + (uint64_t)Flash_Write_Offset;
				}

				Chip_Mode = 0;
			}
			else if ((addr & 0xFFFF) == 0x5555)
			{
				if (Chip_Mode == 0)
				{
					if (value == 0xAA)
					{
						Chip_Mode = 1;
					}
				}
				if (Chip_Mode == 2)
				{
					if (value == 0x10)
					{
						if (Erase_Command)
						{
							if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
							{
								Next_Ready_Cycle = Core_Cycle_Count[0] + 3 * 430000 + (uint64_t)Flash_Chip_Erase_Offset;

								Erase_All = true;

								Force_Bit_6 = false;
							}

							Erase_Command = false;

							Chip_Mode = 0;
						}
					}
					else if (value == 0x80)
					{
						Erase_Command = true;
						Chip_Mode = 0;
					}
					else if (value == 0x90)
					{
						Swapped_Out = true;
						Chip_Mode = 0;
					}
					else if (value == 0xA0)
					{
						Chip_Mode = 3;
					}
					else if (value == 0xB0)
					{
						if ((Size_Mask + 1) == 0x20000)
						{
							Chip_Mode = 4;
						}
					}
					else if (value == 0xF0)
					{
						Swapped_Out = false;
						Chip_Mode = 0;
					}
				}
			}
			else if ((addr & 0xFFFF) == 0x2AAA)
			{
				if (Chip_Mode == 1)
				{
					if (value == 0x55)
					{
						Chip_Mode = 2;
					}
				}
			}
			else if ((addr & 0xFFF) == 0)
			{
				if ((Chip_Mode == 2) && Erase_Command)
				{
					if (value == 0x30)
					{
						if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
						{
							Next_Ready_Cycle = Core_Cycle_Count[0] + 460000 + (uint64_t)Flash_Sector_Erase_Offset;

							Erase_4k = true;

							Erase_4k_Addr = (uint32_t)(addr & 0xF000);

							Force_Bit_6 = false;
						}

						Erase_Command = false;

						Chip_Mode = 0;
					}
				}

				if ((addr & 0xFFFF) == 0)
				{
					if (Chip_Mode == 4)
					{
						if (value == 1)
						{
							Bank_State = 0x10000;
						}
						else
						{
							Bank_State = 0;
						}

						Chip_Mode = 0;
					}
				}
			}
		}

		void Write_Memory_16(uint32_t addr, uint16_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)value);
			}
			else
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 8) & 0xFF));
			}
		}

		void Write_Memory_32(uint32_t addr, uint32_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)value);
			}
			else if ((addr & 3) == 1)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 8) & 0xFF));
			}
			else if ((addr & 3) == 2)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 16) & 0xFF));
			}
			else
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 24) & 0xFF));
			}
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Cart_RAM[(addr & 0xFFFF) + Bank_State];
		}

		void Update_State()
		{
			if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
			{
				if (Erase_4k)
				{
					for (uint32_t i = 0; i < 0x1000; i++)
					{
						Cart_RAM[i + Erase_4k_Addr + Bank_State] = 0xFF;
					}

					Erase_4k = false;
				}
				else if (Erase_All)
				{
					for (uint32_t i = 0; i < (Size_Mask + 1); i++)
					{
						Cart_RAM[i] = 0xFF;
					}

					Erase_All = false;
				}
				else
				{
					Cart_RAM[Access_Address] = Write_Value;
				}

				Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;
			}
		}
	};
	#pragma endregion

	#pragma region FLASH Atmel

	class Mapper_FLASH_Atmel : public Mappers
	{
	public:

		void Reset()
		{
			Write_Value = 0;
			Access_Address = 0;

			Bank_State = 0;
			Chip_Mode = 0;
			Next_Mode = 0;
			Erase_4k_Addr = 0;

			Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;

			Swapped_Out = false;
			Erase_Command = false;
			Erase_4k = false;
			Erase_All = false;
			Force_Bit_6 = false;
			Write_Count = 0;
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			Update_State();

			uint8_t ret_value = 0;

			if (Swapped_Out)
			{
				if ((addr & 0xFFFF) > 1)
				{
					ret_value = Cart_RAM[(addr & 0xFFFF) + Bank_State];
				}
				else if ((addr & 0xFFFF) == 1)
				{
					ret_value = (uint8_t)((Flash_Type_64_Value >> 8) & 0xFF);
				}
				else
				{
					ret_value = (uint8_t)(Flash_Type_64_Value & 0xFF);
				}
			}
			else
			{
				ret_value = Cart_RAM[(addr & 0xFFFF) + Bank_State];
			}

			// according to data sheet, upper bit returns 0 when an operation is in progress
			// this is important to ex Sonic Advance
			if (Next_Ready_Cycle != 0xFFFFFFFFFFFFFFFF)
			{
				// according to data sheet, upper bit returns complement of last written byte when an operation is in progress
				// and the 6th bit alternates
				ret_value &= 0x7F;

				ret_value |= ((~Write_Value) & 0x80);

				ret_value &= 0xBF;

				if (Force_Bit_6)
				{
					ret_value |= 0x40;
				}

				Force_Bit_6 ^= true;
			}

			return ret_value;
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			// 8 bit bus only
			uint16_t ret = Read_Memory_8(addr & 0xFFFE);

			ret = (uint16_t)(ret | (ret << 8));
			return ret;
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			// 8 bit bus only
			uint32_t ret = Read_Memory_8(addr & 0xFFFC);

			ret = (uint32_t)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		void Write_Memory_8(uint32_t addr, uint8_t value)
		{
			Update_State();

			if (Chip_Mode == 3)
			{
				if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
				{
					Access_Address = (addr & 0xFFFF) + Bank_State;
					Write_Value = value;

					Write_Count += 1;

					if (Write_Count == 128)
					{
						Next_Ready_Cycle = Core_Cycle_Count[0] + 30000 + (uint64_t)Flash_Sector_Erase_Offset;

						Chip_Mode = 0;
					}
					else
					{
						Next_Ready_Cycle = Core_Cycle_Count[0] + 1;
					}
				}
			}
			else if ((addr & 0xFFFF) == 0x5555)
			{
				if (Chip_Mode == 0)
				{
					if (value == 0xAA)
					{
						Chip_Mode = 1;
					}
				}
				if (Chip_Mode == 2)
				{
					if (value == 0x10)
					{
						if (Erase_Command)
						{
							if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
							{
								Next_Ready_Cycle = Core_Cycle_Count[0] + 3 * 430000 + (uint64_t)Flash_Chip_Erase_Offset;

								Erase_All = true;

								Force_Bit_6 = false;
							}

							Erase_Command = false;

							Chip_Mode = 0;
						}
					}
					else if (value == 0x80)
					{
						Erase_Command = true;
						Chip_Mode = 0;
					}
					else if (value == 0x90)
					{
						Swapped_Out = true;
						Chip_Mode = 0;
					}
					else if (value == 0xA0)
					{
						Chip_Mode = 3;
						Write_Count = 0;
					}
					else if (value == 0xF0)
					{
						Swapped_Out = false;
						Chip_Mode = 0;
					}
				}
			}
			else if ((addr & 0xFFFF) == 0x2AAA)
			{
				if (Chip_Mode == 1)
				{
					if (value == 0x55)
					{
						Chip_Mode = 2;
					}
				}
			}
		}

		void Write_Memory_16(uint32_t addr, uint16_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)value);
			}
			else
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 8) & 0xFF));
			}
		}

		void Write_Memory_32(uint32_t addr, uint32_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)value);
			}
			else if ((addr & 3) == 1)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 8) & 0xFF));
			}
			else if ((addr & 3) == 2)
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 16) & 0xFF));
			}
			else
			{
				Write_Memory_8((addr & 0xFFFF), (uint8_t)((value >> 24) & 0xFF));
			}
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Cart_RAM[(addr & 0xFFFF) + Bank_State];
		}

		void Update_State()
		{
			if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
			{
				if (Erase_4k)
				{
					for (uint32_t i = 0; i < 0x1000; i++)
					{
						Cart_RAM[i + Erase_4k_Addr + Bank_State] = 0xFF;
					}

					Erase_4k = false;
				}
				else if (Erase_All)
				{
					for (uint32_t i = 0; i < (Size_Mask + 1); i++)
					{
						Cart_RAM[i] = 0xFF;
					}

					Erase_All = false;
				}
				else
				{
					Cart_RAM[Access_Address] = Write_Value;
				}

				Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;
			}
		}
	};
	#pragma endregion

}

#endif