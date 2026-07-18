#ifndef EEPROM_MAPPERS_H
#define EEPROM_MAPPERS_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace GBAHawk
{

	#pragma region EEPROM

	class Mapper_EEPROM : public Mappers
	{
	public:

		void Reset()
		{
			// set up initial variables
			Ready_Flag = true;

			Bit_Offset = Bit_Read = 0;

			Access_Address = 0;

			Current_State = 0;

			Next_State = 0;

			Next_Ready_Cycle = Core_Cycle_Count[0];
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			return 0xFF; // nothing mapped here
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			return 0xFFFF; // nothing mapped here
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			return 0xFFFFFFFF; // nothing mapped here
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		uint8_t Mapper_EEPROM_Read()
		{
			uint32_t cur_read_bit = 0;
			uint32_t cur_read_byte = 0;

			uint8_t ret = 0;

			if (Current_State == 0)
			{
				if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				return (uint8_t)(Ready_Flag ? 1 : 0);
			}
			else if (Current_State != 3)
			{
				// what to return in write state?
				return 0;
			}
			else
			{
				// what are the first 4 bits returned?
				if (Bit_Read < 4)
				{
					Bit_Read++;

					return 0;
				}
				else
				{
					cur_read_bit = Bit_Read - 4;
					cur_read_byte = cur_read_bit >> 3;

					cur_read_bit &= 7;

					cur_read_byte = 7 - cur_read_byte;
					cur_read_bit = 7 - cur_read_bit;

					ret = Cart_RAM[(Access_Address << 3) + cur_read_byte];

					ret >>= cur_read_bit;

					ret &= 1;

					Bit_Read++;

					if (Bit_Read == 68)
					{
						Bit_Read = 0;
						Current_State = 0;
						Ready_Flag = true;
					}

					return ret;
				}
			}
		}

		void Mapper_EEPROM_Write(uint8_t value)
		{
			uint32_t cur_write_bit = 0;
			uint32_t cur_write_byte = 0;

			if (Current_State == 0)
			{
				if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				if (Ready_Flag)
				{
					Access_Address = 0;

					if (Bit_Offset == 0)
					{
						Next_State = value & 1;
						Bit_Offset = 1;
					}
					else
					{
						Next_State <<= 1;
						Next_State |= (value & 1);

						Bit_Offset = 0;

						if (Next_State != 0)
						{
							Current_State = 4 + Next_State;
						}
						else
						{
							Current_State = 0;
						}
					}
				}
			}
			else if (Current_State == 2)
			{
				if (Bit_Read < 64)
				{
					cur_write_bit = Bit_Read;
					cur_write_byte = cur_write_bit >> 3;

					cur_write_bit &= 7;

					cur_write_byte = 7 - cur_write_byte;
					cur_write_bit = 7 - cur_write_bit;

					Cart_RAM[(Access_Address << 3) + cur_write_byte] &= (uint8_t)(~(1 << cur_write_bit));

					Cart_RAM[(Access_Address << 3) + cur_write_byte] |= (uint8_t)((value & 1) << cur_write_bit);
				}

				Bit_Read++;

				if (Bit_Read == 65)
				{
					if ((value & 1) == 0)
					{
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750 + (uint64_t)EEPROM_Offset;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750 + (uint64_t)EEPROM_Offset;
					}

					Ready_Flag = false;
				}
			}
			else if (Current_State == 3)
			{
				// Nothing occurs in read state?
			}
			else if (Current_State == 6)
			{
				// Get Address
				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 6)
					{
						Access_Address |= (uint32_t)(value & 1);
						Access_Address <<= 1;

						Bit_Offset++;

						if (Bit_Offset == 6)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3F;

							// now write the data to the EEPROM
							Bit_Offset = 0;
							Current_State = 2;
						}
					}
				}
				else
				{
					if (Bit_Offset < 14)
					{
						Access_Address |= (uint32_t)(value & 1);
						Access_Address <<= 1;

						Bit_Offset++;

						if (Bit_Offset == 14)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3FF;

							// now write the data to the EEPROM
							Bit_Offset = 0;
							Current_State = 2;
						}
					}
				}
			}
			else if (Current_State == 7)
			{
				// Get Address for reading and wait for zero bit

				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 7)
					{
						if (Bit_Offset < 6)
						{
							Access_Address |= (uint32_t)(value & 1);
							Access_Address <<= 1;
						}

						Bit_Offset++;

						if (Bit_Offset == 7)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3F;

							if ((value & 1) == 0)
							{
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
							else
							{
								// error? seems to ignore this bit even though GBA tek says it should be zero
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
						}
					}
				}
				else
				{
					if (Bit_Offset < 15)
					{
						if (Bit_Offset < 14)
						{
							Access_Address |= (uint32_t)(value & 1);
							Access_Address <<= 1;
						}

						Bit_Offset++;

						if (Bit_Offset == 15)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3FF;

							if ((value & 1) == 0)
							{
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
							else
							{
								// error? seems to ignore this bit even though GBA tek says it should be zero
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
						}
					}
				}
			}
		}
	};

	#pragma endregion

	#pragma region EEPROM Solar

	class Mapper_EEPROM_Solar : public Mappers
	{
	public:

		void Reset()
		{
			// set up initial variables for Solar
			Ready_Flag = true;
			Chip_Select = false;
			Solar_Clock = false;
			Solar_Flag = false;
			Solar_Flag = false;

			Port_State = 0;
			Port_Dir = 0;
			Ports_RW = 0;

			Current_C4 = ROM_C4;
			Current_C5 = ROM_C5;
			Current_C6 = ROM_C6;
			Current_C7 = ROM_C7;
			Current_C8 = ROM_C8;
			Current_C9 = ROM_C9;

			Solar_Clock_Count = 0;

			Core_ROM[0xC4] = Current_C4;
			Core_ROM[0xC5] = Current_C5;
			Core_ROM[0xC6] = Current_C6;
			Core_ROM[0xC7] = Current_C7;
			Core_ROM[0xC8] = Current_C8;
			Core_ROM[0xC9] = Current_C9;

			Bit_Offset = Bit_Read = 0;

			Access_Address = 0;

			Current_State = 0;

			Next_State = 0;

			Next_Ready_Cycle = Core_Cycle_Count[0];

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

		uint8_t Read_Memory_8(uint32_t addr)
		{
			return 0xFF; // nothing mapped here
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			return 0xFFFF; // nothing mapped here
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			return 0xFFFFFFFF; // nothing mapped here
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Read_Memory_8(addr);
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

		uint8_t Mapper_EEPROM_Read()
		{
			uint32_t cur_read_bit = 0;
			uint32_t cur_read_byte = 0;

			uint8_t ret = 0;

			if (Current_State == 0)
			{
				if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				return (uint8_t)(Ready_Flag ? 1 : 0);
			}
			else if (Current_State != 3)
			{
				// what to return in write state?
				return 0;
			}
			else
			{
				// what are the first 4 bits returned?
				if (Bit_Read < 4)
				{
					Bit_Read++;

					return 0;
				}
				else
				{
					cur_read_bit = Bit_Read - 4;
					cur_read_byte = cur_read_bit >> 3;

					cur_read_bit &= 7;

					cur_read_byte = 7 - cur_read_byte;
					cur_read_bit = 7 - cur_read_bit;

					ret = Cart_RAM[(Access_Address << 3) + cur_read_byte];

					ret >>= cur_read_bit;

					ret &= 1;

					Bit_Read++;

					if (Bit_Read == 68)
					{
						Bit_Read = 0;
						Current_State = 0;
						Ready_Flag = true;
					}

					return ret;
				}
			}
		}

		void Mapper_EEPROM_Write(uint8_t value)
		{
			uint32_t cur_write_bit = 0;
			uint32_t cur_write_byte = 0;

			if (Current_State == 0)
			{
				if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				if (Ready_Flag)
				{
					Access_Address = 0;

					if (Bit_Offset == 0)
					{
						Next_State = value & 1;
						Bit_Offset = 1;
					}
					else
					{
						Next_State <<= 1;
						Next_State |= (value & 1);

						Bit_Offset = 0;

						if (Next_State != 0)
						{
							Current_State = 4 + Next_State;
						}
						else
						{
							Current_State = 0;
						}
					}
				}
			}
			else if (Current_State == 2)
			{
				if (Bit_Read < 64)
				{
					cur_write_bit = Bit_Read;
					cur_write_byte = cur_write_bit >> 3;

					cur_write_bit &= 7;

					cur_write_byte = 7 - cur_write_byte;
					cur_write_bit = 7 - cur_write_bit;

					Cart_RAM[(Access_Address << 3) + cur_write_byte] &= (uint8_t)(~(1 << cur_write_bit));

					Cart_RAM[(Access_Address << 3) + cur_write_byte] |= (uint8_t)((value & 1) << cur_write_bit);
				}

				Bit_Read++;

				if (Bit_Read == 65)
				{
					if ((value & 1) == 0)
					{
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750 + (uint64_t)EEPROM_Offset;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750 + (uint64_t)EEPROM_Offset;
					}

					Ready_Flag = false;
				}
			}
			else if (Current_State == 3)
			{
				// Nothing occurs in read state?
			}
			else if (Current_State == 6)
			{
				// Get Address
				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 6)
					{
						Access_Address |= (uint32_t)(value & 1);
						Access_Address <<= 1;

						Bit_Offset++;

						if (Bit_Offset == 6)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3F;

							// now write the data to the EEPROM
							Bit_Offset = 0;
							Current_State = 2;
						}
					}
				}
				else
				{
					if (Bit_Offset < 14)
					{
						Access_Address |= (uint32_t)(value & 1);
						Access_Address <<= 1;

						Bit_Offset++;

						if (Bit_Offset == 14)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3FF;

							// now write the data to the EEPROM
							Bit_Offset = 0;
							Current_State = 2;
						}
					}
				}
			}
			else if (Current_State == 7)
			{
				// Get Address for reading and wait for zero bit

				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 7)
					{
						if (Bit_Offset < 6)
						{
							Access_Address |= (uint32_t)(value & 1);
							Access_Address <<= 1;
						}

						Bit_Offset++;

						if (Bit_Offset == 7)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3F;

							if ((value & 1) == 0)
							{
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
							else
							{
								// error? seems to ignore this bit even though GBA tek says it should be zero
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
						}
					}
				}
				else
				{
					if (Bit_Offset < 15)
					{
						if (Bit_Offset < 14)
						{
							Access_Address |= (uint32_t)(value & 1);
							Access_Address <<= 1;
						}

						Bit_Offset++;

						if (Bit_Offset == 15)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3FF;

							if ((value & 1) == 0)
							{
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
							else
							{
								// error? seems to ignore this bit even though GBA tek says it should be zero
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
						}
					}
				}
			}
		}
	};

	#pragma endregion

	#pragma region EEPROM_Tilt

	class Mapper_EEPROM_Tilt : public Mappers
	{
	public:

		void Reset()
		{
			// set up initial variables
			Ready_Flag = true;

			Bit_Offset = Bit_Read = 0;

			Access_Address = 0;

			Current_State = 0;

			Next_State = 0;

			Next_Ready_Cycle = Core_Cycle_Count[0];

			ADC_Ready_X = ADC_Ready_Y = false;
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			if (addr == 0x8200)
			{
				return (uint8_t)(Core_Acc_X[0]);
			}
			else if (addr == 0x8300)
			{
				return (uint8_t)(((Core_Acc_X[0] >> 8) & 0xF) | (ADC_Ready_X ? 0x80 : 0));
			}
			else if (addr == 0x8400)
			{
				return (uint8_t)(Core_Acc_Y[0]);
			}
			else if (addr == 0x8500)
			{
				return (uint8_t)(((Core_Acc_Y[0] >> 8) & 0xF) | (ADC_Ready_Y ? 0x80 : 0));
			}
			else
			{
				return 0;
			}
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			return 0; // what gets returned?
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			return 0; // what gets returned?
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		void Write_Memory_8(uint32_t addr, uint8_t value)
		{
			if (addr == 0x8000)
			{
				if (value == 0x55)
				{
					ADC_Ready_X = true;
				}
				else
				{
					ADC_Ready_X = false;
				}
			}
			else if (addr == 0x8100)
			{
				if (value == 0xAA)
				{
					ADC_Ready_Y = true;
				}
				else
				{
					ADC_Ready_Y = false;
				}
			}
		}

		uint8_t Mapper_EEPROM_Read()
		{
			uint32_t cur_read_bit = 0;
			uint32_t cur_read_byte = 0;

			uint8_t ret = 0;

			if (Current_State == 0)
			{
				if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				return (uint8_t)(Ready_Flag ? 1 : 0);
			}
			else if (Current_State != 3)
			{
				// what to return in write state?
				return 0;
			}
			else
			{
				// what are the first 4 bits returned?
				if (Bit_Read < 4)
				{
					Bit_Read++;

					return 0;
				}
				else
				{
					cur_read_bit = Bit_Read - 4;
					cur_read_byte = cur_read_bit >> 3;

					cur_read_bit &= 7;

					cur_read_byte = 7 - cur_read_byte;
					cur_read_bit = 7 - cur_read_bit;

					ret = Cart_RAM[(Access_Address << 3) + cur_read_byte];

					ret >>= cur_read_bit;

					ret &= 1;

					Bit_Read++;

					if (Bit_Read == 68)
					{
						Bit_Read = 0;
						Current_State = 0;
						Ready_Flag = true;
					}

					return ret;
				}
			}
		}

		void Mapper_EEPROM_Write(uint8_t value)
		{
			uint32_t cur_write_bit = 0;
			uint32_t cur_write_byte = 0;

			if (Current_State == 0)
			{
				if (Core_Cycle_Count[0] >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				if (Ready_Flag)
				{
					Access_Address = 0;

					if (Bit_Offset == 0)
					{
						Next_State = value & 1;
						Bit_Offset = 1;
					}
					else
					{
						Next_State <<= 1;
						Next_State |= (value & 1);

						Bit_Offset = 0;

						if (Next_State != 0)
						{
							Current_State = 4 + Next_State;
						}
						else
						{
							Current_State = 0;
						}
					}
				}
			}
			else if (Current_State == 2)
			{
				if (Bit_Read < 64)
				{
					cur_write_bit = Bit_Read;
					cur_write_byte = cur_write_bit >> 3;

					cur_write_bit &= 7;

					cur_write_byte = 7 - cur_write_byte;
					cur_write_bit = 7 - cur_write_bit;

					Cart_RAM[(Access_Address << 3) + cur_write_byte] &= (uint8_t)(~(1 << cur_write_bit));

					Cart_RAM[(Access_Address << 3) + cur_write_byte] |= (uint8_t)((value & 1) << cur_write_bit);
				}

				Bit_Read++;

				if (Bit_Read == 65)
				{
					if ((value & 1) == 0)
					{
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750 + (uint64_t)EEPROM_Offset;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750 + (uint64_t)EEPROM_Offset;
					}

					Ready_Flag = false;
				}
			}
			else if (Current_State == 3)
			{
				// Nothing occurs in read state?
			}
			else if (Current_State == 6)
			{
				// Get Address
				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 6)
					{
						Access_Address |= (uint32_t)(value & 1);
						Access_Address <<= 1;

						Bit_Offset++;

						if (Bit_Offset == 6)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3F;

							// now write the data to the EEPROM
							Bit_Offset = 0;
							Current_State = 2;
						}
					}
				}
				else
				{
					if (Bit_Offset < 14)
					{
						Access_Address |= (uint32_t)(value & 1);
						Access_Address <<= 1;

						Bit_Offset++;

						if (Bit_Offset == 14)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3FF;

							// now write the data to the EEPROM
							Bit_Offset = 0;
							Current_State = 2;
						}
					}
				}
			}
			else if (Current_State == 7)
			{
				// Get Address for reading and wait for zero bit

				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 7)
					{
						if (Bit_Offset < 6)
						{
							Access_Address |= (uint32_t)(value & 1);
							Access_Address <<= 1;
						}

						Bit_Offset++;

						if (Bit_Offset == 7)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3F;

							if ((value & 1) == 0)
							{
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
							else
							{
								// error? seems to ignore this bit even though GBA tek says it should be zero
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
						}
					}
				}
				else
				{
					if (Bit_Offset < 15)
					{
						if (Bit_Offset < 14)
						{
							Access_Address |= (uint32_t)(value & 1);
							Access_Address <<= 1;
						}

						Bit_Offset++;

						if (Bit_Offset == 15)
						{
							Access_Address >>= 1;
							Access_Address &= 0x3FF;

							if ((value & 1) == 0)
							{
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
							else
							{
								// error? seems to ignore this bit even though GBA tek says it should be zero
								Bit_Offset = 0;

								// now read the data out from the EEPROM
								Current_State = 3;
							}
						}
					}
				}
			}
		}
	};

	# pragma endregion

}

#endif