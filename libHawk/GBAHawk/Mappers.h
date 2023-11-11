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
 
		// not stated, initialized with power on
		bool Reset_RTC;
		bool RTC_Functional;
		bool Solar_Functional;
		uint8_t ROM_C4, ROM_C5, ROM_C6, ROM_C7, ROM_C8, ROM_C9;

		// stated
		bool Command_Mode;
		bool RTC_Clock;
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

		uint8_t Port_State;
		uint8_t Port_Dir;
		uint8_t Ports_RW;

		uint8_t Command_Byte;
		uint8_t RTC_SIO;
		uint8_t Command_Bit, Command_Bit_Count;
		uint8_t Reg_Bit, Reg_Bit_Count;
		uint8_t Reg_Access;
		uint8_t Reg_Ctrl;
		uint8_t Write_Value;

		uint8_t Current_C4, Current_C5, Current_C6, Current_C7, Current_C8, Current_C9;

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
		
		uint64_t Next_Ready_Cycle;

		uint64_t RTC_Temp_Write;

		uint64_t* Core_Cycle_Count = nullptr;

		uint64_t* Core_Clock_Update_Cycle = nullptr;

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

	#pragma region Common RTC Functions
		
		void Write_IO_RTC(uint32_t addr, uint8_t value)
		{
			bool change_CS = false;
			uint8_t read_value_solar = 0;
			uint8_t read_value_rtc = 0;

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
											Command_Bit = (uint8_t)((value & 2) >> 1);
											RTC_SIO = (uint8_t)(value & 2);
										}
										else
										{
											Command_Bit = 0;
											RTC_SIO = 0;
										}

										Command_Byte |= (uint8_t)(Command_Bit << Command_Bit_Count);

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

												Reg_Access = (uint8_t)((Command_Byte & 0x70) >> 4);

												//Console.WriteLine("Read: " + RTC_Read + " Reg_Access: " + Reg_Access + " Command: " + Command_Byte);

												if ((Command_Byte == 0x06) || (Command_Byte == 0x86))
												{
													// register reset
													Reg_Year = Reg_Week = 0;
													Reg_Hour = Reg_Minute = Reg_Second = 0;
													Reg_Day = Reg_Month = 1;

													Reg_Ctrl = 0;

													Command_Mode = true;

													Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0];
												}
												else if ((Command_Byte == 0x36) || (Command_Byte == 0xB6))
												{
													// Force IRQ
													Command_Mode = true;
												}
											}
											else
											{
												// bad command, restart
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

											Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0];
											break;

										case 1:
											// purpose unknown, always 0xFF
											if ((Port_Dir & 2) == 2)
											{
												RTC_SIO = (uint8_t)(value & 2);
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
												RTC_SIO = (uint8_t)(value & 2);

												if (!RTC_Read)
												{
													RTC_Temp_Write |= (((uint64_t)(value & 2) >> 1) << Reg_Bit_Count);
												}
											}
											else
											{
												if (RTC_Read)
												{
													if (Reg_Bit_Count < 8)
													{
														RTC_SIO = (uint8_t)(((Reg_Year >> Reg_Bit_Count) << 1) & 2);
													}
													else if (Reg_Bit_Count < 16)
													{
														RTC_SIO = (uint8_t)(((Reg_Month >> (Reg_Bit_Count - 8)) << 1) & 2);
													}
													else if (Reg_Bit_Count < 24)
													{
														RTC_SIO = (uint8_t)(((Reg_Day >> (Reg_Bit_Count - 16)) << 1) & 2);
													}
													else if (Reg_Bit_Count < 32)
													{
														RTC_SIO = (uint8_t)(((Reg_Week >> (Reg_Bit_Count - 24)) << 1) & 2);
													}
													else if (Reg_Bit_Count < 40)
													{
														RTC_SIO = (uint8_t)(((Reg_Hour >> (Reg_Bit_Count - 32)) << 1) & 2);
													}
													else if (Reg_Bit_Count < 48)
													{
														RTC_SIO = (uint8_t)(((Reg_Minute >> (Reg_Bit_Count - 40)) << 1) & 2);
													}
													else
													{
														RTC_SIO = (uint8_t)(((Reg_Second >> (Reg_Bit_Count - 48)) << 1) & 2);
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
													Reg_Year = (uint8_t)(RTC_Temp_Write);
													Reg_Month = (uint8_t)((RTC_Temp_Write >> 8) & 0x1F);
													Reg_Day = (uint8_t)((RTC_Temp_Write >> 16) & 0x3F);
													Reg_Week = (uint8_t)((RTC_Temp_Write >> 24) & 0x7);

													if (RTC_24_Hour)
													{
														Reg_Hour = (uint8_t)((RTC_Temp_Write >> 32) & 0x3F);

														if (To_Byte(Reg_Hour) >= 12) { Reg_Hour |= 0x80; }
													}
													else
													{
														Reg_Hour = (uint8_t)((RTC_Temp_Write >> 32) & 0xBF);
													}

													Reg_Minute = (uint8_t)((RTC_Temp_Write >> 40) & 0x7F);
													Reg_Second = (uint8_t)((RTC_Temp_Write >> 48) & 0x7F);

													Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0];
												}
											}
											break;

										case 3:
											// Force IRQ
											Command_Mode = true;
											break;

										case 4:
											// Control
											if ((Port_Dir & 2) == 2)
											{
												RTC_SIO = (uint8_t)(value & 2);

												if (!RTC_Read)
												{
													RTC_Temp_Write |= (((uint64_t)(value & 2) >> 1) << Reg_Bit_Count);
												}
											}
											else
											{
												if (RTC_Read)
												{
													RTC_SIO = (uint8_t)(((Reg_Ctrl >> Reg_Bit_Count) << 1) & 2);
												}
												else
												{
													//what is output when in write mode but port is reading?
													RTC_SIO = 2;
												}
											}

											Reg_Bit_Count += 1;

											if (Reg_Bit_Count == 8)
											{
												Reg_Bit_Count = 0;

												Command_Mode = true;

												if (!RTC_Read)
												{
													Reg_Ctrl = (uint8_t)RTC_Temp_Write;

													RTC_24_Hour = (Reg_Ctrl & 0x40) == 0x40;

													if (RTC_24_Hour)
													{
														if ((Reg_Hour & 0x80) == 0x80)
														{
															uint8_t temp_h1 = To_Byte((uint8_t)(Reg_Hour & 0x3F));

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
														uint8_t temp_h2 = To_Byte((uint8_t)(Reg_Hour & 0x3F));

														if (temp_h2 >= 12)
														{
															temp_h2 -= 12;

															Reg_Hour = To_Byte(temp_h2);
															Reg_Hour |= 0x80;
														}
													}
												}
											}
											break;

										case 5:
											// nothing to do, always 0xFF
											if ((Port_Dir & 2) == 2)
											{
												RTC_SIO = (uint8_t)(value & 2);
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
												RTC_SIO = (uint8_t)(value & 2);

												if (!RTC_Read)
												{
													RTC_Temp_Write |= (((uint64_t)(value & 2) >> 1) << Reg_Bit_Count);
												}
											}
											else
											{
												if (RTC_Read)
												{
													if (Reg_Bit_Count < 8)
													{
														RTC_SIO = (uint8_t)(((Reg_Hour >> Reg_Bit_Count) << 1) & 2);
													}
													else if (Reg_Bit_Count < 16)
													{
														RTC_SIO = (uint8_t)(((Reg_Minute >> (Reg_Bit_Count - 8)) << 1) & 2);
													}
													else
													{
														RTC_SIO = (uint8_t)(((Reg_Second >> (Reg_Bit_Count - 16)) << 1) & 2);
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
														Reg_Hour = (uint8_t)((RTC_Temp_Write) & 0x3F);

														if (To_Byte(Reg_Hour) >= 12) { Reg_Hour |= 0x80; }
													}
													else
													{
														Reg_Hour = (uint8_t)((RTC_Temp_Write) & 0xBF);
													}

													Reg_Minute = (uint8_t)((RTC_Temp_Write >> 8) & 0x7F);
													Reg_Second = (uint8_t)((RTC_Temp_Write >> 16) & 0x7F);

													Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0];
												}
											}

											break;

										case 7:
											// nothing to do, always 0xFF
											if ((Port_Dir & 2) == 2)
											{
												RTC_SIO = (uint8_t)(value & 2);
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
							read_value_rtc |= (uint8_t)(!RTC_Clock ? 1 : 0);
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
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
					else
					{
						// Solar			
						if (Solar_Functional)
						{
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
								}
							}

							if (Core_Solar[0] == Solar_Clock_Count)
							{
								if (!Solar_Reset)
								{
									Solar_Flag = true;
								}
							}

							if ((Port_Dir & 1) == 0)
							{
								read_value_solar |= (uint8_t)(!Solar_Clock ? 1 : 0);
							}
							if ((Port_Dir & 2) == 0)
							{
								read_value_solar |= (uint8_t)(Solar_Reset ? 2 : 0);
							}
							if ((Port_Dir & 8) == 0)
							{
								read_value_solar |= (uint8_t)(Solar_Flag ? 8 : 0);
							}
						}
						else
						{
							// nothing to do, remains zero
						}

						Port_State = read_value_solar;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
				}
				else
				{
					// enable Solar Sensor
					if ((value & 4) == 0)
					{
						Chip_Select = false;

						if (Solar_Functional)
						{
							if ((Port_Dir & 1) == 0)
							{
								read_value_solar |= (uint8_t)(!Solar_Clock ? 1 : 0);
							}
							if ((Port_Dir & 2) == 0)
							{
								read_value_solar |= (uint8_t)(Solar_Reset ? 2 : 0);
							}
							if ((Port_Dir & 8) == 0)
							{
								read_value_solar |= (uint8_t)(Solar_Flag ? 8 : 0);
							}
						}
						else
						{
							// nothing to do, always zero
						}

						Port_State = read_value_solar;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
							Current_C4 = Core_ROM[0xC4];
						}
					}
					else
					{
						Chip_Select = true;

						if ((Port_Dir & 1) == 0)
						{
							read_value_rtc |= (uint8_t)(!RTC_Clock ? 1 : 0);
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
		
		void Update_Clock()
		{
			uint64_t update_cycles = Core_Cycle_Count[0] - Core_Clock_Update_Cycle[0];

			uint8_t temp = 0;

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

						temp = To_Byte((uint8_t)(Reg_Hour & 0x3F));

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

			Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0] - update_cycles;
		}

		void Update_YMD()
		{
			uint8_t temp = 0;
			uint8_t temp2 = 0;

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

		uint8_t To_BCD(uint8_t in_byte)
		{
			uint8_t tens_cnt = 0;

			while (in_byte >= 10)
			{
				tens_cnt += 1;
				in_byte -= 10;
			}

			return (uint8_t)((tens_cnt << 4) | in_byte);
		}

		uint8_t To_Byte(uint8_t in_BCD)
		{
			return (uint8_t)((in_BCD & 0xF) + 10 * ((in_BCD >> 4) & 0xF));
		}

	#pragma endregion

	#pragma endregion

	#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Command_Mode, saver);
			saver = bool_saver(RTC_Clock, saver);
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

			saver = byte_saver(Port_State, saver);
			saver = byte_saver(Port_Dir, saver);
			saver = byte_saver(Ports_RW, saver);

			saver = byte_saver(Command_Byte, saver);
			saver = byte_saver(RTC_SIO, saver);
			saver = byte_saver(Command_Bit, saver); 
			saver = byte_saver(Command_Bit_Count, saver);
			saver = byte_saver(Reg_Bit, saver);
			saver = byte_saver(Reg_Bit_Count, saver);
			saver = byte_saver(Reg_Access, saver);
			saver = byte_saver(Reg_Ctrl, saver);
			saver = byte_saver(Write_Value, saver);

			saver = byte_saver(Current_C4, saver);
			saver = byte_saver(Current_C5, saver);
			saver = byte_saver(Current_C6, saver);
			saver = byte_saver(Current_C7, saver);
			saver = byte_saver(Current_C8, saver);
			saver = byte_saver(Current_C9, saver);

			saver = byte_saver(Reg_Year, saver);
			saver = byte_saver(Reg_Month, saver);
			saver = byte_saver(Reg_Day, saver);
			saver = byte_saver(Reg_Week, saver);
			saver = byte_saver(Reg_Hour, saver);
			saver = byte_saver(Reg_Minute, saver);
			saver = byte_saver(Reg_Second, saver);

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

			saver = long_saver(Next_Ready_Cycle, saver);
			saver = long_saver(RTC_Temp_Write, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Command_Mode, loader);
			loader = bool_loader(&RTC_Clock, loader);
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

			loader = byte_loader(&Port_State, loader);
			loader = byte_loader(&Port_Dir, loader);
			loader = byte_loader(&Ports_RW, loader);

			loader = byte_loader(&Command_Byte, loader);
			loader = byte_loader(&RTC_SIO, loader);
			loader = byte_loader(&Command_Bit, loader);
			loader = byte_loader(&Command_Bit_Count, loader);
			loader = byte_loader(&Reg_Bit, loader);
			loader = byte_loader(&Reg_Bit_Count, loader);
			loader = byte_loader(&Reg_Access, loader);
			loader = byte_loader(&Reg_Ctrl, loader);
			loader = byte_loader(&Write_Value, loader);

			loader = byte_loader(&Current_C4, loader);
			loader = byte_loader(&Current_C5, loader);
			loader = byte_loader(&Current_C6, loader);
			loader = byte_loader(&Current_C7, loader);
			loader = byte_loader(&Current_C8, loader);
			loader = byte_loader(&Current_C9, loader);

			loader = byte_loader(&Reg_Year, loader);
			loader = byte_loader(&Reg_Month, loader);
			loader = byte_loader(&Reg_Day, loader);
			loader = byte_loader(&Reg_Week, loader);
			loader = byte_loader(&Reg_Hour, loader);
			loader = byte_loader(&Reg_Minute, loader);
			loader = byte_loader(&Reg_Second, loader);

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

			loader = long_loader(&Next_Ready_Cycle, loader);
			loader = long_loader(&RTC_Temp_Write, loader);

			Core_ROM[0xC4] = Current_C4;
			Core_ROM[0xC5] = Current_C5;
			Core_ROM[0xC6] = Current_C6;
			Core_ROM[0xC7] = Current_C7;
			Core_ROM[0xC8] = Current_C8;
			Core_ROM[0xC9] = Current_C9;

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

	#pragma region Default RTC

	class Mapper_DefaultRTC : public Mappers
	{
	public:

		void Reset()
		{
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
			RTC_Clock = false;
			RTC_Read = false;

			Command_Byte = 0;
			RTC_SIO = 0;
			Command_Bit = Command_Bit_Count = 0;
			Reg_Bit = Reg_Bit_Count = 0;
			Reg_Access = 0;

			RTC_Temp_Write = 0;

			if (Reset_RTC)
			{
				RTC_24_Hour = false;
				Reg_Year = Reg_Week = Reg_Hour = Reg_Minute = Reg_Second = 0;
				Reg_Day = Reg_Month = 1;

				Reg_Ctrl = 0;
			}
		}

		void Write_ROM_8(uint32_t addr, uint8_t value)
		{
			Write_IO_RTC(addr, value);
		}

		void Write_ROM_16(uint32_t addr, uint16_t value)
		{
			Write_ROM_8(addr, (uint16_t)value);
			Write_ROM_8((addr + 1), (uint16_t)(value >> 8));
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
			Write_ROM_8(addr, (uint16_t)value);
			Write_ROM_8((addr + 1), (uint16_t)(value >> 8));
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
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
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
			RTC_Clock = false;
			RTC_Read = false;

			Command_Byte = 0;
			RTC_SIO = 0;
			Command_Bit = Command_Bit_Count = 0;
			Reg_Bit = Reg_Bit_Count = 0;
			Reg_Access = 0;

			RTC_Temp_Write = 0;

			if (Reset_RTC)
			{
				RTC_24_Hour = false;
				Reg_Year = Reg_Week = Reg_Hour = Reg_Minute = Reg_Second = 0;
				Reg_Day = Reg_Month = 1;

				Reg_Ctrl = 0;
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

		uint8_t PeekMemory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		void Write_ROM_8(uint32_t addr, uint8_t value)
		{
			Write_IO_RTC(addr, value);
		}

		void Write_ROM_16(uint32_t addr, uint16_t value)
		{
			Write_ROM_8(addr, (uint16_t)value);
			Write_ROM_8((addr + 1), (uint16_t)(value >> 8));
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
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
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
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
					}
					else
					{
						// error? GBA Tek says it should be zero
						Bit_Read = 0;
						Current_State = 0;
						Next_Ready_Cycle = Core_Cycle_Count[0] + 0x1A750;
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
						ret_value = 0x1B;
					}
					else
					{
						ret_value = 0x13;
					}
				}
				else
				{
					if ((Size_Mask + 1) == 0x10000)
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
					// for the larger chips, it seems a status register is activated on wirtes, which
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

					Next_Ready_Cycle = Core_Cycle_Count[0] + 325;
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
								Next_Ready_Cycle = Core_Cycle_Count[0] + 3 * 430000;

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
							Next_Ready_Cycle = Core_Cycle_Count[0] + 460000;

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
			RTC_Clock = false;
			RTC_Read = false;

			Command_Byte = 0;
			RTC_SIO = 0;
			Command_Bit = Command_Bit_Count = 0;
			Reg_Bit = Reg_Bit_Count = 0;
			Reg_Access = 0;

			RTC_Temp_Write = 0;

			if (Reset_RTC)
			{
				RTC_24_Hour = false;
				Reg_Year = Reg_Week = Reg_Hour = Reg_Minute = Reg_Second = 0;
				Reg_Day = Reg_Month = 1;

				Reg_Ctrl = 0;
			}
		}

		void Write_ROM_8(uint32_t addr, uint8_t value)
		{
			Write_IO_RTC(addr, value);
		}

		void Write_ROM_16(uint32_t addr, uint16_t value)
		{
			Write_ROM_8(addr, (uint16_t)value);
			Write_ROM_8((addr + 1), (uint16_t)(value >> 8));
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
						ret_value = 0x1B;
					}
					else
					{
						ret_value = 0x13;
					}
				}
				else
				{
					if ((Size_Mask + 1) == 0x10000)
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
					// for the larger chips, it seems a status register is activated on wirtes, which
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

					Next_Ready_Cycle = Core_Cycle_Count[0] + 325;
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
								Next_Ready_Cycle = Core_Cycle_Count[0] + 3 * 430000;

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
							Next_Ready_Cycle = Core_Cycle_Count[0] + 460000;

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
}

#endif
