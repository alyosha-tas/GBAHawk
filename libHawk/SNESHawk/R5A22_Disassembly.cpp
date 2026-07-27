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
		case 0x00:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BRK");
			}
			else
			{
				sprintf_s(val_char_2, 40, "BRK");
			}
			break;

		case 0x01:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0x05:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x06:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ASL $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ASL $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x08:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "PHP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "PHP");
			}
			break;

		case 0x09:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x0A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ASL A");
			}
			else
			{
				sprintf_s(val_char_2, 40, "ASL A");
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

		case 0x0D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x0E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ASL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ASL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x0F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x10:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BPL $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BPL $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0x11:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0x15:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x16:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ASL $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ASL $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x18:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CLC");
			}
			else
			{
				sprintf_s(val_char_2, 40, "CLC");
			}
			break;

		case 0x19:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x1A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			break;

		case 0x1B:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TCS");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TCS");
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

		case 0x1D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x1E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ASL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ASL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x1F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ORA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ORA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x20:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "JSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "JSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x21:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0x25:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x26:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROL $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROL $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x28:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "PLP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "PLP");
			}
			break;

		case 0x29:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x2A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROL A");
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROL A");
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

		case 0x2D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x2E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROL $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x2F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x30:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BMI $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BMI $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0x31:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0x35:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x36:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROL $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROL $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x38:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SEC");
			}
			else
			{
				sprintf_s(val_char_2, 40, "SEC");
			}
			break;

		case 0x39:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x3A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
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

		case 0x3D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x3E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROL $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x3F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AND $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "AND $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x40:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "RTI");
			}
			else
			{
				sprintf_s(val_char_2, 40, "RTI");
			}
			break;

		case 0x41:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0x45:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x46:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LSR $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LSR $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x48:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "PHA");
			}
			else
			{
				sprintf_s(val_char_2, 40, "PHA");
			}
			break;

		case 0x49:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x4A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LSR A");
			}
			else
			{
				sprintf_s(val_char_2, 40, "LSR A");
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

		case 0x4D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x4E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LSR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x4F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x50:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BVC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BVC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0x51:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0x55:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x56:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LSR $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LSR $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x58:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CLI");
			}
			else
			{
				sprintf_s(val_char_2, 40, "CLI");
			}
			break;

		case 0x59:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x5A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			break;

		case 0x5B:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TCD");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TCD");
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

		case 0x5D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x5E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LSR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LSR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x5F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "EOR $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "EOR $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x60:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "RTS");
			}
			else
			{
				sprintf_s(val_char_2, 40, "RTS");
			}
			break;

		case 0x61:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0x65:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x66:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROR $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROR $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x68:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "PLA");
			}
			else
			{
				sprintf_s(val_char_2, 40, "PLA");
			}
			break;

		case 0x69:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x6A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROR A");
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROR A");
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

		case 0x6D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x6E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROR $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x6F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x70:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BVS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BVS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0x71:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0x75:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x76:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROR $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROR $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x78:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SEI");
			}
			else
			{
				sprintf_s(val_char_2, 40, "SEI");
			}
			break;

		case 0x79:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x7A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
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

		case 0x7D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x7E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ROR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ROR $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x7F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "ADC $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "ADC $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x80:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x81:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0x85:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x86:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STX $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STX $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x88:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "DEY");
			}
			else
			{
				sprintf_s(val_char_2, 40, "DEY");
			}
			break;

		case 0x89:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x8A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TXA");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TXA");
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

		case 0x8D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x8E:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x8F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0x90:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BCC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BCC $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0x91:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA ($%02X),Y", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA ($%02X),Y", Sys_pntr->PeekMemory(pc++));
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

		case 0x95:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x96:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STX $%02X,Y", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "STX $%02X,Y", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0x98:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TYA");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TYA");
			}
			break;

		case 0x99:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%04X,Y", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x9A:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TXS");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TXS");
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

		case 0x9D:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0x9F:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "STA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "STA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0xA0:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDY #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDY #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xA1:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0xA5:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xA6:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDX $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDX $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xA8:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TAY");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TAY");
			}
			break;

		case 0xA9:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xAA:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TAX");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TAX");
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

		case 0xAD:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xAE:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDX $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xAF:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0xB0:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BCS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BCS $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0xB1:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0xB5:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xB6:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDX $%02X,Y", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDX $%02X,Y", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xB8:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CLV");
			}
			else
			{
				sprintf_s(val_char_2, 40, "CLV");
			}
			break;

		case 0xB9:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xBA:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "TXS");
			}
			else
			{
				sprintf_s(val_char_2, 40, "TXS");
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

		case 0xBD:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xBE:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDX $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDX $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xBF:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "LDA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "LDA $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0xC0:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CPY #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CPY #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xC1:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0xC5:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xC6:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "DEC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "DEC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xC8:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "INY");
			}
			else
			{
				sprintf_s(val_char_2, 40, "INY");
			}
			break;

		case 0xC9:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xCA:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "DEX");
			}
			else
			{
				sprintf_s(val_char_2, 40, "DEX");
			}
			break;

		case 0xCB:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "AXS $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "AXS $%02X", Sys_pntr->PeekMemory(pc++));
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

		case 0xCD:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xCE:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "DEC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "DEC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xCF:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0xD0:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BNE $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BNE $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0xD1:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0xD5:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xD6:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "DEC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "DEC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xD8:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CLD");
			}
			else
			{
				sprintf_s(val_char_2, 40, "CLD");
			}
			break;

		case 0xD9:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xDA:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
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

		case 0xDD:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xDE:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "DEC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "DEC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xDF:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CMP $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "CMP $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0xE0:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "CPX #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "CPX #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xE1:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC ($%02X,X)", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC ($%02X,X)", Sys_pntr->PeekMemory(pc++));
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

		case 0xE5:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xE6:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "INC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "INC $%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xE8:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "INX");
			}
			else
			{
				sprintf_s(val_char_2, 40, "INX");
			}
			break;

		case 0xE9:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC #$%02X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xEA:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
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

		case 0xED:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xEE:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "INC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "INC $%04X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xEF:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%06X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

		case 0xF0:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "BEQ $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "BEQ $%04X", Sys_pntr->Peek_Memory_8_Branch(pc++));
			}
			break;

		case 0xF1:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC ($%02X),Y *", Sys_pntr->PeekMemory(pc++));
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

		case 0xF5:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xF6:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "INC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			else
			{
				sprintf_s(val_char_2, 40, "INC $%02X,X", Sys_pntr->PeekMemory(pc++));
			}
			break;

		case 0xF8:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SED");
			}
			else
			{
				sprintf_s(val_char_2, 40, "SED");
			}
			break;

		case 0xF9:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%04X,Y *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xFA:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			else
			{
				sprintf_s(val_char_2, 40, "NOP");
			}
			break;

		case 0xFB:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "XCE");
			}
			else
			{
				sprintf_s(val_char_2, 40, "XCE");
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

		case 0xFD:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%04X,X *", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xFE:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "INC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "INC $%04X,X", Sys_pntr->Peek_Memory_16(pc++)); pc++;
			}
			break;

		case 0xFF:
			if (Flag_E || FlagMget())
			{
				sprintf_s(val_char_2, 40, "SBC $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			else
			{
				sprintf_s(val_char_2, 40, "SBC $%06X,X", Sys_pntr->Peek_Memory_24(pc++)); pc++; pc++;
			}
			break;

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
}