using System;
using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
/*
	ROM Prefetcher Emulation
	NOTES:
	
	Current theory of operation:

	The prefetch unit will use the cartridge bus as long as no other components are using it. If the cpu needs an opcode the prefetcher 
	is in the process of fetching, it is af if the prefetcher takes over the access and continues as normal.

	If the cpu needs data from ROM, the current prefetch is abandoned, and ROM access is reset to non-sequential for the prefetcher.
	However, if a ROM data access is needed on the last cycle of a prefetch, the prefetch access is completed first, but the result is NOT
	added to the buffer, it is discarded.
	
	The prefetch unit cannot begin a read on an idle cycle after a data read from ROM.

	If the prefetch unit has 2 16 bit accesses in the buffer for a ARM mode opcode fetch, it will return both on the same cycle.

	When performaing multi load-stores at the transition from VRAM to ROM, it is as if the prefetcher is completely reset on the boundary access.

	When the prefetcher is fetching a 32 bit value and already fetched the first 16 bit portion but is interrupted by a ROM access,
	it will discard the 16 bit portion already fetched.

	What happens on SRAM accesses?

	what happens when changing between ARM and Thumb modes?
*/

#pragma warning disable CS0675 // Bitwise-or operator used on a sign-extended operand

	public partial class GBAHawk_Debug
	{
		public uint pre_Read_Addr, pre_Check_Addr;
		public uint pre_Buffer_Cnt;

		public int pre_Fetch_Cnt, pre_Fetch_Wait, pre_Fetch_Cnt_Inc;

		public bool pre_Cycle_Glitch;

		public bool pre_Enable, pre_Seq_Access;

		public void pre_Reg_Write(ushort value)
		{
			if (!pre_Enable && ((value & 0x4000) == 0x4000))
			{
				// set read address to current cpu address
				pre_Check_Addr = pre_Read_Addr = cpu_Regs[15];
				pre_Buffer_Cnt = 0;
			}
			
			pre_Enable = (value & 0x4000) == 0x4000;

			if (!pre_Enable)
			{
				if (pre_Fetch_Cnt == 0)
				{
					pre_Seq_Access = false;
					pre_Read_Addr = 0;
				}
			}
		}

		public void pre_Tick()
		{
			pre_Cycle_Glitch = false;
			
			// if enabled, try to read from ROM if buffer is not full
			// if not enabled, finish current fetch
			if ((pre_Enable && (pre_Buffer_Cnt < 8)) || (pre_Fetch_Cnt != 0))
			{
				if (pre_Fetch_Cnt == 0)
				{
					if ((cpu_Instr_Type >= 42) && !pre_Seq_Access) { return; } // cannot start an access on the internal cycles of an instruction
					
					pre_Fetch_Wait = 1;

					if (pre_Read_Addr > 0x0C000000)
					{
						if ((pre_Read_Addr & 0x1FFFF) == 0) { pre_Fetch_Wait += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { pre_Fetch_Wait += pre_Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}
					else if (pre_Read_Addr > 0x0A000000)
					{
						if ((pre_Read_Addr & 0x1FFFF) == 0) { pre_Fetch_Wait += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
						else { pre_Fetch_Wait += pre_Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
					}
					else
					{
						if ((pre_Read_Addr & 0x1FFFF) == 0) { pre_Fetch_Wait += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
						else { pre_Fetch_Wait += pre_Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0
					}
				}

				// if Inc is zero, ROM is being accessed by another component, otherwise it is 1
				pre_Fetch_Cnt += pre_Fetch_Cnt_Inc;

				if (pre_Fetch_Cnt == pre_Fetch_Wait)
				{
					pre_Buffer_Cnt += 1;
					pre_Fetch_Cnt = 0;
					pre_Read_Addr +=2;

					pre_Cycle_Glitch = true;
				}
			}
		}

		public void pre_Reset()
		{
			pre_Read_Addr = pre_Check_Addr = 0;
			pre_Buffer_Cnt = 0;

			pre_Fetch_Cnt = pre_Fetch_Wait = 0;
			pre_Fetch_Cnt_Inc = 1;

			pre_Cycle_Glitch = false;

			pre_Enable = pre_Seq_Access = false;
		}

		public void pre_SyncState(Serializer ser)
		{
			ser.Sync(nameof(pre_Read_Addr), ref pre_Read_Addr);
			ser.Sync(nameof(pre_Check_Addr), ref pre_Check_Addr);
			ser.Sync(nameof(pre_Buffer_Cnt), ref pre_Buffer_Cnt);

			ser.Sync(nameof(pre_Fetch_Cnt), ref pre_Fetch_Cnt);
			ser.Sync(nameof(pre_Fetch_Wait), ref pre_Fetch_Wait);
			ser.Sync(nameof(pre_Fetch_Cnt_Inc), ref pre_Fetch_Cnt_Inc);
			ser.Sync(nameof(pre_Cycle_Glitch), ref pre_Cycle_Glitch);

			ser.Sync(nameof(pre_Enable), ref pre_Enable);
			ser.Sync(nameof(pre_Seq_Access), ref pre_Seq_Access);	
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}
