#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "R5A22.h"
#include "SNES_System.h"

namespace SNESHawk
{
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

		sprintf_s(val_char_1, 7, "%06X", get_PC_Addr());
		trace_string.append(val_char_1, 6);
		trace_string.append(":  ");

		uint16_t dis_pc = PC;
		for (uint32_t i = 0; i < op_size; i++)
		{
			sprintf_s(val_char_1, 3, "%02X", Sys_pntr->PeekMemory(dis_pc++));

			trace_string.append(val_char_1, 2);
			trace_string.append(" ");
		}

		while (trace_string.length() < 22)
		{
			trace_string.append(" ");
		}

		trace_string.append(disasm);

		while (trace_string.length() < 43)
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
		sprintf_s(val_char_1, 5, "%04X", A);
		trace_string.append(val_char_1, 4);

		trace_string.append(" X:");
		sprintf_s(val_char_1, 5, "%04X", X);
		trace_string.append(val_char_1, 4);

		trace_string.append(" Y:");
		sprintf_s(val_char_1, 5, "%04X", Y);
		trace_string.append(val_char_1, 4);

		trace_string.append(" SP:");
		sprintf_s(val_char_1, 5, "%04X", S);
		trace_string.append(val_char_1, 4);

		trace_string.append(" D:");
		sprintf_s(val_char_1, 5, "%04X", D);
		trace_string.append(val_char_1, 4);

		trace_string.append(" PBR:");
		sprintf_s(val_char_1, 3, "%02X", PBR);
		trace_string.append(val_char_1, 2);

		trace_string.append(" DBR:");
		sprintf_s(val_char_1, 3, "%02X", DBR);
		trace_string.append(val_char_1, 2);

		trace_string.append(" P:");
		sprintf_s(val_char_1, 3, "%02X", P);
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

		while (trace_string.length() < 120)
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
			case 0x10: sprintf_s(val_char_2, 40, "BPL $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			case 0x20: sprintf_s(val_char_2, 40, "JSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x30: sprintf_s(val_char_2, 40, "BMI $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			case 0x40: sprintf_s(val_char_2, 40, "RTI"); break;
			case 0x50: sprintf_s(val_char_2, 40, "BVC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			case 0x60: sprintf_s(val_char_2, 40, "RTS"); break;
			case 0x70: sprintf_s(val_char_2, 40, "BVS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			case 0x80: sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x90: sprintf_s(val_char_2, 40, "BCC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			//case 0xA0: LDY # break;
			case 0xB0: sprintf_s(val_char_2, 40, "BCS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			//case 0xC0: CPY # break;
			case 0xD0: sprintf_s(val_char_2, 40, "BNE $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;
			//case 0xC0: CPX # break;
			case 0xF0: sprintf_s(val_char_2, 40, "BEQ $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++)); break;

			case 0x01: sprintf_s(val_char_2, 40, "ORA ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0x11: sprintf_s(val_char_2, 40, "ORA ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x21: sprintf_s(val_char_2, 40, "AND ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0x31: sprintf_s(val_char_2, 40, "AND ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x41: sprintf_s(val_char_2, 40, "EOR ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0x51: sprintf_s(val_char_2, 40, "EOR ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x61: sprintf_s(val_char_2, 40, "ADC ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0x71: sprintf_s(val_char_2, 40, "ADC ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x81: sprintf_s(val_char_2, 40, "STA ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0x91: sprintf_s(val_char_2, 40, "STA ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xA1: sprintf_s(val_char_2, 40, "LDA ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0xB1: sprintf_s(val_char_2, 40, "LDA ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xC1: sprintf_s(val_char_2, 40, "CMP ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0xD1: sprintf_s(val_char_2, 40, "CMP ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xE1: sprintf_s(val_char_2, 40, "SBC ($D,X+$%02X)", Sys_pntr->PeekMemory(pc++)); break;
			case 0xF1: sprintf_s(val_char_2, 40, "SBC ($D),Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;

			case 0x05: sprintf_s(val_char_2, 40, "ORA $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x15: sprintf_s(val_char_2, 40, "ORA $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x25: sprintf_s(val_char_2, 40, "AND $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x35: sprintf_s(val_char_2, 40, "AND $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x45: sprintf_s(val_char_2, 40, "EOR $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x55: sprintf_s(val_char_2, 40, "EOR $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x65: sprintf_s(val_char_2, 40, "ADC $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x75: sprintf_s(val_char_2, 40, "ADC $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x85: sprintf_s(val_char_2, 40, "STA $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x95: sprintf_s(val_char_2, 40, "STA $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xA5: sprintf_s(val_char_2, 40, "LDA $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xB5: sprintf_s(val_char_2, 40, "LDA $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xC5: sprintf_s(val_char_2, 40, "CMP $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xD5: sprintf_s(val_char_2, 40, "CMP $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xE5: sprintf_s(val_char_2, 40, "SBC $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xF5: sprintf_s(val_char_2, 40, "SBC $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;

			case 0x06: sprintf_s(val_char_2, 40, "ASL $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x16: sprintf_s(val_char_2, 40, "ASL $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x26: sprintf_s(val_char_2, 40, "ROL $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x36: sprintf_s(val_char_2, 40, "ROL $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x46: sprintf_s(val_char_2, 40, "LSR $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x56: sprintf_s(val_char_2, 40, "LSR $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x66: sprintf_s(val_char_2, 40, "ROR $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x76: sprintf_s(val_char_2, 40, "ROR $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x86: sprintf_s(val_char_2, 40, "STX $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0x96: sprintf_s(val_char_2, 40, "STX $D,Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xA6: sprintf_s(val_char_2, 40, "LDX $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xB6: sprintf_s(val_char_2, 40, "LDX $D,Y+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xC6: sprintf_s(val_char_2, 40, "DEC $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xD6: sprintf_s(val_char_2, 40, "DEC $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xE6: sprintf_s(val_char_2, 40, "INC $D+$%02X", Sys_pntr->PeekMemory(pc++)); break;
			case 0xF6: sprintf_s(val_char_2, 40, "INC $D,X+$%02X", Sys_pntr->PeekMemory(pc++)); break;

			case 0x07: sprintf_s(val_char_2, 40, "ORA [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0x17: sprintf_s(val_char_2, 40, "ORA [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0x27: sprintf_s(val_char_2, 40, "AND [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0x37: sprintf_s(val_char_2, 40, "AND [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0x47: sprintf_s(val_char_2, 40, "EOR [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0x57: sprintf_s(val_char_2, 40, "EOR [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0x67: sprintf_s(val_char_2, 40, "ADC [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0x77: sprintf_s(val_char_2, 40, "ADC [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0x87: sprintf_s(val_char_2, 40, "STA [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0x97: sprintf_s(val_char_2, 40, "STA [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0xA7: sprintf_s(val_char_2, 40, "LDA [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0xB7: sprintf_s(val_char_2, 40, "LDA [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0xC7: sprintf_s(val_char_2, 40, "CMP [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0xD7: sprintf_s(val_char_2, 40, "CMP [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;
			case 0xE7: sprintf_s(val_char_2, 40, "SBC [$D+$%02X]", Sys_pntr->PeekMemory(pc++)); break;
			case 0xF7: sprintf_s(val_char_2, 40, "SBC [$D+$%02X],Y", Sys_pntr->PeekMemory(pc++)); break;

			case 0x08: sprintf_s(val_char_2, 40, "PHP"); break;
			case 0x18: sprintf_s(val_char_2, 40, "CLC"); break;
			case 0x28: sprintf_s(val_char_2, 40, "PLP"); break;
			case 0x38: sprintf_s(val_char_2, 40, "SEC"); break;
			case 0x48: sprintf_s(val_char_2, 40, "PHA"); break;
			case 0x58: sprintf_s(val_char_2, 40, "CLI"); break;
			case 0x68: sprintf_s(val_char_2, 40, "PLA"); break;
			case 0x78: sprintf_s(val_char_2, 40, "SEI"); break;
			case 0x88: sprintf_s(val_char_2, 40, "DEY"); break;
			case 0x98: sprintf_s(val_char_2, 40, "TYA"); break;
			case 0xA8: sprintf_s(val_char_2, 40, "TAY"); break;
			case 0xB8: sprintf_s(val_char_2, 40, "CLV"); break;
			case 0xC8: sprintf_s(val_char_2, 40, "INY"); break;
			case 0xD8: sprintf_s(val_char_2, 40, "CLD"); break;
			case 0xE8: sprintf_s(val_char_2, 40, "INX"); break;
			case 0xF8: sprintf_s(val_char_2, 40, "SED"); break;

			case 0x19: sprintf_s(val_char_2, 40, "ORA $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x39: sprintf_s(val_char_2, 40, "AND $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x59: sprintf_s(val_char_2, 40, "EOR $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x79: sprintf_s(val_char_2, 40, "ADC $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x99: sprintf_s(val_char_2, 40, "STA $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xB9: sprintf_s(val_char_2, 40, "LDA $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xD9: sprintf_s(val_char_2, 40, "CMP $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xF9: sprintf_s(val_char_2, 40, "SBC $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;

			case 0x0A: sprintf_s(val_char_2, 40, "ASL A"); break;
			case 0x1A: sprintf_s(val_char_2, 40, "INC A"); break;
			case 0x2A: sprintf_s(val_char_2, 40, "ROL A"); break;
			case 0x3A: sprintf_s(val_char_2, 40, "DEC A"); break;
			case 0x4A: sprintf_s(val_char_2, 40, "LSR A"); break;
			case 0x5A: sprintf_s(val_char_2, 40, "PHY"); break;
			case 0x6A: sprintf_s(val_char_2, 40, "ROR A"); break;
			case 0x7A: sprintf_s(val_char_2, 40, "PLY"); break;
			case 0x8A: sprintf_s(val_char_2, 40, "TXA"); break;
			case 0x9A: sprintf_s(val_char_2, 40, "TXS"); break;
			case 0xAA: sprintf_s(val_char_2, 40, "TAX"); break;
			case 0xBA: sprintf_s(val_char_2, 40, "TXS"); break;
			case 0xCA: sprintf_s(val_char_2, 40, "DEX"); break;
			case 0xDA: sprintf_s(val_char_2, 40, "PHX"); break;
			case 0xEA: sprintf_s(val_char_2, 40, "NOP"); break;
			case 0xFA: sprintf_s(val_char_2, 40, "PLX"); break;

			case 0x0B: sprintf_s(val_char_2, 40, "PHD"); break;
			case 0x1B: sprintf_s(val_char_2, 40, "TCS"); break;
			case 0x2B: sprintf_s(val_char_2, 40, "PLD"); break;
			case 0x3B: sprintf_s(val_char_2, 40, "TSC"); break;
			case 0x4B: sprintf_s(val_char_2, 40, "PHK"); break;
			case 0x5B: sprintf_s(val_char_2, 40, "TCD"); break;
			case 0x6B: sprintf_s(val_char_2, 40, "RTL"); break;
			case 0x7B: sprintf_s(val_char_2, 40, "TDC"); break;
			case 0x8B: sprintf_s(val_char_2, 40, "PHB"); break;
			case 0x9B: sprintf_s(val_char_2, 40, "TXY"); break;
			case 0xAB: sprintf_s(val_char_2, 40, "PLB"); break;
			case 0xBB: sprintf_s(val_char_2, 40, "TYX"); break;
			case 0xCB: sprintf_s(val_char_2, 40, "WAI"); break;
			case 0xDB: sprintf_s(val_char_2, 40, "STP"); break;
			case 0xEB: sprintf_s(val_char_2, 40, "XBA"); break;
			case 0xFB: sprintf_s(val_char_2, 40, "XCE"); break;

			case 0x0D: sprintf_s(val_char_2, 40, "ORA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x1D: sprintf_s(val_char_2, 40, "ORA $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x2D: sprintf_s(val_char_2, 40, "AND $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x3D: sprintf_s(val_char_2, 40, "AND $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x4D: sprintf_s(val_char_2, 40, "EOR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x5D: sprintf_s(val_char_2, 40, "EOR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x6D: sprintf_s(val_char_2, 40, "ADC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x7D: sprintf_s(val_char_2, 40, "ADC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x8D: sprintf_s(val_char_2, 40, "STA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x9D: sprintf_s(val_char_2, 40, "STA $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xAD: sprintf_s(val_char_2, 40, "LDA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xBD: sprintf_s(val_char_2, 40, "LDA $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xCD: sprintf_s(val_char_2, 40, "CMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xDD: sprintf_s(val_char_2, 40, "CMP $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xED: sprintf_s(val_char_2, 40, "SBC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xFD: sprintf_s(val_char_2, 40, "SBC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;

			case 0x0E: sprintf_s(val_char_2, 40, "ASL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x1E: sprintf_s(val_char_2, 40, "ASL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x2E: sprintf_s(val_char_2, 40, "ROL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x3E: sprintf_s(val_char_2, 40, "ROL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x4E: sprintf_s(val_char_2, 40, "LSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x5E: sprintf_s(val_char_2, 40, "LSR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x6E: sprintf_s(val_char_2, 40, "ROR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x7E: sprintf_s(val_char_2, 40, "ROR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x8E: sprintf_s(val_char_2, 40, "STX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0x9E: sprintf_s(val_char_2, 40, "STZ $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xAE: sprintf_s(val_char_2, 40, "LDX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xBE: sprintf_s(val_char_2, 40, "LDX $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xCE: sprintf_s(val_char_2, 40, "DEC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xDE: sprintf_s(val_char_2, 40, "DEC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xEE: sprintf_s(val_char_2, 40, "INC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
			case 0xFE: sprintf_s(val_char_2, 40, "INC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;

			case 0x0F: sprintf_s(val_char_2, 40, "ORA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x1F: sprintf_s(val_char_2, 40, "ORA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x2F: sprintf_s(val_char_2, 40, "AND $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x3F: sprintf_s(val_char_2, 40, "AND $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x4F: sprintf_s(val_char_2, 40, "EOR $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x5F: sprintf_s(val_char_2, 40, "EOR $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x6F: sprintf_s(val_char_2, 40, "ADC $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x7F: sprintf_s(val_char_2, 40, "ADC $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x8F: sprintf_s(val_char_2, 40, "STA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0x9F: sprintf_s(val_char_2, 40, "STA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0xAF: sprintf_s(val_char_2, 40, "LDA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0xBF: sprintf_s(val_char_2, 40, "LDA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0xCF: sprintf_s(val_char_2, 40, "CMP $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0xDF: sprintf_s(val_char_2, 40, "CMP $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0xEF: sprintf_s(val_char_2, 40, "SBC $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;
			case 0xFF: sprintf_s(val_char_2, 40, "SBC $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++; break;


			// these opcodes all depend on the processor state
			case 0xA0:
				if (Flag_E || FlagXget())
				{
					sprintf_s(val_char_2, 40, "LDY #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "LDY #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0xC0:
				if (Flag_E || FlagXget())
				{
					sprintf_s(val_char_2, 40, "CPY #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "CPY #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0xE0:
				if (Flag_E || FlagXget())
				{
					sprintf_s(val_char_2, 40, "CPX #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "CPX #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0x09:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "ORA #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "ORA #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0x29:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "AND #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "AND #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0x49:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "EOR #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "EOR #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0x69:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "ADC #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "ADC #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0x89:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "BIT #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "BIT #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0xA9:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "LDA #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "LDA #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0xC9:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "CMP #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "CMP #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;

			case 0xE9:
				if (Flag_E || FlagMget())
				{
					sprintf_s(val_char_2, 40, "SBC #$%02X", Sys_pntr->PeekMemory(pc++));
				}
				else
				{
					sprintf_s(val_char_2, 40, "SBC #$%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++; break;
				}
				break;





		case 0x04:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;



		case 0x0C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%04X)", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%04X)", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x14:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x1C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x24:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BIT $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BIT $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x2C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BIT $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "BIT $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x34:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x3C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x44:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x4C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "JMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "JMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x54:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x5C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x64:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x6C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "JMP ($%04X)", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "JMP ($%04X)", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x74:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x7C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x82:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x84:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STY $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STY $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x8C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x94:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STY $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STY $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;



		case 0x9C:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STZ $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STZ $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;



		case 0xA2:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDX #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDX #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xA4:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDY $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDY $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xAC:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xB3:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LAX ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LAX ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xB4:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDY $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDY $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;



		case 0xBC:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDY $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDY $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xC2:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "REP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "REP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xC4:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CPY $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CPY $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xCC:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CPY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CPY $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;



		case 0xD4:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;



		case 0xDC:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xE2:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xE4:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CPX $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CPX $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;



		case 0xEC:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CPX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CPX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xF4:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xFC:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			break;
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
}