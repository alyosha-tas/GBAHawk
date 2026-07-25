#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "R5A22.h"
#include "SNES_System.h"

namespace SNESHawk
{
	void R5A22::Decode_Next_Cycle()
	{
		switch (Instr_Type)
		{
			case OpT::Imp:
			case OpT::Acc:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;										
						break;

					case 1:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;
		
			case OpT::Imm:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						PC++;

						Instr_Cycle += Instr_Skip;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						PC++;
						break;

					case 2:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::CSI:
				switch (Instr_Cycle)
				{
					case 0:
						iflag_pending = ALU_Type != ALU::CLI;
						
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle_No_Check;
						break;
				}
				break;

			case OpT::PH:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;

						Push_Shift = 8;
						get_Push_value();

						Instr_Cycle += Instr_Skip;
						break;

					case 2:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;

						Push_Shift = 0;
						get_Push_value();
						break;

					case 3:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::PL:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:						
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;

						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						if (ALU_Type == ALU::PLP)
						{
							ALU_Operation();
							Cycle_Type = CPU_Cycle_Type::Fetch_Cycle_No_Check;
						}
						else
						{
							Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						}
						break;
				}
				break;

			case OpT::JSR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC >> 8;
						break;

					case 4:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC;
						break;

					case 5:
						PC = opcode2 | ((uint32_t)opcode3 << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;

				}
				break;
			
			case OpT::JMP:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						Instr_Cycle += Instr_Skip;
						// if not a long jump, we will just re-assign PBR with the same value again
						opcode4 = PBR;
						break;

					case 2:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 3:
						PBR = opcode4;
						PC = opcode2 | ((uint32_t)opcode3 << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::JMPI:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						ea = (opcode3 << 8) + opcode2;
						address_bus = ea;
						ea = (ea + 1) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
						address_bus = ea;
						ea = (ea + 1) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						Instr_Cycle += Instr_Skip;
						// if not a long jump, we will just re-assign PBR with the same value again
						opcode4 = PBR;
						break;

					case 4:
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 5:
						PBR = opcode4;
						PC = alu_temp | ((uint32_t)alu_temp_hi << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::RTI:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;
					
					case 3:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 4:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						// don't pull PBR in emulation mode
						if (Flag_E) { Instr_Cycle += 1; }
						// update P here
						P = opcode2;
						// if we don't update PBR we just copy the same value
						opcode4 = PBR;
						break;

					case 5:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 6:
						PBR = opcode4;
						PC = alu_temp | ((uint32_t)alu_temp_hi << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::RTS:
			case OpT::RTL:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 4:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						// don't pull PBR in emulation mode
						if (Flag_E) { Instr_Cycle += 1; }

						// if we don't update PBR we just copy the same value
						opcode4 = PBR;
						break;

					case 5:
						if (Instr_Type == OpT::RTL)
						{
							Inc_S();
							Cycle_Type = CPU_Cycle_Type::Fetch_4;
						}
						else
						{
							Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						}
						address_bus = S;		
						break;

					case 6:
						PBR = opcode4;
						PC = alu_temp | ((uint32_t)alu_temp_hi << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
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

						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						PC++;

						if (branch_taken)
						{
							branch_taken = false;
						}
						else
						{
							Instr_Cycle += 2;
						}
						break;

					case 1:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						alu_temp = ((uint32_t)(PC & 0xFF) + (int8_t)opcode2);

						if (!Flag_E)
						{
							Instr_Cycle++;
						}
						else if (!((alu_temp & 0x100) == 0x100))
						{
							Instr_Cycle++;
						}
						break;

					case 2:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						PC &= 0xFF00;
						PC |= (uint16_t)(alu_temp & 0xFF);
						if (((alu_temp & 0x80000000) == 0x80000000))
							PC = (uint16_t)(PC - 0x100);
						else PC = (uint16_t)(PC + 0x100);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::Brl:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						alu_temp = PC;
						PC++;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						PC++;
						break;

					case 2:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						PC = alu_temp + (opcode2 + (opcode3 << 8));
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;
		
			case OpT::DPR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						ea = (D + opcode2) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						Index_Add = 0;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DPW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						ea = (D + opcode2) & 0xFFFF;
						address_bus = ea;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Push_Shift = 8;

						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DPRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0)
						{
							Instr_Cycle++;
						}
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						ea = (D + opcode2) & 0xFFFF;
						Index_Add = opcode2;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						get_Direct_Addr_Inc();
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 5:
						// same address bus value as above
						value8 = value16 >> 8;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;
					case 6:
						address_bus = (D + opcode2) & 0xFFFF;
						value8 = value16 & 0xFF;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DPXR:
			case OpT::DPYR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = get_PC_Addr();
						if (Instr_Type == OpT::DPXR)
						{
							Index_Add = X + opcode2;
						}
						else
						{
							Index_Add = Y + opcode2;
						}

						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 5:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DPXW:
			case OpT::DPYW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = get_PC_Addr();
						if (Instr_Type == OpT::DPXW)
						{
							Index_Add = X + opcode2;
						}
						else
						{
							Index_Add = Y + opcode2;
						}

						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 5:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DPXRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0)
						{
							Instr_Cycle++;
						}
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = get_PC_Addr();
						if ((Instr_Type == OpT::DPXR) || (Instr_Type == OpT::DPXW))
						{
							Index_Add = X + opcode2;
						}
						else
						{
							Index_Add = Y + opcode2;
						}

						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 5:
						get_Direct_Addr_Inc();
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						// same address bus value as above
						value8 = value16 >> 8;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;
					case 7:
						address_bus = (D + opcode2 + Index_Add) & 0xFFFF;
						value8 = value16 & 0xFF;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 8:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::AbsR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						address_bus = ((uint32_t)DBR << 16) | (uint32_t)(opcode3 << 8) | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::AbsW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						address_bus = ((uint32_t)DBR << 16) | (uint32_t)(opcode3 << 8) | opcode2;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						address_bus++;
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::AbsRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						ea = ((uint32_t)DBR << 16) | (uint32_t)(opcode3 << 8) | opcode2;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						address_bus = ea + 1;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						address_bus = ea + 1;
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 5:
						// same address bus value as above
						value8 = value16 >> 8;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;
					case 6:
						address_bus = ((uint32_t)DBR << 16) | (uint32_t)(opcode3 << 8) | opcode2;
						value8 = value16 & 0xFF;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::AbsLR:
			case OpT::ALXR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_4;

						if (Instr_Type == OpT::AbsLR)
						{
							Index_Add = 0;
						}
						else
						{
							Index_Add = X;
						}
						break;

					case 3:
						address_bus = ((uint32_t)opcode4 << 16) | ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 5:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::AbsLW:
			case OpT::ALXW:
				switch (Instr_Cycle)
				{
				case 0:
					address_bus = get_PC_Addr();
					PC++;
					Cycle_Type = CPU_Cycle_Type::Fetch_2;
					break;

				case 1:
					address_bus = get_PC_Addr();
					PC++;
					Cycle_Type = CPU_Cycle_Type::Fetch_3;
					break;

				case 2:
					address_bus = get_PC_Addr();
					PC++;
					Cycle_Type = CPU_Cycle_Type::Fetch_4;

					if (Instr_Type == OpT::AbsLW)
					{
						Index_Add = 0;
					}
					else
					{
						Index_Add = X;
					}
					break;

				case 3:
					address_bus = ((uint32_t)opcode4 << 16) | ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
					Push_Shift = 0;
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					Instr_Cycle += Instr_Skip;
					break;

				case 4:
					address_bus++;
					Push_Shift = 8;
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 5:
					Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
					break;
				}
				break;

			case OpT::DIXR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = get_PC_Addr();
						Index_Add = X + opcode2;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 5:
						ea = ((uint32_t)DBR << 16) | (uint32_t)(opcode4 << 8) | opcode3;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DIXW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = get_PC_Addr();
						Index_Add = X + opcode2;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 5:
						ea = ((uint32_t)DBR << 16) | (uint32_t)(opcode4 << 8) | opcode3;
						address_bus = ea;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus++;
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DIR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = opcode2;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						ea = ((uint32_t)DBR << 16) | (uint32_t)(opcode4 << 8) | opcode3;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 5:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 6:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DIW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = opcode2;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						ea = ((uint32_t)DBR << 16) | (uint32_t)(opcode4 << 8) | opcode3;
						address_bus = ea;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 5:
						address_bus++;
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 6:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DLR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = opcode2;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 5:						
						address_bus = (alu_temp << 16) | (uint32_t)(opcode4 << 8) | opcode3;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DLW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;

						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = opcode2;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 5:
						address_bus = (alu_temp << 16) | (uint32_t)(opcode4 << 8) | opcode3;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus++;
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DSR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = (S + opcode2) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						address_bus = (address_bus + 1) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DSW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = (S + opcode2) & 0xFFFF;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 3:
						address_bus = (address_bus + 1) & 0xFFFF;
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DSIR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = (S + opcode2) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						address_bus = (address_bus + 1) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						// address bus same as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						ea = ((uint32_t)DBR << 16) | ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus = ((uint32_t)DBR << 16) | ((address_bus + 1) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DSIW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = (S + opcode2) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						address_bus = (address_bus + 1) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						// address bus same as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						ea = ((uint32_t)DBR << 16) | ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						address_bus = ea;
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus = ((uint32_t)DBR << 16) | ((address_bus + 1) & 0xFFFF);
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::AIXR:
			case OpT::AIYR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;

						if (Instr_Type == OpT::AIXR)
						{
							Index_Add = X;
						}
						else
						{
							Index_Add = Y;
						}

						// determine if cycle is skipped
						if (FlagXget())
						{
							if (((uint32_t)opcode2 + (Index_Add & 0xFF)) < 0x100)
							{
								Instr_Cycle++;
							}
						}
						break;

					case 2:
						address_bus = (uint32_t)(DBR << 16) | ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						address_bus = (uint32_t)(DBR << 16) | ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						address_bus = (uint32_t)(DBR << 16) | ((address_bus + 1) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 5:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::AIXW:
			case OpT::AIYW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;

						if (Instr_Type == OpT::AIXW)
						{
							Index_Add = X;
						}
						else
						{
							Index_Add = Y;
						}
						break;

					case 2:
						address_bus = (uint32_t)(DBR << 16) | ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						address_bus = (uint32_t)(DBR << 16) | ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						address_bus = (uint32_t)(DBR << 16) | ((address_bus + 1) & 0xFFFF);
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 5:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::AIXRW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						address_bus = (uint32_t)(DBR << 16) | ((uint32_t)(opcode3 << 8)) | ((opcode2 + (X & 0xFF)) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						ea = (uint32_t)(DBR << 16) | ((((uint32_t)(opcode3 << 8) | opcode2) + X) & 0xFFFF);
						address_bus = ea;

						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 4:
						address_bus = (uint32_t)(DBR << 16) | ((ea + 1) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 5:
						address_bus = (uint32_t)(DBR << 16) | ((ea + 1) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						ALU_Operation();
						break;

					case 6:
						address_bus = (uint32_t)(DBR << 16) | ((ea + 1) & 0xFFFF);

						Push_Shift = 8;

						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;

						Instr_Cycle += Instr_Skip;
						break;

					case 7:
						address_bus = (uint32_t)(DBR << 16) | (ea & 0xFFFF);

						Push_Shift = 0;

						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 8:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DIIYR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = 0;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;

						// determine if cycle is skipped
						if (FlagXget())
						{
							if (((uint32_t)opcode3 + (Y & 0xFF)) < 0x100)
							{
								Instr_Cycle++;
							}
						}
						break;

					case 4:
						address_bus = (uint32_t)(DBR << 16) | ((uint32_t)(opcode4 << 8)) | ((opcode3 + (Y & 0xFF)) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						address_bus = (uint32_t)(DBR << 16) | ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus = (uint32_t)(DBR << 16) | ((address_bus + 1) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DIIYW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = 0;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						address_bus = (uint32_t)(DBR << 16) | ((uint32_t)(opcode4 << 8)) | ((opcode3 + (Y & 0xFF)) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						address_bus = (uint32_t)(DBR << 16) | ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus = (uint32_t)(DBR << 16) | ((address_bus + 1) & 0xFFFF);
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::DLIYR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = 0;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 5:
						address_bus = (alu_temp << 16) | ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::DLIYW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						// extra cycle if DL not zero
						if ((D & 0xFF) == 0) { Instr_Cycle++; }
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						Index_Add = 0;
						ea = (D + Index_Add) & 0xFFFF;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 5:
						address_bus = (alu_temp << 16) | ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						Push_Shift = 0;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						Instr_Cycle += Instr_Skip;
						break;

					case 6:
						address_bus++;
						Push_Shift = 8;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
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
}