#ifndef MAPPERS_H
#define MAPPERS_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace std;

namespace GBAHawk
{
	class Mappers
	{
	public:
	#pragma region mapper base

		// not stated
		uint8_t ROM_C4, ROM_C6, ROM_C8;
		
		bool Ready_Flag;
		bool ADC_Ready_X, ADC_Ready_Y;
		bool Swapped_Out;
		bool Erase_Command;
		bool Erase_4k;

		uint8_t Port_State;
		uint8_t Port_Dir;
		uint8_t Ports_RW;

		uint8_t Current_C4, Current_C6, Current_C8;

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
		
		uint64_t Next_Ready_Cycle;

		uint64_t* Core_Cycle_Count = nullptr;

		uint16_t* Core_Acc_X = nullptr;

		uint16_t* Core_Acc_Y = nullptr;

		uint8_t* Core_Solar = nullptr;

		uint8_t* Cart_RAM = nullptr;

		uint8_t* Core_ROM = nullptr;
		
		Mappers()
		{
			Reset();
		}

		virtual uint8_t Read_Memory_8(uint32_t addr)
		{
			return 0;
		}

		virtual uint16_t Read_Memory_16(uint32_t addr)
		{
			return 0;
		}

		virtual uint32_t Read_Memory_32(uint32_t addr)
		{
			return 0;
		}

		virtual uint8_t Peek_Memory(uint32_t addr)
		{
			return 0;
		}

		virtual void Write_Memory_8(uint32_t addr, uint8_t value)
		{
		}

		virtual void Write_Memory_16(uint32_t addr, uint16_t value)
		{
		}

		virtual void Write_Memory_32(uint32_t addr, uint32_t value)
		{
		}

		virtual void Write_ROM_8(uint32_t addr, uint8_t value)
		{
		}

		virtual void Write_ROM_16(uint32_t addr, uint16_t value)
		{
		}

		virtual void Write_ROM_32(uint32_t addr, uint32_t value)
		{
		}

		virtual void Poke_Memory(uint32_t addr, uint8_t value)
		{
		}

		virtual void Dispose()
		{
		}

		virtual void Reset()
		{
		}

		virtual uint8_t Mapper_EEPROM_Read()
		{
			return 0xFF;
		}

		virtual void Mapper_EEPROM_Write(uint8_t value)
		{

		}

		virtual void Update_State()
		{
		}

	#pragma endregion

	#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Ready_Flag, saver);
			saver = bool_saver(ADC_Ready_X, saver);
			saver = bool_saver(ADC_Ready_Y, saver);
			saver = bool_saver(Swapped_Out, saver);
			saver = bool_saver(Erase_Command, saver);
			saver = bool_saver(Erase_4k, saver);

			saver = byte_saver(Port_State, saver);
			saver = byte_saver(Port_Dir, saver);
			saver = byte_saver(Ports_RW, saver);

			saver = byte_saver(Current_C4, saver);
			saver = byte_saver(Current_C6, saver);
			saver = byte_saver(Current_C8, saver);

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

			saver = long_saver(Next_Ready_Cycle, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Ready_Flag, loader);
			loader = bool_loader(&ADC_Ready_X, loader);
			loader = bool_loader(&ADC_Ready_Y, loader);
			loader = bool_loader(&Swapped_Out, loader);
			loader = bool_loader(&Erase_Command, loader);
			loader = bool_loader(&Erase_4k, loader);

			loader = byte_loader(&Port_State, loader);
			loader = byte_loader(&Port_Dir, loader);
			loader = byte_loader(&Ports_RW, loader);

			loader = byte_loader(&Current_C4, loader);
			loader = byte_loader(&Current_C6, loader);
			loader = byte_loader(&Current_C8, loader);

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

			loader = long_loader(&Next_Ready_Cycle, loader);

			Core_ROM[0xC4] = Current_C4;
			Core_ROM[0xC6] = Current_C6;
			Core_ROM[0xC8] = Current_C8;

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
			to_load[0] = *to_load == 1; loader++;

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
			to_load[0] = *loader; loader++; to_load[0] |= (*loader << 8); loader++;
			to_load[0] |= (*loader << 16); loader++; to_load[0] |= (*loader << 24); loader++;

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

	#pragma endregion

	};

	#pragma region Default

	class Mapper_Default : public Mappers
	{
	public:

		void Reset()
		{
			// nothing to initialize
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

		uint8_t PeekMemory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}
	};

	#pragma endregion

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

			Next_Ready_Cycle = 0;
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

		uint8_t PeekMemory(uint32_t addr)
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
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
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
			// set up initial variables
			Ready_Flag = true;

			Port_State = 0;
			Port_Dir = 0;
			Ports_RW = 0;

			Current_C4 = ROM_C4;
			Current_C6 = ROM_C6;
			Current_C8 = ROM_C8;

			Core_ROM[0xC4] = Current_C4;
			Core_ROM[0xC6] = Current_C6;
			Core_ROM[0xC8] = Current_C8;

			Bit_Offset = Bit_Read = 0;

			Access_Address = 0;

			Current_State = 0;

			Next_State = 0;

			Next_Ready_Cycle = 0;
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

		uint8_t PeekMemory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		void Write_ROM_8(uint32_t addr, uint8_t value)
		{
			if ((addr & 1) == 0) { Write_ROM_16(addr, (uint16_t)value); }
		}

		void Write_ROM_16(uint32_t addr, uint16_t value)
		{
			if (addr == 0x080000C4)
			{

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
					Core_ROM[0xC6] = Port_Dir;
					Core_ROM[0xC8] = Ports_RW;
				}
				else
				{
					Core_ROM[0xC4] = ROM_C4;
					Core_ROM[0xC6] = ROM_C6;
					Core_ROM[0xC8] = ROM_C8;
				}
			}

			Current_C4 = Core_ROM[0xC4];
			Current_C6 = Core_ROM[0xC6];
			Current_C8 = Core_ROM[0xC8];
		}

		void Write_ROM_32(uint32_t addr, uint32_t value)
		{
			Write_ROM_16(addr, (uint16_t)value);
			Write_ROM_16((addr + 2), (uint16_t)(value >> 8));
		}

		uint8_t Mapper_EEPROM_Read()
		{
			uint32_t cur_read_bit = 0;
			uint32_t cur_read_byte = 0;

			uint8_t ret = 0;

			if (Current_State == 0)
			{
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
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
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

			Next_Ready_Cycle = 0;

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

		uint8_t PeekMemory(uint32_t addr)
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
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
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

	#pragma region FLASH

	class Mapper_FLASH : public Mappers
	{
	public:

		void Reset()
		{
			Bank_State = 0;
			Chip_Mode = 0;
			Next_Mode = 0;
			Erase_4k_Addr = 0;

			Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;

			Swapped_Out = false;
			Erase_Command = false;
			Erase_4k = false;
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			Update_State();

			if (Swapped_Out)
			{
				if ((addr & 0xFFFF) > 1)
				{
					return Cart_RAM[(addr & 0xFFFF) + Bank_State];
				}
				else if ((addr & 0xFFFF) == 1)
				{
					if ((Size_Mask + 1) == 0x10000)
					{
						return 0x1B;
					}
					else
					{
						return 0x13;
					}
				}
				else
				{
					if ((Size_Mask + 1) == 0x10000)
					{
						return 0x32;
					}
					else
					{
						return 0x62;
					}
				}
			}
			else
			{
				return Cart_RAM[(addr & 0xFFFF) + Bank_State];
			}
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
			if (Chip_Mode == 3)
			{
				Cart_RAM[(addr & 0xFFFF) + Bank_State] = value;

				// instant writes good enough?
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
							Next_Ready_Cycle = Core_Cycle_Count[0] + 4 * (uint64_t)(Size_Mask + 1);
							//Next_Ready_Cycle = 0;

							Erase_4k = false;

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
						Next_Ready_Cycle = Core_Cycle_Count[0] + (uint64_t)0x4000;
						//Next_Ready_Cycle = 0;

						Erase_4k = true;

						Erase_4k_Addr = (uint32_t)(addr & 0xF000);

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
					}
				}
			}
		}

		void Write_Memory_16(uint32_t addr, uint16_t value)
		{
			Write_Memory_8((addr & 0xFFFE), (uint8_t)value);
		}

		void Write_Memory_32(uint32_t addr, uint32_t value)
		{
			Write_Memory_8((addr & 0xFFFC), (uint8_t)value);
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
				}
				else
				{
					for (uint32_t i = 0; i < (Size_Mask + 1); i++)
					{
						Cart_RAM[i] = 0xFF;
					}
				}

				Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;
			}
		}
	};
	#pragma endregion

}

#endif
