#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "GBA_System.h"

namespace GBAHawk
{

	void GBA_System::cpu_Decode_TMB()
	{
		if (TraceCallback) TraceCallback(0);

		int ofst = 0;

		// whether or not to take the branch is determined as part of decode
		cpu_Take_Branch = false;

		switch ((cpu_Instr_TMB_2 >> 13) & 7)
		{
			case 0:
				if ((cpu_Instr_TMB_2 & 0x1800) == 0x1800)
				{
					// add / sub
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
				}
				else
				{
					// Shift
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
				}
				break;

			case 1:
				// data ops (immedaite)
				cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
								case cpu_Thumb_AND:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);

									cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
									break;

								case cpu_Thumb_EOR:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] ^ cpu_Regs[cpu_ALU_Reg_Src];

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);

									cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
									break;

								case cpu_Thumb_LSL:
									cpu_Instr_Type = cpu_Prefetch_Ex_TMB; // extra internal cycle

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
									cpu_Instr_Type = cpu_Prefetch_Ex_TMB; // extra internal cycle

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
									cpu_Instr_Type = cpu_Prefetch_Ex_TMB; // extra internal cycle

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
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
									cpu_Instr_Type = cpu_Prefetch_Ex_TMB; // extra internal cycle

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
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);
									break;

								case cpu_Thumb_NEG:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
									cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

									cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

									cpu_ALU_Long_Result &= cpu_Cast_Int;

									cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);
									break;

								case cpu_Thumb_CMN:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
									cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];

									cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

									cpu_ALU_Long_Result &= cpu_Cast_Int;

									cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);
									break;

								case cpu_Thumb_ORR:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] | cpu_Regs[cpu_ALU_Reg_Src];

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);

									cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
									break;

								case cpu_Thumb_MUL:
									cpu_Instr_Type = cpu_Multiply_TMB;

									cpu_Calculate_Mul_Cycles();

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] * cpu_Regs[cpu_ALU_Reg_Src];

									cpu_ALU_Long_Result &= cpu_Cast_Int;

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);

									cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
									break;

								case cpu_Thumb_BIC:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & (~cpu_Regs[cpu_ALU_Reg_Src]);

									cpu_ALU_Long_Result &= cpu_Cast_Int;

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);

									cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
									break;

								case cpu_Thumb_MVN:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

									cpu_ALU_Long_Result = ~(uint64_t)cpu_Regs[cpu_ALU_Reg_Src];

									cpu_ALU_Long_Result &= cpu_Cast_Int;

									cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
									cpu_FlagZset(cpu_ALU_Long_Result == 0);

									cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
									break;
							}
						}
						else
						{
							// High Regs / Branch and exchange
							switch ((cpu_Instr_TMB_2 >> 8) & 3)
							{
								case 0:
									cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;

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

								case 1:
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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

								case 2:
									if (((cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8)) == 0xF)
									{
										cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
									}
									else
									{
										cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
									}

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

								case 3:
									// Decodes correctly even if bit 7 is set
									cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_TMB;

									// start in thumb mode, always branch
									cpu_Take_Branch = true;

									// Branch and exchange mode (possibly)
									cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 0xF);

									cpu_Temp_Reg = (cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFE);
									break;
							}
						}
					}
					else
					{
						// PC relative load
						cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
						cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;

						cpu_LS_Is_Load = true;

						cpu_Overwrite_Base_Reg = false;

						cpu_Base_Reg = 15;

						cpu_Temp_Addr = (uint32_t)((cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));

						cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
					}
				}
				else
				{
					// Load / store Relative offset
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;

					switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
					{
						case 0: cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB; cpu_LS_Is_Load = false; cpu_Sign_Extend_Load = false; break;
						case 1: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; cpu_LS_Is_Load = false; cpu_Sign_Extend_Load = false; break;
						case 2: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; cpu_LS_Is_Load = false; cpu_Sign_Extend_Load = false; break;
						case 3: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
						case 4: cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB; cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = false; break;
						case 5: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = false; break;
						case 6: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = false; break;
						case 7: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
					}

					cpu_Overwrite_Base_Reg = false;

					cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);
					cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 6) & 7);

					cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);

					cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
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

			case 4:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Load / store half word
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
					cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB;
					cpu_Sign_Extend_Load = false;

					cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

					cpu_Overwrite_Base_Reg = false;

					cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

					cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 5) & 0x3E));

					cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
				}
				else
				{
					// SP relative load store
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;

					cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

					cpu_Overwrite_Base_Reg = false;

					cpu_Temp_Addr = (uint32_t)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));

					cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
				}
				break;

			case 5:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Load Address
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

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
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0xF00) == 0x0)
					{
						// Add offset to stack
						cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;

						if ((cpu_Instr_TMB_2 & 0x80) == 0x0)
						{
							cpu_Regs[13] += (uint32_t)((cpu_Instr_TMB_2 & 0x7F) << 2);
						}
						else
						{
							cpu_Regs[13] -= (uint32_t)((cpu_Instr_TMB_2 & 0x7F) << 2);
						}
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0x600) == 0x400)
						{
							// Push / Pop
							cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
							cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_TMB;

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
								cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
								cpu_Multi_List_Size += ((cpu_Instr_TMB_2 >> i) & 1);
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

					cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

					cpu_Base_Reg = (cpu_Instr_TMB_2 >> 8) & 7;

					cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

					cpu_LS_First_Access = true;

					// always overwrite base reg, unless reading and including base reg
					cpu_Overwrite_Base_Reg = !cpu_LS_Is_Load || (((cpu_Instr_TMB_2 >> cpu_Base_Reg) & 1) == 0);

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
						cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
						cpu_Multi_List_Size += ((cpu_Instr_TMB_2 >> i) & 1);
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

						ofst = (uint32_t)((cpu_Instr_TMB_2 & 0xFF) << 1);

						// offset is signed
						if ((ofst & 0x100) == 0x100) { ofst = (uint32_t)(ofst | 0xFFFFFE00); }

						cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

						cpu_Take_Branch = cpu_TMB_Condition_Check();
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

						ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 1);

						// offset is signed
						if ((ofst & 0x800) == 0x800) { ofst = (uint32_t)(ofst | 0xFFFFF000); }

						cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

						cpu_Take_Branch = true;
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

						ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 12);

						// offset is signed
						if ((ofst & 0x400000) == 0x400000) { ofst = (uint32_t)(ofst | 0xFF800000); }

						cpu_Regs[14] = (uint32_t)(cpu_Regs[15] + ofst);
					}
					else
					{
						// Actual branch operation (can it occur without the first one?)
						cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;

						ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 1);

						cpu_Temp_Reg = (uint32_t)(cpu_Regs[14] + ofst);

						// NOTE: OR with 1, probably reuses the same cpu circuitry that triggers switch to Thumb mode when writing to R[15] directly?
						cpu_Regs[14] = (uint32_t)((cpu_Regs[15] - 2) | 1);

						cpu_Take_Branch = true;
					}
				}
				break;
		}
	}
}