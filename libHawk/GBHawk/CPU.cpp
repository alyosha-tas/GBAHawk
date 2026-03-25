#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "GB_System.h"
#include "PPUs.h"

namespace GBHawk
{
	void GB_System::cpu_Execute_One()
	{
		bool cd_chk_retc = false;
		bool cd_chk_jpcond = false;
		bool cd_chk_jrcond = false;
		bool cd_chk_call = false;
	
		switch (cpu_Instr_Type)
		{
			case OpT::INT_OP:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Exec_INT_Func();  break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::REG_OP:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Exec_REG_Func();  break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::REG_OP_IND_INC:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 5:						
						switch (cpu_Opcode)
						{
							case 0xC6: cpu_ADD8_Func(cpu_A, cpu_Z); break;
							case 0xCE: cpu_ADC8_Func(cpu_A, cpu_Z); break;
							case 0xD6: cpu_SUB8_Func(cpu_A, cpu_Z); break;
							case 0xDE: cpu_SBC8_Func(cpu_A, cpu_Z); break;
							case 0xE6: cpu_AND8_Func(cpu_A, cpu_Z); break;
							case 0xEE: cpu_XOR8_Func(cpu_A, cpu_Z); break;
							case 0xF6: cpu_OR8_Func(cpu_A, cpu_Z); break;
							case 0xFE: cpu_CP8_Func(cpu_A, cpu_Z); break;
						}
						  break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_IND_16:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: 
						switch (cpu_Opcode)
						{
							case 0x01: cpu_Regs[cpu_C] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x11: cpu_Regs[cpu_E] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x21: cpu_Regs[cpu_L] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x31: cpu_Regs[cpu_SPl] = GB_System::Read_Memory(cpu_RegPCget()); break;
						}
						break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7:
						switch (cpu_Opcode)
						{
							case 0x01: cpu_Regs[cpu_B] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x11: cpu_Regs[cpu_D] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x21: cpu_Regs[cpu_H] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x31: cpu_Regs[cpu_SPh] = GB_System::Read_Memory(cpu_RegPCget()); break;
						}
						break;
					case 8: break;
					case 9: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));  break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_8_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: 
						switch (cpu_Opcode)
						{
							case 0x02: GB_System::Write_Memory(cpu_RegBCget(), cpu_Regs[cpu_A]); break;
							case 0x12: GB_System::Write_Memory(cpu_RegDEget(), cpu_Regs[cpu_A]); break;
							case 0x70: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_B]); break;
							case 0x71: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_C]); break;
							case 0x72: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_D]); break;
							case 0x73: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_E]); break;
							case 0x74: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_H]); break;
							case 0x75: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_L]); break;
							case 0x77: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_A]); break;
						}
						break;				
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::INC_DEC_16:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3:
						switch (cpu_Opcode)
						{
							case 0x03: cpu_RegBCset((uint16_t)(cpu_RegBCget() + 1)); break;
							case 0x0B: cpu_RegBCset((uint16_t)(cpu_RegBCget() - 1)); break;
							case 0x13: cpu_RegDEset((uint16_t)(cpu_RegDEget() + 1)); break;
							case 0x1B: cpu_RegDEset((uint16_t)(cpu_RegDEget() - 1)); break;
							case 0x23: cpu_RegHLset((uint16_t)(cpu_RegHLget() + 1)); break;
							case 0x2B: cpu_RegHLset((uint16_t)(cpu_RegHLget() - 1)); break;
							case 0x33: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
							case 0x3B: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
						}
						break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_IND_8_INC:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3:
						switch (cpu_Opcode)
						{
							case 0x06: cpu_Regs[cpu_B] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x0E: cpu_Regs[cpu_C] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x16: cpu_Regs[cpu_D] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x1E: cpu_Regs[cpu_E] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x26: cpu_Regs[cpu_H] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x2E: cpu_Regs[cpu_L] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x3E: cpu_Regs[cpu_A] = GB_System::Read_Memory(cpu_RegPCget()); break;
						}
						break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_R_IM:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 8: break;
					case 9: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 10: break;
					case 11: GB_System::Write_Memory(cpu_RegWZget(), cpu_Regs[cpu_SPl]); break;
					case 12: break;
					case 13: cpu_RegWZset((uint16_t)(cpu_RegWZget() + 1)); break;
					case 14: break;
					case 15: GB_System::Write_Memory(cpu_RegWZget(), cpu_Regs[cpu_SPh]); break;
					case 16: break;
					case 17: break;
					case 18: cpu_Halt_Check_Func(); break;
					case 19: cpu_Op_Func(); break;
				}
				break;

			case OpT::ADD_16:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3:
						switch (cpu_Opcode)
						{
							case 0x09: cpu_ADD16_Func(cpu_L, cpu_H, cpu_C, cpu_B); break;
							case 0x19: cpu_ADD16_Func(cpu_L, cpu_H, cpu_E, cpu_D); break;
							case 0x29: cpu_ADD16_Func(cpu_L, cpu_H, cpu_L, cpu_H); break;
							case 0x39: cpu_ADD16_Func(cpu_L, cpu_H, cpu_SPl, cpu_SPh); break;
						}
						break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::STOP:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 1: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 2: break;
					case 3: cpu_STOP_Ex(); break;
				}
				break;

			case OpT::JR_COND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 5:
						cd_chk_jrcond = false;
						switch (cpu_Opcode)
						{
							case 0x18: cd_chk_jrcond = true; break;
							case 0x20: cd_chk_jrcond = !cpu_FlagZget(); break;
							case 0x28: cd_chk_jrcond = cpu_FlagZget(); break;
							case 0x30: cd_chk_jrcond = !cpu_FlagCget(); break;
							case 0x38: cd_chk_jrcond = cpu_FlagCget(); break;
						}
						if (!cd_chk_jrcond)
						{
							cpu_Instr_Cycle += 4;
						}
						break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = 0; break;
					case 8: break;
					case 9: cpu_ADDS_Func(cpu_PCl, cpu_PCh, cpu_Z, cpu_W); break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}

				break;

			case OpT::LD_8_IND_INC_DEC:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_A]); break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x22: cpu_RegHLset((uint16_t)(cpu_RegHLget() + 1)); break;
							case 0x32: cpu_RegHLset((uint16_t)(cpu_RegHLget() - 1)); break;
						}
						break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_IND_8_INC_DEC_HL:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_A] = GB_System::Read_Memory(cpu_RegHLget()); break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x2A: cpu_RegHLset((uint16_t)(cpu_RegHLget() + 1)); break;
							case 0x3A: cpu_RegHLset((uint16_t)(cpu_RegHLget() - 1)); break;
						}
						break;					
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::INC_DEC_8_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegHLget()); break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x34: cpu_INC8_Func(cpu_Z); break;
							case 0x35: cpu_DEC8_Func(cpu_Z); break;
						}
						break;
					case 6: break;
					case 7: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_Z]); break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_8_IND_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_Z]);  break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::RET_COND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3:
						cd_chk_retc = false;
						switch (cpu_Opcode)
						{
							case 0xC0: cd_chk_retc = !cpu_FlagZget(); break;
							case 0xC8: cd_chk_retc = cpu_FlagZget(); break;
							case 0xD0: cd_chk_retc = !cpu_FlagCget(); break;
							case 0xD8: cd_chk_retc = cpu_FlagCget(); break;
						}
						if (!cd_chk_retc)
						{
							cpu_Instr_Cycle += 12;
						}
						break;
					case 4: break;
					case 5: break;
					case 6: break;
					case 7: cpu_Regs[cpu_PCl] = GB_System::Read_Memory(cpu_RegSPget()); break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 10: break;
					case 11: cpu_Regs[cpu_PCh] = GB_System::Read_Memory(cpu_RegSPget()); break;
					case 12: break;
					case 13: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 14: break;
					case 15: break;
					case 16: break;
					case 17: break;
					case 18: cpu_Halt_Check_Func(); break;
					case 19: cpu_Op_Func(); break;
				}
				break;

			case OpT::POP:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3:
						switch (cpu_Opcode)
						{
							case 0xC1: cpu_Regs[cpu_C] = GB_System::Read_Memory(cpu_RegSPget()); break;
							case 0xD1: cpu_Regs[cpu_E] = GB_System::Read_Memory(cpu_RegSPget()); break;
							case 0xE1: cpu_Regs[cpu_L] = GB_System::Read_Memory(cpu_RegSPget()); break;
							// Bottom 4 bits of F always 0
							case 0xF1: cpu_Regs[cpu_F] = (GB_System::Read_Memory(cpu_RegSPget()) & 0xF0); break;
						}
						break;
					case 4: break;
					case 5: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 6: break;
					case 7:
						switch (cpu_Opcode)
						{
							case 0xC1: cpu_Regs[cpu_B] = GB_System::Read_Memory(cpu_RegSPget()); break;
							case 0xD1: cpu_Regs[cpu_D] = GB_System::Read_Memory(cpu_RegSPget()); break;
							case 0xE1: cpu_Regs[cpu_H] = GB_System::Read_Memory(cpu_RegSPget()); break;
							case 0xF1: cpu_Regs[cpu_A] = GB_System::Read_Memory(cpu_RegSPget()); break;
						}
						break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::JP_COND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 8: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 9:
						cd_chk_jpcond = false;
						switch (cpu_Opcode)
						{
							case 0xC2: cd_chk_jpcond = !cpu_FlagZget(); break;
							case 0xC3: cd_chk_jpcond = true; break;
							case 0xCA: cd_chk_jpcond = cpu_FlagZget(); break;
							case 0xD2: cd_chk_jpcond = !cpu_FlagCget(); break;
							case 0xDA: cd_chk_jpcond = cpu_FlagCget(); break;
						}
						if (!cd_chk_jpcond)
						{
							cpu_Instr_Cycle += 4;
						}
						break;
					case 10: break;
					case 11: break;
					case 12: cpu_RegPCset((uint16_t)(cpu_RegWZget())); break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::CALL_COND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 8: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 9: 
						cd_chk_call = false;
						switch (cpu_Opcode)
						{
							case 0xC4: cd_chk_call = !cpu_FlagZget(); break;
							case 0xCC: cd_chk_call = cpu_FlagZget(); break;
							case 0xCD: cd_chk_call = true; break;
							case 0xD4: cd_chk_call = !cpu_FlagCget(); break;
							case 0xDC: cd_chk_call = cpu_FlagCget(); break;
						}
						if (!cd_chk_call)
						{
							cpu_Instr_Cycle += 12;
						}
						break;
					case 10: break;
					case 11: break;
					case 12: break;
					case 13: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 14: break;
					case 15: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCh]); break;
					case 16: break;
					case 17: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 18: break;
					case 19: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCl]); break;
					case 20: break;
					case 21: cpu_RegPCset((uint16_t)(cpu_RegWZget())); break;
					case 22: cpu_Halt_Check_Func(); break;
					case 23: cpu_Op_Func(); break;
				}
				break;

			case OpT::PUSH:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: break;
					case 4: break;
					case 5: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 6: break;
					case 7:
						switch (cpu_Opcode)
						{
							case 0xC5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_B]); break;
							case 0xD5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_D]); break;
							case 0xE5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_H]); break;
							case 0xF5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_A]); break;
						}
						break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 10: break;
					case 11:
						switch (cpu_Opcode)
						{
							case 0xC5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_C]); break;
							case 0xD5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_E]); break;
							case 0xE5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_L]); break;
							case 0xF5: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_F]); break;
						}
						break;
					case 12: break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::REG_OP_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3:
						switch (cpu_Opcode)
						{
							case 0x0A: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegBCget()); break;
							case 0x1A: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegDEget()); break;
							case 0x46:
							case 0x4E:
							case 0x56:
							case 0x5E:
							case 0x66:
							case 0x6E:
							case 0x7E:
							case 0x86:
							case 0x8E:
							case 0x96:
							case 0x9E:
							case 0xA6:
							case 0xAE:
							case 0xB6:
							case 0xBE: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegHLget()); break;
						}
						break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x0A: cpu_TR_Func(cpu_A, cpu_Z); break;
							case 0x1A: cpu_TR_Func(cpu_A, cpu_Z); break;
							case 0x46: cpu_TR_Func(cpu_B, cpu_Z); break;
							case 0x4E: cpu_TR_Func(cpu_C, cpu_Z); break;
							case 0x56: cpu_TR_Func(cpu_D, cpu_Z); break;
							case 0x5E: cpu_TR_Func(cpu_E, cpu_Z); break;
							case 0x66: cpu_TR_Func(cpu_H, cpu_Z); break;
							case 0x6E: cpu_TR_Func(cpu_L, cpu_Z); break;
							case 0x7E: cpu_TR_Func(cpu_A, cpu_Z); break;
							case 0x86: cpu_ADD8_Func(cpu_A, cpu_Z); break;
							case 0x8E: cpu_ADC8_Func(cpu_A, cpu_Z); break;
							case 0x96: cpu_SUB8_Func(cpu_A, cpu_Z); break;
							case 0x9E: cpu_SBC8_Func(cpu_A, cpu_Z); break;
							case 0xA6: cpu_AND8_Func(cpu_A, cpu_Z); break;
							case 0xAE: cpu_XOR8_Func(cpu_A, cpu_Z); break;
							case 0xB6: cpu_OR8_Func(cpu_A, cpu_Z); break;
							case 0xBE: cpu_CP8_Func(cpu_A, cpu_Z); break;
						}
						break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::RST:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: break;
					case 4: break;
					case 5: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 6: break;
					case 7: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCh]); break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 10: break;
					case 11: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCl]); break;
					case 12: cpu_Regs[cpu_PCh] = 0; break;
					case 13:
						switch (cpu_Opcode)
						{
							case 0xC7: cpu_Regs[cpu_PCl] = 0; break;
							case 0xCF: cpu_Regs[cpu_PCl] = 8; break;
							case 0xD7: cpu_Regs[cpu_PCl] = 0x10; break;
							case 0xDF: cpu_Regs[cpu_PCl] = 0x18; break;
							case 0xE7: cpu_Regs[cpu_PCl] = 0x20; break;
							case 0xEF: cpu_Regs[cpu_PCl] = 0x28; break;
							case 0xF7: cpu_Regs[cpu_PCl] = 0x30; break;
							case 0xFF: cpu_Regs[cpu_PCl] = 0x38; break;
						}
						break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::RET:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_PCl] = GB_System::Read_Memory(cpu_RegSPget()); break;
					case 4: break;
					case 5: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_PCh] = GB_System::Read_Memory(cpu_RegSPget()); break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 10: break;
					case 11: break;
					case 12: break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::PREFIX:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_CB_Prefix = true; break;
					case 1: break;
					case 2: break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::JAM:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Jammed = true; break;
					case 1: cpu_Instr_Cycle -= 1; break;
					case 2: break;
					case 3: break;
				}
				break;

			case OpT::RETI:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_PCl] = GB_System::Read_Memory(cpu_RegSPget()); break;
					case 4: break;
					case 5: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_PCh] = GB_System::Read_Memory(cpu_RegSPget()); break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() + 1)); break;
					case 10: break;
					case 11: cpu_EI_Pending = 1; break;
					case 12: break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_FF_IND_8:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 5: break;
					case 6: cpu_Regs[cpu_W] = 0xFF; break;
					case 7: GB_System::Write_Memory(cpu_RegWZget(), cpu_Regs[cpu_A]); break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_FFC_IND_8:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: GB_System::Write_Memory((uint16_t)(0xFF00 | cpu_Regs[cpu_C]), cpu_Regs[cpu_A]); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::ADD_SP:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = 0; break;
					case 8: break;
					case 9: break;
					case 10: break;
					case 11: cpu_ADDS_Func(cpu_SPl, cpu_SPh, cpu_Z, cpu_W); break;
					case 12: break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::JP_HL:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_RegPCset((uint16_t)(cpu_RegHLget())); break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_FF_IND_16:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 8: break;
					case 9: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 10: break;
					case 11: GB_System::Write_Memory(cpu_RegWZget(), cpu_Regs[cpu_A]); break;
					case 12: break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_8_IND_FF:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 5: break;
					case 6: cpu_Regs[cpu_W] = 0xFF; break;
					case 7: cpu_Regs[cpu_A] = GB_System::Read_Memory(cpu_RegWZget()); break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_8_IND_FFC:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_A] = GB_System::Read_Memory((uint16_t)(0xFF00 | cpu_Regs[cpu_C])); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::EI_DI:
				switch (cpu_Instr_Cycle)
				{
					case 0:
						if (cpu_Opcode == 0xF3) { cpu_Interrupts_Enabled = false; cpu_EI_Pending = 0; }
						else { if (cpu_EI_Pending == 0) { cpu_EI_Pending = 2; } }
						break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_HL_SPn:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_RegHLset((uint16_t)(cpu_RegSPget())); break;
					case 8: cpu_Regs[cpu_W] = 0;  break;
					case 9: cpu_ADDS_Func(cpu_L, cpu_H, cpu_Z, cpu_W); break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_SP_HL:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_RegSPset((uint16_t)(cpu_RegHLget())); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_16_IND_FF:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_W] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 8: break;
					case 9: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 10: break;
					case 11: cpu_Regs[cpu_A] = GB_System::Read_Memory(cpu_RegWZget()); break;
					case 12: break;
					case 13: break;
					case 14: cpu_Halt_Check_Func(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::INT_OP_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegHLget()); break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x106: cpu_RLC_Func(cpu_Z); break;
							case 0x10E: cpu_RRC_Func(cpu_Z); break;
							case 0x116: cpu_RL_Func(cpu_Z); break;
							case 0x11E: cpu_RR_Func(cpu_Z); break;
							case 0x126: cpu_SLA_Func(cpu_Z); break;
							case 0x12E: cpu_SRA_Func(cpu_Z); break;
							case 0x136: cpu_SWAP_Func(cpu_Z); break;
							case 0x13E: cpu_SRL_Func(cpu_Z); break;
						}
						break;
					case 6: break;
					case 7: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_Z]); break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::BIT_OP:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Exec_Bit_Func();  break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::BIT_TE_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegHLget()); break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x146: cpu_BIT_Func(0, cpu_Z); break;
							case 0x14E: cpu_BIT_Func(1, cpu_Z); break;
							case 0x156: cpu_BIT_Func(2, cpu_Z); break;
							case 0x15E: cpu_BIT_Func(3, cpu_Z); break;
							case 0x166: cpu_BIT_Func(4, cpu_Z); break;
							case 0x16E: cpu_BIT_Func(5, cpu_Z); break;
							case 0x176: cpu_BIT_Func(6, cpu_Z); break;
							case 0x17E: cpu_BIT_Func(7, cpu_Z); break;
						}
						break;
					case 6: cpu_Halt_Check_Func(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::BIT_OP_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegHLget()); break;
					case 4: break;
					case 5:
						switch (cpu_Opcode)
						{
							case 0x186: cpu_RES_Func(0, cpu_Z); break;
							case 0x18E: cpu_RES_Func(1, cpu_Z); break;
							case 0x196: cpu_RES_Func(2, cpu_Z); break;
							case 0x19E: cpu_RES_Func(3, cpu_Z); break;
							case 0x1A6: cpu_RES_Func(4, cpu_Z); break;
							case 0x1AE: cpu_RES_Func(5, cpu_Z); break;
							case 0x1B6: cpu_RES_Func(6, cpu_Z); break;
							case 0x1BE: cpu_RES_Func(7, cpu_Z); break;
							case 0x1C6: cpu_SET_Func(0, cpu_Z); break;
							case 0x1CE: cpu_SET_Func(1, cpu_Z); break;
							case 0x1D6: cpu_SET_Func(2, cpu_Z); break;
							case 0x1DE: cpu_SET_Func(3, cpu_Z); break;
							case 0x1E6: cpu_SET_Func(4, cpu_Z); break;
							case 0x1EE: cpu_SET_Func(5, cpu_Z); break;
							case 0x1F6: cpu_SET_Func(6, cpu_Z); break;
							case 0x1FE: cpu_SET_Func(7, cpu_Z); break;
						}
						break;
					case 6: break;
					case 7: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_Z]); break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::RESET:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: break;
					case 4: break;
					case 5: break;
					case 6: break;
					case 7: break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check_Func(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::EXIT:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case OpT::SKIP:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Halt_Ex(0); break;
				}
				break;

			case OpT::HALT:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Halt_Enter(); break;
					case 1: break;
					case 2: break;
					case 3: cpu_Op_G_Func(); break;
					case 4: cpu_Halt_Check_Func(); break;
					case 5:if (cpu_FlagI && !cpu_I_Use) { cpu_Halt_bug_5 = true; } break;
					case 6: cpu_Halt_Ex(0); break;
				}
				break;

			case OpT::GBC_HALT:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_Halt_Ex(0); break;
				}
				break;

			case OpT::SPC_HALT:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Halt_Check_Func(); break;
					case 1: break;
					case 2: break;
					case 3: cpu_Halt_Ex(0); break;
				}
				break;

			case OpT::STOP_LOOP:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_STOP_Ex(); break;
				}
				break;

			case OpT::INTRPT:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: break;
					case 4: break;
					case 5: break;
					case 6: break;
					case 7: break;
					case 8: break;
					case 9: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 10: break;
					case 11: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCh]); break;
					case 12: cpu_Regs[cpu_PCh] = 0; break;
					case 13: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 14: cpu_Int_Get_Func(cpu_W, 1); break;
					case 15: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCl]); break;
					case 16: break;
					case 17: cpu_IRQ_Clear_Func(); break;
					case 18: cpu_Regs[cpu_PCl] = cpu_Regs[cpu_W]; break;
					case 19: cpu_Op_Func(); break;
				}
				break;

			case OpT::INTRPT_GBC:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Instruction_Start = Cycle_Count + 1; break;
					case 4: break;
					case 5: break;
					case 6: break;
					case 7: break;
					case 8: break;
					case 9: break;
					case 10: break;
					case 11: break;
					case 12: break;
					case 13: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 14: break;
					case 15: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCh]); break;
					case 16: cpu_Regs[cpu_PCh] = 0; break;
					case 17: cpu_RegSPset((uint16_t)(cpu_RegSPget() - 1)); break;
					case 18: cpu_Int_Get_Func(cpu_W, 1); break;
					case 19: GB_System::Write_Memory(cpu_RegSPget(), cpu_Regs[cpu_PCl]); break;
					case 20: break;
					case 21: cpu_IRQ_Clear_Func(); break;
					case 22: cpu_Regs[cpu_PCl] = cpu_Regs[cpu_W]; break;
					case 23: cpu_Op_Func(); break;
				}
				break;

			case OpT::WAIT:
				cpu_Instr_Cycle = -1;
				break;

			case OpT::SPD_CHG:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: SpeedFunc(1); break;
					case 2: cpu_Halt_Check_Func(); break;
					case 3: cpu_STOP_Ex(); break;
				}
				break;
		}

		cpu_Instr_Cycle += 1;
	}

	inline uint32_t GB_System::SpeedFunc(uint32_t val)
	{
		if (val == 0)
		{
			if (speed_switch)
			{
				speed_switch = false;

				Message_String = "Speed Switch: " + to_string(Cycle_Count);

				MessageCallback(Message_String.length());

				uint32_t ret = Double_Speed ? 32770 : 32770; // actual time needs checking
				return ret;
			}

			// if we are not switching speed, return 0
			return 0;
		}
		else if (val == 1)
		{
			// reset the divider (only way for speed_change_timing_fine.gbc and speed_change_cancel.gbc to both work)
			// Console.WriteLine("at stop " + timer.divider_reg + " " + timer.timer_control);

			// only if the timer mode is 1, an extra tick of the timer is counted before the reset
			// this varies between console revisions
			if ((tim_Control & 7) == 5)
			{
				if ((tim_Divider_Reg & 0x7) == 7)
				{
					tim_Old_State = true;
				}
			}

			tim_Divider_Reg = 0xFFFF;

			Double_Speed = !Double_Speed;

			ppu_pntr->LYC_offset = Double_Speed ? 1 : 2;

			ppu_pntr->LY_153_change = Double_Speed ? 8 : 10;

			return 0;
		}
		else
		{

			return 0;
		}
	}

	inline void GB_System::cpu_Halt_Enter()
	{
		if (cpu_was_FlagI && (cpu_EI_Pending == 0) && !cpu_Interrupts_Enabled)
		{
			// in GBC mode, the HALT bug is worked around by simply adding a NOP
			// so it just takes 4 cycles longer to reach the next instruction

			// otherwise, if interrupts are disabled,
			// a glitchy decrement to the program counter happens

			// either way, nothing needs to be done here
		}
		else
		{
			cpu_Instr_Cycle += 3;

			if (!Is_GBC) { cpu_Skip_Once = true; }
			// If the interrupt flag is not currently set, but it does get set in the first check
			// then a bug is triggered
			// With interrupts enabled, this runs the halt command twice
			// when they are disabled, it reads the next byte twice
			if (!cpu_was_FlagI || (cpu_was_FlagI && !cpu_Interrupts_Enabled)) { cpu_Halt_bug_2 = true; }

			// If the I flag was set right before hitting this point, then there is no extra cycle for the halt
			// also there is a glitched increment to the program counter
			if (cpu_was_FlagI && cpu_Interrupts_Enabled)
			{
				cpu_Halt_bug_4 = true;
			}
		}
	}

	inline void GB_System::cpu_Halt_Ex(uint8_t param)
	{
		cpu_Halted = true;

		bool temp = false;

		if (param == 1)
		{
			temp = cpu_FlagI;
		}
		else
		{
			temp = cpu_I_Use;
		}

		if (cpu_EI_Pending > 0 && !cpu_CB_Prefix)
		{
			cpu_EI_Pending--;
			if (cpu_EI_Pending == 0)
			{
				cpu_Interrupts_Enabled = true;
			}
		}

		// if the I flag is asserted at the time of halt, don't halt
		if (cpu_Halt_bug_5)
		{
			cpu_Halt_bug_5 = cpu_Halt_bug_3 = cpu_Halted = cpu_Skip_Once = false;

			if (cpu_Interrupts_Enabled)
			{
				cpu_Interrupts_Enabled = false;

				if (TraceCallback) TraceCallback(3); // IRQ

				cpu_RegPCset(cpu_RegPCget() - 1);

				// TODO: If interrupt priotrity is checked differently in GBC, then this is correct
				// a new interrupt vector would be needed
				cpu_Instr_Type = OpT::INTRPT; // point to Interrupt
			}
			else
			{
				if (TraceCallback) TraceCallback(2); // Un-Halt

				if (TraceCallback && !cpu_CB_Prefix) TraceCallback(0);

				cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget())); // note no increment
			}
		}
		else if (temp && cpu_Interrupts_Enabled)
		{
			cpu_Interrupts_Enabled = false;

			if (TraceCallback) TraceCallback(3); // IRQ
			cpu_Halted = false;

			if (cpu_Halt_bug_4)
			{
				// TODO: If interrupt priotrity is checked differently in GBC, then this is incorrect
				// a new interrupt vector would be needed
				cpu_RegPCset(cpu_RegPCget() - 1);
				cpu_Instr_Type = OpT::INTRPT; // point to Interrupt

				cpu_Halt_bug_4 = false;
				cpu_Skip_Once = false;
				cpu_Halt_bug_3 = false;
			}
			else if (Is_GBC)
			{
				// call the interrupt processor after 4 extra cycles
				if (!cpu_Halt_bug_3)
				{
					cpu_Instr_Type = OpT::INTRPT_GBC; // point to Interrupt for GBC
				}
				else
				{
					// TODO: If interrupt priotrity is checked differently in GBC, then this is incorrect
					// a new interrupt vector would be needed
					cpu_Instr_Type = OpT::INTRPT; // point to Interrupt
					cpu_Halt_bug_3 = false;
					//Console.WriteLine("Hit INT");
				}
			}
			else
			{
				// call interrupt processor
				cpu_Instr_Type = OpT::INTRPT; // point to Interrupt
				cpu_Halt_bug_3 = false;
			}
		}
		else if (temp)
		{
			// even if interrupt servicing is disabled, any interrupt flag raised still resumes execution
			if (TraceCallback) TraceCallback(2); // Un-Halt
			cpu_Halted = false;

			if (Is_GBC)
			{
				// extra 4 cycles for GBC
				if (cpu_Halt_bug_3)
				{
					if (TraceCallback && !cpu_CB_Prefix) TraceCallback(0);

					cpu_RegPCset(cpu_RegPCget() + 1);
					cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget()));

					cpu_Halt_bug_3 = false;
					//Console.WriteLine("Hit un");
				}
				else
				{
					cpu_Instr_Type = OpT::EXIT; // exit halt loop
				}
			}
			else
			{
				if (TraceCallback && !cpu_CB_Prefix) TraceCallback(0);

				if (cpu_Halt_bug_3)
				{
					//special variant of halt bug where RegPC also isn't incremented post fetch
					cpu_RegPCset(cpu_RegPCget() + 1);
					cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget()));

					cpu_Halt_bug_3 = false;
				}
				else
				{
					cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget()));
					cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));
				}
			}
		}
		else
		{
			if (cpu_Skip_Once)
			{
				cpu_Instr_Type = OpT::SKIP; // point to skipped loop
				cpu_Skip_Once = false;
			}
			else
			{
				if (Is_GBC)
				{
					cpu_Instr_Type = OpT::GBC_HALT; // point to GBC Halt loop
				}
				else
				{
					cpu_Instr_Type = OpT::SPC_HALT; // point to spec Halt loop
				}
			}
		}

		cpu_Instr_Cycle = -1;
		cpu_I_Use = false;
	}

	inline void GB_System::cpu_STOP_Ex()
	{
		cpu_Stopped = true;
		if (!cpu_Stop_Check)
		{
			// Z contains the second stop byte, not sure if it's useful at all
			cpu_Stop_Time = SpeedFunc(0);
			cpu_Stop_Check = true;
		}

		uint8_t buttons_pressed = GetButtons(0);

		if (cpu_Stop_Time > 0)
		{
			// Timer interrupts can prematurely terminate a speedchange, not sure about other sources
			// NOTE: some testing around the edge case of where the speed actually changes is needed						
			if (cpu_I_Use && cpu_Interrupts_Enabled)
			{
				cpu_Interrupts_Enabled = false;
				cpu_I_Use = false;

				if (TraceCallback) TraceCallback(1); // Un-Stop

				cpu_Stopped = false;
				cpu_Stop_Check = false;
				cpu_Stop_Time = 0;

				if (TraceCallback) TraceCallback(3); // IRQ

				// call interrupt processor 
				// lowest bit set is highest priority
				cpu_Instr_Type = OpT::INTRPT; // point to Interrupt
				cpu_Instr_Cycle = -1;
				return;
			}

			if (cpu_Stop_Time == 32770)
			{
				// point to speed cange loop
				cpu_Instr_Type = OpT::SPD_CHG;
				cpu_Instr_Cycle = -1;
				cpu_Stop_Time--;
				return;
			}

			cpu_Stop_Time--;

			if (cpu_Stop_Time == 0)
			{
				if (TraceCallback) TraceCallback(1); // Un-Stop

				cpu_Stopped = false;

				// it takes the CPU 4 cycles longer to restart then the rest of the system.
				cpu_Instr_Type = OpT::EXIT;
				cpu_Instr_Cycle = -1;

				cpu_Stop_Check = false;

				return;
			}

			// If a button is pressed during speed change, the processor will jam
			if ((buttons_pressed & 0xF) != 0xF)
			{
				cpu_Stop_Time++;
				cpu_Instr_Cycle -= 1; // TODO: check
				return;
			}
		}

		// Button press will exit stop loop even if speed change in progress, even without interrupts enabled
		if ((buttons_pressed & 0xF) != 0xF)
		{
			// TODO: On a gameboy, you can only un-STOP once, needs further testing
			if (TraceCallback) TraceCallback(1); // Un-Stop

			cpu_Stopped = false;
			if (TraceCallback && !cpu_CB_Prefix) TraceCallback(0);
			
			cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget()));
			cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));
			cpu_Instr_Cycle = -1;

			cpu_Stop_Check = false;
		}
		else
		{
			cpu_Instr_Type = OpT::STOP_LOOP; // point to stop loop
			cpu_Instr_Cycle = -1;
		}
	}
}