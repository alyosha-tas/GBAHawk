#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "GB_System.h"

namespace GBHawk
{
	inline void GB_System::cpu_Execute_One()
	{
		switch (cpu_Instr_Type)
		{
			case OpT::INT_OP:
				break;

			case OpT::REG_OP:
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
							case 0xC6: cpu_ADD8_Func(cpu_A, cpu_Z); break;
							case 0xCE: cpu_ADC8_Func(cpu_A, cpu_Z); break;
							case 0xD6: cpu_SUB8_Func(cpu_A, cpu_Z); break;
							case 0xDE: cpu_SBC8_Func(cpu_A, cpu_Z); break;
							case 0x1E: cpu_AND8_Func(cpu_A, cpu_Z); break;
							case 0xEE: cpu_XOR8_Func(cpu_A, cpu_Z); break;
							case 0xF6: cpu_OR8_Func(cpu_A, cpu_Z); break;
							case 0xFE: cpu_CP8_Func(cpu_A, cpu_Z); break;
						}
						break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check(); break;
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
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));  break;
					case 6: break;
					case 7:
						switch (cpu_Opcode)
						{
							case 0x01: cpu_Regs[cpu_C] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x11: cpu_Regs[cpu_E] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x21: cpu_Regs[cpu_L] = GB_System::Read_Memory(cpu_RegPCget()); break;
							case 0x31: cpu_Regs[cpu_SPl] = GB_System::Read_Memory(cpu_RegPCget()); break;
						}
						break;
					case 8: break;
					case 9: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));  break;
					case 10: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_R_IM:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_W] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: break;
					case 7: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
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
					case 18: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 10: cpu_Halt_Check(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::LD_8_IND_IND:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegHLget()); break;
					case 4: break;
					case 5: cpu_RegHLset((uint16_t)(cpu_RegHLget() + 1)); break;
					case 6: break;
					case 7: GB_System::Write_Memory(cpu_RegHLget(), cpu_Regs[cpu_Z]);  break;
					case 8: break;
					case 9: break;
					case 10: cpu_Halt_Check(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::HALT:
				break;

			case OpT::RET_COND:
				break;

			case OpT::POP:
				if (src_l != F)
				{
					cur_instr = new[]
					{ IDLE,
					IDLE,
					IDLE,
					RD, src_l, SPl, SPh,
					IDLE,
					INC16, SPl, SPh,
					IDLE,
					RD, src_h, SPl, SPh,
					IDLE,
					INC16, SPl, SPh,
					HALT_CHK,
					OP };
				}
				else
				{
					cur_instr = new[]
					{ IDLE,
					IDLE,
					IDLE,
					RD_F, src_l, SPl, SPh,
					IDLE,
					INC16, SPl, SPh,
					IDLE,
					RD, src_h, SPl, SPh,
					IDLE,
					INC16, SPl, SPh,
					HALT_CHK,
					OP };
				}
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
					case 10: cpu_Halt_Check(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::JP_COND:
				break;

			case OpT::CALL_COND:
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
					case 14: cpu_Halt_Check(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::REG_OP_IND_INC:
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
					case 6: cpu_Halt_Check(); break;
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
					case 14: cpu_Halt_Check(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::RET:
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
					case 10: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 14: cpu_Halt_Check(); break;
					case 15: cpu_Op_Func(); break;
				}
				break;

			case OpT::JP_HL:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_RegPCset((uint16_t)(cpu_RegHLget())); break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
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
					case 14: cpu_Halt_Check(); break;
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
					case 10: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 2: cpu_Halt_Check(); break;
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
					case 10: cpu_Halt_Check(); break;
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
					case 6: cpu_Halt_Check(); break;
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
					case 14: cpu_Halt_Check(); break;
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
					case 10: cpu_Halt_Check(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::BIT_OP:
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
							case 0x156: cpu_BIT_Func(3, cpu_Z); break;
							case 0x15E: cpu_BIT_Func(3, cpu_Z); break;
							case 0x166: cpu_BIT_Func(4, cpu_Z); break;
							case 0x16E: cpu_BIT_Func(5, cpu_Z); break;
							case 0x176: cpu_BIT_Func(6, cpu_Z); break;
							case 0x17E: cpu_BIT_Func(7, cpu_Z); break;
						}
						break;
					case 6: cpu_Halt_Check(); break;
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
							case 0x196: cpu_RES_Func(3, cpu_Z); break;
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
					case 10: cpu_Halt_Check(); break;
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
					case 10: cpu_Halt_Check(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case OpT::EXIT:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
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

			case OpT::GBC_HALT:
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Halt_Ex(0); break;
				}
				break;

			case OpT::SPC_HALT:
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_Halt_Check(); break;
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
					case 2: cpu_Halt_Check(); break;
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
				break;

			case OpT::SPD_CHG:
				break;
		

			case 0x05:													// DEC B
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_DEC8_Func(cpu_B); break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case 0x07:													// RLCA
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_RLC_Func(cpu_Aim); break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case 0x0A:													// LD A, (BC)
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegBCget()); break;
					case 4: break;
					case 5: cpu_Regs[cpu_A] = cpu_Regs[cpu_Z]; break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;
		}
	}

	inline void cpu_Halt_Ex(uint8_t param)
	{
		halted = true;


		bool temp = false;

		if (instr_table[instr_pntr++] == 1)
		{
			temp = FlagI;
		}
		else
		{
			temp = I_use;
		}

		if (EI_pending > 0 && !CB_prefix)
		{
			EI_pending--;
			if (EI_pending == 0)
			{
				interrupts_enabled = true;
			}
		}

		// if the I flag is asserted at the time of halt, don't halt
		if (Halt_bug_5)
		{
			Halt_bug_5 = Halt_bug_3 = halted = skip_once = false;

			if (interrupts_enabled)
			{
				interrupts_enabled = false;

				TraceCallback ? .Invoke(new(disassembly: "====IRQ====", registerInfo : string.Empty));

				RegPC--;

				// TODO: If interrupt priotrity is checked differently in GBC, then this is incorrect
				// a new interrupt vector would be needed
				instr_pntr = 256 * 60 * 2 + 60 * 6; // point to Interrupt
			}
			else
			{
				TraceCallback ? .Invoke(new(disassembly: "====un-halted====", registerInfo : string.Empty));

				OnExecFetch ? .Invoke(RegPC);
				if (TraceCallback != null && !CB_prefix) TraceCallback(State(useRGBDSSyntax));
				CDLCallback ? .Invoke(RegPC, eCDLogMemFlags.FetchFirst);

				FetchInstruction(ReadMemory(RegPC));
			}
		}
		else if (temp && interrupts_enabled)
		{
			interrupts_enabled = false;

			TraceCallback ? .Invoke(new(disassembly: "====IRQ====", registerInfo : string.Empty));
			halted = false;

			if (Halt_bug_4)
			{
				// TODO: If interrupt priotrity is checked differently in GBC, then this is incorrect
				// a new interrupt vector would be needed
				DEC16_Func(PCl, PCh);
				instr_pntr = 256 * 60 * 2 + 60 * 6; // point to Interrupt
				Halt_bug_4 = false;
				skip_once = false;
				Halt_bug_3 = false;
			}
			else if (is_GBC)
			{
				// call the interrupt processor after 4 extra cycles
				if (!Halt_bug_3)
				{
					instr_pntr = 256 * 60 * 2 + 60 * 7; // point to Interrupt for GBC
				}
				else
				{
					// TODO: If interrupt priotrity is checked differently in GBC, then this is incorrect
					// a new interrupt vector would be needed
					instr_pntr = 256 * 60 * 2 + 60 * 6; // point to Interrupt
					Halt_bug_3 = false;
					//Console.WriteLine("Hit INT");
				}
			}
			else
			{
				// call interrupt processor
				instr_pntr = 256 * 60 * 2 + 60 * 6; // point to Interrupt
				Halt_bug_3 = false;
			}
		}
		else if (temp)
		{
			// even if interrupt servicing is disabled, any interrupt flag raised still resumes execution
			TraceCallback ? .Invoke(new(disassembly: "====un-halted====", registerInfo : string.Empty));
			halted = false;

			if (is_GBC)
			{
				// extra 4 cycles for GBC
				if (Halt_bug_3)
				{
					OnExecFetch ? .Invoke(RegPC);
					if (TraceCallback != null && !CB_prefix) TraceCallback(State(useRGBDSSyntax));
					CDLCallback ? .Invoke(RegPC, eCDLogMemFlags.FetchFirst);

					RegPC++;
					FetchInstruction(ReadMemory(RegPC));
					Halt_bug_3 = false;
					//Console.WriteLine("Hit un");
				}
				else
				{
					instr_pntr = 256 * 60 * 2 + 60; // exit halt loop
				}
			}
			else
			{
				OnExecFetch ? .Invoke(RegPC);
				if (TraceCallback != null && !CB_prefix) TraceCallback(State(useRGBDSSyntax));
				CDLCallback ? .Invoke(RegPC, eCDLogMemFlags.FetchFirst);

				if (Halt_bug_3)
				{
					//special variant of halt bug where RegPC also isn't incremented post fetch
					RegPC++;
					FetchInstruction(ReadMemory(RegPC));
					Halt_bug_3 = false;
				}
				else
				{
					FetchInstruction(ReadMemory(RegPC++));
				}
			}
		}
		else
		{
			if (skip_once)
			{
				instr_pntr = 256 * 60 * 2 + 60 * 2; // point to skipped loop
				skip_once = false;
			}
			else
			{
				if (is_GBC)
				{
					instr_pntr = 256 * 60 * 2 + 60 * 3; // point to GBC Halt loop
				}
				else
				{
					instr_pntr = 256 * 60 * 2 + 60 * 4; // point to spec Halt loop
				}
			}
		}
		I_use = false;
	}

	inline void cpu_STOP_Ex()
	{
		stopped = true;
		if (!stop_check)
		{
			// Z contains the second stop byte, not sure if it's useful at all
			stop_time = SpeedFunc(0);
			stop_check = true;
		}

		buttons_pressed = GetButtons(0);

		if (stop_time > 0)
		{
			// Timer interrupts can prematurely terminate a speedchange, not sure about other sources
			// NOTE: some testing around the edge case of where the speed actually changes is needed						
			if (I_use && interrupts_enabled)
			{
				interrupts_enabled = false;
				I_use = false;

				TraceCallback ? .Invoke(new(disassembly: "====un-stop====", registerInfo : string.Empty));

				stopped = false;
				stop_check = false;
				stop_time = 0;

				TraceCallback ? .Invoke(new(disassembly: "====IRQ====", registerInfo : string.Empty));

				// call interrupt processor 
				// lowest bit set is highest priority
				instr_pntr = 256 * 60 * 2 + 60 * 6; // point to Interrupt
				break;
			}

			if (stop_time == 32770)
			{
				// point to speed cange loop
				instr_pntr = 256 * 60 * 2 + 60 * 9;
				stop_time--;
				break;
			}

			stop_time--;

			if (stop_time == 0)
			{
				TraceCallback ? .Invoke(new(disassembly: "====un-stop====", registerInfo : string.Empty));

				stopped = false;

				// it takes the CPU 4 cycles longer to restart then the rest of the system.
				instr_pntr = 256 * 60 * 2 + 60;

				stop_check = false;

				break;
			}

			// If a button is pressed during speed change, the processor will jam
			if ((buttons_pressed & 0xF) != 0xF)
			{
				stop_time++;
				break;
			}
		}

		// Button press will exit stop loop even if speed change in progress, even without interrupts enabled
		if ((buttons_pressed & 0xF) != 0xF)
		{
			// TODO: On a gameboy, you can only un-STOP once, needs further testing
			TraceCallback ? .Invoke(new(disassembly: "====un-stop====", registerInfo : string.Empty));

			stopped = false;
			if (TraceCallback != null && !CB_prefix) TraceCallback(State());
			FetchInstruction(ReadMemory(RegPC++));

			stop_check = false;
		}
		else
		{
			instr_pntr = 256 * 60 * 2 + 60 * 5; // point to stop loop
		}
	}
}