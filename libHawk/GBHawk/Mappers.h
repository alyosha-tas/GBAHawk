#ifndef MAPPERS_H
#define MAPPERS_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace std;

namespace GBHawk
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
		int16_t EEPROM_Offset;
		int16_t Flash_Write_Offset;
		int32_t Flash_Sector_Erase_Offset;
		int32_t Flash_Chip_Erase_Offset;

		uint16_t Flash_Type_64_Value;
		uint16_t Flash_Type_128_Value;

		// stated
		bool Command_Mode;
		bool Command_Mode_Switch;
		bool RTC_Clock_Low;
		bool RTC_Clock_High;
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
		bool Commit_Write_To_Reg;

		uint8_t Port_State;
		uint8_t Port_Dir;
		uint8_t Ports_RW;
		uint8_t CTRL_Reg;
		uint8_t CTRL_Reg_Commit;
		uint8_t CTRL_Reg_Latch;

		uint8_t Command_Byte;
		uint8_t RTC_SIO;
		uint8_t RTC_SIO_Old;
		uint8_t RTC_Latched_SIO;
		uint8_t Command_Bit, Command_Bit_Count;
		uint8_t Reg_Bit, Reg_Bit_Count;
		uint8_t Reg_Access;
		uint8_t Write_Value;
		uint8_t Write_Count; // for Atmel devices only

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

		string* Core_Message_String = nullptr;

		void (*RumbleCallback)(bool);

		void (*MessageCallback)(int);
		
		Mappers()
		{
			Reset();
		}

		virtual uint8_t ReadMemoryLow(uint16_t addr)
		{
			return 0;
		}

		virtual uint8_t ReadMemoryHigh(uint16_t addr)
		{
			return 0;
		}

		virtual uint8_t PeekMemoryLow(uint16_t addr)
		{
			return 0;
		}

		virtual uint8_t PeekMemoryHigh(uint16_t addr)
		{
			return 0;
		}

		virtual void WriteMemory(uint16_t addr, uint8_t value)
		{
		}

		virtual void PokeMemory(uint16_t addr, uint8_t value)
		{
		}

		virtual void Dispose()
		{
		}

		virtual void Reset()
		{
		}

		virtual void Mapper_Tick()
		{
		}

		virtual void RTC_Get(int value, int index)
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
				// not sure if correct or what happens?
				
				// what happens if CS pin is in read direction?
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

				//Core_Message_String->assign("test: " + to_string(change_CS) + " cs: " + to_string(Chip_Select) + " Command: " + to_string(Command_Mode) + " port: " + to_string((Port_Dir & 1)) + " val: " + to_string((value & 1)) + " rtc: " + to_string(RTC_Clock_Low) + " cnt: " + to_string(Reg_Bit_Count) + " dir 2: " + to_string(((Port_Dir & 2) == 2)));

				//MessageCallback(Core_Message_String->length());

				if (!change_CS)
				{
					if (Chip_Select)
					{
						// in write direction SIO always updated, other updates only occur on falling edge
						if ((Port_Dir & 2) == 2)
						{
							RTC_SIO_Old = RTC_SIO;
							
							RTC_SIO = (uint8_t)(value & 2);
						}
						
						// RTC
						// Data is latched in on falling edges and registered into the chip on rising edges
						if ((Port_Dir & 1) == 1)
						{
							if ((value & 1) == 0)
							{
								// always latch in data
								RTC_Latched_SIO = value;

								Update_RTC_Command_Read(RTC_Latched_SIO);

								// Falling edge
								if (RTC_Clock_High)
								{
									RTC_Clock_High = false;
									RTC_Clock_Low = true;
								}
							}

							if ((value & 1) == 1)
							{
								// Rising edge
								if (RTC_Clock_Low)
								{
									RTC_Clock_Low = false;

									Update_RTC_Command_Write(RTC_Latched_SIO);
								}

								RTC_Clock_High = true;
							}
						}

						// if we want the RTC to be non-functional, always return zero
						if (!RTC_Functional) { RTC_SIO = 0; }

						if ((Port_Dir & 1) == 0)
						{
							read_value_rtc |= (uint8_t)(!RTC_Clock_Low ? 1 : 0);
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
							// SIO pin is high
							read_value_solar = 2;
						}

						Port_State = read_value_solar;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
						}
					}
				}
				else
				{
					// enable Solar Sensor
					if ((value & 4) == 0)
					{
						if (Commit_Write_To_Reg)
						{
							if (!RTC_Read)
							{
								CTRL_Reg = CTRL_Reg_Commit;

								RTC_24_Hour = (CTRL_Reg & 0x40) == 0x40;

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

							Commit_Write_To_Reg = false;
						}
											
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
							// SIO pin is high
							read_value_solar = 2;
						}

						Port_State = read_value_solar;

						if (Ports_RW == 1)
						{
							Core_ROM[0xC4] = Port_State;
						}
					}
					else
					{
						Chip_Select = true;

						// always reset to command mode
						Command_Mode = true;
						Command_Mode_Switch = false;
						Command_Bit_Count = 0;
						Command_Byte = 0;

						// falling edge state is not recognized on chip activation, so no update of data
						if ((Port_Dir & 1) == 1)
						{
							if ((value & 1) == 1)
							{
								RTC_Clock_High = true;
								RTC_Clock_Low = false;
							}
							else
							{
								RTC_Clock_High = false;
								RTC_Clock_Low = true;
							}
						}
						else
						{
							RTC_Clock_High = true;
							RTC_Clock_Low = false;
						}

						if ((Port_Dir & 1) == 0)
						{
							read_value_rtc |= (uint8_t)(!RTC_Clock_Low ? 1 : 0);
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

					// in RTC mode, pin 2 (SIO) is always high when in write mode (including command mode) but port is reading 
					if (Chip_Select && RTC_Functional && ((Port_Dir & 2) == 0))
					{
						if (Command_Mode || Command_Mode_Switch || !RTC_Read)
						{
							Port_State |= 2;
						}
					}

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

		void Update_RTC_Command_Read(uint8_t value)
		{
			if (Command_Mode)
			{
				// What is output in command mode when reading?
				if ((Port_Dir & 2) == 0)
				{
					RTC_SIO = 2;
				}
			}
			else
			{
				if ((Port_Dir & 2) == 0)
				{
					RTC_SIO = 2;
				}

				switch (Reg_Access)
				{
				case 0:
					// force reset
					break;

				case 1:
					// purpose unknown, always 0xFF
					break;

				case 2:
					// date time
					if ((Port_Dir & 2) == 0)
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
					}
					break;

				case 3:
					// Force IRQ
					break;

				case 4:
					// Control
					if ((Port_Dir & 2) == 0)
					{
						if (RTC_Read)
						{
							RTC_SIO = (uint8_t)(((CTRL_Reg_Latch >> Reg_Bit_Count) << 1) & 2);
						}
					}
					else
					{
						// corruption occurs on the shift register in some cases, but not the underlying data
						if (RTC_Read)
						{
							// only 0 causes corruption, in new or old state
							if ((RTC_SIO == 0) || (RTC_SIO_Old == 0))
							{
								if (Reg_Bit_Count > 0)
								{
									uint8_t temp_ctrl = (uint8_t)((CTRL_Reg_Latch >> Reg_Bit_Count) & 1);

									CTRL_Reg_Latch &= (0xFF - (1 << (Reg_Bit_Count - 1)));
								}
							}
						}
					}

					if (Commit_Write_To_Reg)
					{
						if (!RTC_Read)
						{
							CTRL_Reg_Commit >>= 1;
							CTRL_Reg_Commit |= (uint8_t)((RTC_Temp_Write & 1) << 7);

							CTRL_Reg = CTRL_Reg_Commit;

							RTC_24_Hour = (CTRL_Reg & 0x40) == 0x40;

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

						Commit_Write_To_Reg = false;
					}
					break;

				case 5:

					break;

				case 6:
					// time
					if ((Port_Dir & 2) == 0)
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
					}
					break;

				case 7:
					// nothing to do, always 0xFF

					break;
				}
			}
		}

		void Update_RTC_Command_Write(uint8_t value)
		{			
			if (Command_Mode)
			{
				Command_Bit = (uint8_t)((value & 2) >> 1);

				Command_Byte |= (uint8_t)(Command_Bit << Command_Bit_Count);

				Command_Bit_Count += 1;

				if (Command_Bit_Count == 8)
				{
					Reg_Bit_Count = 0;

					// change mode if valid command, otherwise start over
					if ((Command_Byte & 0xF) == 6)
					{
						Command_Mode = false;
						Command_Mode_Switch = true;

						if ((Command_Byte & 0x80) == 0x80)
						{
							RTC_Read = true;
						}
						else
						{
							RTC_Read = false;
						}

						Reg_Access = (uint8_t)((Command_Byte & 0x70) >> 4);

						CTRL_Reg_Latch = CTRL_Reg;

						if ((Command_Byte == 0x06) || (Command_Byte == 0x86))
						{
							// register reset
							Reg_Year = Reg_Week = 0;
							Reg_Hour = Reg_Minute = Reg_Second = 0;
							Reg_Day = Reg_Month = 1;

							CTRL_Reg = 0;

							//Command_Mode = true;

							Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0];
						}
						else if ((Command_Byte == 0x36) || (Command_Byte == 0xB6))
						{
							// Force IRQ
							//Command_Mode = true;
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
			}
			else
			{
				Command_Mode_Switch = false;

				switch (Reg_Access)
				{
					case 0:
						// force reset
						Reg_Year = Reg_Week = 0;
						Reg_Hour = Reg_Minute = Reg_Second = 0;
						Reg_Day = Reg_Month = 1;

						CTRL_Reg = 0;

						Core_Clock_Update_Cycle[0] = Core_Cycle_Count[0];
						break;

					case 1:
						// purpose unknown, always 0xFF
						Reg_Bit_Count += 1;

						if (Reg_Bit_Count == 8)
						{
							Reg_Bit_Count = 0;
						}

						break;

					case 2:
						// date time
						if ((Port_Dir & 2) == 2)
						{
							if (!RTC_Read)
							{
								RTC_Temp_Write |= (((uint64_t)(RTC_Latched_SIO & 2) >> 1) << Reg_Bit_Count);
							}
						}

						Reg_Bit_Count += 1;

						if (Reg_Bit_Count == 56)
						{
							Reg_Bit_Count = 0;

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

							RTC_Temp_Write = 0;
						}
						break;

					case 3:
						// Force IRQ
						break;

					case 4:
						// Control
						if ((Port_Dir & 2) == 2)
						{
							if (!RTC_Read)
							{
								RTC_Temp_Write |= (((uint64_t)(RTC_Latched_SIO & 2) >> 1) << Reg_Bit_Count);
							}
						}

						if (Commit_Write_To_Reg)
						{
							if (!RTC_Read)
							{
								CTRL_Reg_Commit >>= 1;
								CTRL_Reg_Commit |= (uint8_t)((RTC_Temp_Write & 1) << 7);

								CTRL_Reg = CTRL_Reg_Commit;

								RTC_24_Hour = (CTRL_Reg & 0x40) == 0x40;

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

							Commit_Write_To_Reg = false;
						}

						Reg_Bit_Count += 1;

						if (Reg_Bit_Count == 8)
						{
							Reg_Bit_Count = 0;

							Commit_Write_To_Reg = true;

							CTRL_Reg_Commit = (uint8_t)RTC_Temp_Write;

							RTC_Temp_Write = 0;
						}
						break;

					case 5:
						// nothing to do, always 0xFF
						Reg_Bit_Count += 1;

						if (Reg_Bit_Count == 8)
						{
							Reg_Bit_Count = 0;
						}

						break;

					case 6:
						// time
						if ((Port_Dir & 2) == 2)
						{
							if (!RTC_Read)
							{
								RTC_Temp_Write |= (((uint64_t)(RTC_Latched_SIO & 2) >> 1) << Reg_Bit_Count);
							}
						}

						Reg_Bit_Count += 1;

						if (Reg_Bit_Count == 24)
						{
							Reg_Bit_Count = 0;

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

							RTC_Temp_Write = 0;
						}

						break;

					case 7:
						// nothing to do, always 0xFF
						Reg_Bit_Count += 1;

						if (Reg_Bit_Count == 8)
						{
							Reg_Bit_Count = 0;
						}

						break;
				}
			}
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
			saver = bool_saver(Command_Mode_Switch, saver);
			saver = bool_saver(RTC_Clock_Low, saver);
			saver = bool_saver(RTC_Clock_High, saver);
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
			saver = bool_saver(Commit_Write_To_Reg, saver);

			saver = byte_saver(Port_State, saver);
			saver = byte_saver(Port_Dir, saver);
			saver = byte_saver(Ports_RW, saver);

			saver = byte_saver(Command_Byte, saver);
			saver = byte_saver(RTC_SIO, saver);
			saver = byte_saver(RTC_SIO_Old, saver);
			saver = byte_saver(RTC_Latched_SIO, saver);
			saver = byte_saver(Command_Bit, saver); 
			saver = byte_saver(Command_Bit_Count, saver);
			saver = byte_saver(Reg_Bit, saver);
			saver = byte_saver(Reg_Bit_Count, saver);
			saver = byte_saver(Reg_Access, saver);
			saver = byte_saver(CTRL_Reg, saver);
			saver = byte_saver(Write_Value, saver);
			saver = byte_saver(Write_Count, saver);

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
			saver = byte_saver(CTRL_Reg_Commit, saver);
			saver = byte_saver(CTRL_Reg_Latch, saver);

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
			loader = bool_loader(&Command_Mode_Switch, loader);
			loader = bool_loader(&RTC_Clock_Low, loader);
			loader = bool_loader(&RTC_Clock_High, loader);
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
			loader = bool_loader(&Commit_Write_To_Reg, loader);

			loader = byte_loader(&Port_State, loader);
			loader = byte_loader(&Port_Dir, loader);
			loader = byte_loader(&Ports_RW, loader);

			loader = byte_loader(&Command_Byte, loader);
			loader = byte_loader(&RTC_SIO, loader);
			loader = byte_loader(&RTC_SIO_Old, loader);
			loader = byte_loader(&RTC_Latched_SIO, loader);
			loader = byte_loader(&Command_Bit, loader);
			loader = byte_loader(&Command_Bit_Count, loader);
			loader = byte_loader(&Reg_Bit, loader);
			loader = byte_loader(&Reg_Bit_Count, loader);
			loader = byte_loader(&Reg_Access, loader);
			loader = byte_loader(&CTRL_Reg, loader);
			loader = byte_loader(&Write_Value, loader);
			loader = byte_loader(&Write_Count, loader);

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
			loader = byte_loader(&CTRL_Reg_Commit, loader);
			loader = byte_loader(&CTRL_Reg_Latch, loader);

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
			to_load[0] = *loader == 1; loader++;

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
			to_load[0] = *loader; loader++; to_load[0] |= (uint32_t)(*loader << 8); loader++;
			to_load[0] |= (uint32_t)(*loader << 16); loader++; to_load[0] |= (uint32_t)(*loader << 24); loader++;

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
}

#endif
