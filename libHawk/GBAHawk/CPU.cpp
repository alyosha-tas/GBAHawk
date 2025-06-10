#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "GBA_System.h"

namespace GBAHawk
{
	void GBA_System::cpu_Reset()
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

		cpu_Exception_Type = cpu_Next_Load_Store_Type = 0;

		ResetRegisters();

		FrameCycle = 0;

		cpu_Seq_Access = cpu_IRQ_Input = cpu_IRQ_Input_Use = cpu_Is_Paused = cpu_Take_Branch = false;

		cpu_No_IRQ_Clock = cpu_Restore_IRQ_Clock = false;

		cpu_Next_IRQ_Input = cpu_Next_IRQ_Input_2 = cpu_Next_IRQ_Input_3 = false;

		cpu_LS_Is_Load = cpu_LS_First_Access = cpu_Internal_Save_Access = cpu_Invalidate_Pipeline = false;

		cpu_Overwrite_Base_Reg = cpu_Multi_Before = cpu_Multi_Inc = cpu_Multi_S_Bit = cpu_Multi_Swap = false;

		cpu_ALU_S_Bit = cpu_Sign_Extend_Load = cpu_Dest_Is_R15 = false;

		cpu_Swap_Store = cpu_Swap_Lock = cpu_Clear_Pipeline = cpu_Special_Inc = false;

		cpu_FlagI_Old = cpu_LDM_Glitch_Mode = cpu_LDM_Glitch_Store = false;

		stopped = false;

		cpu_Trigger_Unhalt = cpu_Trigger_Unhalt_2 = cpu_Trigger_Unhalt_3 = cpu_Trigger_Unhalt_4 = false;
	}

	void GBA_System::cpu_Decode_ARM()
	{
		switch ((cpu_Instr_ARM_2 >> 25) & 7)
		{
		case 0:
			if ((cpu_Instr_ARM_2 & 0x90) == 0x90)
			{
				// miscellaneous
				if (((cpu_Instr_ARM_2 & 0xF0) == 0x90))
				{
					switch ((cpu_Instr_ARM_2 >> 22) & 0x7)
					{
						case 0x0:
						case 0x1:
							// Multiply
							cpu_Instr_Type = cpu_Multiply_ARM;
							cpu_Exec_ARM = cpu_ARM_MUL;
							cpu_Calculate_Mul_Cycles();
							break;

						case 0x2:
							// Multiply Long - Unsigned
							cpu_Instr_Type = cpu_Multiply_ARM;
							cpu_Exec_ARM = cpu_ARM_MUL_UL;
							cpu_Calculate_Mul_Cycles_UL();
							break;
						case 0x3:
							// Multiply Long - Signed
							cpu_Instr_Type = cpu_Multiply_ARM;
							cpu_Exec_ARM = cpu_ARM_MUL_SL;
							cpu_Calculate_Mul_Cycles_SL();
							break;

						case 0x4:
						case 0x5:
						case 0x6:
						case 0x7:
							// Swap
							cpu_Instr_Type = cpu_Prefetch_Swap_ARM;
							cpu_Next_Load_Store_Type = cpu_Swap_ARM;
							cpu_Exec_ARM = cpu_ARM_Swap;
							cpu_Swap_Store = false;
							break;
					}
				}
				else
				{
					// halfword or byte transfers
					cpu_Exec_ARM = cpu_ARM_Imm_LS;
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

					switch ((cpu_Instr_ARM_2 >> 5) & 0x3)
					{
						// 0 case is not a load store instruction
						case 0x1:
							// Unsigned halfword
							cpu_Next_Load_Store_Type = cpu_Load_Store_Half_ARM;
							cpu_Sign_Extend_Load = false;
							break;

						case 0x2:
							// Signed Byte
							cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
							cpu_Sign_Extend_Load = true;
							break;
						case 0x3:
							// Signed halfword
							cpu_Next_Load_Store_Type = cpu_Load_Store_Half_ARM;
							cpu_Sign_Extend_Load = true;
							break;
					}

					if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
					{
						cpu_Addr_Offset = ((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF);
					}
					else
					{
						cpu_Addr_Offset = cpu_Regs[cpu_Instr_ARM_2 & 0xF];
					}
				}
			}
			else
			{
				// ALU ops
				cpu_ALU_S_Bit = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

				cpu_ALU_Reg_Dest = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

				cpu_ALU_Reg_Src = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);
				cpu_Temp_Reg = cpu_Regs[cpu_ALU_Reg_Src];

				// slightly different code path for R15 as destination, since it's closer to a branch
				cpu_Dest_Is_R15 = (cpu_ALU_Reg_Dest == 15);

				switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
				{
					case 0x0: cpu_Exec_ARM = cpu_ARM_AND; cpu_Clear_Pipeline = true; break;
					case 0x1: cpu_Exec_ARM = cpu_ARM_EOR; cpu_Clear_Pipeline = true; break;
					case 0x2: cpu_Exec_ARM = cpu_ARM_SUB; cpu_Clear_Pipeline = true; break;
					case 0x3: cpu_Exec_ARM = cpu_ARM_RSB; cpu_Clear_Pipeline = true; break;
					case 0x4: cpu_Exec_ARM = cpu_ARM_ADD; cpu_Clear_Pipeline = true; break;
					case 0x5: cpu_Exec_ARM = cpu_ARM_ADC; cpu_Clear_Pipeline = true; break;
					case 0x6: cpu_Exec_ARM = cpu_ARM_SBC; cpu_Clear_Pipeline = true; break;
					case 0x7: cpu_Exec_ARM = cpu_ARM_RSC; cpu_Clear_Pipeline = true; break;
					case 0x8: cpu_Exec_ARM = cpu_ARM_TST; cpu_Clear_Pipeline = false; break;
					case 0x9: cpu_Exec_ARM = cpu_ARM_TEQ; cpu_Clear_Pipeline = false; break;
					case 0xA: cpu_Exec_ARM = cpu_ARM_CMP; cpu_Clear_Pipeline = false; break;
					case 0xB: cpu_Exec_ARM = cpu_ARM_CMN; cpu_Clear_Pipeline = false; break;
					case 0xC: cpu_Exec_ARM = cpu_ARM_ORR; cpu_Clear_Pipeline = true; break;
					case 0xD: cpu_Exec_ARM = cpu_ARM_MOV; cpu_Clear_Pipeline = true; break;
					case 0xE: cpu_Exec_ARM = cpu_ARM_BIC; cpu_Clear_Pipeline = true; break;
					case 0xF: cpu_Exec_ARM = cpu_ARM_MVN; cpu_Clear_Pipeline = true; break;
				}

				// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
				cpu_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

				bool is_RRX = false;

				cpu_ALU_Long_Result = cpu_Regs[cpu_Instr_ARM_2 & 0xF];

				if ((cpu_Instr_ARM_2 & 0x10) != 0x0)
				{
					// if the pc is the shifted value or operand, and its a register shift, it is the incremented value that is used
					if ((cpu_Instr_ARM_2 & 0xF) == 15)
					{
						cpu_ALU_Long_Result += 4; cpu_ALU_Long_Result &= cpu_Cast_Int;
					}

					if (cpu_ALU_Reg_Src == 15)
					{
						cpu_Temp_Reg += 4;
					}

					// register shifts take an extra cycle
					if (cpu_Instr_Type == cpu_Internal_And_Prefetch_ARM) { cpu_Instr_Type = cpu_Internal_And_Prefetch_2_ARM; }
					if (cpu_Instr_Type == cpu_Internal_And_Branch_2_ARM) { cpu_Instr_Type = cpu_Internal_And_Branch_3_ARM; }
				}

				cpu_ALU_Shift_Carry = (uint32_t)(cpu_FlagCget() ? 1 : 0);

				switch ((cpu_Instr_ARM_2 >> 5) & 3)
				{
				case 0:         // LSL
					// calculate immedaite value
					if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
					{
						// immediate shift
						cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);
					}
					else
					{
						// register shift
						cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
					}

					cpu_ALU_Long_Result = cpu_ALU_Long_Result << cpu_Shift_Imm;

					if (cpu_Shift_Imm != 0)
					{
						cpu_ALU_Shift_Carry = (uint32_t)((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare ? 1 : 0);
					}
					break;

				case 1:         // LSR
					// calculate immedaite value
					if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
					{
						// immediate shift
						cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

						if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
					}
					else
					{
						// register shift
						cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
					}

					if (cpu_Shift_Imm != 0)
					{
						cpu_ALU_Shift_Carry = (uint32_t)((cpu_ALU_Long_Result >> (cpu_Shift_Imm - 1)) & 1);
						cpu_ALU_Long_Result = cpu_ALU_Long_Result >> cpu_Shift_Imm;
					}
					break;

				case 2:         // ASR
					// calculate immedaite value
					if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
					{
						// immediate shift
						cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

						if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
					}
					else
					{
						// register shift
						cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
					}

					cpu_ALU_Temp_S_Val = (uint32_t)(cpu_ALU_Long_Result & cpu_Neg_Compare);

					for (int i = 1; i <= cpu_Shift_Imm; i++)
					{
						cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
						cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
						cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
					}
					break;

				case 3:         // RRX
					// calculate immedaite value
					if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
					{
						// immediate shift
						cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

						if (cpu_Shift_Imm == 0) { is_RRX = true; }
					}
					else
					{
						// register shift
						cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
					}

					if (is_RRX)
					{
						cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
						cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
						cpu_ALU_Long_Result |= cpu_FlagCget() ? 0x80000000 : 0;
					}
					else
					{
						for (int i = 1; i <= cpu_Shift_Imm; i++)
						{
							cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
							cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
							cpu_ALU_Long_Result |= (cpu_ALU_Shift_Carry << 31);
						}
					}
					break;
				}

				cpu_ALU_Temp_Val = (uint32_t)cpu_ALU_Long_Result;

				// overwrite certain instructions
				if (!cpu_ALU_S_Bit)
				{
					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
						case 0x8:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_ARM;
							cpu_Exec_ARM = cpu_ARM_MRS;
							break;
						case 0x9:
							if ((cpu_Instr_ARM_2 & 0xFFF90) == 0xFFF10)
							{
								// Branch and exchange
								cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_ARM;
								cpu_Exec_ARM = cpu_ARM_Bx;
							}
							else
							{
								cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
								cpu_Exec_ARM = cpu_ARM_MSR;
							}
							break;
						case 0xA:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_ARM;
							cpu_Exec_ARM = cpu_ARM_MRS;
							break;
						case 0xB:
							if ((cpu_Instr_ARM_2 & 0xFFF90) == 0xFFF10)
							{
								// Branch and exchange
								cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_ARM;
								cpu_Exec_ARM = cpu_ARM_Bx;
							}
							else
							{
								cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
								cpu_Exec_ARM = cpu_ARM_MSR;
							}
							break;
					}
				}
			}
			break;

		case 1:
			// ALU ops
			cpu_ALU_S_Bit = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

			cpu_ALU_Reg_Dest = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

			cpu_ALU_Reg_Src = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);
			cpu_Temp_Reg = cpu_Regs[cpu_ALU_Reg_Src];

			// slightly different code path for R15 as destination, since it's closer to a branch
			cpu_Dest_Is_R15 = (cpu_ALU_Reg_Dest == 15);

			switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
			{
				case 0x0: cpu_Exec_ARM = cpu_ARM_AND; cpu_Clear_Pipeline = true; break;
				case 0x1: cpu_Exec_ARM = cpu_ARM_EOR; cpu_Clear_Pipeline = true; break;
				case 0x2: cpu_Exec_ARM = cpu_ARM_SUB; cpu_Clear_Pipeline = true; break;
				case 0x3: cpu_Exec_ARM = cpu_ARM_RSB; cpu_Clear_Pipeline = true; break;
				case 0x4: cpu_Exec_ARM = cpu_ARM_ADD; cpu_Clear_Pipeline = true; break;
				case 0x5: cpu_Exec_ARM = cpu_ARM_ADC; cpu_Clear_Pipeline = true; break;
				case 0x6: cpu_Exec_ARM = cpu_ARM_SBC; cpu_Clear_Pipeline = true; break;
				case 0x7: cpu_Exec_ARM = cpu_ARM_RSC; cpu_Clear_Pipeline = true; break;
				case 0x8: cpu_Exec_ARM = cpu_ARM_TST; cpu_Clear_Pipeline = false; break;
				case 0x9: cpu_Exec_ARM = cpu_ARM_TEQ; cpu_Clear_Pipeline = false; break;
				case 0xA: cpu_Exec_ARM = cpu_ARM_CMP; cpu_Clear_Pipeline = false; break;
				case 0xB: cpu_Exec_ARM = cpu_ARM_CMN; cpu_Clear_Pipeline = false; break;
				case 0xC: cpu_Exec_ARM = cpu_ARM_ORR; cpu_Clear_Pipeline = true; break;
				case 0xD: cpu_Exec_ARM = cpu_ARM_MOV; cpu_Clear_Pipeline = true; break;
				case 0xE: cpu_Exec_ARM = cpu_ARM_BIC; cpu_Clear_Pipeline = true; break;
				case 0xF: cpu_Exec_ARM = cpu_ARM_MVN; cpu_Clear_Pipeline = true; break;
			}

			// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
			cpu_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

			// calculate immedaite value
			cpu_ALU_Temp_Val = cpu_Instr_ARM_2 & 0xFF;

			cpu_ALU_Shift_Carry = (uint32_t)(cpu_FlagCget() ? 1 : 0);

			// Note: the shift val is multiplied by 2 (so only shift by 7 below)
			for (int i = 1; i <= ((cpu_Instr_ARM_2 >> 7) & 0x1E); i++)
			{
				cpu_ALU_Shift_Carry = cpu_ALU_Temp_Val & 1;
				cpu_ALU_Temp_Val = (cpu_ALU_Temp_Val >> 1) | (cpu_ALU_Shift_Carry << 31);
			}

			// overwrite certain instructions
			if (!cpu_ALU_S_Bit)
			{
				switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
				{
					case 0x8:
						// Gltiched version of MSR
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR_Glitchy;
						break;
					case 0x9:
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR;
						break;
					case 0xA:
						// Gltiched version of MSR
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR_Glitchy;
						break;
					case 0xB:
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR;
						break;
				}
			}
			break;

		case 2:
			// load / store immediate offset
			cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

			if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
			{
				cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
				cpu_Sign_Extend_Load = false;
			}
			else
			{
				cpu_Next_Load_Store_Type = cpu_Load_Store_Word_ARM;
			}

			cpu_Exec_ARM = cpu_ARM_Imm_LS;
			cpu_Addr_Offset = cpu_Instr_ARM_2 & 0xFFF;
			break;

		case 3:
			if ((cpu_Instr_ARM_2 & 0x10) == 0)
			{
				// load / store register offset
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

				if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
					cpu_Sign_Extend_Load = false;
				}
				else
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_ARM;
				}

				cpu_Exec_ARM = cpu_ARM_Reg_LS;
			}
			else
			{
				// Undefined Opcode Exception
				cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
				cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
				cpu_Exception_Type = cpu_Undef_Exc;
			}
			break;

		case 4:
			// block transfer
			cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;
			cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_ARM;
			cpu_Exec_ARM = cpu_ARM_Multi_1;
			break;

		case 5:
			// branch
			cpu_Instr_Type = cpu_Internal_And_Branch_1_ARM;
			cpu_Exec_ARM = cpu_ARM_Branch;
			break;

		case 6:
			// Coprocessor Instruction (treat as Undefined Opcode Exception)
			cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
			cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
			cpu_Exception_Type = cpu_Undef_Exc;
			break;

		case 7:
			if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
			{
				// software interrupt
				cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
				cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
				cpu_Exception_Type = cpu_SWI_Exc;
			}
			else
			{
				// Coprocessor Instruction (treat as Undefined Opcode Exception)
				cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
				cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
				cpu_Exception_Type = cpu_Undef_Exc;
			}
			break;

		default:
			cpu_Instr_Type = cpu_Internal_Halted;
			break;
		}

		if (TraceCallback) TraceCallback(0);
	}

	void GBA_System::cpu_Decode_TMB()
	{
		switch ((cpu_Instr_TMB_2 >> 13) & 7)
		{
		case 0:
			// shift / add / sub
			if ((cpu_Instr_TMB_2 & 0x1800) == 0x1800)
			{
				cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
				cpu_Exec_TMB = cpu_Thumb_Add_Sub_Reg;
			}
			else
			{
				cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
				cpu_Exec_TMB = cpu_Thumb_Shift;
			}
			break;

		case 1:
			// data ops (immedaite)
			cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
			cpu_Exec_TMB = cpu_Thumb_ALU_Imm;
			break;

		case 2:
			if ((cpu_Instr_TMB_2 & 0x1000) == 0x0)
			{
				if ((cpu_Instr_TMB_2 & 0x800) == 0x0)
				{
					if ((cpu_Instr_TMB_2 & 0x400) == 0x0)
					{
						// ALU Ops
						// shifts in this path take an extra cycle, as the shift length is register defined
						cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

						cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

						switch ((cpu_Instr_TMB_2 >> 6) & 0xF)
						{
							case 0x0: cpu_Exec_TMB = cpu_Thumb_AND; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x1: cpu_Exec_TMB = cpu_Thumb_EOR; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x2: cpu_Exec_TMB = cpu_Thumb_LSL; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x3: cpu_Exec_TMB = cpu_Thumb_LSR; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x4: cpu_Exec_TMB = cpu_Thumb_ASR; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x5: cpu_Exec_TMB = cpu_Thumb_ADC; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x6: cpu_Exec_TMB = cpu_Thumb_SBC; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x7: cpu_Exec_TMB = cpu_Thumb_ROR; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x8: cpu_Exec_TMB = cpu_Thumb_TST; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x9: cpu_Exec_TMB = cpu_Thumb_NEG; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xA: cpu_Exec_TMB = cpu_Thumb_CMP; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xB: cpu_Exec_TMB = cpu_Thumb_CMN; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xC: cpu_Exec_TMB = cpu_Thumb_ORR; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xD: cpu_Exec_TMB = cpu_Thumb_MUL; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xE: cpu_Exec_TMB = cpu_Thumb_BIC; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xF: cpu_Exec_TMB = cpu_Thumb_MVN; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
						}

						// special case for multiply
						if (((cpu_Instr_TMB_2 >> 6) & 0xF) == 0xD)
						{
							cpu_Instr_Type = cpu_Multiply_TMB;
							cpu_Calculate_Mul_Cycles();
						}
					}
					else
					{
						// High Regs / Branch and exchange
						switch ((cpu_Instr_TMB_2 >> 8) & 3)
						{
							case 0:
								cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
								cpu_Exec_TMB = cpu_Thumb_High_Add;
								break;

							case 1:
								cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
								cpu_Exec_TMB = cpu_Thumb_High_Cmp;
								break;

							case 2:
								if (((cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8)) == 0xF)
								{
									cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
								}
								else
								{
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
								}

								cpu_Exec_TMB = cpu_Thumb_High_MOV;
								break;

							case 3:
								// Decodes correctly even if bit 7 is set
								cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_TMB;
								cpu_Exec_TMB = cpu_Thumb_High_Bx;
								break;
						}
					}
				}
				else
				{
					// PC relative load
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;
					cpu_Exec_TMB = cpu_Thumb_PC_Rel_LS;
				}
			}
			else
			{
				// Load / store Relative offset
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
				cpu_Sign_Extend_Load = false;

				switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
				{
					case 0: cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB; break;
					case 1: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; break;
					case 2: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; break;
					case 3: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; break;
					case 4: cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB; break;
					case 5: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; break;
					case 6: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; break;
					case 7: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; break;
				}

				cpu_Exec_TMB = cpu_Thumb_Rel_LS;
			}
			break;

		case 3:
			// Load / store Immediate offset
			cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;

			if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
			{
				cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB;
				cpu_Sign_Extend_Load = false;
			}
			else
			{
				cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;
			}

			cpu_Exec_TMB = cpu_Thumb_Imm_LS;
			break;

		case 4:
			if ((cpu_Instr_TMB_2 & 0x1000) == 0)
			{
				// Load / store half word
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
				cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB;
				cpu_Sign_Extend_Load = false;
				cpu_Exec_TMB = cpu_Thumb_Half_LS;
			}
			else
			{
				// SP relative load store
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
				cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;
				cpu_Exec_TMB = cpu_Thumb_SP_REL_LS;
			}
			break;

		case 5:
			if ((cpu_Instr_TMB_2 & 0x1000) == 0)
			{
				// Load Address
				cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
				cpu_Exec_TMB = cpu_Thumb_Add_SP_PC;
			}
			else
			{
				if ((cpu_Instr_TMB_2 & 0xF00) == 0x0)
				{
					// Add offset to stack
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
					cpu_Exec_TMB = cpu_Thumb_Add_Sub_Stack;
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0x600) == 0x400)
					{
						// Push / Pop
						cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
						cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_TMB;
						cpu_Exec_TMB = cpu_Thumb_Push_Pop;
					}
					else
					{
						// Undefined Opcode Exception
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exception_Type = cpu_Undef_Exc;
					}
				}
			}
			break;

		case 6:
			if ((cpu_Instr_TMB_2 & 0x1000) == 0)
			{
				// Multiple Load/Store
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
				cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_TMB;
				cpu_Exec_TMB = cpu_Thumb_Multi_1;
			}
			else
			{
				if ((cpu_Instr_TMB_2 & 0xF00) == 0xF00)
				{
					// Software Interrupt
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exception_Type = cpu_SWI_Exc;
				}
				else if ((cpu_Instr_TMB_2 & 0xE00) == 0xE00)
				{
					// Undefined instruction
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				else
				{
					// Conditional Branch
					cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
					cpu_Exec_TMB = cpu_Thumb_Branch_Cond;
				}
			}

			break;

		case 7:
			if ((cpu_Instr_TMB_2 & 0x1000) == 0)
			{
				if ((cpu_Instr_TMB_2 & 0x800) == 0)
				{
					// Unconditional branch
					cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
					cpu_Exec_TMB = cpu_Thumb_Branch;
				}
				else
				{
					// Undefined Opcode Exception
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
			}
			else
			{
				// Branch with link
				if ((cpu_Instr_TMB_2 & 0x800) == 0)
				{
					// A standard data operation assigning the upper part of the branch
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
					cpu_Exec_TMB = cpu_Thumb_Branch_Link_1;
				}
				else
				{
					// Actual branch operation (can it occur without the first one?)
					cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
					cpu_Exec_TMB = cpu_Thumb_Branch_Link_2;
				}
			}
			break;

		default:
			cpu_Instr_Type = cpu_Internal_Halted;
			break;
		}

		if (TraceCallback) TraceCallback(0);
	}

	void GBA_System::cpu_Execute_Internal_Only_ARM()
	{
		// Do Tracer Stuff here
		int ofst = 0;

		// local variables not stated (evaluated each time)
		bool Use_Reg_15 = false;
		uint32_t byte_mask = 0;
		uint32_t total_mask = 0;
		int64_t s1 = 0;
		int64_t s2 = 0;
		int64_t sf1 = 0;
		int64_t sf2 = 0;

		// Condition code check
		if (cpu_ARM_Condition_Check())
		{
			cpu_ARM_Cond_Passed = true;

			// Do Tracer stuff here
			switch (cpu_Exec_ARM)
			{
				case cpu_ARM_AND:
					cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_EOR:
					cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_SUB:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_RSB:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= cpu_Temp_Reg;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_ADD:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_ADC:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result += (uint64_t)(cpu_FlagCget() ? 1 : 0);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_SBC:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_RSC:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= cpu_Temp_Reg;
					cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_TST:
					cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					break;

				case cpu_ARM_TEQ:
					cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					break;

				case cpu_ARM_CMP:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}
					break;

				case cpu_ARM_CMN:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}
					break;

				case cpu_ARM_ORR:
					cpu_ALU_Long_Result = cpu_Temp_Reg | cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MOV:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_BIC:
					cpu_ALU_Long_Result = cpu_Temp_Reg & (~cpu_ALU_Temp_Val);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MVN:
					cpu_ALU_Long_Result = ~cpu_ALU_Temp_Val;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MSR:
					if ((cpu_Instr_ARM_2 & 0x10000) == 0x10000) { byte_mask |= 0x000000FF; }
					if ((cpu_Instr_ARM_2 & 0x20000) == 0x20000) { byte_mask |= 0x0000FF00; }
					if ((cpu_Instr_ARM_2 & 0x40000) == 0x40000) { byte_mask |= 0x00FF0000; }
					if ((cpu_Instr_ARM_2 & 0x80000) == 0x80000) { byte_mask |= 0xFF000000; }

					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						// user (unpriviliged)
						if ((cpu_Regs[16] & 0x1F) == 0x10)
						{
							total_mask = byte_mask & cpu_User_Mask;
						}
						else
						{
							if ((cpu_ALU_Temp_Val & cpu_State_Mask) != 0)
							{
								// architecturally unpredictable, but on hardwarae has no ill effects (ex. feline.gba transparency when seen by lab rat.)
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
							else
							{
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
						}

						//upper bit of mode must always be set
						cpu_ALU_Temp_Val |= 0x10;

						cpu_Swap_Regs(((cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask)) & 0x1F, false, false);
						cpu_Regs[16] = (cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
					}
					else
					{
						//upper bit of mode must always be set for spsr as well
						cpu_ALU_Temp_Val |= 0x10;
						
						// user and system have no SPSR to write to
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							// unpredictable
						}
						else
						{
							total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask | cpu_State_Mask);
							cpu_Regs[17] = (cpu_Regs[17] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
						}
					}
					break;

				case cpu_ARM_MSR_Glitchy:
					if ((cpu_Instr_ARM_2 & 0x10000) == 0x10000) { byte_mask |= 0x000000FF; }
					if ((cpu_Instr_ARM_2 & 0x20000) == 0x20000) { byte_mask |= 0x0000FF00; }
					if ((cpu_Instr_ARM_2 & 0x40000) == 0x40000) { byte_mask |= 0x00FF0000; }
					if ((cpu_Instr_ARM_2 & 0x80000) == 0x80000) { byte_mask |= 0xFF000000; }

					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						// user (unpriviliged)
						if ((cpu_Regs[16] & 0x1F) == 0x10)
						{
							total_mask = byte_mask & cpu_User_Mask;
						}
						else
						{
							if ((cpu_ALU_Temp_Val & cpu_State_Mask) != 0)
							{
								// architecturally unpredictable, but on hardwarae has no ill effects (ex. feline.gba transparency when seen by lab rat.)
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
							else
							{
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
						}

						//upper bit of mode must always be set
						cpu_ALU_Temp_Val |= 0x10;

						cpu_Swap_Regs(((cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask)) & 0x1F, false, false);
						cpu_Regs[16] = (cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
					}
					else
					{
						//upper bit of mode must always be set for spsr as well
						cpu_ALU_Temp_Val |= 0x10;

						// user and system have no SPSR to write to
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							// unpredictable
						}
						else
						{
							total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask | cpu_State_Mask);
							cpu_Regs[17] = (cpu_Regs[17] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
						}
					}
					break;

				case cpu_ARM_MRS:
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[16];
					}
					else
					{
						// user and system have no SPSR to read from
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[16];
						}
						else
						{
							cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[17];
						}
					}
					break;

				case cpu_ARM_Bx:
					// Branch and exchange mode (possibly)
					cpu_Base_Reg = (uint32_t)(cpu_Instr_ARM_2 & 0xF);

					cpu_Temp_Reg = cpu_Regs[cpu_Base_Reg];
					break;

				case cpu_ARM_MUL:
					cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
					cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						cpu_ALU_Long_Result += cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
					}

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					// mul does not effect R15
					if (((cpu_Instr_ARM_2 >> 16) & 0xF) != 0xF)
					{
						cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					}
					break;

				case cpu_ARM_MUL_UL:
					cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
					cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{

						uint64_t a1 = cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
						uint64_t a2 = cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF];
						a2 = a2 << 32;
						cpu_ALU_Long_Result += (a1 + a2);
					}

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);

						// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1
						// then it does not change (probably multiplier takes a short cut for these cases)
						// and changes differently if it is -1
						sf1 = (int32_t)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						sf2 = (int32_t)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
						if (sf2 == -1)
						{
							// maybe its a carry from lower 16 bits)???
							if ((sf1 & 0xFFFF) >= 2) { cpu_FlagCset(true); }
							else { cpu_FlagCset(false); }
						}
						else if ((sf2 != 0) && (sf2 != 1))
						{
							cpu_FlagCset(((sf1 & 0x80000000) == 0x80000000));
						}
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)(cpu_ALU_Long_Result >> 32);
					break;

				case cpu_ARM_MUL_SL:
					s1 = (int32_t)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
					s2 = (int32_t)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

					cpu_ALU_Signed_Long_Result = s1 * s2;

					cpu_ALU_Long_Result = (uint64_t)cpu_ALU_Signed_Long_Result;

					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						uint64_t a1 = cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
						uint64_t a2 = cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF];
						a2 = a2 << 32;
						cpu_ALU_Long_Result += (a1 + a2);
					}

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);

						// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1,-1
						// then it does not change (probably multiplier takes a short cut for these cases)
						if ((s2 != 0) && (s2 != 1) && (s2 != -1))
						{
							cpu_FlagCset(!((s1 & 0x80000000) == (s2 & 0x80000000)));
						}
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)(cpu_ALU_Long_Result >> 32);
					break;

				case cpu_ARM_Swap:
					cpu_Base_Reg_2 = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);
					cpu_Base_Reg = cpu_Regs[(uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF)];
					cpu_Temp_Reg_Ptr = (uint32_t)(cpu_Instr_ARM_2 & 0xF);
					break;

				case cpu_ARM_Imm_LS:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg_Ptr = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// increment first
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Addr_Offset);
						}
						else
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] - cpu_Addr_Offset);
						}
					}
					else
					{
						// increment last
						cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];
					}

					cpu_Overwrite_Base_Reg = false;

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;

						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr + cpu_Addr_Offset);
						}
						else
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - cpu_Addr_Offset);
						}
					}
					else if ((cpu_Instr_ARM_2 & 0x200000) == 0x200000)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// use incremented address
					//if (cpu_Base_Reg == 15) { cpu_Write_Back_Addr += 4; }

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}

					break;

				case cpu_ARM_Reg_LS:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg_Ptr = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

					cpu_Temp_Data = cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

					switch ((cpu_Instr_ARM_2 >> 5) & 3)
					{
					case 0:         // LSL
						cpu_Temp_Data = cpu_Temp_Data << cpu_Shift_Imm;
						break;

					case 1:			// LSR
						if (cpu_Shift_Imm == 0) { cpu_Temp_Data = 0; }
						else { cpu_Temp_Data = cpu_Temp_Data >> cpu_Shift_Imm; }
						break;

					case 2:         // ASR
						if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }

						cpu_ALU_Temp_S_Val = cpu_Temp_Data & cpu_Neg_Compare;

						for (int i = 1; i <= cpu_Shift_Imm; i++)
						{
							cpu_Temp_Data = (cpu_Temp_Data >> 1);
							cpu_Temp_Data |= cpu_ALU_Temp_S_Val;
						}
						break;

					case 3:         // RRX
						if (cpu_Shift_Imm == 0)
						{
							cpu_Temp_Data = (cpu_Temp_Data >> 1);
							cpu_Temp_Data |= cpu_FlagCget() ? 0x80000000 : 0;
						}
						else
						{
							for (int i = 1; i <= cpu_Shift_Imm; i++)
							{
								cpu_ALU_Temp_S_Val = (uint32_t)(cpu_Temp_Data & 1);
								cpu_Temp_Data = (cpu_Temp_Data >> 1);
								cpu_Temp_Data |= (cpu_ALU_Temp_S_Val << 31);
							}
						}
						break;
					}

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// increment first
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Temp_Data);
						}
						else
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] - cpu_Temp_Data);
						}
					}
					else
					{
						// increment last
						cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];
					}

					cpu_Overwrite_Base_Reg = false;

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;

						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr + cpu_Temp_Data);
						}
						else
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - cpu_Temp_Data);
						}
					}
					else if ((cpu_Instr_ARM_2 & 0x200000) == 0x200000)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// use incremented address
					//if (cpu_Base_Reg == 15) { cpu_Write_Back_Addr += 4; }

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}
					break;

				case cpu_ARM_Multi_1:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

					cpu_LS_First_Access = true;

					cpu_Overwrite_Base_Reg = (cpu_Instr_ARM_2 & 0x200000) == 0x200000;

					cpu_Multi_Before = (cpu_Instr_ARM_2 & 0x1000000) == 0x1000000;

					cpu_Multi_Inc = (cpu_Instr_ARM_2 & 0x800000) == 0x800000;

					// build list of registers to access
					cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

					// need some special logic here, if the S bit is set, we swap out registers to user mode for the accesses
					// then swap back afterwards, but only if reg15 is not accessed in a load
					cpu_Multi_S_Bit = (cpu_Instr_ARM_2 & 0x400000) == 0x400000;
					cpu_Multi_Swap = false;

					for (int i = 0; i < 16; i++)
					{
						if (((cpu_Instr_ARM_2 >> i) & 1) == 1)
						{
							cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
							if ((i == 15) && cpu_LS_Is_Load) { Use_Reg_15 = true; }

							cpu_Multi_List_Size += 1;

							// The documentation gives this case as unpredictable for now copy Thumb logic
							if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
						}
					}

					cpu_Special_Inc = false;

					// No registers selected loads / stores R15 instead
					if (cpu_Multi_List_Size == 0)
					{
						cpu_Multi_List_Size = 1;
						cpu_Regs_To_Access[0] = 15;
						cpu_Special_Inc = true;
						Use_Reg_15 = true; // ?
					}

					// when decrementing, start address is at the bottom, (similar to a POP instruction)
					if (!cpu_Multi_Inc)
					{
						cpu_Temp_Addr -= (uint32_t)((cpu_Multi_List_Size - 1) * 4);

						// in either case, write back is the same
						cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - 4);

						if (cpu_Multi_Before) { cpu_Temp_Addr -= 4; }

						if (cpu_Special_Inc)
						{
							cpu_Write_Back_Addr -= 0x3C;
							cpu_Temp_Addr -= 0x3C;
							cpu_Special_Inc = false;
						}
					}
					else
					{
						if (cpu_Multi_Before) { cpu_Temp_Addr += 4; }
					}

					// swap out registers for user mode ones
					if (cpu_Multi_S_Bit && !Use_Reg_15)
					{
						cpu_Multi_Swap = true;
						cpu_Temp_Mode = cpu_Regs[16] & 0x1F;

						cpu_Swap_Regs(0x10, false, false);
					}
					break;

				case cpu_ARM_Branch:
					ofst = (uint32_t)((cpu_Instr_ARM_2 & 0xFFFFFF) << 2);

					// offset is signed
					if ((ofst & 0x2000000) == 0x2000000) { ofst = (uint32_t)(ofst | 0xFC000000); }

					cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

					// Link if link bit set
					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						cpu_Regs[14] = (uint32_t)(cpu_Regs[15] - 4);
					}

					cpu_Take_Branch = true;
					break;

				case cpu_ARM_AND_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_EOR_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_SUB_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_RSB_LDM:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= cpu_Temp_Reg;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_ADD_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					//if (cpu_LDM_Glitch_Store) { cpu_LDM_Glitch_Set_Reg((uint)cpu_ALU_Reg_Dest, (uint)cpu_ALU_Long_Result); }
					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_ADC_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result += (uint64_t)(cpu_FlagCget() ? 1 : 0);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_SBC_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_RSC_LDM:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= cpu_Temp_Reg;
					cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_TST_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					break;

				case cpu_ARM_TEQ_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					break;

				case cpu_ARM_CMP_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}
					break;

				case cpu_ARM_CMN_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}
					break;

				case cpu_ARM_ORR_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg | cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MOV_LDM:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_BIC_LDM:
					cpu_ALU_Long_Result = cpu_Temp_Reg & (~cpu_ALU_Temp_Val);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MVN_LDM:
					cpu_ALU_Long_Result = ~cpu_ALU_Temp_Val;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MSR_LDM:
					if ((cpu_Instr_ARM_2 & 0x10000) == 0x10000) { byte_mask |= 0x000000FF; }
					if ((cpu_Instr_ARM_2 & 0x20000) == 0x20000) { byte_mask |= 0x0000FF00; }
					if ((cpu_Instr_ARM_2 & 0x40000) == 0x40000) { byte_mask |= 0x00FF0000; }
					if ((cpu_Instr_ARM_2 & 0x80000) == 0x80000) { byte_mask |= 0xFF000000; }

					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						// user (unpriviliged)
						if ((cpu_Regs[16] & 0x1F) == 0x10)
						{
							total_mask = byte_mask & cpu_User_Mask;
						}
						else
						{
							if ((cpu_ALU_Temp_Val & cpu_State_Mask) != 0)
							{
								// architecturally unpredictable, but on hardwarae has no ill effects (ex. feline.gba transparency when seen by lab rat.)
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
							else
							{
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
						}

						//upper bit of mode must always be set
						cpu_ALU_Temp_Val |= 0x10;

						cpu_Swap_Regs(((cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask)) & 0x1F, false, false);
						cpu_Regs[16] = (cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
					}
					else
					{
						//upper bit of mode must always be set
						cpu_ALU_Temp_Val |= 0x10;
						
						// user and system have no SPSR to write to
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							// unpredictable
						}
						else
						{
							total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask | cpu_State_Mask);
							cpu_Regs[17] = (cpu_Regs[17] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
						}
					}
					break;

				case cpu_ARM_MSR_LDM_Glitchy:
					if ((cpu_Instr_ARM_2 & 0x10000) == 0x10000) { byte_mask |= 0x000000FF; }
					if ((cpu_Instr_ARM_2 & 0x20000) == 0x20000) { byte_mask |= 0x0000FF00; }
					if ((cpu_Instr_ARM_2 & 0x40000) == 0x40000) { byte_mask |= 0x00FF0000; }
					if ((cpu_Instr_ARM_2 & 0x80000) == 0x80000) { byte_mask |= 0xFF000000; }

					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						// user (unpriviliged)
						if ((cpu_Regs[16] & 0x1F) == 0x10)
						{
							total_mask = byte_mask & cpu_User_Mask;
						}
						else
						{
							if ((cpu_ALU_Temp_Val & cpu_State_Mask) != 0)
							{
								// architecturally unpredictable, but on hardwarae has no ill effects (ex. feline.gba transparency when seen by lab rat.)
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
							else
							{
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
						}

						//upper bit of mode must always be set
						cpu_ALU_Temp_Val |= 0x10;

						cpu_Swap_Regs(((cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask)) & 0x1F, false, false);
						cpu_Regs[16] = (cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
					}
					else
					{
						//upper bit of mode must always be set
						cpu_ALU_Temp_Val |= 0x10;

						// user and system have no SPSR to write to
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							// unpredictable
						}
						else
						{
							total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask | cpu_State_Mask);
							cpu_Regs[17] = (cpu_Regs[17] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
						}
					}
					break;

				case cpu_ARM_MRS_LDM:
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[16];
					}
					else
					{
						// user and system have no SPSR to read from
						if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
						{
							cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[16];
						}
						else
						{
							cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[17];
						}
					}
					break;

				case cpu_ARM_Bx_LDM:
					// Branch and exchange mode (possibly)
					cpu_Base_Reg = (cpu_Instr_ARM_2 & 0xF);

					cpu_Temp_Reg = cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg);
					break;

				case cpu_ARM_MUL_LDM:
					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
						cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

						cpu_ALU_Long_Result += cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 12) & 0xF);
					}
					else
					{
						cpu_ALU_Long_Result = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF);
						cpu_ALU_Long_Result *= cpu_LDM_Glitch_Get_Reg(cpu_Instr_ARM_2 & 0xF);
					}

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					// mul does not effect R15
					if (((cpu_Instr_ARM_2 >> 16) & 0xF) != 0xF)
					{
						cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					}
					break;

				case cpu_ARM_MUL_UL_LDM:
					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
						cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

						uint64_t a1 = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 12) & 0xF);
						uint64_t a2 = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 16) & 0xF);

						a2 = a2 << 32;
						cpu_ALU_Long_Result += (a1 + a2);
					}
					else
					{
						cpu_ALU_Long_Result = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF);
						cpu_ALU_Long_Result *= cpu_LDM_Glitch_Get_Reg(cpu_Instr_ARM_2 & 0xF);
					}

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);

						// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1
						// then it does not change (probably multiplier takes a short cut for these cases)
						// and changes differently if it is -1
						sf1 = (int32_t)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						sf2 = (int32_t)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
						if (sf2 == -1)
						{
							// maybe its a carry from lower 16 bits)???
							if ((sf1 & 0xFFFF) >= 2) { cpu_FlagCset(true); }
							else { cpu_FlagCset(false); }
						}
						else if ((sf2 != 0) && (sf2 != 1))
						{
							cpu_FlagCset(((sf1 & 0x80000000) == 0x80000000));
						}
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)(cpu_ALU_Long_Result >> 32);
					break;

				case cpu_ARM_MUL_SL_LDM:
					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						s1 = (int32_t)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						s2 = (int32_t)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

						cpu_ALU_Signed_Long_Result = s1 * s2;

						cpu_ALU_Long_Result = (uint64_t)cpu_ALU_Signed_Long_Result;

						uint64_t a1 = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 12) & 0xF);
						uint64_t a2 = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 16) & 0xF);

						a2 = a2 << 32;
						cpu_ALU_Long_Result += (a1 + a2);
					}
					else
					{
						s1 = (int32_t)cpu_LDM_Glitch_Get_Reg(cpu_Instr_ARM_2 & 0xF);
						s2 = (int32_t)cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF);

						cpu_ALU_Signed_Long_Result = s1 * s2;

						cpu_ALU_Long_Result = (uint64_t)cpu_ALU_Signed_Long_Result;
					}

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);

						// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1,-1
						// then it does not change (probably multiplier takes a short cut for these cases)
						if ((s2 != 0) && (s2 != 1) && (s2 != -1))
						{
							cpu_FlagCset(!((s1 & 0x80000000) == (s2 & 0x80000000)));
						}
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)(cpu_ALU_Long_Result >> 32);
					break;

				case cpu_ARM_Swap_LDM:
					cpu_Base_Reg = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 16) & 0xF);
					cpu_Base_Reg_2 = ((cpu_Instr_ARM_2 >> 12) & 0xF);
					cpu_Temp_Reg_Ptr = (cpu_Instr_ARM_2 & 0xF);
					break;

				case cpu_ARM_Imm_LS_LDM:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = ((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg_Ptr = ((cpu_Instr_ARM_2 >> 12) & 0xF);

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// increment first
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Temp_Addr = (uint32_t)(cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg) + cpu_Addr_Offset);
						}
						else
						{
							cpu_Temp_Addr = (uint32_t)(cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg) - cpu_Addr_Offset);
						}
					}
					else
					{
						// increment last
						cpu_Temp_Addr = cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg);
					}

					cpu_Overwrite_Base_Reg = false;

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;

						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr + cpu_Addr_Offset);
						}
						else
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - cpu_Addr_Offset);
						}
					}
					else if ((cpu_Instr_ARM_2 & 0x200000) == 0x200000)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// use incremented address
					//if (cpu_Base_Reg == 15) { cpu_Write_Back_Addr += 4; }

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}

					break;

				case cpu_ARM_Reg_LS_LDM:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = ((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg_Ptr = ((cpu_Instr_ARM_2 >> 12) & 0xF);

					cpu_Temp_Data = cpu_LDM_Glitch_Get_Reg(cpu_Instr_ARM_2 & 0xF);

					cpu_Shift_Imm = ((cpu_Instr_ARM_2 >> 7) & 0x1F);

					switch ((cpu_Instr_ARM_2 >> 5) & 3)
					{
					case 0:         // LSL
						cpu_Temp_Data = cpu_Temp_Data << cpu_Shift_Imm;
						break;

					case 1:         // LSR
						if (cpu_Shift_Imm == 0) { cpu_Temp_Data = 0; }
						else { cpu_Temp_Data = cpu_Temp_Data >> cpu_Shift_Imm; }
						break;

					case 2:         // ASR
						if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }

						cpu_ALU_Temp_S_Val = cpu_Temp_Data & cpu_Neg_Compare;

						for (int i = 1; i <= cpu_Shift_Imm; i++)
						{
							cpu_Temp_Data = (cpu_Temp_Data >> 1);
							cpu_Temp_Data |= cpu_ALU_Temp_S_Val;
						}
						break;

					case 3:         // RRX
						if (cpu_Shift_Imm == 0)
						{
							cpu_Temp_Data = (cpu_Temp_Data >> 1);
							cpu_Temp_Data |= cpu_FlagCget() ? 0x80000000 : 0;
						}
						else
						{
							for (int i = 1; i <= cpu_Shift_Imm; i++)
							{
								cpu_ALU_Temp_S_Val = (uint32_t)(cpu_Temp_Data & 1);
								cpu_Temp_Data = (cpu_Temp_Data >> 1);
								cpu_Temp_Data |= (cpu_ALU_Temp_S_Val << 31);
							}
						}
						break;
					}

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// increment first
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Temp_Addr = (uint32_t)(cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg) + cpu_Temp_Data);
						}
						else
						{
							cpu_Temp_Addr = (uint32_t)(cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg) - cpu_Temp_Data);
						}
					}
					else
					{
						// increment last
						cpu_Temp_Addr = cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg);
					}

					cpu_Overwrite_Base_Reg = false;

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;

						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr + cpu_Temp_Data);
						}
						else
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - cpu_Temp_Data);
						}
					}
					else if ((cpu_Instr_ARM_2 & 0x200000) == 0x200000)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// use incremented address
					//if (cpu_Base_Reg == 15) { cpu_Write_Back_Addr += 4; }

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}
					break;

				case cpu_ARM_Multi_1_LDM:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = ((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Addr = cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg);

					cpu_LS_First_Access = true;

					cpu_Overwrite_Base_Reg = (cpu_Instr_ARM_2 & 0x200000) == 0x200000;

					cpu_Multi_Before = (cpu_Instr_ARM_2 & 0x1000000) == 0x1000000;

					cpu_Multi_Inc = (cpu_Instr_ARM_2 & 0x800000) == 0x800000;

					// build list of registers to access
					cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

					// need some special logic here, if the S bit is set, we swap out registers to user mode for the accesses
					// then swap back afterwards, but only if reg15 is not accessed in a load
					cpu_Multi_S_Bit = (cpu_Instr_ARM_2 & 0x400000) == 0x400000;
					cpu_Multi_Swap = false;
					Use_Reg_15 = false;

					for (int i = 0; i < 16; i++)
					{
						if (((cpu_Instr_ARM_2 >> i) & 1) == 1)
						{
							cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
							if ((i == 15) && cpu_LS_Is_Load) { Use_Reg_15 = true; }

							cpu_Multi_List_Size += 1;

							// The documentation gives this case as unpredictable for now copy Thumb logic
							if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
						}
					}

					cpu_Special_Inc = false;

					// No registers selected loads / stores R15 instead
					if (cpu_Multi_List_Size == 0)
					{
						cpu_Multi_List_Size = 1;
						cpu_Regs_To_Access[0] = 15;
						cpu_Special_Inc = true;
						Use_Reg_15 = true; // ?
					}

					// when decrementing, start address is at the bottom, (similar to a POP instruction)
					if (!cpu_Multi_Inc)
					{
						cpu_Temp_Addr -= (uint32_t)((cpu_Multi_List_Size - 1) * 4);

						// in either case, write back is the same
						cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - 4);

						if (cpu_Multi_Before) { cpu_Temp_Addr -= 4; }

						if (cpu_Special_Inc)
						{
							cpu_Write_Back_Addr -= 0x3C;
							cpu_Temp_Addr -= 0x3C;
							cpu_Special_Inc = false;
						}
					}
					else
					{
						if (cpu_Multi_Before) { cpu_Temp_Addr += 4; }
					}

					// swap out registers for user mode ones
					if (cpu_Multi_S_Bit && !Use_Reg_15)
					{
						cpu_Multi_Swap = true;
						cpu_Temp_Mode = cpu_Regs[16] & 0x1F;

						cpu_Swap_Regs(0x10, false, false);
					}
					break;

				case cpu_ARM_Branch_LDM:
					ofst = ((cpu_Instr_ARM_2 & 0xFFFFFF) << 2);

					// offset is signed
					if ((ofst & 0x2000000) == 0x2000000) { ofst = (ofst | 0xFC000000); }

					cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

					// Link if link bit set
					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						cpu_Regs[14] = (uint32_t)(cpu_Regs[15] - 4);
					}

					cpu_Take_Branch = true;
					break;
			}
		}
		else
		{
			cpu_ARM_Cond_Passed = false;
		}
	}

	void GBA_System::cpu_Execute_Internal_Only_TMB()
	{
		int ofst = 0;

		// Do Tracer stuff here
		switch (cpu_Exec_TMB)
		{
		case cpu_Thumb_Shift:
			cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

			cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

			cpu_ALU_Temp_Val = (uint32_t)((cpu_Instr_TMB_2 >> 6) & 0x1F);

			switch ((cpu_Instr_TMB_2 >> 11) & 0x3)
			{
				case 0:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
					cpu_ALU_Long_Result = cpu_ALU_Long_Result << (uint32_t)cpu_ALU_Temp_Val;

					if (cpu_ALU_Temp_Val != 0) { cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare); }
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case 1:
					if (cpu_ALU_Temp_Val != 0)
					{
						cpu_FlagCset(((cpu_Regs[cpu_ALU_Reg_Src] >> (uint32_t)(cpu_ALU_Temp_Val - 1)) & 1) == 1);
						cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src] >> (uint32_t)cpu_ALU_Temp_Val;
					}
					else
					{
						cpu_FlagCset((cpu_Regs[cpu_ALU_Reg_Src] & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_ALU_Long_Result = 0;
					}

					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case 2:
					cpu_ALU_Temp_S_Val = cpu_Regs[cpu_ALU_Reg_Src] & cpu_Neg_Compare;

					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];

					if (cpu_ALU_Temp_Val == 0) { cpu_ALU_Temp_Val = 32; }

					for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & 1) == 1);
						cpu_ALU_Long_Result = cpu_ALU_Long_Result >> 1;
						cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
					}

					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;
			}
			break;

		case cpu_Thumb_Add_Sub_Reg:
			cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

			cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

			if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
			{
				cpu_ALU_Temp_Val = (uint32_t)((cpu_Instr_TMB_2 >> 6) & 0x7);
			}
			else
			{
				cpu_ALU_Temp_Val = cpu_Regs[(cpu_Instr_TMB_2 >> 6) & 0x7];
			}

			if ((cpu_Instr_TMB_2 & 0x200) == 0x200)
			{
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
				cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Src], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			}
			else
			{
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
				cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Src], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			}
			break;

		case cpu_Thumb_AND:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_EOR:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] ^ cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_LSL:
			cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
			cpu_ALU_Long_Result = cpu_ALU_Long_Result << (uint32_t)cpu_ALU_Temp_Val;

			if (cpu_ALU_Temp_Val != 0) { cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare); }
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_LSR:
			cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

			// NOTE: This is necessary due to C# only using lower 5 bits of shift count on integer sized values.
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

			if (cpu_ALU_Temp_Val != 0)
			{
				cpu_FlagCset(((cpu_ALU_Long_Result >> (uint32_t)(cpu_ALU_Temp_Val - 1)) & 1) == 1);
			}

			cpu_ALU_Long_Result = cpu_ALU_Long_Result >> (uint32_t)cpu_ALU_Temp_Val;

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_ASR:
			cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;
			cpu_ALU_Temp_S_Val = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Neg_Compare;

			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

			for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
			{
				cpu_FlagCset((cpu_ALU_Long_Result & 1) == 1);
				cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
				cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
			}

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_ADC:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
			cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];
			cpu_ALU_Long_Result += (uint64_t)(cpu_FlagCget() ? 1 : 0);

			cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_SBC:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
			cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];
			cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

			cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_ROR:
			cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

			for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
			{
				cpu_ALU_Temp_S_Val = (uint32_t)(cpu_ALU_Long_Result & 1);
				cpu_ALU_Long_Result = cpu_ALU_Long_Result >> 1;
				cpu_ALU_Long_Result |= (cpu_ALU_Temp_S_Val << 31);
			}

			if (cpu_ALU_Temp_Val != 0) { cpu_FlagCset(cpu_ALU_Temp_S_Val == 1); }

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_TST:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);
			break;

		case cpu_Thumb_NEG:
			cpu_ALU_Long_Result = 0;
			cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagVset(cpu_Calc_V_Flag_Sub(0, cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_CMP:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
			cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);
			break;

		case cpu_Thumb_CMN:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
			cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);
			break;

		case cpu_Thumb_ORR:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] | cpu_Regs[cpu_ALU_Reg_Src];

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_MUL:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] * cpu_Regs[cpu_ALU_Reg_Src];

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_BIC:
			cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & (~cpu_Regs[cpu_ALU_Reg_Src]);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_MVN:
			cpu_ALU_Long_Result = (~cpu_Regs[cpu_ALU_Reg_Src]);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);

			cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
			break;

		case cpu_Thumb_High_Add:
			// Add but no flags change
			// unpredictable if both registers are low registers?
			cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

			cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 3) & 0xF);

			if (cpu_Base_Reg == 15)
			{
				// this becomes a branch
				cpu_Take_Branch = true;

				cpu_Temp_Reg = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);
			}
			else
			{
				cpu_Take_Branch = false;
				cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);
			}
			break;

		case cpu_Thumb_High_Cmp:
			// Sub but no flags change
			// unpredictable if first register is 15?
			cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

			cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 3) & 0xF);

			cpu_ALU_Long_Result = cpu_Regs[cpu_Base_Reg];
			cpu_ALU_Long_Result -= cpu_Regs[cpu_Base_Reg_2];

			cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

			cpu_ALU_Long_Result &= cpu_Cast_Int;

			cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_Base_Reg], cpu_Regs[cpu_Base_Reg_2], (uint32_t)cpu_ALU_Long_Result));
			cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
			cpu_FlagZset(cpu_ALU_Long_Result == 0);
			break;

		case cpu_Thumb_High_Bx:
			// Branch and exchange mode (possibly)
			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 0xF);

			cpu_Temp_Reg = (cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFE);
			break;

		case cpu_Thumb_High_MOV:
			cpu_Temp_Reg = cpu_Regs[((cpu_Instr_TMB_2 >> 3) & 0xF)];

			cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

			// don't change the read address before branching if destination is a branch
			if (cpu_Base_Reg != 0xF)
			{
				cpu_Regs[cpu_Base_Reg] = cpu_Temp_Reg;
			}
			else
			{
				cpu_Take_Branch = true;
			}
			break;

		case cpu_Thumb_ALU_Imm:
			cpu_ALU_Reg_Dest = (cpu_Instr_TMB_2 >> 8) & 7;

			cpu_ALU_Temp_Val = (uint32_t)(cpu_Instr_TMB_2 & 0xFF);

			switch ((cpu_Instr_TMB_2 >> 11) & 3)
			{
				case 0:			// MOV
					cpu_Regs[cpu_ALU_Reg_Dest] = cpu_ALU_Temp_Val;
					cpu_FlagNset(false);
					cpu_FlagZset(cpu_ALU_Temp_Val == 0);
					break;

				case 1:         // CMP
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);
					break;

				case 2:         // ADD
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case 3:         // SUB
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;
			}
			break;

		case cpu_Thumb_PC_Rel_LS:
			cpu_LS_Is_Load = true;

			cpu_Overwrite_Base_Reg = false;

			cpu_Base_Reg = 15;

			cpu_Temp_Addr = (uint32_t)((cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));

			cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
			break;

		case cpu_Thumb_Rel_LS:
			switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
			{
				case 0: cpu_LS_Is_Load = false; cpu_Sign_Extend_Load = false; break;
				case 1: cpu_LS_Is_Load = false; cpu_Sign_Extend_Load = false; break;
				case 2: cpu_LS_Is_Load = false; cpu_Sign_Extend_Load = false; break;
				case 3: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
				case 4: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = false; break;
				case 5: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = false; break;
				case 6: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = false; break;
				case 7: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
			}

			cpu_Overwrite_Base_Reg = false;

			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);
			cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 6) & 7);

			cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);

			cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
			break;

		case cpu_Thumb_Imm_LS:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

			cpu_Overwrite_Base_Reg = false;

			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

			if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
			{
				cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 6) & 0x1F));
			}
			else
			{
				cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 4) & 0x7C));
			}

			cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
			break;

		case cpu_Thumb_Half_LS:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

			cpu_Overwrite_Base_Reg = false;

			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

			cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 5) & 0x3E));

			cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
			break;

		case cpu_Thumb_SP_REL_LS:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

			cpu_Overwrite_Base_Reg = false;

			cpu_Temp_Addr = (uint32_t)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));

			cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
			break;

		case cpu_Thumb_Add_SP_PC:
			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 8) & 7);

			cpu_Overwrite_Base_Reg = false;

			if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
			{
				cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));
			}
			else
			{
				cpu_Regs[cpu_Base_Reg] = (uint32_t)((cpu_Regs[15] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));
			}
			break;

		case cpu_Thumb_Add_Sub_Stack:
			if ((cpu_Instr_TMB_2 & 0x80) == 0x0)
			{
				cpu_Regs[13] += (uint32_t)((cpu_Instr_TMB_2 & 0x7F) << 2);
			}
			else
			{
				cpu_Regs[13] -= (uint32_t)((cpu_Instr_TMB_2 & 0x7F) << 2);
			}
			break;

		case cpu_Thumb_Push_Pop:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

			// base reg is always 13 for push / pop
			cpu_Base_Reg = 13;

			cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

			cpu_LS_First_Access = true;

			// always overwrite base reg
			cpu_Overwrite_Base_Reg = true;

			// Increment timing depends on it being a push or a pop
			cpu_Multi_Before = !cpu_LS_Is_Load;

			// Increment / Decrement depends on it being a push or a pop
			cpu_Multi_Inc = cpu_LS_Is_Load;

			// No analog to ARM S Bit here
			cpu_Multi_S_Bit = false;
			cpu_Multi_Swap = false;

			// build list of registers to access
			cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

			for (int i = 0; i < 8; i++)
			{
				if (((cpu_Instr_TMB_2 >> i) & 1) == 1)
				{
					cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
					cpu_Multi_List_Size += 1;

					if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
				}
			}

			// additionally, may impact reg 14 (Link), or 15 (PC)
			if ((cpu_Instr_TMB_2 & 0x100) == 0x100)
			{
				if (cpu_LS_Is_Load)
				{
					cpu_Regs_To_Access[cpu_Multi_List_Size] = 15;
				}
				else
				{
					cpu_Regs_To_Access[cpu_Multi_List_Size] = 14;
				}

				cpu_Multi_List_Size += 1;
			}

			cpu_Special_Inc = false;

			// For Pop, start address is at the bottom
			if (!cpu_Multi_Inc)
			{
				cpu_Temp_Addr -= (uint32_t)((cpu_Multi_List_Size - 1) * 4);
				cpu_Temp_Addr -= 4;
				cpu_Write_Back_Addr = cpu_Temp_Addr;
			}

			// No registers selected is unpredictable
			if (cpu_Multi_List_Size == 0)
			{
				cpu_Multi_List_Size = 1;
				cpu_Regs_To_Access[0] = 15;
				cpu_Special_Inc = true;
			}
			break;

		case cpu_Thumb_Multi_1:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

			cpu_Base_Reg = (cpu_Instr_TMB_2 >> 8) & 7;

			cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

			cpu_LS_First_Access = true;

			// always overwrite base reg
			cpu_Overwrite_Base_Reg = true;

			// always increment after
			cpu_Multi_Before = false;

			// always increment
			cpu_Multi_Inc = true;

			// No analog to ARM S Bit here
			cpu_Multi_S_Bit = false;
			cpu_Multi_Swap = false;

			// build list of registers to access
			cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

			for (int i = 0; i < 8; i++)
			{
				if (((cpu_Instr_TMB_2 >> i) & 1) == 1)
				{
					cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
					cpu_Multi_List_Size += 1;

					if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
				}
			}

			cpu_Special_Inc = false;

			// No registers selected loads R15 instead
			if (cpu_Multi_List_Size == 0)
			{
				cpu_Multi_List_Size = 1;
				cpu_Regs_To_Access[0] = 15;
				cpu_Special_Inc = true;
			}
			break;

		case cpu_Thumb_Branch:
			ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 1);

			// offset is signed
			if ((ofst & 0x800) == 0x800) { ofst = (uint32_t)(ofst | 0xFFFFF000); }

			cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

			cpu_Take_Branch = true;
			break;

		case cpu_Thumb_Branch_Cond:
			ofst = (uint32_t)((cpu_Instr_TMB_2 & 0xFF) << 1);

			// offset is signed
			if ((ofst & 0x100) == 0x100) { ofst = (uint32_t)(ofst | 0xFFFFFE00); }

			cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

			cpu_Take_Branch = cpu_TMB_Condition_Check();
			break;

		case cpu_Thumb_Branch_Link_1:
			ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 12);

			// offset is signed
			if ((ofst & 0x400000) == 0x400000) { ofst = (uint32_t)(ofst | 0xFF800000); }

			cpu_Regs[14] = (uint32_t)(cpu_Regs[15] + ofst);
			break;

		case cpu_Thumb_Branch_Link_2:
			ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 1);

			cpu_Temp_Reg = (uint32_t)(cpu_Regs[14] + ofst);

			// NOTE: OR with 1, probably reuses the same cpu circuitry that triggers switch to Thumb mode when writing to R[15] directly?
			cpu_Regs[14] = (uint32_t)((cpu_Regs[15] - 2) | 1);

			cpu_Take_Branch = true;
			break;
		}
	}

	// LDM^ glitch details:
	// If the next register access happens immediately following the completion of the LDM^/STM^ instruction,
	// and this access is to one of the banked registers,
	// then the returned value will be the banked register and user mode register OR'd together
	void GBA_System::cpu_LDM_Glitch_Decode_ARM()
	{
		switch ((cpu_Instr_ARM_2 >> 25) & 7)
		{
			case 0:
				if ((cpu_Instr_ARM_2 & 0x90) == 0x90)
				{
					// miscellaneous
					if (((cpu_Instr_ARM_2 & 0xF0) == 0x90))
					{
						switch ((cpu_Instr_ARM_2 >> 22) & 0x7)
						{
							case 0x0:
							case 0x1:
								// Multiply
								cpu_Instr_Type = cpu_Multiply_ARM;
								cpu_Exec_ARM = cpu_ARM_MUL_LDM;
								cpu_Calculate_Mul_Cycles_LDM();
								break;

							case 0x2:
								// Multiply Long - Unsigned
								cpu_Instr_Type = cpu_Multiply_ARM;
								cpu_Exec_ARM = cpu_ARM_MUL_UL_LDM;
								cpu_Calculate_Mul_Cycles_UL_LDM();
								break;
							case 0x3:
								// Multiply Long - Signed
								cpu_Instr_Type = cpu_Multiply_ARM;
								cpu_Exec_ARM = cpu_ARM_MUL_SL_LDM;
								cpu_Calculate_Mul_Cycles_SL_LDM();
								break;

							case 0x4:
							case 0x5:
							case 0x6:
							case 0x7:
								// Swap
								cpu_Instr_Type = cpu_Prefetch_Swap_ARM;
								cpu_Next_Load_Store_Type = cpu_Swap_ARM;
								cpu_Exec_ARM = cpu_ARM_Swap_LDM;
								cpu_Swap_Store = false;
								break;
						}
					}
					else
					{
						// halfword or byte transfers
						cpu_Exec_ARM = cpu_ARM_Imm_LS_LDM;
						cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

						switch ((cpu_Instr_ARM_2 >> 5) & 0x3)
						{
							// 0 case is not a load store instruction
							case 0x1:
								// Unsigned halfword
								cpu_Next_Load_Store_Type = cpu_Load_Store_Half_ARM;
								cpu_Sign_Extend_Load = false;
								break;

							case 0x2:
								// Signed Byte
								cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
								cpu_Sign_Extend_Load = true;
								break;
							case 0x3:
								// Signed halfword
								cpu_Next_Load_Store_Type = cpu_Load_Store_Half_ARM;
								cpu_Sign_Extend_Load = true;
								break;
						}

						if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
						{
							cpu_Addr_Offset = ((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF);
						}
						else
						{
							cpu_Addr_Offset = cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						}
					}
				}
				else
				{
					// ALU ops
					cpu_ALU_S_Bit = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_ALU_Reg_Dest = ((cpu_Instr_ARM_2 >> 12) & 0xF);

					cpu_ALU_Reg_Src = ((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg = cpu_LDM_Glitch_Get_Reg((uint32_t)cpu_ALU_Reg_Src);

					// slightly different code path for R15 as destination, since it's closer to a branch
					cpu_Dest_Is_R15 = (cpu_ALU_Reg_Dest == 15);

					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
						case 0x0: cpu_Exec_ARM = cpu_ARM_AND_LDM; cpu_Clear_Pipeline = true; break;
						case 0x1: cpu_Exec_ARM = cpu_ARM_EOR_LDM; cpu_Clear_Pipeline = true; break;
						case 0x2: cpu_Exec_ARM = cpu_ARM_SUB_LDM; cpu_Clear_Pipeline = true; break;
						case 0x3: cpu_Exec_ARM = cpu_ARM_RSB_LDM; cpu_Clear_Pipeline = true; break;
						case 0x4: cpu_Exec_ARM = cpu_ARM_ADD_LDM; cpu_Clear_Pipeline = true; break;
						case 0x5: cpu_Exec_ARM = cpu_ARM_ADC_LDM; cpu_Clear_Pipeline = true; break;
						case 0x6: cpu_Exec_ARM = cpu_ARM_SBC_LDM; cpu_Clear_Pipeline = true; break;
						case 0x7: cpu_Exec_ARM = cpu_ARM_RSC_LDM; cpu_Clear_Pipeline = true; break;
						case 0x8: cpu_Exec_ARM = cpu_ARM_TST_LDM; cpu_Clear_Pipeline = false; break;
						case 0x9: cpu_Exec_ARM = cpu_ARM_TEQ_LDM; cpu_Clear_Pipeline = false; break;
						case 0xA: cpu_Exec_ARM = cpu_ARM_CMP_LDM; cpu_Clear_Pipeline = false; break;
						case 0xB: cpu_Exec_ARM = cpu_ARM_CMN_LDM; cpu_Clear_Pipeline = false; break;
						case 0xC: cpu_Exec_ARM = cpu_ARM_ORR_LDM; cpu_Clear_Pipeline = true; break;
						case 0xD: cpu_Exec_ARM = cpu_ARM_MOV_LDM; cpu_Clear_Pipeline = true; break;
						case 0xE: cpu_Exec_ARM = cpu_ARM_BIC_LDM; cpu_Clear_Pipeline = true; break;
						case 0xF: cpu_Exec_ARM = cpu_ARM_MVN_LDM; cpu_Clear_Pipeline = true; break;
					}

					// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
					cpu_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

					bool is_RRX = false;

					cpu_ALU_Long_Result = cpu_LDM_Glitch_Get_Reg(cpu_Instr_ARM_2 & 0xF);

					cpu_LDM_Glitch_Store = true;

					if ((cpu_Instr_ARM_2 & 0x10) != 0x0)
					{
						// don't use glitched operands because the glitched reg is read first
						cpu_LDM_Glitch_Store = false;
						cpu_Temp_Reg = cpu_Regs[cpu_ALU_Reg_Src];
						cpu_ALU_Long_Result = cpu_Regs[cpu_Instr_ARM_2 & 0xF];

						// if the pc is the shifted value or operand, and its a register shift, it is the incremented value that is used
						if ((cpu_Instr_ARM_2 & 0xF) == 15)
						{
							cpu_ALU_Long_Result += 4; cpu_ALU_Long_Result &= cpu_Cast_Int;
						}

						if (cpu_ALU_Reg_Src == 15)
						{
							cpu_Temp_Reg += 4;
						}

						// register shifts take an extra cycle
						if (cpu_Instr_Type == cpu_Internal_And_Prefetch_ARM) { cpu_Instr_Type = cpu_Internal_And_Prefetch_2_ARM; }
						if (cpu_Instr_Type == cpu_Internal_And_Branch_2_ARM) { cpu_Instr_Type = cpu_Internal_And_Branch_3_ARM; }
					}

					cpu_ALU_Shift_Carry = (uint32_t)(cpu_FlagCget() ? 1 : 0);

					switch ((cpu_Instr_ARM_2 >> 5) & 3)
					{
						case 0:         // LSL
							// calculate immedaite value
							if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
							{
								// immediate shift
								cpu_Shift_Imm = ((cpu_Instr_ARM_2 >> 7) & 0x1F);
							}
							else
							{
								// register shift
								cpu_Shift_Imm = (cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF) & 0xFF);
							}

							cpu_ALU_Long_Result = cpu_ALU_Long_Result << cpu_Shift_Imm;

							if (cpu_Shift_Imm != 0)
							{
								cpu_ALU_Shift_Carry = (uint32_t)((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare ? 1 : 0);
							}
							break;

						case 1:         // LSR
							// calculate immedaite value
							if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
							{
								// immediate shift
								cpu_Shift_Imm = ((cpu_Instr_ARM_2 >> 7) & 0x1F);

								if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
							}
							else
							{
								// register shift
								cpu_Shift_Imm = (cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF) & 0xFF);
							}

							if (cpu_Shift_Imm != 0)
							{
								cpu_ALU_Shift_Carry = (uint32_t)((cpu_ALU_Long_Result >> (cpu_Shift_Imm - 1)) & 1);
								cpu_ALU_Long_Result = cpu_ALU_Long_Result >> cpu_Shift_Imm;
							}
							break;

						case 2:         // ASR
							// calculate immedaite value
							if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
							{
								// immediate shift
								cpu_Shift_Imm = ((cpu_Instr_ARM_2 >> 7) & 0x1F);

								if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
							}
							else
							{
								// register shift
								cpu_Shift_Imm = (cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF) & 0xFF);
							}

							cpu_ALU_Temp_S_Val = (uint32_t)(cpu_ALU_Long_Result & cpu_Neg_Compare);

							for (int i = 1; i <= cpu_Shift_Imm; i++)
							{
								cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
								cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
								cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
							}
							break;

						case 3:         // RRX
							// calculate immedaite value
							if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
							{
								// immediate shift
								cpu_Shift_Imm = ((cpu_Instr_ARM_2 >> 7) & 0x1F);

								if (cpu_Shift_Imm == 0) { is_RRX = true; }
							}
							else
							{
								// register shift
								cpu_Shift_Imm = (cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF) & 0xFF);
							}

							if (is_RRX)
							{
								cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
								cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
								cpu_ALU_Long_Result |= cpu_FlagCget() ? 0x80000000 : 0;
							}
							else
							{
								for (int i = 1; i <= cpu_Shift_Imm; i++)
								{
									cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
									cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
									cpu_ALU_Long_Result |= (cpu_ALU_Shift_Carry << 31);
								}
							}
							break;
					}

					cpu_ALU_Temp_Val = (uint32_t)cpu_ALU_Long_Result;

					// overwrite certain instructions
					if (!cpu_ALU_S_Bit)
					{
						switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
						{
							case 0x8:
								cpu_Instr_Type = cpu_Internal_And_Prefetch_ARM;
								cpu_Exec_ARM = cpu_ARM_MRS_LDM;
								break;
							case 0x9:
								if ((cpu_Instr_ARM_2 & 0xFFF90) == 0xFFF10)
								{
									// Branch and exchange
									cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_ARM;
									cpu_Exec_ARM = cpu_ARM_Bx_LDM;
								}
								else
								{
									cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
									cpu_Exec_ARM = cpu_ARM_MSR_LDM;
								}
								break;
							case 0xA:
								cpu_Instr_Type = cpu_Internal_And_Prefetch_ARM;
								cpu_Exec_ARM = cpu_ARM_MRS_LDM;
								break;
							case 0xB:
								if ((cpu_Instr_ARM_2 & 0xFFF90) == 0xFFF10)
								{
									// Branch and exchange
									cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_ARM;
									cpu_Exec_ARM = cpu_ARM_Bx_LDM;
								}
								else
								{
									cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
									cpu_Exec_ARM = cpu_ARM_MSR_LDM;
								}
								break;
						}
					}
				}
				break;

			case 1:
				// ALU ops
				cpu_ALU_S_Bit = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

				cpu_ALU_Reg_Dest = ((cpu_Instr_ARM_2 >> 12) & 0xF);

				cpu_ALU_Reg_Src = ((cpu_Instr_ARM_2 >> 16) & 0xF);
				cpu_Temp_Reg = cpu_LDM_Glitch_Get_Reg(cpu_ALU_Reg_Src);

				// slightly different code path for R15 as destination, since it's closer to a branch
				cpu_Dest_Is_R15 = (cpu_ALU_Reg_Dest == 15);

				switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
				{
					case 0x0: cpu_Exec_ARM = cpu_ARM_AND_LDM; cpu_Clear_Pipeline = true; break;
					case 0x1: cpu_Exec_ARM = cpu_ARM_EOR_LDM; cpu_Clear_Pipeline = true; break;
					case 0x2: cpu_Exec_ARM = cpu_ARM_SUB_LDM; cpu_Clear_Pipeline = true; break;
					case 0x3: cpu_Exec_ARM = cpu_ARM_RSB_LDM; cpu_Clear_Pipeline = true; break;
					case 0x4: cpu_Exec_ARM = cpu_ARM_ADD_LDM; cpu_Clear_Pipeline = true; break;
					case 0x5: cpu_Exec_ARM = cpu_ARM_ADC_LDM; cpu_Clear_Pipeline = true; break;
					case 0x6: cpu_Exec_ARM = cpu_ARM_SBC_LDM; cpu_Clear_Pipeline = true; break;
					case 0x7: cpu_Exec_ARM = cpu_ARM_RSC_LDM; cpu_Clear_Pipeline = true; break;
					case 0x8: cpu_Exec_ARM = cpu_ARM_TST_LDM; cpu_Clear_Pipeline = false; break;
					case 0x9: cpu_Exec_ARM = cpu_ARM_TEQ_LDM; cpu_Clear_Pipeline = false; break;
					case 0xA: cpu_Exec_ARM = cpu_ARM_CMP_LDM; cpu_Clear_Pipeline = false; break;
					case 0xB: cpu_Exec_ARM = cpu_ARM_CMN_LDM; cpu_Clear_Pipeline = false; break;
					case 0xC: cpu_Exec_ARM = cpu_ARM_ORR_LDM; cpu_Clear_Pipeline = true; break;
					case 0xD: cpu_Exec_ARM = cpu_ARM_MOV_LDM; cpu_Clear_Pipeline = true; break;
					case 0xE: cpu_Exec_ARM = cpu_ARM_BIC_LDM; cpu_Clear_Pipeline = true; break;
					case 0xF: cpu_Exec_ARM = cpu_ARM_MVN_LDM; cpu_Clear_Pipeline = true; break;
				}

				cpu_LDM_Glitch_Store = true;

				// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
				cpu_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

				// calculate immedaite value
				cpu_ALU_Temp_Val = cpu_Instr_ARM_2 & 0xFF;

				cpu_ALU_Shift_Carry = (uint32_t)(cpu_FlagCget() ? 1 : 0);

				// Note: the shift val is multiplied by 2 (so only shift by 7 below)
				for (int i = 1; i <= ((cpu_Instr_ARM_2 >> 7) & 0x1E); i++)
				{
					cpu_ALU_Shift_Carry = cpu_ALU_Temp_Val & 1;
					cpu_ALU_Temp_Val = (cpu_ALU_Temp_Val >> 1) | (cpu_ALU_Shift_Carry << 31);
				}

				// overwrite certain instructions
				if (!cpu_ALU_S_Bit)
				{
					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
						case 0x8:
							// Gltiched version of MSR
							cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR_LDM_Glitchy;
							break;
						case 0x9:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR_LDM;
							break;
						case 0xA:
							// Gltiched version of MSR
							cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR_LDM_Glitchy;
							break;
						case 0xB:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR_LDM;
							break;
					}
				}
				break;

			case 2:
				// load / store immediate offset
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

				if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
					cpu_Sign_Extend_Load = false;
				}
				else
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_ARM;
				}

				cpu_Exec_ARM = cpu_ARM_Imm_LS_LDM;
				cpu_Addr_Offset = cpu_Instr_ARM_2 & 0xFFF;
				break;

			case 3:
				if ((cpu_Instr_ARM_2 & 0x10) == 0)
				{
					// load / store register offset
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

					if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
					{
						cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
						cpu_Sign_Extend_Load = false;
					}
					else
					{
						cpu_Next_Load_Store_Type = cpu_Load_Store_Word_ARM;
					}

					cpu_Exec_ARM = cpu_ARM_Reg_LS_LDM;
				}
				else
				{
					// Undefined Opcode Exception
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only_LDM;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				break;

			case 4:
				// block transfer
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;
				cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_ARM;
				cpu_Exec_ARM = cpu_ARM_Multi_1_LDM;
				break;

			case 5:
				// branch
				cpu_Instr_Type = cpu_Internal_And_Branch_1_ARM;
				cpu_Exec_ARM = cpu_ARM_Branch_LDM;
				break;

			case 6:
				// Coprocessor Instruction (treat as Undefined Opcode Exception)
				cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
				cpu_Exec_ARM = cpu_ARM_Cond_Check_Only_LDM;
				cpu_Exception_Type = cpu_Undef_Exc;
				break;

			case 7:
				if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
				{
					// software interrupt
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only_LDM;
					cpu_Exception_Type = cpu_SWI_Exc;
				}
				else
				{
					// Coprocessor Instruction (treat as Undefined Opcode Exception)
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only_LDM;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				break;

			default:
				cpu_Instr_Type = cpu_Internal_Halted;
				break;
		}

		cpu_LDM_Glitch_Mode = false;

		if (TraceCallback) TraceCallback(0);
	}
}