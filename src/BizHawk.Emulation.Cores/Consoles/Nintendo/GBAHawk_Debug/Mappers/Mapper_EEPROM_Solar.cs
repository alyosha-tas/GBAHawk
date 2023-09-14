using BizHawk.Common;
using System;
using System.Security.Cryptography.X509Certificates;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	// Cart with EEEPROM and solar sensor with RTC, used in Boktai games
	public class MapperEEPROM_Solar : MapperBase
	{
		// general and solar sensor variables
		public bool Ready_Flag;
		public bool Chip_Select;
		public bool Solar_Clock;
		public bool Solar_Flag;
		public bool Solar_Reset;

		public byte Port_State;
		public byte Port_Dir;
		public byte Ports_RW;

		public byte Current_C4, Current_C5, Current_C6, Current_C7, Current_C8, Current_C9;

		public ushort Solar_Clock_Count;

		// EEPROM Variables
		public int Size_Mask = 0;
		public int Bit_Offset, Bit_Read;
		public int Access_Address;

		// 0 = ready for command
		// 2 = writing
		// 3 = reading
		// 5 = get address for write
		// 6 = get address for read
		public int Current_State;
		public int Next_State;

		public ulong Next_Ready_Cycle;

		// RTC Variables
		public bool Command_Mode;
		public bool RTC_Clock;
		public bool RTC_Read;

		public byte Command_Byte;
		public byte RTC_SIO;
		public byte Command_Bit, Command_Bit_Count;
		public byte Reg_Bit, Reg_Bit_Count;
		public byte Reg_Access;

		public ulong RTC_Temp_Write;

		public override void Reset()
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

			Core.ROM[0xC4] = Current_C4;
			Core.ROM[0xC5] = Current_C5;
			Core.ROM[0xC6] = Current_C6;
			Core.ROM[0xC7] = Current_C7;
			Core.ROM[0xC8] = Current_C8;
			Core.ROM[0xC9] = Current_C9;

			// set up initial variables for EEPROM
			Size_Mask = Core.cart_RAM.Length - 1;

			Bit_Offset = Bit_Read = 0;

			Access_Address = 0;

			Current_State = 0;

			Next_State = 0;

			Next_Ready_Cycle = Core.CycleCount;

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

		// EEPROM is not mapped to SRAM region
		public override byte ReadMemory8(uint addr)
		{
			return 0xFF; // nothing mapped here
		}

		public override ushort ReadMemory16(uint addr)
		{
			return 0xFFFF; // nothing mapped here
		}

		public override uint ReadMemory32(uint addr)
		{
			return 0xFFFFFFFF; // nothing mapped here
		}

		public override byte PeekMemory(uint addr)
		{
			return ReadMemory8(addr);
		}

		public override void WriteMemory8(uint addr, byte value)
		{

		}

		public override void WriteMemory16(uint addr, ushort value)
		{
		
		}

		public override void WriteMemory32(uint addr, uint value)
		{

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
															Reg_Hour = (byte)((RTC_Temp_Write >> 32) & 0xBF);
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

														if (RTC_24_Hour)
														{
															if ((Reg_Hour & 0x80) == 0x80)
															{
																byte temp_h1 = To_Byte((byte)(Reg_Hour & 0x3F));

																if (temp_h1 < 12)
																{
																	temp_h1 += 12;

																	Reg_Hour = To_Byte(temp_h1);
																	Reg_Hour |= 0x80;
																}
															}
														}
														else
														{
															byte temp_h2 = To_Byte((byte)(Reg_Hour & 0x3F));

															if (temp_h2 >= 12)
															{
																temp_h2 -= 12;

																Reg_Hour = To_Byte(temp_h2);
																Reg_Hour |= 0x80;
															}
														}
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
															Reg_Hour = (byte)((RTC_Temp_Write) & 0xBF);
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

						// if we want the RTC to be non-functional, always return zero
						if (!RTC_Functional) { RTC_SIO = 0; }

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
						// Solar			
						if ((Port_Dir & 2) == 2)
						{
							if ((value & 2) == 2)
							{
								Solar_Reset = true;
								Solar_Flag = false;
								Solar_Clock = false;
								Solar_Clock_Count = 0;
							}
							else
							{
								Solar_Reset = false;
							}
						}

						if ((Port_Dir & 1) == 1)
						{
							if ((value & 1) == 0)
							{
								if (!Solar_Reset)
								{
									Solar_Clock = true;
								}
							}
							else
							{
								if (!Solar_Reset)
								{
									if (Solar_Clock)
									{
										Solar_Clock_Count += 1;
										Solar_Clock_Count &= 0xFFF;
									}

									Solar_Clock = false;
								}

								//Console.WriteLine("Res: " + Solar_Reset + " Clk: " + Solar_Clock + " Cnt: " + Solar_Clock_Count);
							}
						}

						if (Core.Solar_state == Solar_Clock_Count)
						{
							if (!Solar_Reset)
							{
								Solar_Flag = true;
								//Console.WriteLine("Match");
							}
						}

						if ((Port_Dir & 1) == 0)
						{
							read_value_solar |= (byte)(!Solar_Clock ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_solar |= (byte)(Solar_Reset ? 2 : 0);
						}
						if ((Port_Dir & 8) == 0)
						{
							read_value_solar |= (byte)(Solar_Flag ? 8 : 0);
						}

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

						if ((Port_Dir & 1) == 0)
						{
							read_value_solar |= (byte)(!Solar_Clock ? 1 : 0);
						}
						if ((Port_Dir & 2) == 0)
						{
							read_value_solar |= (byte)(Solar_Reset ? 2 : 0);
						}
						if ((Port_Dir & 8) == 0)
						{
							read_value_solar |= (byte)(Solar_Flag ? 8 : 0);
						}

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

		public override byte Mapper_EEPROM_Read()
		{
			int cur_read_bit = 0;
			int cur_read_byte = 0;

			byte ret = 0;
			
			if (Current_State == 0)
			{
				if (Core.CycleCount >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				return (byte)(Ready_Flag ? 1 : 0);
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

					ret = Core.cart_RAM[(Access_Address << 3) + cur_read_byte];

					ret >>= cur_read_bit;

					ret &= 1;

					Bit_Read++;

					//Console.WriteLine("Read get data: " + ret + " Bit_Read: " + Bit_Read);

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

		public override void Mapper_EEPROM_Write(byte value)
		{
			int cur_write_bit = 0;
			int cur_write_byte = 0;

			if (Current_State == 0)
			{
				if (Core.CycleCount >= Next_Ready_Cycle)
				{
					Ready_Flag = true;
				}

				//Console.WriteLine("state 0: " + (value & 1) + " Bit_Offset: " + Bit_Offset);

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

					Core.cart_RAM[(Access_Address << 3) + cur_write_byte] &= (byte)(~(1 << cur_write_bit));

					Core.cart_RAM[(Access_Address << 3) + cur_write_byte] |= (byte)((value & 1) << cur_write_bit);
				}
				
				Bit_Read++;

				if (Bit_Read == 65)
				{
					if ((value & 1) == 0)
					{
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core.CycleCount + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core.CycleCount + 0x1A750;
					}

					Ready_Flag = false;
				}
			}
			else if (Current_State == 3)
			{
				// Nothing occurs in read state?
				//Console.WriteLine("Bad write");
			}
			else if (Current_State == 6)
			{
				// Get Address
				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 6)
					{
						Access_Address |= (int)(value & 1);
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
						Access_Address |= (int)(value & 1);
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

				//Console.WriteLine("Read get addr: " + (value & 1) + " Bit_Offset: " + Bit_Offset);

				if (Size_Mask == 0x1FF)
				{
					if (Bit_Offset < 7)
					{
						if (Bit_Offset < 6)
						{
							Access_Address |= (int)(value & 1);
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
							Access_Address |= (int)(value & 1);
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

		public override void SyncState(Serializer ser)
		{
			ser.Sync(nameof(Ready_Flag), ref Ready_Flag);
			ser.Sync(nameof(Chip_Select), ref Chip_Select);
			ser.Sync(nameof(Solar_Clock), ref Solar_Clock);
			ser.Sync(nameof(Solar_Flag), ref Solar_Flag);
			ser.Sync(nameof(Solar_Reset), ref Solar_Reset);

			ser.Sync(nameof(Port_State), ref Port_State);
			ser.Sync(nameof(Port_Dir), ref Port_Dir);
			ser.Sync(nameof(Ports_RW), ref Ports_RW);

			ser.Sync(nameof(Current_C4), ref Current_C4);
			ser.Sync(nameof(Current_C5), ref Current_C5);
			ser.Sync(nameof(Current_C6), ref Current_C6);
			ser.Sync(nameof(Current_C7), ref Current_C7);
			ser.Sync(nameof(Current_C8), ref Current_C8);
			ser.Sync(nameof(Current_C9), ref Current_C9);

			ser.Sync(nameof(Solar_Clock_Count), ref Solar_Clock_Count);

			Core.ROM[0xC4] = Current_C4;
			Core.ROM[0xC5] = Current_C5;
			Core.ROM[0xC6] = Current_C6;
			Core.ROM[0xC7] = Current_C7;
			Core.ROM[0xC8] = Current_C8;
			Core.ROM[0xC9] = Current_C9;

			ser.Sync(nameof(Bit_Offset), ref Bit_Offset);
			ser.Sync(nameof(Bit_Read), ref Bit_Read);

			ser.Sync(nameof(Current_State), ref Current_State);
			ser.Sync(nameof(Next_State), ref Next_State);

			ser.Sync(nameof(Access_Address), ref Access_Address);

			ser.Sync(nameof(Next_Ready_Cycle), ref Next_Ready_Cycle);

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
