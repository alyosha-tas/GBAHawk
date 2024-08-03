using System;
using System.Collections.Generic;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		public string cpu_Disassemble_ARM()
		{
			var ret = new StringBuilder();

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
									if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
									{
										return $"MLA R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{((cpu_Instr_ARM_2 >> 8) & 0xF):D2} * R{(cpu_Instr_ARM_2 & 0xF):D2} + R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									}
									else
									{
										return $"MUL R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{((cpu_Instr_ARM_2 >> 8) & 0xF):D2} * R{(cpu_Instr_ARM_2 & 0xF):D2}";
									}

								case 0x1:
									// Undefined Opcode Exception
									return "Undefined";

								case 0x2:
									// Multiply Long - Unsigned
									return $"MUL-LU R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}H  R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}L, " +
										   $"R{((cpu_Instr_ARM_2 >> 8) & 0xF):D2} * R{(cpu_Instr_ARM_2 & 0xF):D2}";

								case 0x3:
									// Multiply Long - Signed
									return $"MUL-LS R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}H  R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}L, " +
										   $"R{((cpu_Instr_ARM_2 >> 8) & 0xF):D2} * R{(cpu_Instr_ARM_2 & 0xF):D2}";

								case 0x4:
								case 0x5:
									// Swap
									if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
									{
										return $"SWAPB R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}), " +
											   $"R{(cpu_Instr_ARM_2 & 0xF):D2}";
									}
									else
									{
										return $"SWAP R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}), " +
											   $"R{(cpu_Instr_ARM_2 & 0xF):D2}";
									}

								case 0x6:
								case 0x7:
									// Undefined Opcode Exception
									return "Undefined";
							}
						}
						else
						{
							// halfword or byte transfers
							if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
							{
								ret.Append("LD");
							}
							else
							{
								ret.Append("ST");
							}

							switch ((cpu_Instr_ARM_2 >> 5) & 0x3)
							{
								// 0 case is not a load store instruction
								case 0x1:
									// Unsigned halfword
									ret.Append("H ");
									break;

								case 0x2:
									// Signed Byte
									ret.Append("SB ");
									break;
								case 0x3:
									// Signed halfword
									ret.Append("SH ");
									break;
							}

							if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
							{
								ret.Append($"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, ");

								if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
								{
									ret.Append($"{((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF):X2})");
								}
								else
								{
									ret.Append($"R{(cpu_Instr_ARM_2 & 0xF):D2})");
								}
							}
							else
							{
								ret.Append($"(R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, ");

								if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
								{
									ret.Append($"{((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF):X2}), ");
								}
								else
								{
									ret.Append($"R{(cpu_Instr_ARM_2 & 0xF):D2}), ");
								}

								ret.Append($"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}"); 
							}

							return ret.ToString();
						}
					}
					else
					{
						// ALU ops
						if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
						{
							// update flags
							switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
							{
								case 0x0: ret.Append("AND "); break;
								case 0x1: ret.Append("EOR "); break;
								case 0x2: ret.Append("SUB "); break;
								case 0x3: ret.Append("RSB "); break;
								case 0x4: ret.Append("ADD "); break;
								case 0x5: ret.Append("ADC "); break;
								case 0x6: ret.Append("SBC "); break;
								case 0x7: ret.Append("RSC "); break;
								case 0x8: ret.Append("TST "); break;
								case 0x9: ret.Append("TEQ "); break;
								case 0xA: ret.Append("CMP "); break;
								case 0xB: ret.Append("CMN "); break;
								case 0xC: ret.Append("ORR "); break;
								case 0xD: ret.Append("MOV "); break;
								case 0xE: ret.Append("BIC "); break;
								case 0xF: ret.Append("MVN "); break;
							}

							ret.Append($"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, ");
						}
						else
						{
							// don't update flags
							switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
							{
								case 0x0: ret.Append("AND "); break;
								case 0x1: ret.Append("EOR "); break;
								case 0x2: ret.Append("SUB "); break;
								case 0x3: ret.Append("RSB "); break;
								case 0x4: ret.Append("ADD "); break;
								case 0x5: ret.Append("ADC "); break;
								case 0x6: ret.Append("SBC "); break;
								case 0x7: ret.Append("RSC "); break;
								case 0x8: return $"MRS R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, CPSR";
								case 0x9:
									if ((cpu_Instr_ARM_2 & 0XFFFF0) == 0xFFF10)
									{
										return $"Bx R{(cpu_Instr_ARM_2 & 0xF):D2}";
									}
									else
									{
										ret.Append($"MSR CPSR, mask:{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, "); break;
									}
								case 0xA: return $"MRS R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, SPSR";
								case 0xB: ret.Append($"MSR SPSR, mask:{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, "); break;
								case 0xC: ret.Append("ORR "); break;
								case 0xD: ret.Append("MOV "); break;
								case 0xE: ret.Append("BIC "); break;
								case 0xF: ret.Append("MVN "); break;
							}

							if ((((cpu_Instr_ARM_2 >> 21) & 0xF) == 0x9) || (((cpu_Instr_ARM_2 >> 21) & 0xF) == 0xB))
							{
								// nothing to append
							}
							else
							{
								ret.Append("(no flags) ");
								ret.Append($"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, ");
							}
						}

						ret.Append($"(R{(cpu_Instr_ARM_2 & 0xF):D2} ");

						switch ((cpu_Instr_ARM_2 >> 5) & 3)
						{
							case 0:         // LSL
								ret.Append("<< ");
								break;

							case 1:         // LSR
								ret.Append(">> ");
								break;

							case 2:         // ASR
								ret.Append("ASR ");
								break;

							case 3:         // RRX
								ret.Append("RRX ");
								break;
						}

						if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
						{
							// immediate shift
							ret.Append($"{((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})");
						}
						else
						{
							// register shift
							ret.Append($"R{((cpu_Instr_ARM_2 >> 8) & 0xF):D2} & FF)");
						}

						return ret.ToString();
					}
					break;

				case 1:
					// ALU ops
					if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
					{
						// update flags
						switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
						{
							case 0x0: ret.Append("AND "); break;
							case 0x1: ret.Append("EOR "); break;
							case 0x2: ret.Append("SUB "); break;
							case 0x3: ret.Append("RSB "); break;
							case 0x4: ret.Append("ADD "); break;
							case 0x5: ret.Append("ADC "); break;
							case 0x6: ret.Append("SBC "); break;
							case 0x7: ret.Append("RSC "); break;
							case 0x8: ret.Append("TST "); break;
							case 0x9: ret.Append("TEQ "); break;
							case 0xA: ret.Append("CMP "); break;
							case 0xB: ret.Append("CMN "); break;
							case 0xC: ret.Append("ORR "); break;
							case 0xD: ret.Append("MOV "); break;
							case 0xE: ret.Append("BIC "); break;
							case 0xF: ret.Append("MVN "); break;
						}

						ret.Append($"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
								   $"({(cpu_Instr_ARM_2 & 0xFF):X2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1E):X2})");
					}
					else
					{
						// don't update flags
						switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
						{
							case 0x0: ret.Append("AND "); break;
							case 0x1: ret.Append("EOR "); break;
							case 0x2: ret.Append("SUB "); break;
							case 0x3: ret.Append("RSB "); break;
							case 0x4: ret.Append("ADD "); break;
							case 0x5: ret.Append("ADC "); break;
							case 0x6: ret.Append("SBC "); break;
							case 0x7: ret.Append("RSC "); break;
							case 0x8: return "Undefined";
							case 0x9: return $"MSR CPSR, mask:{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
											 $"({((cpu_Instr_ARM_2 >> 16) & 0xF):X2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1E):X2})";
							case 0xA: return "Undefined";
							case 0xB: return $"MSR SPSR, mask:{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
											 $"({((cpu_Instr_ARM_2 >> 16) & 0xF):X2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1E):X2})";
							case 0xC: ret.Append("ORR "); break;
							case 0xD: ret.Append("MOV "); break;
							case 0xE: ret.Append("BIC "); break;
							case 0xF: ret.Append("MVN "); break;
						}
						ret.Append("(no flags) ");
						ret.Append($"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
								   $"({(cpu_Instr_ARM_2 & 0xFF):X2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1E):X2})");
					}
					return ret.ToString();

				case 2:
					// load / store immediate offset
					if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
					{
						if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
						{
							return $"LDB R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
								   $"{(cpu_Instr_ARM_2 & 0xFFF):X3})";
						}
						else
						{
							return $"LD R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
								   $"{(cpu_Instr_ARM_2 & 0xFFF):X3})";
						}
					}
					else
					{
						if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
						{
							return $"STB (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, {(cpu_Instr_ARM_2 & 0xFFF):X3}), " +
								   $"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
						}
						else
						{
							return $"ST (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, {(cpu_Instr_ARM_2 & 0xFFF):X3}), " +
								   $"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
						}
					}

				case 3:
					if ((cpu_Instr_ARM_2 & 0x10) == 0)
					{
						// load / store register offset
						if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
						{
							if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
							{
								switch ((cpu_Instr_ARM_2 >> 5) & 3)
								{
									case 0:         // LSL
										return $"LDB R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} << {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
									case 1:         // LSR
										return $"LDB R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
									case 2:         // ASR
										return $"LDB R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} ASR {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
									case 3:         // RRX
										return $"LDB R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} RRX {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
								}
							}
							else
							{
								switch ((cpu_Instr_ARM_2 >> 5) & 3)
								{
									case 0:         // LSL
										return $"LD R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} << {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
									case 1:         // LSR
										return $"LD R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
									case 2:         // ASR
										return $"LD R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} ASR {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
									case 3:         // RRX
										return $"LD R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}, (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, " +
												$"R{(cpu_Instr_ARM_2 & 0xF):D2} RRX {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2})";
								}
							}							
						}
						else
						{
							if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
							{
								switch ((cpu_Instr_ARM_2 >> 5) & 3)
								{
									case 0:         // LSL
										return $"STB (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} << {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									case 1:         // LSR
										return $"STB (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									case 2:         // ASR
										return $"STB (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} ASR {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									case 3:         // RRX
										return $"STB (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} RRX {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
								}
							}
							else
							{
								switch ((cpu_Instr_ARM_2 >> 5) & 3)
								{
									case 0:         // LSL
										return $"ST (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} << {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									case 1:         // LSR
										return $"ST (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} >> {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									case 2:         // ASR
										return $"ST (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} ASR {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
									case 3:         // RRX
										return $"ST (R{((cpu_Instr_ARM_2 >> 16) & 0xF):D2}, R{(cpu_Instr_ARM_2 & 0xF):D2} RRX {((cpu_Instr_ARM_2 >> 7) & 0x1F):X2}), " +
												$"R{((cpu_Instr_ARM_2 >> 12) & 0xF):D2}";
								}
							}								
						}
					}
					else
					{
						// Undefined Opcode Exception
						return "Undefined";
					}
					return "";

				case 4:
					// block transfer
					if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
					{
						// user mode
						if((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
						{
							return $"LDMU {cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)]:X8} regs:{(cpu_Instr_ARM_2 & 0xFFFF):X4}";
						}
						else
						{
							return $"LDM {cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)]:X8} regs:{(cpu_Instr_ARM_2 & 0xFFFF):X4}";
						}
					}
					else
					{
						// user mode
						if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
						{
							return $"STMU {cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)]:X8} regs:{(cpu_Instr_ARM_2 & 0xFFFF):X4}";
						}
						else
						{
							return $"STM {cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)]:X8} regs:{(cpu_Instr_ARM_2 & 0xFFFF):X4}";
						}
					}

				case 5:
					// branch
					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// Link if link bit set
						// offset is signed
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							return $"BL {(((cpu_Instr_ARM_2 & 0xFFFFFF) << 2) | 0xFC000000):X8}";
						}
						else
						{
							return $"BL {((cpu_Instr_ARM_2 & 0xFFFFFF) << 2):X8}";
						}
					}
					else
					{
						// offset is signed
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							return $"B {(((cpu_Instr_ARM_2 & 0xFFFFFF) << 2) | 0xFC000000):X8}";
						}
						else
						{
							return $"B {((cpu_Instr_ARM_2 & 0xFFFFFF) << 2):X8}";
						}
					}

				case 6:
					// Coprocessor Instruction (treat as Undefined Opcode Exception)
					return "Undefined";

				case 7:
					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// software interrupt
						return "SWI";
					}
					else
					{
						// Coprocessor Instruction (treat as Undefined Opcode Exception)
						return "Undefined";
					}
			}

			return "";
		}

		public string cpu_Disassemble_TMB()
		{
			switch ((cpu_Instr_TMB_2 >> 13) & 7)
			{
				case 0:
					// shift / add / sub
					if ((cpu_Instr_TMB_2 & 0x1800) == 0x1800)
					{
						if ((cpu_Instr_TMB_2 & 0x200) == 0x200)
						{
							if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
							{
								return $"SUB R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} - {((cpu_Instr_TMB_2 >> 6) & 0x7):X2}";
							}
							else
							{
								return $"SUB R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} - R{((cpu_Instr_TMB_2 >> 6) & 7):D2}";
							}
						}
						else
						{
							if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
							{
								return $"ADD R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 6) & 0x7):X2}";
							}
							else
							{
								return $"ADD R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2}";
							}
						}
					}
					else
					{
						switch ((cpu_Instr_TMB_2 >> 11) & 0x3)
						{
							case 0:
								return $"LSL IMM R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} << {((cpu_Instr_TMB_2 >> 6) & 0x1F):X2}";

							case 1:
								return $"LSR IMM R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} >> {((cpu_Instr_TMB_2 >> 6) & 0x1F):X2}";

							case 2:
								return $"ROR IMM R{(cpu_Instr_TMB_2 & 7):D2} = R{((cpu_Instr_TMB_2 >> 3) & 7):D2} >> {((cpu_Instr_TMB_2 >> 6) & 0x1F):X2}";
						}


					}
					return "";

				case 1:
					// data ops (immedaite)
					switch ((cpu_Instr_TMB_2 >> 11) & 3)
					{
						case 0:         // MOV
							return $"MOV IMM R{((cpu_Instr_TMB_2 >> 8) & 7):D2}, {(cpu_Instr_TMB_2 & 0xFF):X2}";

						case 1:         // CMP
							return $"CMP IMM R{((cpu_Instr_TMB_2 >> 8) & 7):D2}, {(cpu_Instr_TMB_2 & 0xFF):X2}";

						case 2:         // ADD
							return $"ADD IMM R{((cpu_Instr_TMB_2 >> 8) & 7):D2}, {(cpu_Instr_TMB_2 & 0xFF):X2}";

						case 3:         // SUB
							return $"SUB IMM R{((cpu_Instr_TMB_2 >> 8) & 7):D2}, {(cpu_Instr_TMB_2 & 0xFF):X2}";
					}
					return "";

				case 2:
					if ((cpu_Instr_TMB_2 & 0x1000) == 0x0)
					{
						if ((cpu_Instr_TMB_2 & 0x800) == 0x0)
						{
							if ((cpu_Instr_TMB_2 & 0x400) == 0x0)
							{
								// ALU Ops
								switch ((cpu_Instr_TMB_2 >> 6) & 0xF)
								{
									case 0x0: return $"AND R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x1: return $"EOR R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x2: return $"LSL R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x3: return $"LSR R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x4: return $"ASR R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x5: return $"ADC R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x6: return $"SBC R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x7: return $"ROR R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x8: return $"TST R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0x9: return $"NEG R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0xA: return $"CMP R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0xB: return $"CMN R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0xC: return $"ORR R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0xD: return $"MUL R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0xE: return $"BIC R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
									case 0xF: return $"MVN R{(cpu_Instr_TMB_2 & 7):D2}, R{((cpu_Instr_TMB_2 >> 3) & 7):D2}";
								}
							}
							else
							{
								// High Regs / Branch and exchange
								switch ((cpu_Instr_TMB_2 >> 8) & 3)
								{
									case 0:
										return $"ADD R{(cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8):D2}, R{((cpu_Instr_TMB_2 >> 3) & 0xF):D2}";

									case 1:
										return $"CMP R{(cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8):D2}, R{((cpu_Instr_TMB_2 >> 3) & 0xF):D2}";

									case 2:
										if ((cpu_Instr_TMB_2 & 0xC0) == 0x0)
										{
											return "CPY?";
										}
										else
										{
											return $"MOV R{(cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8):D2}, R{((cpu_Instr_TMB_2 >> 3) & 0xF):D2}";
										}

									case 3:
										if ((cpu_Instr_TMB_2 & 0x80) == 0)
										{
											return $"Bx (R{((cpu_Instr_TMB_2 >> 3) & 0xF):D2})";
										}
										else
										{
											// This version only available in ARM V5 and above
											return "";
										}
								}
							}
						}
						else
						{
							// PC relative load
							return $"LD R{((cpu_Instr_TMB_2 >> 8) & 7):D2}, (PC + {((cpu_Instr_TMB_2 & 0xFF) << 2):X3})";
						}
					}
					else
					{
						// Load / store Relative offset
						switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
						{
							case 0: return $"ST (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2}), R{(cpu_Instr_TMB_2 & 7):D2}";
							case 1: return $"STH (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2}), R{(cpu_Instr_TMB_2 & 7):D2}";
							case 2: return $"STB (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2}), R{(cpu_Instr_TMB_2 & 7):D2}";
							case 3: return $"LDSB R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2})";
							case 4: return $"LD R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2})";
							case 5: return $"LDH R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2})";
							case 6: return $"LDB R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2})";
							case 7: return $"LDSH R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + R{((cpu_Instr_TMB_2 >> 6) & 7):D2})";
						}
					}
					return "";

				case 3:
					// Load / store Immediate offset
					if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
					{
						if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
						{
							return $"LDB R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 6) & 0x1F):X2})";
						}
						else
						{
							return $"STB (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 6) & 0x1F):X2}), R{(cpu_Instr_TMB_2 & 7):D2}";
						}
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
						{
							return $"LD R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 4) & 0x7C):X2})";
						}
						else
						{
							return $"ST (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 4) & 0x7C):X2}), R{(cpu_Instr_TMB_2 & 7):D2}";
						}
					}

				case 4:
					if ((cpu_Instr_TMB_2 & 0x1000) == 0)
					{
						// Load / store half word
						if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
						{
							return $"LDH R{(cpu_Instr_TMB_2 & 7):D2}, (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 5) & 0x3E):X2})";
						}
						else
						{
							return $"STH (R{((cpu_Instr_TMB_2 >> 3) & 7):D2} + {((cpu_Instr_TMB_2 >> 5) & 0x3E):X2}), R{(cpu_Instr_TMB_2 & 7):D2}";
						}
					}
					else
					{
						// SP relative load store
						if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
						{
							return $"LD R{((cpu_Instr_TMB_2 >> 8) & 7):D2}, (R13 + {((cpu_Instr_TMB_2 & 0xFF) << 2):X3})";
						}
						else
						{
							return $"ST (R13 + {((cpu_Instr_TMB_2 & 0xFF) << 2):X3}), R{((cpu_Instr_TMB_2 >> 8) & 7):D2}";
						}
					}

				case 5:
					if ((cpu_Instr_TMB_2 & 0x1000) == 0)
					{
						// Load Address
						if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
						{
							return $"R{((cpu_Instr_TMB_2 >> 8) & 7):D2} = R13 + {((cpu_Instr_TMB_2 & 0xFF) << 2):X3}";
						}
						else
						{
							return $"R{((cpu_Instr_TMB_2 >> 8) & 7):D2} = R15 + {((cpu_Instr_TMB_2 & 0xFF) << 2):X3}";
						}
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0xF00) == 0x0)
						{
							// Add offset to stack
							if ((cpu_Instr_TMB_2 & 0x80) == 0x0)
							{
								return $"ADD SP:{(cpu_Instr_TMB_2 & 0x7F):X2}";
							}
							else
							{
								return $"SUB SP:{(cpu_Instr_TMB_2 & 0x7F):X2}";
							}
						}
						else
						{
							if ((cpu_Instr_TMB_2 & 0x600) == 0x400)
							{
								// Push / Pop
								if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
								{
									return $"Pop {cpu_Regs[13]:X8} regs:{(cpu_Instr_TMB_2 & 0x1FF):X3}";
								}
								else
								{
									return $"Push {cpu_Regs[13]:X8} regs:{(cpu_Instr_TMB_2 & 0x1FF):X3}";
								}
							}
							else
							{
								// Undefined Opcode Exception
								return "Undefined";
							}
						}
					}

				case 6:
					if ((cpu_Instr_TMB_2 & 0x1000) == 0)
					{
						// Multiple Load/Store
						if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
						{
							return $"LDM {cpu_Regs[(cpu_Instr_TMB_2 >> 8) & 7]:X8} regs:{(cpu_Instr_TMB_2 & 0xFF):X2}";
						}
						else
						{
							return $"STM {cpu_Regs[(cpu_Instr_TMB_2 >> 8) & 7]:X8} regs:{(cpu_Instr_TMB_2 & 0xFF):X2}";
						}
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0xF00) == 0xF00)
						{
							// Software Interrupt
							return "SWI";
						}
						else if ((cpu_Instr_TMB_2 & 0xE00) == 0xE00)
						{
							// Undefined instruction
							return "Undefined";
						}
						else
						{
							// Conditional Branch
							if (cpu_TMB_Condition_Check())
							{
								if ((cpu_Instr_TMB_2 & 0x80) == 0x80)
								{
									return $"B (-){(cpu_Instr_TMB_2 & 0xFF):X2}";
								}
								else
								{
									return $"B (+){(cpu_Instr_TMB_2 & 0xFF):X2}";
								}
							}
							else
							{
								return "B (failed)";
							}
						}
					}

				case 7:
					if ((cpu_Instr_TMB_2 & 0x1000) == 0)
					{
						if ((cpu_Instr_TMB_2 & 0x800) == 0)
						{
							// Unconditional branch
							if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
							{
								return $"B (-){(cpu_Instr_TMB_2 & 0x7FF):X3}";
							}
							else
							{
								return $"B (+){(cpu_Instr_TMB_2 & 0x7FF):X3}";
							}
						}
						else
						{
							// Undefined Opcode Exception
							return "Undefined";
						}
					}
					else
					{
						// Branch with link
						if ((cpu_Instr_TMB_2 & 0x800) == 0)
						{
							// A standard data operation assigning the upper part of the branch

							// offset is signed
							if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
							{
								return $"BL 1 (-){(cpu_Instr_TMB_2 & 0x7FF):X3}";
							}
							else
							{
								return $"BL 1 (+){(cpu_Instr_TMB_2 & 0x7FF):X3}";
							}
						}
						else
						{
							// Actual branch operation (can it occur without the first one?)
							if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
							{
								return $"BL 2 {(cpu_Instr_TMB_2 & 0x7FF):X3}";
							}
							else
							{
								return $"BL 2 {(cpu_Instr_TMB_2 & 0x7FF):X3}";
							}
						}
					}
			}

			return "";
		}

		public string Disassemble()
		{
			var ret = new StringBuilder();

			if (cpu_Thumb_Mode)
			{
				ret.Append($"{(cpu_Regs[15] - 4):X8}:  ");
				ret.Append($"    {cpu_Instr_TMB_2:X4}  ");
				ret.Append(cpu_Disassemble_TMB());
			}
			else
			{
				ret.Append($"{(cpu_Regs[15] - 8):X8}:  ");
				ret.Append($"{cpu_Instr_ARM_2:X8}  ");
				if (!cpu_ARM_Condition_Check()) { ret.Append("(fail)  "); }
				ret.Append(cpu_Disassemble_ARM());
			}

			return ret.ToString();
		}
	}
}
