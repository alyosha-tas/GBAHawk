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
						alu_temp_16 = (alu_temp_hi << 8) | alu_temp;
						break;

					case CPU_Cycle_Type::Write_Cycle:
						Sys_pntr->WriteMemory(address_bus, value8);
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

					case CPU_Cycle_Type::Fetch_4:
						opcode4 = Sys_pntr->ReadMemory(address_bus);
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
		}

		TotalExecutedCycles++;
	}

	void R5A22::ALU_Operation()
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

			case ALU::REP:
				temp8 = 0;
				for (int i = 0; i < 8; i++)
				{
					if (((alu_temp >> i) & 1) == 1)
					{
						// these bits are reset
					}
					else
					{
						temp8 |= (((P >> i)& 1) << i);
					}
				}
				P = temp8;
				break;

			case ALU::SEP:
				temp8 = 0;
				for (int i = 0; i < 8; i++)
				{
					if (((alu_temp >> i) & 1) == 1)
					{
						// these bits are set
						temp8 |= (1 << i);
					}
					else
					{
						temp8 |= (((P >> i) & 1) << i);
					}
				}
				P = temp8;

				// register size changes automatically set 0's to high bytes
				if (FlagMget()) { A &= 0xFF; }
				if (FlagXget()) { X &= 0xFF; Y &= 0xFF; }

				break;

			case ALU::TSB:
				temp8 = 0;
				for (int i = 0; i < 8; i++)
				{
					if (((A >> i) & 1) == 1)
					{
						// these bits are reset
					}
					else
					{
						temp8 |= (((alu_temp >> i) & 1) << i);
					}
				}
				FlagZset(temp8 == 0);
				alu_temp_16 = temp8;
				break;

			case ALU::TRB:
				temp8 = 0;
				for (int i = 0; i < 8; i++)
				{
					if (((A >> i) & 1) == 1)
					{
						// these bits are set
						temp8 |= (1 << i);
					}
					else
					{
						temp8 |= (((alu_temp >> i) & 1) << i);
					}
				}
				FlagZset(temp8 == 0);
				alu_temp_16 = temp8;
				break;

			case ALU::XCE:
				booltemp = Flag_E;
				Flag_E = FlagCget();
				FlagCset(booltemp);

				if (Flag_E)
				{
					FlagMset(true);
					FlagXset(true);
				}
				break;

			case ALU::TCD:
				D = A;
				NZ_Set_16(A);
				break;

			case ALU::TCS:
				S = A;
				break;

			case ALU::TXS:
				S = X;
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

			case ALU::PLD:
				// shouldn't reach
				break;

			case ALU::PLB:
				DBR = alu_temp;
				NZ_Set(DBR);
				break;

			case ALU::PLX:
				X = alu_temp;
				NZ_Set(X);
				break;

			case ALU::PLY:
				Y = alu_temp;
				NZ_Set(Y);
				break;

			case ALU::NOP_16:
				break;

			case ALU::ORA_16:
				A |= alu_temp_16;
				NZ_Set_16(A);
				break;

			case ALU::AND_16:
				A &= alu_temp_16;
				NZ_Set_16(A);
				break;

			case ALU::EOR_16:
				A ^= alu_temp_16;
				NZ_Set_16(A);
				break;

			case ALU::LDA_16:
				A = alu_temp_16;
				NZ_Set_16(A);
				break;

			case ALU::LDX_16:
				X = alu_temp_16;
				NZ_Set_16(X);
				break;

			case ALU::LDY_16:
				Y = alu_temp_16;
				NZ_Set_16(Y);
				break;

			case ALU::ASR_16:
				A &= alu_temp_16;
				FlagCset((A & 0x1) == 0x1);
				A >>= 1;
				NZ_Set_16(A);
				break;

			case ALU::INX_16:
				X++;
				NZ_Set_16(X);
				break;

			case ALU::INY_16:
				Y++;
				NZ_Set_16(Y);
				break;

			case ALU::DEX_16:
				X--;
				NZ_Set_16(X);
				break;

			case ALU::DEY_16:
				Y--;
				NZ_Set_16(Y);
				break;

			case ALU::INC_16:
				alu_temp_16 = alu_temp_16 + 1;
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::DEC_16:
				alu_temp_16 = alu_temp_16 - 1;
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::ASL_16:
				FlagCset((alu_temp_16 & 0x8000) != 0);
				alu_temp_16 = (alu_temp_16 << 1);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::ROL_16:
				value16 = alu_temp_16;
				alu_temp_16 = ((alu_temp_16 << 1) | (P & 1));
				FlagCset((value16 & 0x8000) != 0);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::ROR_16:
				value16 = alu_temp_16;
				alu_temp_16 = ((alu_temp_16 >> 1) | ((P & 1) << 7));
				FlagCset((value16 & 1) != 0);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::LSR_16:
				FlagCset((alu_temp_16 & 1) != 0);
				alu_temp_16 = (alu_temp_16 >> 1);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::ASLA_16:
				FlagCset((A & 0x8000) != 0);
				A = (uint8_t)(A << 1);
				NZ_Set(A);
				break;

			case ALU::ROLA_16:
				value16 = A;
				A = ((A << 1) | (P & 1));
				FlagCset((value16 & 0x8000) != 0);
				NZ_Set_16(A);
				break;

			case ALU::RORA_16:
				value16 = A;
				A = ((A >> 1) | ((P & 1) << 7));
				FlagCset((value16 & 1) != 0);
				NZ_Set_16(A);
				break;

			case ALU::LSRA_16:
				FlagCset((A & 1) != 0);
				A = (A >> 1);
				NZ_Set_16(A);
				break;

			case ALU::TXS_16:
				S = X;
				break;

			case ALU::TSX_16:
				X = S;
				NZ_Set_16(X);
				break;

			case ALU::TAX_16:
				X = A;
				NZ_Set_16(X);
				break;

			case ALU::TAY_16:
				Y = A;
				NZ_Set_16(Y);
				break;

			case ALU::TYA_16:
				A = Y;
				NZ_Set_16(A);
				break;

			case ALU::TXA_16:
				A = X;
				NZ_Set_16(A);
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
				FlagNset((alu_temp_16 & 0x8000) != 0);
				FlagVset((alu_temp_16 & 0x4000) != 0);
				FlagZset((A & alu_temp_16) == 0);
				break;

			case ALU::CMP_16:
				value16 = alu_temp_16;
				alu_temp_16 = (A - value16);
				FlagCset(A >= value16);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::CPX_16:
				value16 = alu_temp_16;
				alu_temp_16 = (X - value16);
				FlagCset(X >= value16);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::CPY_16:
				value16 = alu_temp_16;
				alu_temp_16 = (Y - value16);
				FlagCset(Y >= value16);
				NZ_Set_16(alu_temp_16);
				break;

			case ALU::ADC_16:
				value16 = alu_temp_16;
				if (FlagDget())
				{
					// nibble 0
					tempint = (A & 0x0F) + (value16 & 0x0F) + (FlagCget() ? 0x01 : 0x00);
					if (tempint > 0x09)
						tempint += 0x06;
					// nibble 1
					tempint = (tempint & 0x0F) + (A & 0xF0) + (value16 & 0xF0) + (tempint > 0x0F ? 0x10 : 0x00);
					if ((tempint & 0x1F0) > 0x090)
						tempint += 0x060;
					// nibble 2
					tempint = (tempint & 0x0FF) + (A & 0xF00) + (value16 & 0xF00) + (tempint > 0x0FF ? 0x100 : 0x00);
					if ((tempint & 0x1F00) > 0x0900)
						tempint += 0x0600;
					// nibble 3
					tempint = (tempint & 0x0FFF) + (A & 0xF000) + (value16 & 0xF000) + (tempint > 0x0FFF ? 0x1000 : 0x00);			

					FlagVset((~(A ^ value16) & (A ^ tempint) & 0x8000) != 0);
					FlagZset(((A + value16 + (FlagCget() ? 1 : 0)) & 0xFFFF) == 0);
					FlagNset((tempint & 0x8000) != 0);

					if ((tempint & 0x1F000) > 0x09000)
						tempint += 0x06000;
					
					FlagCset(tempint > 0xFFFF);
					A = (uint16_t)(tempint & 0xFFFF);
				}
				else
				{
					tempint = value16 + A + (FlagCget() ? 1 : 0);
					FlagVset((~(A ^ value16) & (A ^ tempint) & 0x8000) != 0);
					FlagCset(tempint > 0xFFFF);
					A = (uint16_t)tempint;
					NZ_Set_16(A);
				}
				break;

			case ALU::SBC_16:
				value16 = (uint16_t)alu_temp_16;
				tempint = A - value16 - (FlagCget() ? 0 : 1);
				if (FlagDget())
				{
					lo = (A & 0x0F) - (value16 & 0x0F) - (FlagCget() ? 0 : 1);
					hi = (A & 0xF0) - (value16 & 0xF0);
					if ((lo & 0xF0) != 0) lo -= 0x06;
					if ((lo & 0x80) != 0) hi -= 0x10;
					if ((hi & 0x0F00) != 0) hi -= 0x60;
					FlagVset(((A ^ value16) & (A ^ tempint) & 0x8000) != 0);
					FlagZset((tempint & 0xFFFF) == 0);
					FlagNset((tempint & 0x8000) != 0);
					FlagCset((hi & 0xFF00) == 0);
					A = (uint8_t)((lo & 0x0F) | (hi & 0xF0));
				}
				else
				{
					FlagVset(((A ^ value16) & (A ^ tempint) & 0x8000) != 0);
					FlagCset(tempint >= 0);
					A = (uint16_t)tempint;
					value16 = (uint16_t)alu_temp_16;
				}
				break;

			case ALU::PLA_16:
				A = alu_temp_16;
				NZ_Set_16(A);
				break;

			case ALU::PLP_16:
				// shouldn't be reached
				break;

			case ALU::PLD_16:
				D = alu_temp_16;
				NZ_Set_16(D);
				break;

			case ALU::PLB_16:
				// shouldn't be reached
				break;

			case ALU::PLX_16:
				X = alu_temp_16;
				NZ_Set_16(X);
				break;

			case ALU::PLY_16:
				Y = alu_temp_16;
				NZ_Set_16(Y);
				break;

			default:
				throw exception("bad op");

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

		address_bus = get_PC_Addr();

		if (NMI)
		{
			if (TraceCallback) TraceCallback(1);
			ea = NMIVector;

			Instr_Type = OpT::INT;
			IRQ_Type = 0; // NMI
			NMI = false;
			return;
		}

		if (IRQ && !my_iflag)
		{
			if (TraceCallback) TraceCallback(2);
			ea = IRQVector;

			Instr_Type = OpT::INT;
			IRQ_Type = 1; // IRQ
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

	void R5A22::Fetch_Opcode_No_Interrupt()
	{
		Instr_Cycle = -1;

		address_bus = get_PC_Addr();

		OnExecFetch(PC);
		if (TraceCallback) TraceCallback(0);
		opcode = Sys_pntr->ReadMemory(address_bus);
		PC++;
		Decode(opcode);
	}
}