using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk
{
	public partial class GBAHawk
	{
		public void cpu_Execute_Internal_Only_ARM()
		{
			// Do Tracer Stuff here
			int ofst = 0;

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
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_EOR:
						cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_SUB:
						cpu_ALU_Long_Result = cpu_Temp_Reg;
						cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_RSB:
						cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
						cpu_ALU_Long_Result -= cpu_Temp_Reg;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint)cpu_ALU_Long_Result);
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_ADD:
						cpu_ALU_Long_Result = cpu_Temp_Reg;
						cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_ADC:
						cpu_ALU_Long_Result = cpu_Temp_Reg;
						cpu_ALU_Long_Result += cpu_ALU_Temp_Val;
						cpu_ALU_Long_Result += (ulong)(cpu_FlagC ? 1 : 0);

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_SBC:
						cpu_ALU_Long_Result = cpu_Temp_Reg;
						cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;
						cpu_ALU_Long_Result -= (ulong)(cpu_FlagC ? 0 : 1);

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_RSC:
						cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
						cpu_ALU_Long_Result -= cpu_Temp_Reg;
						cpu_ALU_Long_Result -= (ulong)(cpu_FlagC ? 0 : 1);

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint)cpu_ALU_Long_Result);
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_TST:
						cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}
						break;

					case cpu_ARM_TEQ:
						cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}
						break;

					case cpu_ARM_CMP:
						cpu_ALU_Long_Result = cpu_Temp_Reg;
						cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						}
						break;

					case cpu_ARM_CMN:
						cpu_ALU_Long_Result = cpu_Temp_Reg;
						cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit)
						{
							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						}
						break;

					case cpu_ARM_ORR:
						cpu_ALU_Long_Result = cpu_Temp_Reg | cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_MOV:
						cpu_ALU_Long_Result = cpu_ALU_Temp_Val;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_BIC:
						cpu_ALU_Long_Result = cpu_Temp_Reg & (~cpu_ALU_Temp_Val);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_MVN:
						cpu_ALU_Long_Result = ~cpu_ALU_Temp_Val;

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
						{
							cpu_FlagC = cpu_ALU_Shift_Carry == 1;
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_MSR:
						uint byte_mask = 0;
						uint total_mask = 0;

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

							cpu_Swap_Regs(((cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask)) & 0x1F, false, false);
							cpu_Regs[16] = (cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);

							cpu_Thumb_Mode = cpu_FlagT;
						}
						else
						{
							// user (unpriviliged)
							if ((cpu_Regs[16] & 0x1F) == 0x10)
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
							cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[17];
						}
						break;

					case cpu_ARM_Bx:
						// Branch and exchange mode (possibly)
						cpu_Base_Reg = (int)(cpu_Instr_ARM_2 & 0xF);

						cpu_Temp_Reg = (cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFE);
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
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
						}
							
						cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint)cpu_ALU_Long_Result;
						break;

					case cpu_ARM_MUL_UL:
						cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
						cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

						if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
						{
							
							ulong a1 = cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
							ulong a2 = cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF];
							a2 = a2 << 32;
							cpu_ALU_Long_Result += (a1 + a2);
						}

						if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
						{
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1
							// then it does not change (probably multiplier takes a short cut for these cases)
							// and changes differently if it is -1
							long sf1 = (int)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
							long sf2 = (int)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
							if (sf2 == -1)
							{
								// maybe its a carry from lower 16 bits)???
								if ((sf1 & 0xFFFF) >= 2) { cpu_FlagC = true; }
								else { cpu_FlagC = false; }
							}
							else if ((sf2 != 0) && (sf2 != 1))
							{
								cpu_FlagC = ((sf1 & 0x80000000) == 0x80000000);
							}
						}

						cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint)cpu_ALU_Long_Result;
						cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint)(cpu_ALU_Long_Result >> 32);
						break;

					case cpu_ARM_MUL_SL:
						long s1 = (int)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						long s2 = (int)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

						cpu_ALU_Signed_Long_Result = s1 * s2;

						cpu_ALU_Long_Result = (ulong)cpu_ALU_Signed_Long_Result;

						if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
						{
							ulong a1 = cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
							ulong a2 = cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF];
							a2 = a2 << 32;
							cpu_ALU_Long_Result += (a1 + a2);
						}

						if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
						{
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1,-1
							// then it does not change (probably multiplier takes a short cut for these cases)
							if ((s2 != 0) && (s2 != 1) && (s2 != -1))
							{
								cpu_FlagC = !((s1 & 0x80000000) == (s2 & 0x80000000));
							}
						}

						cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint)cpu_ALU_Long_Result;
						cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint)(cpu_ALU_Long_Result >> 32);
						break;

					case cpu_ARM_Swap:
						cpu_Temp_Reg_Ptr = (int)((cpu_Instr_ARM_2 >> 12) & 0xF);
						cpu_Base_Reg = (int)((cpu_Instr_ARM_2 >> 16) & 0xF);
						cpu_Base_Reg_2 = (int)(cpu_Instr_ARM_2 & 0xF);
						break;

					case cpu_ARM_Imm_LS:
						cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

						cpu_Base_Reg = (int)((cpu_Instr_ARM_2 >> 16) & 0xF);

						cpu_Temp_Reg_Ptr = (int)((cpu_Instr_ARM_2 >> 12) & 0xF);

						if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
						{
							// increment first
							if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
							{
								cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] + cpu_Addr_Offset);
							}
							else
							{
								cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] - cpu_Addr_Offset);
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
								cpu_Write_Back_Addr = (uint)(cpu_Temp_Addr + cpu_Addr_Offset);
							}
							else
							{
								cpu_Write_Back_Addr = (uint)(cpu_Temp_Addr - cpu_Addr_Offset);
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

						cpu_Base_Reg = (int)((cpu_Instr_ARM_2 >> 16) & 0xF);

						cpu_Temp_Reg_Ptr = (int)((cpu_Instr_ARM_2 >> 12) & 0xF);

						cpu_Temp_Data = cpu_Regs[cpu_Instr_ARM_2 & 0xF];

						cpu_Shift_Imm = (int)((cpu_Instr_ARM_2 >> 7) & 0x1F);

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
									cpu_Temp_Data |= cpu_FlagC ? 0x80000000 : 0;
								}
								else
								{
									for (int i = 1; i <= cpu_Shift_Imm; i++)
									{
										cpu_ALU_Temp_S_Val = (uint)(cpu_Temp_Data & 1);
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
								cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] + cpu_Temp_Data);
							}
							else
							{
								cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] - cpu_Temp_Data);
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
								cpu_Write_Back_Addr = (uint)(cpu_Temp_Addr + cpu_Temp_Data);
							}
							else
							{
								cpu_Write_Back_Addr = (uint)(cpu_Temp_Addr - cpu_Temp_Data);
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

						cpu_Base_Reg = (int)((cpu_Instr_ARM_2 >> 16) & 0xF);

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
						bool Use_Reg_15 = false;

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
							cpu_Temp_Addr -= (uint)((cpu_Multi_List_Size - 1) * 4);

							// in either case, write back is the same
							cpu_Write_Back_Addr = (uint)(cpu_Temp_Addr - 4);

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
						ofst = (int)((cpu_Instr_ARM_2 & 0xFFFFFF) << 2);

						// offset is signed
						if ((ofst & 0x2000000) == 0x2000000) { ofst = (int)(ofst | 0xFC000000); }

						cpu_Temp_Reg = (uint)(cpu_Regs[15] + ofst);

						// Link if link bit set
						if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
						{
							cpu_Regs[14] = (uint)(cpu_Regs[15] - 4);
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

		public void cpu_Execute_Internal_Only_TMB()
		{
			int ofst = 0;

			// Do Tracer stuff here
			switch (cpu_Exec_TMB)
			{
				case cpu_Thumb_Shift:
					cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

					cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

					cpu_ALU_Temp_Val = (uint)((cpu_Instr_TMB_2 >> 6) & 0x1F);

					switch ((cpu_Instr_TMB_2 >> 11) & 0x3)
					{
						case 0:
							cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
							cpu_ALU_Long_Result = cpu_ALU_Long_Result << (int)cpu_ALU_Temp_Val;

							if (cpu_ALU_Temp_Val != 0) { cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare; }
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
							break;

						case 1:	
							if (cpu_ALU_Temp_Val != 0)
							{
								cpu_FlagC = ((cpu_Regs[cpu_ALU_Reg_Src] >> (int)(cpu_ALU_Temp_Val - 1)) & 1) == 1;
								cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src] >> (int)cpu_ALU_Temp_Val;
							}
							else
							{
								cpu_FlagC = (cpu_Regs[cpu_ALU_Reg_Src] & cpu_Neg_Compare) == cpu_Neg_Compare;
								cpu_ALU_Long_Result = 0;
							}

							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
							break;

						case 2:
							cpu_ALU_Temp_S_Val = cpu_Regs[cpu_ALU_Reg_Src] & cpu_Neg_Compare;

							cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];

							if (cpu_ALU_Temp_Val == 0) { cpu_ALU_Temp_Val = 32;}

							for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
							{
								cpu_FlagC = (cpu_ALU_Long_Result & 1) == 1;
								cpu_ALU_Long_Result = cpu_ALU_Long_Result >> 1;
								cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
							}

							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
							break;
					}
					break;

				case cpu_Thumb_Add_Sub_Reg:
					cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

					cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

					if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
					{
						cpu_ALU_Temp_Val = (uint)((cpu_Instr_TMB_2 >> 6) & 0x7);
					}
					else
					{
						cpu_ALU_Temp_Val = cpu_Regs[(cpu_Instr_TMB_2 >> 6) & 0x7];
					}

					if ((cpu_Instr_TMB_2 & 0x200) == 0x200)
					{
						cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
						cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

						cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Src], cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
						cpu_FlagZ = cpu_ALU_Long_Result == 0;

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					}
					else
					{
						cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
						cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

						cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Src], cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
						cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
						cpu_FlagZ = cpu_ALU_Long_Result == 0;

						cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					}
					break;

				case cpu_Thumb_AND:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_EOR:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] ^ cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_LSL:
					cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result = cpu_ALU_Long_Result << (int)cpu_ALU_Temp_Val;

					if (cpu_ALU_Temp_Val != 0) { cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare; }			
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_LSR:
					cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

					// NOTE: This is necessary due to C# only using lower 5 bits of shift count on integer sized values.
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
	
					if (cpu_ALU_Temp_Val != 0)
					{
						cpu_FlagC = ((cpu_ALU_Long_Result >> (int)(cpu_ALU_Temp_Val - 1)) & 1) == 1;
					}

					cpu_ALU_Long_Result = cpu_ALU_Long_Result >> (int)cpu_ALU_Temp_Val;

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_ASR:
					cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;
					cpu_ALU_Temp_S_Val = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Neg_Compare;

					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

					for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
					{
						cpu_FlagC = (cpu_ALU_Long_Result & 1) == 1;
						cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
						cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
					}

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_ADC:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];
					cpu_ALU_Long_Result += (ulong)(cpu_FlagC ? 1 : 0);

					cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint)cpu_ALU_Long_Result);
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_SBC:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];
					cpu_ALU_Long_Result -= (ulong)(cpu_FlagC ? 0 : 1);

					cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint)cpu_ALU_Long_Result);
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_ROR:
					cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

					for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
					{
						cpu_ALU_Temp_S_Val = (uint)(cpu_ALU_Long_Result & 1);
						cpu_ALU_Long_Result = cpu_ALU_Long_Result >> 1;
						cpu_ALU_Long_Result |= (cpu_ALU_Temp_S_Val << 31);
					}

					if (cpu_ALU_Temp_Val != 0) { cpu_FlagC = cpu_ALU_Temp_S_Val == 1; }

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_TST:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;
					break;

				case cpu_Thumb_NEG:
					cpu_ALU_Long_Result = 0;
					cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagV = cpu_Calc_V_Flag_Sub(0, cpu_Regs[cpu_ALU_Reg_Src], (uint)cpu_ALU_Long_Result);
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_CMP:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint)cpu_ALU_Long_Result);
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;
					break;

				case cpu_Thumb_CMN:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint)cpu_ALU_Long_Result);
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;
					break;

				case cpu_Thumb_ORR:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] | cpu_Regs[cpu_ALU_Reg_Src];

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_MUL:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] * cpu_Regs[cpu_ALU_Reg_Src];

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_BIC:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & (~cpu_Regs[cpu_ALU_Reg_Src]);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
					break;

				case cpu_Thumb_MVN:
					cpu_ALU_Long_Result = (~cpu_Regs[cpu_ALU_Reg_Src]);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
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

						cpu_Temp_Reg = (uint)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);
					}
					else
					{
						cpu_Take_Branch = false;
						cpu_Regs[cpu_Base_Reg] = (uint)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);
					}				
					break;

				case cpu_Thumb_High_Cmp:
					// Sub but no flags change
					// unpredictable if first register is 15?
					cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

					cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 3) & 0xF);

					cpu_ALU_Long_Result = cpu_Regs[cpu_Base_Reg];
					cpu_ALU_Long_Result -= cpu_Regs[cpu_Base_Reg_2];

					cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_Base_Reg], cpu_Regs[cpu_Base_Reg_2], (uint)cpu_ALU_Long_Result);
					cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
					cpu_FlagZ = cpu_ALU_Long_Result == 0;
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

					cpu_ALU_Temp_Val = (uint)(cpu_Instr_TMB_2 & 0xFF);

					switch ((cpu_Instr_TMB_2 >> 11) & 3)
					{
						case 0:			// MOV
							cpu_Regs[cpu_ALU_Reg_Dest] = cpu_ALU_Temp_Val;
							cpu_FlagN = false;
							cpu_FlagZ = cpu_ALU_Temp_Val == 0;
							break;

						case 1:         // CMP
							cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
							cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;
							break;

						case 2:         // ADD
							cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
							cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare;
							
							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagV = cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
							break;

						case 3:         // SUB
							cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
							cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

							cpu_FlagC = (cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare;

							cpu_ALU_Long_Result &= cpu_Cast_Int;

							cpu_FlagV = cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint)cpu_ALU_Long_Result);
							cpu_FlagN = (cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare;
							cpu_FlagZ = cpu_ALU_Long_Result == 0;

							cpu_Regs[cpu_ALU_Reg_Dest] = (uint)cpu_ALU_Long_Result;
							break;
					}
					break;

				case cpu_Thumb_PC_Rel_LS:
					cpu_LS_Is_Load = true;

					cpu_Base_Reg = 15;

					cpu_Temp_Addr = (uint)((cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));

					cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
					break;

				case cpu_Thumb_Rel_LS:	
					switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
					{
						case 0: cpu_LS_Is_Load = false; break;
						case 1: cpu_LS_Is_Load = false; break;
						case 2: cpu_LS_Is_Load = false; break;
						case 3: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
						case 4: cpu_LS_Is_Load = true; break;
						case 5: cpu_LS_Is_Load = true; break;
						case 6: cpu_LS_Is_Load = true; break;
						case 7: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
					}

					cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);
					cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 6) & 7);

					cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);

					cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
					break;

				case cpu_Thumb_Imm_LS:	
					cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

					cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

					if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
					{
						cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 6) & 0x1F));
					}
					else
					{
						cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 4) & 0x7C));
					}
					

					cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
					break;

				case cpu_Thumb_Half_LS:
					cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

					cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

					cpu_Temp_Addr = (uint)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 5) & 0x3E));

					cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
					break;

				case cpu_Thumb_SP_REL_LS:
					cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

					cpu_Temp_Addr = (uint)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));

					cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
					break;

				case cpu_Thumb_Add_SP_PC:
					cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 8) & 7);

					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						cpu_Regs[cpu_Base_Reg] = (uint)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));				
					}
					else
					{
						cpu_Regs[cpu_Base_Reg] = (uint)((cpu_Regs[15] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));
					}
					break;

				case cpu_Thumb_Add_Sub_Stack:
					if ((cpu_Instr_TMB_2 & 0x80) == 0x0)
					{
						cpu_Regs[13] += (uint)((cpu_Instr_TMB_2 & 0x7F) << 2);
					}
					else
					{
						cpu_Regs[13] -= (uint)((cpu_Instr_TMB_2 & 0x7F) << 2);
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
						cpu_Temp_Addr -= (uint)((cpu_Multi_List_Size - 1) * 4);
						cpu_Temp_Addr -= 4;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// No registers selected is unpredictable
					if (cpu_Multi_List_Size == 0)
					{
						throw new Exception("no registers selected in Multi Load/Store at " + cpu_Regs[15]);
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
					ofst = (int)((cpu_Instr_TMB_2 & 0x7FF) << 1);

					// offset is signed
					if ((ofst & 0x800) == 0x800) { ofst = (int)(ofst | 0xFFFFF000); }

					cpu_Temp_Reg = (uint)(cpu_Regs[15] + ofst);

					cpu_Take_Branch = true;
					break;

				case cpu_Thumb_Branch_Cond:
					ofst = (int)((cpu_Instr_TMB_2 & 0xFF) << 1);

					// offset is signed
					if ((ofst & 0x100) == 0x100) { ofst = (int)(ofst | 0xFFFFFE00); }

					cpu_Temp_Reg = (uint)(cpu_Regs[15] + ofst);

					cpu_Take_Branch = cpu_TMB_Condition_Check();
					break;

				case cpu_Thumb_Branch_Link_1:
					ofst = (int)((cpu_Instr_TMB_2 & 0x7FF) << 12);
					
					// offset is signed
					if ((ofst & 0x400000) == 0x400000) { ofst = (int)(ofst | 0xFF800000); }
					
					cpu_Regs[14] = (uint)(cpu_Regs[15] + ofst);					
					break;

				case cpu_Thumb_Branch_Link_2:
					ofst = (int)((cpu_Instr_TMB_2 & 0x7FF) << 1);

					cpu_Temp_Reg = (uint)(cpu_Regs[14] + ofst);

					// NOTE: OR with 1, probably reuses the same cpu circuitry that triggers switch to Thumb mode when writing to R[15] directly?
					cpu_Regs[14] = (uint)((cpu_Regs[15] - 2) | 1);

					cpu_Take_Branch = true;
					break;
			}
		}

		public bool cpu_ARM_Condition_Check()
		{
			switch (cpu_Instr_ARM_2 & 0xF0000000)
			{
				case 0x00000000: return cpu_FlagZ;
				case 0x10000000: return !cpu_FlagZ;
				case 0x20000000: return cpu_FlagC;
				case 0x30000000: return !cpu_FlagC;
				case 0x40000000: return cpu_FlagN;
				case 0x50000000: return !cpu_FlagN;
				case 0x60000000: return cpu_FlagV;
				case 0x70000000: return !cpu_FlagV;
				case 0x80000000: return (cpu_FlagC & !cpu_FlagZ);
				case 0x90000000: return (!cpu_FlagC | cpu_FlagZ);
				case 0xA0000000: return (cpu_FlagN == cpu_FlagV);
				case 0xB0000000: return (cpu_FlagN != cpu_FlagV);
				case 0xC0000000: return (!cpu_FlagZ & (cpu_FlagN == cpu_FlagV));
				case 0xD0000000: return (cpu_FlagZ | (cpu_FlagN != cpu_FlagV));
				case 0xE0000000: return true;
				case 0xF0000000: return false; // Architecturally invalid, but is just false
			}

			return true;
		}

		public bool cpu_Calc_V_Flag_Add(uint val1, uint val2, uint val3)
		{
			if ((val1 & cpu_Neg_Compare) == cpu_Neg_Compare)
			{
				if ((val2 & cpu_Neg_Compare) == cpu_Neg_Compare)
				{
					if ((val3 & cpu_Neg_Compare) == 0)
					{
						return true;
					}

					return false;
				}

				return false;
			}
			else
			{
				if ((val2 & cpu_Neg_Compare) == 0)
				{
					if ((val3 & cpu_Neg_Compare) == cpu_Neg_Compare)
					{
						return true;
					}

					return false;
				}

				return false;
			}
		}

		public bool cpu_Calc_V_Flag_Sub(uint val1, uint val2, uint val3)
		{
			if ((val1 & cpu_Neg_Compare) == cpu_Neg_Compare)
			{
				if ((val2 & cpu_Neg_Compare) == 0)
				{
					if ((val3 & cpu_Neg_Compare) == 0)
					{
						return true;
					}

					return false;
				}

				return false;
			}
			else
			{
				if ((val2 & cpu_Neg_Compare) == cpu_Neg_Compare)
				{
					if ((val3 & cpu_Neg_Compare) == cpu_Neg_Compare)
					{
						return true;
					}

					return false;
				}

				return false;
			}
		}

		public bool cpu_TMB_Condition_Check()
		{
			switch (cpu_Instr_TMB_2 & 0xF00)
			{
				case 0x000: return cpu_FlagZ;
				case 0x100: return !cpu_FlagZ;
				case 0x200: return cpu_FlagC;
				case 0x300: return !cpu_FlagC;
				case 0x400: return cpu_FlagN;
				case 0x500: return !cpu_FlagN;
				case 0x600: return cpu_FlagV;
				case 0x700: return !cpu_FlagV;
				case 0x800: return (cpu_FlagC & !cpu_FlagZ);
				case 0x900: return (!cpu_FlagC | cpu_FlagZ);
				case 0xA00: return (cpu_FlagN == cpu_FlagV);
				case 0xB00: return (cpu_FlagN != cpu_FlagV);
				case 0xC00: return (!cpu_FlagZ & (cpu_FlagN == cpu_FlagV));
				case 0xD00: return (cpu_FlagZ | (cpu_FlagN != cpu_FlagV));
				case 0xE00: return true;
				case 0xF00: return false; // Architecturally invalid, but is just false
			}

			return true;
		}
	}
}