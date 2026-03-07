#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "SNES_System.h"

/*
*	Notes:
*	In order to pass accuracy coin tests we need the address updated even if RDY is false
*	Need to do the same IRQ behavior to interrupts as branch?
*   check timing of interaction of NMI hijacking interrrupt when RDY low
* 
*/

namespace SNESHawk
{
	void SNES_System::ExecuteOneOp()
	{
		switch (cpu_Instr_Type)
		{
			case OpT::Imp:
			case OpT::Acc:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
							cpu_ALU_Operation();
						}
						break;

					case 1:
						End();
						break;
				}
				break;
		
			case OpT::Imm:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
							PC++;
							cpu_ALU_Operation();
						}
						break;

					case 1:
						End();
						break;
				}
				break;

			case OpT::CSI:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						// not effected by RDY
						if (cpu_ALU_Type == ALU::CLI)
						{
							iflag_pending = false;
						}
						else
						{
							iflag_pending = true;
						}
						
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 1:
						End_ISpecial();
						break;
				}
				break;

			case OpT::PH:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
						}
						break;

					case 1:
						if (cpu_ALU_Type == ALU::PHP)
						{
							cpu_FlagBset(true);
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
						}
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(0x100 | S);

						if (RDY)
						{
							DummyReadMemory(address_bus);
							S++;
						}
						break;

					case 2:						
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							if (cpu_ALU_Type == ALU::PLP)
							{
								my_iflag = cpu_FlagIget();
								P = ReadMemory(address_bus);
								iflag_pending = cpu_FlagIget();
								cpu_FlagIset(my_iflag);
								cpu_FlagTset(true); //force T always to remain true
							}
							else
							{
								A = ReadMemory(address_bus);
								NZ_A();
							}
						}
						break;

					case 3:
						if (cpu_ALU_Type == ALU::PLP)
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
						}
						break;

					case 2:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						break;

					case 3:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						break;

					case 4:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							PC = (uint16_t)((ReadMemory(address_bus) << 8) + opcode2);
						}
						break;

					case 5:
						End();
						break;

				}
				break;
			
			case OpT::JMP:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							PC = (uint16_t)((ReadMemory(address_bus) << 8) + opcode2);
						}
						break;

					case 2:
						End();
						break;
				}
				break;

			case OpT::JMPI:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						Fetch3();
						break;

					case 2:
						RDY_Freeze = !RDY;
						ea = (opcode3 << 8) + opcode2;
						address_bus = (uint16_t)ea;

						if (RDY)
						{							
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						ea = (opcode3 << 8) + (uint8_t)(opcode2 + 1);
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp += ReadMemory(address_bus) << 8;
							PC = (uint16_t)alu_temp;
						}
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::RTI:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
						}
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(0x100 | S);

						if (RDY)
						{
							DummyReadMemory(address_bus);
							S++;
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							P = ReadMemory(address_bus);
							cpu_FlagTset(true); //force T always to remain true
							S++;
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF00;
							PC |= ReadMemory(address_bus);
							S++;
						}
						break;

					case 4:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF;
							PC |= (uint16_t)(ReadMemory(address_bus) << 8);
						}
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::RTS:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
						}
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(0x100 | S);

						if (RDY)
						{
							DummyReadMemory(address_bus);
							S++;
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF00;
							PC |= ReadMemory(address_bus);
							S++;
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF;
							PC |= (uint16_t)(ReadMemory(address_bus) << 8);
						}
						break;

					case 4:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
							PC++;
						}
						break;

					case 5:
						End();
						break;
				}
				break;


			case OpT::Br:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						switch (cpu_ALU_Type)
						{
							case ALU::BPL:
								branch_taken = !cpu_FlagNget();
								break;
							case ALU::BMI:
								branch_taken = cpu_FlagNget();
								break;
							case ALU::BVC:
								branch_taken = !cpu_FlagVget();
								break;
							case ALU::BVS:
								branch_taken = cpu_FlagVget();
								break;
							case ALU::BCC:
								branch_taken = !cpu_FlagCget();
								break;
							case ALU::BCS:
								branch_taken = cpu_FlagCget();
								break;
							case ALU::BNE:
								branch_taken = !cpu_FlagZget();
								break;
							case ALU::BEQ:
								branch_taken = cpu_FlagZget();
								break;
						}

						NMI_Br = false;
						IRQ_Br = false;

						RDY_Freeze = !RDY;
						address_bus = PC;

						cpu_First_Check = true;

						if (RDY)
						{
							opcode2 = ReadMemory(address_bus);
							PC++;

							if (branch_taken)
							{
								branch_taken = false;
							}
							else
							{
								cpu_Instr_Cycle+=2;
							}
						}
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (cpu_First_Check)
						{
							// This is the last check for interrupts in the non-branch crossing case
							NMI_Br = NMI;
							IRQ_Br = IRQ;

							cpu_First_Check = false;
						}

						if (RDY)
						{
							DummyReadMemory(address_bus);
							alu_temp = ((uint8_t)PC + (int8_t)opcode2);
							PC &= 0xFF00;
							PC |= (uint16_t)(alu_temp & 0xFF);

							if (!((alu_temp & 0x100) == 0x100))
							{								
								branch_irq_hack = true;

								cpu_Instr_Cycle++;
							}
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
							if (((alu_temp & 0x80000000) == 0x80000000))
								PC = (uint16_t)(PC - 0x100);
							else PC = (uint16_t)(PC + 0x100);
						}
						break;

					case 3:
						End_Branch();
						break;
				}
				break;
		
			case OpT::ZPR:
			case OpT::ZPW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						if (cpu_Instr_Type == OpT::ZPR)
						{
							RDY_Freeze = !RDY;
							address_bus = opcode2;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								cpu_ALU_Operation();
							}
						}
						else
						{
							ea = opcode2;
							cpu_Write_Operation();
						}
						break;

					case 2:
						End();
						break;
				}
				break;

			case OpT::ZPRW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 2:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						cpu_ALU_Operation();
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							ReadMemory(address_bus);

							if ((cpu_Instr_Type == OpT::ZPXR) || (cpu_Instr_Type == OpT::ZPXW))
							{
								opcode2 = (uint8_t)(opcode2 + X);
							}
							else
							{
								opcode2 = (uint8_t)(opcode2 + Y);
							}
						}
						break;

					case 2:
						if ((cpu_Instr_Type == OpT::ZPXR) || (cpu_Instr_Type == OpT::ZPYR))
						{
							RDY_Freeze = !RDY;
							address_bus = opcode2;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								cpu_ALU_Operation();
							}
						}
						else
						{
							ea = opcode2;
							cpu_Write_Operation();
						}
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::ZPXRW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							ReadMemory(address_bus);
							opcode2 = (uint8_t)(opcode2 + X);
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 3:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						cpu_ALU_Operation();
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						Fetch3();
						break;

					case 2:
						ea = (uint16_t)((opcode3 << 8) + opcode2);
						if (cpu_Instr_Type == OpT::AbsR)
						{
							RDY_Freeze = !RDY;
							address_bus = ea;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								cpu_ALU_Operation();
							}
						}
						else
						{
							cpu_Write_Operation();
						}
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::AbsRW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						Fetch3();
						break;

					case 2:
						RDY_Freeze = !RDY;
						ea = (opcode3 << 8) + opcode2;
						address_bus = ea;

						if (RDY)
						{				
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 3:
						WriteMemory(ea, (uint8_t)alu_temp);
						cpu_ALU_Operation();
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							DummyReadMemory(address_bus);
							alu_temp = (opcode2 + X) & 0xFF;
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)alu_temp;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint8_t)(alu_temp + 1);

						if (RDY)
						{
							ea += (ReadMemory(address_bus) << 8);
						}
						break;

					case 4:
						if (cpu_Instr_Type == OpT::AdXR)
						{
							RDY_Freeze = !RDY;
							address_bus = (uint16_t)ea;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								cpu_ALU_Operation();
							}
						}
						else
						{
							cpu_Write_Operation();
						}
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::AdXRW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							DummyReadMemory(address_bus);
							alu_temp = (opcode2 + X) & 0xFF;
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)alu_temp;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint8_t)(alu_temp + 1);

						if (RDY)
						{
							ea += (ReadMemory(address_bus) << 8);
						}
						break;

					case 4:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						cpu_ALU_Operation();
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							opcode3 = ReadMemory(address_bus);
							PC++;

							if ((cpu_Instr_Type == OpT::AIXR) || (cpu_Instr_Type == OpT::AIXW))
							{
								alu_temp = opcode2 + X;
							}
							else
							{
								alu_temp = opcode2 + Y;
							}
							ea = (opcode3 << 8) + (alu_temp & 0xFF);
							H = 0; // In preparation for SHY, set H to 0.

							if ((cpu_Instr_Type == OpT::AIXR) || (cpu_Instr_Type == OpT::AIYR))
							{
								// skip a cycle if no page crossing
								if (!((alu_temp & 0x100) == 0x100))
								{
									cpu_Instr_Cycle++;
								}
							}
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;
						
						if (RDY)
						{
							if ((cpu_Instr_Type == OpT::AIXR) || (cpu_Instr_Type == OpT::AIYR))
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
						}
						break;

					case 3:
						if ((cpu_Instr_Type == OpT::AIXR) || (cpu_Instr_Type == OpT::AIYR))
						{
							RDY_Freeze = !RDY;
							address_bus = (uint16_t)ea;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);

								cpu_ALU_Operation();
							}
						}
						else
						{
							cpu_Write_Operation();
						}
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::AIXRW:
			case OpT::AIYRW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							opcode3 = ReadMemory(address_bus);
							PC++;

							if (cpu_Instr_Type == OpT::AIXRW)
							{
								alu_temp = opcode2 + X;
							}
							else
							{
								alu_temp = opcode2 + Y;
							}
							ea = (opcode3 << 8) + (alu_temp & 0xFF);

							H = 0; // In preparation for SHY, set H to 0.
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
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
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 4:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						cpu_ALU_Operation();
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
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
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (cpu_ALU_Type == ALU::SHS)
						{
							if (RDY)
							{
								H |= (uint8_t)((ea >> 8) + 1);
								bool adjust = ((alu_temp & 0x100) == 0x100);
								alu_temp = ReadMemory(address_bus);

								if (adjust)
								{
									ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & ((A & X) << 8)));
								}
							}
							else
							{
								H = 0xFF; //If the RDY line is low here, the SHS instruction omits the bitwise AND with H
							}
						}
						else if (cpu_ALU_Type == ALU::SHY)
						{
							if (RDY)
							{
								H |= (uint8_t)((ea >> 8) + 1);
								bool adjust = ((alu_temp & 0x100) == 0x100);
								alu_temp = ReadMemory(address_bus);

								if (adjust)
								{
									ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & (Y << 8)));
								}
							}
							else
							{
								H = 0xFF; //If the RDY line is low here, the SHY instruction omits the bitwise AND with H
							}
						}
						else if (cpu_ALU_Type == ALU::SHX)
						{
							if (RDY)
							{
								H |= (uint8_t)((ea >> 8) + 1);
								bool adjust = ((alu_temp & 0x100) == 0x100);
								alu_temp = ReadMemory(address_bus);

								if (adjust)
								{
									ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & (X << 8)));
								}
							}
							else
							{
								H = 0xFF; //If the RDY line is low here, the SHX instruction omits the bitwise AND with H
							}
						}
						else // SHA
						{
							if (RDY)
							{
								H |= (uint8_t)((ea >> 8) + 1);
								bool adjust = ((alu_temp & 0x100) == 0x100);
								alu_temp = ReadMemory(address_bus);

								if (adjust)
								{
									ea = (uint16_t)((ea & 0xFF) | ((ea + 0x100) & 0xFF00 & ((A & X) << 8)));
								}
							}
							else
							{
								H = 0xFF; //If the RDY line is low here, the SHA instruction omits the bitwise AND with H
							}
						}
						break;

					case 3:
						cpu_Write_Operation();
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::IIYR:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint8_t)(opcode2 + 1);

						if (RDY)
						{
							alu_temp = ea + Y;
							ea = (ReadMemory(address_bus) << 8)
								| ((alu_temp & 0xFF));

							H = 0; // In preparation for SHA (indirect, X), set H to 0.

							if (!((alu_temp & 0x100) == 0x100))
							{
								cpu_Instr_Cycle++;
							}
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							ReadMemory(address_bus);
							ea = (uint16_t)(ea + 0x100);
						}
						break;

					case 4:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
							cpu_ALU_Operation();
						}
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::IIYW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint8_t)(opcode2 + 1);

						if (RDY)
						{
							alu_temp = ea + Y;
							ea = (ReadMemory(address_bus) << 8)
								| ((alu_temp & 0xFF));

							H = 0; // In preparation for SHA (indirect, X), set H to 0.
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (opcode == 0x91)
						{
							if (RDY)
							{
								ReadMemory(address_bus);
								ea += (alu_temp >> 8) << 8;
							}
						}
						else
						{
							if (RDY)
							{
								H |= (uint8_t)((ea >> 8) + 1);
								ReadMemory(address_bus);

								if ((alu_temp & 0x100) == 0x100)
								{
									ea = (uint16_t)(ea & 0xFF | ((ea + 0x100) & 0xFF00 & ((A & X) << 8)));
								}
							}
							else
							{
								H = 0xFF; //If the RDY line is low here, the SHA instruction omits the bitwise AND with H
							}
						}
						break;

					case 4:
						cpu_Write_Operation();
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::IIYRW:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						Fetch2();
						break;

					case 1:
						RDY_Freeze = !RDY;
						address_bus = opcode2;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 2:
						RDY_Freeze = !RDY;
						address_bus = (uint8_t)(opcode2 + 1);

						if (RDY)
						{
							alu_temp = ea + Y;
							ea = (ReadMemory(address_bus) << 8)
								| ((alu_temp & 0xFF));

							H = 0; // In preparation for SHA (indirect, X), set H to 0.
						}
						break;

					case 3:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							ReadMemory(address_bus);
							if ((alu_temp & 0x100) == 0x100)
								ea = (uint16_t)(ea + 0x100);
						}
						break;

					case 4:
						RDY_Freeze = !RDY;
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						cpu_ALU_Operation();
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
				switch (cpu_Instr_Cycle)
				{
					case 0:
						RDY_Freeze = !RDY;
						address_bus = PC;

						if (RDY)
						{
							DummyReadMemory(address_bus);
						}
						break;

					case 1:
						// IRQ / NMI push PC, reset does not
						if (cpu_IRQ_Type < 2)
						{
							WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						}
						else
						{
							RDY_Freeze = !RDY;
							address_bus = (uint16_t)(S + 0x100);

							if (RDY)
							{
								DummyReadMemory(address_bus);
								S--;
							}
						}
						break;

					case 2:
						// IRQ / NMI push PC, reset does not
						if (cpu_IRQ_Type < 2)
						{
							WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						}
						else
						{
							RDY_Freeze = !RDY;
							address_bus = (uint16_t)(S + 0x100);

							if (RDY)
							{
								DummyReadMemory(address_bus);
								S--;
							}
						}
						break;

					case 3:
						// NMI, IRQ, Reset
						if (cpu_IRQ_Type == 0)
						{
							cpu_FlagBset(false);
							WriteMemory((uint16_t)(S-- + 0x100), P);
							cpu_FlagIset(true); //is this right?
							ea = NMIVector;
						}
						else if (cpu_IRQ_Type == 1)
						{
							cpu_FlagBset(false);
							WriteMemory((uint16_t)(S-- + 0x100), P);
							cpu_FlagIset(true);
							ea = IRQVector;
						}
						else
						{
							RDY_Freeze = !RDY;
							address_bus = (uint16_t)(S + 0x100);

							if (RDY)
							{
								ea = ResetVector;
								DummyReadMemory(address_bus);
								S--;
								cpu_FlagIset(true);
							}
						}
						break;

					case 4:
						RDY_Freeze = !RDY;
						if (ea == IRQVector && !cpu_FlagBget() && NMI)
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
						RDY_Freeze = !RDY;
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
				switch (cpu_Instr_Cycle)
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
						cpu_FlagBset(true);
						WriteMemory((uint16_t)(S-- + 0x100), P);
						cpu_FlagIset(true);
						ea = BRKVector;
						break;

					case 4:
						RDY_Freeze = !RDY;
						if (ea == BRKVector && cpu_FlagBget() && NMI)
						{
							NMI = false;
							ea = NMIVector;
						}
						address_bus = (uint16_t)(ea);

						if (RDY)
						{	
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 5:
						RDY_Freeze = !RDY;
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

			case OpT::DRMI:		
				Fetch_Dummy_Interrupt();
				break;

			case OpT::FONI:
				Fetch_Opcode_No_Interrupt();
				break;
		}

		if (!RDY_Freeze)
		{
			cpu_Instr_Cycle++;
		}
	}

	void SNES_System::cpu_ALU_Operation()
	{
		switch (cpu_ALU_Type)
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
				cpu_FlagCset((A & 0x80) == 0x80);
				NZ_A();
				break;

			case ALU::ASR:
				A &= (uint8_t)alu_temp;
				cpu_FlagCset((A & 0x1) == 0x1);
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
				cpu_FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)(value8 << 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROL:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				cpu_FlagCset((temp8 & 0x80) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROR:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				cpu_FlagCset((temp8 & 1) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::LSR:
				value8 = (uint8_t)alu_temp;
				cpu_FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ASLA:
				cpu_FlagCset((A & 0x80) != 0);
				A = (uint8_t)(A << 1);
				NZ_A();
				break;

			case ALU::ROLA:
				temp8 = A;
				A = (uint8_t)((A << 1) | (P & 1));
				cpu_FlagCset((temp8 & 0x80) != 0);
				NZ_A();
				break;

			case ALU::RORA:
				temp8 = A;
				A = (uint8_t)((A >> 1) | ((P & 1) << 7));
				cpu_FlagCset((temp8 & 1) != 0);
				NZ_A();
				break;

			case ALU::LSRA:
				cpu_FlagCset((A & 1) != 0);
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
				cpu_FlagCset(true);
				break;

			case ALU::CLC:
				cpu_FlagCset(false);
				break;

			case ALU::SED:
				cpu_FlagDset(true);
				break;

			case ALU::CLD:
				cpu_FlagDset(false);
				break;

			case ALU::CLV:
				cpu_FlagVset(false);
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
				cpu_FlagCset((alu_temp & 0x100) == 0x0);
				NZ_X();
				break;

			case ALU::BIT:
				cpu_FlagNset((alu_temp & 0x80) != 0);
				cpu_FlagVset((alu_temp & 0x40) != 0);
				cpu_FlagZset((A & alu_temp) == 0);
				break;

			case ALU::DCP:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)(value8 - 1);
				cpu_FlagCset((temp8 & 1) != 0);
				// pass through to cmp

			case ALU::CMP:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(A - value8);
				cpu_FlagCset(A >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPX:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(X - value8);
				cpu_FlagCset(X >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPY:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(Y - value8);
				cpu_FlagCset(Y >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::RRA:
				value8 = (uint8_t)alu_temp;
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				cpu_FlagCset((temp8 & 1) != 0);
				// pass through to adc

			case ALU::ADC:
				//TODO - an extra cycle penalty on 65C02 only
				value8 = (uint8_t)alu_temp;
				if (cpu_FlagDget() && BCD_Enabled)
				{
					tempint = (A & 0x0F) + (value8 & 0x0F) + (cpu_FlagCget() ? 0x01 : 0x00);
					if (tempint > 0x09)
						tempint += 0x06;
					tempint = (tempint & 0x0F) + (A & 0xF0) + (value8 & 0xF0) + (tempint > 0x0F ? 0x10 : 0x00);
					cpu_FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					cpu_FlagZset(((A + value8 + (cpu_FlagCget() ? 1 : 0)) & 0xFF) == 0);
					cpu_FlagNset((tempint & 0x80) != 0);
					if ((tempint & 0x1F0) > 0x090)
						tempint += 0x060;
					cpu_FlagCset(tempint > 0xFF);
					A = (uint8_t)(tempint & 0xFF);
				}
				else
				{
					tempint = value8 + A + (cpu_FlagCget() ? 1 : 0);
					cpu_FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					cpu_FlagCset(tempint > 0xFF);
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
				tempint = A - value8 - (cpu_FlagCget() ? 0 : 1);
				if (cpu_FlagDget() && BCD_Enabled)
				{
					lo = (A & 0x0F) - (value8 & 0x0F) - (cpu_FlagCget() ? 0 : 1);
					hi = (A & 0xF0) - (value8 & 0xF0);
					if ((lo & 0xF0) != 0) lo -= 0x06;
					if ((lo & 0x80) != 0) hi -= 0x10;
					if ((hi & 0x0F00) != 0) hi -= 0x60;
					cpu_FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					cpu_FlagZset((tempint & 0xFF) == 0);
					cpu_FlagNset((tempint & 0x80) != 0);
					cpu_FlagCset((hi & 0xFF00) == 0);
					A = (uint8_t)((lo & 0x0F) | (hi & 0xF0));
				}
				else
				{
					cpu_FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					cpu_FlagCset(tempint >= 0);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			case ALU::ARR:
				A &= (uint8_t)alu_temp;

				if (cpu_FlagDget() && BCD_Enabled)
				{
					// Shift logic
					uint8_t next = (A >> 1) | (cpu_FlagCget() ? 0x80 : 0x00);
					cpu_FlagVset(((A ^ next) & 0x40) != 0);
					cpu_FlagNset(cpu_FlagCget());
					cpu_FlagZset((next & 0xFF) == 0);

					// BCD fixup
					if ((A & 0x0F) + (A & 0x01) > 0x05)
					{
						next = (next & 0xF0) | ((next + 0x06) & 0x0F);
					}
					if ((A & 0xF0) + (A & 0x10) > 0x50)
					{
						next = (next & 0x0F) | ((next + 0x60) & 0xF0);
						cpu_FlagCset(true);
					}
					else
					{
						cpu_FlagCset(false);
					}

					A = (uint8_t)next;
				}
				else
				{
					booltemp = (A & 1) == 1;
					A = (uint8_t)((A >> 1) | (cpu_FlagCget() ? 0x80 : 0x00));
					cpu_FlagCset(booltemp);
					if ((A & 0x20) == 0x20)
						if ((A & 0x40) == 0x40)
						{
							cpu_FlagCset(true); cpu_FlagVset(false);
						}
						else { cpu_FlagVset(true); cpu_FlagCset(false); }
					else if ((A & 0x40) == 0x40)
					{
						cpu_FlagVset(true); cpu_FlagCset(true);
					}
					else { cpu_FlagVset(false); cpu_FlagCset(false); }
					NZ_A();
				}
				break;

			case ALU::RLA:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				cpu_FlagCset((temp8 & 0x80) != 0);
				A &= value8;
				NZ_A();
				break;

			case ALU::SLO:
				value8 = (uint8_t)alu_temp;
				cpu_FlagCset((value8 & 0x80) != 0);
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
				cpu_FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				A ^= value8;
				NZ_A();
				break;

			default:
				throw exception("bad op");

		}
	}

	void SNES_System::cpu_Write_Operation()
	{
		switch (cpu_ALU_Type)
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

	void SNES_System::Execute(int cycles)
	{
		for (int i = 0; i < cycles; i++)
		{
			ExecuteOne();
		}
	}

	void SNES_System::Fetch1()
	{
		cpu_Instr_Cycle = -1;
		my_iflag = cpu_FlagIget();
		cpu_FlagIset(iflag_pending);

		if (NMI)
		{
			if (TraceCallback) TraceCallback(1);
			ea = NMIVector;

			cpu_Instr_Type = OpT::INT;
			cpu_IRQ_Type = 0; // NMI
			NMI = false;

			Fetch_Dummy_Interrupt();
			return;
		}

		if (IRQ && !my_iflag)
		{
			if (TraceCallback) TraceCallback(2);
			ea = IRQVector;

			cpu_Instr_Type = OpT::INT;
			cpu_IRQ_Type = 1; // IRQ

			Fetch_Dummy_Interrupt();
			return;
		}

		Fetch_Opcode_No_Interrupt();
	}

	void SNES_System::Fetch1_Branch()
	{
		cpu_Instr_Cycle = -1;
		my_iflag = cpu_FlagIget();
		cpu_FlagIset(iflag_pending);

		if (!branch_irq_hack)
		{
			// will interrupt if either poll is true
			if (NMI_Br || NMI)
			{
				if (TraceCallback) TraceCallback(1);
				ea = NMIVector;

				cpu_Instr_Type = OpT::INT;
				cpu_IRQ_Type = 0; // NMI
				NMI = false;

				Fetch_Dummy_Interrupt();
				return;
			}

			if ((IRQ_Br || IRQ) && !my_iflag)
			{
				if (TraceCallback) TraceCallback(2);
				ea = IRQVector;

				cpu_Instr_Type = OpT::INT;
				cpu_IRQ_Type = 1; // IRQ

				Fetch_Dummy_Interrupt();
				return;
			}
		}
		else
		{
			if (NMI_Br)
			{
				if (TraceCallback) TraceCallback(1);
				ea = NMIVector;

				cpu_Instr_Type = OpT::INT;
				cpu_IRQ_Type = 0; // NMI
				NMI = false;

				Fetch_Dummy_Interrupt();
				return;
			}

			if (IRQ_Br && !my_iflag)
			{
				if (TraceCallback) TraceCallback(2);
				ea = IRQVector;

				cpu_Instr_Type = OpT::INT;
				cpu_IRQ_Type = 1; // IRQ

				Fetch_Dummy_Interrupt();
				return;
			}
		}

		Fetch_Opcode_No_Interrupt();
	}

	void SNES_System::Fetch_Opcode_No_Interrupt()
	{
		cpu_Instr_Type = OpT::FONI;
		cpu_Instr_Cycle = -1;
		
		RDY_Freeze = !RDY;
		address_bus = PC;

		if (RDY)
		{
			branch_irq_hack = false;
			OnExecFetch(PC);
			if (TraceCallback) TraceCallback(0);
			opcode = ReadMemory(address_bus);
			PC++;
			cpu_Decode(opcode);
		}
	}

	void SNES_System::Fetch_Dummy_Interrupt()
	{
		cpu_Instr_Type = OpT::DRMI;
		cpu_Instr_Cycle = -1;
		
		RDY_Freeze = !RDY;
		address_bus = PC;

		if (RDY)
		{
			DummyReadMemory(address_bus);
			cpu_Instr_Type = OpT::INT;
		}
	}

	void SNES_System::Fetch2()
	{
		RDY_Freeze = !RDY;
		address_bus = PC;

		if (RDY)
		{
			opcode2 = ReadMemory(address_bus);
			PC++;
		}
	}

	void SNES_System::Fetch3()
	{
		RDY_Freeze = !RDY;
		address_bus = PC;

		if (RDY)
		{
			opcode3 = ReadMemory(address_bus);
			PC++;
		}
	}
}