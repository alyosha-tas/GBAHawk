using BizHawk.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
/*
	DMA Emulation
	NOTES:

	Need to implement DRQ?

	is video capture DMA delayed by a frame?

	Can any DMA parameters be changed by writing to DMA registers while an DMA is ongoing but intermittenly paused?

	Are there any extra cycles when one DMA is paused by another of higher priority (currently assumed yes)?

	What happens when src address control mode 3 is set?

	What happens when channel 0 start condition is set to 3?

	What happens when channel 3 has game pak DMA and repeat selected?

	Look at edge cases of start / stop writes

	Assumption: read / write cycle is atomic
*/

#pragma warning disable CS0675 // Bitwise-or operator used on a sign-extended operand

	public partial class GBAHawk_Debug
	{
		public readonly uint[] dma_SRC_Mask = { 0x7FFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF };
		public readonly uint[] dma_DST_Mask = { 0x7FFFFFF, 0x7FFFFFF, 0x7FFFFFF, 0xFFFFFFF };

		public readonly int[] dma_CNT_Mask_0 = { 0x4000, 0x4000, 0x4000, 0x10000 };

		public readonly ushort[] dma_CNT_Mask = { 0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF };

		public ulong[] dma_Run_En_Time = new ulong[4];

		public int[] dma_CNT_intl = new int[4];
		public int[] dma_ST_Time = new int[4];
		public int[] dma_IRQ_cd = new int[4];

		public int dma_Access_Cnt, dma_Access_Wait, dma_Chan_Exec;
		public int dma_ST_Time_Adjust;

		public uint[] dma_SRC = new uint[4];
		public uint[] dma_DST = new uint[4];
		public uint[] dma_SRC_intl = new uint[4];
		public uint[] dma_DST_intl = new uint[4];
		public uint[] dma_SRC_INC = new uint[4];
		public uint[] dma_DST_INC = new uint[4];
		public uint[] dma_Last_Bus_Value = new uint[4];
		public uint[] dma_ROM_Addr = new uint[4];

		public uint dma_TFR_Word;

		public ushort[] dma_CNT = new ushort[4];
		public ushort[] dma_CTRL = new ushort[4];

		public ushort dma_TFR_HWord;
		public ushort dma_Held_CPU_Instr;

		public bool[] dma_Go = new bool[4]; // Tell Condition checkers when the channel is on
		public bool[] dma_Start_VBL = new bool[4];
		public bool[] dma_Start_HBL = new bool[4];
		public bool[] dma_Start_Snd_Vid = new bool[4];
		public bool[] dma_Run = new bool[4]; // Actually run the DMA channel
		public bool[] dma_Access_32 = new bool [4];
		public bool[] dma_Use_ROM_Addr_SRC = new bool[4];
		public bool[] dma_Use_ROM_Addr_DST = new bool[4];
		public bool[] dma_ROM_Being_Used = new bool[4];
		public bool[] dma_External_Source = new bool[4];

		public bool dma_Seq_Access;
		public bool dma_Read_Cycle;
		public bool dma_Pausable;
		public bool dma_All_Off;
		public bool dma_Shutdown;
		public bool dma_Release_Bus;
		public bool dma_Delay;
		public bool dma_Video_DMA_Start;

		public byte dma_Read_Reg_8(uint addr)
		{
			byte ret = 0;

			switch (addr)
			{
				case 0xB8: ret = 0; break;
				case 0xB9: ret = 0; break;
				case 0xBA: ret = (byte)(dma_CTRL[0] & 0xFF); break;
				case 0xBB: ret = (byte)((dma_CTRL[0] & 0xFF00) >> 8); break;

				case 0xC4: ret = 0; break;
				case 0xC5: ret = 0; break;
				case 0xC6: ret = (byte)(dma_CTRL[1] & 0xFF); break;
				case 0xC7: ret = (byte)((dma_CTRL[1] & 0xFF00) >> 8); break;

				case 0xD0: ret = 0; break;
				case 0xD1: ret = 0; break;
				case 0xD2: ret = (byte)(dma_CTRL[2] & 0xFF); break;
				case 0xD3: ret = (byte)((dma_CTRL[2] & 0xFF00) >> 8); break;

				case 0xDC: ret = 0; break;
				case 0xDD: ret = 0; break;
				case 0xDE: ret = (byte)(dma_CTRL[3] & 0xFF); break;
				case 0xDF: ret = (byte)((dma_CTRL[3] & 0xFF00) >> 8); break;

				default: ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		public ushort dma_Read_Reg_16(uint addr)
		{
			ushort ret = 0;

			switch (addr)
			{
				case 0xB8: ret = 0; break;
				case 0xBA: ret = dma_CTRL[0]; break;

				case 0xC4: ret = 0; break;
				case 0xC6: ret = dma_CTRL[1]; break;

				case 0xD0: ret = 0; break;
				case 0xD2: ret = dma_CTRL[2]; break;

				case 0xDC: ret = 0; break;
				case 0xDE: ret = dma_CTRL[3]; break;

				default: ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		public uint dma_Read_Reg_32(uint addr)
		{
			uint ret = 0;

			switch (addr)
			{
				case 0xB8: ret = (uint)((dma_CTRL[0] << 16) | 0); break;

				case 0xC4: ret = (uint)((dma_CTRL[1] << 16) | 0); break;

				case 0xD0: ret = (uint)((dma_CTRL[2] << 16) | 0); break;

				case 0xDC: ret = (uint)((dma_CTRL[3] << 16) | 0); break;

				default: ret = cpu_Last_Bus_Value; break; // open bus
			}

			return ret;
		}

		public void dma_Write_Reg_8(uint addr, byte value)
		{
			switch (addr)
			{
				case 0xB0: dma_SRC[0] = (uint)((dma_SRC[0] & 0xFFFFFF00) | value); break;
				case 0xB1: dma_SRC[0] = (uint)((dma_SRC[0] & 0xFFFF00FF) | (value << 8)); break;
				case 0xB2: dma_SRC[0] = (uint)((dma_SRC[0] & 0xFF00FFFF) | (value << 16)); break;
				case 0xB3: dma_SRC[0] = (uint)((dma_SRC[0] & 0x00FFFFFF) | (value << 24)); break;
				case 0xB4: dma_DST[0] = (uint)((dma_DST[0] & 0xFFFFFF00) | value); break;
				case 0xB5: dma_DST[0] = (uint)((dma_DST[0] & 0xFFFF00FF) | (value << 8)); break;
				case 0xB6: dma_DST[0] = (uint)((dma_DST[0] & 0xFF00FFFF) | (value << 16)); break;
				case 0xB7: dma_DST[0] = (uint)((dma_DST[0] & 0x00FFFFFF) | (value << 24)); break;
				case 0xB8: dma_CNT[0] = (ushort)((dma_CNT[0] & 0xFF00) | value); break;
				case 0xB9: dma_CNT[0] = (ushort)((dma_CNT[0] & 0x00FF) | (value << 8)); break;
				case 0xBA: dma_Update_CTRL((ushort)((dma_CTRL[0] & 0xFF00) | value), 0); break;
				case 0xBB: dma_Update_CTRL((ushort)((dma_CTRL[0] & 0x00FF) | (value << 8)), 0); break;

				case 0xBC: dma_SRC[1] = (uint)((dma_SRC[1] & 0xFFFFFF00) | value); break;
				case 0xBD: dma_SRC[1] = (uint)((dma_SRC[1] & 0xFFFF00FF) | (value << 8)); break;
				case 0xBE: dma_SRC[1] = (uint)((dma_SRC[1] & 0xFF00FFFF) | (value << 16)); break;
				case 0xBF: dma_SRC[1] = (uint)((dma_SRC[1] & 0x00FFFFFF) | (value << 24)); break;
				case 0xC0: dma_DST[1] = (uint)((dma_DST[1] & 0xFFFFFF00) | value); break;
				case 0xC1: dma_DST[1] = (uint)((dma_DST[1] & 0xFFFF00FF) | (value << 8)); break;
				case 0xC2: dma_DST[1] = (uint)((dma_DST[1] & 0xFF00FFFF) | (value << 16)); break;
				case 0xC3: dma_DST[1] = (uint)((dma_DST[1] & 0x00FFFFFF) | (value << 24)); break;
				case 0xC4: dma_CNT[1] = (ushort)((dma_CNT[1] & 0xFF00) | value); break;
				case 0xC5: dma_CNT[1] = (ushort)((dma_CNT[1] & 0x00FF) | (value << 8)); break;
				case 0xC6: dma_Update_CTRL((ushort)((dma_CTRL[1] & 0xFF00) | value), 1); break;
				case 0xC7: dma_Update_CTRL((ushort)((dma_CTRL[1] & 0x00FF) | (value << 8)), 1); break;

				case 0xC8: dma_SRC[2] = (uint)((dma_SRC[2] & 0xFFFFFF00) | value); break;
				case 0xC9: dma_SRC[2] = (uint)((dma_SRC[2] & 0xFFFF00FF) | (value << 8)); break;
				case 0xCA: dma_SRC[2] = (uint)((dma_SRC[2] & 0xFF00FFFF) | (value << 16)); break;
				case 0xCB: dma_SRC[2] = (uint)((dma_SRC[2] & 0x00FFFFFF) | (value << 24)); break;
				case 0xCC: dma_DST[2] = (uint)((dma_DST[2] & 0xFFFFFF00) | value); break;
				case 0xCD: dma_DST[2] = (uint)((dma_DST[2] & 0xFFFF00FF) | (value << 8)); break;
				case 0xCE: dma_DST[2] = (uint)((dma_DST[2] & 0xFF00FFFF) | (value << 16)); break;
				case 0xCF: dma_DST[2] = (uint)((dma_DST[2] & 0x00FFFFFF) | (value << 24)); break;
				case 0xD0: dma_CNT[2] = (ushort)((dma_CNT[2] & 0xFF00) | value); break;
				case 0xD1: dma_CNT[2] = (ushort)((dma_CNT[2] & 0x00FF) | (value << 8)); break;
				case 0xD2: dma_Update_CTRL((ushort)((dma_CTRL[2] & 0xFF00) | value), 2); break;
				case 0xD3: dma_Update_CTRL((ushort)((dma_CTRL[2] & 0x00FF) | (value << 8)), 2); break;

				case 0xD4: dma_SRC[3] = (uint)((dma_SRC[3] & 0xFFFFFF00) | value); break;
				case 0xD5: dma_SRC[3] = (uint)((dma_SRC[3] & 0xFFFF00FF) | (value << 8)); break;
				case 0xD6: dma_SRC[3] = (uint)((dma_SRC[3] & 0xFF00FFFF) | (value << 16)); break;
				case 0xD7: dma_SRC[3] = (uint)((dma_SRC[3] & 0x00FFFFFF) | (value << 24)); break;
				case 0xD8: dma_DST[3] = (uint)((dma_DST[3] & 0xFFFFFF00) | value); break;
				case 0xD9: dma_DST[3] = (uint)((dma_DST[3] & 0xFFFF00FF) | (value << 8)); break;
				case 0xDA: dma_DST[3] = (uint)((dma_DST[3] & 0xFF00FFFF) | (value << 16)); break;
				case 0xDB: dma_DST[3] = (uint)((dma_DST[3] & 0x00FFFFFF) | (value << 24)); break;
				case 0xDC: dma_CNT[3] = (ushort)((dma_CNT[3] & 0xFF00) | value); break;
				case 0xDD: dma_CNT[3] = (ushort)((dma_CNT[3] & 0x00FF) | (value << 8)); break;
				case 0xDE: dma_Update_CTRL((ushort)((dma_CTRL[3] & 0xFF00) | value), 3); break;
				case 0xDF: dma_Update_CTRL((ushort)((dma_CTRL[3] & 0x00FF) | (value << 8)), 3); break;
			}
		}

		public void dma_Write_Reg_16(uint addr, ushort value)
		{
			switch (addr)
			{
				case 0xB0: dma_SRC[0] = (uint)((dma_SRC[0] & 0xFFFF0000) | value); break;
				case 0xB2: dma_SRC[0] = (uint)((dma_SRC[0] & 0x0000FFFF) | (value << 16)); break;
				case 0xB4: dma_DST[0] = (uint)((dma_DST[0] & 0xFFFF0000) | value); break;
				case 0xB6: dma_DST[0] = (uint)((dma_DST[0] & 0x0000FFFF) | (value << 16)); break;
				case 0xB8: dma_CNT[0] = value; break;
				case 0xBA: dma_Update_CTRL(value, 0); break;

				case 0xBC: dma_SRC[1] = (uint)((dma_SRC[1] & 0xFFFF0000) | value); break;
				case 0xBE: dma_SRC[1] = (uint)((dma_SRC[1] & 0x0000FFFF) | (value << 16)); break;
				case 0xC0: dma_DST[1] = (uint)((dma_DST[1] & 0xFFFF0000) | value); break;
				case 0xC2: dma_DST[1] = (uint)((dma_DST[1] & 0x0000FFFF) | (value << 16)); break;
				case 0xC4: dma_CNT[1] = value; break;
				case 0xC6: dma_Update_CTRL(value, 1); break;

				case 0xC8: dma_SRC[2] = (uint)((dma_SRC[2] & 0xFFFF0000) | value); break;
				case 0xCA: dma_SRC[2] = (uint)((dma_SRC[2] & 0x0000FFFF) | (value << 16)); break;
				case 0xCC: dma_DST[2] = (uint)((dma_DST[2] & 0xFFFF0000) | value); break;
				case 0xCE: dma_DST[2] = (uint)((dma_DST[2] & 0x0000FFFF) | (value << 16)); break;
				case 0xD0: dma_CNT[2] = value; break;
				case 0xD2: dma_Update_CTRL(value, 2); break;

				case 0xD4: dma_SRC[3] = (uint)((dma_SRC[3] & 0xFFFF0000) | value); break;
				case 0xD6: dma_SRC[3] = (uint)((dma_SRC[3] & 0x0000FFFF) | (value << 16)); break;
				case 0xD8: dma_DST[3] = (uint)((dma_DST[3] & 0xFFFF0000) | value); break;
				case 0xDA: dma_DST[3] = (uint)((dma_DST[3] & 0x0000FFFF) | (value << 16)); break;
				case 0xDC: dma_CNT[3] = value; break;
				case 0xDE: dma_Update_CTRL(value, 3); break;
			}
		}

		public void dma_Write_Reg_32(uint addr, uint value)
		{
			switch (addr)
			{
				case 0xB0: dma_SRC[0] = value; break;
				case 0xB4: dma_DST[0] = value; break;
				case 0xB8: dma_CNT[0] = (ushort)(value & 0xFFFF);
						   dma_Update_CTRL((ushort)((value >> 16) & 0xFFFF), 0); break;

				case 0xBC: dma_SRC[1] = value; break;
				case 0xC0: dma_DST[1] = value; break;
				case 0xC4: dma_CNT[1] = (ushort)(value & 0xFFFF);
						   dma_Update_CTRL((ushort)((value >> 16) & 0xFFFF), 1); break;

				case 0xC8: dma_SRC[2] = value; break;
				case 0xCC: dma_DST[2] = value; break;
				case 0xD0: dma_CNT[2] = (ushort)(value & 0xFFFF);
						   dma_Update_CTRL((ushort)((value >> 16) & 0xFFFF), 2); break;

				case 0xD4: dma_SRC[3] = value; break;
				case 0xD8: dma_DST[3] = value; break;
				case 0xDC: dma_CNT[3] = (ushort)(value & 0xFFFF);
						   dma_Update_CTRL((ushort)((value >> 16) & 0xFFFF), 3); break;
			}
		}

		public void dma_Update_CTRL(ushort value, int chan)
		{
			if(((dma_CTRL[chan] & 0x8000) == 0) && ((value & 0x8000) != 0))
			{
				dma_SRC_intl[chan] = (uint)(dma_SRC[chan] & dma_SRC_Mask[chan]);

				dma_DST_intl[chan] = (uint)(dma_DST[chan] & dma_DST_Mask[chan]);

				dma_CNT_intl[chan] = (dma_CNT[chan] & dma_CNT_Mask[chan]);

				if (dma_CNT_intl[chan] == 0) { dma_CNT_intl[chan] = dma_CNT_Mask_0[chan]; }

				dma_Access_32[chan] = (value & 0x400) == 0x400;

				if (dma_Access_32[chan])
				{
					if ((value & 0x60) == 0) { dma_DST_INC[chan] = 4; }
					else if ((value & 0x60) == 0x20) { dma_DST_INC[chan] = 0xFFFFFFFC; }
					else if ((value & 0x60) == 0x40) { dma_DST_INC[chan] = 0; }
					else { dma_DST_INC[chan] = 4; }

					if ((value & 0x180) == 0) { dma_SRC_INC[chan] = 4; }
					else if ((value & 0x180) == 0x80) { dma_SRC_INC[chan] = 0xFFFFFFFC; }
					else if ((value & 0x180) == 0x100) { dma_SRC_INC[chan] = 0; }
					else { dma_SRC_INC[chan] = 4; } // Prohibited?
				}
				else
				{
					if ((value & 0x60) == 0) { dma_DST_INC[chan] = 2; }
					else if ((value & 0x60) == 0x20) { dma_DST_INC[chan] = 0xFFFFFFFE; }
					else if ((value & 0x60) == 0x40) { dma_DST_INC[chan] = 0; }
					else { dma_DST_INC[chan] = 2; }

					if ((value & 0x180) == 0) { dma_SRC_INC[chan] = 2; }
					else if ((value & 0x180) == 0x80) { dma_SRC_INC[chan] = 0xFFFFFFFE; }
					else if ((value & 0x180) == 0x100) { dma_SRC_INC[chan] = 0; }
					else { dma_SRC_INC[chan] = 2; } // Prohibited?
				}

				dma_Start_VBL[chan] = dma_Start_HBL[chan] = dma_Run[chan] = false;
				dma_Run_En_Time[chan] = 0xFFFFFFFFFFFFFFFF;

				dma_Start_Snd_Vid[chan] = false;
				if ((value & 0x3000) == 0x0000) { dma_Run_En_Time[chan] = CycleCount + 3; }
				else if ((value & 0x3000) == 0x1000) { dma_Start_VBL[chan] = true; }
				else if ((value & 0x3000) == 0x2000) { dma_Start_HBL[chan] = true; }
				else 
				{ 
					if (chan == 0)
					{
						// Prohibited? What happens?
					}
					else
					{
						dma_Start_Snd_Vid[chan] = true;

						if ((chan == 1) || (chan == 2))
						{
							// ignore word count
							dma_CNT_intl[chan] = 4;

							// for sound FIFO DMA, always DMA to same destination
							dma_DST_INC[chan] = 0;

							// always word size accesses
							dma_Access_32[chan] = true;

							if ((value & 0x180) == 0) { dma_SRC_INC[chan] = 4; }
							else if ((value & 0x180) == 0x80) { dma_SRC_INC[chan] = 0xFFFFFFFC; }
							else if ((value & 0x180) == 0x100) { dma_SRC_INC[chan] = 0; }
							else { dma_SRC_INC[chan] = 4; } // Prohibited? 
						}

						if (chan == 3)
						{
							dma_Video_DMA_Start = false;
						}
					}
				}

				//Console.WriteLine(chan + " " + value + " " + ((dma_CTRL[chan] & 0x200) == 0x200));

				dma_Go[chan] = true;

				dma_All_Off = false;

				dma_External_Source[chan] = false;
			}

			if ((value & 0x8000) == 0)
			{			
				// if the channel isnt currently running, turn it off
				dma_Run[chan] = false;
				dma_Go[chan] = false;
				dma_Run_En_Time[chan] = 0xFFFFFFFFFFFFFFFF;

				dma_All_Off = true;

				for (int i = 0; i < 4; i++) { dma_All_Off &= !dma_Go[i]; }
			}

			//if (!dma_All_Off) { Console.WriteLine(dma_Go[0] + " " + dma_Go[1] + " " + dma_Go[2] + " " + dma_Go[3]); }

			if (chan == 3)
			{
				dma_CTRL[chan] = (ushort)(value & 0xFFE0);
			}
			else
			{
				dma_CTRL[chan] = (ushort)(value & 0xF7E0);
			}		
		}

		public void dma_Tick()
		{		
			// if no channels are on, skip
			if (dma_All_Off) { return; }

			if (dma_Shutdown)
			{
				if (dma_Release_Bus)
				{
					cpu_Is_Paused = false;

					dma_All_Off = true;

					for (int i = 0; i < 4; i++) { dma_All_Off &= !dma_Go[i]; }

					if (dma_Delay) { dma_All_Off = false; }

					dma_Shutdown = false;
				}
				else
				{
					dma_Release_Bus = true;
				}
			}

			if (dma_Delay)
			{
				for (int i = 0; i < 4; i++)
				{
					if (dma_IRQ_cd[i] > 0)
					{
						dma_IRQ_cd[i]--;
						if (dma_IRQ_cd[i] == 2)
						{
							INT_Flags |= (ushort)(0x1 << (8 + i));
							if ((INT_EN & (0x1 << (8 + i))) == (0x1 << (8 + i))) { cpu_Trigger_Unhalt = true; }

						}
						else if (dma_IRQ_cd[i] == 0)
						{

							// trigger IRQ (Bits 8 through 11)
							if ((INT_EN & (0x1 << (8 + i))) == (0x1 << (8 + i)))
							{
								if (INT_Master_On) { cpu_IRQ_Input = true; }
							}
						}
					}			
				}

				dma_Delay = false;

				for (int i = 0; i < 4; i++)
				{
					if (dma_IRQ_cd[i] != 0) { dma_Delay = true; }
				}

				dma_All_Off = true;

				for (int i = 0; i < 4; i++) { dma_All_Off &= !dma_Go[i]; }

				if (dma_Delay) { dma_All_Off = false; }

				if (dma_Shutdown) { dma_All_Off = false; }
			}

			if (!dma_Pausable)
			{
				// if a channel is in the middle of read/write cycle, continue with the cycle
				if (dma_Read_Cycle)
				{
					if (dma_Access_Cnt == 0)
					{
						if (dma_Use_ROM_Addr_SRC[dma_Chan_Exec])
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								dma_Access_Wait = Wait_State_Access_32(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								dma_Access_Wait = Wait_State_Access_16(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}

							// for ROM to ROM transfers, every access after the first is sequential (even though the write address is unrelated)
							if (!dma_Seq_Access && dma_Use_ROM_Addr_DST[dma_Chan_Exec])
							{
								dma_Seq_Access = true;
							}
						}
						else
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								dma_Access_Wait = Wait_State_Access_32(dma_SRC_intl[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								dma_Access_Wait = Wait_State_Access_16(dma_SRC_intl[dma_Chan_Exec], dma_Seq_Access);
							}
						}
					}

					dma_Access_Cnt++;

					if (dma_Access_Cnt == dma_Access_Wait)
					{
						if (dma_Access_32[dma_Chan_Exec])
						{
							// This updates the bus value.
							if (dma_Use_ROM_Addr_SRC[dma_Chan_Exec])
							{
								Read_Memory_32_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_Chan_Exec);

								dma_ROM_Addr[dma_Chan_Exec] += 4;
							}
							else
							{
								Read_Memory_32_DMA(dma_SRC_intl[dma_Chan_Exec], dma_Chan_Exec);
							}
							// The transfer value is now whatever the bus value is.
							dma_TFR_Word = dma_Last_Bus_Value[dma_Chan_Exec];
							// also update the cpu open bus state
							cpu_Last_Bus_Value = dma_TFR_Word;
						}
						else
						{
							// This updates the bus value.
							if (dma_Use_ROM_Addr_SRC[dma_Chan_Exec])
							{
								Read_Memory_16_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_Chan_Exec);

								dma_ROM_Addr[dma_Chan_Exec] += 2;
							}
							else
							{
								Read_Memory_16_DMA(dma_SRC_intl[dma_Chan_Exec], dma_Chan_Exec);
							}
							// The transfer value might be a rotated version of the bus value, depending on the destination address.
							// In normal cases this doesn't matter since both the upper and lower half words of the bus are updated with the same value.
							// But in open bus cases they could be different.
							if ((dma_DST_intl[dma_Chan_Exec] & 2) == 0)
							{
								dma_TFR_HWord = (ushort)(dma_Last_Bus_Value[dma_Chan_Exec] & 0xFFFF);
							}
							else
							{
								// does it also update the bus?
								dma_TFR_HWord = (ushort)((dma_Last_Bus_Value[dma_Chan_Exec] >> 16) & 0xFFFF);
							}
							// also update the cpu open bus state
							cpu_Last_Bus_Value = dma_TFR_HWord;
						}

						dma_SRC_intl[dma_Chan_Exec] += dma_SRC_INC[dma_Chan_Exec];
						dma_SRC_intl[dma_Chan_Exec] &= dma_SRC_Mask[dma_Chan_Exec];

						if (((dma_SRC_intl[dma_Chan_Exec] & 0x08000000) != 0) && ((dma_SRC_intl[dma_Chan_Exec] & 0x0E000000) != 0x0E000000))
						{
							// If ROM is not already being accessed
							if (!dma_ROM_Being_Used[dma_Chan_Exec])
							{
								dma_ROM_Addr[dma_Chan_Exec] = dma_SRC_intl[dma_Chan_Exec];
							}

							dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = true;

							dma_ROM_Being_Used[dma_Chan_Exec] = true;
						}
						else
						{
							dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = false;
						}

						dma_Read_Cycle = !dma_Read_Cycle;
						dma_Access_Cnt = 0;
					}
				}
				else
				{
					if (dma_Access_Cnt == 0)
					{
						if (dma_Use_ROM_Addr_DST[dma_Chan_Exec])
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								dma_Access_Wait = Wait_State_Access_32(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								dma_Access_Wait = Wait_State_Access_16(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}
						}
						else
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								dma_Access_Wait = Wait_State_Access_32(dma_DST_intl[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								dma_Access_Wait = Wait_State_Access_16(dma_DST_intl[dma_Chan_Exec], dma_Seq_Access);
							}
						}
					}

					dma_Access_Cnt++;

					if (dma_Access_Cnt == dma_Access_Wait)
					{
						if (dma_Use_ROM_Addr_DST[dma_Chan_Exec])
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								Write_Memory_32_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_TFR_Word, dma_Chan_Exec);
							}
							else
							{
								Write_Memory_16_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_TFR_HWord, dma_Chan_Exec);
							}

							dma_ROM_Addr[dma_Chan_Exec] += (uint)(dma_Access_32[dma_Chan_Exec] ? 4 : 2);
						}
						else
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								Write_Memory_32_DMA(dma_DST_intl[dma_Chan_Exec], dma_TFR_Word, dma_Chan_Exec);
							}
							else
							{
								Write_Memory_16_DMA(dma_DST_intl[dma_Chan_Exec], dma_TFR_HWord, dma_Chan_Exec);
							}
						}

						dma_DST_intl[dma_Chan_Exec] += dma_DST_INC[dma_Chan_Exec];
						dma_DST_intl[dma_Chan_Exec] &= dma_DST_Mask[dma_Chan_Exec];

						if (((dma_DST_intl[dma_Chan_Exec] & 0x08000000) != 0) && ((dma_DST_intl[dma_Chan_Exec] & 0x0E000000) != 0x0E000000))
						{
							// If ROM is not already being accessed
							if (!dma_ROM_Being_Used[dma_Chan_Exec])
							{
								dma_ROM_Addr[dma_Chan_Exec] = dma_DST_intl[dma_Chan_Exec];
							}

							dma_Use_ROM_Addr_DST[dma_Chan_Exec] = true;

							dma_ROM_Being_Used[dma_Chan_Exec] = true;
						}
						else
						{
							dma_Use_ROM_Addr_DST[dma_Chan_Exec] = false;
						}

						dma_Read_Cycle = !dma_Read_Cycle;
						dma_Access_Cnt = 0;
						dma_Seq_Access = true;

						// end of a write cycle allows a possibility for a pause
						dma_Pausable = true;
						dma_ST_Time_Adjust = 1;

						// check if the DMA is complete
						dma_CNT_intl[dma_Chan_Exec] -= 1;

						if (dma_CNT_intl[dma_Chan_Exec] == 0)
						{
							// it seems as though DMA's triggered by external sources take an extra cycle to shut down
							if (dma_External_Source[dma_Chan_Exec])
							{
								dma_Release_Bus = false;
							}
							else
							{
								dma_Release_Bus = true;
							}

							//Console.WriteLine("complete " + dma_Chan_Exec);
							// generate an IRQ if needed
							if ((dma_CTRL[dma_Chan_Exec] & 0x4000) == 0x4000)
							{
								if (dma_IRQ_cd[dma_Chan_Exec] == 0)
								{
									dma_IRQ_cd[dma_Chan_Exec] = 3;
									dma_Delay = true;
								}					
							}

							// Repeat if necessary, or turn the channel off
							if ((dma_CTRL[dma_Chan_Exec] & 0x200) == 0x200)
							{
								dma_CNT_intl[dma_Chan_Exec] = dma_CNT[dma_Chan_Exec];

								if ((dma_Chan_Exec == 1) || (dma_Chan_Exec == 2))
								{
									if (dma_Start_Snd_Vid[dma_Chan_Exec] == true)
									{
										// ignore word count
										dma_CNT_intl[dma_Chan_Exec] = 4;
									}
								}

								// reload destination register
								if ((dma_CTRL[dma_Chan_Exec] & 0x60) == 0x60)
								{
									dma_DST_intl[dma_Chan_Exec] = (uint)(dma_DST[dma_Chan_Exec] & dma_DST_Mask[dma_Chan_Exec]);
								}

								// repeat forever if DMA immediately is set?
								if ((dma_CTRL[dma_Chan_Exec] & 0x3000) == 0x0000)
								{
									dma_Run[dma_Chan_Exec] = true;
								}
								else
								{
									dma_Run[dma_Chan_Exec] = false;
								}

								// for channel 3 running video capture mode, turn off after scanline 161
								if ((dma_Chan_Exec == 3) && dma_Start_Snd_Vid[3] && (ppu_LY == 161))
								{
									dma_CTRL[dma_Chan_Exec] &= 0x7FFF;

									dma_Run[dma_Chan_Exec] = false;
									dma_Go[dma_Chan_Exec] = false;
									dma_Run_En_Time[dma_Chan_Exec] = 0xFFFFFFFFFFFFFFFF;
								}
							}
							else
							{
								dma_CTRL[dma_Chan_Exec] &= 0x7FFF;

								dma_Run[dma_Chan_Exec] = false;
								dma_Go[dma_Chan_Exec] = false;
								dma_Run_En_Time[dma_Chan_Exec] = 0xFFFFFFFFFFFFFFFF;
							}

							// In any case, we start a new DMA
							dma_Chan_Exec = 4;

							dma_Shutdown = true;
						}
						else if (!dma_Run[dma_Chan_Exec])
						{
							// DMA channel was turned off by the DMA itself
							dma_Chan_Exec = 4;

							dma_Shutdown = true;
							dma_Release_Bus = true;

							dma_All_Off = false;
						}
					}
				}
			}

			if (dma_Pausable)
			{
				// if no channel is currently running, or we have completed a memory cycle, look for a new one to run
				// zero is highest priority channel
				for (int i = 0; i < 4; i++)
				{
					if (CycleCount >= dma_Run_En_Time[i])
					{
						dma_Run[i] = true;
						dma_Run_En_Time[i] = 0xFFFFFFFFFFFFFFFF;
					}

					if (dma_Run[i])
					{
						// if the current channel is less then the previous (i.e. not just unpaused) reset execution timing
						if (i < dma_Chan_Exec)
						{
							if ((dma_SRC_intl[i] >= 0xE000000) && (dma_DST_intl[i] >= 0xE000000))
							{
								dma_ST_Time[i] = 3;
							}
							else
							{
								dma_ST_Time[i] = 1;
							}

							// we don't need to add extra start cycles if we are resuming a DMA paused by a higher priority one
							// but still have at least one cycle wait, otherwise we would be counting twice, since the current cycle just ran
							if (dma_ST_Time_Adjust != 0) { dma_ST_Time[i] = 1; }

							dma_Chan_Exec = i;

							// Is this correct for all cases?
							dma_Use_ROM_Addr_DST[dma_Chan_Exec] = dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = false;

							dma_ROM_Being_Used[dma_Chan_Exec] = false;

							if (((dma_DST_intl[i] & 0x08000000) != 0) && ((dma_DST_intl[i] & 0x0E000000) != 0x0E000000))
							{
								// special case for ROM area
								dma_ROM_Addr[dma_Chan_Exec] = dma_DST_intl[i];
								dma_Use_ROM_Addr_DST[dma_Chan_Exec] = true;
								dma_ROM_Being_Used[dma_Chan_Exec] = true;
							}

							if (((dma_SRC_intl[i] & 0x08000000) != 0) && ((dma_SRC_intl[i] & 0x0E000000) != 0x0E000000))
							{
								// special case for ROM area
								dma_ROM_Addr[dma_Chan_Exec] = dma_SRC_intl[i];
								dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = true;
								dma_ROM_Being_Used[dma_Chan_Exec] = true;
							}

							//Console.WriteLine("DMA " + i + " running at " + CycleCount + " from " + dma_SRC_intl[i] + " to " + dma_DST_intl[i]);
							//Console.WriteLine("len " + dma_CNT_intl[i] + " inc s " + dma_SRC_INC[i] + " inc d " + dma_DST_INC[i] + " rep " + ((dma_CTRL[dma_Chan_Exec] & 0x200) == 0x200));
							//Console.WriteLine("St time: " + dma_ST_Time[i] + " SRC: " + dma_SRC[i] + " DST: " + dma_DST[i]);
						}
					}
				}

				dma_ST_Time_Adjust = 0;

				if (dma_Chan_Exec < 4)
				{
					dma_Shutdown = false;
					
					if (dma_ST_Time[dma_Chan_Exec] > 0)
					{
						dma_ST_Time[dma_Chan_Exec] -= 1;

						if (!cpu_Is_Paused)
						{
							if (cpu_Instr_Type == cpu_Pause_For_DMA)
							{
								// we just ended a DMA and immediately want to start another one
								// or paused a DMA to do a higher priority one
								cpu_Is_Paused = true;

								dma_Seq_Access = false;
								dma_Read_Cycle = true;

								dma_Access_Cnt = 0;
								dma_Access_Wait = 0;
							}
							else
							{
								// CPU can be paused on the edge of a memory access, if bus is not locked
								if ((cpu_Fetch_Cnt == 0) && !cpu_Swap_Lock)
								{
									// hold the current cpu instruction
									dma_Held_CPU_Instr = cpu_Instr_Type;
									cpu_Instr_Type = cpu_Pause_For_DMA;
									cpu_Is_Paused = true;

									dma_Seq_Access = false;
									dma_Read_Cycle = true;

									dma_Access_Cnt = 0;
									dma_Access_Wait = 0;
								}
								else
								{
									dma_ST_Time[dma_Chan_Exec] += 1;
								}
							}
						}
					}

					if (dma_ST_Time[dma_Chan_Exec] == 0)
					{
						dma_Pausable = false;
					}
				}		
			}
		}

		public void dma_Reset()
		{
			for (int i = 0; i < 4; i++)
			{
				dma_CNT_intl[i] = 0;

				dma_Run_En_Time[i] = 0xFFFFFFFFFFFFFFFF;

				dma_ST_Time[i] = 0;
				dma_IRQ_cd[i] = 0;
				dma_SRC[i] = 0;
				dma_DST[i] = 0;
				dma_SRC_intl[i] = 0;
				dma_DST_intl[i] = 0;
				dma_SRC_INC[i] = 0;
				dma_DST_INC[i] = 0;
				dma_Last_Bus_Value[i] = 0;

				dma_CNT[i] = 0;
				dma_CTRL[i] = 0;

				dma_ROM_Addr[i] = 0;

				dma_Go[i] = false;
				dma_Start_VBL[i] = false;
				dma_Start_HBL[i] = false;
				dma_Start_Snd_Vid[i] = false;
				dma_Run[i] = false;
				dma_Access_32[i] = false;
				dma_Use_ROM_Addr_SRC[i] = false;		
				dma_Use_ROM_Addr_DST[i] = false;
				dma_ROM_Being_Used[i] = false;
				dma_External_Source[i] = false;
			}

			dma_Access_Cnt = dma_Access_Wait = 0;
			
			dma_Chan_Exec = 4;

			dma_ST_Time_Adjust = 0;
			dma_TFR_Word = 0;
			dma_TFR_HWord = dma_Held_CPU_Instr = 0;

			dma_Seq_Access = false;
			dma_Read_Cycle = true;
			dma_Pausable = true;
			dma_All_Off = true;
			dma_Shutdown =  dma_Release_Bus = false;
			dma_Delay = false;
			dma_Video_DMA_Start = false;
		}

		public void dma_SyncState(Serializer ser)
		{
			ser.Sync(nameof(dma_Run_En_Time), ref dma_Run_En_Time, false);
			ser.Sync(nameof(dma_CNT_intl), ref dma_CNT_intl, false);
			ser.Sync(nameof(dma_ST_Time), ref dma_ST_Time, false);
			ser.Sync(nameof(dma_IRQ_cd), ref dma_IRQ_cd, false);

			ser.Sync(nameof(dma_Access_Cnt), ref dma_Access_Cnt);
			ser.Sync(nameof(dma_Access_Wait), ref dma_Access_Wait);
			ser.Sync(nameof(dma_Chan_Exec), ref dma_Chan_Exec);
			ser.Sync(nameof(dma_ST_Time_Adjust), ref dma_ST_Time_Adjust);
			
			ser.Sync(nameof(dma_SRC), ref dma_SRC, false);
			ser.Sync(nameof(dma_DST), ref dma_DST, false);
			ser.Sync(nameof(dma_SRC_intl), ref dma_SRC_intl, false);
			ser.Sync(nameof(dma_DST_intl), ref dma_DST_intl, false);
			ser.Sync(nameof(dma_SRC_INC), ref dma_SRC_INC, false);
			ser.Sync(nameof(dma_DST_INC), ref dma_DST_INC, false);
			ser.Sync(nameof(dma_Last_Bus_Value), ref dma_Last_Bus_Value, false);
			ser.Sync(nameof(dma_ROM_Addr), ref dma_ROM_Addr, false);

			ser.Sync(nameof(dma_TFR_Word), ref dma_TFR_Word);

			ser.Sync(nameof(dma_CNT), ref dma_CNT, false);
			ser.Sync(nameof(dma_CTRL), ref dma_CTRL, false);

			ser.Sync(nameof(dma_TFR_HWord), ref dma_TFR_HWord);
			ser.Sync(nameof(dma_Held_CPU_Instr), ref dma_Held_CPU_Instr);

			ser.Sync(nameof(dma_Go), ref dma_Go, false);
			ser.Sync(nameof(dma_Start_VBL), ref dma_Start_VBL, false);
			ser.Sync(nameof(dma_Start_HBL), ref dma_Start_HBL, false);
			ser.Sync(nameof(dma_Start_Snd_Vid), ref dma_Start_Snd_Vid, false);
			ser.Sync(nameof(dma_Run), ref dma_Run, false);
			ser.Sync(nameof(dma_Access_32), ref dma_Access_32, false);

			ser.Sync(nameof(dma_Use_ROM_Addr_SRC), ref dma_Use_ROM_Addr_SRC, false);
			ser.Sync(nameof(dma_Use_ROM_Addr_DST), ref dma_Use_ROM_Addr_DST, false);
			ser.Sync(nameof(dma_ROM_Being_Used), ref dma_ROM_Being_Used, false);
			ser.Sync(nameof(dma_External_Source), ref dma_External_Source, false);

			ser.Sync(nameof(dma_Seq_Access), ref dma_Seq_Access);
			ser.Sync(nameof(dma_Read_Cycle), ref dma_Read_Cycle);
			ser.Sync(nameof(dma_Pausable), ref dma_Pausable);
			ser.Sync(nameof(dma_All_Off), ref dma_All_Off);
			ser.Sync(nameof(dma_Shutdown), ref dma_Shutdown);
			ser.Sync(nameof(dma_Release_Bus), ref dma_Release_Bus);
			ser.Sync(nameof(dma_Delay), ref dma_Delay);
			ser.Sync(nameof(dma_Video_DMA_Start), ref dma_Video_DMA_Start);
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}
