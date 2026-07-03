#ifndef SRAM_MAPPERS_H
#define SRAM_MAPPERS_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace GBAHawk
{

	#pragma region SRAM

	class Mapper_SRAM : public Mappers
	{
	public:

		void Reset()
		{
			// nothing to initialize
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			return Cart_RAM[addr & 0x7FFF];
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			// 8 bit bus only
			uint16_t ret = Cart_RAM[addr & 0x7FFF];
			ret = (uint16_t)(ret | (ret << 8));
			return ret;
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			// 8 bit bus only
			uint32_t ret = Cart_RAM[addr & 0x7FFF];
			ret = (uint32_t)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		void Write_Memory_8(uint32_t addr, uint8_t value)
		{
			Cart_RAM[addr & 0x7FFF] = value;
		}

		void Write_Memory_16(uint32_t addr, uint16_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)(value & 0xFF);
			}
			else
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 8) & 0xFF);
			}
		}

		void Write_Memory_32(uint32_t addr, uint32_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)(value & 0xFF);
			}
			else if ((addr & 3) == 1)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 8) & 0xFF);
			}
			else if ((addr & 3) == 2)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 16) & 0xFF);
			}
			else
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 24) & 0xFF);
			}
		}

	};
	#pragma endregion

	#pragma region SRAM Gyro

	class Mapper_SRAM_Gyro : public Mappers
	{
	public:

		void Reset()
		{
			// set up initial variables for Gyro
			Ready_Flag = true;
			Chip_Select = false;
			Gyro_Clock = false;
			Gyro_Bit = false;
			Rumble_Bit = false;

			Port_State = 0;
			Port_Dir = 0;
			Ports_RW = 0;

			Current_C4 = ROM_C4;
			Current_C5 = ROM_C5;
			Current_C6 = ROM_C6;
			Current_C7 = ROM_C7;
			Current_C8 = ROM_C8;
			Current_C9 = ROM_C9;

			Gyro_Clock_Count = 0;

			Core_ROM[0xC4] = Current_C4;
			Core_ROM[0xC5] = Current_C5;
			Core_ROM[0xC6] = Current_C6;
			Core_ROM[0xC7] = Current_C7;
			Core_ROM[0xC8] = Current_C8;
			Core_ROM[0xC9] = Current_C9;
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			return Cart_RAM[addr & 0x7FFF];
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			// 8 bit bus only
			uint16_t ret = Cart_RAM[addr & 0x7FFF];
			ret = (uint16_t)(ret | (ret << 8));
			return ret;
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			// 8 bit bus only
			uint32_t ret = Cart_RAM[addr & 0x7FFF];
			ret = (uint32_t)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		uint8_t Peek_Memory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		void Write_Memory_8(uint32_t addr, uint8_t value)
		{
			Cart_RAM[addr & 0x7FFF] = value;
		}

		void Write_Memory_16(uint32_t addr, uint16_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)(value & 0xFF);
			}
			else
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 8) & 0xFF);
			}
		}

		void Write_Memory_32(uint32_t addr, uint32_t value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)(value & 0xFF);
			}
			else if ((addr & 3) == 1)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 8) & 0xFF);
			}
			else if ((addr & 3) == 2)
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 16) & 0xFF);
			}
			else
			{
				Cart_RAM[addr & 0x7FFF] = (uint8_t)((value >> 24) & 0xFF);
			}
		}


		void Write_ROM_8(uint32_t addr, uint8_t value)
		{
			bool change_CS = false;
			uint8_t read_value_gyro = 0;
			uint8_t read_value_rumble = 0;

			if (addr == 0x080000C4)
			{
				// if changing chip select, ignore other writes
				// not sure if correct or what happes?
				if ((Port_Dir & 4) == 4)
				{
					if ((value & 4) == 0)
					{
						if (Chip_Select)
						{
							change_CS = true;
						}
					}
					if ((value & 4) == 4)
					{
						if (!Chip_Select)
						{
							change_CS = true;
						}
					}
				}

				if (!change_CS)
				{
					if (!Chip_Select)
					{
						// Rate Gyro			
						if ((Port_Dir & 1) == 1)
						{
							if ((value & 1) == 1)
							{
								Gyro_Reset = true;
								Gyro_Clock = false;
								Gyro_Clock_Count = 0;
								Gyro_Bit = false;
							}
							else
							{
								Gyro_Reset = false;
							}
						}

						if ((Port_Dir & 2) == 2)
						{
							if ((value & 2) == 2)
							{
								if (!Gyro_Reset)
								{
									Gyro_Clock = true;
								}
							}
							else
							{
								if (!Gyro_Reset)
								{
									if (Gyro_Clock)
									{
										Gyro_Clock_Count += 1;
										Gyro_Clock_Count &= 0xFFF;
									}

									if ((Gyro_Clock_Count < 4) || (Gyro_Clock_Count > 16))
									{
										Gyro_Bit = false;
									}
									else
									{
										Gyro_Bit = ((Core_Acc_X[0] >> (12 - (Gyro_Clock_Count - 4))) & 1) == 1;
									}

									Gyro_Clock = false;
								}
							}
						}

						if ((Port_Dir & 1) == 0)
						{
							read_value_gyro |= (uint8_t)(!Gyro_Reset ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_gyro |= (uint8_t)(Gyro_Clock ? 2 : 0);
						}
						if ((Port_Dir & 4) == 0)
						{
							read_value_gyro |= (uint8_t)(Gyro_Bit ? 4 : 0);
						}

						Port_State = read_value_gyro;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
					else
					{
						// rumble
						if ((Port_Dir & 8) == 8)
						{
							if ((value & 8) == 8)
							{
								Rumble_Bit = true;
							}
							else
							{
								Rumble_Bit = false;
							}

							RumbleCallback(Rumble_Bit);
						}

						if ((Port_Dir & 8) == 0)
						{
							read_value_rumble |= (uint8_t)(Rumble_Bit ? 8 : 0);
						}

						Port_State = read_value_rumble;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
				}
				else
				{
					// enable Gyro
					if ((value & 4) == 0)
					{
						Chip_Select = false;

						if ((Port_Dir & 1) == 0)
						{
							read_value_gyro |= (uint8_t)(!Gyro_Reset ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_gyro |= (uint8_t)(Gyro_Clock ? 2 : 0);
						}
						if ((Port_Dir & 4) == 0)
						{
							read_value_gyro |= (uint8_t)(Gyro_Bit ? 4 : 0);
						}

						Port_State = read_value_gyro;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
					else
					{
						Chip_Select = true;

						if ((Port_Dir & 8) == 0)
						{
							read_value_rumble |= (uint8_t)(Rumble_Bit ? 8 : 0);
						}

						Port_State = read_value_rumble;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
				}
			}
			else if (addr == 0x080000C6)
			{
				Port_Dir = (uint8_t)(value & 0xF);

				if (Ports_RW == 1)
				{
					Core_ROM[0xC6] = Port_Dir;

					Port_State &= (uint8_t)((~Port_Dir) & 0xF);

					Core_ROM[0xC4] = Port_State;
				}
			}
			else if (addr == 0x080000C8)
			{
				Ports_RW = (uint8_t)(value & 1);

				if ((value & 1) == 1)
				{
					Core_ROM[0xC4] = Port_State;
					Core_ROM[0xC5] = 0;
					Core_ROM[0xC6] = Port_Dir;
					Core_ROM[0xC7] = 0;
					Core_ROM[0xC8] = Ports_RW;
					Core_ROM[0xC9] = 0;
				}
				else
				{
					Core_ROM[0xC4] = ROM_C4;
					Core_ROM[0xC5] = ROM_C5;
					Core_ROM[0xC6] = ROM_C6;
					Core_ROM[0xC7] = ROM_C7;
					Core_ROM[0xC8] = ROM_C8;
					Core_ROM[0xC9] = ROM_C9;
				}
			}

			Current_C4 = Core_ROM[0xC4];
			Current_C5 = Core_ROM[0xC5];
			Current_C6 = Core_ROM[0xC6];
			Current_C7 = Core_ROM[0xC7];
			Current_C8 = Core_ROM[0xC8];
			Current_C9 = Core_ROM[0xC9];
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
	};
	#pragma endregion


}

#endif