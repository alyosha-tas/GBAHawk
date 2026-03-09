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

			case OpT::LD_IND_16:
				break;

			case OpT::LD_8_IND:
				break;

			case OpT::INC_16:
				break;

			case OpT::LD_IND_8_INC:
				break;

			case OpT::LD_R_IM:
				break;

			case OpT::ADD_16:
				break;

			case OpT::REG_OP_IND:
				break;

			case OpT::DEC_16:
				break;

			case OpT::STOP:
				break;

			case OpT::JR_COND:
				break;

			case OpT::LD_8_IND_INC:
				break;

			case OpT::LD_IND_8_INC_HL:
				break;

			case OpT::LD_8_IND_DEC:
				break;

			case OpT::INC_DEC_8_IND:
				break;

			case OpT::LD_8_IND_IND:
				break;

			case OpT::LD_IND_8_DEC_HL:
				break;

			case OpT::HALT:
				break;

			case OpT::RET_COND:
				break;

			case OpT::POP:
				break;

			case OpT::JP_COND:
				break;

			case OpT::CALL_COND:
				break;

			case OpT::PUSH:
				break;

			case OpT::REG_OP_IND_INC:
				break;

			case OpT::RST:
				break;

			case OpT::RET:
				break;

			case OpT::PREFIX:
				break;

			case OpT::JAM:
				break;

			case OpT::RETI:
				break;

			case OpT::LD_FF_IND_8:
				break;

			case OpT::LD_FFC_IND_8:
				break;

			case OpT::ADD_SP:
				break;

			case OpT::JP_HL:
				break;

			case OpT::LD_FF_IND_16:
				break;

			case OpT::LD_8_IND_FF:
				break;

			case OpT::LD_8_IND_FFC:
				break;

			case OpT::EI_DI:
				break;

			case OpT::LD_HL_SPn:
				break;

			case OpT::LD_SP_HL:
				break;

			case OpT::LD_16_IND_FF:
				break;

			case OpT::INT_OP_IND:
				break;

			case OpT::BIT_OP:
				break;

			case OpT::BIT_TE_IND:
				break;

			case OpT::BIT_OP_IND:
				break;

			case OpT::RESET:
				break;

			case OpT::EXIT:
				break;

			case OpT::SKIP:
				break;

			case OpT::GBC_HALT:
				break;

			case OpT::SPC_HALT:
				break;

			case OpT::STOP_LOOP:
				break;

			case OpT::INTRPT:
				break;

			case OpT::INTRPT_GBC:
				break;

			case OpT::WAIT:
				break;

			case OpT::SPD_CHG:
				break;
		
		
			case 0x00:													// NOP						
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;
			
			case 0x01:													// LD BC, nn
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_C] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));  break;
					case 6: break;
					case 7: cpu_Regs[cpu_B] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 8: break;
					case 9: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));  break;
					case 10: cpu_Halt_Check(); break;
					case 11: cpu_Op_Func(); break;
				}
				break;

			case 0x02:													// LD (BC), A
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: GB_System::Write_Memory(cpu_RegBCget(), cpu_Regs[cpu_A]); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case 0x03:													// INC BC
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_RegBCset((uint16_t)(cpu_RegBCget() + 1)); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
				}			
				break;
				
			case 0x04:													// INC B
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_INC8_Func(cpu_B); break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Op_Func(); break;
				}
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

			case 0x06:													// LD B, n
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_B] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
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

			case 0x08:													// LD (imm), SP
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

			case 0x09:													// ADD HL, BC
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_ADD16_Func(cpu_L, cpu_H, cpu_C, cpu_B); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
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

			case 0x0B:													// DEC BC
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_RegBCset((uint16_t)(cpu_RegBCget() - 1)); break;
					case 4: break;
					case 5: break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
				}				
				break;

			case 0x0C:													// INC C
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_INC8_Func(cpu_C); break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case 0x0D:													// DEC C
				switch (cpu_Instr_Cycle)
				{
					case 0: cpu_DEC8_Func(cpu_C); break;
					case 1: break;
					case 2: cpu_Halt_Check(); break;
					case 3: cpu_Op_Func(); break;
				}
				break;

			case 0x0E:													// LD C, n
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_C] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: break;
					case 5: cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 6: cpu_Halt_Check(); break;
					case 7: cpu_Op_Func(); break;
				}
				break;

			case 0x0F:													// RRCA
				break;
			case 0x10:													// STOP
				break;
			case 0x11:													// LD DE, nn
				break;
			case 0x12:													// LD (DE), A
				break;
			case 0x13:													// INC DE
				break;
			case 0x14:													// INC D
				break;
			case 0x15:													// DEC D
				break;
			case 0x16:													// LD D, n
				break;
			case 0x17:													// RLA
				break;
			case 0x18:													// JR, r8
				break;
			case 0x19:													// ADD HL, DE
				break;
			case 0x1A:													// LD A, (DE)
				break;
			case 0x1B:													// DEC DE
				break;
			case 0x1C:													// INC E
				break;
			case 0x1D:													// DEC E
				break;
			case 0x1E:													// LD E, n
				break;
			case 0x1F:													// RRA
				break;
			case 0x20:													// JR NZ, r8
				break;
			case 0x21:													// LD HL, nn
				break;
			case 0x22:													// LD (HL+), A
				break;
			case 0x23:													// INC HL
				break;
			case 0x24:													// INC H
				break;
			case 0x25:													// DEC H
				break;
			case 0x26:													// LD H, n
				break;
			case 0x27:													// DAA
				break;
			case 0x28:													// JR Z, r8
				break;
			case 0x29:													// ADD HL, HL
				break;
			case 0x2A:													// LD A, (HL+)
				break;
			case 0x2B:													// DEC HL
				break;
			case 0x2C:													// INC L
				break;
			case 0x2D:													// DEC L
				break;
			case 0x2E:													// LD L, n
				break;
			case 0x2F:													// CPL
				break;
			case 0x30:													// JR NC, r8
				break;
			case 0x31:													// LD SP, nn
				break;
			case 0x32:													// LD (HL-), A
				break;
			case 0x33:													// INC SP
				break;
			case 0x34:													// INC (HL)
				break;
			case 0x35:													// DEC (HL)
				break;
			case 0x36:													// LD (HL), n
				break;
			case 0x37:													// SCF
				break;
			case 0x38:													// JR C, r8
				break;
			case 0x39:													// ADD HL, SP
				break;
			case 0x3A:													// LD A, (HL-)
				break;
			case 0x3B:													// DEC SP
				break;
			case 0x3C:													// INC A
				break;
			case 0x3D:													// DEC A
				break;
			case 0x3E:													// LD A, n
				break;
			case 0x3F:													// CCF
				break;
			case 0x40:													// LD B, B
				break;
			case 0x41:													// LD B, C
				break;
			case 0x42:													// LD B, D
				break;
			case 0x43:													// LD B, E
				break;
			case 0x44:													// LD B, H
				break;
			case 0x45:													// LD B, L
				break;
			case 0x46:													// LD B, (HL)
				break;
			case 0x47:													// LD B, A
				break;
			case 0x48:													// LD C, B
				break;
			case 0x49:													// LD C, C
				break;
			case 0x4A:													// LD C, D
				break;
			case 0x4B:													// LD C, E
				break;
			case 0x4C													// LD C, H
				break;
			case 0x4D:													// LD C, L
				break;
			case 0x4E:													// LD C, (HL)
				break;
			case 0x4F:													// LD C, A
				break;
			case 0x50: REG_OP(TR, D, B);						break;// LD D, B
			case 0x51: REG_OP(TR, D, C);						break;// LD D, C
			case 0x52: REG_OP(TR, D, D);						break;// LD D, D
			case 0x53: REG_OP(TR, D, E);						break;// LD D, E
			case 0x54: REG_OP(TR, D, H);						break;// LD D, H
			case 0x55: REG_OP(TR, D, L);						break;// LD D, L
			case 0x56: REG_OP_IND(TR, D, L, H);					break;// LD D, (HL)
			case 0x57: REG_OP(TR, D, A);						break;// LD D, A
			case 0x58: REG_OP(TR, E, B);						break;// LD E, B
			case 0x59: REG_OP(TR, E, C);						break;// LD E, C
			case 0x5A: REG_OP(TR, E, D);						break;// LD E, D
			case 0x5B: REG_OP(TR, E, E);						break;// LD E, E
			case 0x5C: REG_OP(TR, E, H);						break;// LD E, H
			case 0x5D: REG_OP(TR, E, L);						break;// LD E, L
			case 0x5E: REG_OP_IND(TR, E, L, H);					break;// LD E, (HL)
			case 0x5F: REG_OP(TR, E, A);						break;// LD E, A
			case 0x60: REG_OP(TR, H, B);						break;// LD H, B
			case 0x61: REG_OP(TR, H, C);						break;// LD H, C
			case 0x62: REG_OP(TR, H, D);						break;// LD H, D
			case 0x63: REG_OP(TR, H, E);						break;// LD H, E
			case 0x64: REG_OP(TR, H, H);						break;// LD H, H
			case 0x65: REG_OP(TR, H, L);						break;// LD H, L
			case 0x66: REG_OP_IND(TR, H, L, H);					break;// LD H, (HL)
			case 0x67: REG_OP(TR, H, A);						break;// LD H, A
			case 0x68: REG_OP(TR, L, B);						break;// LD L, B
			case 0x69: REG_OP(TR, L, C);						break;// LD L, C
			case 0x6A: REG_OP(TR, L, D);						break;// LD L, D
			case 0x6B: REG_OP(TR, L, E);						break;// LD L, E
			case 0x6C: REG_OP(TR, L, H);						break;// LD L, H
			case 0x6D: REG_OP(TR, L, L);						break;// LD L, L
			case 0x6E: REG_OP_IND(TR, L, L, H);					break;// LD L, (HL)
			case 0x6F: REG_OP(TR, L, A);						break;// LD L, A
			case 0x70: LD_8_IND(L, H, B);						break;// LD (HL), B
			case 0x71: LD_8_IND(L, H, C);						break;// LD (HL), C
			case 0x72: LD_8_IND(L, H, D);						break;// LD (HL), D
			case 0x73: LD_8_IND(L, H, E);						break;// LD (HL), E
			case 0x74: LD_8_IND(L, H, H);						break;// LD (HL), H
			case 0x75: LD_8_IND(L, H, L);						break;// LD (HL), L
			case 0x76: HALT_();									break;// HALT
			case 0x77: LD_8_IND(L, H, A);						break;// LD (HL), A
			case 0x78: REG_OP(TR, A, B);						break;// LD A, B
			case 0x79: REG_OP(TR, A, C);						break;// LD A, C
			case 0x7A: REG_OP(TR, A, D);						break;// LD A, D
			case 0x7B: REG_OP(TR, A, E);						break;// LD A, E
			case 0x7C: REG_OP(TR, A, H);						break;// LD A, H
			case 0x7D: REG_OP(TR, A, L);						break;// LD A, L
			case 0x7E: REG_OP_IND(TR, A, L, H);					break;// LD A, (HL)
			case 0x7F: REG_OP(TR, A, A);						break;// LD A, A
			case 0x80: REG_OP(ADD8, A, B);						break;// ADD A, B
			case 0x81: REG_OP(ADD8, A, C);						break;// ADD A, C
			case 0x82: REG_OP(ADD8, A, D);						break;// ADD A, D
			case 0x83: REG_OP(ADD8, A, E);						break;// ADD A, E
			case 0x84: REG_OP(ADD8, A, H);						break;// ADD A, H
			case 0x85: REG_OP(ADD8, A, L);						break;// ADD A, L
			case 0x86: REG_OP_IND(ADD8, A, L, H);				break;// ADD A, (HL)
			case 0x87: REG_OP(ADD8, A, A);						break;// ADD A, A
			case 0x88: REG_OP(ADC8, A, B);						break;// ADC A, B
			case 0x89: REG_OP(ADC8, A, C);						break;// ADC A, C
			case 0x8A: REG_OP(ADC8, A, D);						break;// ADC A, D
			case 0x8B: REG_OP(ADC8, A, E);						break;// ADC A, E
			case 0x8C: REG_OP(ADC8, A, H);						break;// ADC A, H
			case 0x8D: REG_OP(ADC8, A, L);						break;// ADC A, L
			case 0x8E: REG_OP_IND(ADC8, A, L, H);				break;// ADC A, (HL)
			case 0x8F: REG_OP(ADC8, A, A);						break;// ADC A, A
			case 0x90: REG_OP(SUB8, A, B);						break;// SUB A, B
			case 0x91: REG_OP(SUB8, A, C);						break;// SUB A, C
			case 0x92: REG_OP(SUB8, A, D);						break;// SUB A, D
			case 0x93: REG_OP(SUB8, A, E);						break;// SUB A, E
			case 0x94: REG_OP(SUB8, A, H);						break;// SUB A, H
			case 0x95: REG_OP(SUB8, A, L);						break;// SUB A, L
			case 0x96: REG_OP_IND(SUB8, A, L, H);				break;// SUB A, (HL)
			case 0x97: REG_OP(SUB8, A, A);						break;// SUB A, A
			case 0x98: REG_OP(SBC8, A, B);						break;// SBC A, B
			case 0x99: REG_OP(SBC8, A, C);						break;// SBC A, C
			case 0x9A: REG_OP(SBC8, A, D);						break;// SBC A, D
			case 0x9B: REG_OP(SBC8, A, E);						break;// SBC A, E
			case 0x9C: REG_OP(SBC8, A, H);						break;// SBC A, H
			case 0x9D: REG_OP(SBC8, A, L);						break;// SBC A, L
			case 0x9E: REG_OP_IND(SBC8, A, L, H);				break;// SBC A, (HL)
			case 0x9F: REG_OP(SBC8, A, A);						break;// SBC A, A
			case 0xA0: REG_OP(AND8, A, B);						break;// AND A, B
			case 0xA1: REG_OP(AND8, A, C);						break;// AND A, C
			case 0xA2: REG_OP(AND8, A, D);						break;// AND A, D
			case 0xA3: REG_OP(AND8, A, E);						break;// AND A, E
			case 0xA4: REG_OP(AND8, A, H);						break;// AND A, H
			case 0xA5: REG_OP(AND8, A, L);						break;// AND A, L
			case 0xA6: REG_OP_IND(AND8, A, L, H);				break;// AND A, (HL)
			case 0xA7: REG_OP(AND8, A, A);						break;// AND A, A
			case 0xA8: REG_OP(XOR8, A, B);						break;// XOR A, B
			case 0xA9: REG_OP(XOR8, A, C);						break;// XOR A, C
			case 0xAA: REG_OP(XOR8, A, D);						break;// XOR A, D
			case 0xAB: REG_OP(XOR8, A, E);						break;// XOR A, E
			case 0xAC: REG_OP(XOR8, A, H);						break;// XOR A, H
			case 0xAD: REG_OP(XOR8, A, L);						break;// XOR A, L
			case 0xAE: REG_OP_IND(XOR8, A, L, H);				break;// XOR A, (HL)
			case 0xAF: REG_OP(XOR8, A, A);						break;// XOR A, A
			case 0xB0: REG_OP(OR8, A, B);						break;// OR A, B
			case 0xB1: REG_OP(OR8, A, C);						break;// OR A, C
			case 0xB2: REG_OP(OR8, A, D);						break;// OR A, D
			case 0xB3: REG_OP(OR8, A, E);						break;// OR A, E
			case 0xB4: REG_OP(OR8, A, H);						break;// OR A, H
			case 0xB5: REG_OP(OR8, A, L);						break;// OR A, L
			case 0xB6: REG_OP_IND(OR8, A, L, H);				break;// OR A, (HL)
			case 0xB7: REG_OP(OR8, A, A);						break;// OR A, A
			case 0xB8: REG_OP(CP8, A, B);						break;// CP A, B
			case 0xB9: REG_OP(CP8, A, C);						break;// CP A, C
			case 0xBA: REG_OP(CP8, A, D);						break;// CP A, D
			case 0xBB: REG_OP(CP8, A, E);						break;// CP A, E
			case 0xBC: REG_OP(CP8, A, H);						break;// CP A, H
			case 0xBD: REG_OP(CP8, A, L);						break;// CP A, L
			case 0xBE: REG_OP_IND(CP8, A, L, H);				break;// CP A, (HL)
			case 0xBF: REG_OP(CP8, A, A);						break;// CP A, A
			case 0xC0: RET_COND(FLAG_NZ);						break;// Ret NZ
			case 0xC1: POP_(C, B);								break;// POP BC
			case 0xC2: JP_COND(FLAG_NZ);						break;// JP NZ
			case 0xC3: JP_COND(ALWAYS_T);						break;// JP
			case 0xC4: CALL_COND(FLAG_NZ);						break;// CALL NZ
			case 0xC5: PUSH_(C, B);								break;// PUSH BC
			case 0xC6: REG_OP_IND_INC(ADD8, A, PCl, PCh);		break;// ADD A, n
			case 0xC7: RST_(0);									break;// RST 0
			case 0xC8: RET_COND(FLAG_Z);						break;// RET Z
			case 0xC9: RET_();									break;// RET
			case 0xCA: JP_COND(FLAG_Z);							break;// JP Z
			case 0xCB: PREFIX_();								break;// PREFIX
			case 0xCC: CALL_COND(FLAG_Z);						break;// CALL Z
			case 0xCD: CALL_COND(ALWAYS_T);						break;// CALL
			case 0xCE: REG_OP_IND_INC(ADC8, A, PCl, PCh);		break;// ADC A, n
			case 0xCF: RST_(0x08);								break;// RST 0x08
			case 0xD0: RET_COND(FLAG_NC);						break;// Ret NC
			case 0xD1: POP_(E, D);								break;// POP DE
			case 0xD2: JP_COND(FLAG_NC);						break;// JP NC
			case 0xD3: JAM_();									break;// JAM
			case 0xD4: CALL_COND(FLAG_NC);						break;// CALL NC
			case 0xD5: PUSH_(E, D);								break;// PUSH DE
			case 0xD6: REG_OP_IND_INC(SUB8, A, PCl, PCh);		break;// SUB A, n
			case 0xD7: RST_(0x10);								break;// RST 0x10
			case 0xD8: RET_COND(FLAG_C);						break;// RET C
			case 0xD9: RETI_();									break;// RETI
			case 0xDA: JP_COND(FLAG_C);							break;// JP C
			case 0xDB: JAM_();									break;// JAM
			case 0xDC: CALL_COND(FLAG_C);						break;// CALL C
			case 0xDD: JAM_();									break;// JAM
			case 0xDE: REG_OP_IND_INC(SBC8, A, PCl, PCh);		break;// SBC A, n
			case 0xDF: RST_(0x18);								break;// RST 0x18
			case 0xE0: LD_FF_IND_8(PCl, PCh, A);				break;// LD(n), A
			case 0xE1: POP_(L, H);								break;// POP HL
			case 0xE2: LD_FFC_IND_8(PCl, PCh, A);				break;// LD(C), A
			case 0xE3: JAM_();									break;// JAM
			case 0xE4: JAM_();                                  break;// JAM
			case 0xE5: PUSH_(L, H);								break;// PUSH HL
			case 0xE6: REG_OP_IND_INC(AND8, A, PCl, PCh);		break;// AND A, n
			case 0xE7: RST_(0x20);								break;// RST 0x20
			case 0xE8: ADD_SP();								break;// ADD SP,n
			case 0xE9: JP_HL();									break;// JP (HL)
			case 0xEA: LD_FF_IND_16(PCl, PCh, A);				break;// LD(nn), A
			case 0xEB: JAM_();									break;// JAM
			case 0xEC: JAM_();									break;// JAM
			case 0xED: JAM_();									break;// JAM
			case 0xEE: REG_OP_IND_INC(XOR8, A, PCl, PCh);		break;// XOR A, n
			case 0xEF: RST_(0x28);								break;// RST 0x28
			case 0xF0: LD_8_IND_FF(A, PCl, PCh);				break;// A, LD(n)
			case 0xF1: POP_(F, A);								break;// POP AF
			case 0xF2: LD_8_IND_FFC(A, PCl, PCh);				break;// A, LD(C)
			case 0xF3: DI_();									break;// DI
			case 0xF4: JAM_();									break;// JAM
			case 0xF5: PUSH_(F, A);								break;// PUSH AF
			case 0xF6: REG_OP_IND_INC(OR8, A, PCl, PCh);		break;// OR A, n
			case 0xF7: RST_(0x30);								break;// RST 0x30
			case 0xF8: LD_HL_SPn();								break;// LD HL, SP+n
			case 0xF9: LD_SP_HL();								break;// LD, SP, HL
			case 0xFA: LD_16_IND_FF(A, PCl, PCh);				break;// A, LD(nn)
			case 0xFB: EI_();									break;// EI
			case 0xFC: JAM_();									break;// JAM
			case 0xFD: JAM_();									break;// JAM
			case 0xFE: REG_OP_IND_INC(CP8, A, PCl, PCh);		break;// CP A, n
			case 0xFF: RST_(0x38);								break;// RST 0x38
		}
	}
}