using BizHawk.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	// Cart with ordinary SRAM and Z axis Gyro
	public class MapperSRAMGyro : MapperBase
	{
		// general and solar sensor variables
		public bool Ready_Flag;
		public bool Chip_Select;
		public bool Gyro_Clock;
		public bool Gyro_Reset;
		public bool Gyro_Bit;
		public bool Rumble_Bit;

		public byte Port_State;
		public byte Port_Dir;
		public byte Ports_RW;

		public byte Current_C4, Current_C5, Current_C6, Current_C7, Current_C8, Current_C9;

		public ushort Gyro_Clock_Count;

		public override void Reset()
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

			Core.ROM[0xC4] = Current_C4;
			Core.ROM[0xC5] = Current_C5;
			Core.ROM[0xC6] = Current_C6;
			Core.ROM[0xC7] = Current_C7;
			Core.ROM[0xC8] = Current_C8;
			Core.ROM[0xC9] = Current_C9;
		}

		public override byte ReadMemory8(uint addr)
		{
			return Core.cart_RAM[addr & 0x7FFF];
		}

		public override ushort ReadMemory16(uint addr)
		{
			// 8 bit bus only
			ushort ret = Core.cart_RAM[addr & 0x7FFF];
			ret = (ushort)(ret | (ret << 8));
			return ret;
		}

		public override uint ReadMemory32(uint addr)
		{
			// 8 bit bus only
			uint ret = Core.cart_RAM[addr & 0x7FFF];
			ret = (uint)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		public override byte PeekMemory(uint addr)
		{
			return ReadMemory8(addr);
		}

		public override void WriteMemory8(uint addr, byte value)
		{
			Core.cart_RAM[addr & 0x7FFF] = value;
		}

		public override void WriteMemory16(uint addr, ushort value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)(value & 0xFF);
			}
			else
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 8) & 0xFF);
			}			
		}

		public override void WriteMemory32(uint addr, uint value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)(value & 0xFF);
			}
			else if ((addr & 3) == 1)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 8) & 0xFF);
			}
			else if ((addr & 3) == 2)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 16) & 0xFF);
			}
			else
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 24) & 0xFF);
			}
		}

		public override void PokeMemory(uint addr, byte value)
		{
			WriteMemory8(addr, value);
		}

		public override void WriteROM8(uint addr, byte value)
		{
			/*
			if ((addr & 1)== 0)
			{
				if (Chip_Select) { Console.WriteLine("addr: " + (addr & 0xF) + " " + value + " Port: " + Port_Dir); }
			}
			*/

			bool change_CS = false;
			byte read_value_gyro = 0;
			byte read_value_rumble = 0;

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
							//Console.WriteLine("activating Gyro");
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
										Gyro_Bit = ((Core.Acc_X_state >> (12 - (Gyro_Clock_Count - 4))) & 1) == 1;
									}									

									Gyro_Clock = false;
								}

								//Console.WriteLine("Res: " + Gyro_Reset + " Clk: " + Gyro_Clock + " Cnt: " + Gyro_Clock_Count + " bit: " + Gyro_Bit + " acc: " + Core.Acc_X_state);
							}
						}

						if ((Port_Dir & 1) == 0)
						{
							read_value_gyro |= (byte)(!Gyro_Reset ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_gyro |= (byte)(Gyro_Clock ? 2 : 0);
						}
						if ((Port_Dir & 4) == 0)
						{
							read_value_gyro |= (byte)(Gyro_Bit ? 4 : 0);
						}

						Port_State = read_value_gyro;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
						}

						//Console.WriteLine(Current_C4);
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
						}

						if ((Port_Dir & 8) == 0)
						{
							read_value_rumble |= (byte)(Rumble_Bit ? 8 : 0);
						}

						Port_State = read_value_rumble;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
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
							read_value_gyro |= (byte)(!Gyro_Reset ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_gyro |= (byte)(Gyro_Clock ? 2 : 0);
						}
						if ((Port_Dir & 4) == 0)
						{
							read_value_gyro |= (byte)(Gyro_Bit ? 4 : 0);
						}

						Port_State = read_value_gyro;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
						}
					}
					else
					{
						Chip_Select = true;

						if ((Port_Dir & 8) == 0)
						{
							read_value_rumble |= (byte)(Rumble_Bit ? 8 : 0);
						}

						Port_State = read_value_rumble;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
						}
					}
				}
			}
			else if (addr == 0x080000C6)
			{
				Port_Dir = (byte)(value & 0xF);

				if (Ports_RW == 1)
				{
					Core.ROM[0xC6] = Port_Dir;

					Port_State &= (byte)((~Port_Dir) & 0xF);

					Core.ROM[0xC4] = Port_State;
				}
			}
			else if (addr == 0x080000C8)
			{
				Ports_RW = (byte)(value & 1);

				if ((value & 1) == 1)
				{
					Core.ROM[0xC4] = Port_State;
					Core.ROM[0xC5] = 0;
					Core.ROM[0xC6] = Port_Dir;
					Core.ROM[0xC7] = 0;
					Core.ROM[0xC8] = Ports_RW;
					Core.ROM[0xC9] = 0;
				}
				else
				{
					Core.ROM[0xC4] = ROM_C4;
					Core.ROM[0xC5] = ROM_C5;
					Core.ROM[0xC6] = ROM_C6;
					Core.ROM[0xC7] = ROM_C7;
					Core.ROM[0xC8] = ROM_C8;
					Core.ROM[0xC9] = ROM_C9;
				}
			}

			Current_C4 = Core.ROM[0xC4];
			Current_C5 = Core.ROM[0xC5];
			Current_C6 = Core.ROM[0xC6];
			Current_C7 = Core.ROM[0xC7];
			Current_C8 = Core.ROM[0xC8];
			Current_C9 = Core.ROM[0xC9];
		}

		public override void WriteROM16(uint addr, ushort value)
		{
			WriteROM8(addr, (byte)value);
			WriteROM8((addr + 1), (byte)(value >> 8));
		}

		public override void WriteROM32(uint addr, uint value)
		{
			WriteROM8(addr, (byte)value);
			WriteROM8((addr + 1), (byte)(value >> 8));
			WriteROM8((addr + 2), (byte)(value >> 16));
			WriteROM8((addr + 3), (byte)(value >> 24));
		}

		public override void SyncState(Serializer ser)
		{
			ser.Sync(nameof(Ready_Flag), ref Ready_Flag);
			ser.Sync(nameof(Chip_Select), ref Chip_Select);
			ser.Sync(nameof(Gyro_Clock), ref Gyro_Clock);
			ser.Sync(nameof(Gyro_Reset), ref Gyro_Reset);
			ser.Sync(nameof(Gyro_Bit), ref Gyro_Bit);
			ser.Sync(nameof(Rumble_Bit), ref Rumble_Bit);

			ser.Sync(nameof(Port_State), ref Port_State);
			ser.Sync(nameof(Port_Dir), ref Port_Dir);
			ser.Sync(nameof(Ports_RW), ref Ports_RW);

			ser.Sync(nameof(Current_C4), ref Current_C4);
			ser.Sync(nameof(Current_C5), ref Current_C5);
			ser.Sync(nameof(Current_C6), ref Current_C6);
			ser.Sync(nameof(Current_C7), ref Current_C7);
			ser.Sync(nameof(Current_C8), ref Current_C8);
			ser.Sync(nameof(Current_C9), ref Current_C9);

			ser.Sync(nameof(Gyro_Clock_Count), ref Gyro_Clock_Count);

			Core.ROM[0xC4] = Current_C4;
			Core.ROM[0xC5] = Current_C5;
			Core.ROM[0xC6] = Current_C6;
			Core.ROM[0xC7] = Current_C7;
			Core.ROM[0xC8] = Current_C8;
			Core.ROM[0xC9] = Current_C9;

			ser.Sync(nameof(Reg_Year), ref Reg_Year);
			ser.Sync(nameof(Reg_Month), ref Reg_Month);
			ser.Sync(nameof(Reg_Week), ref Reg_Week);
			ser.Sync(nameof(Reg_Day), ref Reg_Day);
			ser.Sync(nameof(Reg_Hour), ref Reg_Hour);
			ser.Sync(nameof(Reg_Minute), ref Reg_Minute);
			ser.Sync(nameof(Reg_Second), ref Reg_Second);

			ser.Sync(nameof(Reg_Ctrl), ref Reg_Ctrl);
		}
	}
}
