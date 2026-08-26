#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "SNES_System.h"
#include "APU.h"

namespace SNESHawk
{
	void APU::Decode_Next_Cycle()
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

			case OpT::Imp3:
				switch (Instr_Cycle)
				{
				case 0:
					address_bus = get_PC_Addr();
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 1:
					// same address as above
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 2:
					// same address as above
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
						break;

					case 2:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::ImmD:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						PC++;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						PC++;
						break;

					case 2:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						// same address as above
						value8 = opcode3;
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
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
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						get_Push_value();
						break;

					case 2:
						// same address as above
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
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

			case OpT::JMPI:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC >> 8;
						break;

					case 2:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC;
						break;

					case 3:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 4:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						address_bus = (((((uint32_t)opcode3 << 8) | opcode2) + X) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 6:
						address_bus = (((((uint32_t)opcode3 << 8) | opcode2) + X + 1) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 7:
						PC = (alu_temp_hi << 8) | alu_temp;
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
						break;

					case 2:
						PC = opcode2 | ((uint32_t)opcode3 << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::TCALL:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = 0xFF00 | (0xDE - (opcode >> 5));
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						// address bus same as above
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 2:
						address_bus++;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						// same address bus as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC >> 8;
						break;

					case 6:
						// same address bus as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 7:
						PC = opcode2 | ((uint32_t)opcode3 << 8);
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
						// update P here
						P = opcode2;
						break;

					case 5:
						PC = alu_temp | ((uint32_t)alu_temp_hi << 8);
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::RTS:
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
						break;

					case 3:
						Inc_S();
						address_bus = S;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						address_bus = S;
						break;

					case 5:
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
						alu_temp = ((((uint32_t)PC & 0xFF) + (int8_t)opcode2)) & 0x1FF;
						PC += alu_temp;
						break;

					case 2:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::BrB:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						PC++;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 2:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						temp8 = temp8 = (opcode >> 1) & 7;
						
						if ((opcode & 1) == 0)
						{
							branch_taken = (((alu_temp >> temp8) & 1) == 1);
						}
						else
						{
							branch_taken = (((alu_temp >> temp8) & 1) == 0);
						}

						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
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

					case 4:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						alu_temp = ((((uint32_t)PC & 0xFF) + (int8_t)opcode3)) & 0x1FF;
						PC += alu_temp;
						break;

					case 5:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 6:
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
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 2:
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
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						// same address bus value as above
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 3:
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
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 2:
						// same address bus as above
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 3:
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
						break;

					case 1:
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

					case 2:
						address_bus = (D + Index_Add) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
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
						break;

					case 1:
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

					case 2:
						address_bus = (D + Index_Add) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						// same address bus value as above
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
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
						break;

					case 1:
						address_bus = get_PC_Addr();
						Index_Add = X + opcode2;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						address_bus = (D + Index_Add) & 0xFFFF;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
						// same address bus as above
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
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
						address_bus = (uint32_t)(opcode3 << 8) | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
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
						address_bus = (uint32_t)(opcode3 << 8) | opcode2;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						// same address bus as above
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
						address_bus = (uint32_t)(opcode3 << 8) | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
						// same address bus as above
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
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
						break;

					case 1:
						address_bus = (D + (Index_Add & 0xFF));
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						// same address as above
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						address_bus = (D + ((Index_Add + 1) & 0xFF));
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						address_bus = (uint32_t)(opcode4 << 8) | opcode3;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 5:
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
						break;

					case 1:
						address_bus = (D + (Index_Add & 0xFF));
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 2:
						// same address as above
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 3:
						address_bus = (D + ((Index_Add + 1) & 0xFF));
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 4:
						address_bus = (uint32_t)(opcode4 << 8) | opcode3;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						// same address as above
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 6:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
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
						break;

					case 2:
						address_bus = ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF); // ??
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						address_bus = ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 4:
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
						address_bus = ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF); // ??
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						address_bus = ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 4:
						// same address bus as above
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
						Index_Add = X;
						break;

					case 2:
						address_bus = ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF); // ??
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						address_bus = ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 4:
						// same address bus as above
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 5:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::IXR:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = D | X;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 2:
						Cycle_Type = CPU_Cycle_Type::Fetch_ALU_Cycle;
						break;
				}
				break;

			case OpT::IXW:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = D | X;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
						// same address bus as above
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
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
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						address_bus = D |( (opcode2 + 1) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 3:
						address_bus = ((uint32_t)(opcode4 << 8)) | ((opcode3 + (Y & 0xFF)) & 0xFF); // ?? 
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 4:
						address_bus = ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 5:
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
						break;

					case 1:
						address_bus = D | opcode2;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 2:
						address_bus = D | ((opcode2 + 1) & 0xFF);
						Cycle_Type = CPU_Cycle_Type::Fetch_4;
						break;

					case 3:
						address_bus = ((uint32_t)(opcode4 << 8)) | ((opcode3 + (Y & 0xFF)) & 0xFF); // ?? 
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 4:
						address_bus = ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						// same address bus as above
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 6:
						Cycle_Type = CPU_Cycle_Type::Fetch_Cycle;
						break;
				}
				break;

			case OpT::STP:
				// fall through to JAM because nothing can unstop from here
			case OpT::Jam:
				// do nothing, stuck
				Instr_Cycle--;
				break;

			case OpT::RESET:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 1:
						value8 = PC >> 8;
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 2:
						value8 = PC;
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
						value8 = PC;
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 4:
						FlagIset(true);
						FlagDset(false);
						ea = ResetVector;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 5:
						address_bus = (uint16_t)(ea + 1);
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 6:
						PC = opcode2 | ((uint32_t)opcode3 << 8);
						Fetch_Opcode_No_Interrupt();
						break;
				}
				break;

			case OpT::BRK:
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
						break;

					case 2:
						value8 = PC >> 8;
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 3:
						value8 = PC;
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						value8 = P;
						address_bus = S;
						Dec_S();

						Cycle_Type = CPU_Cycle_Type::Write_Cycle;

						ea = BRKVector;
						value8 = P | 0x40;
						FlagBset(true);
						break;

					case 5:
						FlagDset(false);
						FlagIset(true);

						// IRQ hijacking?

						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_2;
						break;

					case 6:
						address_bus = (uint16_t)(ea + 1);

						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Fetch_3;
						break;

					case 7:
						PC = opcode2 | ((uint32_t)opcode3 << 8);
						Fetch_Opcode_No_Interrupt();
						break;
				}
				break;
		}
	}

	void APU::RunCpuOne()
	{
		Fetch_Cnt++;

		if (Fetch_Cnt == Fetch_Op)
		{
			switch (Cycle_Type)
			{
				case CPU_Cycle_Type::Read_Cycle:
					alu_temp = ReadMemory(address_bus);
					break;

				case CPU_Cycle_Type::Read_Cycle_Hi:
					alu_temp_hi = ReadMemory(address_bus);
					alu_temp_16 = (alu_temp_hi << 8) | alu_temp;
					break;

				case CPU_Cycle_Type::Write_Cycle:
					WriteMemory(address_bus, value8);
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
					opcode2 = ReadMemory(address_bus);
					break;

				case CPU_Cycle_Type::Fetch_3:
					opcode3 = ReadMemory(address_bus);
					break;

				case CPU_Cycle_Type::Internal_Cycle:

					break;

				case CPU_Cycle_Type::Fetch_Reset:
					// do nothing as this is just a cycle that immediately goes into the interrupt handler with a reset
					break;
			}
		}
		else if (Fetch_Cnt == Fetch_Wait)
		{
			Instr_Cycle++;

			Fetch_Cnt = 0;

			//Core_Message_String->assign("test: " + to_string(Instr_Cycle) + " cyc: " + to_string(TotalExecutedCycles));

			//MessageCallback(Core_Message_String->length());

			Decode_Next_Cycle();

			Calculate_Wait_States();
		}

		TotalExecutedCycles++;
	}

	void APU::ALU_Operation()
	{
		switch (ALU_Type)
		{
			case ALU::NOP:
				break;

			case ALU::CLP:
				FlagPset(false);
				D = 0;
				break;

			case ALU::SEP:
				FlagPset(true);
				D = 0x100;
				break;

			case ALU::ORA:
				A |= (uint8_t)alu_temp;
				NZ_Set(A);
				break;

			case ALU::AND:
				A &= (uint8_t)alu_temp;
				NZ_Set(A);
				break;

			case ALU::EOR:
				A ^= (uint8_t)alu_temp;
				A &= 0xFF;
				NZ_Set(A);
				break;

			case ALU::LDA:
				A = (uint8_t)alu_temp;
				NZ_Set(A);
				break;

			case ALU::LDX:
				X = (uint8_t)alu_temp;
				NZ_Set(X);
				break;

			case ALU::LDY:
				Y = (uint8_t)alu_temp;
				NZ_Set(Y);
				break;

			case ALU::ASR:
				A &= (uint8_t)alu_temp;
				FlagCset((A & 0x1) == 0x1);
				A >>= 1;
				NZ_Set(A);
				break;

			case ALU::INX:
				X++;
				X &= 0xFF;
				NZ_Set(X);
				break;

			case ALU::INY:
				Y++;
				Y &= 0xFF;
				NZ_Set(Y);
				break;

			case ALU::DEX:
				X--;
				X &= 0xFF;
				NZ_Set(X);
				break;

			case ALU::DEY:
				Y--;
				Y &= 0xFF;
				NZ_Set(Y);
				break;

			case ALU::INC:
				alu_temp = (uint8_t)((alu_temp + 1) & 0xFF);
				NZ_Set(alu_temp);
				break;

			case ALU::DEC:
				alu_temp = (uint8_t)((alu_temp - 1) & 0xFF);
				NZ_Set(alu_temp);
				break;

			case ALU::ASL:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)(value8 << 1);
				NZ_Set(value8);
				break;

			case ALU::ROL:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				NZ_Set(value8);
				break;

			case ALU::ROR:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				NZ_Set(value8);
				break;

			case ALU::LSR:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				NZ_Set(value8);
				break;

			case ALU::ASLA:
				FlagCset((A & 0x80) != 0);
				A = (uint8_t)(A << 1);
				NZ_Set(A);
				break;

			case ALU::ROLA:
				temp8 = A;
				A = (uint8_t)((A << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				NZ_Set(A);
				break;

			case ALU::RORA:
				temp8 = A;
				A = (uint8_t)((A >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				NZ_Set(A);
				break;

			case ALU::LSRA:
				FlagCset((A & 1) != 0);
				A = (uint8_t)(A >> 1);
				NZ_Set(A);
				break;

			case ALU::INCA:
				A = (uint8_t)((A + 1) & 0xFF);
				NZ_Set(A);
				break;

			case ALU::DECA:
				A = (uint8_t)((A - 1) & 0xFF);
				NZ_Set(A);
				break;

			case ALU::TXS:
				S = A;
				break;

			case ALU::TSX:
				X = S;
				X &= 0xFF;
				NZ_Set(X);
				break;

			case ALU::TAX:
				X = A;
				X &= 0xFF;
				NZ_Set(X);
				break;

			case ALU::TAY:
				Y = A;
				Y &= 0xFF;
				NZ_Set(Y);
				break;

			case ALU::TYA:
				A = Y;
				A &= 0xFF;
				NZ_Set(A);
				break;

			case ALU::TXA:
				A = X;
				A &= 0xFF;
				NZ_Set(A);
				break;

			case ALU::SEC:
				FlagCset(true);
				break;

			case ALU::CLC:
				FlagCset(false);
				break;

			case ALU::NOTC:
				FlagCset(!FlagCget());
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

			case ALU::SETB:
				temp8 = (opcode >> 1) & 7;
				alu_temp |= (1 << temp8);
				break;

			case ALU::CLRB:
				temp8 = (opcode >> 1) & 7;
				alu_temp &= (0xFF - (1 << temp8));
				break;

			case ALU::CMP:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(A - value8);
				FlagCset(A >= value8);
				NZ_Set(value16);
				break;

			case ALU::CPX:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(X - value8);
				FlagCset(X >= value8);
				NZ_Set(value16);
				break;

			case ALU::CPY:
				value8 = (uint8_t)alu_temp;
				value16 = (uint16_t)(Y - value8);
				FlagCset(Y >= value8);
				NZ_Set(value16);
				break;

			case ALU::ADC:
				value8 = (uint8_t)alu_temp;

				tempint = value8 + A + (FlagCget() ? 1 : 0);
				FlagVset((~(A ^ value8) & (A ^ tempint) & 0x80) != 0);
				FlagCset(tempint > 0xFF);
				A = (uint8_t)tempint;
				NZ_Set(A);

				break;

			case ALU::SBC:
				value8 = (uint8_t)alu_temp;
				tempint = A - value8 - (FlagCget() ? 0 : 1);

				FlagVset(((A ^ value8) & (A ^ tempint) & 0x80) != 0);
				FlagCset(tempint >= 0);
				A = (uint8_t)tempint;
				NZ_Set(A);

				break;

			case ALU::PLA:
				A = alu_temp;
				NZ_Set(A);
				break;

			case ALU::PLP:
				my_iflag = FlagIget();
				P = alu_temp;
				iflag_pending = FlagIget();
				FlagIset(my_iflag);
				break;

			case ALU::PLX:
				X = alu_temp;
				NZ_Set(X);
				break;

			case ALU::PLY:
				Y = alu_temp;
				NZ_Set(Y);
				break;

			// for direct page ops the direct page value starts in value8
			case ALU::ORv:
				value8 |= (uint8_t)alu_temp;
				NZ_Set(value8);
				break;

			case ALU::ANDv:
				value8 &= (uint8_t)alu_temp;
				NZ_Set(value8);
				break;

			case ALU::EORv:
				value8 ^= (uint8_t)alu_temp;
				value8 &= 0xFF;
				NZ_Set(value8);
				break;

			case ALU::CMPv:
				value16 = (uint16_t)(value8 - alu_temp);
				FlagCset(value8 >= alu_temp);
				NZ_Set(value16);
				break;

			case ALU::ADCv:
				tempint = alu_temp + value8 + (FlagCget() ? 1 : 0);
				FlagVset((~(value8 ^ alu_temp) & (value8 ^ tempint) & 0x80) != 0);
				FlagCset(tempint > 0xFF);
				value8 = (uint8_t)tempint;
				NZ_Set(value8);
				break;

			case ALU::SBCv:
				tempint = value8 - alu_temp - (FlagCget() ? 0 : 1);
				FlagVset(((value8 ^ alu_temp) & (value8 ^ tempint) & 0x80) != 0);
				FlagCset(tempint >= 0);
				value8 = (uint8_t)tempint;
				NZ_Set(value8);
				break;

			case ALU::LDv:
				value8 = (uint8_t)alu_temp;
				NZ_Set(value8);
				break;

			default:
				throw exception("bad op");

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
		Instr_Cycle = -1;
		my_iflag = FlagIget();
		FlagIset(iflag_pending);

		address_bus = get_PC_Addr();

		OnExecFetch(PC);
		if (TraceCallback) TraceCallback(0);
		opcode = ReadMemory(address_bus);
		PC++;
		Decode(opcode);
	}

	void APU::OnExecFetch(uint16_t addr)
	{

	}

	void APU::Fetch_Opcode_No_Interrupt()
	{
		Instr_Cycle = -1;

		address_bus = get_PC_Addr();

		OnExecFetch(PC);
		if (TraceCallback) TraceCallback(0);
		opcode = ReadMemory(address_bus);
		PC++;
		Decode(opcode);
	}
}