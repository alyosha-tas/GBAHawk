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

		cpu_LDM_Glitch_Instr_Type = 0;

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
						// Multiply
						cpu_Instr_Type = cpu_Multiply_ARM;
						cpu_Exec_ARM = cpu_ARM_MUL;
						cpu_Calculate_Mul_Cycles();
						break;

					case 0x1:
						// Undefined Opcode Exception
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
						cpu_Exception_Type = cpu_Undef_Exc;
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
						// Swap
						cpu_Instr_Type = cpu_Prefetch_Swap_ARM;
						cpu_Next_Load_Store_Type = cpu_Swap_ARM;
						cpu_Exec_ARM = cpu_ARM_Swap;
						cpu_Swap_Store = false;
						break;

					case 0x6:
					case 0x7:
						// Undefined Opcode Exception
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
						cpu_Exception_Type = cpu_Undef_Exc;
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
						if ((cpu_Instr_ARM_2 & 0XFFFF0) == 0xFFF10)
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
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR;
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
					// Undefined Opcode Exception
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_Undef_Exc;
					break;
				case 0x9:
					cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
					cpu_Exec_ARM = cpu_ARM_MSR;
					break;
				case 0xA:
					// Undefined Opcode Exception
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_Undef_Exc;
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
							if ((cpu_Instr_TMB_2 & 0xC0) == 0x0)
							{
								// copy only available in ARMv6 and above
								throw new std::invalid_argument("undefined instruction (CPY) 01000110 " + to_string(cpu_Regs[15]));
							}
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
							if ((cpu_Instr_TMB_2 & 0x80) == 0)
							{
								cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_TMB;
								cpu_Exec_TMB = cpu_Thumb_High_Bx;
							}
							else
							{
								// This version only available in ARM V5 and above
								throw new std::invalid_argument("undefined instruction (BLX) 010001111 " + to_string(cpu_Regs[15]));
							}
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

				cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
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
				cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);
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

			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);
			cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 6) & 7);

			cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);

			cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
			break;

		case cpu_Thumb_Imm_LS:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

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

			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

			cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 5) & 0x3E));

			cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
			break;

		case cpu_Thumb_SP_REL_LS:
			cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

			cpu_Temp_Addr = (uint32_t)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));

			cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
			break;

		case cpu_Thumb_Add_SP_PC:
			cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 8) & 7);

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
				throw new std::invalid_argument("no registers selected in Multi Load/Store at " + to_string(cpu_Regs[15]));
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
								// Multiply
								cpu_LDM_Glitch_Instr_Type = cpu_Multiply_ARM;
								cpu_Exec_ARM = cpu_ARM_MUL;
								cpu_Calculate_Mul_Cycles();
								break;

							case 0x1:
								// Undefined Opcode Exception
								cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
								cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
								cpu_Exception_Type = cpu_Undef_Exc;
								break;

							case 0x2:
								// Multiply Long - Unsigned
								cpu_LDM_Glitch_Instr_Type = cpu_Multiply_ARM;
								cpu_Exec_ARM = cpu_ARM_MUL_UL;
								cpu_Calculate_Mul_Cycles_UL();
								break;
							case 0x3:
								// Multiply Long - Signed
								cpu_LDM_Glitch_Instr_Type = cpu_Multiply_ARM;
								cpu_Exec_ARM = cpu_ARM_MUL_SL;
								cpu_Calculate_Mul_Cycles_SL();
								break;

							case 0x4:
							case 0x5:
								// Swap
								cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_Swap_ARM;
								cpu_Next_Load_Store_Type = cpu_Swap_ARM;
								cpu_Exec_ARM = cpu_ARM_Swap;
								cpu_Swap_Store = false;
								break;

							case 0x6:
							case 0x7:
								// Undefined Opcode Exception
								cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
								cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
								cpu_Exception_Type = cpu_Undef_Exc;
								break;
						}
					}
					else
					{
						// halfword or byte transfers
						cpu_Exec_ARM = cpu_ARM_Imm_LS;
						cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

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
					cpu_LDM_Glitch_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

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
						if (cpu_LDM_Glitch_Instr_Type == cpu_Internal_And_Prefetch_ARM) { cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_2_ARM; }
						if (cpu_LDM_Glitch_Instr_Type == cpu_Internal_And_Branch_2_ARM) { cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Branch_3_ARM; }
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
								cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_ARM;
								cpu_Exec_ARM = cpu_ARM_MRS;
								break;
							case 0x9:
								if ((cpu_Instr_ARM_2 & 0XFFFF0) == 0xFFF10)
								{
									// Branch and exchange
									cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_Branch_Ex_ARM;
									cpu_Exec_ARM = cpu_ARM_Bx;
								}
								else
								{
									cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
									cpu_Exec_ARM = cpu_ARM_MSR;
								}
								break;
							case 0xA:
								cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_ARM;
								cpu_Exec_ARM = cpu_ARM_MRS;
								break;
							case 0xB:
								cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
								cpu_Exec_ARM = cpu_ARM_MSR;
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

				cpu_LDM_Glitch_Store = true;

				// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
				cpu_LDM_Glitch_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

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
							// Undefined Opcode Exception
							cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
							cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
							cpu_Exception_Type = cpu_Undef_Exc;
							break;
						case 0x9:
							cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR;
							break;
						case 0xA:
							// Undefined Opcode Exception
							cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
							cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
							cpu_Exception_Type = cpu_Undef_Exc;
							break;
						case 0xB:
							cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR;
							break;
					}
				}
				break;

			case 2:
				// load / store immediate offset
				cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

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
					cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

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
					cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				break;

			case 4:
				// block transfer
				cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;
				cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_ARM;
				cpu_Exec_ARM = cpu_ARM_Multi_1;
				break;

			case 5:
				// branch
				cpu_LDM_Glitch_Instr_Type = cpu_Internal_And_Branch_1_ARM;
				cpu_Exec_ARM = cpu_ARM_Branch;
				break;

			case 6:
				// Coprocessor Instruction (treat as Undefined Opcode Exception)
				cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
				cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
				cpu_Exception_Type = cpu_Undef_Exc;
				break;

			case 7:
				if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
				{
					// software interrupt
					cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_SWI_Exc;
				}
				else
				{
					// Coprocessor Instruction (treat as Undefined Opcode Exception)
					cpu_LDM_Glitch_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				break;

			default:
				cpu_LDM_Glitch_Instr_Type = cpu_Internal_Halted;
				break;
		}

		cpu_LDM_Glitch_Mode = false;

		if (TraceCallback) TraceCallback(0);
	}

	void GBA_System::cpu_LDM_Glitch_Execute_Internal_Only_ARM()
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

					//if (cpu_LDM_Glitch_Store) { cpu_LDM_Glitch_Set_Reg((uint)cpu_ALU_Reg_Dest, (uint)cpu_ALU_Long_Result); }
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
					cpu_Base_Reg = (cpu_Instr_ARM_2 & 0xF);

					cpu_Temp_Reg = cpu_LDM_Glitch_Get_Reg(cpu_Base_Reg);
					break;

				case cpu_ARM_MUL:
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

					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MUL_UL:
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

				case cpu_ARM_MUL_SL:
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

				case cpu_ARM_Swap:
					cpu_Base_Reg = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 16) & 0xF);
					cpu_Base_Reg_2 = ((cpu_Instr_ARM_2 >> 12) & 0xF);
					cpu_Temp_Reg_Ptr = (cpu_Instr_ARM_2 & 0xF);
					break;

				case cpu_ARM_Imm_LS:
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

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}

					break;

				case cpu_ARM_Reg_LS:
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

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}
					break;

				case cpu_ARM_Multi_1:
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

	void GBA_System::cpu_LDM_Glitch_Tick()
	{
		switch (cpu_LDM_Glitch_Instr_Type)
		{
		case cpu_Internal_And_Prefetch_ARM:
			// In this code path the instruction takes only one internal cycle, a pretech is also done
			// so necessarily the condition code check happens here, and interrupts may occur
			if (cpu_Fetch_Cnt == 0)
			{
				cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

				cpu_IRQ_Input_Use = cpu_IRQ_Input;
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
				cpu_Regs[15] += 4;

				cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
				cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

				if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
				cpu_FlagI_Old = cpu_FlagIget();

				cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

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

					if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
				cpu_FlagI_Old = cpu_FlagIget();

				cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

				cpu_IRQ_Input_Use = cpu_IRQ_Input;
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

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

						if (cpu_FlagTget()) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
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

						if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = true;
					}
				}
				else
				{
					cpu_Regs[15] += 4;

					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }

					cpu_Seq_Access = true;
				}

				cpu_Fetch_Cnt = 0;
				cpu_Fetch_Wait = 0;
			}
			break;

			// NOTE: From glitched path this always goes back to the non-glitched path
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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }

					cpu_Seq_Access = true;
				}

				cpu_Fetch_Cnt = 0;
				cpu_Fetch_Wait = 0;
			}
			break;

		case cpu_Prefetch_And_Load_Store_ARM:
			// First cycle of load / store, cannot be interrupted (if executed), prefetch occurs
			if (cpu_Fetch_Cnt == 0)
			{
				cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

				cpu_IRQ_Input_Use = cpu_IRQ_Input;
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
				cpu_Regs[15] += 4;

				if (cpu_ARM_Cond_Passed)
				{
					cpu_LDM_Glitch_Instr_Type = cpu_Next_Load_Store_Type;
					cpu_Seq_Access = false;
				}
				else
				{
					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
							cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
								cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFF0000);
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
								cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFFFF00);
							}

							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
						else
						{
							cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

							cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));

							cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
					}
				}
				else
				{
					Write_Memory_16(cpu_Temp_Addr, (uint16_t)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFFFF));
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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
							cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFFFF00);
						}
					}

					cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
				}
				else
				{
					Write_Memory_8(cpu_Temp_Addr, (uint8_t)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFF));
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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
							cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size) - 4);
						}
						else
						{
							cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size));
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
					}

					if (cpu_LS_Is_Load)
					{
						if (cpu_Regs_To_Access[cpu_Multi_List_Ptr - 1] == 15)
						{
							// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
							if (cpu_Multi_S_Bit)
							{
								cpu_Regs[16] = cpu_Regs[17];

								if (cpu_FlagTget()) { cpu_Thumb_Mode = true; }
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
						// stm does not cause the glitch
						cpu_LDM_Glitch_Mode = false;
						
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

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

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

				cpu_IRQ_Input_Use = cpu_IRQ_Input;
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
				cpu_Regs[15] += 4;

				cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
				cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

				if (cpu_ARM_Cond_Passed) { cpu_LDM_Glitch_Instr_Type = cpu_Multiply_Cycles; }
				else
				{
					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

				cpu_IRQ_Input_Use = cpu_IRQ_Input;
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
				cpu_Regs[15] += 4;

				if (cpu_ARM_Cond_Passed)
				{
					cpu_LDM_Glitch_Instr_Type = cpu_Next_Load_Store_Type;
					cpu_Seq_Access = false;

					// Lock the bus
					cpu_Swap_Lock = true;
				}
				else
				{
					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
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
					cpu_Fetch_Wait = Wait_State_Access_32(cpu_Base_Reg, cpu_Seq_Access);
				}
				else
				{
					cpu_Fetch_Wait = Wait_State_Access_8(cpu_Base_Reg, cpu_Seq_Access);
				}
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				if (cpu_Swap_Store)
				{
					if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
					{
						Write_Memory_8(cpu_Base_Reg, (uint8_t)cpu_Regs[cpu_Temp_Reg_Ptr]);
					}
					else
					{
						Write_Memory_32(cpu_Base_Reg, cpu_Regs[cpu_Temp_Reg_Ptr]);
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
						cpu_Temp_Data = Read_Memory_8(cpu_Base_Reg);
					}
					else
					{
						// deal with misaligned accesses
						cpu_Temp_Addr = cpu_Base_Reg;

						if ((cpu_Temp_Addr & 3) == 0)
						{
							cpu_Temp_Data = Read_Memory_32(cpu_Temp_Addr);
						}
						else
						{
							cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

							for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
							{
								cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
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

				cpu_LDM_Glitch_Execute_Internal_Only_ARM();

				cpu_IRQ_Input_Use = cpu_IRQ_Input;
			}

			cpu_Fetch_Cnt += 1;

			if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				if (cpu_ARM_Cond_Passed)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

					cpu_Regs[15] = cpu_Temp_Reg & 0xFFFFFFFE;

					cpu_FlagTset((cpu_Temp_Reg & 1) == 1);
					cpu_Thumb_Mode = cpu_FlagTget();

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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }

					cpu_Seq_Access = true;
				}

				cpu_Fetch_Cnt = 0;
				cpu_Fetch_Wait = 0;
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
					cpu_LDM_Glitch_Execute_Internal_Only_ARM();
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
						if (TraceCallback) TraceCallback(1); // SWI

						// supervisor mode
						cpu_Swap_Regs(0x13, true, false);

						// R14 becomes return address
						cpu_Regs[14] = cpu_Thumb_Mode ? (cpu_Regs[15] - 2) : (cpu_Regs[15] - 4);

						// take exception vector 0x8
						cpu_Regs[15] = 0x00000008;
					}
					else
					{
						if (TraceCallback) TraceCallback(2); // UDF

						// undefined instruction mode
						cpu_Swap_Regs(0x1B, true, false);

						// R14 becomes return address
						cpu_Regs[14] = cpu_Thumb_Mode ? (cpu_Regs[15] - 2) : (cpu_Regs[15] - 4);

						// take exception vector 0x4
						cpu_Regs[15] = 0x00000004;
					}

					// Normal Interrupts disabled
					cpu_FlagIset(true);

					// switch into ARM mode
					cpu_Thumb_Mode = false;
					cpu_FlagTset(false);

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

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
				}

				cpu_Fetch_Cnt = 0;
				cpu_Fetch_Wait = 0;
			}
			break;

		case cpu_Prefetch_IRQ:
			// IRQ uses a prefetch cycle not an internal cycle (just like swi and undef)
			if (cpu_Fetch_Cnt == 0)
			{
				if (TraceCallback) TraceCallback(3); // IRQ

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
				cpu_FlagIset(true);

				// take exception vector 0x18
				cpu_Regs[15] = 0x00000018;

				// switch into ARM mode
				cpu_Thumb_Mode = false;
				cpu_FlagTset(false);

				// Invalidate instruction pipeline
				cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;

				cpu_Seq_Access = false;

				cpu_Fetch_Cnt = 0;
				cpu_Fetch_Wait = 0;
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
				if (cpu_Internal_Save_Access) {}

				// next instruction was already prefetched, decode it here
				cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
				cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

				if (cpu_LDM_Glitch_Mode)
				{
					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; cpu_LDM_Glitch_Mode = false; }
					else { cpu_LDM_Glitch_Decode_ARM(); }
				}
				else
				{
					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }
				}
			}

			cpu_Internal_Save_Access = false;
			cpu_Invalidate_Pipeline = false;
			break;

		case cpu_Multiply_Cycles:
			// cycles of the multiply instruction
			// check for IRQs at the end
			cpu_Mul_Cycles_Cnt += 1;

			if (cpu_Mul_Cycles_Cnt == cpu_Mul_Cycles)
			{
				cpu_IRQ_Input_Use = cpu_IRQ_Input;

				if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
				else if (cpu_Thumb_Mode) { cpu_Decode_TMB(); }
				else { cpu_Decode_ARM(); }

				cpu_Mul_Cycles_Cnt = 0;

				// Multiply forces the next access to be non-sequential
				cpu_Seq_Access = false;
			}
			break;

			// Check timing?
		case cpu_Pause_For_DMA:
			if (dma_Held_CPU_LDM_Glitch_Instr >= 42)
			{
				switch (dma_Held_CPU_LDM_Glitch_Instr)
				{
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
						if (cpu_Internal_Save_Access) {}

						// next instruction was already prefetched, decode it here
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_LDM_Glitch_Mode)
						{
							if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; cpu_LDM_Glitch_Mode = false; }
							else { cpu_LDM_Glitch_Decode_ARM(); }
						}
						else
						{
							if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }
						}
					}

					cpu_Internal_Save_Access = false;
					cpu_Invalidate_Pipeline = false;
					break;

				case cpu_Multiply_Cycles:
					// cycles of the multiply instruction
					// check for IRQs at the end
					cpu_Mul_Cycles_Cnt += 1;

					if (cpu_Mul_Cycles_Cnt == cpu_Mul_Cycles)
					{
						cpu_IRQ_Input_Use = cpu_IRQ_Input;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else if (cpu_Thumb_Mode) { cpu_Decode_TMB(); }
						else { cpu_Decode_ARM(); }

						cpu_Mul_Cycles_Cnt = 0;

						// Multiply forces the next access to be non-sequential
						cpu_Seq_Access = false;
					}
					break;
				}

				if (cpu_LDM_Glitch_Instr_Type != cpu_Pause_For_DMA)
				{
					// change the DMA held instruction
					dma_Held_CPU_LDM_Glitch_Instr = cpu_LDM_Glitch_Instr_Type;
					cpu_LDM_Glitch_Instr_Type = cpu_Pause_For_DMA;
				}
			}

			if (!cpu_Is_Paused) { cpu_LDM_Glitch_Instr_Type = dma_Held_CPU_LDM_Glitch_Instr; }
			break;
		}
	}
}