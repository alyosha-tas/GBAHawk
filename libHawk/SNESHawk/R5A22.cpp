#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "R5A22.h"
#include "SNES_System.h"

namespace SNESHawk
{

	void R5A22::RunCpuOne()
	{
		if (RDY)
		{
			Fetch_Cnt++;

			if (Fetch_Cnt == Fetch_Op)
			{
				switch (Cycle_Type)
				{
					case CPU_Cycle_Type::Read_Cycle:
						alu_temp = Sys_pntr->ReadMemory(address_bus);
						break;

					case CPU_Cycle_Type::Read_Cycle_Hi:
						alu_temp_hi = Sys_pntr->ReadMemory(address_bus);
						break;

					case CPU_Cycle_Type::Write_Cycle:
						break;

					case CPU_Cycle_Type::Fetch_ALU_Cycle:
						ALU_Operation();
						// fall through to normal fetch cycle

					case CPU_Cycle_Type::Fetch_Cycle:
						iflag_pending = FlagIget();
						// fall through to no check case

					case CPU_Cycle_Type::Fetch_Cycle_No_Check:
						Fetch1();
						break;

					case CPU_Cycle_Type::Fetch_2:
						opcode2 = Sys_pntr->ReadMemory(address_bus);
						break;

					case CPU_Cycle_Type::Fetch_3:
						opcode3 = Sys_pntr->ReadMemory(address_bus);
						break;

					case CPU_Cycle_Type::Internal_Cycle:

						break;

					case CPU_Cycle_Type::PC_Change_Cycle:
						break;

					case CPU_Cycle_Type::Fetch_Reset:
						// do nothing as this is just a cycle that immediately goes into the interrupt handler with a reset
						Sys_pntr->ReadMemory(address_bus);
						break;
				}
			}
			else if (Fetch_Cnt == Fetch_Wait)
			{
				Instr_Cycle++;

				Fetch_Cnt = 0;

				ExecuteOneOp();

				Fetch_Wait = Calculate_Wait_States();
			}
		}
	}
	
	
	void R5A22::ExecuteOneOp()
	{
		switch (Instr_Type)
		{
			case OpT::Imp:
			case OpT::Acc:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;										
						break;

					case 1:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;
		
			case OpT::Imm:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						PC++;

						Instr_Cycle += Instr_Skip;
						break;

					case 1:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						PC++;
						break;

					case 2:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::CSI:
				switch (Instr_Cycle)
				{
					case 0:
						iflag_pending = ALU_Type != ALU::CLI;
						
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle_No_Check;
						break;
				}
				break;

			case OpT::PH:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						// calculate stack address

						// pick a value to push
						if (ALU_Type == ALU::PHP)
						{
							FlagBset(true);
							WriteMemory((uint16_t)(S-- + 0x100), P);
						}
						else
						{
							WriteMemory((uint16_t)(S-- + 0x100), A);
						}
						Instr_Cycle += Instr_Skip;
						break;

					case 2:
						if (ALU_Type == ALU::PHP)
						{
							FlagBset(true);
							WriteMemory((uint16_t)(S-- + 0x100), P);
						}
						else
						{
							WriteMemory((uint16_t)(S-- + 0x100), A);
						}
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::PL:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
						}
						break;

					case 1:
						address_bus = (uint16_t)(0x100 | S);

						if (RDY)
						{
							ReadMemory(address_bus);
							S++;
						}
						break;

					case 2:						
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							if (ALU_Type == ALU::PLP)
							{
								my_iflag = FlagIget();
								P = ReadMemory(address_bus);
								iflag_pending = FlagIget();
								FlagIset(my_iflag);
								FlagTset(true); //force T always to remain true
							}
							else
							{
								A = ReadMemory(address_bus);
								NZ_A();
							}
						}
						break;

					case 3:
						if (ALU_Type == ALU::PLP)
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
						}
						break;

					case 2:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						break;

					case 3:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						break;

					case 4:
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						ea = (opcode3 << 8) + opcode2;
						address_bus = (uint16_t)ea;

						if (RDY)
						{							
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 3:
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
						}
						break;

					case 1:
						address_bus = (uint16_t)(0x100 | S);

						if (RDY)
						{
							ReadMemory(address_bus);
							S++;
						}
						break;

					case 2:
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							P = ReadMemory(address_bus);
							FlagTset(true); //force T always to remain true
							S++;
						}
						break;

					case 3:
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF00;
							PC |= ReadMemory(address_bus);
							S++;
						}
						break;

					case 4:
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
						}
						break;

					case 1:
						address_bus = (uint16_t)(0x100 | S);

						if (RDY)
						{
							ReadMemory(address_bus);
							S++;
						}
						break;

					case 2:
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF00;
							PC |= ReadMemory(address_bus);
							S++;
						}
						break;

					case 3:
						address_bus = (uint16_t)(S + 0x100);

						if (RDY)
						{
							PC &= 0xFF;
							PC |= (uint16_t)(ReadMemory(address_bus) << 8);
						}
						break;

					case 4:
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
				switch (Instr_Cycle)
				{
					case 0:
						switch (ALU_Type)
						{
							case ALU::BPL:
								branch_taken = !FlagNget();
								break;
							case ALU::BMI:
								branch_taken = FlagNget();
								break;
							case ALU::BVC:
								branch_taken = !FlagVget();
								break;
							case ALU::BVS:
								branch_taken = FlagVget();
								break;
							case ALU::BCC:
								branch_taken = !FlagCget();
								break;
							case ALU::BCS:
								branch_taken = FlagCget();
								break;
							case ALU::BNE:
								branch_taken = !FlagZget();
								break;
							case ALU::BEQ:
								branch_taken = FlagZget();
								break;
						}

						address_bus = PC;

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
								Instr_Cycle+=2;
							}
						}
						break;

					case 1:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
							alu_temp = ((uint8_t)PC + (int8_t)opcode2);
							PC &= 0xFF00;
							PC |= (uint16_t)(alu_temp & 0xFF);

							if (!((alu_temp & 0x100) == 0x100))
							{								
								Instr_Cycle++;
							}
						}
						break;

					case 2:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
							if (((alu_temp & 0x80000000) == 0x80000000))
								PC = (uint16_t)(PC - 0x100);
							else PC = (uint16_t)(PC + 0x100);
						}
						break;

					case 3:
						End();
						break;
				}
				break;
		
			case OpT::ZPR:
			case OpT::ZPW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						if (Instr_Type == OpT::ZPR)
						{
							address_bus = opcode2;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								ALU_Operation();
							}
						}
						else
						{
							ea = opcode2;
							Write_Operation();
						}
						break;

					case 2:
						End();
						break;
				}
				break;

			case OpT::ZPRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 2:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						ALU_Operation();
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ReadMemory(address_bus);

							if ((Instr_Type == OpT::ZPXR) || (Instr_Type == OpT::ZPXW))
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
						if ((Instr_Type == OpT::ZPXR) || (Instr_Type == OpT::ZPYR))
						{
							address_bus = opcode2;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								ALU_Operation();
							}
						}
						else
						{
							ea = opcode2;
							Write_Operation();
						}
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::ZPXRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ReadMemory(address_bus);
							opcode2 = (uint8_t)(opcode2 + X);
						}
						break;

					case 2:
						address_bus = opcode2;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 3:
						WriteMemory(opcode2, (uint8_t)alu_temp);
						ALU_Operation();
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						ea = (uint16_t)((opcode3 << 8) + opcode2);
						if (Instr_Type == OpT::AbsR)
						{
							address_bus = ea;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								ALU_Operation();
							}
						}
						else
						{
							Write_Operation();
						}
						break;

					case 3:
						End();
						break;
				}
				break;

			case OpT::AbsRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						ea = (opcode3 << 8) + opcode2;
						address_bus = ea;

						if (RDY)
						{				
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 3:
						WriteMemory(ea, (uint8_t)alu_temp);
						ALU_Operation();
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ReadMemory(address_bus);
							alu_temp = (opcode2 + X) & 0xFF;
						}
						break;

					case 2:
						address_bus = (uint16_t)alu_temp;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 3:
						address_bus = (uint8_t)(alu_temp + 1);

						if (RDY)
						{
							ea += (ReadMemory(address_bus) << 8);
						}
						break;

					case 4:
						if (Instr_Type == OpT::AdXR)
						{
							address_bus = (uint16_t)ea;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);
								ALU_Operation();
							}
						}
						else
						{
							Write_Operation();
						}
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::AdXRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ReadMemory(address_bus);
							alu_temp = (opcode2 + X) & 0xFF;
						}
						break;

					case 2:
						address_bus = (uint16_t)alu_temp;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 3:
						address_bus = (uint8_t)(alu_temp + 1);

						if (RDY)
						{
							ea += (ReadMemory(address_bus) << 8);
						}
						break;

					case 4:
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						ALU_Operation();
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = PC;

						if (RDY)
						{
							opcode3 = ReadMemory(address_bus);
							PC++;

							if ((Instr_Type == OpT::AIXR) || (Instr_Type == OpT::AIXW))
							{
								alu_temp = opcode2 + X;
							}
							else
							{
								alu_temp = opcode2 + Y;
							}
							ea = (opcode3 << 8) + (alu_temp & 0xFF);
							H = 0; // In preparation for SHY, set H to 0.

							if ((Instr_Type == OpT::AIXR) || (Instr_Type == OpT::AIYR))
							{
								// skip a cycle if no page crossing
								if (!((alu_temp & 0x100) == 0x100))
								{
									Instr_Cycle++;
								}
							}
						}
						break;

					case 2:
						address_bus = (uint16_t)ea;
						
						if (RDY)
						{
							if ((Instr_Type == OpT::AIXR) || (Instr_Type == OpT::AIYR))
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
						if ((Instr_Type == OpT::AIXR) || (Instr_Type == OpT::AIYR))
						{
							address_bus = (uint16_t)ea;

							if (RDY)
							{
								alu_temp = ReadMemory(address_bus);

								ALU_Operation();
							}
						}
						else
						{
							Write_Operation();
						}
						break;

					case 4:
						End();
						break;
				}
				break;

			case OpT::AIXRW:
			case OpT::AIYRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = PC;

						if (RDY)
						{
							opcode3 = ReadMemory(address_bus);
							PC++;

							if (Instr_Type == OpT::AIXRW)
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
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 4:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						ALU_Operation();
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						break;

					case 6:
						End();
						break;
				}
				break;

			case OpT::IIYR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 2:
						address_bus = (uint8_t)(opcode2 + 1);

						if (RDY)
						{
							alu_temp = ea + Y;
							ea = (ReadMemory(address_bus) << 8)
								| ((alu_temp & 0xFF));

							H = 0; // In preparation for SHA (indirect, X), set H to 0.

							if (!((alu_temp & 0x100) == 0x100))
							{
								Instr_Cycle++;
							}
						}
						break;

					case 3:
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							ReadMemory(address_bus);
							ea = (uint16_t)(ea + 0x100);
						}
						break;

					case 4:
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
							ALU_Operation();
						}
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::IIYW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 2:
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
						Write_Operation();
						break;

					case 5:
						End();
						break;
				}
				break;

			case OpT::IIYRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = opcode2;

						if (RDY)
						{
							ea = ReadMemory(address_bus);
						}
						break;

					case 2:
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
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							ReadMemory(address_bus);
							if ((alu_temp & 0x100) == 0x100)
								ea = (uint16_t)(ea + 0x100);
						}
						break;

					case 4:
						address_bus = (uint16_t)ea;

						if (RDY)
						{
							alu_temp = ReadMemory(address_bus);
						}
						break;

					case 5:
						WriteMemory((uint16_t)ea, (uint8_t)alu_temp);
						ALU_Operation();
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;

						if (RDY)
						{
							ReadMemory(address_bus);
						}
						break;

					case 1:
						// IRQ / NMI push PC, reset does not
						if (IRQ_Type < 2)
						{
							WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						}
						else
						{
							address_bus = (uint16_t)(S + 0x100);

							if (RDY)
							{
								ReadMemory(address_bus);
								S--;
							}
						}
						break;

					case 2:
						// IRQ / NMI push PC, reset does not
						if (IRQ_Type < 2)
						{
							WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						}
						else
						{
							address_bus = (uint16_t)(S + 0x100);

							if (RDY)
							{
								ReadMemory(address_bus);
								S--;
							}
						}
						break;

					case 3:
						// NMI, IRQ, Reset
						if (IRQ_Type == 0)
						{
							FlagBset(false);
							WriteMemory((uint16_t)(S-- + 0x100), P);
							FlagIset(true); //is this right?
							ea = NMIVector;
						}
						else if (IRQ_Type == 1)
						{
							FlagBset(false);
							WriteMemory((uint16_t)(S-- + 0x100), P);
							FlagIset(true);
							ea = IRQVector;
						}
						else
						{
							address_bus = (uint16_t)(S + 0x100);

							if (RDY)
							{
								ea = ResetVector;
								ReadMemory(address_bus);
								S--;
								FlagIset(true);
							}
						}
						break;

					case 4:
						if (ea == IRQVector && !FlagBget() && NMI)
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
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = PC;
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)(PC >> 8));
						break;

					case 2:
						WriteMemory((uint16_t)(S-- + 0x100), (uint8_t)PC);
						break;

					case 3:
						FlagBset(true);
						WriteMemory((uint16_t)(S-- + 0x100), P);
						FlagIset(true);
						ea = BRKVector;
						break;

					case 4:
						if (ea == BRKVector && FlagBget() && NMI)
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
		}
	}

	void R5A22::ALU_Operation()
	{
		switch (ALU_Type)
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

			case ALU::ASR:
				A &= (uint8_t)alu_temp;
				FlagCset((A & 0x1) == 0x1);
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
				FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)(value8 << 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROL:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROR:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::LSR:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ASLA:
				FlagCset((A & 0x80) != 0);
				A = (uint8_t)(A << 1);
				NZ_A();
				break;

			case ALU::ROLA:
				temp8 = A;
				A = (uint8_t)((A << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				NZ_A();
				break;

			case ALU::RORA:
				temp8 = A;
				A = (uint8_t)((A >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				NZ_A();
				break;

			case ALU::LSRA:
				FlagCset((A & 1) != 0);
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
				FlagCset(true);
				break;

			case ALU::CLC:
				FlagCset(false);
				break;

			case ALU::SED:
				FlagDset(true);
				break;

			case ALU::CLD:
				FlagDset(false);
				break;

			case ALU::CLV:
				FlagVset(false);
				break;

			case ALU::BIT:
				FlagNset((alu_temp & 0x80) != 0);
				FlagVset((alu_temp & 0x40) != 0);
				FlagZset((A & alu_temp) == 0);
				break;

			case ALU::CMP:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(A - value8);
				FlagCset(A >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPX:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(X - value8);
				FlagCset(X >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPY:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(Y - value8);
				FlagCset(Y >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::RRA:
				value8 = (uint8_t)alu_temp;
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				// pass through to adc

			case ALU::ADC:
				//TODO - an extra cycle penalty on 65C02 only
				value8 = (uint8_t)alu_temp;
				if (FlagDget() && BCD_Enabled)
				{
					tempint = (A & 0x0F) + (value8 & 0x0F) + (FlagCget() ? 0x01 : 0x00);
					if (tempint > 0x09)
						tempint += 0x06;
					tempint = (tempint & 0x0F) + (A & 0xF0) + (value8 & 0xF0) + (tempint > 0x0F ? 0x10 : 0x00);
					FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagZset(((A + value8 + (FlagCget() ? 1 : 0)) & 0xFF) == 0);
					FlagNset((tempint & 0x80) != 0);
					if ((tempint & 0x1F0) > 0x090)
						tempint += 0x060;
					FlagCset(tempint > 0xFF);
					A = (uint8_t)(tempint & 0xFF);
				}
				else
				{
					tempint = value8 + A + (FlagCget() ? 1 : 0);
					FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagCset(tempint > 0xFF);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			case ALU::SBC:
				value8 = (uint8_t)alu_temp;
				tempint = A - value8 - (FlagCget() ? 0 : 1);
				if (FlagDget() && BCD_Enabled)
				{
					lo = (A & 0x0F) - (value8 & 0x0F) - (FlagCget() ? 0 : 1);
					hi = (A & 0xF0) - (value8 & 0xF0);
					if ((lo & 0xF0) != 0) lo -= 0x06;
					if ((lo & 0x80) != 0) hi -= 0x10;
					if ((hi & 0x0F00) != 0) hi -= 0x60;
					FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagZset((tempint & 0xFF) == 0);
					FlagNset((tempint & 0x80) != 0);
					FlagCset((hi & 0xFF00) == 0);
					A = (uint8_t)((lo & 0x0F) | (hi & 0xF0));
				}
				else
				{
					FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagCset(tempint >= 0);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			case ALU::NOP_16:
				break;

			case ALU::ORA_16:
				A |= (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::AND_16:
				A &= (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::EOR_16:
				A ^= (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::LDA_16:
				A = (uint8_t)alu_temp;
				NZ_A();
				break;

			case ALU::LDX_16:
				X = (uint8_t)alu_temp;
				NZ_X();
				break;

			case ALU::LDY_16:
				Y = (uint8_t)alu_temp;
				NZ_Y();
				break;

			case ALU::ASR_16:
				A &= (uint8_t)alu_temp;
				FlagCset((A & 0x1) == 0x1);
				A >>= 1;
				NZ_A();
				break;

			case ALU::INX_16:
				X++;
				NZ_X();
				break;

			case ALU::INY_16:
				Y++;
				NZ_Y();
				break;

			case ALU::DEX_16:
				X--;
				NZ_X();
				break;

			case ALU::DEY_16:
				Y--;
				NZ_Y();
				break;

			case ALU::INC_16:
				alu_temp = (uint8_t)((alu_temp + 1) & 0xFF);
				P = (uint8_t)((P & 0x7D) | TableNZ[alu_temp]);
				break;

			case ALU::DEC_16:
				alu_temp = (uint8_t)((alu_temp - 1) & 0xFF);
				P = (uint8_t)((P & 0x7D) | TableNZ[alu_temp]);
				break;

			case ALU::ASL_16:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)(value8 << 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROL_16:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ROR_16:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::LSR_16:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				P = (uint8_t)((P & 0x7D) | TableNZ[value8]);
				break;

			case ALU::ASLA_16:
				FlagCset((A & 0x80) != 0);
				A = (uint8_t)(A << 1);
				NZ_A();
				break;

			case ALU::ROLA_16:
				temp8 = A;
				A = (uint8_t)((A << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				NZ_A();
				break;

			case ALU::RORA_16:
				temp8 = A;
				A = (uint8_t)((A >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				NZ_A();
				break;

			case ALU::LSRA_16:
				FlagCset((A & 1) != 0);
				A = (uint8_t)(A >> 1);
				NZ_A();
				break;

			case ALU::TXS_16:
				S = X;
				break;

			case ALU::TSX_16:
				X = S;
				NZ_X();
				break;

			case ALU::TAX_16:
				X = A;
				NZ_X();
				break;

			case ALU::TAY_16:
				Y = A;
				NZ_Y();
				break;

			case ALU::TYA_16:
				A = Y;
				NZ_A();
				break;

			case ALU::TXA_16:
				A = X;
				NZ_A();
				break;

			case ALU::SEC_16:
				FlagCset(true);
				break;

			case ALU::CLC_16:
				FlagCset(false);
				break;

			case ALU::SED_16:
				FlagDset(true);
				break;

			case ALU::CLD_16:
				FlagDset(false);
				break;

			case ALU::CLV_16:
				FlagVset(false);
				break;

			case ALU::BIT_16:
				FlagNset((alu_temp & 0x80) != 0);
				FlagVset((alu_temp & 0x40) != 0);
				FlagZset((A & alu_temp) == 0);
				break;

			case ALU::CMP_16:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(A - value8);
				FlagCset(A >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPX_16:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(X - value8);
				FlagCset(X >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::CPY_16:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(Y - value8);
				FlagCset(Y >= value8);
				P = (uint8_t)((P & 0x7D) | TableNZ[(uint8_t)value16]);
				break;

			case ALU::RRA_16:
				value8 = (uint8_t)alu_temp;
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				// pass through to adc

			case ALU::ADC_16:
				//TODO - an extra cycle penalty on 65C02 only
				value8 = (uint8_t)alu_temp;
				if (FlagDget() && BCD_Enabled)
				{
					tempint = (A & 0x0F) + (value8 & 0x0F) + (FlagCget() ? 0x01 : 0x00);
					if (tempint > 0x09)
						tempint += 0x06;
					tempint = (tempint & 0x0F) + (A & 0xF0) + (value8 & 0xF0) + (tempint > 0x0F ? 0x10 : 0x00);
					FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagZset(((A + value8 + (FlagCget() ? 1 : 0)) & 0xFF) == 0);
					FlagNset((tempint & 0x80) != 0);
					if ((tempint & 0x1F0) > 0x090)
						tempint += 0x060;
					FlagCset(tempint > 0xFF);
					A = (uint8_t)(tempint & 0xFF);
				}
				else
				{
					tempint = value8 + A + (FlagCget() ? 1 : 0);
					FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagCset(tempint > 0xFF);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			case ALU::SBC_16:
				value8 = (uint8_t)alu_temp;
				tempint = A - value8 - (FlagCget() ? 0 : 1);
				if (FlagDget() && BCD_Enabled)
				{
					lo = (A & 0x0F) - (value8 & 0x0F) - (FlagCget() ? 0 : 1);
					hi = (A & 0xF0) - (value8 & 0xF0);
					if ((lo & 0xF0) != 0) lo -= 0x06;
					if ((lo & 0x80) != 0) hi -= 0x10;
					if ((hi & 0x0F00) != 0) hi -= 0x60;
					FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagZset((tempint & 0xFF) == 0);
					FlagNset((tempint & 0x80) != 0);
					FlagCset((hi & 0xFF00) == 0);
					A = (uint8_t)((lo & 0x0F) | (hi & 0xF0));
				}
				else
				{
					FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
					FlagCset(tempint >= 0);
					A = (uint8_t)tempint;
					NZ_A();
				}
				break;

			default:
				throw exception("bad op");

		}
	}

	void R5A22::Write_Operation()
	{
		switch (ALU_Type)
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
		}
	}

	void R5A22::Execute(int cycles)
	{
		for (int i = 0; i < cycles; i++)
		{
			ExecuteOne();
		}
	}

	void R5A22::Fetch1()
	{
		Instr_Cycle = -1;
		my_iflag = FlagIget();
		FlagIset(iflag_pending);

		if (NMI)
		{
			if (TraceCallback) TraceCallback(1);
			ea = NMIVector;

			Instr_Type = OpT::INT;
			IRQ_Type = 0; // NMI
			NMI = false;

			Sys_pntr->ReadMemory(address_bus);
			return;
		}

		if (IRQ && !my_iflag)
		{
			if (TraceCallback) TraceCallback(2);
			ea = IRQVector;

			Instr_Type = OpT::INT;
			IRQ_Type = 1; // IRQ

			Sys_pntr->ReadMemory(address_bus);
			return;
		}

		OnExecFetch(PC);
		if (TraceCallback) TraceCallback(0);
		opcode = Sys_pntr->ReadMemory(address_bus);
		PC++;
		Decode(opcode);
	}

	void R5A22::OnExecFetch(uint16_t addr)
	{

	}


	#pragma region Disassembly
	string R5A22::CPUDMAStateOAM()
	{
		val_char_1 = replacer;

		string reg_state = "  OAM ST ADDR: ";

		sprintf_s(val_char_1, 5, "%04X", 1);
		reg_state.append(val_char_1, 4);

		reg_state.append("  OAM ADDR: ");

		sprintf_s(val_char_1, 3, "%02X", 1);
		reg_state.append(val_char_1, 2);

		while (reg_state.length() < 87)
		{
			reg_state.append(" ");
		}

		return reg_state;
	}

	string R5A22::CPUDisassembly()
	{
		string trace_string = "";

		string disasm = Disassemble(PC);

		val_char_1 = replacer;

		sprintf_s(val_char_1, 5, "%04X", PC);
		trace_string.append(val_char_1, 4);
		trace_string.append(":  ");

		uint16_t dis_pc = PC;
		for (uint32_t i = 0; i < op_size; i++)
		{
			sprintf_s(val_char_1, 3, "%02X", Sys_pntr->PeekMemory(dis_pc++));

			trace_string.append(val_char_1, 2);
			trace_string.append(" ");
		}

		while (trace_string.length() < 18)
		{
			trace_string.append(" ");
		}

		trace_string.append(disasm);

		while (trace_string.length() < 38)
		{
			trace_string.append(" ");
		}

		return trace_string;
	}

	string R5A22::CPURegisterState()
	{
		string trace_string = " ";

		val_char_1 = replacer;

		trace_string.append(" A:");
		sprintf_s(val_char_1, 3, "%02X", A);
		trace_string.append(val_char_1, 2);

		trace_string.append(" X:");
		sprintf_s(val_char_1, 3, "%02X", X);
		trace_string.append(val_char_1, 2);

		trace_string.append(" Y:");
		sprintf_s(val_char_1, 3, "%02X", Y);
		trace_string.append(val_char_1, 2);

		trace_string.append(" P:");
		sprintf_s(val_char_1, 3, "%02X", P);
		trace_string.append(val_char_1, 2);

		trace_string.append(" SP:");
		sprintf_s(val_char_1, 3, "%02X", S);
		trace_string.append(val_char_1, 2);

		trace_string.append("  ");
		trace_string.append(FlagNget() ? "N" : "n");
		trace_string.append(FlagVget() ? "V" : "v");
		trace_string.append(FlagMget() ? "M" : "m");
		trace_string.append(FlagXget() ? "X" : "x");
		trace_string.append(FlagVget() ? "D" : "d");
		trace_string.append(FlagIget() ? "I" : "i");
		trace_string.append(FlagZget() ? "Z" : "z");
		trace_string.append(FlagCget() ? "C" : "c");
		trace_string.append(RDY ? "R" : "r");
		trace_string.append("  ");

		trace_string.append("Cy:");
		sprintf_s(val_char_1, 17, "%16lld", TotalExecutedCycles);
		trace_string.append(val_char_1, 16);

		trace_string.append(" LY:");
		sprintf_s(val_char_1, 4, "%3u", Sys_pntr->status_sl);
		trace_string.append(val_char_1, 3);

		trace_string.append(" F-Cy:");
		sprintf_s(val_char_1, 17, "%16lld", Sys_pntr->FrameCycle);
		trace_string.append(val_char_1, 16);

		while (trace_string.length() < 91)
		{
			trace_string.append(" ");
		}

		return trace_string;
	}

	string R5A22::Disassemble(uint16_t pc)
	{
		uint16_t diff = pc;

		uint8_t op = Sys_pntr->PeekMemory(pc++);

		string ret = "";

		val_char_2 = replacer;

		switch (op)
		{
		case 0x00: sprintf_s(val_char_2, 40, "BRK"); break;
		case 0x01: sprintf_s(val_char_2, 40, "ORA ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x04: sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x05: sprintf_s(val_char_2, 40, "ORA $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x06: sprintf_s(val_char_2, 40, "ASL $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x08: sprintf_s(val_char_2, 40, "PHP"); break;
		case 0x09: sprintf_s(val_char_2, 40, "ORA #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x0A: sprintf_s(val_char_2, 40, "ASL A"); break;
		case 0x0C: sprintf_s(val_char_2, 40, "NOP ($%04X)", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x0D: sprintf_s(val_char_2, 40, "ORA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x0E: sprintf_s(val_char_2, 40, "ASL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x10: sprintf_s(val_char_2, 40, "BPL $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0x11: sprintf_s(val_char_2, 40, "ORA ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0x14: sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x15: sprintf_s(val_char_2, 40, "ORA $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x16: sprintf_s(val_char_2, 40, "ASL $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x18: sprintf_s(val_char_2, 40, "CLC"); break;
		case 0x19: sprintf_s(val_char_2, 40, "ORA $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x1A: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0x1C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x1D: sprintf_s(val_char_2, 40, "ORA $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x1E: sprintf_s(val_char_2, 40, "ASL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x20: sprintf_s(val_char_2, 40, "JSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x21: sprintf_s(val_char_2, 40, "AND ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x24: sprintf_s(val_char_2, 40, "BIT $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x25: sprintf_s(val_char_2, 40, "AND $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x26: sprintf_s(val_char_2, 40, "ROL $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x28: sprintf_s(val_char_2, 40, "PLP"); break;
		case 0x29: sprintf_s(val_char_2, 40, "AND #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x2A: sprintf_s(val_char_2, 40, "ROL A"); break;
		case 0x2C: sprintf_s(val_char_2, 40, "BIT $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x2D: sprintf_s(val_char_2, 40, "AND $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x2E: sprintf_s(val_char_2, 40, "ROL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x30: sprintf_s(val_char_2, 40, "BMI $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0x31: sprintf_s(val_char_2, 40, "AND ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0x34: sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x35: sprintf_s(val_char_2, 40, "AND $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x36: sprintf_s(val_char_2, 40, "ROL $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x38: sprintf_s(val_char_2, 40, "SEC"); break;
		case 0x39: sprintf_s(val_char_2, 40, "AND $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x3A: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0x3C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x3D: sprintf_s(val_char_2, 40, "AND $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x3E: sprintf_s(val_char_2, 40, "ROL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x40: sprintf_s(val_char_2, 40, "RTI"); break;
		case 0x41: sprintf_s(val_char_2, 40, "EOR ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x44: sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x45: sprintf_s(val_char_2, 40, "EOR $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x46: sprintf_s(val_char_2, 40, "LSR $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x48: sprintf_s(val_char_2, 40, "PHA"); break;
		case 0x49: sprintf_s(val_char_2, 40, "EOR #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x4A: sprintf_s(val_char_2, 40, "LSR A"); break;
		case 0x4C: sprintf_s(val_char_2, 40, "JMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x4D: sprintf_s(val_char_2, 40, "EOR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x4E: sprintf_s(val_char_2, 40, "LSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x50: sprintf_s(val_char_2, 40, "BVC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0x51: sprintf_s(val_char_2, 40, "EOR ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0x54: sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x55: sprintf_s(val_char_2, 40, "EOR $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x56: sprintf_s(val_char_2, 40, "LSR $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x58: sprintf_s(val_char_2, 40, "CLI"); break;
		case 0x59: sprintf_s(val_char_2, 40, "EOR $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x5A: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0x5C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x5D: sprintf_s(val_char_2, 40, "EOR $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x5E: sprintf_s(val_char_2, 40, "LSR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x60: sprintf_s(val_char_2, 40, "RTS"); break;
		case 0x61: sprintf_s(val_char_2, 40, "ADC ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x64: sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x65: sprintf_s(val_char_2, 40, "ADC $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x66: sprintf_s(val_char_2, 40, "ROR $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x68: sprintf_s(val_char_2, 40, "PLA"); break;
		case 0x69: sprintf_s(val_char_2, 40, "ADC #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x6A: sprintf_s(val_char_2, 40, "ROR A"); break;
		case 0x6C: sprintf_s(val_char_2, 40, "JMP ($%04X)", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x6D: sprintf_s(val_char_2, 40, "ADC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x6E: sprintf_s(val_char_2, 40, "ROR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x70: sprintf_s(val_char_2, 40, "BVS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0x71: sprintf_s(val_char_2, 40, "ADC ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0x74: sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x75: sprintf_s(val_char_2, 40, "ADC $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x76: sprintf_s(val_char_2, 40, "ROR $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x78: sprintf_s(val_char_2, 40, "SEI"); break;
		case 0x79: sprintf_s(val_char_2, 40, "ADC $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x7A: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0x7C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x7D: sprintf_s(val_char_2, 40, "ADC $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x7E: sprintf_s(val_char_2, 40, "ROR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x80: sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x81: sprintf_s(val_char_2, 40, "STA ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0x82: sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x84: sprintf_s(val_char_2, 40, "STY $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x85: sprintf_s(val_char_2, 40, "STA $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x86: sprintf_s(val_char_2, 40, "STX $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x88: sprintf_s(val_char_2, 40, "DEY"); break;
		case 0x89: sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x8A: sprintf_s(val_char_2, 40, "TXA"); break;
		case 0x8C: sprintf_s(val_char_2, 40, "STY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x8D: sprintf_s(val_char_2, 40, "STA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x8E: sprintf_s(val_char_2, 40, "STX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x90: sprintf_s(val_char_2, 40, "BCC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0x91: sprintf_s(val_char_2, 40, "STA ($%02X),Y", Sys_pntr->PeekMemory(pc++)); break;
		case 0x94: sprintf_s(val_char_2, 40, "STY $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x95: sprintf_s(val_char_2, 40, "STA $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0x96: sprintf_s(val_char_2, 40, "STX $%02X,Y", Sys_pntr->PeekMemory(pc++)); break;
		case 0x98: sprintf_s(val_char_2, 40, "TYA"); break;
		case 0x99: sprintf_s(val_char_2, 40, "STA $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0x9A: sprintf_s(val_char_2, 40, "TXS"); break;
		case 0x9D: sprintf_s(val_char_2, 40, "STA $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xA0: sprintf_s(val_char_2, 40, "LDY #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xA1: sprintf_s(val_char_2, 40, "LDA ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0xA2: sprintf_s(val_char_2, 40, "LDX #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xA4: sprintf_s(val_char_2, 40, "LDY $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xA5: sprintf_s(val_char_2, 40, "LDA $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xA6: sprintf_s(val_char_2, 40, "LDX $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xA8: sprintf_s(val_char_2, 40, "TAY"); break;
		case 0xA9: sprintf_s(val_char_2, 40, "LDA #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xAA: sprintf_s(val_char_2, 40, "TAX"); break;
		case 0xAC: sprintf_s(val_char_2, 40, "LDY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xAD: sprintf_s(val_char_2, 40, "LDA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xAE: sprintf_s(val_char_2, 40, "LDX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xB0: sprintf_s(val_char_2, 40, "BCS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0xB1: sprintf_s(val_char_2, 40, "LDA ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0xB3: sprintf_s(val_char_2, 40, "LAX ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0xB4: sprintf_s(val_char_2, 40, "LDY $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xB5: sprintf_s(val_char_2, 40, "LDA $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xB6: sprintf_s(val_char_2, 40, "LDX $%02X,Y", Sys_pntr->PeekMemory(pc++)); break;
		case 0xB8: sprintf_s(val_char_2, 40, "CLV"); break;
		case 0xB9: sprintf_s(val_char_2, 40, "LDA $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xBA: sprintf_s(val_char_2, 40, "TXS"); break;
		case 0xBC: sprintf_s(val_char_2, 40, "LDY $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xBD: sprintf_s(val_char_2, 40, "LDA $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xBE: sprintf_s(val_char_2, 40, "LDX $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xC0: sprintf_s(val_char_2, 40, "CPY #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xC1: sprintf_s(val_char_2, 40, "CMP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0xC2: sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xC4: sprintf_s(val_char_2, 40, "CPY $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xC5: sprintf_s(val_char_2, 40, "CMP $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xC6: sprintf_s(val_char_2, 40, "DEC $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xC8: sprintf_s(val_char_2, 40, "INY"); break;
		case 0xC9: sprintf_s(val_char_2, 40, "CMP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xCA: sprintf_s(val_char_2, 40, "DEX"); break;
		case 0xCB: sprintf_s(val_char_2, 40, "AXS $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xCC: sprintf_s(val_char_2, 40, "CPY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xCD: sprintf_s(val_char_2, 40, "CMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xCE: sprintf_s(val_char_2, 40, "DEC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xD0: sprintf_s(val_char_2, 40, "BNE $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0xD1: sprintf_s(val_char_2, 40, "CMP ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0xD4: sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xD5: sprintf_s(val_char_2, 40, "CMP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xD6: sprintf_s(val_char_2, 40, "DEC $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xD8: sprintf_s(val_char_2, 40, "CLD"); break;
		case 0xD9: sprintf_s(val_char_2, 40, "CMP $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xDA: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0xDC: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0xDD: sprintf_s(val_char_2, 40, "CMP $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xDE: sprintf_s(val_char_2, 40, "DEC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xE0: sprintf_s(val_char_2, 40, "CPX #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xE1: sprintf_s(val_char_2, 40, "SBC ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0xE2: sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xE4: sprintf_s(val_char_2, 40, "CPX $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xE5: sprintf_s(val_char_2, 40, "SBC $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xE6: sprintf_s(val_char_2, 40, "INC $%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xE8: sprintf_s(val_char_2, 40, "INX"); break;
		case 0xE9: sprintf_s(val_char_2, 40, "SBC #$%02X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xEA: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0xEC: sprintf_s(val_char_2, 40, "CPX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xED: sprintf_s(val_char_2, 40, "SBC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xEE: sprintf_s(val_char_2, 40, "INC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xF0: sprintf_s(val_char_2, 40, "BEQ $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
		case 0xF1: sprintf_s(val_char_2, 40, "SBC ($%02X),Y *", Sys_pntr->PeekMemory(pc++)); break;
		case 0xF4: sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xF5: sprintf_s(val_char_2, 40, "SBC $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xF6: sprintf_s(val_char_2, 40, "INC $%02X,X", Sys_pntr->PeekMemory(pc++)); break;
		case 0xF8: sprintf_s(val_char_2, 40, "SED"); break;
		case 0xF9: sprintf_s(val_char_2, 40, "SBC $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xFA: sprintf_s(val_char_2, 40, "NOP"); break;
		case 0xFC: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++)); break;
		case 0xFD: sprintf_s(val_char_2, 40, "SBC $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		case 0xFE: sprintf_s(val_char_2, 40, "INC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
		default: sprintf_s(val_char_2, 40, "???"); break;
		}

		if (pc > diff)
		{
			op_size = pc - diff;
		}
		else
		{
			uint32_t h_diff = (uint32_t)pc + 0x10000;

			op_size = h_diff - diff;
		}

		return std::string(val_char_2, 20);
	}

	#pragma endregion
}