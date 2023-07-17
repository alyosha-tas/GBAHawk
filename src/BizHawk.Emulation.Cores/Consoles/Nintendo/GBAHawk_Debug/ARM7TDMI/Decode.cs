using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		public void cpu_Decode_ARM()
		{
			switch (((cpu_Instr_ARM_2 >> 25) & 7) + cpu_HS_Ofst_ARM2)
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
									if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
									{
										cpu_Mul_Cycles = 2;
									}
									else
									{
										cpu_Calculate_Mul_Cycles();
									}
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

						cpu_ALU_Reg_Dest = (int)((cpu_Instr_ARM_2 >> 12) & 0xF);

						cpu_ALU_Reg_Src = (int)((cpu_Instr_ARM_2 >> 16) & 0xF);
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

						cpu_ALU_Shift_Carry = (uint)(cpu_FlagC ? 1 : 0);

						switch ((cpu_Instr_ARM_2 >> 5) & 3)
						{
							case 0:         // LSL
								// calculate immedaite value
								if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
								{
									// immediate shift
									cpu_Shift_Imm = (int)((cpu_Instr_ARM_2 >> 7) & 0x1F);
								}
								else
								{
									// register shift
									cpu_Shift_Imm = (int)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);								
								}

								cpu_ALU_Long_Result = cpu_ALU_Long_Result << cpu_Shift_Imm;
								
								if (cpu_Shift_Imm != 0) 
								{ 
									cpu_ALU_Shift_Carry = (uint)((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare ? 1 : 0);
								}
								break;

							case 1:         // LSR
								// calculate immedaite value
								if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
								{
									// immediate shift
									cpu_Shift_Imm = (int)((cpu_Instr_ARM_2 >> 7) & 0x1F);
									
									if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
								}
								else
								{
									// register shift
									cpu_Shift_Imm = (int)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
								}

								if (cpu_Shift_Imm != 0) 
								{
									cpu_ALU_Shift_Carry = (uint)((cpu_ALU_Long_Result >> (cpu_Shift_Imm - 1)) & 1);
									cpu_ALU_Long_Result = cpu_ALU_Long_Result >> cpu_Shift_Imm;
								}
								break;

							case 2:         // ASR
								// calculate immedaite value
								if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
								{
									// immediate shift
									cpu_Shift_Imm = (int)((cpu_Instr_ARM_2 >> 7) & 0x1F);

									if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
								}
								else
								{
									// register shift
									cpu_Shift_Imm = (int)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
								}

								cpu_ALU_Temp_S_Val = (uint)(cpu_ALU_Long_Result & cpu_Neg_Compare);

								for (int i = 1; i <= cpu_Shift_Imm; i++)
								{
									cpu_ALU_Shift_Carry = (uint)(cpu_ALU_Long_Result & 1);
									cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
									cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
								}
								break;

							case 3:         // RRX
								// calculate immedaite value
								if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
								{
									// immediate shift
									cpu_Shift_Imm = (int)((cpu_Instr_ARM_2 >> 7) & 0x1F);

									if (cpu_Shift_Imm == 0) { is_RRX = true; }
								}
								else
								{
									// register shift
									cpu_Shift_Imm = (int)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
								}

								if (is_RRX)
								{
									cpu_ALU_Shift_Carry = (uint)(cpu_ALU_Long_Result & 1);
									cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
									cpu_ALU_Long_Result |= cpu_FlagC ? 0x80000000 : 0;
								}
								else
								{
									for (int i = 1; i <= cpu_Shift_Imm; i++)
									{
										cpu_ALU_Shift_Carry = (uint)(cpu_ALU_Long_Result & 1);
										cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
										cpu_ALU_Long_Result |= (cpu_ALU_Shift_Carry << 31);
									}
								}					
								break;
						}

						cpu_ALU_Temp_Val = (uint)cpu_ALU_Long_Result;

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

					cpu_ALU_Reg_Dest = (int)((cpu_Instr_ARM_2 >> 12) & 0xF);

					cpu_ALU_Reg_Src = (int)((cpu_Instr_ARM_2 >> 16) & 0xF);
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

					cpu_ALU_Shift_Carry = (uint)(cpu_FlagC ? 1 : 0);

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

			if ((TraceCallback != null) && (cpu_HS_Ofst_ARM2 == 0)) TraceCallback(State());

			cpu_HS_Ofst_ARM2 = cpu_HS_Ofst_ARM1;
			cpu_HS_Ofst_ARM1 = cpu_HS_Ofst_ARM0;
		}

		public void cpu_Decode_TMB()
		{
			switch (((cpu_Instr_TMB_2 >> 13) & 7) + cpu_HS_Ofst_TMB2)
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
											throw new Exception("undefined instruction (CPY) 01000110 " + cpu_Regs[15]);
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
											throw new Exception("undefined instruction (BLX) 010001111 " + cpu_Regs[15]);
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
						cpu_Exec_TMB = cpu_Thumb_Half_LS;
						cpu_Sign_Extend_Load = false;
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

			if ((TraceCallback != null) && (cpu_HS_Ofst_TMB2 == 0)) TraceCallback(State());

			cpu_HS_Ofst_TMB2 = cpu_HS_Ofst_TMB1;
			cpu_HS_Ofst_TMB1 = cpu_HS_Ofst_TMB0;
		}

		public void cpu_Calculate_Mul_Cycles()
		{
			uint temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

			if (cpu_Thumb_Mode)
			{
				temp_calc = cpu_Regs[cpu_ALU_Reg_Dest];
			}
			
			// all F's seems to be a special case
			if ((temp_calc & 0xFF000000) == 0xFF000000)
			{
				cpu_Mul_Cycles = 3;

				if ((temp_calc & 0x00FF0000) == 0x00FF0000)
				{
					cpu_Mul_Cycles -= 1;

					if ((temp_calc & 0x0000FF00) == 0x0000FF00)
					{
						cpu_Mul_Cycles -= 1;
					}
				}		
			}
			else if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 2;
			}
			else
			{
				cpu_Mul_Cycles = 1;
			}
		}

		// seems to be based on the number of non-zero upper bits
		public void cpu_Calculate_Mul_Cycles_UL()
		{
			uint temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

			if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 5;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else
			{
				cpu_Mul_Cycles = 2;
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		public void cpu_Calculate_Mul_Cycles_SL()
		{
			uint temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

			if ((temp_calc & 0x80000000) == 0)
			{
				// seems to be based on the number of non-zero upper bits if sign bit is zero
				if ((temp_calc & 0xFF000000) != 0)
				{
					cpu_Mul_Cycles = 5;
				}
				else if ((temp_calc & 0x00FF0000) != 0)
				{
					cpu_Mul_Cycles = 4;
				}
				else if ((temp_calc & 0x0000FF00) != 0)
				{
					cpu_Mul_Cycles = 3;
				}
				else
				{
					cpu_Mul_Cycles = 2;
				}
			}
			else
			{
				// seems to be based on the number of non-zero lower bits if sign bit is not zero
				if ((temp_calc & 0x00FFFFFF) == 0)
				{
					cpu_Mul_Cycles = 4;
				}
				else if ((temp_calc & 0x0000FFFF) == 0)
				{
					cpu_Mul_Cycles = 3;
				}
				else if ((temp_calc & 0x000000FF) == 0)
				{
					cpu_Mul_Cycles = 2;
				}
				else
				{
					cpu_Mul_Cycles = 2;
				}
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}
	}
}