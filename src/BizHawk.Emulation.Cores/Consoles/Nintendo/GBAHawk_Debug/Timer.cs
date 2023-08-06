using System;
using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
/*
	Timer Emulation
	NOTES: 

	Can tick-by-previous be changed while channel is running?

	Need to test more edge cases of changing prescaler exactly when a tick would happen

	How is count up timing effected by the glitch for activiating with value 0xFFFF?	

	Todo: check timing of sound FIFO interrupts
*/

	partial class GBAHawk_Debug
	{
		public readonly ushort[] PreScales = { 0, 0x3F, 0xFF, 0x3FF };

		public ushort[] tim_Timer = new ushort[4];

		public ushort[] tim_Reload = new ushort[4];

		public ushort[] tim_Control = new ushort[4];

		public ushort[] tim_PreSc = new ushort[4];

		public ushort[] tim_PreSc_En = new ushort[4];

		public ushort[] tim_ST_Time = new ushort[4];

		public ushort[] tim_IRQ_CD = new ushort[4];

		public ushort[] tim_IRQ_Time = new ushort[4];

		public bool[] tim_Go = new bool[4];

		public bool[] tim_Tick_By_Prev = new bool[4];

		public bool[] tim_Prev_Tick = new bool[5];

		public bool[] tim_Disable = new bool[4];

		public int tim_Just_Reloaded;

		public ushort tim_SubCnt;

		public ushort tim_Old_Reload;

		public bool tim_All_Off;

		public byte tim_Read_Reg_8(uint addr)
		{
			byte ret = 0;

			switch (addr)
			{
				case 0x100: ret = (byte)(tim_Timer[0] & 0xFF); break;
				case 0x101: ret = (byte)((tim_Timer[0] & 0xFF00) >> 8); break;
				case 0x102: ret = (byte)(tim_Control[0] & 0xFF); break;
				case 0x103: ret = (byte)((tim_Control[0] & 0xFF00) >> 8); break;

				case 0x104: ret = (byte)(tim_Timer[1] & 0xFF); break;
				case 0x105: ret = (byte)((tim_Timer[1] & 0xFF00) >> 8); break;
				case 0x106: ret = (byte)(tim_Control[1] & 0xFF); break;
				case 0x107: ret = (byte)((tim_Control[1] & 0xFF00) >> 8); break;

				case 0x108: ret = (byte)(tim_Timer[2] & 0xFF); break;
				case 0x109: ret = (byte)((tim_Timer[2] & 0xFF00) >> 8); break;
				case 0x10A: ret = (byte)(tim_Control[2] & 0xFF); break;
				case 0x10B: ret = (byte)((tim_Control[2] & 0xFF00) >> 8); break;

				case 0x10C: ret = (byte)(tim_Timer[3] & 0xFF); break;
				case 0x10D: ret = (byte)((tim_Timer[3] & 0xFF00) >> 8); break;
				case 0x10E: ret = (byte)(tim_Control[3] & 0xFF); break;
				case 0x10F: ret = (byte)((tim_Control[3] & 0xFF00) >> 8); break;

				default: ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		public ushort tim_Read_Reg_16(uint addr)
		{
			ushort ret = 0;

			switch (addr)
			{
				case 0x100: ret = tim_Timer[0]; break;
				case 0x102: ret = tim_Control[0]; break;

				case 0x104: ret = tim_Timer[1]; break;
				case 0x106: ret = tim_Control[1]; break;

				case 0x108: ret = tim_Timer[2]; break;
				case 0x10A: ret = tim_Control[2]; break;

				case 0x10C: ret = tim_Timer[3]; break;
				case 0x10E: ret = tim_Control[3]; break;

				default: ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		public uint tim_Read_Reg_32(uint addr)
		{
			uint ret = 0;

			switch (addr)
			{
				case 0x100: ret = (uint)((tim_Control[0] << 16) | tim_Timer[0]); break;
				
				case 0x104: ret = (uint)((tim_Control[1] << 16) | tim_Timer[1]); break;

				case 0x108: ret = (uint)((tim_Control[2] << 16) | tim_Timer[2]); break;

				case 0x10C: ret = (uint)((tim_Control[3] << 16) | tim_Timer[3]); break;

				default: ret = cpu_Last_Bus_Value; break; // open bus
			}

			return ret;
		}

		public void tim_Write_Reg_8(uint addr, byte value)
		{
			switch (addr)
			{
				case 0x100: tim_rld_upd((ushort)((tim_Reload[0] & 0xFF00) | value), 0); break;
				case 0x101: tim_rld_upd((ushort)((tim_Reload[0] & 0x00FF) | (value << 8)), 0); break;
				case 0x102: tim_upd((ushort)((tim_Control[0] & 0xFF00) | value), 0); break;
				case 0x103: tim_Control[0] = (ushort)((tim_Control[0] & 0x00FF) | (value << 8)); break;

				case 0x104: tim_rld_upd((ushort)((tim_Reload[1] & 0xFF00) | value), 1); break;
				case 0x105: tim_rld_upd((ushort)((tim_Reload[1] & 0x00FF) | (value << 8)), 1); break;
				case 0x106: tim_upd((ushort)((tim_Control[1] & 0xFF00) | value), 1); break;
				case 0x107: tim_Control[1] = (ushort)((tim_Control[1] & 0x00FF) | (value << 8)); break;

				case 0x108: tim_rld_upd((ushort)((tim_Reload[2] & 0xFF00) | value), 2); break;
				case 0x109: tim_rld_upd((ushort)((tim_Reload[2] & 0x00FF) | (value << 8)), 2); break;
				case 0x10A: tim_upd((ushort)((tim_Control[2] & 0xFF00) | value), 2); break;
				case 0x10B: tim_Control[2] = (ushort)((tim_Control[2] & 0x00FF) | (value << 8)); break;

				case 0x10C: tim_rld_upd((ushort)((tim_Reload[3] & 0xFF00) | value), 3); break;
				case 0x10D: tim_rld_upd((ushort)((tim_Reload[3] & 0x00FF) | (value << 8)), 3); break;
				case 0x10E: tim_upd((ushort)((tim_Control[3] & 0xFF00) | value), 3); break;
				case 0x10F: tim_Control[3] = (ushort)((tim_Control[3] & 0x00FF) | (value << 8)); break;
			}
		}

		public void tim_Write_Reg_16(uint addr, ushort value)
		{
			switch (addr)
			{
				case 0x100: tim_rld_upd(value, 0); break;
				case 0x102: tim_upd(value, 0); break;

				case 0x104: tim_rld_upd(value, 1); break;
				case 0x106: tim_upd(value, 1); break;

				case 0x108: tim_rld_upd(value, 2); break;
				case 0x10A: tim_upd(value, 2); break;

				case 0x10C: tim_rld_upd(value, 3); break;
				case 0x10E: tim_upd(value, 3); break;
			}
		}

		// Note that in case of 32 bit write, new reload value is used when enabling the timer
		public void tim_Write_Reg_32(uint addr, uint value)
		{
			switch (addr)
			{
				case 0x100: tim_rld_upd((ushort)(value & 0xFFFF), 0);
							tim_upd((ushort)((value >> 16) & 0xFFFF), 0); break;

				case 0x104: tim_rld_upd((ushort)(value & 0xFFFF), 1);
							tim_upd((ushort)((value >> 16) & 0xFFFF), 1); break;

				case 0x108: tim_rld_upd((ushort)(value & 0xFFFF), 2);
							tim_upd((ushort)((value >> 16) & 0xFFFF), 2); break;

				case 0x10C: tim_rld_upd((ushort)(value & 0xFFFF), 3);
							tim_upd((ushort)((value >> 16) & 0xFFFF), 3); break;
			}
		}

		public void tim_rld_upd(ushort value, int nbr)
		{
			tim_Old_Reload = tim_Reload[nbr];

			tim_Reload[nbr] = value;

			tim_Just_Reloaded = nbr;
			//Console.WriteLine("rld " + nbr + " v " + value + " t " + tim_Timer[nbr] + " sub " + tim_SubCnt + " " + CycleCount);
		}

		public void tim_upd(ushort value, int nbr) 	
		{
			if (((tim_Control[nbr] & 0x80) == 0) && ((value & 0x80) != 0))
			{
				tim_Timer[nbr] = tim_Reload[nbr];
				
				tim_ST_Time[nbr] = 3;

				tim_PreSc_En[nbr] = PreScales[value & 3];

				if (nbr != 0) { tim_Tick_By_Prev[nbr] = ((value & 0x4) == 0x4); }

				tim_IRQ_Time[nbr] = 3;

				tim_All_Off = false;

				// if enabling at 0xFFFF and no prescale, it is as if the timer is delayed an extra tick
				// but the IRQ is still triggered immediately
				if ((tim_Timer[nbr] == 0xFFFF) && ((value & 3) == 0) && !tim_Tick_By_Prev[nbr])
				{
					tim_ST_Time[nbr] = 4;
					tim_IRQ_Time[nbr] = 2;
				}
			}
			else if (((tim_Control[nbr] & 0x80) != 0) && ((value & 0x80) != 0))
			{
				// some settings can be updated even while the timer is running (which ones?)
				// what happens if these changes happen very close together? (The cpu could change them within 2 clocks, but takes 3 to start channel)
				// for now use the new value		
				tim_PreSc_En[nbr] = PreScales[value & 3];

				if (tim_ST_Time[nbr] == 0)
				{
					tim_ST_Time[nbr] = 2;
				}			
			}

			if ((value & 0x80) == 0)
			{
				// timer ticks for one additional cycle when disabled
				if (tim_Go[nbr])
				{
					tim_Disable[nbr] = true;
				}
			}

			//Console.WriteLine("ctrl " + nbr + " v " + value + " c " + tim_Reload[nbr] + " sub " + tim_SubCnt + " " + CycleCount);

			tim_Control[nbr] = value;
		}

		public void tim_Tick()
		{
			bool tim_do_tick = false;

			tim_SubCnt += 1;

			if (!tim_All_Off)
			{
				for (int i = 0; i < 4; i++)
				{
					if (tim_ST_Time[i] > 0)
					{
						tim_ST_Time[i] -= 1;

						if (tim_ST_Time[i] == 0)
						{
							tim_Go[i] = true;

							tim_PreSc[i] = tim_PreSc_En[i];
						}
					}

					if (tim_IRQ_CD[i] > 0)
					{
						tim_IRQ_CD[i] -= 1;

						// trigger IRQ
						if (tim_IRQ_CD[i] == 2)
						{
							INT_Flags |= (ushort)(0x8 << i);				
						}
						else if (tim_IRQ_CD[i] == 0)
						{
							if ((INT_EN & INT_Flags & (0x8 << i)) == (0x8 << i))
							{ 
								cpu_Trigger_Unhalt = true;
								if (INT_Master_On) { cpu_IRQ_Input = true; }
							}
						}

						// check if all timers disabled
						if (!tim_Go[i])
						{
							tim_All_Off = true;
							for (int j = 0; j < 4; j++) 
							{ 
								tim_All_Off &= !tim_Go[j];
								tim_All_Off &= (tim_IRQ_CD[j] == 0);
								tim_All_Off &= (tim_ST_Time[j] == 0);
							}
						}
					}

					if (tim_Go[i])
					{
						tim_do_tick = false;

						if (!tim_Tick_By_Prev[i])
						{
							if ((tim_SubCnt & tim_PreSc[i]) == 0)
							{ 
								tim_do_tick = true;
							}					
						}
						else if (tim_Prev_Tick[i]) { tim_do_tick = true; }

						if (tim_do_tick)
						{
							tim_Timer[i] += 1;

							if (tim_Timer[i] == 0)
							{
								if ((tim_Control[i] & 0x40) == 0x40)
								{
									// don't re-trigger if an IRQ is already pending
									if (tim_IRQ_CD[i] == 0)
									{
										tim_IRQ_CD[i] = tim_IRQ_Time[i];

										if (tim_IRQ_CD[i] == 2)
										{ 
											INT_Flags |= (ushort)(0x8 << i); 
										}
									}
								}

								// reload the timer
								tim_Timer[i] = tim_Reload[i];

								// if the reload register was just written to, use the old value
								if (tim_Just_Reloaded == i)
								{
									tim_Timer[i] = tim_Old_Reload;
								}

								tim_Just_Reloaded = i;

								// Trigger sound FIFO updates
								if (snd_FIFO_A_Timer == i) { snd_FIFO_A_Tick = snd_CTRL_power; }
								if (snd_FIFO_B_Timer == i) { snd_FIFO_B_Tick = snd_CTRL_power; }

								tim_Prev_Tick[i + 1] = true;
							}
						}

						if (tim_Disable[i])
						{
							tim_Go[i] = false;

							tim_ST_Time[i] = 0;

							tim_All_Off = true;

							for (int k = 0; k < 4; k++)
							{
								tim_All_Off &= !tim_Go[k];
								tim_All_Off &= (tim_IRQ_CD[k] == 0);
								tim_All_Off &= (tim_ST_Time[k] == 0);
							}

							if (tim_IRQ_CD[i] > 0) { tim_All_Off = false; }

							tim_Disable[i] = false;
						}
					}

					tim_Prev_Tick[i] = false;
				}
			}


			tim_Just_Reloaded = 5;
		}

		public void tim_Reset()
		{
			for (int i = 0; i < 4; i++)
			{
				tim_Timer[i] = 0;
				tim_Reload[i] = 0;
				tim_Control[i] = 0;
				tim_PreSc[i] = 0;
				tim_PreSc_En[i] = 0;
				tim_ST_Time[i] = 0;
				tim_IRQ_CD[i] = 0;
				tim_IRQ_Time[i] = 0;

				tim_Go[i] = false;
				tim_Tick_By_Prev[i] = false;
				tim_Prev_Tick[i] = false;
				tim_Disable[i] = false;
			}

			tim_Just_Reloaded = 5;

			tim_SubCnt = 0;

			tim_Old_Reload = 0;

			tim_All_Off = true;
		}

		public void tim_SyncState(Serializer ser)
		{
			ser.Sync(nameof(tim_Timer), ref tim_Timer, false);
			ser.Sync(nameof(tim_Reload), ref tim_Reload, false);
			ser.Sync(nameof(tim_Control), ref tim_Control, false);
			ser.Sync(nameof(tim_PreSc), ref tim_PreSc, false);
			ser.Sync(nameof(tim_PreSc_En), ref tim_PreSc_En, false);
			ser.Sync(nameof(tim_ST_Time), ref tim_ST_Time, false);
			ser.Sync(nameof(tim_IRQ_CD), ref tim_IRQ_CD, false);
			ser.Sync(nameof(tim_IRQ_Time), ref tim_IRQ_Time, false);

			ser.Sync(nameof(tim_Go), ref tim_Go, false);
			ser.Sync(nameof(tim_Tick_By_Prev), ref tim_Tick_By_Prev, false);
			ser.Sync(nameof(tim_Prev_Tick), ref tim_Prev_Tick, false);
			ser.Sync(nameof(tim_Disable), ref tim_Disable, false);

			ser.Sync(nameof(tim_Just_Reloaded), ref tim_Just_Reloaded);

			ser.Sync(nameof(tim_SubCnt), ref tim_SubCnt);
			ser.Sync(nameof(tim_Old_Reload), ref tim_Old_Reload);

			ser.Sync(nameof(tim_All_Off), ref tim_All_Off);
		}
	}
}