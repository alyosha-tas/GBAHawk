using System;

using BizHawk.Common;
using Newtonsoft.Json.Linq;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	// Cart with FLASH RAM and RTC
	// Only emulates non-atmel FLASH type
	// how long do things take?

	public class MapperFLASH_RTC : MapperBase
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

		// general IO
		public bool Chip_Select;

		public byte Port_State;
		public byte Port_Dir;
		public byte Ports_RW;

		public byte Current_C4, Current_C5, Current_C6, Current_C7, Current_C8, Current_C9;

		// RTC Variables
		public bool Command_Mode;
		public bool RTC_Clock;
		public bool RTC_Read;
		public bool RTC_24_Hour;

		public byte Command_Byte;
		public byte RTC_SIO;
		public byte Command_Bit, Command_Bit_Count;
		public byte Reg_Bit, Reg_Bit_Count;
		public byte Reg_Access;

		public byte Reg_Year, Reg_Month, Reg_Day, Reg_Week, Reg_Hour, Reg_Minute, Reg_Second;

		public byte Reg_Ctrl;

		public ulong RTC_Temp_Write;

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

			Core.ROM[0xC4] = Current_C4;
			Core.ROM[0xC5] = Current_C5;
			Core.ROM[0xC6] = Current_C6;
			Core.ROM[0xC7] = Current_C7;
			Core.ROM[0xC8] = Current_C8;
			Core.ROM[0xC9] = Current_C9;

			// set up initial variables for RTC
			Command_Mode = true;
			RTC_Clock = false;
			RTC_Read = false;

			Command_Byte = 0;
			RTC_SIO = 0;
			Command_Bit = Command_Bit_Count = 0;
			Reg_Bit = Reg_Bit_Count = 0;
			Reg_Access = 0;

			RTC_Temp_Write = 0;

			if (Core.Reset_RTC)
			{
				RTC_24_Hour = false;
				Reg_Year = Reg_Week = Reg_Hour = Reg_Minute = Reg_Second = 0;
				Reg_Day = Reg_Month = 1;

				Reg_Ctrl = 0;
				Console.WriteLine("reset all rtc");
			}
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
			byte read_value_solar = 0;
			byte read_value_rtc = 0;

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
							//Console.WriteLine("activating RTC");
						}
					}
				}

				if (!change_CS)
				{
					if (Chip_Select)
					{
						// RTC
						if (Command_Mode)
						{
							if ((Port_Dir & 1) == 1)
							{
								if ((value & 1) == 0)
								{
									RTC_Clock = true;
								}
								else
								{
									// clock in next bit on falling edge
									if (RTC_Clock)
									{
										if ((Port_Dir & 2) == 2)
										{
											Command_Bit = (byte)((value & 2) >> 1);
											RTC_SIO = (byte)(value & 2);
										}
										else
										{
											Command_Bit = 0;
											RTC_SIO = 0;
										}

										Command_Byte |= (byte)(Command_Bit << Command_Bit_Count);

										Command_Bit_Count += 1;

										if (Command_Bit_Count == 8)
										{
											// change mode if valid command, otherwise start over
											if ((Command_Byte & 0xF) == 6)
											{
												Command_Mode = false;

												if ((Command_Byte & 0x80) == 0x80)
												{
													RTC_Read = true;
												}
												else
												{
													RTC_Read = false;
												}

												Reg_Access = (byte)((Command_Byte & 0x70) >> 4);

												//Console.WriteLine("Read: " + RTC_Read + " Reg_Access: " + Reg_Access + " Command: " + Command_Byte);

												if ((Command_Byte == 0x06) || (Command_Byte == 0x86))
												{
													// register reset
													Reg_Year = Reg_Week = 0;
													Reg_Hour = Reg_Minute = Reg_Second = 0;
													Reg_Day = Reg_Month = 1;

													Reg_Ctrl = 0;

													Command_Mode = true;

													Core.Clock_Update_Cycle = Core.CycleCount;
												}
												else if ((Command_Byte == 0x36) || (Command_Byte == 0xB6))
												{
													// Force IRQ
													Command_Mode = true;
												}
											}
											else
											{
												Console.WriteLine("Bad Command: " + Command_Byte);
											}

											Command_Byte = 0;
											Command_Bit_Count = 0;

											RTC_Temp_Write = 0;

											Update_Clock();
										}

										RTC_Clock = false;
									}
								}
							}
						}
						else
						{
							if ((Port_Dir & 1) == 1)
							{
								if ((value & 1) == 0)
								{
									RTC_Clock = true;
								}
								else
								{
									// clock in next bit on falling edge
									if (RTC_Clock)
									{
										switch (Reg_Access)
										{
											case 0:
												// force reset
												Reg_Year = Reg_Week = 0;
												Reg_Hour = Reg_Minute = Reg_Second = 0;
												Reg_Day = Reg_Month = 1;

												Reg_Ctrl = 0;

												Command_Mode = true;

												Core.Clock_Update_Cycle = Core.CycleCount;
												break;

											case 1:
												// purpose unknown, always 0xFF
												if ((Port_Dir & 2) == 2)
												{
													RTC_SIO = (byte)(value & 2);
												}
												else
												{
													RTC_SIO = 2;
												}

												Reg_Bit_Count += 1;

												if (Reg_Bit_Count == 8)
												{
													Reg_Bit_Count = 0;

													Command_Mode = true;
												}

												break;

											case 2:
												// date time
												if ((Port_Dir & 2) == 2)
												{
													RTC_SIO = (byte)(value & 2);

													if (!RTC_Read)
													{
														RTC_Temp_Write |= (((ulong)(value & 2) >> 1) << Reg_Bit_Count);
													}
												}
												else
												{
													if (RTC_Read)
													{
														if (Reg_Bit_Count < 8)
														{
															RTC_SIO = (byte)(((Reg_Year >> Reg_Bit_Count) << 1) & 2);
														}
														else if (Reg_Bit_Count < 16)
														{
															RTC_SIO = (byte)(((Reg_Month >> (Reg_Bit_Count - 8)) << 1) & 2);
														}
														else if (Reg_Bit_Count < 24)
														{
															RTC_SIO = (byte)(((Reg_Day >> (Reg_Bit_Count - 16)) << 1) & 2);
														}
														else if (Reg_Bit_Count < 32)
														{
															RTC_SIO = (byte)(((Reg_Week >> (Reg_Bit_Count - 24)) << 1) & 2);
														}
														else if (Reg_Bit_Count < 40)
														{
															RTC_SIO = (byte)(((Reg_Hour >> (Reg_Bit_Count - 32)) << 1) & 2);
														}
														else if (Reg_Bit_Count < 48)
														{
															RTC_SIO = (byte)(((Reg_Minute >> (Reg_Bit_Count - 40)) << 1) & 2);
														}
														else
														{
															RTC_SIO = (byte)(((Reg_Second >> (Reg_Bit_Count - 48)) << 1) & 2);
														}
													}
													else
													{
														//what is output when in write mode but port is reading?
														RTC_SIO = 2;
													}
												}

												Reg_Bit_Count += 1;

												//Console.WriteLine("new bit: " + Reg_Bit_Count);

												if (Reg_Bit_Count == 56)
												{
													Reg_Bit_Count = 0;

													Command_Mode = true;

													//Console.WriteLine("hour: " + Reg_Hour);

													if (!RTC_Read)
													{
														Reg_Year = (byte)(RTC_Temp_Write);
														Reg_Month = (byte)((RTC_Temp_Write >> 8) & 0x1F);
														Reg_Day = (byte)((RTC_Temp_Write >> 16) & 0x3F);
														Reg_Week = (byte)((RTC_Temp_Write >> 24) & 0x7);

														if (RTC_24_Hour)
														{
															Reg_Hour = (byte)((RTC_Temp_Write >> 32) & 0x3F);

															if (To_Byte(Reg_Hour) >= 12) { Reg_Hour |= 0x80; }
														}
														else
														{
															Reg_Hour = (byte)((RTC_Temp_Write >> 32) & 0x7F);
														}

														Reg_Minute = (byte)((RTC_Temp_Write >> 40) & 0x7F);
														Reg_Second = (byte)((RTC_Temp_Write >> 48) & 0x7F);

														Core.Clock_Update_Cycle = Core.CycleCount;

														/*
														Console.WriteLine("Yr: " + To_Byte(Reg_Year) + 
																			" Month: " +  To_Byte(Reg_Month) +
																			" Day: " + To_Byte(Reg_Day) +
																			" Week: " + To_Byte(Reg_Week) + 
																			" Hour: " + To_Byte(Reg_Hour) + " Hour (h): " + Reg_Hour +
																			" Minute: " + To_Byte(Reg_Minute) +
																			" Second: " + To_Byte(Reg_Second));
														*/
													}
												}
												break;

											case 3:
												// Force IRQ
												Command_Mode = true;
												Console.WriteLine("Cart IRQ Force?");
												break;

											case 4:
												// Control
												if ((Port_Dir & 2) == 2)
												{
													RTC_SIO = (byte)(value & 2);

													if (!RTC_Read)
													{
														RTC_Temp_Write |= (((ulong)(value & 2) >> 1) << Reg_Bit_Count);
													}
												}
												else
												{
													if (RTC_Read)
													{
														RTC_SIO = (byte)(((Reg_Ctrl >> Reg_Bit_Count) << 1) & 2);
													}
													else
													{
														//what is output when in write mode but port is reading?
														RTC_SIO = 2;
													}
												}

												Reg_Bit_Count += 1;

												//Console.WriteLine("new bit: " + Reg_Bit_Count);

												if (Reg_Bit_Count == 8)
												{
													Reg_Bit_Count = 0;

													Command_Mode = true;

													if (!RTC_Read)
													{
														Reg_Ctrl = (byte)RTC_Temp_Write;

														RTC_24_Hour = (Reg_Ctrl & 0x40) == 0x40;

														//Console.WriteLine("New Ctrl: " + Reg_Ctrl);
													}
												}
												break;

											case 5:
												// nothing to do, always 0xFF
												if ((Port_Dir & 2) == 2)
												{
													RTC_SIO = (byte)(value & 2);
												}
												else
												{
													RTC_SIO = 2;
												}

												Reg_Bit_Count += 1;

												if (Reg_Bit_Count == 8)
												{
													Reg_Bit_Count = 0;

													Command_Mode = true;
												}

												break;

											case 6:
												// time
												if ((Port_Dir & 2) == 2)
												{
													RTC_SIO = (byte)(value & 2);

													if (!RTC_Read)
													{
														RTC_Temp_Write |= (((ulong)(value & 2) >> 1) << Reg_Bit_Count);
													}
												}
												else
												{
													if (RTC_Read)
													{
														if (Reg_Bit_Count < 8)
														{
															RTC_SIO = (byte)(((Reg_Hour >> Reg_Bit_Count) << 1) & 2);
														}
														else if (Reg_Bit_Count < 16)
														{
															RTC_SIO = (byte)(((Reg_Minute >> (Reg_Bit_Count - 8)) << 1) & 2);
														}
														else
														{
															RTC_SIO = (byte)(((Reg_Second >> (Reg_Bit_Count - 16)) << 1) & 2);
														}
													}
													else
													{
														//what is output when in write mode but port is reading?
														RTC_SIO = 2;
													}
												}

												Reg_Bit_Count += 1;

												//Console.WriteLine("new bit: " + Reg_Bit_Count);

												if (Reg_Bit_Count == 24)
												{
													Reg_Bit_Count = 0;

													Command_Mode = true;

													if (!RTC_Read)
													{
														if (RTC_24_Hour)
														{
															Reg_Hour = (byte)((RTC_Temp_Write) & 0x3F);

															if (To_Byte(Reg_Hour) >= 12) { Reg_Hour |= 0x80; }
														}
														else
														{
															Reg_Hour = (byte)((RTC_Temp_Write) & 0x7F);
														}

														Reg_Minute = (byte)((RTC_Temp_Write >> 8) & 0x7F);
														Reg_Second = (byte)((RTC_Temp_Write >> 16) & 0x7F);

														Core.Clock_Update_Cycle = Core.CycleCount;

														/*
														Console.WriteLine(" Hour: " + To_Byte(Reg_Hour) +
																			" Minute: " + To_Byte(Reg_Minute) +
																			" Second: " + To_Byte(Reg_Second));
														*/
													}
												}

												break;

											case 7:
												// nothing to do, always 0xFF
												if ((Port_Dir & 2) == 2)
												{
													RTC_SIO = (byte)(value & 2);
												}
												else
												{
													RTC_SIO = 2;
												}

												Reg_Bit_Count += 1;

												if (Reg_Bit_Count == 8)
												{
													Reg_Bit_Count = 0;

													Command_Mode = true;
												}

												break;
										}

										RTC_Clock = false;
									}
								}
							}
						}

						if ((Port_Dir & 1) == 0)
						{
							read_value_rtc |= (byte)(!RTC_Clock ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_rtc |= RTC_SIO;
						}
						if ((Port_Dir & 4) == 0)
						{
							read_value_rtc |= 4;
						}

						Port_State = read_value_rtc;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
						}
					}
					else
					{
						// nothing here, return 0?

						Port_State = read_value_solar;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
						}

						//Console.WriteLine(Current_C4);
					}
				}
				else
				{
					// enable Solar Sensor
					if ((value & 4) == 0)
					{
						Chip_Select = false;

						// nothing here, return 0?

						Port_State = read_value_solar;

						if (Ports_RW == 1)
						{
							Core.ROM[0xC4] = Port_State;
							Current_C4 = Core.ROM[0xC4];
						}
					}
					else
					{
						Chip_Select = true;

						if ((Port_Dir & 1) == 0)
						{
							read_value_rtc |= (byte)(!RTC_Clock ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_rtc |= RTC_SIO;
						}
						if ((Port_Dir & 4) == 0)
						{
							read_value_rtc |= 4;
						}

						Port_State = read_value_rtc;

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


		public void Update_Clock()
		{
			ulong update_cycles = Core.CycleCount - Core.Clock_Update_Cycle;

			byte temp = 0;

			bool update_days = false;
			bool pm_flag = false;

			// using clock rate of 16,777,216 cycles per second
			while (update_cycles >= 16777216)
			{
				temp = To_Byte(Reg_Second);

				temp += 1;

				if (temp == 60)
				{
					Reg_Second = 0;

					temp = To_Byte(Reg_Minute);

					temp += 1;

					if (temp == 60)
					{
						Reg_Minute = 0;

						temp = To_Byte((byte)(Reg_Hour & 0x3F));

						temp += 1;

						if (RTC_24_Hour)
						{
							if (temp == 24)
							{
								Reg_Hour = 0;
								update_days = true;
							}
							else
							{
								if (temp >= 12) { pm_flag = true; }

								Reg_Hour = To_BCD(temp);

								if (pm_flag) { Reg_Hour |= 0x80; }
							}
						}
						else
						{
							if (temp == 12)
							{
								if ((Reg_Hour & 0x40) == 0x40)
								{
									Reg_Hour = 0;

									update_days = true;
								}
								else
								{
									Reg_Hour = 0x40;
								}
							}
							else
							{
								pm_flag = ((Reg_Hour & 0x40) == 0x80);

								Reg_Hour = To_BCD(temp);

								if (pm_flag) { Reg_Hour |= 0x80; }
							}
						}

						if (update_days)
						{
							Update_YMD();
						}
					}
					else
					{
						Reg_Minute = To_BCD(temp);
					}
				}
				else
				{
					Reg_Second = To_BCD(temp);
				}

				update_cycles -= 16777216;
			}

			Core.Clock_Update_Cycle = Core.CycleCount - update_cycles;
		}

		public void Update_YMD()
		{
			byte temp = 0;
			byte temp2 = 0;

			Reg_Week += 1;

			if (Reg_Week == 7)
			{
				Reg_Week = 0;
			}

			temp = To_Byte(Reg_Day);

			temp += 1;

			switch (To_Byte(Reg_Month))
			{
				case 1:
				case 3:
				case 5:
				case 7:
				case 8:
				case 10:
					if (temp == 32)
					{
						Reg_Day = 0;

						temp = To_Byte(Reg_Month);

						temp += 1;

						Reg_Month = To_BCD(temp);
					}
					else
					{
						Reg_Day = To_BCD(temp);
					}
					break;

				case 4:
				case 6:
				case 9:
				case 11:
					if (temp == 31)
					{
						Reg_Day = 0;

						temp = To_Byte(Reg_Month);

						temp += 1;

						Reg_Month = To_BCD(temp);
					}
					else
					{
						Reg_Day = To_BCD(temp);
					}
					break;

				case 2:
					// leap year
					temp2 = To_Byte(Reg_Year);

					if ((temp2 & 3) == 0)
					{
						if (temp == 30)
						{
							Reg_Day = 0;

							temp = To_Byte(Reg_Month);

							temp += 1;

							Reg_Month = To_BCD(temp);
						}
						else
						{
							Reg_Day = To_BCD(temp);
						}
					}
					else
					{
						if (temp == 29)
						{
							Reg_Day = 0;

							temp = To_Byte(Reg_Month);

							temp += 1;

							Reg_Month = To_BCD(temp);
						}
						else
						{
							Reg_Day = To_BCD(temp);
						}
					}
					break;

				case 12:
					if (temp == 32)
					{
						Reg_Day = 0;

						Reg_Month = 0;

						temp = To_Byte(Reg_Year);

						temp += 1;

						if (temp == 100)
						{
							Reg_Year = 0; // does it match up with day of week?
						}
						else
						{
							Reg_Year = To_BCD(temp);
						}
					}
					else
					{
						Reg_Day = To_BCD(temp);
					}
					break;
			}
		}

		public byte To_BCD(byte in_byte)
		{
			byte tens_cnt = 0;

			while (in_byte > 10)
			{
				tens_cnt += 1;
				in_byte -= 10;
			}

			return (byte)((tens_cnt << 4) | in_byte);
		}

		public byte To_Byte(byte in_BCD)
		{
			return (byte)((in_BCD & 0xF) + 10 * ((in_BCD >> 4) & 0xF));
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

			ser.Sync(nameof(Chip_Select), ref Chip_Select);

			ser.Sync(nameof(Port_State), ref Port_State);
			ser.Sync(nameof(Port_Dir), ref Port_Dir);
			ser.Sync(nameof(Ports_RW), ref Ports_RW);

			ser.Sync(nameof(Current_C4), ref Current_C4);
			ser.Sync(nameof(Current_C5), ref Current_C5);
			ser.Sync(nameof(Current_C6), ref Current_C6);
			ser.Sync(nameof(Current_C7), ref Current_C7);
			ser.Sync(nameof(Current_C8), ref Current_C8);
			ser.Sync(nameof(Current_C9), ref Current_C9);

			Core.ROM[0xC4] = Current_C4;
			Core.ROM[0xC5] = Current_C5;
			Core.ROM[0xC6] = Current_C6;
			Core.ROM[0xC7] = Current_C7;
			Core.ROM[0xC8] = Current_C8;
			Core.ROM[0xC9] = Current_C9;

			ser.Sync(nameof(Command_Mode), ref Command_Mode);
			ser.Sync(nameof(RTC_Clock), ref RTC_Clock);
			ser.Sync(nameof(RTC_Read), ref RTC_Read);
			ser.Sync(nameof(RTC_24_Hour), ref RTC_24_Hour);

			ser.Sync(nameof(Command_Byte), ref Command_Byte);
			ser.Sync(nameof(RTC_SIO), ref RTC_SIO);
			ser.Sync(nameof(Command_Bit), ref Command_Bit);
			ser.Sync(nameof(Command_Bit_Count), ref Command_Bit_Count);
			ser.Sync(nameof(Reg_Bit), ref Reg_Bit);
			ser.Sync(nameof(Reg_Bit_Count), ref Reg_Bit_Count);
			ser.Sync(nameof(Reg_Access), ref Reg_Access);

			ser.Sync(nameof(Reg_Year), ref Reg_Year);
			ser.Sync(nameof(Reg_Month), ref Reg_Month);
			ser.Sync(nameof(Reg_Week), ref Reg_Week);
			ser.Sync(nameof(Reg_Day), ref Reg_Day);
			ser.Sync(nameof(Reg_Hour), ref Reg_Hour);
			ser.Sync(nameof(Reg_Minute), ref Reg_Minute);
			ser.Sync(nameof(Reg_Second), ref Reg_Second);

			ser.Sync(nameof(Reg_Ctrl), ref Reg_Ctrl);

			ser.Sync(nameof(RTC_Temp_Write), ref RTC_Temp_Write);
		}
	}
}
