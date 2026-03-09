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
					case 3: cpu_STOP_Func(); break;
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
				switch (cpu_Instr_Cycle)
				{
					case 0: break;
					case 1: break;
					case 2: break;
					case 3: cpu_Regs[cpu_Z] = GB_System::Read_Memory(cpu_RegPCget()); break;
					case 4: cpu_RegHLset((uint16_t)(cpu_RegPCget() + 1)); break;
					case 5: break;
					case 6: cpu_Regs[cpu_W] = 0xFF; break;
					case 7: GB_System::Write_Memory(cpu_RegWZget(), cpu_Regs[cpu_A]);  break;
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
				break;

			case OpT::JP_HL:
				break;

			case OpT::LD_FF_IND_16:
				break;

			case OpT::LD_8_IND_FF:
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
		}
	}

	inline void cpu_STOP_Func()
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