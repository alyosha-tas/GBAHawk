#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "APU_System.h"

namespace SNESHawk
{
	void APU::ExecuteOneOp()
	{
		switch (spc_Instr_Type)
		{
			case OpT::Imp:
			case OpT::Acc:
				switch (spc_Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						alu_temp = ReadMemory(address_bus);
						spc_ALU_Operation();
						break;

					case 1:
						End();
						break;
				}
				break;
		
			case OpT::Imm:
				switch (spc_Instr_Cycle)
				{
					case 0:	
						address_bus = PC;

						alu_temp = ReadMemory(address_bus);
						PC++;
						spc_ALU_Operation();
						break;

					case 1:
						End();
						break;
				}
				break;

			case OpT::CSI:
				switch (spc_Instr_Cycle)
				{
					case 0:
						// not effected by RDY
						if (spc_ALU_Type == ALU::CLI)
						{
							spc_FlagIset(false);
						}
						else
						{
							spc_FlagIset(true);
						}
											
						address_bus = PC;

						alu_temp = ReadMemory(address_bus);
						break;

					case 1:
						End_ISpecial();
						break;
				}
				break;

			case OpT::PH:
				switch (spc_Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						ReadMemory(address_bus);
						break;

					case 1:
						if (spc_ALU_Type == ALU::PHP)
						{
							spc_FlagBset(true);
							WriteMemory((uint16_t)(S-- + 0x100), P);
						}
						else
						{
							WriteMemory((uint16_t)(S-- + 0x100), A);
						}
						break;

					case 2:
						End();
						break;
				}
				break;

			case OpT::PL:
				switch (spc_Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						ReadMemory(address_bus);
						break;

					case 1:
						address_bus = (uint16_t)(0x100 | S);

						ReadMemory(address_bus);
						S++;
						break;

					case 2:						
						address_bus = (uint16_t)(S + 0x100);

						if (spc_ALU_Type == ALU::PLP)
						{
							my_iflag = spc_FlagIget();
							P = ReadMemory(address_bus);
							spc_FlagIset(my_iflag);
							spc_FlagTset(true); //force T always to remain true
						}
						else
						{
							A = ReadMemory(address_bus);
							NZ_A();
						}
						break;

					case 3:
						if (spc_ALU_Type == ALU::PLP)
						{
							End_ISpecial();
						}
						else
						{
							End();
						}
					break;
				}
				break;

			case OpT::JSR:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = PC;

						ReadMemory(address_bus);
						break;

					case 2:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						break;

					case 3:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						break;

					case 4:
						address_bus = PC;

						PC = (uint16_t)((ReadMemory(address_bus) << 8) + opcode2);
						break;

					case 5:
						End();
						break;

				}
				break;
			
			case OpT::JMP:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = PC;

						PC = (uint16_t)((ReadMemory(address_bus) << 8) + opcode2);
						break;

					case 2:
						End();
						break;
				}
				break;

			case OpT::JMPI:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						Fetch3();
						break;

					case 2:
						ea = (opcode3 << 8) + opcode2;
						address_bus = (uint16_t)ea;

						alu_temp = ReadMemory(address_bus);
						break;

					case 3:
						ea = (opcode3 << 8) + (uint8_t)(opcode2 + 1);
						address_bus = (uint16_t)ea;

						alu_temp += ReadMemory(address_bus) << 8;
						PC = (uint16_t)alu_temp;
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::RTI:
				switch (spc_Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						ReadMemory(address_bus);
						break;

					case 1:
						address_bus = (uint16_t)(0x100 | S);

						ReadMemory(address_bus);
						S++;
						break;

					case 2:
						address_bus = (uint16_t)(S + 0x100);

						P = ReadMemory(address_bus);
						spc_FlagTset(true); //force T always to remain true
						S++;
						break;

					case 3:
						address_bus = (uint16_t)(S + 0x100);

						PC &= 0xFF00;
						PC |= ReadMemory(address_bus);
						S++;
						break;

					case 4:
						address_bus = (uint16_t)(S + 0x100);

						PC &= 0xFF;
						PC |= (uint16_t)(ReadMemory(address_bus) << 8);
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::RTS:
				switch (spc_Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						ReadMemory(address_bus);
						break;

					case 1:
						address_bus = (uint16_t)(0x100 | S);

						ReadMemory(address_bus);
						S++;
						break;

					case 2:					
						address_bus = (uint16_t)(S + 0x100);

						PC &= 0xFF00;
						PC |= ReadMemory(address_bus);
						S++;
						break;

					case 3:					
						address_bus = (uint16_t)(S + 0x100);

						PC &= 0xFF;
						PC |= (uint16_t)(ReadMemory(address_bus) << 8);
						break;

					case 4:			
						address_bus = PC;

						ReadMemory(address_bus);
						PC++;
						break;

					case 5:
						End();
						break;
				}
				break;


			case OpT::Br:
				switch (spc_Instr_Cycle)
				{
					case 0:
						switch (spc_ALU_Type)
						{
							case ALU::BPL:
								branch_taken = !spc_FlagNget();
								break;
							case ALU::BMI:
								branch_taken = spc_FlagNget();
								break;
							case ALU::BVC:
								branch_taken = !spc_FlagVget();
								break;
							case ALU::BVS:
								branch_taken = spc_FlagVget();
								break;
							case ALU::BCC:
								branch_taken = !spc_FlagCget();
								break;
							case ALU::BCS:
								branch_taken = spc_FlagCget();
								break;
							case ALU::BNE:
								branch_taken = !spc_FlagZget();
								break;
							case ALU::BEQ:
								branch_taken = spc_FlagZget();
								break;
						}

						address_bus = PC;

						opcode2 = ReadMemory(address_bus);
						PC++;

						if (branch_taken)
						{
							branch_taken = false;
						}
						else
						{
							spc_Instr_Cycle+=2;
						}
						break;

					case 1:
						address_bus = PC;

						ReadMemory(address_bus);
						alu_temp = ((uint8_t)PC + (int8_t)opcode2);
						PC &= 0xFF00;
						PC |= (uint16_t)(alu_temp & 0xFF);

						if (!((alu_temp & 0x100) == 0x100))
						{								
							branch_irq_hack = true;

							spc_Instr_Cycle++;
						}
						break;

					case 2:
						address_bus = PC;

						ReadMemory(address_bus);
						if (((alu_temp & 0x80000000) == 0x80000000))
							PC = (uint16_t)(PC - 0x100);
						else PC = (uint16_t)(PC + 0x100);
						break;

					case 3:
						End_Branch();
						break;
				}
				break;
		
			case OpT::ZPR:
			case OpT::ZPW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						if (spc_Instr_Type == OpT::ZPR)
						{
							address_bus = opcode2;

							alu_temp = ReadMemory(address_bus);
							spc_ALU_Operation();
						}
						else
						{
							ea = opcode2;
							spc_Write_Operation();
						}
						break;

					case 2:
						End();
						break;
				}
				break;

			case OpT::ZPRW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						alu_temp = ReadMemory(address_bus);
						break;

					case 2:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						spc_ALU_Operation();
						break;

					case 3:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::ZPXR:
			case OpT::ZPXW:
			case OpT::ZPYR:
			case OpT::ZPYW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ReadMemory(address_bus);

						if ((spc_Instr_Type == OpT::ZPXR) || (spc_Instr_Type == OpT::ZPXW))
						{
							opcode2 = (uint8_t)(opcode2 + X);
						}
						else
						{
							opcode2 = (uint8_t)(opcode2 + Y);
						}
						break;

					case 2:
						if ((spc_Instr_Type == OpT::ZPXR) || (spc_Instr_Type == OpT::ZPYR))
						{
							address_bus = opcode2;

							alu_temp = ReadMemory(address_bus);
							spc_ALU_Operation();
						}
						else
						{
							ea = opcode2;
							spc_Write_Operation();
						}
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::ZPXRW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ReadMemory(address_bus);
						opcode2 = (uint8_t)(opcode2 + X);
						break;

					case 2:
						address_bus = opcode2;

						alu_temp = ReadMemory(address_bus);
						break;

					case 3:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						spc_ALU_Operation();
						break;

					case 4:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::AbsR:
			case OpT::AbsW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						Fetch3();
						break;

					case 2:
						ea = (uint16_t)((opcode3 << 8) + opcode2);
						if (spc_Instr_Type == OpT::AbsR)
						{
							address_bus = ea;

							alu_temp = ReadMemory(address_bus);
							spc_ALU_Operation();
						}
						else
						{
							spc_Write_Operation();
						}
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::AbsRW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						Fetch3();
						break;

					case 2:
						ea = (opcode3 << 8) + opcode2;
						address_bus = ea;

						alu_temp = ReadMemory(address_bus);
						break;

					case 3:
						WriteMemory(ea, (uint8_t)alu_temp);
						spc_ALU_Operation();
						break;

					case 4:
						WriteMemory(ea, (uint8_t)alu_temp);
						break;

					case 5:
						End();
						break;
				}
				break;
		
			case OpT::AdXR:
			case OpT::AdXW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ReadMemory(address_bus);
						alu_temp = (opcode2 + X) & 0xFF;
						break;

					case 2:
						address_bus = (uint16_t)alu_temp;

						ea = ReadMemory(address_bus);
						break;

					case 3:
						address_bus = (uint8_t)(alu_temp + 1);

						ea += (ReadMemory(address_bus) << 8);
						break;

					case 4:
						if (spc_Instr_Type == OpT::AdXR)
						{
							address_bus = (uint16_t)ea;

							alu_temp = ReadMemory(address_bus);
							spc_ALU_Operation();
						}
						else
						{
							spc_Write_Operation();
						}
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::AdXRW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ReadMemory(address_bus);
						alu_temp = (opcode2 + X) & 0xFF;
						break;

					case 2:
						address_bus = (uint16_t)alu_temp;

						ea = ReadMemory(address_bus);
						break;

					case 3:
						address_bus = (uint8_t)(alu_temp + 1);

						ea += (ReadMemory(address_bus) << 8);
						break;

					case 4:
						address_bus = (uint16_t)ea;

						alu_temp = ReadMemory(address_bus);
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						spc_ALU_Operation();
						break;

					case 6:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						break;

					case 7:
						End();
						break;
				}
				break;

			case OpT::AIXR:
			case OpT::AIYR:
			case OpT::AIXW:
			case OpT::AIYW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = PC;

						opcode3 = ReadMemory(address_bus);
						PC++;

						if ((spc_Instr_Type == OpT::AIXR) || (spc_Instr_Type == OpT::AIXW))
						{
							alu_temp = opcode2 + X;
						}
						else
						{
							alu_temp = opcode2 + Y;
						}
						ea = (opcode3 << 8) + (alu_temp & 0xFF);
						H = 0; // In preparation for SHY, set H to 0.

						if ((spc_Instr_Type == OpT::AIXR) || (spc_Instr_Type == OpT::AIYR))
						{
							// skip a cycle if no page crossing
							if (!((alu_temp & 0x100) == 0x100))
							{
								spc_Instr_Cycle++;
							}
						}
						break;

					case 2:
						address_bus = (uint16_t)ea;

						if ((spc_Instr_Type == OpT::AIXR) || (spc_Instr_Type == OpT::AIYR))
						{
							alu_temp = ReadMemory(address_bus);
							ea = (uint16_t)(ea + 0x100);
						}
						else
						{
							bool adjust = ((alu_temp & 0x100) == 0x100);
							alu_temp = ReadMemory(address_bus);

							if (adjust)
							{
								ea = (uint16_t)(ea + 0x100);
							}
						}
						break;

					case 3:
						if ((spc_Instr_Type == OpT::AIXR) || (spc_Instr_Type == OpT::AIYR))
						{
							address_bus = (uint16_t)ea;

							alu_temp = ReadMemory(address_bus);

							spc_ALU_Operation();
						}
						else
						{
							spc_Write_Operation();
						}
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::AIXRW:
			case OpT::AIYRW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = PC;

						opcode3 = ReadMemory(address_bus);
						PC++;

						if (spc_Instr_Type == OpT::AIXRW)
						{
							alu_temp = opcode2 + X;
						}
						else
						{
							alu_temp = opcode2 + Y;
						}
						ea = (opcode3 << 8) + (alu_temp & 0xFF);

						H = 0; // In preparation for SHY, set H to 0.
						break;

					case 2:
						address_bus = (uint16_t)ea;

						bool adjust = ((alu_temp & 0x100) == 0x100);
						alu_temp = ReadMemory(address_bus);

						if (adjust)
						{
							ea = (uint16_t)(ea + 0x100);
						}
						break;

					case 3:
						address_bus = (uint16_t)ea;

						alu_temp = ReadMemory(address_bus);
						break;

					case 4:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						spc_ALU_Operation();
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						break;

					case 6:
						End();
						break;
				}
				break;
		
			case OpT::AIUW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = PC;

						opcode3 = ReadMemory(address_bus);
						PC++;

						if ((opcode & 0xF) == 0xC)
						{
							alu_temp = opcode2 + X;
						}
						else
						{
							alu_temp = opcode2 + Y;
						}
						ea = (opcode3 << 8) + (alu_temp & 0xFF);
						address_bus = (uint16_t)ea;
						H = 0; // In preparation for SHY, set H to 0.
						break;

					case 2:
						
						address_bus = (uint16_t)ea;

						if (spc_ALU_Type == ALU::SHS)
						{
							H |= (uint8_t)((ea >> 8) + 1);
							bool adjust = ((alu_temp & 0x100) == 0x100);
							alu_temp = ReadMemory(address_bus);

							if (adjust)
							{
								ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & ((A & X) << 8)));
							}
						}
						else if (spc_ALU_Type == ALU::SHY)
						{
							H |= (uint8_t)((ea >> 8) + 1);
							bool adjust = ((alu_temp & 0x100) == 0x100);
							alu_temp = ReadMemory(address_bus);

							if (adjust)
							{
								ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & (Y << 8)));
							}
						}
						else if (spc_ALU_Type == ALU::SHX)
						{
							H |= (uint8_t)((ea >> 8) + 1);
							bool adjust = ((alu_temp & 0x100) == 0x100);
							alu_temp = ReadMemory(address_bus);

							if (adjust)
							{
								ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & (X << 8)));
							}
						}
						else // SHA
						{
							H |= (uint8_t)((ea >> 8) + 1);
							bool adjust = ((alu_temp & 0x100) == 0x100);
							alu_temp = ReadMemory(address_bus);

							if (adjust)
							{
								ea = (uint16_t)((ea & 0xFF) | ((ea + 0x100) & 0xFF00 & ((A & X) << 8)));
							}
						}
						break;

					case 3:
						spc_Write_Operation();
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::IIYR:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ea = ReadMemory(address_bus);
						break;

					case 2:
						address_bus = (uint8_t)(opcode2 + 1);

						alu_temp = ea + Y;
						ea = (ReadMemory(address_bus) << 8)
							| ((alu_temp & 0xFF));

						H = 0; // In preparation for SHA (indirect, X), set H to 0.

						if (!((alu_temp & 0x100) == 0x100))
						{
							spc_Instr_Cycle++;
						}
						break;

					case 3:
						address_bus = (uint16_t)ea;

						ReadMemory(address_bus);
						ea = (uint16_t)(ea + 0x100);
						break;

					case 4:
						address_bus = (uint16_t)ea;

						alu_temp = ReadMemory(address_bus);
						spc_ALU_Operation();
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::IIYW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ea = ReadMemory(address_bus);
						break;

					case 2:
						address_bus = (uint8_t)(opcode2 + 1);

						alu_temp = ea + Y;
						ea = (ReadMemory(address_bus) << 8)
							| ((alu_temp & 0xFF));

						H = 0; // In preparation for SHA (indirect, X), set H to 0.
						break;

					case 3:
						address_bus = (uint16_t)ea;

						if (opcode == 0x91)
						{
							ReadMemory(address_bus);
							ea += (alu_temp >> 8) << 8;
						}
						else
						{
							H |= (uint8_t)((ea >> 8) + 1);
							ReadMemory(address_bus);

							if ((alu_temp & 0x100) == 0x100)
							{
								ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & ((A & X) << 8)));
							}
						}
						break;

					case 4:
						spc_Write_Operation();
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::IIYRW:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						address_bus = opcode2;

						ea = ReadMemory(address_bus);
						break;

					case 2:
						
						address_bus = (uint8_t)(opcode2 + 1);

						alu_temp = ea + Y;
						ea = (ReadMemory(address_bus) << 8)
							| ((alu_temp & 0xFF));

						H = 0; // In preparation for SHA (indirect, X), set H to 0.
						break;

					case 3:
						address_bus = (uint16_t)ea;

						ReadMemory(address_bus);
						if ((alu_temp & 0x100) == 0x100)
							ea = (uint16_t)(ea + 0x100);
						break;

					case 4:
						address_bus = (uint16_t)ea;

						alu_temp = ReadMemory(address_bus);
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						spc_ALU_Operation();
						break;

					case 6:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						break;

					case 7:
						End();
						break;
				}
				break;

			case OpT::Jam:
				// do nothing, stuck
				break;

			case OpT::INT:
				switch (spc_Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						ReadMemory(address_bus);
						break;

					case 1:
						// IRQ / NMI push PC, reset does not
						if (spc_IRQ_Type < 2)
						{
							WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						}
						else
						{
							address_bus = (uint16_t)(S + 0x100);

							ReadMemory(address_bus);
							S--;
						}
						break;

					case 2:
						// IRQ / NMI push PC, reset does not
						if (spc_IRQ_Type < 2)
						{
							WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						}
						else
						{
							address_bus = (uint16_t)(S + 0x100);

							ReadMemory(address_bus);
							S--;
						}
						break;

					case 3:
						// NMI, IRQ, Reset
						if (spc_IRQ_Type == 0)
						{
							spc_FlagBset(false);
							WriteMemory((uint16_t)(S-- + 0x100), P);
							spc_FlagIset(true); //is this right?
							ea = NMIVector;
						}
						else if (spc_IRQ_Type == 1)
						{
							spc_FlagBset(false);
							WriteMemory((uint16_t)(S-- + 0x100), P);
							spc_FlagIset(true);
							ea = IRQVector;
						}
						else
						{
							address_bus = (uint16_t)(S + 0x100);

							ea = ResetVector;
							ReadMemory(address_bus);
							S--;
							spc_FlagIset(true);
						}
						break;

					case 4:
						
						if (ea == IRQVector && !spc_FlagBget() && NMI)
						{
							NMI = false;
							ea = NMIVector;
						}
						address_bus = ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 5:
						
						address_bus = (uint16_t)(ea + 1);

						if (RDY)
						{
							alu_temp += ReadMemory(address_bus) << 8;
							PC = (uint16_t)alu_temp;
						}
						break;

					case 6:
						Fetch_Opcode_No_Interrupt();
						break;
				}
				break;

			case OpT::BRK:
				switch (spc_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						break;

					case 2:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						break;

					case 3:
						spc_FlagBset(true);
						WriteMemory((uint16_t)(S-- + 0x100), P);
						spc_FlagIset(true);
						ea = BRKVector;
						break;

					case 4:
						
						if (ea == BRKVector && spc_FlagBget() && NMI)
						{
							NMI = false;
							ea = NMIVector;
						}
						address_bus = (uint16_t)(ea);

						alu_temp = ReadMemory(address_bus);
						break;

					case 5:
						
						address_bus = (uint16_t)(ea + 1);

						alu_temp += ReadMemory(address_bus) << 8;
						PC = (uint16_t)alu_temp;
						break;

					case 6:
						Fetch_Opcode_No_Interrupt();
						break;
				}
				break;

			case OpT::DRMI:		
				Fetch_Dummy_Interrupt();
				break;

			case OpT::FONI:
				Fetch_Opcode_No_Interrupt();
				break;
		}

		spc_Instr_Cycle++;
	}

	void APU::spc_ALU_Operation()
	{
		switch (spc_ALU_Type)
		{
			case ALU::NOP:
				break;

			case ALU::ORA:
				A |= (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::AND:
				A &= (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::EOR:
				A ^= (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::LDA:
				A = (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::LDX:
				X = (uint8_t)alu_temp;
				NZ_X();
				break;

			case ALU::LDY:
				Y = (uint8_t)alu_temp;
				NZ_Y();
				break;

			case ALU::LAX:
				A = X = (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::ANC:
				A &= (uint8_t)alu_temp;
				spc_FlagCset((A & 0x80) == 0x80);
				NZ_A();
				break;

			case ALU::ASR:
				A &= (uint8_t)alu_temp;
				spc_FlagCset((A & 0x1) == 0x1);
				A >>= 1;
				NZ_A();
				break;

			case ALU::INX:
				X++;
				NZ_X();
				break;

			case ALU::INY:
				Y++;
				NZ_Y();
				break;

			case ALU::DEX:
				X--;
				NZ_X();
				break;

			case ALU::DEY:
				Y--;
				NZ_Y();
				break;

			case ALU::INC:
				alu_temp = (uint8_t)((alu_temp + 1) & 0xFF);
				P = (uint8_t)((P & 0x7D) | TableNZ[alu_temp]);
				break;

			case ALU::DEC:
				alu_temp = (uint8_t)((alu_temp - 1) & 0xFF);
				P = (uint8_t)((P & 0x7D) | TableNZ[alu_temp]);
				break;

			case ALU::ASL:
				value8 = (uint8_t)alu_temp;
				spc_FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)(value8 << 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROL:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				spc_FlagCset((temp8 & 0x80) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROR:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				spc_FlagCset((temp8 & 1) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::LSR:
				value8 = (uint8_t)alu_temp;
				spc_FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ASLA:
				spc_FlagCset((A & 0x80) != 0);
				A = (uint8_t)(A << 1);
				NZ_A();
				break;

			case ALU::ROLA:
				temp8 = A;
				A = (uint8_t)((A << 1) | (P & 1));
				spc_FlagCset((temp8 & 0x80) != 0);
				NZ_A();
				break;

			case ALU::RORA:
				temp8 = A;
				A = (uint8_t)((A >> 1) | ((P & 1) << 7));
				spc_FlagCset((temp8 & 1) != 0);
				NZ_A();
				break;

			case ALU::LSRA:
				spc_FlagCset((A & 1) != 0);
				A = (uint8_t)(A >> 1);
				NZ_A();
				break;

			case ALU::TXS:
				S = X;
				break;

			case ALU::TSX:
				X = S;
				NZ_X();
				break;

			case ALU::TAX:
				X = A;
				NZ_X();
				break;

			case ALU::TAY:
				Y = A;
				NZ_Y();
				break;

			case ALU::TYA:
				A = Y;
				NZ_A();
				break;

			case ALU::TXA:
				A = X;
				NZ_A();
				break;

			case ALU::SEC:
				spc_FlagCset(true);
				break;

			case ALU::CLC:
				spc_FlagCset(false);
				break;

			case ALU::SED:
				spc_FlagDset(true);
				break;

			case ALU::CLD:
				spc_FlagDset(false);
				break;

			case ALU::CLV:
				spc_FlagVset(false);
				break;

			case ALU::LAS:
				S &= (uint8_t)alu_temp;
				X = S;
				A = S;
				P = (uint8_t)((P & 0x7D) | TableNZ[S]);
				break;

			case ALU::AXS:
				X &= A;
				alu_temp = X - (uint8_t)alu_temp;
				X = (uint8_t)alu_temp;
				spc_FlagCset((alu_temp & 0x100) == 0x0);
				NZ_X();
				break;

			case ALU::BIT:
				spc_FlagNset((alu_temp & 0x80) != 0);
				spc_FlagVset((alu_temp & 0x40) != 0);
				spc_FlagZset((A & alu_temp) == 0);
				break;

			case ALU::DCP:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)(value8 - 1);
				spc_FlagCset((temp8 & 1) != 0);
				// pass through to cmp

			case ALU::CMP:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(A - value8);
				spc_FlagCset(A >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPX:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(X - value8);
				spc_FlagCset(X >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPY:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(Y - value8);
				spc_FlagCset(Y >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::RRA:
				value8 = (uint8_t)alu_temp;
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				spc_FlagCset((temp8 & 1) != 0);
				// pass through to adc

			case ALU::ADC:
				//TODO - an extra cycle penalty on 65C02 only
				value8 = (uint8_t)alu_temp;
				if (spc_FlagDget() && BCD_Enabled)
				{
					tempint = (A & 0x0F) + (value8 & 0x0F) + (spc_FlagCget() ? 0x01 : 0x00);
					if (tempint > 0x09)
						tempint += 0x06;
					tempint = (tempint & 0x0F) + (A & 0xF0) + (value8 & 0xF0) + (tempint > 0x0F ? 0x10 : 0x00);
					spc_FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					spc_FlagZset(((A + value8 + (spc_FlagCget() ? 1 : 0)) & 0xFF) == 0);
					spc_FlagNset((tempint & 0x80) != 0);
					if ((tempint & 0x1F0) > 0x090)
						tempint += 0x060;
					spc_FlagCset(tempint > 0xFF);
					A = (uint8_t)(tempint & 0xFF);
				}
				else
				{
					tempint = value8 + A + (spc_FlagCget() ? 1 : 0);
					spc_FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					spc_FlagCset(tempint > 0xFF);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			case ALU::ISC:
				value8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)(value8 + 1);
				// pass through to sbc

			case ALU::SBC:
				value8 = (uint8_t)alu_temp;
				tempint = A - value8 - (spc_FlagCget() ? 0 : 1);
				if (spc_FlagDget() && BCD_Enabled)
				{
					lo = (A & 0x0F) - (value8 & 0x0F) - (spc_FlagCget() ? 0 : 1);
					hi = (A & 0xF0) - (value8 & 0xF0);
					if ((lo & 0xF0) != 0) lo -= 0x06;
					if ((lo & 0x80) != 0) hi -= 0x10;
					if ((hi & 0x0F00) != 0) hi -= 0x60;
					spc_FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					spc_FlagZset((tempint & 0xFF) == 0);
					spc_FlagNset((tempint & 0x80) != 0);
					spc_FlagCset((hi & 0xFF00) == 0);
					A = (uint8_t)((lo & 0x0F) | (hi & 0xF0));
				}
				else
				{
					spc_FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					spc_FlagCset(tempint >= 0);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			case ALU::ARR:
				A &= (uint8_t)alu_temp;

				if (spc_FlagDget() && BCD_Enabled)
				{
					// Shift logic
					uint8_t next = (A >> 1) | (spc_FlagCget() ? 0x80 : 0x00);
					spc_FlagVset(((A ^ next) & 0x40) != 0);
					spc_FlagNset(spc_FlagCget());
					spc_FlagZset((next & 0xFF) == 0);

					// BCD fixup
					if ((A & 0x0F) + (A & 0x01) > 0x05)
					{
						next = (next & 0xF0) | ((next + 0x06) & 0x0F);
					}
					if ((A & 0xF0) + (A & 0x10) > 0x50)
					{
						next = (next & 0x0F) | ((next + 0x60) & 0xF0);
						spc_FlagCset(true);
					}
					else
					{
						spc_FlagCset(false);
					}

					A = (uint8_t)next;
				}
				else
				{
					booltemp = (A & 1) == 1;
					A = (uint8_t)((A >> 1) | (spc_FlagCget() ? 0x80 : 0x00));
					spc_FlagCset(booltemp);
					if ((A & 0x20) == 0x20)
						if ((A & 0x40) == 0x40)
						{
							spc_FlagCset(true); spc_FlagVset(false);
						}
						else { spc_FlagVset(true); spc_FlagCset(false); }
					else if ((A & 0x40) == 0x40)
					{
						spc_FlagVset(true); spc_FlagCset(true);
					}
					else { spc_FlagVset(false); spc_FlagCset(false); }
					NZ_A();
				}
				break;

			case ALU::RLA:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				spc_FlagCset((temp8 & 0x80) != 0);
				A &= value8;
				NZ_A();
				break;

			case ALU::SLO:
				value8 = (uint8_t)alu_temp;
				spc_FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)((value8 << 1));
				A |= value8;
				NZ_A();
				break;

			case ALU::ANE:
				// Many varied reports on what this should be.
				// safe value is 0xFF. Commodore 64 needs 0xEF.
				A |= AneConstant;
				A &= (uint8_t)(X & alu_temp);
				NZ_A();
				break;

			case ALU::LXA:
				//there is some debate about what this should be. it may depend on the 6502 variant.
				//this is suggested by qeed's doc for the SNES and passes blargg's instruction test
				A |= LxaConstant;
				A &= (uint8_t)alu_temp;
				X = A;
				NZ_A();
				break;

			case ALU::SRE:
				value8 = (uint8_t)alu_temp;
				spc_FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				A ^= value8;
				NZ_A();
				break;

			default:
				throw exception("bad op");

		}
	}

	void APU::spc_Write_Operation()
	{
		switch (spc_ALU_Type)
		{
			case ALU::STA:
				WriteMemory((uint16_t)ea, A);
				break;

			case ALU::STY:
				WriteMemory((uint16_t)ea, Y);
				break;

			case ALU::STX:
				WriteMemory((uint16_t)ea, X);
				break;

			case ALU::SAX:
				alu_temp = A & X;
				WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
				break;

			case ALU::SHA:
				alu_temp = A & X & H;
				WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
				break;

			case ALU::SHX:
				alu_temp = X & H;
				WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
				break;

			case ALU::SHY:
				alu_temp = Y & H;
				WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
				break;

			case ALU::SHS:
				S = (uint8_t)(X & A);
				WriteMemory((uint16_t)ea, (uint8_t)(S & H));
				break;
		}
	}

	void APU::Execute(int cycles)
	{
		for (int i = 0; i < cycles; i++)
		{
			ExecuteOne();
		}
	}

	void APU::Fetch1()
	{
		spc_Instr_Cycle = -1;

		Fetch_Opcode_No_Interrupt();
	}

	void APU::Fetch1_Branch()
	{
		spc_Instr_Cycle = -1;

		Fetch_Opcode_No_Interrupt();
	}

	void APU::Fetch_Opcode_No_Interrupt()
	{
		spc_Instr_Type = OpT::FONI;
		spc_Instr_Cycle = -1;
		
		
		address_bus = PC;

		branch_irq_hack = false;

		if (TraceCallback) TraceCallback(0);
		opcode = ReadMemory(address_bus);
		PC++;
		spc_Decode(opcode);
	}

	void APU::Fetch_Dummy_Interrupt()
	{
		spc_Instr_Type = OpT::DRMI;
		spc_Instr_Cycle = -1;
		
		
		address_bus = PC;

		ReadMemory(address_bus);
		spc_Instr_Type = OpT::INT;
	}

	void APU::Fetch2()
	{
		address_bus = PC;

		opcode2 = ReadMemory(address_bus);
		PC++;
	}

	void APU::Fetch3()
	{
		address_bus = PC;

		opcode3 = ReadMemory(address_bus);
		PC++;
	}
}