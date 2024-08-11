﻿using System;

/*
	$10000000-$FFFFFFFF    Unmapped
	$0E010000-$0FFFFFFF    Unmapped
	$0E000000-$0E00FFFF    SRAM
	$0C000000-$0DFFFFFF    ROM - Wait State 2
	$0A000000-$0BFFFFFF    ROM - Wait State 1
	$08000000-$09FFFFFF    ROM - Wait State 0
	$07000400-$07FFFFFF    Unmapped
	$07000000-$070003FF    OAM
	$06018000-$06FFFFFF    Unmapped
	$06000000-$06017FFF    VRAM
	$05000400-$05FFFFFF    Unmapped
	$05000000-$050003FF    Palette RAM
	$04XX0800-$04XX0800    Mem Control
	$04000000-$040007FF    I/O Regs
	$03008000-$03FFFFFF    Unmapped
	$03000000-$03007FFF    IWRAM
	$02040000-$02FFFFFF    Unmapped
	$02000000-$0203FFFF    WRAM
	$00004000-$01FFFFFF    Unmapped
	$00000000-$00003FFF    BIOS
*/

// NOTE: For 32 bit accesses, PALRAM and VRAM take 2 accesses, but could be interrupted by rendering. So the 32 bit wait state processors need to know about
// the destination and value in this case.
// For the CPU, the value will be in cpu_Regs[cpu_Temp_Reg_Ptr], for DMA it is in dma_TFR_Word. 
// For the CPU, whether it is a write or not is in cpu_LS_Is_Load, for DMA it is in dma_Read_Cycle

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		public int Wait_State_Access_8(uint addr, bool Seq_Access)
		{
			int wait_ret = 1;
			
			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
					pre_Cycle_Glitch_2 = false;
					pre_Boundary_Reached = false;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;

								ppu_VRAM_High_In_Use = true;
							}
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;

									ppu_VRAM_In_Use = true;
								}
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;

									ppu_VRAM_High_In_Use = true;
								}
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;

							ppu_VRAM_In_Use = true;
						}
					}	
				}
				else
				{
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;

						ppu_PALRAM_In_Use = true;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += WRAM_Waits; //WRAM
			}

			return wait_ret;
		}

		public int Wait_State_Access_16(uint addr, bool Seq_Access)
		{
			int wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
					pre_Cycle_Glitch_2 = false;
					pre_Boundary_Reached = false;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;

								ppu_VRAM_High_In_Use = true;
							}
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;

									ppu_VRAM_In_Use = true;
								}
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;

									ppu_VRAM_High_In_Use = true;
								}
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;

							ppu_VRAM_In_Use = true;
						}
					}
				}
				else
				{
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;

						ppu_PALRAM_In_Use = true;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += WRAM_Waits; //WRAM
			}

			return wait_ret;
		}

		public int Wait_State_Access_32(uint addr, bool Seq_Access)
		{
			int wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
					pre_Cycle_Glitch_2 = false;
					pre_Boundary_Reached = false;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{			
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;
							}

							// set to true since we also need to check the next cycle
							ppu_VRAM_High_In_Use = true;
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_In_Use = true;
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_High_In_Use = true;
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;
						}

						// set to true since we also need to check the next cycle
						ppu_VRAM_In_Use = true;
					}

					if (!cpu_LS_Is_Load)
					{
						VRAM_32_Check = true;
						VRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						VRAM_32W_Addr = addr;
						VRAM_32W_Value = (ushort)cpu_Regs[cpu_Temp_Reg_Ptr];
					}
				}
				else
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_PALRAM_In_Use = true;

					if (!cpu_LS_Is_Load)
					{
						PALRAM_32_Check = true;
						PALRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;		

						PALRAM_32W_Addr = addr;
						PALRAM_32W_Value = (ushort)cpu_Regs[cpu_Temp_Reg_Ptr];
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
			}

			return wait_ret;
		}

		public int Wait_State_Access_32_DMA(uint addr, bool Seq_Access)
		{
			int wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
					pre_Cycle_Glitch_2 = false;
					pre_Boundary_Reached = false;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;
							}

							// set to true since we also need to check the next cycle
							ppu_VRAM_High_In_Use = true;
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_In_Use = true;
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_High_In_Use = true;
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;
						}

						// set to true since we also need to check the next cycle
						ppu_VRAM_In_Use = true;
					}

					if (!dma_Read_Cycle[dma_Chan_Exec])
					{ 
						VRAM_32_Check = true;
						VRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						VRAM_32W_Addr = addr;
						VRAM_32W_Value = (ushort)dma_TFR_Word;
					}
				}
				else
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_PALRAM_In_Use = true;

					if (!dma_Read_Cycle[dma_Chan_Exec])
					{ 
						PALRAM_32_Check = true;
						PALRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						PALRAM_32W_Addr = addr;
						PALRAM_32W_Value = (ushort)dma_TFR_Word;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
			}

			return wait_ret;
		}

		public int Wait_State_Access_16_Instr(uint addr, bool Seq_Access)
		{
			int wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
				{
					if (addr == pre_Check_Addr)
					{
						if ((pre_Check_Addr != pre_Read_Addr) && (pre_Buffer_Cnt > 0))
						{
							// we have this address, can immediately read it
							pre_Check_Addr += 2;
							pre_Buffer_Cnt -= 1;

							if (pre_Buffer_Cnt == 0)
							{
								if (pre_Buffer_Was_Full) { pre_Check_Addr = 0; pre_Force_Non_Seq = true; }

								if (!pre_Enable) { pre_Check_Addr = 0; }
							}
						}
						else
						{
							// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
							if (!Seq_Access && (pre_Fetch_Cnt == 1) && !pre_Following)
							{
								//Console.WriteLine("16 " + TotalExecutedCycles + " " + cpu_Instr_Type);
								// this happens in a branch to the current prefetcher fetch address
								if (addr < 0x0A000000)
								{
									wait_ret += ROM_Waits_0_N;
								}
								else if (addr < 0x0C000000)
								{
									wait_ret += ROM_Waits_1_N;
								}
								else
								{
									wait_ret += ROM_Waits_2_N;
								}
							}
							else
							{
								// plus 1 since the prefetcher already used this cycle, so don't double count
								wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;
							}

							pre_Read_Addr += 2;
							pre_Check_Addr += 2;
							pre_Fetch_Cnt = 0;

							if (!pre_Enable) { pre_Check_Addr = 0; pre_Run = false; }

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Inactive = true;
						}
					}
					else
					{
						// the address is not related to the current ones available to the prefetcher
						Seq_Access &= !pre_Force_Non_Seq;

						if (addr < 0x0A000000)
						{
							if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0
						}
						else if (addr < 0x0C000000)
						{
							if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
						}
						else
						{
							if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
						}

						pre_Force_Non_Seq = false;

						if (pre_Cycle_Glitch || pre_Cycle_Glitch_2)
						{
							// lose 1 cycle if prefetcher is holding the bus
							wait_ret += 1;
						}

						// abandon the prefetcher current fetch and reset the address
						pre_Buffer_Cnt = 0;
						pre_Fetch_Cnt = 0;
						pre_Run = pre_Enable;
						pre_Buffer_Was_Full = false;
						pre_Boundary_Reached = false;
						pre_Following = false;
						pre_Cycle_Glitch_2 = false;

						pre_Inactive = true;

						if (pre_Enable) { pre_Check_Addr = pre_Read_Addr = addr + 2; }
						else { pre_Check_Addr = 0; }
					}
				}
				else if (addr < 0x10000000)
				{
					wait_ret += SRAM_Waits; // SRAM

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
					pre_Cycle_Glitch_2 = false;
					pre_Boundary_Reached = false;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;

								ppu_VRAM_High_In_Use = true;
							}
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;

									ppu_VRAM_In_Use = true;
								}
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;

									ppu_VRAM_High_In_Use = true;
								}
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;

							ppu_VRAM_In_Use = true;
						}
					}
				}
				else
				{
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;

						ppu_PALRAM_In_Use = true;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += WRAM_Waits; //WRAM
			}

			return wait_ret;
		}

		public int Wait_State_Access_32_Instr(uint addr, bool Seq_Access)
		{
			int wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
				{
					if (addr == pre_Check_Addr)
					{
						if ((pre_Check_Addr != pre_Read_Addr) && (pre_Buffer_Cnt > 0))
						{
							// we have this address, can immediately read it
							pre_Check_Addr += 2;
							pre_Buffer_Cnt -= 1;

							// check if we have the next address as well
							if ((pre_Check_Addr != pre_Read_Addr) && (pre_Buffer_Cnt > 0))
							{
								// the prefetcher can return 32 bits in 1 cycle if it has it available
								pre_Check_Addr += 2;
								pre_Buffer_Cnt -= 1;

								if (pre_Buffer_Cnt == 0)
								{
									if (pre_Buffer_Was_Full) { pre_Check_Addr = 0; pre_Force_Non_Seq = true; }

									if (!pre_Enable) { pre_Check_Addr = 0; }
								}
							}
							else
							{
								// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
								if (pre_Buffer_Was_Full && (pre_Buffer_Cnt == 0))
								{
									Console.WriteLine("Jam due to disabled prefetcher mid instruction");
									wait_ret = (int)0x7FFFFFF0;					

									//abandon the prefetcher current fetch and reset
									pre_Fetch_Cnt = 0;
									pre_Check_Addr = 0;
								}
								else
								{
									// plus 1 since the prefetcher already used this cycle, so don't double count
									wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;

									// it is as if the cpu takes over a regular access, so reset the pre-fetcher
									pre_Inactive = true;

									pre_Read_Addr += 2;
									pre_Check_Addr += 2;
									pre_Fetch_Cnt = 0;
									pre_Buffer_Cnt = 0;
									pre_Run = pre_Enable;

									if (!pre_Enable) { pre_Check_Addr = 0; }
								}							
							}
						}
						else
						{
							// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
							if (!Seq_Access && (pre_Fetch_Cnt == 1) && !pre_Following)
							{
								//Console.WriteLine("32 " + TotalExecutedCycles + " " + cpu_Instr_Type);
								// this happens in a branch to the current prefetcher fetch address
								if (addr < 0x0A000000)
								{
									wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1;
								}
								else if (addr < 0x0C000000)
								{
									wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1;
								}
								else
								{
									wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1;
								}
							}
							else
							{
								// plus 1 since the prefetcher already used this cycle, so don't double count
								wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;

								// then add the second access
								if (addr < 0x0A000000)
								{
									wait_ret += ROM_Waits_0_S + 1; // ROM 0					
								}
								else if (addr < 0x0C000000)
								{
									wait_ret += ROM_Waits_1_S + 1; // ROM 1
								}
								else
								{
									wait_ret += ROM_Waits_2_S + 1; // ROM 2
								}
							}

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Inactive = true;

							pre_Read_Addr += 4;
							pre_Check_Addr += 4;
							pre_Fetch_Cnt = 0;
							pre_Run = pre_Enable;

							if (!pre_Enable) { pre_Check_Addr = 0; }
						}
					}
					else
					{
						// the address is not related to the current ones available to the prefetcher
						Seq_Access &= !pre_Force_Non_Seq;

						if (addr < 0x0A000000)
						{
							if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)				
						}
						else if (addr < 0x0C000000)
						{
							if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
						}
						else
						{
							if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
						}

						pre_Force_Non_Seq = false;

						if (pre_Cycle_Glitch)
						{
							// lose 1 cycle if prefetcher is holding the bus
							wait_ret += 1;
						}

						// abandon the prefetcher current fetch and reset the address
						pre_Buffer_Cnt = 0;
						pre_Fetch_Cnt = 0;
						pre_Run = pre_Enable;
						pre_Buffer_Was_Full = false;
						pre_Boundary_Reached = false;
						pre_Following = false;
						pre_Cycle_Glitch_2 = false;

						pre_Inactive = true;

						if (pre_Enable) { pre_Check_Addr = pre_Read_Addr = addr + 4; }
						else { pre_Check_Addr = 0; }
					}
				}
				else if (addr < 0x10000000)
				{
					wait_ret += SRAM_Waits; // SRAM

					if (pre_Cycle_Glitch || pre_Cycle_Glitch_2)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
					pre_Cycle_Glitch_2 = false;
					pre_Boundary_Reached = false;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;
							}

							// set to true since we also need to check the next cycle
							ppu_VRAM_High_In_Use = true;
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_In_Use = true;
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_High_In_Use = true;
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;
						}

						// set to true since we also need to check the next cycle
						ppu_VRAM_In_Use = true;
					}
				}
				else
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					// check both edges of the access
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_PALRAM_In_Use = true;
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
			}

			return wait_ret;
		}
	}
}
