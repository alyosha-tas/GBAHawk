using System;
using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Common.NumberExtensions;

/* 
	GameBoy Advance CPU (ARM7TDMI) Emulation
	NOTES:
	For now assume instruction after exceptions are not effected by LDM gltich

	Is SRAM mirrored every 32K/64K?

	Check timing of Stop

	What happens with swap when using register 15 as destination? (For now assume nothing.)

	What happens when specifying R15 in an MRS instruction? (For now assume nothing.)

	Investigate various edge cases of Bx instructions

	What value is written when a multi store accesses the base reg? (For now it is the initial value in all cases.)

	Instruction decoding happens after the pipeline is incremented, so is always done on cpu_Instr_ARM_2 or cpu_Instr_TMB_2
*/
namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		// General Execution
		public ulong CycleCount;

		public ulong Clock_Update_Cycle;

		public int[] cpu_Regs_To_Access = new int[16];

		public int cpu_Fetch_Cnt, cpu_Fetch_Wait;

		public int cpu_Multi_List_Ptr, cpu_Multi_List_Size, cpu_Temp_Reg_Ptr, cpu_Base_Reg, cpu_Base_Reg_2;

		public int cpu_ALU_Reg_Dest, cpu_ALU_Reg_Src;

		public int cpu_Mul_Cycles, cpu_Mul_Cycles_Cnt;

		public uint cpu_Instr_ARM_0, cpu_Instr_ARM_1, cpu_Instr_ARM_2;
		public uint cpu_Temp_Reg;
		public uint cpu_Temp_Addr;
		public uint cpu_Temp_Data;
		public uint cpu_Temp_Mode;
		public uint cpu_Bit_To_Check;
		public uint cpu_Write_Back_Addr;
		public uint cpu_Addr_Offset;
		public uint cpu_Last_Bus_Value;
		public uint cpu_Last_Bus_Value_Old;

		public uint cpu_ALU_Temp_Val, cpu_ALU_Temp_S_Val, cpu_ALU_Shift_Carry;

		public ushort cpu_Instr_TMB_0, cpu_Instr_TMB_1, cpu_Instr_TMB_2;
		public ushort cpu_Instr_Type;
		public ushort cpu_LDM_Glitch_Instr_Type;
		public ushort cpu_Exception_Type;
		public ushort cpu_Next_Load_Store_Type;

		public bool cpu_Thumb_Mode;
		public bool cpu_ARM_Cond_Passed;
		public bool cpu_Seq_Access;
		public bool cpu_IRQ_Input;
		public bool cpu_IRQ_Input_Use;
		public bool cpu_Next_IRQ_Input;
		public bool cpu_Next_IRQ_Input_2;
		public bool cpu_Next_IRQ_Input_3;
		public bool cpu_Is_Paused;
		public bool cpu_Take_Branch;
		public bool cpu_LS_Is_Load;
		public bool cpu_LS_First_Access;
		public bool cpu_Internal_Save_Access;
		public bool cpu_Invalidate_Pipeline;
		public bool cpu_Overwrite_Base_Reg;
		public bool cpu_Multi_Before;
		public bool cpu_Multi_Inc;
		public bool cpu_Multi_S_Bit;
		public bool cpu_ALU_S_Bit;
		public bool cpu_Multi_Swap;
		public bool cpu_Sign_Extend_Load;
		public bool cpu_Dest_Is_R15;
		public bool cpu_Swap_Store;
		public bool cpu_Swap_Lock;
		public bool cpu_Clear_Pipeline;
		public bool cpu_Special_Inc;
		public bool cpu_FlagI_Old;
		public bool cpu_LDM_Glitch_Mode;

		// ARM Related Variables
		public ushort cpu_Exec_ARM;

		// Thumb related Variables
		public ushort cpu_Exec_TMB;

		// Not stated, used internally only
		public long cpu_ALU_Signed_Long_Result;
		public ulong cpu_ALU_Long_Result;
		public int cpu_Shift_Imm;

		public bool stopped;

		public bool cpu_Trigger_Unhalt, cpu_Trigger_Unhalt_2, cpu_Trigger_Unhalt_3;

		public void cpu_Reset()
		{
			for (int i = 0; i < 16; i++) { cpu_Regs_To_Access[i] = 0; }

			cpu_Fetch_Cnt = cpu_Fetch_Wait = 0;

			cpu_Multi_List_Ptr = cpu_Multi_List_Size = cpu_Temp_Reg_Ptr = cpu_Base_Reg = cpu_Base_Reg_2 = 0;

			cpu_ALU_Reg_Dest = cpu_ALU_Reg_Src = 0;

			cpu_Mul_Cycles = cpu_Mul_Cycles_Cnt = 0;

			cpu_Temp_Reg = cpu_Temp_Addr = cpu_Temp_Data = cpu_Temp_Mode = cpu_Bit_To_Check = 0;

			cpu_Write_Back_Addr = cpu_Addr_Offset = cpu_Last_Bus_Value = cpu_Last_Bus_Value_Old = 0;		

			cpu_ALU_Temp_Val = cpu_ALU_Temp_S_Val = cpu_ALU_Shift_Carry = 0;

			cpu_Thumb_Mode = cpu_ARM_Cond_Passed = false; // Reset is exitted in ARM mode

			cpu_Instr_Type = cpu_Internal_Reset_1; // 2 internal cycles pass after rest before instruction fetching begins

			cpu_LDM_Glitch_Instr_Type = 0;

			cpu_Exception_Type = cpu_Next_Load_Store_Type = 0;

			ResetRegisters();

			cpu_Seq_Access = cpu_IRQ_Input = cpu_IRQ_Input_Use = cpu_Is_Paused = cpu_Take_Branch = false;

			cpu_Next_IRQ_Input = cpu_Next_IRQ_Input_2 = cpu_Next_IRQ_Input_3 = false;

			cpu_LS_Is_Load  = cpu_LS_First_Access = cpu_Internal_Save_Access = cpu_Invalidate_Pipeline = false;

			cpu_Overwrite_Base_Reg = cpu_Multi_Before = cpu_Multi_Inc = cpu_Multi_S_Bit = cpu_Multi_Swap = false;

			cpu_ALU_S_Bit = cpu_Sign_Extend_Load = cpu_Dest_Is_R15 = false;

			cpu_Swap_Store = cpu_Swap_Lock = cpu_Clear_Pipeline = cpu_Special_Inc = false;

			cpu_FlagI_Old = cpu_LDM_Glitch_Mode = false;

			stopped = false;

			cpu_Trigger_Unhalt = cpu_Trigger_Unhalt_2 = cpu_Trigger_Unhalt_3 = false;
		}

		//this only calls when the first byte of an instruction is fetched.
		public Action<ushort> OnExecFetch;

		// Execute instructions
		public void cpu_Tick()
		{
			switch (cpu_Instr_Type)
			{
				case cpu_Internal_And_Prefetch_ARM:
					// In this code path the instruction takes only one internal cycle, a pretech is also done
					// so necessarily the condition code check happens here, and interrupts may occur
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = true;
					}
					break;

				case cpu_Internal_And_Prefetch_2_ARM:
					// In this code path the instruction takes 2 internal cycles, a pretech is also done
					// so necessarily the condition code check happens here, and interrupts may occur if it fails
					// TODO: In ARM mode, does a failed condition execute 1 cycle or 2? (For now assume 1)
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						if (cpu_ARM_Cond_Passed)
						{
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;

							// instructions with internal cycles revert to non-sequential accesses 
							cpu_Seq_Access = false;
						}
						else
						{
							// do interrupt check and proceed to next instruction
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;					
					}
					break;

				case cpu_Internal_And_Prefetch_3_ARM:
					// This code path comes from instructions that modify CPSR (only from ARM mode)
					// if we end up in thumb state, invalidate instruction pipeline
					// otherwise check interrupts
					// NOTE: Here we must use the old value of the I flag
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_FlagI_Old = cpu_FlagI;

						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						if (cpu_Thumb_Mode)
						{
							// invalidate the instruction pipeline
							cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;

							cpu_Seq_Access = false;
						}
						else
						{
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Internal_And_Branch_1_ARM:
					// decide whether or not to branch. If no branch taken, interrupts may occur
					if (cpu_Fetch_Cnt == 0)
					{
						// whether or not to take the branch is determined in the instruction execution
						cpu_Take_Branch = false;

						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

						if (cpu_Take_Branch)
						{
							cpu_Regs[15] = cpu_Temp_Reg;

							// Invalidate instruction pipeline
							cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
						}
						else
						{
							cpu_Regs[15] += 4;

							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = cpu_Take_Branch ? false : true;
					}
					break;

				case cpu_Internal_And_Branch_2_ARM:
					// this code path comes from an ALU instruction in ARM mode using R15 as the destination register
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_FlagI_Old = cpu_FlagI;
						
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

						cpu_Execute_Internal_Only_ARM();

						if (cpu_ARM_Cond_Passed)
						{
							// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
							if (cpu_ALU_S_Bit)
							{
								if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
								{
									//Console.WriteLine("using reg swap on bad mode");
								}
								else
								{
									//upper bit of mode must always be set
									cpu_Swap_Regs((cpu_Regs[17] & 0x1F) | 0x10, false, true);
								}

								if (cpu_FlagT) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
								else { cpu_Thumb_Mode = false; }

								cpu_ALU_S_Bit = false;
							}

							// Invalidate instruction pipeline if necessary
							if (cpu_Clear_Pipeline)
							{
								if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
								else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }
								cpu_Seq_Access = false;
							}
							else
							{
								cpu_Regs[15] += 4;

								cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
								cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

								if (cpu_IRQ_Input_Use & !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
								else { cpu_Decode_ARM(); }

								cpu_Seq_Access = true;
							}
						}
						else
						{
							cpu_Regs[15] += 4;

							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Internal_And_Branch_3_ARM:
					// This code path comes from an ALU instruction in ARM mode using R15 as the destination register
					// and the shift is defined by another register (therefore taking an extra cycle to complete)
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

						if (cpu_ARM_Cond_Passed)
						{
							cpu_Instr_Type = cpu_Internal_And_Branch_4_ARM;
						}
						else
						{
							cpu_Regs[15] += 4;

							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Prefetch_Only_1_ARM:
					// In this code path the instruction pipeline is being refilled, and is part of an atomic instruction (cannot be interrupted)
					// so no instruction execution takes place
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Instr_Type = cpu_Prefetch_Only_2_ARM;
						cpu_Seq_Access = true;
					}
					break;

				case cpu_Prefetch_Only_2_ARM:
					// This code path is the last cycle of pipeline refill, no instruction execution but interrupts may occur
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = true;
					}
					break;

				case cpu_Prefetch_And_Load_Store_ARM:
					// First cycle of load / store, cannot be interrupted (if executed), prefetch occurs
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						if (cpu_ARM_Cond_Passed)
						{
							cpu_Instr_Type = cpu_Next_Load_Store_Type;
							cpu_Seq_Access = false;
						}
						else
						{
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }
							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;					
					}
					break;

				case cpu_Load_Store_Word_ARM:
					// Single load / store word, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// 32 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							// deal with misaligned accesses
							if ((cpu_Temp_Addr & 3) == 0)
							{
								cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
							}
							else
							{
								cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

								for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
								{
									cpu_ALU_Temp_Val = (uint)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
								}

								cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}	
						}
						else
						{
							Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
						}

						if (cpu_Overwrite_Base_Reg)
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							cpu_Overwrite_Base_Reg = false;
						}

						if (cpu_LS_Is_Load)
						{
							if (cpu_Temp_Reg_Ptr == 15)
							{
								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								cpu_Seq_Access = false;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = false;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Load_Store_Half_ARM:
					// Single load / store half word, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// 16 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_16(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							// deal with misaligned accesses
							if ((cpu_Temp_Addr & 1) == 0)
							{
								cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);
								
								if (cpu_Sign_Extend_Load)
								{
									if ((cpu_ALU_Temp_Val & 0x8000) == 0x8000)
									{
										cpu_ALU_Temp_Val = (uint)(cpu_ALU_Temp_Val | 0xFFFF0000);
									}
								}

								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
							else
							{
								if (cpu_Sign_Extend_Load)
								{
									cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

									if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
									{
										cpu_ALU_Temp_Val = (uint)(cpu_ALU_Temp_Val | 0xFFFFFF00);
									}

									cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
								}
								else
								{
									cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

									cpu_ALU_Temp_Val = (uint)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));

									cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
									cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
								}
							}		
						}
						else
						{
							Write_Memory_16(cpu_Temp_Addr, (ushort)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFFFF));
						}

						if (cpu_Overwrite_Base_Reg)
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							cpu_Overwrite_Base_Reg = false;
						}

						cpu_Sign_Extend_Load = false;

						if (cpu_LS_Is_Load)
						{
							if (cpu_Temp_Reg_Ptr == 15)
							{
								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								cpu_Seq_Access = false;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = false;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Load_Store_Byte_ARM:
					// Single load / store byte, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// 8 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_8(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

							if (cpu_Sign_Extend_Load)
							{
								if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
								{
									cpu_ALU_Temp_Val = (uint)(cpu_ALU_Temp_Val | 0xFFFFFF00);
								}
							}

							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
						else
						{
							Write_Memory_8(cpu_Temp_Addr, (byte)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFF));
						}

						if (cpu_Overwrite_Base_Reg)
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							cpu_Overwrite_Base_Reg = false;
						}

						cpu_Sign_Extend_Load = false;

						if (cpu_LS_Is_Load)
						{
							if (cpu_Temp_Reg_Ptr == 15)
							{
								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								cpu_Seq_Access = false;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = false;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Multi_Load_Store_ARM:
					// Repeated load / store operations, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// update this here so the wait state processor knows about it for 32 bit accesses to VRAM and PALRAM
						cpu_Temp_Reg_Ptr = cpu_Regs_To_Access[cpu_Multi_List_Ptr];

						// 32 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
						}
						else
						{
							Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
						}

						// base register is updated after the first memory access
						if (cpu_LS_First_Access && cpu_Overwrite_Base_Reg)
						{
							if (cpu_Multi_Inc)
							{
								if (cpu_Multi_Before)
								{
									cpu_Regs[cpu_Base_Reg] = (uint)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size) - 4);
								}
								else
								{
									cpu_Regs[cpu_Base_Reg] = (uint)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size));
								}

								if (cpu_Special_Inc)
								{
									cpu_Special_Inc = false;
									cpu_Regs[cpu_Base_Reg] += 0x3C;
								}
							}
							else
							{
								cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							}

							cpu_LS_First_Access = false;
							cpu_Overwrite_Base_Reg = false;
						}

						// always incrementing since addresses always start at the lowest one
						// always after because 'before' cases built into address at initialization
						cpu_Temp_Addr += 4;

						cpu_Multi_List_Ptr++;

						// if done, the next cycle depends on whether or not Reg 15 was written to
						if (cpu_Multi_List_Ptr == cpu_Multi_List_Size)
						{
							cpu_LDM_Glitch_Mode = false;

							if (cpu_Multi_Swap)
							{
								cpu_Swap_Regs(cpu_Temp_Mode, false, false);

								cpu_LDM_Glitch_Mode = true;

								Console.WriteLine("LDM_Glitch");
							}

							if (cpu_LS_Is_Load)
							{									
								if (cpu_Regs_To_Access[cpu_Multi_List_Ptr - 1] == 15)
								{
									// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
									if (cpu_Multi_S_Bit)
									{
										cpu_Regs[16] = cpu_Regs[17];

										if (cpu_FlagT) { cpu_Thumb_Mode = true; }
										else { cpu_Thumb_Mode = false; }

										cpu_Multi_S_Bit = false;
									}

									// is the timing of this correct?
									if (cpu_Thumb_Mode) { cpu_Regs[15] &= 0xFFFFFFFE; }
									else { cpu_Regs[15] &= 0xFFFFFFFC; }

									// Invalidate instruction pipeline
									cpu_Invalidate_Pipeline = true;
									cpu_Seq_Access = false;

									if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Internal_Can_Save_TMB; }
									else { cpu_Instr_Type = cpu_Internal_Can_Save_ARM; }
								}
								else
								{
									// if the next cycle is a memory access, one cycle can be saved
									if (cpu_LDM_Glitch_Mode)
									{
										cpu_Instr_Type = cpu_LDM_Glitch_Mode_Execute;

										cpu_LDM_Glitch_Instr_Type = cpu_Internal_Can_Save_ARM;
									}
									else
									{
										cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
									}									
									
									cpu_Internal_Save_Access = true;
									cpu_Seq_Access = false;
								}
							}
							else
							{
								// when writing, there is no last internal cycle, proceed to the next instruction
								// also check for interrupts
								cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
								cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

								if (cpu_LDM_Glitch_Mode)
								{
									if (cpu_IRQ_Input_Use & !cpu_FlagI)
									{										
										cpu_Instr_Type = cpu_Prefetch_IRQ;
										cpu_LDM_Glitch_Mode = false;
									}
									else
									{
										cpu_Instr_Type = cpu_LDM_Glitch_Mode_Execute;
										cpu_LDM_Glitch_Decode_ARM();
									}
								}
								else
								{
									if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
									else { cpu_Decode_ARM(); }
								}

								cpu_Seq_Access = false;
							}						
						}
						else
						{
							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;	
					}
					break;

				case cpu_Multiply_ARM:
					// Multiplication with possibly early termination
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_ARM_Cond_Passed) { cpu_Instr_Type = cpu_Multiply_Cycles; }
						else 
						{
							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = true;
					}
					break;

				case cpu_Prefetch_Swap_ARM:
					// First cycle of swap, locks the bus at the end of the cycle
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						if (cpu_ARM_Cond_Passed)
						{
							cpu_Instr_Type = cpu_Next_Load_Store_Type;
							cpu_Seq_Access = false;

							// Lock the bus
							cpu_Swap_Lock = true;
						}
						else
						{
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }
							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Swap_ARM:
					// Swap data between registers and memory
					// no intervening memory accesses can occur between the read and the write
					// note that interrupts are checked on the last instruction cycle, handled in cpu_Internal_Can_Save
					if (cpu_Fetch_Cnt == 0)
					{
						// need this here for the 32 bit wait state processor in case VRAM and PALRAM accesses are interrupted
						cpu_LS_Is_Load = !cpu_Swap_Store;

						if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
						{
							cpu_Fetch_Wait = Wait_State_Access_32(cpu_Regs[cpu_Base_Reg], cpu_Seq_Access);
						}
						else
						{
							cpu_Fetch_Wait = Wait_State_Access_8(cpu_Regs[cpu_Base_Reg], cpu_Seq_Access);
						}
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_Swap_Store)
						{
							if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
							{
								Write_Memory_8(cpu_Regs[cpu_Base_Reg], (byte)cpu_Regs[cpu_Temp_Reg_Ptr]);
							}
							else
							{
								Write_Memory_32(cpu_Regs[cpu_Base_Reg], cpu_Regs[cpu_Temp_Reg_Ptr]);
							}

							cpu_Regs[cpu_Base_Reg_2] = cpu_Temp_Data;

							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Invalidate_Pipeline = false;
							cpu_Internal_Save_Access = true;

							// unlock the bus
							cpu_Swap_Lock = false;
						}
						else
						{
							if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
							{
								cpu_Temp_Data = Read_Memory_8(cpu_Regs[cpu_Base_Reg]);
							}
							else
							{
								// deal with misaligned accesses
								cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

								if ((cpu_Temp_Addr & 3) == 0)
								{
									cpu_Temp_Data = Read_Memory_32(cpu_Temp_Addr);
								}
								else
								{
									cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

									for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
									{
										cpu_ALU_Temp_Val = (uint)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
									}

									cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
									cpu_Temp_Data = cpu_ALU_Temp_Val;
								}
							}

							cpu_Swap_Store = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = false;
					}
					break;

				case cpu_Prefetch_And_Branch_Ex_ARM:
					// Branch from ARM mode to Thumb (possibly)
					// interrupt only if condition failed
					if (cpu_Fetch_Cnt == 0)
					{
						// start in thumb mode, always branch
						cpu_Take_Branch = true;

						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_ARM();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_ARM_Cond_Passed)
						{
							cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

							cpu_Regs[15] = cpu_Temp_Reg;

							cpu_FlagT = (cpu_Regs[cpu_Base_Reg] & 1) == 1;
							cpu_Thumb_Mode = cpu_FlagT;

							// Invalidate instruction pipeline
							if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
							else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }

							cpu_Seq_Access = false;
						}
						else
						{
							cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
							cpu_Regs[15] += 4;

							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Internal_And_Prefetch_TMB:
					// In this code path the instruction takes only one internal cycle, a pretech is also done
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_TMB();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						cpu_Regs[15] += 2;

						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = true;
					}
					break;

				case cpu_Internal_And_Prefetch_2_TMB:
					// In this code path the instruction takes 2 internal cycles, a pretech is also done
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_TMB();
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						cpu_Regs[15] += 2;

						cpu_Instr_Type = cpu_Internal_Can_Save_TMB;

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						// instructions with internal cycles revert to non-sequential accesses 
						cpu_Seq_Access = false;
					}
					break;

				case cpu_Internal_And_Branch_1_TMB:
					// decide whether or not to branch. If no branch taken, interrupts may occur
					if (cpu_Fetch_Cnt == 0)
					{
						// whether or not to take the branch is determined in the instruction execution
						cpu_Take_Branch = false;

						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_TMB();

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);

						if (cpu_Take_Branch)
						{
							cpu_Regs[15] = (cpu_Temp_Reg & 0xFFFFFFFE);

							// Invalidate instruction pipeline
							cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;
						}
						else
						{
							cpu_Regs[15] += 2;

							cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
							cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_TMB(); }
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = cpu_Take_Branch ? false : true;
					}
					break;

				case cpu_Prefetch_Only_1_TMB:
					// In this code path the instruction pipeline is being refilled, and is part of an atomic instruction (cannot be interrupted)
					// so no instruction execution takes place
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						cpu_Regs[15] += 2;

						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Instr_Type = cpu_Prefetch_Only_2_TMB;
						cpu_Seq_Access = true;
					}
					break;

				case cpu_Prefetch_Only_2_TMB:
					// This code path is the last cycle of pipeline refill, no instruction execution but interrupts may occur
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						cpu_Regs[15] += 2;

						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = true;
					}
					break;

				case cpu_Prefetch_And_Load_Store_TMB:
					// First cycle of load / store, cannot be interrupted (if executed), prefetch occurs
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_TMB();
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						cpu_Regs[15] += 2;

						cpu_Instr_Type = cpu_Next_Load_Store_Type;
						cpu_Seq_Access = false;

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Load_Store_Word_TMB:
					// Single load / store word, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// 32 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							// deal with misaligned accesses
							if ((cpu_Temp_Addr & 3) == 0)
							{
								cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
							}
							else
							{
								cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

								for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
								{
									cpu_ALU_Temp_Val = (uint)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
								}

								cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
						}
						else
						{
							Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
						}

						if (cpu_Overwrite_Base_Reg)
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							cpu_Overwrite_Base_Reg = false;
						}

						if (cpu_LS_Is_Load)
						{
							if (cpu_Temp_Reg_Ptr == 15)
							{
								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Seq_Access = false;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
							cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_TMB(); }

							cpu_Seq_Access = false;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Load_Store_Half_TMB:
					// Single load / store half word, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// 16 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_16(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							// deal with misaligned accesses
							if ((cpu_Temp_Addr & 1) == 0)
							{
								cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

								if (cpu_Sign_Extend_Load)
								{
									if ((cpu_ALU_Temp_Val & 0x8000) == 0x8000)
									{
										cpu_ALU_Temp_Val = (uint)(cpu_ALU_Temp_Val | 0xFFFF0000);
									}
								}

								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
							else
							{
								if (cpu_Sign_Extend_Load)
								{
									cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

									if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
									{
										cpu_ALU_Temp_Val = (uint)(cpu_ALU_Temp_Val | 0xFFFFFF00);
									}

									cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
								}
								else
								{
									cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

									cpu_ALU_Temp_Val = (uint)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));

									cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
									cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
								}
							}
						}
						else
						{
							Write_Memory_16(cpu_Temp_Addr, (ushort)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFFFF));
						}

						if (cpu_Overwrite_Base_Reg)
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							cpu_Overwrite_Base_Reg = false;
						}

						cpu_Sign_Extend_Load = false;

						if (cpu_LS_Is_Load)
						{
							if (cpu_Temp_Reg_Ptr == 15)
							{
								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Seq_Access = false;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
							cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_TMB(); }

							cpu_Seq_Access = false;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Load_Store_Byte_TMB:
					// Single load / store byte, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// 8 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_8(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

							if (cpu_Sign_Extend_Load)
							{
								if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
								{
									cpu_ALU_Temp_Val = (uint)(cpu_ALU_Temp_Val | 0xFFFFFF00);
								}
							}

							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
						else
						{
							Write_Memory_8(cpu_Temp_Addr, (byte)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFF));
						}

						if (cpu_Overwrite_Base_Reg)
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							cpu_Overwrite_Base_Reg = false;
						}

						cpu_Sign_Extend_Load = false;

						if (cpu_LS_Is_Load)
						{
							if (cpu_Temp_Reg_Ptr == 15)
							{
								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Seq_Access = false;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
							cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_TMB(); }

							cpu_Seq_Access = false;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Multi_Load_Store_TMB:
					// Repeated load / store operations, last cycle of a write can be interrupted
					// Can be interrupted by an abort, but those don't occur in GBA
					if (cpu_Fetch_Cnt == 0)
					{
						// update this here so the wait state processor knows about it for 32 bit accesses to VRAM and PALRAM
						cpu_Temp_Reg_Ptr = cpu_Regs_To_Access[cpu_Multi_List_Ptr];

						// 32 bit fetch regardless of mode
						cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_LS_Is_Load)
						{
							cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
						}
						else
						{
							Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
						}

						// base register is updated after the first memory access
						if (cpu_LS_First_Access && cpu_Overwrite_Base_Reg)
						{
							if (cpu_Multi_Inc)
							{
								if (cpu_Multi_Before)
								{
									cpu_Regs[cpu_Base_Reg] = (uint)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size) - 4);
								}
								else
								{
									cpu_Regs[cpu_Base_Reg] = (uint)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size));
								}

								if (cpu_Special_Inc)
								{
									cpu_Special_Inc = false;
									cpu_Regs[cpu_Base_Reg] += 0x3C;
								}
							}
							else
							{
								cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
							}

							cpu_LS_First_Access = false;
							cpu_Overwrite_Base_Reg = false;
						}

						// always incrementing since addresses always start at the lowest one
						// always after because 'before' cases built into address at initialization
						cpu_Temp_Addr += 4;

						cpu_Multi_List_Ptr++;

						// if done, the next cycle depends on whether or not Reg 15 was written to
						if (cpu_Multi_List_Ptr == cpu_Multi_List_Size)
						{
							if (cpu_Multi_Swap)
							{
								cpu_Swap_Regs(cpu_Temp_Mode, false, false);
							}

							if (cpu_LS_Is_Load)
							{
								if (cpu_Regs_To_Access[cpu_Multi_List_Ptr - 1] == 15)
								{
									// Thumb mode has a special case of adding 0x40 to the base reg
									// is it maybe 16 accesses all writing to PC? (effects timing)
									cpu_Regs[15] &= 0xFFFFFFFE;

									// Invalidate instruction pipeline
									cpu_Invalidate_Pipeline = true;
									cpu_Seq_Access = false;
									cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								}
								else
								{
									// if the next cycle is a memory access, one cycle can be saved
									cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
									cpu_Internal_Save_Access = true;
									cpu_Seq_Access = false;
								}
							}
							else
							{
								// when writing, there is no last internal cycle, proceed to the next instruction
								// also check for interrupts
								cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
								cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

								if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
								else { cpu_Decode_TMB(); }

								cpu_Seq_Access = false;
							}
						}
						else
						{
							cpu_Seq_Access = true;
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Multiply_TMB:
					// Multiplication with possibly early termination
					if (cpu_Fetch_Cnt == 0)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_TMB();
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						cpu_Regs[15] += 2;

						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						cpu_Instr_Type = cpu_Multiply_Cycles;

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = true;
					}
					break;

				case cpu_Prefetch_And_Branch_Ex_TMB:
					// Branch from Thumb mode to ARM mode, no interrupt check due to pipeline invalidation
					if (cpu_Fetch_Cnt == 0)
					{
						// start in thumb mode, always branch
						cpu_Take_Branch = true;

						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

						cpu_Execute_Internal_Only_TMB();
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);

						cpu_Regs[15] = cpu_Temp_Reg;

						cpu_FlagT = (cpu_Regs[cpu_Base_Reg] & 1) == 1;
						cpu_Thumb_Mode = cpu_FlagT;

						// Invalidate instruction pipeline
						if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
						else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;

						cpu_Seq_Access = false;
					}
					break;

				case cpu_Prefetch_And_SWI_Undef:
					// This code path is the exception pretech cycle for SWI and undefined instructions
					if (cpu_Fetch_Cnt == 0)
					{
						if (cpu_Thumb_Mode)
						{
							cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);
						}
						else
						{
							cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

							// In ARM mode, we might not actually generate an exception if the condition code fails
							cpu_Execute_Internal_Only_ARM();
						}

						cpu_IRQ_Input_Use = cpu_IRQ_Input;
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_Thumb_Mode || cpu_ARM_Cond_Passed)
						{
							if (cpu_Thumb_Mode)
							{
								cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
							}
							else
							{
								cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
							}

							cpu_Seq_Access = false;

							if (cpu_Exception_Type == cpu_SWI_Exc)
							{
								TraceCallback?.Invoke(new(disassembly: "==== SWI ====", registerInfo: string.Empty));

								// supervisor mode
								cpu_Swap_Regs(0x13, true, false);

								// R14 becomes return address
								cpu_Regs[14] = cpu_Thumb_Mode ? (cpu_Regs[15] - 2) : (cpu_Regs[15] - 4);

								// take exception vector 0x8
								cpu_Regs[15] = 0x00000008;
							}
							else
							{
								TraceCallback?.Invoke(new(disassembly: "==== UNDEF ====", registerInfo: string.Empty));

								// undefined instruction mode
								cpu_Swap_Regs(0x1B, true, false);

								// R14 becomes return address
								cpu_Regs[14] = cpu_Thumb_Mode ? (cpu_Regs[15] - 2) : (cpu_Regs[15] - 4);

								// take exception vector 0x4
								cpu_Regs[15] = 0x00000004;
							}		

							// Normal Interrupts disabled
							cpu_FlagI = true;

							// switch into ARM mode
							cpu_Thumb_Mode = false;
							cpu_FlagT = false;

							// Invalidate instruction pipeline
							cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
						}
						else
						{
							cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

							cpu_Regs[15] += 4;

							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							cpu_Decode_ARM();
							cpu_Seq_Access = true;

							if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						}

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Prefetch_IRQ:
					// IRQ uses a prefetch cycle not an internal cycle (just like swi and undef)
					if (cpu_Fetch_Cnt == 0)
					{
						TraceCallback?.Invoke(new(disassembly: "====IRQ====", registerInfo: string.Empty));

						if (cpu_Thumb_Mode)
						{
							cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);
						}
						else
						{
							cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);
						}
					}

					cpu_Fetch_Cnt += 1;

					if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
					{
						if (cpu_Thumb_Mode)
						{
							cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						}
						else
						{
							cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						}

						// IRQ mode
						cpu_Swap_Regs(0x12, true, false);

						// R14 becomes return address + 4
						if (cpu_Thumb_Mode)
						{
							cpu_Regs[14] = cpu_Regs[15];
						}
						else
						{
							cpu_Regs[14] = cpu_Regs[15] - 4;
						}

						// Normal Interrupts disabled
						cpu_FlagI = true;

						// take exception vector 0x18
						cpu_Regs[15] = 0x00000018;

						// switch into ARM mode
						cpu_Thumb_Mode = false;
						cpu_FlagT = false;

						// Invalidate instruction pipeline
						cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;

						cpu_Seq_Access = false;

						cpu_Fetch_Cnt = 0;
						cpu_Fetch_Wait = 0;
					}
					break;

				case cpu_Internal_Reset_1:
					cpu_Instr_Type = cpu_Internal_Reset_2;
					cpu_Exception_Type = cpu_Reset_Exc;
					break;

				case cpu_Internal_Reset_2:
					// switch into ARM mode
					cpu_Thumb_Mode = false;
					cpu_FlagT = false;

					// Invalidate instruction pipeline
					cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
					break;

				case cpu_Internal_And_Branch_4_ARM:
					// This code path comes from an ALU instruction in ARM mode using R15 as the destination register
					// and is the second half of the (implied) branch. No memory access, and no possible cycle save due to branch
					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;

					cpu_FlagI_Old = cpu_FlagI;

					// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
					if (cpu_ALU_S_Bit)
					{
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							//Console.WriteLine("using reg swap on bad mode");
						}
						else
						{
							//upper bit of mode must always be set
							cpu_Swap_Regs((cpu_Regs[17] & 0x1F) | 0x10, false, true);
						}

						if (cpu_FlagT) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
						else { cpu_Thumb_Mode = false; }

						cpu_ALU_S_Bit = false;
					}

					// Invalidate instruction pipeline if necessary
					if (cpu_Clear_Pipeline)
					{
						if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
						else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }
						cpu_Seq_Access = false;
					}
					else
					{
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						// instructions with internal cycles revert to non-sequential accesses 
						cpu_Seq_Access = false;
					}
					break;

				case cpu_Internal_Can_Save_ARM:
					// Last Internal cycle of an instruction, note that the actual operation was already completed
					// This cycle is interruptable
					// acording to ARM documentation, this cycle can be combined with the following memory access
					// but it appears that the GBA does not do so
					cpu_IRQ_Input_Use = cpu_IRQ_Input;

					if (cpu_Invalidate_Pipeline)
					{
						cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
					}
					else
					{
						// A memory access cycle could be saved here, but the GBA does not seem to implement it
						if (cpu_Internal_Save_Access) {  }

						// next instruction was already prefetched, decode it here
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }
					}

					cpu_Internal_Save_Access = false;
					cpu_Invalidate_Pipeline = false;
					break;

				case cpu_Internal_Can_Save_TMB:
					// Last Internal cycle of an instruction, note that the actual operation was already completed
					// This cycle is interruptable
					// acording to ARM documentation, this cycle can be combined with the following memory access
					// but it appears that the GBA does not do so			
					cpu_IRQ_Input_Use = cpu_IRQ_Input;

					if (cpu_Invalidate_Pipeline)
					{
						cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;
					}
					else
					{
						// A memory access cycle could be saved here, but the GBA does not seem to implement it
						if (cpu_Internal_Save_Access) { }

						// next instruction was already prefetched, decode it here
						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }
					}

					cpu_Internal_Save_Access = false;
					cpu_Invalidate_Pipeline = false;
					break;

				case cpu_LDM_Glitch_Mode_Execute:
					//Console.WriteLine("glitch tick");
					cpu_LDM_Glitch_Tick();
					break;

				case cpu_Internal_Halted:
					if (cpu_Trigger_Unhalt)
					{
						if (!Halt_Leave)
						{
							Halt_Leave = true;
							Halt_Leave_cd = 2;
							IRQ_Delays = true;
							delays_to_process = true;
						}
					}
					break;

				case cpu_Multiply_Cycles:
					// cycles of the multiply instruction
					// check for IRQs at the end
					cpu_Mul_Cycles_Cnt += 1;

					if (cpu_Mul_Cycles_Cnt == cpu_Mul_Cycles)
					{
						cpu_IRQ_Input_Use = cpu_IRQ_Input;

						if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else if (cpu_Thumb_Mode) { cpu_Decode_TMB(); }
						else { cpu_Decode_ARM(); }

						cpu_Mul_Cycles_Cnt = 0;

						// Multiply forces the next access to be non-sequential
						cpu_Seq_Access = false;
					}
					break;

				// Check timing?
				case cpu_Pause_For_DMA:
					if (dma_Held_CPU_Instr >= 42)
					{
						switch (dma_Held_CPU_Instr)
						{
							case cpu_Internal_And_Branch_4_ARM:
								// This code path comes from an ALU instruction in ARM mode using R15 as the destination register
								// and is the second half of the (implied) branch. No memory access, and no possible cycle save due to branch
								cpu_Execute_Internal_Only_ARM();

								cpu_IRQ_Input_Use = cpu_IRQ_Input;

								cpu_FlagI_Old = cpu_FlagI;

								// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
								if (cpu_ALU_S_Bit)
								{
									if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
									{
										//Console.WriteLine("using reg swap on bad mode");
									}
									else
									{
										//upper bit of mode must always be set
										cpu_Swap_Regs((cpu_Regs[17] & 0x1F) | 0x10, false, true);
									}

									if (cpu_FlagT) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
									else { cpu_Thumb_Mode = false; }

									cpu_ALU_S_Bit = false;
								}

								// Invalidate instruction pipeline if necessary
								if (cpu_Clear_Pipeline)
								{
									if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
									else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }
									cpu_Seq_Access = false;
								}
								else
								{
									cpu_Regs[15] += 4;

									cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
									cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

									if (cpu_IRQ_Input_Use & !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
									else { cpu_Decode_ARM(); }

									// instructions with internal cycles revert to non-sequential accesses 
									cpu_Seq_Access = false;
								}
								break;

							case cpu_Internal_Can_Save_ARM:
								// Last Internal cycle of an instruction, note that the actual operation was already completed
								// This cycle is interruptable,
								// acording to ARM documentation, this cycle can be combined with the following memory access
								// but it appears that the GBA does not do so				
								cpu_IRQ_Input_Use = cpu_IRQ_Input;

								if (cpu_Invalidate_Pipeline)
								{
									cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
								}
								else
								{
									// A memory access cycle could be saved here, but the GBA does not seem to implement it
									if (cpu_Internal_Save_Access) { }

									// next instruction was already prefetched, decode it here
									cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
									cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

									if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
									else { cpu_Decode_ARM(); }
								}

								cpu_Internal_Save_Access = false;
								cpu_Invalidate_Pipeline = false;
								break;

							case cpu_Internal_Can_Save_TMB:
								// Last Internal cycle of an instruction, note that the actual operation was already completed
								// This cycle is interruptable,
								// acording to ARM documentation, this cycle can be combined with the following memory access
								// but it appears that the GBA does not do so				
								cpu_IRQ_Input_Use = cpu_IRQ_Input;

								if (cpu_Invalidate_Pipeline)
								{
									cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;
								}
								else
								{
									// A memory access cycle could be saved here, but the GBA does not seem to implement it
									if (cpu_Internal_Save_Access) { }

									// next instruction was already prefetched, decode it here
									cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
									cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

									if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
									else { cpu_Decode_TMB(); }
								}

								cpu_Internal_Save_Access = false;
								cpu_Invalidate_Pipeline = false;
								break;

							case cpu_LDM_Glitch_Mode_Execute:
								cpu_LDM_Glitch_Tick();
								break;

							case cpu_Internal_Halted:

								if (cpu_Trigger_Unhalt)
								{
									if (!Halt_Leave)
									{
										Halt_Leave = true;
										Halt_Leave_cd = 2;
										IRQ_Delays = true;
										delays_to_process = true;
									}
								}

								break;

							case cpu_Multiply_Cycles:
								// cycles of the multiply instruction
								// check for IRQs at the end
								cpu_Mul_Cycles_Cnt += 1;

								if (cpu_Mul_Cycles_Cnt == cpu_Mul_Cycles)
								{
									cpu_IRQ_Input_Use = cpu_IRQ_Input;

									if (cpu_IRQ_Input_Use & !cpu_FlagI) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
									else if (cpu_Thumb_Mode) { cpu_Decode_TMB(); }
									else { cpu_Decode_ARM(); }

									cpu_Mul_Cycles_Cnt = 0;

									// Multiply forces the next access to be non-sequential
									cpu_Seq_Access = false;
								}
								break;
						}

						if (cpu_Instr_Type != cpu_Pause_For_DMA)
						{
							// change the DMA held instruction
							dma_Held_CPU_Instr = cpu_Instr_Type;
							cpu_Instr_Type = cpu_Pause_For_DMA;
						}
					}

					if (!cpu_Is_Paused) { cpu_Instr_Type = dma_Held_CPU_Instr; }
					break;			
			}
		}

		// tracer stuff
		public Action<TraceInfo> TraceCallback;

		public string TraceHeader => "ARM7TDMI: PC, machine code, mnemonic, operands, registers, Cy, flags (ZNHCIFE)";

		public TraceInfo State(bool disassemble = true)
			=> new(
				disassembly: $"{(disassemble ? Disassemble() : "---")} ".PadRight(70),
				registerInfo: string.Join(" ",
					$"R0:{cpu_Regs[0]:X8}",
					$"R1:{cpu_Regs[1]:X8}",
					$"R2:{cpu_Regs[2]:X8}",
					$"R3:{cpu_Regs[3]:X8}",
					$"R4:{cpu_Regs[4]:X8}",
					$"R5:{cpu_Regs[5]:X8}",
					$"R6:{cpu_Regs[6]:X8}",
					$"R7:{cpu_Regs[7]:X8}",
					$"R8:{cpu_Regs[8]:X8}",
					$"R9:{cpu_Regs[9]:X8}",
					$"R10:{cpu_Regs[10]:X8}",
					$"R11:{cpu_Regs[11]:X8}",
					$"R12:{cpu_Regs[12]:X8}",
					$"R13:{cpu_Regs[13]:X8}",
					$"R14:{cpu_Regs[14]:X8}",
					$"R15:{cpu_Regs[15]:X8}",
					$"CPSR:{cpu_Regs[16]:X8}",
					$"SPSR:{cpu_Regs[17]:X8}",
					$"Cy:{CycleCount}",
					$"LY:{ppu_LY}",
					string.Concat(
						cpu_FlagN ? "N" : "n",
						cpu_FlagZ ? "Z" : "z",
						cpu_FlagC ? "C" : "c",
						cpu_FlagV ? "V" : "v",
						cpu_FlagI ? "I" : "i",
						cpu_FlagF ? "F" : "f",
						INT_Master_On ? "E" : "e")));

		// State Save/Load
		public void cpu_SyncState(Serializer ser)
		{
			// General
			ser.Sync(nameof(CycleCount), ref CycleCount);
			ser.Sync(nameof(Clock_Update_Cycle), ref Clock_Update_Cycle);

			ser.Sync(nameof(cpu_Regs_To_Access), ref cpu_Regs_To_Access, false);

			ser.Sync(nameof(cpu_Fetch_Cnt), ref cpu_Fetch_Cnt);
			ser.Sync(nameof(cpu_Fetch_Wait), ref cpu_Fetch_Wait);

			ser.Sync(nameof(cpu_Multi_List_Ptr), ref cpu_Multi_List_Ptr);
			ser.Sync(nameof(cpu_Multi_List_Size), ref cpu_Multi_List_Size);
			ser.Sync(nameof(cpu_Temp_Reg_Ptr), ref cpu_Temp_Reg_Ptr);
			ser.Sync(nameof(cpu_Base_Reg), ref cpu_Base_Reg);
			ser.Sync(nameof(cpu_Base_Reg_2), ref cpu_Base_Reg_2);

			ser.Sync(nameof(cpu_ALU_Reg_Dest), ref cpu_ALU_Reg_Dest);
			ser.Sync(nameof(cpu_ALU_Reg_Src), ref cpu_ALU_Reg_Src);

			ser.Sync(nameof(cpu_Mul_Cycles), ref cpu_Mul_Cycles);
			ser.Sync(nameof(cpu_Mul_Cycles_Cnt), ref cpu_Mul_Cycles_Cnt);

			ser.Sync(nameof(cpu_Instr_ARM_0), ref cpu_Instr_ARM_0);
			ser.Sync(nameof(cpu_Instr_ARM_1), ref cpu_Instr_ARM_1);
			ser.Sync(nameof(cpu_Instr_ARM_2), ref cpu_Instr_ARM_2);

			ser.Sync(nameof(cpu_Instr_TMB_0), ref cpu_Instr_TMB_0);
			ser.Sync(nameof(cpu_Instr_TMB_1), ref cpu_Instr_TMB_1);
			ser.Sync(nameof(cpu_Instr_TMB_2), ref cpu_Instr_TMB_2);

			ser.Sync(nameof(cpu_Temp_Reg), ref cpu_Temp_Reg);
			ser.Sync(nameof(cpu_Temp_Addr), ref cpu_Temp_Addr);
			ser.Sync(nameof(cpu_Temp_Data), ref cpu_Temp_Data);
			ser.Sync(nameof(cpu_Temp_Mode), ref cpu_Temp_Mode);
			ser.Sync(nameof(cpu_Bit_To_Check), ref cpu_Bit_To_Check);
			ser.Sync(nameof(cpu_Write_Back_Addr), ref cpu_Write_Back_Addr);
			ser.Sync(nameof(cpu_Addr_Offset), ref cpu_Addr_Offset);
			ser.Sync(nameof(cpu_Last_Bus_Value), ref cpu_Last_Bus_Value);
			ser.Sync(nameof(cpu_Last_Bus_Value_Old), ref cpu_Last_Bus_Value_Old);

			ser.Sync(nameof(cpu_ALU_Temp_Val), ref cpu_ALU_Temp_Val);
			ser.Sync(nameof(cpu_ALU_Temp_S_Val), ref cpu_ALU_Temp_S_Val);
			ser.Sync(nameof(cpu_ALU_Shift_Carry), ref cpu_ALU_Shift_Carry);

			ser.Sync(nameof(cpu_Instr_Type), ref cpu_Instr_Type);
			ser.Sync(nameof(cpu_LDM_Glitch_Instr_Type), ref cpu_LDM_Glitch_Instr_Type);
			ser.Sync(nameof(cpu_Exception_Type), ref cpu_Exception_Type);
			ser.Sync(nameof(cpu_Next_Load_Store_Type), ref cpu_Next_Load_Store_Type);

			ser.Sync(nameof(cpu_Thumb_Mode), ref cpu_Thumb_Mode);
			ser.Sync(nameof(cpu_ARM_Cond_Passed), ref cpu_ARM_Cond_Passed);
			ser.Sync(nameof(cpu_Seq_Access), ref cpu_Seq_Access);
			ser.Sync(nameof(cpu_IRQ_Input), ref cpu_IRQ_Input);
			ser.Sync(nameof(cpu_IRQ_Input_Use), ref cpu_IRQ_Input_Use);
			ser.Sync(nameof(cpu_Next_IRQ_Input), ref cpu_Next_IRQ_Input);
			ser.Sync(nameof(cpu_Next_IRQ_Input_2), ref cpu_Next_IRQ_Input_2);
			ser.Sync(nameof(cpu_Next_IRQ_Input_3), ref cpu_Next_IRQ_Input_3);
			ser.Sync(nameof(cpu_Is_Paused), ref cpu_Is_Paused);
			ser.Sync(nameof(cpu_Take_Branch), ref cpu_Take_Branch);
			ser.Sync(nameof(cpu_LS_Is_Load), ref cpu_LS_Is_Load);
			ser.Sync(nameof(cpu_LS_First_Access), ref cpu_LS_First_Access);
			ser.Sync(nameof(cpu_Internal_Save_Access), ref cpu_Internal_Save_Access);
			ser.Sync(nameof(cpu_Invalidate_Pipeline), ref cpu_Invalidate_Pipeline);
			ser.Sync(nameof(cpu_Overwrite_Base_Reg), ref cpu_Overwrite_Base_Reg);
			ser.Sync(nameof(cpu_Multi_Before), ref cpu_Multi_Before);
			ser.Sync(nameof(cpu_Multi_Inc), ref cpu_Multi_Inc);
			ser.Sync(nameof(cpu_Multi_S_Bit), ref cpu_Multi_S_Bit);
			ser.Sync(nameof(cpu_ALU_S_Bit), ref cpu_ALU_S_Bit);
			ser.Sync(nameof(cpu_Multi_Swap), ref cpu_Multi_Swap);
			ser.Sync(nameof(cpu_Sign_Extend_Load), ref cpu_Sign_Extend_Load);
			ser.Sync(nameof(cpu_Dest_Is_R15), ref cpu_Dest_Is_R15);
			ser.Sync(nameof(cpu_Swap_Store), ref cpu_Swap_Store);
			ser.Sync(nameof(cpu_Swap_Lock), ref cpu_Swap_Lock);
			ser.Sync(nameof(cpu_Clear_Pipeline), ref cpu_Clear_Pipeline);
			ser.Sync(nameof(cpu_Special_Inc), ref cpu_Special_Inc);
			ser.Sync(nameof(cpu_FlagI_Old), ref cpu_FlagI_Old);

			// ARM related
			ser.Sync(nameof(cpu_Exec_ARM), ref cpu_Exec_ARM);

			// Thumb related
			ser.Sync(nameof(cpu_Exec_TMB), ref cpu_Exec_TMB);

			// regs
			ser.Sync(nameof(cpu_user_R8), ref cpu_user_R8); 
			ser.Sync(nameof(cpu_user_R9), ref cpu_user_R9); 
			ser.Sync(nameof(cpu_user_R10), ref cpu_user_R10); 
			ser.Sync(nameof(cpu_user_R11), ref cpu_user_R11); 
			ser.Sync(nameof(cpu_user_R12), ref cpu_user_R12); 
			ser.Sync(nameof(cpu_user_R13), ref cpu_user_R13); 
			ser.Sync(nameof(cpu_user_R14), ref cpu_user_R14); 

			ser.Sync(nameof(cpu_spr_R13), ref cpu_spr_R13); 
			ser.Sync(nameof(cpu_spr_R14), ref cpu_spr_R14); 
			ser.Sync(nameof(cpu_spr_S), ref cpu_spr_S); 

			ser.Sync(nameof(cpu_abort_R13), ref cpu_abort_R13); 
			ser.Sync(nameof(cpu_abort_R14), ref cpu_abort_R14); 
			ser.Sync(nameof(cpu_abort_S), ref cpu_abort_S); 

			ser.Sync(nameof(cpu_undf_R13), ref cpu_undf_R13); 
			ser.Sync(nameof(cpu_undf_R14), ref cpu_undf_R14); 
			ser.Sync(nameof(cpu_undf_S), ref cpu_undf_S); 

			ser.Sync(nameof(cpu_intr_R13), ref cpu_intr_R13); 
			ser.Sync(nameof(cpu_intr_R14), ref cpu_intr_R14); 
			ser.Sync(nameof(cpu_intr_S), ref cpu_intr_S); 

			ser.Sync(nameof(cpu_fiq_R8), ref cpu_fiq_R8); 
			ser.Sync(nameof(cpu_fiq_R9), ref cpu_fiq_R9); 
			ser.Sync(nameof(cpu_fiq_R10), ref cpu_fiq_R10); 
			ser.Sync(nameof(cpu_fiq_R11), ref cpu_fiq_R11); 
			ser.Sync(nameof(cpu_fiq_R12), ref cpu_fiq_R12); 
			ser.Sync(nameof(cpu_fiq_R13), ref cpu_fiq_R13); 
			ser.Sync(nameof(cpu_fiq_R14), ref cpu_fiq_R14); 
			ser.Sync(nameof(cpu_fiq_S), ref cpu_fiq_S); 

			ser.Sync(nameof(cpu_Regs), ref cpu_Regs, false);

			// Other
			ser.Sync(nameof(stopped), ref stopped);

			ser.Sync(nameof(cpu_Trigger_Unhalt), ref cpu_Trigger_Unhalt);
			ser.Sync(nameof(cpu_Trigger_Unhalt_2), ref cpu_Trigger_Unhalt_2);
			ser.Sync(nameof(cpu_Trigger_Unhalt_3), ref cpu_Trigger_Unhalt_3);
		}
	}
}