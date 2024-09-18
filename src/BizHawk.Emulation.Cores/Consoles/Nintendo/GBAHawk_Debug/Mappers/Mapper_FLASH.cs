using System;

using BizHawk.Common;
using Newtonsoft.Json.Linq;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	// Cart with FLASH RAM
	// Only emulates non-atmel FLASH type
	// how long do things take?

	public class MapperFLASH : MapperBase
	{
		public ulong Next_Ready_Cycle;

		// 0 - Ready
		// 1 - First comand write dne
		// 2 - Second command write done
		// 3 - Write a byte
		// 4 - Change Bank
		public int Chip_Mode;
		public int Bank_State;
		public int Next_Mode;
		public int Erase_4k_Addr;
		public int Access_Address;

		public byte Write_Value;

		public bool Swapped_Out;
		public bool Erase_Command;
		public bool Erase_4k;
		public bool Erase_All;
		public bool Force_Bit_6;

		public override void Reset()
		{
			Bank_State = 0;
			Chip_Mode = 0;
			Next_Mode = 0;
			Erase_4k_Addr = 0;
			Access_Address = 0;

			Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;

			Write_Value = 0;

			Swapped_Out = false;
			Erase_Command = false;
			Erase_4k = false;
			Erase_All = false;
			Force_Bit_6 = false;
		}

		public override byte ReadMemory8(uint addr)
		{
			//Console.WriteLine("Read: " + Chip_Mode + " " + (addr & 0xFFFF) + " " + Core.CycleCount);

			Update_State();

			byte ret_value = 0;

			if (Swapped_Out)
			{
				if ((addr & 0xFFFF) > 1)
				{
					ret_value = Core.cart_RAM[(addr & 0xFFFF) + Bank_State];
				}
				else if ((addr & 0xFFFF) == 1)
				{
					if (Core.cart_RAM.Length == 0x10000)
					{
						ret_value = 0x1B;
					}
					else
					{
						ret_value = 0x13;
					}
				}
				else
				{
					if (Core.cart_RAM.Length == 0x10000)
					{
						ret_value = 0x32;
					}
					else
					{
						ret_value = 0x62;
					}
				}
			}
			else
			{
				ret_value = Core.cart_RAM[(addr & 0xFFFF) + Bank_State];
			}
			
			if (Next_Ready_Cycle != 0xFFFFFFFFFFFFFFFF)
			{
				if (Core.cart_RAM.Length == 0x10000)
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
					// for the larger chips, it seems a status register is activated on wirtes, which
					// returns 0 until done (for our purposes since operations always succeed.)

					ret_value = 0;
				}

				//Console.WriteLine("not ready " + Force_Bit_6 + " " + ret_value);
			}

			return ret_value;
		}

		public override ushort ReadMemory16(uint addr)
		{
			// 8 bit bus only
			ushort ret = ReadMemory8(addr & 0xFFFE);

			ret = (ushort)(ret | (ret << 8));
			return ret;
		}

		public override uint ReadMemory32(uint addr)
		{
			// 8 bit bus only
			uint ret = ReadMemory8(addr & 0xFFFC);

			ret = (uint)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		public override void WriteMemory8(uint addr, byte value)
		{
			//Console.WriteLine("Write: " + Chip_Mode + " " + (addr & 0xFFFF) + " " + value + " bank " + Bank_State + " " + Core.CycleCount);

			Update_State();

			if (Chip_Mode == 3)
			{
				Console.WriteLine("write byte");

				if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
				{
					Access_Address = (int)((addr & 0xFFFF) + Bank_State);
					Write_Value = value;

					Next_Ready_Cycle = Core.CycleCount + 325 + (ulong)Core._syncSettings.FlashWriteOffset;
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
							Console.WriteLine("Erase All");

							if (Next_Ready_Cycle == 0xFFFFFFFFFFFFFFFF)
							{
								Next_Ready_Cycle = Core.CycleCount + 3 * 430000 + (ulong)Core._syncSettings.FlashChipEraseOffset;

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
						if (Core.cart_RAM.Length == 0x20000)
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
							Console.WriteLine("Erase 4k");

							Next_Ready_Cycle = Core.CycleCount + 460000 + (ulong)Core._syncSettings.FlashSectorEraseOffset;

							Erase_4k = true;

							Erase_4k_Addr = (int)(addr & 0xF000);

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

		public override void WriteMemory16(uint addr, ushort value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				WriteMemory8((addr & 0xFFFF), (byte)value);
			}
			else
			{
				WriteMemory8((addr & 0xFFFF), (byte)((value >> 8) & 0xFF));
			}
		}

		public override void WriteMemory32(uint addr, uint value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				WriteMemory8((addr & 0xFFFF), (byte)value);
			}
			else if ((addr & 3) == 1)
			{
				WriteMemory8((addr & 0xFFFF), (byte)((value >> 8) & 0xFF));
			}
			else if ((addr & 3) == 2)
			{
				WriteMemory8((addr & 0xFFFF), (byte)((value >> 16) & 0xFF));
			}
			else
			{
				WriteMemory8((addr & 0xFFFF), (byte)((value >> 24) & 0xFF));
			}		
		}

		public void Update_State()
		{
			if (Core.CycleCount >= Next_Ready_Cycle)
			{
				if (Erase_4k)
				{
					for (int i = 0; i < 0x1000; i++)
					{
						Core.cart_RAM[i + Erase_4k_Addr + Bank_State] = 0xFF;
					}

					Erase_4k = false;
				}
				else if (Erase_All)
				{
					for (int i = 0; i < Core.cart_RAM.Length; i++)
					{
						Core.cart_RAM[i] = 0xFF;
					}

					Erase_All = false;
				}
				else
				{
					Core.cart_RAM[Access_Address] = Write_Value;
				}

				Next_Ready_Cycle = 0xFFFFFFFFFFFFFFFF;
			}
		}

		public override byte PeekMemory(uint addr)
		{
			return Core.cart_RAM[(addr & 0xFFFF) + Bank_State];
		}

		public override void PokeMemory(uint addr, byte value)
		{
			Core.cart_RAM[(addr & 0xFFFF) + Bank_State] = value;
		}

		public override void SyncState(Serializer ser)
		{
			ser.Sync(nameof(Next_Ready_Cycle), ref Next_Ready_Cycle);

			ser.Sync(nameof(Write_Value), ref Write_Value);

			ser.Sync(nameof(Chip_Mode), ref Chip_Mode);
			ser.Sync(nameof(Bank_State), ref Bank_State);
			ser.Sync(nameof(Next_Mode), ref Next_Mode);
			ser.Sync(nameof(Erase_4k_Addr), ref Erase_4k_Addr);
			ser.Sync(nameof(Access_Address), ref Access_Address);

			ser.Sync(nameof(Swapped_Out), ref Swapped_Out);
			ser.Sync(nameof(Erase_Command), ref Erase_Command);
			ser.Sync(nameof(Erase_4k), ref Erase_4k);
			ser.Sync(nameof(Erase_All), ref Erase_All);
			ser.Sync(nameof(Force_Bit_6), ref Force_Bit_6);
		}
	}
}
