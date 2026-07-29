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

			case OpT::Imm:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						PC++;
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

			case OpT::Imm3:
				switch (Instr_Cycle)
				{
					case 0:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						PC++;
						break;

					case 1:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						PC++;
						break;

					case 2:
						address_bus = get_PC_Addr();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 3:
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

						get_Push_value();
						break;

					case 2:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;

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
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 3:
						// same address bus as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 4:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC >> 8;
						break;

					case 5:
						address_bus = S;
						Dec_S();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						value8 = PC;
						break;

					case 6:
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
						alu_temp = (((uint32_t)PC & 0xFF) + (int8_t)opcode2);
						PC &= 0xFF00;
						PC |= (uint16_t)(alu_temp & 0xFF);

						if (!((alu_temp & 0x100) == 0x100))
						{
							Instr_Cycle++;
						}
						break;

					case 2:
						// same address bus value as above
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						if ((alu_temp & 0x80000000) == 0x80000000)
							PC = (uint16_t)(PC - 0x100);
						else PC = (uint16_t)(PC + 0x100);
						break;

					case 3:
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
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 3:
						get_Direct_Addr_Inc();
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
						break;

					case 3:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						get_Direct_Addr_Inc();
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						// same address bus value as above
						value8 = alu_temp_16 >> 8;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;
					case 6:
						address_bus = (D + opcode2) & 0xFFFF;
						value8 = alu_temp_16 & 0xFF;
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
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 4:
						get_Direct_Addr_Inc();
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
						break;

					case 4:
						get_Direct_Addr_Inc();
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 5:
						get_Direct_Addr_Inc();
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 6:
						// same address bus value as above
						value8 = alu_temp_16 >> 8;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;
					case 7:
						address_bus = (D + opcode2 + Index_Add) & 0xFFFF;
						value8 = alu_temp_16 & 0xFF;
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
						address_bus = (uint32_t)(opcode3 << 8) | opcode2;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
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
						address_bus = (uint32_t)(opcode3 << 8) | opcode2;
						get_Write_value();
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 3:
						address_bus++;
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
						ea = (uint32_t)(opcode3 << 8) | opcode2;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
						break;

					case 3:
						address_bus = ea + 1;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
						break;

					case 4:
						address_bus = ea + 1;
						ALU_Operation();
						Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
						break;

					case 5:
						// same address bus value as above
						value8 = alu_temp_16 >> 8;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 6:
						address_bus = (uint32_t)(opcode3 << 8) | opcode2;
						value8 = alu_temp_16 & 0xFF;
						Cycle_Type = CPU_Cycle_Type::Write_Cycle;
						break;

					case 7:
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
						ea = (uint32_t)(opcode4 << 8) | opcode3;
						address_bus = ea;
						Cycle_Type = CPU_Cycle_Type::Read_Cycle;
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
					ea = (uint32_t)(opcode4 << 8) | opcode3;
					address_bus = ea;
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 6:
					address_bus++;
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
					ea = (uint32_t)(opcode4 << 8) | opcode3;
					address_bus = ea;
					Cycle_Type = CPU_Cycle_Type::Read_Cycle;
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
					ea = (uint32_t)(opcode4 << 8) | opcode3;
					address_bus = ea;
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 5:
					address_bus++;
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
					address_bus = (uint32_t)(opcode4 << 8) | opcode3;
					Cycle_Type = CPU_Cycle_Type::Read_Cycle;
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
					address_bus = (uint32_t)(opcode4 << 8) | opcode3;
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
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
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
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
					ea = ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
					address_bus = ea;
					Cycle_Type = CPU_Cycle_Type::Read_Cycle;
					break;

				case 6:
					address_bus = ((address_bus + 1) & 0xFFFF);
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
					ea = ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
					address_bus = ea;
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 6:
					address_bus = ((address_bus + 1) & 0xFFFF);
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
					break;

				case 2:
					address_bus = ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF);
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 3:
					address_bus = ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
					Cycle_Type = CPU_Cycle_Type::Read_Cycle;
					break;

				case 4:
					address_bus = ((address_bus + 1) & 0xFFFF);
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
					address_bus = ((uint32_t)(opcode3 << 8)) | ((opcode2 + (Index_Add & 0xFF)) & 0xFF);
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 3:
					address_bus = ((((uint32_t)(opcode3 << 8) | opcode2) + Index_Add) & 0xFFFF);
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 4:
					address_bus = ((address_bus + 1) & 0xFFFF);
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
					address_bus = ((uint32_t)(opcode3 << 8)) | ((opcode2 + (X & 0xFF)) & 0xFF);
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 3:
					ea = ((((uint32_t)(opcode3 << 8) | opcode2) + X) & 0xFFFF);
					address_bus = ea;

					Cycle_Type = CPU_Cycle_Type::Read_Cycle;
					break;

				case 4:
					address_bus = ((ea + 1) & 0xFFFF);
					Cycle_Type = CPU_Cycle_Type::Read_Cycle_Hi;
					break;

				case 5:
					address_bus = ((ea + 1) & 0xFFFF);
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					ALU_Operation();
					break;

				case 6:
					address_bus = ((ea + 1) & 0xFFFF);
					value8 = alu_temp_16 >> 8;
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 7:
					address_bus = (ea & 0xFFFF);
					value8 = alu_temp_16 & 0xFF;
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
					Index_Add = opcode2;
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
					address_bus = ((uint32_t)(opcode4 << 8)) | ((opcode3 + (Y & 0xFF)) & 0xFF);
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 5:
					address_bus = ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
					Cycle_Type = CPU_Cycle_Type::Read_Cycle;
					break;

				case 6:
					address_bus = ((address_bus + 1) & 0xFFFF);
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
					address_bus = ((uint32_t)(opcode4 << 8)) | ((opcode3 + (Y & 0xFF)) & 0xFF);
					Cycle_Type = CPU_Cycle_Type::Internal_Cycle;
					break;

				case 5:
					address_bus = ((((uint32_t)(opcode4 << 8) | opcode3) + Y) & 0xFFFF);
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 6:
					address_bus = ((address_bus + 1) & 0xFFFF);
					get_Write_value();
					Cycle_Type = CPU_Cycle_Type::Write_Cycle;
					break;

				case 7:
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
				alu_temp_16 = alu_temp;
				break;

			case ALU::DEC:
				alu_temp = (uint8_t)((alu_temp - 1) & 0xFF);
				NZ_Set(alu_temp);
				alu_temp_16 = alu_temp;
				break;

			case ALU::ASL:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 0x80) != 0);
				alu_temp = value8 = (uint8_t)(value8 << 1);
				NZ_Set(value8);
				alu_temp_16 = alu_temp;
				break;

			case ALU::ROL:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 << 1) | (P & 1));
				FlagCset((temp8 & 0x80) != 0);
				NZ_Set(value8);
				alu_temp_16 = alu_temp;
				break;

			case ALU::ROR:
				value8 = temp8 = (uint8_t)alu_temp;
				alu_temp = value8 = (uint8_t)((value8 >> 1) | ((P & 1) << 7));
				FlagCset((temp8 & 1) != 0);
				NZ_Set(value8);
				alu_temp_16 = alu_temp;
				break;

			case ALU::LSR:
				value8 = (uint8_t)alu_temp;
				FlagCset((value8 & 1) != 0);
				alu_temp = value8 = (uint8_t)(value8 >> 1);
				NZ_Set(value8);
				alu_temp_16 = alu_temp;
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
				if (FlagDget())
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
					NZ_Set(A);
				}
				break;

			case ALU::SBC:
				value8 = (uint8_t)alu_temp;
				tempint = A - value8 - (FlagCget() ? 0 : 1);
				if (FlagDget())
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
					NZ_Set(A);
				}
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