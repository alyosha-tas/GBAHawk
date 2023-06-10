using BizHawk.Common.NumberExtensions;
using BizHawk.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
/*
	Serial Port Emulation
	NOTES: 

	only most basic serial comm implemented, needs a lot more work
*/

#pragma warning disable CS0675 // Bitwise-or operator used on a sign-extended operand

	public partial class GBAHawk_Debug
	{
		public uint ser_RECV_J, ser_TRANS_J;

		public int ser_Delay_cd, key_Delay_cd;
		
		public ushort ser_Data_0, ser_Data_1, ser_Data_2, ser_Data_3, ser_Data_M;

		public ushort ser_CTRL, ser_CTRL_J, ser_STAT_J, ser_Mode;

		public ushort key_CTRL;

		public byte ser_div_cnt, ser_Mask;
		public byte ser_Bit_Count, ser_Bit_Total;

		public bool ser_Internal_Clock, ser_Start;

		public bool ser_Delay, key_Delay;

		public byte ser_OUT_State;

		public byte ser_Read_Reg_8(uint addr)
		{
			byte ret = 0;

			switch (addr)
			{
				case 0x120: ret = (byte)(ser_Data_0 & 0xFF); break;
				case 0x121: ret = (byte)((ser_Data_0 & 0xFF00) >> 8); break;
				case 0x122: ret = (byte)(ser_Data_1 & 0xFF); break;
				case 0x123: ret = (byte)((ser_Data_1 & 0xFF00) >> 8); break;
				case 0x124: ret = (byte)(ser_Data_2 & 0xFF); break;
				case 0x125: ret = (byte)((ser_Data_2 & 0xFF00) >> 8); break;
				case 0x126: ret = (byte)(ser_Data_3 & 0xFF); break;
				case 0x127: ret = (byte)((ser_Data_3 & 0xFF00) >> 8); break;
				case 0x128: ret = (byte)(ser_CTRL & 0xFF); break;
				case 0x129: ret = (byte)((ser_CTRL & 0xFF00) >> 8); break;
				case 0x12A: ret = (byte)(ser_Data_M & 0xFF); break;
				case 0x12B: ret = (byte)((ser_Data_M & 0xFF00) >> 8); break;

				case 0x130: ret = (byte)(controller_state & 0xFF); Is_Lag = false; break;
				case 0x131: ret = (byte)((controller_state & 0xFF00) >> 8); Is_Lag = false; break;
				case 0x132: ret = (byte)(key_CTRL & 0xFF); break;
				case 0x133: ret = (byte)((key_CTRL & 0xFF00) >> 8); break;

				case 0x134: ret = (byte)(ser_Mode & 0xFF); break;
				case 0x135: ret = (byte)((ser_Mode & 0xFF00) >> 8); break;
				case 0x136: ret = 0; break;
				case 0x137: ret = 0; break;

				case 0x140: ret = (byte)(ser_CTRL_J & 0xFF); break;
				case 0x141: ret = (byte)((ser_CTRL_J & 0xFF00) >> 8); break;
				case 0x142: ret = 0; break;
				case 0x143: ret = 0; break;

				case 0x150: ret = (byte)(ser_RECV_J & 0xFF); break;
				case 0x151: ret = (byte)((ser_RECV_J & 0xFF00) >> 8); break;
				case 0x152: ret = (byte)((ser_RECV_J & 0xFF0000) >> 16); break;
				case 0x153: ret = (byte)((ser_RECV_J & 0xFF000000) >> 24); break;
				case 0x154: ret = (byte)(ser_TRANS_J & 0xFF); break;
				case 0x155: ret = (byte)((ser_TRANS_J & 0xFF00) >> 8); break;
				case 0x156: ret = (byte)((ser_TRANS_J & 0xFF0000) >> 16); break;
				case 0x157: ret = (byte)((ser_TRANS_J & 0xFF000000) >> 24); break;
				case 0x158: ret = (byte)(ser_STAT_J & 0xFF); break;
				case 0x159: ret = (byte)((ser_STAT_J & 0xFF00) >> 8); break;
				case 0x15A: ret = 0; break;
				case 0x15B: ret = 0; break;

				default: ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		public ushort ser_Read_Reg_16(uint addr)
		{
			ushort ret = 0;

			switch (addr)
			{
				case 0x120: ret = ser_Data_0; break;
				case 0x122: ret = ser_Data_1; break;
				case 0x124: ret = ser_Data_2; break;
				case 0x126: ret = ser_Data_3; break;
				case 0x128: ret = ser_CTRL; break;
				case 0x12A: ret = ser_Data_M; break;

				case 0x130: ret = controller_state; Is_Lag = false; break;
				case 0x132: ret = key_CTRL; break;

				case 0x134: ret = ser_Mode; break;
				case 0x136: ret = 0; break;

				case 0x140: ret = ser_CTRL_J; break;
				case 0x142: ret = 0; break;

				case 0x150: ret = (ushort)(ser_RECV_J & 0xFFFF); break;
				case 0x152: ret = (ushort)((ser_RECV_J & 0xFFFF0000) >> 16); break;
				case 0x154: ret = (ushort)(ser_TRANS_J & 0xFFFF); break;
				case 0x156: ret = (ushort)((ser_TRANS_J & 0xFFFF0000) >> 16); break;
				case 0x158: ret = ser_STAT_J; break;
				case 0x15A: ret = 0; break;

				default: ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		public uint ser_Read_Reg_32(uint addr)
		{
			uint ret = 0;

			switch (addr)
			{
				case 0x120: ret = (uint)((ser_Data_1 << 16) | ser_Data_0); break;
				case 0x124: ret = (uint)((ser_Data_3 << 16) | ser_Data_2); break;
				case 0x128: ret = (uint)((ser_Data_M << 16) | ser_CTRL); break;

				case 0x130: ret = (uint)((key_CTRL << 16) | controller_state); Is_Lag = false; break;

				case 0x134: ret = (uint)((0x00000000) | ser_Mode); break;

				case 0x140: ret = (uint)((0x00000000) | ser_CTRL_J); break;

				case 0x150: ret = ser_RECV_J; break;
				case 0x154: ret = ser_TRANS_J; break;
				case 0x158: ret = (uint)((0x00000000) | ser_STAT_J); break;

				default: ret = cpu_Last_Bus_Value; break; // open bus
			}

			return ret;
		}

		public void ser_Write_Reg_8(uint addr, byte value)
		{
			switch (addr)
			{
				case 0x120: ser_Data_0 = (ushort)((ser_Data_0 & 0xFF00) | value); break;
				case 0x121: ser_Data_0 = (ushort)((ser_Data_0 & 0x00FF) | (value << 8)); break;
				case 0x122: ser_Data_1 = (ushort)((ser_Data_1 & 0xFF00) | value); break;
				case 0x123: ser_Data_1 = (ushort)((ser_Data_1 & 0x00FF) | (value << 8)); break;
				case 0x124: ser_Data_2 = (ushort)((ser_Data_2 & 0xFF00) | value); break;
				case 0x125: ser_Data_2 = (ushort)((ser_Data_2 & 0x00FF) | (value << 8)); break;
				case 0x126: ser_Data_3 = (ushort)((ser_Data_3 & 0xFF00) | value); break;
				case 0x127: ser_Data_3 = (ushort)((ser_Data_3 & 0x00FF) | (value << 8)); break;
				case 0x128: ser_CTRL_Update((ushort)((ser_CTRL & 0xFF00) | value)); break;
				case 0x129: ser_CTRL_Update((ushort)((ser_CTRL & 0x00FF) | (value << 8))); break;
				case 0x12A: ser_Data_M = (ushort)((ser_Data_M & 0xFF00) | value); break;
				case 0x12B: ser_Data_M = (ushort)((ser_Data_M & 0x00FF) | (value << 8)); break;

				case 0x130: // no effect
				case 0x131: // no effect
				case 0x132: key_CTRL = (ushort)((key_CTRL & 0xFF00) | value); do_controller_check(); do_controller_check_glitch(); break;
				// note no check here, does not seem to trigger onhardware, see joypad.gba
				case 0x133: key_CTRL = (ushort)((key_CTRL & 0x00FF) | (value << 8)); /* do_controller_check(); do_controller_check_glitch(); */ break;

				case 0x134: ser_Mode_Update((ushort)((ser_Mode & 0xFF00) | value)); break;
				case 0x135: ser_Mode_Update((ushort)((ser_Mode & 0x00FF) | (value << 8))); break;

				case 0x140: ser_CTRL_J = (ushort)((ser_CTRL_J & 0xFF00) | value); break;
				case 0x141: ser_CTRL_J = (ushort)((ser_CTRL_J & 0x00FF) | (value << 8)); break;

				case 0x150: ser_RECV_J = (uint)((ser_RECV_J & 0xFFFFFF00) | value); break;
				case 0x151: ser_RECV_J = (uint)((ser_RECV_J & 0xFFFF00FF) | (value << 8)); break;
				case 0x152: ser_RECV_J = (uint)((ser_RECV_J & 0xFF00FFFF) | (value << 16)); break;
				case 0x153: ser_RECV_J = (uint)((ser_RECV_J & 0x00FFFFFF) | (value << 24)); break;
				case 0x154: ser_TRANS_J = (uint)((ser_TRANS_J & 0xFFFFFF00) | value); break;
				case 0x155: ser_TRANS_J = (uint)((ser_TRANS_J & 0xFFFF00FF) | (value << 8)); break;
				case 0x156: ser_TRANS_J = (uint)((ser_TRANS_J & 0xFF00FFFF) | (value << 16)); break;
				case 0x157: ser_TRANS_J = (uint)((ser_TRANS_J & 0x00FFFFFF) | (value << 24)); break;
				case 0x158: ser_STAT_J = (ushort)((ser_STAT_J & 0xFF00) | value); break;
				case 0x159: ser_STAT_J = (ushort)((ser_STAT_J & 0x00FF) | (value << 8)); break;
			}
		}

		public void ser_Write_Reg_16(uint addr, ushort value)
		{
			switch (addr)
			{
				case 0x120: ser_Data_0 = value; break;
				case 0x122: ser_Data_1 = value; break;
				case 0x124: ser_Data_2 = value; break;
				case 0x126: ser_Data_3 = value; break;
				case 0x128: ser_CTRL_Update(value); break;
				case 0x12A: ser_Data_M = value; break;

				case 0x130: // no effect
				case 0x132: key_CTRL = value; do_controller_check(); do_controller_check_glitch(); break;

				case 0x134: ser_Mode_Update(value); break;

				case 0x140: ser_CTRL_J = value; break;

				case 0x150: ser_RECV_J = (uint)((ser_RECV_J & 0xFFFF0000) | value); break;
				case 0x152: ser_RECV_J = (uint)((ser_RECV_J & 0x0000FFFF) | (value << 16)); break;
				case 0x154: ser_TRANS_J = (uint)((ser_TRANS_J & 0xFFFF0000) | value); break;
				case 0x156: ser_TRANS_J = (uint)((ser_TRANS_J & 0x0000FFFF) | (value << 16)); break;
				case 0x158: ser_STAT_J = value; break;
			}
		}

		public void ser_Write_Reg_32(uint addr, uint value)
		{
			switch (addr)
			{
				case 0x120: ser_Data_0 = (ushort)(value & 0xFFFF);
							ser_Data_1 = (ushort)((value >> 16) & 0xFFFF); break;
				case 0x124: ser_Data_2 = (ushort)(value & 0xFFFF);
							ser_Data_3 = (ushort)((value >> 16) & 0xFFFF); break;
				case 0x128: ser_CTRL_Update((ushort)(value & 0xFFFF));
							ser_Data_M = (ushort)((value >> 16) & 0xFFFF); break;

				case 0x130: key_CTRL = (ushort)((value >> 16) & 0xFFFF); do_controller_check(); do_controller_check_glitch(); break;

				case 0x134: ser_Mode_Update((ushort)(value & 0xFFFF)); break;

				case 0x140: ser_CTRL_J = (ushort)(value & 0xFFFF); break;

				case 0x150: ser_RECV_J = value; break;
				case 0x154: ser_TRANS_J = value; break;
				case 0x158: ser_STAT_J = (ushort)(value & 0xFFFF); break;
			}
		}

		public void ser_CTRL_Update(ushort value)
		{
			Console.WriteLine("CTRL: " + ext_name + " " + value + " " + TotalExecutedCycles);
			
			// actiavte the port
			if (!ser_Start && ((value & 0x80) == 0x80))
			{
				ser_Bit_Count = 0;

				ser_Bit_Total = (byte)((value & 0x1000) == 0x1000 ? 32 : 8);

				ser_Mask = (byte)((value & 0x2) == 0x2 ? 0x7 : 0xF);

				ser_Internal_Clock = (value & 0x1) == 0x1;

				ser_Start = true;
			}

			if ((value & 0x80) != 0x80) { ser_Start = false; }

			ser_CTRL = value;

			ser_CTRL |= ser_OUT_State;
		}

		public void ser_Mode_Update(ushort value)
		{
			Console.WriteLine("Mode: " + ext_name + " " + value + " " + TotalExecutedCycles);

			ser_Mode = value;
		}


		public void ser_Tick()
		{
			ser_div_cnt += 1;

			if (ser_Start)
			{
				if (ser_Internal_Clock)
				{
					// transfer 1 bit
					if ((ser_div_cnt & ser_Mask) == 0)
					{
						ser_Bit_Count += 1;

						if (ser_Bit_Count == ser_Bit_Total)
						{
							// reset start bit
							ser_Start = false;
							ser_CTRL &= 0xFF7F;

							// trigger interrupt if needed
							if ((ser_CTRL & 0x4000) == 0x4000)
							{
								INT_Flags |= 0x80;

								ser_Delay = true;
								Misc_Delays = true;
								ser_Delay_cd = 2;
								delays_to_process = true;
							}
						}
					}
				}
			}
		}

		public void ser_Reset()
		{
			ser_RECV_J = ser_TRANS_J = 0;

			ser_Delay_cd = key_Delay_cd = 0;

			ser_Data_0 = ser_Data_1 = ser_Data_2 = ser_Data_3 = ser_Data_M = 0;

			ser_CTRL = ser_OUT_State = 4; // assuming no connection
			
			ser_CTRL_J = ser_STAT_J = ser_Mode = 0;

			key_CTRL = 0;

			ser_div_cnt = 0;

			ser_Mask = 0xF;

			ser_Bit_Count = ser_Bit_Total = 0;

			ser_Internal_Clock = ser_Start = false;

			ser_Delay = key_Delay = false;
		}

		public void ser_SyncState(Serializer ser)
		{
			ser.Sync(nameof(ser_RECV_J), ref ser_RECV_J);
			ser.Sync(nameof(ser_TRANS_J), ref ser_TRANS_J);

			ser.Sync(nameof(ser_Delay_cd), ref ser_Delay_cd);
			ser.Sync(nameof(key_Delay_cd), ref key_Delay_cd);

			ser.Sync(nameof(ser_Data_0), ref ser_Data_0);
			ser.Sync(nameof(ser_Data_1), ref ser_Data_1);
			ser.Sync(nameof(ser_Data_2), ref ser_Data_2);
			ser.Sync(nameof(ser_Data_3), ref ser_Data_3);
			ser.Sync(nameof(ser_Data_M), ref ser_Data_M);

			ser.Sync(nameof(ser_CTRL), ref ser_CTRL);
			ser.Sync(nameof(ser_CTRL_J), ref ser_CTRL_J);
			ser.Sync(nameof(ser_STAT_J), ref ser_STAT_J);
			ser.Sync(nameof(ser_Mode), ref ser_Mode);

			ser.Sync(nameof(key_CTRL), ref key_CTRL);

			ser.Sync(nameof(ser_div_cnt), ref ser_div_cnt);
			ser.Sync(nameof(ser_Mask), ref ser_Mask);
			ser.Sync(nameof(ser_Bit_Count), ref ser_Bit_Count);
			ser.Sync(nameof(ser_Bit_Total), ref ser_Bit_Total);

			ser.Sync(nameof(ser_Internal_Clock), ref ser_Internal_Clock);
			ser.Sync(nameof(ser_Start), ref ser_Start);

			ser.Sync(nameof(ser_Delay), ref ser_Delay);
			ser.Sync(nameof(key_Delay), ref key_Delay);
			ser.Sync(nameof(ser_OUT_State), ref ser_OUT_State);
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}
