using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public class MapperBase
	{
		public GBAHawk_Debug Core { get; set; }

		public byte ROM_C4, ROM_C5, ROM_C6, ROM_C7, ROM_C8, ROM_C9;

		public byte Reg_Year, Reg_Month, Reg_Day, Reg_Week, Reg_Hour, Reg_Minute, Reg_Second;

		public byte Reg_Ctrl;

		public bool RTC_24_Hour;

		public virtual byte ReadMemory8(uint addr)
		{
			return 0;
		}

		public virtual ushort ReadMemory16(uint addr)
		{
			return 0;
		}

		public virtual uint ReadMemory32(uint addr)
		{
			return 0;
		}

		public virtual byte PeekMemory(uint addr)
		{
			return 0;
		}

		public virtual void WriteMemory8(uint addr, byte value)
		{
		}

		public virtual void WriteMemory16(uint addr, ushort value)
		{
		}

		public virtual void WriteMemory32(uint addr, uint value)
		{
		}

		public virtual void WriteROM8(uint addr, byte value)
		{
		}

		public virtual void WriteROM16(uint addr, ushort value)
		{
		}

		public virtual void WriteROM32(uint addr, uint value)
		{
		}

		public virtual void PokeMemory(uint addr, byte value)
		{
		}

		public virtual void SyncState(Serializer ser)
		{
		}

		public virtual void Dispose()
		{
		}

		public virtual void Reset()
		{
		}

		public virtual byte Mapper_EEPROM_Read()
		{
			return 0xFF;
		}

		public virtual void Mapper_EEPROM_Write(byte value)
		{

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

			while (in_byte >= 10)
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
	}
}
