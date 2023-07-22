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

		public bool Swapped_Out;
		public bool Erase_Command;
		public bool Erase_4k;

		public override void Reset()
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

		public override byte ReadMemory8(uint addr)
		{
			//Console.WriteLine("Read: " + Chip_Mode + " " + (addr & 0xFFFF) + " " + Core.CycleCount);

			Update_State();
			
			if (Swapped_Out)
			{
				if ((addr & 0xFFFF) > 1)
				{
					return Core.cart_RAM[(addr & 0xFFFF) + Bank_State];
				}
				else if ((addr & 0xFFFF) == 1)
				{
					if (Core.cart_RAM.Length == 0x10000)
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
					if (Core.cart_RAM.Length == 0x10000)
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
				return Core.cart_RAM[(addr & 0xFFFF) + Bank_State];
			}	
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
			//Console.WriteLine("Write: " + Chip_Mode + " " + (addr & 0xFFFF) + " " + value + " " + Core.CycleCount);
			
			if (Chip_Mode == 3)
			{
				Core.cart_RAM[(addr & 0xFFFF) + Bank_State] = value;

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
							Next_Ready_Cycle = Core.CycleCount + 4 * (ulong)Core.cart_RAM.Length;

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
						Next_Ready_Cycle = Core.CycleCount + 4 * 0x1000;

						Erase_4k = true;

						Erase_4k_Addr = (int)(addr & 0xF000);
						
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
				}
				else
				{
					for (int i = 0; i < Core.cart_RAM.Length; i++)
					{
						Core.cart_RAM[i] = 0xFF;
					}
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

			ser.Sync(nameof(Chip_Mode), ref Chip_Mode);
			ser.Sync(nameof(Bank_State), ref Bank_State);
			ser.Sync(nameof(Next_Mode), ref Next_Mode);
			ser.Sync(nameof(Erase_4k_Addr), ref Erase_4k_Addr);

			ser.Sync(nameof(Swapped_Out), ref Swapped_Out);
			ser.Sync(nameof(Erase_Command), ref Erase_Command);
			ser.Sync(nameof(Erase_4k), ref Erase_4k);
		}
	}
}
