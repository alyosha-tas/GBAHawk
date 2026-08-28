#pragma once

#ifndef APU_H
#define APU_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>
#include <inttypes.h>
#include <cmath>

#include "../Common/Savestate.h"
#include "../Common/Common_Ops.h"

#ifndef _WIN32
#define sprintf_s snprintf
#endif

using namespace std;

// Notes:

/*
*	contains the APU as well as the SPC700 cpu compoment
*
*/

//Message_String = "Uop " + to_string((int)uop) + " cyc: " + to_string(TotalExecutedCycles);

//MessageCallback(Message_String.length());

namespace SNESHawk
{
	class APU
	{
	public:

	#pragma region functions and general variables
		// external core pointers
		uint32_t* Core_status_sl = nullptr;

		// IPL loaded with core
		uint8_t IPL[0x40] = { };

		uint8_t Echo_Value;

		uint16_t Echo_Addr;

		uint32_t Audio_Sample_Clock;
		uint32_t Audio_Num_Samples;

		uint8_t RAM[0x10000] = { };

		uint64_t FrameCycle;

		int32_t Audio_Samples[25000] = {};


		void HardReset()
		{
			// fill initial RAm according to fullsnes
			int i = 0;

			while (i < 65536)
			{
				for (int j = 0; j < 32; j++)
				{
					RAM[i] = 0xFF;
					i++;
				}

				for (int k = 0; k < 32; k++)
				{
					RAM[i] = 0;
					i++;
				}
			}
			
			cpu_Reset();
		}

		void SoftReset()
		{
			cpu_SoftReset();
		}

		uint8_t ReadMemory(uint32_t addr);

		uint8_t ReadMemoryDMA(uint32_t addr);

		uint8_t DummyReadMemory(uint32_t addr);

		void OnExecFetch(uint16_t addr);

		void WriteMemory(uint32_t addr, uint8_t value);

		uint8_t PeekMemory(uint32_t addr);

		uint16_t Peek_Memory_16(uint32_t addr);

		uint16_t Peek_Memory_8_Branch(uint32_t addr);

		uint8_t read_joyport(uint32_t addr);

		void write_joyport(uint8_t value);

		uint8_t peek_joyport(uint32_t addr);

		uint8_t PeekReg(uint32_t addr);

		uint8_t ReadReg(uint32_t addr);

		void WriteReg(uint32_t addr, uint8_t value);
	#pragma endregion

	#pragma region APU savestate
		uint8_t* apu_SaveState(uint8_t* saver)
		{
			saver = byte_saver(Echo_Value, saver);
			
			saver = short_saver(Echo_Addr, saver);

			saver = int_saver(Audio_Sample_Clock, saver);
			saver = int_saver(Audio_Num_Samples, saver);
			
			saver = long_saver(FrameCycle, saver);
			
			saver = byte_array_saver(RAM, saver, 0x10000);

			saver = cpu_SaveState(saver);

			return saver;
		}

		uint8_t* apu_LoadState(uint8_t* loader)
		{
			loader = byte_loader(&Echo_Value, loader);
			
			loader = short_loader(&Echo_Addr, loader);

			loader = int_loader(&Audio_Sample_Clock, loader);
			loader = int_loader(&Audio_Num_Samples, loader);
			
			loader = long_loader(&FrameCycle, loader);
			
			loader = byte_array_loader(RAM, loader, 0x10000);

			loader = cpu_LoadState(loader);

			return loader;
		}

	#pragma endregion

	#pragma region SPC700
	#pragma region Variables

		uint16_t Instr_Cycle;
		uint16_t IRQ_Type;

		uint32_t Instr_Type_Save;
		uint32_t ALU_Type_Save;

		bool BCD_Enabled = false;
		bool debug = false;
		bool IRQ;
		bool NMI;
		bool RDY;
		bool IRQ_Br;
		bool NMI_Br;

		uint8_t A;
		uint8_t X;
		uint8_t Y;
		uint8_t P;
		uint16_t PC;
		uint16_t D;
		uint16_t S;
		uint16_t Index_Add;

		uint64_t TotalExecutedCycles;
		uint64_t Total_CPU_Clock_Cycles;

		bool iflag_pending;
		bool RDY_Freeze;
		bool branch_irq_hack;
		bool cpu_First_Check;

		uint8_t opcode2, opcode3, opcode4;
		uint8_t H;

		uint16_t address_bus;

		uint32_t opcode;
		uint32_t ea, alu_temp, alu_temp_hi, alu_temp_16;

		uint32_t Fetch_Cnt, Fetch_Wait, Fetch_Op;

		uint8_t value8, temp8;
		uint16_t value16;
		bool branch_taken = false;
		bool my_iflag;
		bool booltemp;
		int32_t tempint;
		uint32_t lo, hi;

		const static uint8_t AneConstant = 0xFF;
		const static uint8_t LxaConstant = 0xFF;

		const static uint16_t NMIVector = 0xFFFA;
		const static uint16_t ResetVector = 0xFFFC;
		const static uint16_t BRKVector = 0xFFFE;
		const static uint16_t IRQVector = 0xFFFE;

		void cpu_Reset()
		{
			A = 0;
			X = 0;
			Y = 0;
			P = 0x20; // 5th bit always set
			S = 0x1FF;
			PC = 0;
			D = 0;
			TotalExecutedCycles = 0;

			Instr_Type = OpT::DRMI;
			IRQ_Type = 2;
			ALU_Type = ALU::NOP;
			Instr_Cycle = 0;

			opcode = 0;
			iflag_pending = true;
			RDY = true;
			BCD_Enabled = false;
			cpu_First_Check = false;

			IRQ = false;
			NMI = false;

			IRQ_Br = false;
			NMI_Br = false;

			Fetch_Cnt = 0;
			Fetch_Wait = 2;
			Fetch_Op = 1;
		}

		void cpu_SoftReset()
		{
			Instr_Type = OpT::DRMI;
			IRQ_Type = 2;
			ALU_Type = ALU::NOP;
			Instr_Cycle = 0;
			D = 0;

			opcode = 0;

			iflag_pending = true;
			FlagIset(true);

			Fetch_Cnt = 0;
			Fetch_Wait = 2;
			Fetch_Op = 1;
		}

		inline bool FlagCget() { return (P & 0x01) != 0; }
		inline void FlagCset(bool value) { P = (uint8_t)((P & ~0x01) | (value ? 0x01 : 0x00)); }

		inline bool FlagZget() { return (P & 0x02) != 0; }
		inline void FlagZset(bool value) { P = (uint8_t)((P & ~0x02) | (value ? 0x02 : 0x00)); }

		inline bool FlagIget() { return (P & 0x04) != 0; }
		inline void FlagIset(bool value) { P = (uint8_t)((P & ~0x04) | (value ? 0x04 : 0x00)); }

		inline bool FlagDget() { return (P & 0x08) != 0; }
		inline void FlagDset(bool value) { P = (uint8_t)((P & ~0x08) | (value ? 0x08 : 0x00)); }

		inline bool FlagBget() { return (P & 0x10) != 0; }
		inline void FlagBset(bool value) { P = (uint8_t)((P & ~0x10) | (value ? 0x10 : 0x00)); }

		inline bool FlagPget() { return (P & 0x20) != 0; }
		inline void FlagPset(bool value) { P = (uint8_t)((P & ~0x20) | (value ? 0x20 : 0x00)); }

		inline bool FlagVget() { return (P & 0x40) != 0; }
		inline void FlagVset(bool value) { P = (uint8_t)((P & ~0x40) | (value ? 0x40 : 0x00)); }

		inline bool FlagNget() { return (P & 0x80) != 0; }
		inline void FlagNset(bool value) { P = (uint8_t)((P & ~0x80) | (value ? 0x80 : 0x00)); }

		// SO pin
		inline void SetOverflow() { FlagVset(true); }

	#pragma endregion

	#pragma region Constant Declarations
		enum class OpT
		{
			Br,			// Branch
			BrB,		// Branch on Bit State
			JSR,		// JSR
			JMP,		// Jump
			JMPI,		// Jump Indirect
			Imp,		// Implied
			Imp3,		// Implies (3-cycle)
			Imm,		// Immediate
			ImmD,		// Immediate to direct
			Acc,		// Accumulator
			PL,			// Pull
			PH,			// Push
			RTS,		// RTS
			RTI,		// RTI
			CSI,		// CLI, SEI
			BRK,		// Break
			TCALL,		// Call

			// int value 13
			AbsR,		// [absolute READ]
			AbsW,		// [absolute Write]
			AbsRW,		// [absolute RMW]

			AdXR,		// (addr,X) [indexed indirect READ]
			AdXW,		// (addr,X) [indexed indirect WRITE]
			AdXRW,		// (addr,X) [indexed indirect RMW]

			IIYR,		// (addr),Y* [indirect indexed READ]
			IIYW,		// (addr),Y* [indirect indexed WRITE]
			IIYRW,		// (addr),Y* [indirect indexed RMW]

			DIXR,		// (D,X) [Direct,X indirect READ]
			DIXW,		// (D,X) [Direct,X indirect WRITE]

			DIIYR,		// (D),Y [indirect indexed READ]
			DIIYW,		// (D),Y [indirect indexed WRITE]

			DPR,		// [Direct page READ]
			DPW,		// [Direct page WRITE]
			DPRW,		// [Direct page RMW]

			DPXR,		// D,X [Direct page indexed READ X]
			DPYR,		// D,Y [Direct page indexed READ Y]
			DPXW,		// D,X [Direct page indexed WRITE X]
			DPYW,		// D,Y [Direct page indexed WRITE Y]
			DPXRW,		// D,X [Direct page indexed RMW]

			AIXR,		// addr,X [absolute indexed READ X]
			AIYR,		// addr,Y [absolute indexed READ Y]
			AIXW,		// addr,X [absolute indexed WRITE X]
			AIYW,		// addr,Y [absolute indexed WRITE Y]
			AIXRW,		// addr,X [absolute indexed RMW X]
			AIYRW,		// addr,Y [absolute indexed RMW Y]

			IXR,		// (X) [Indirect X READ]
			IXW,		// (X) [Indirect X Write]

			DDS,		// Direct page to Direct page
			IDXY,		// Indirect page to indirect page
			BitC,		// Individual BIT ops with C or memory

			Jam,		// Jam
			STP,		// Stop
			RESET,		// Reset
			DRMI,		// Dummy reads for interrupts
			FONI		// Fetch opcode no interrupts
		};

		OpT Instr_Type;

		OpT Instr_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::DDS  , OpT::BitC , OpT::DPRW , OpT::AbsRW, OpT::PH   , OpT::AbsRW, OpT::BRK  ,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::ImmD , OpT::IDXY , OpT::Acc  , OpT::DPXRW, OpT::Acc  , OpT::Imp  , OpT::AIXRW, OpT::AIXRW,
			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::DDS  , OpT::BitC , OpT::DPR  , OpT::AbsRW, OpT::PH   , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::ImmD , OpT::IDXY , OpT::Acc  , OpT::DPXRW, OpT::Acc  , OpT::Imp  , OpT::DPR  , OpT::AIXRW,

			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::DDS  , OpT::BitC , OpT::DPRW , OpT::AbsRW, OpT::PH   , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::ImmD , OpT::IDXY , OpT::Acc  , OpT::DPXRW, OpT::Acc  , OpT::Imp  , OpT::AIXRW, OpT::AIXRW,
			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::DDS  , OpT::BitC , OpT::DPRW , OpT::AbsRW, OpT::PH   , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::ImmD , OpT::IDXY , OpT::Acc  , OpT::DPXRW, OpT::Acc  , OpT::Imp  , OpT::DPR  , OpT::AIXRW,

			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::DDS  , OpT::BitC , OpT::DPRW , OpT::AbsRW, OpT::Imm  , OpT::PL   , OpT::ImmD ,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::ImmD , OpT::IDXY , OpT::Imp  , OpT::DPXRW, OpT::Acc  , OpT::Imp  , OpT::AbsRW, OpT::AbsRW,
			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::DDS  , OpT::BitC , OpT::DPRW , OpT::AbsRW, OpT::Imm  , OpT::PL   , OpT::AbsR ,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::ImmD , OpT::IDXY , OpT::Imp  , OpT::DPXRW, OpT::Acc  , OpT::Imp  , OpT::AIYR , OpT::AIYR ,

			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPW  , OpT::AbsW , OpT::IXW  , OpT::DIXW , OpT::Imm  , OpT::AbsW , OpT::BitC , OpT::DPW  , OpT::AbsW , OpT::Imm  , OpT::PL   , OpT::AbsRW,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXW , OpT::AIXW , OpT::AIYW , OpT::DIIYW, OpT::DPW  , OpT::DPYW , OpT::Imp  , OpT::DPXW , OpT::Imp  , OpT::Imp  , OpT::AIXRW, OpT::AIXRW,
			OpT::Imp  , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPR  , OpT::AbsR , OpT::IXR  , OpT::DIXR , OpT::Imm  , OpT::AbsR , OpT::BitC , OpT::DPR  , OpT::AbsR , OpT::Imp3 , OpT::PL   , OpT::STP  ,
			OpT::Br   , OpT::TCALL, OpT::DPRW , OpT::BrB  , OpT::DPXR , OpT::AIXR , OpT::AIYR , OpT::DIIYR, OpT::DPR  , OpT::DPYR , OpT::DDS  , OpT::DPXR , OpT::Imp  , OpT::Imp  , OpT::AIXRW, OpT::STP  ,
		};


		enum class CPU_Cycle_Type
		{
			Read_Cycle,
			Read_Cycle_Hi,
			Write_Cycle,
			Fetch_ALU_Cycle,
			Fetch_Cycle,
			Fetch_Cycle_No_Check,
			Fetch_Cycle_No_Int,
			Fetch_2,
			Fetch_3,
			Fetch_4,
			Internal_Cycle,
			Fetch_Reset,
		};

		CPU_Cycle_Type Cycle_Type;


		enum class ALU
		{
			// regular ops
			NOP, CLP, SEP, SEC, SEI, CLC, CLI, BIT, SETB, CLRB, AND, EOR, ORA, ADC, CMP, CPY, CPX, ASL, SBC, ROL,
			LSR, ASR, ROR, RRA, DEX, DEY, TXA, TYA, TXS, TAY, TAX, CLV, TSX, DEC, INY, CLD, INC, INX, NOTC,

			// int value 33
			// A implied
			ASLA, ROLA, LSRA, RORA, INCA, DECA,

			// unofficial
			LAX, SLO, ANC, RLA, SRE, ARR, SAX, ANE, SHA, SHS, SHY, SHX, LXA,
			LAS, DCP, AXS, ISC, SED,

			// Branch conditions
			BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ,

			// push pull op
			PLP, PLA, PHA, PHP, PHX, PLX, PHY, PLY,

			// loads / stores
			STA, STX, STY, LDA, LDX, LDY,

			// ALU ops on immediate data not stored in A
			ORv, ANDv, EORv, CMPv, ADCv, SBCv, LDv,

		};

		ALU ALU_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			ALU::NOP  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORv  , ALU::NOP  , ALU::ASL  , ALU::ASL  , ALU::PHP  , ALU::ASL  , ALU::NOP  ,
			ALU::BPL  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORv  , ALU::NOP  , ALU::ASL  , ALU::ASLA , ALU::DEX  , ALU::ASL  , ALU::SLO  ,
			ALU::CLP  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::ANDv , ALU::NOP  , ALU::ROL  , ALU::ROL  , ALU::PHA  , ALU::ROL  , ALU::RLA  ,
			ALU::BMI  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::ANDv , ALU::NOP  , ALU::ROL  , ALU::ROLA , ALU::INX  , ALU::CPX  , ALU::RLA  ,

			ALU::SEP  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EORv , ALU::NOP  , ALU::LSR  , ALU::LSR  , ALU::EOR  , ALU::LSR  , ALU::SRE  ,
			ALU::BVC  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EORv , ALU::NOP  , ALU::LSR  , ALU::LSRA , ALU::PHX  , ALU::LSR  , ALU::SRE  ,
			ALU::CLC  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMPv , ALU::NOP  , ALU::ROR  , ALU::ROR  , ALU::ADC  , ALU::ROR  , ALU::RRA  ,
			ALU::BVS  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMPv , ALU::NOP  , ALU::ROR  , ALU::RORA , ALU::PHY  , ALU::CPY  , ALU::RRA  ,

			ALU::SEC  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADCv , ALU::NOP  , ALU::DEC  , ALU::DEC  , ALU::LDY  , ALU::PLP  , ALU::LDv  ,
			ALU::BCC  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADCv , ALU::TXS  , ALU::DEC  , ALU::DECA , ALU::TSX  , ALU::SHX  , ALU::SHA  ,
			ALU::SEI  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBCv , ALU::NOP  , ALU::INC  , ALU::INC  , ALU::CPY  , ALU::PLA  , ALU::LAX  ,
			ALU::BCS  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBCv , ALU::TSX  , ALU::INC  , ALU::INCA , ALU::TXS  , ALU::LDX  , ALU::LAX  ,

			ALU::CLI  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::STA  , ALU::STA  , ALU::STA  , ALU::STA  , ALU::CPX  , ALU::STX  , ALU::NOP  , ALU::STY  , ALU::STY  , ALU::LDX  , ALU::PLX  , ALU::DCP  ,
			ALU::BNE  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::STA  , ALU::STA  , ALU::STA  , ALU::STA  , ALU::STX  , ALU::STX  , ALU::NOP  , ALU::STY  , ALU::DEY  , ALU::TYA  , ALU::DEC  , ALU::DCP  ,
			ALU::CLV  , ALU::NOP  , ALU::SETB , ALU::SETB , ALU::LDA  , ALU::LDA  , ALU::LDA  , ALU::LDA  , ALU::LDA  , ALU::LDX  , ALU::NOP  , ALU::LDY  , ALU::LDY  , ALU::NOTC , ALU::PLY  , ALU::ISC  ,
			ALU::BEQ  , ALU::NOP  , ALU::CLRB , ALU::CLRB , ALU::LDA  , ALU::LDA  , ALU::LDA  , ALU::LDA  , ALU::LDX  , ALU::LDX  , ALU::LDv  , ALU::LDY  , ALU::INY  , ALU::TAY  , ALU::INC  , ALU::ISC  ,
		};

		ALU ALU_Type;

	#pragma endregion

	#pragma region SPC700 functions

		void NZ_Set(uint8_t index)
		{
			P &= 0x7D;

			FlagZset(index == 0);
			FlagNset((index & 0x80) == 0x80);
		}

		void NZ_Set_16(uint16_t index)
		{
			P &= 0x7D;

			FlagZset(index == 0);
			FlagNset((index & 0x8000) == 0x8000);
		}

		void Decode(uint8_t opcode)
		{
			Instr_Type = Instr_Type_List[opcode];
			ALU_Type = ALU_Type_List[opcode];
		}

		void Calculate_Wait_States()
		{
			if (Cycle_Type == CPU_Cycle_Type::Internal_Cycle)
			{
				Fetch_Op = 1;
				Fetch_Wait = 2;
			}
			else
			{
				Fetch_Op = 1;
				Fetch_Wait = 2;
			}
		}

		void OnExecFetch(uint16_t addr);

		void ALU_Operation();

		void Execute(int cycles);

		void Fetch_Opcode_No_Interrupt();

		void Fetch1();

		void Decode_Next_Cycle();

		void RunCpuOne();

		void End_ISpecial()
		{
			// no irq flag check here
			Fetch1();
		}

		void End()
		{


		}

		void ExecuteOne()
		{
			Decode_Next_Cycle();
			TotalExecutedCycles++;
			Total_CPU_Clock_Cycles++;
		}

		inline uint32_t get_PC_Addr()
		{
			return PC;
		}

		inline void get_Direct_Addr_Inc()
		{
			address_bus = (ea + 1) & 0xFF;
			address_bus |= D;
		}

		inline void Dec_S()
		{
			S--;
			S &= 0xFF;
			S |= 0x100;
		}

		inline void Inc_S()
		{
			S++;
			S &= 0xFF;
			S |= 0x100;
		}

		void get_Push_value()
		{
			switch (ALU_Type)
			{
				case ALU::PHA:
					value8 = A;
					break;

				case ALU::PHP:
					value8 = P;
					break;

				case ALU::PHX:
					value8 = X;
					break;

				case ALU::PHY:
					value8 = Y;
					break;
			}
		}

		void get_Write_value()
		{
			switch (ALU_Type)
			{
				case ALU::STA:
					value8 = A;
					break;

				case ALU::STX:
					value8 = X;
					break;

				case ALU::STY:
					value8 = Y;
					break;
			}
		}

	#pragma endregion


	#pragma region Disassemble

		// disassemblies will also return strings of the same length
		const char* TraceHeader = "6502: PC, machine code, mnemonic, operands, registers (A, X, Y, P, SP), flags (NVTBDIZCR)  Cycles      SL     F Cycle      ";
		const char* NMI_event = "             ====NMI====             ";
		const char* IRQ_event = "             ====IRQ====             ";
		const char* DMA_event = "             ====DMA====             ";

		const char* Reg_Template = "  A:XX X:XX Y:XX P:XX SP:XX  NVTBDIZCR  Cy:0123456789ABCDEF SLZ:LYL F-Cyc:0123456789ABCDEF";
		const char* Reg_Blank = "                                                                                          ";
		const char* Disasm_template = "PCPC:  AA BB CC  Di Di Di Di Di      ";

		char replacer[40] = {};
		char* val_char_1 = nullptr;
		char* val_char_2 = nullptr;
		uint32_t temp_reg;

		uint32_t op_size = 0;

		void (*TraceCallback)(int);

		string APU_Echo_Write()
		{
			val_char_1 = replacer;

			string reg_state = "  Echo ADDR: ";

			sprintf_s(val_char_1, 5, "%04X", Echo_Addr);
			reg_state.append(val_char_1, 4);

			reg_state.append("  Write Value: ");

			sprintf_s(val_char_1, 3, "%02X", Echo_Value);
			reg_state.append(val_char_1, 2);

			while (reg_state.length() < 87)
			{
				reg_state.append(" ");
			}

			return reg_state;
		}

		string CPUDisassembly()
		{
			string trace_string = "";

			string disasm = Disassemble(PC);

			val_char_1 = replacer;

			sprintf_s(val_char_1, 5, "%04X", PC);
			trace_string.append(val_char_1, 4);
			trace_string.append(":  ");

			uint16_t dis_pc = PC;
			for (uint32_t i = 0; i < op_size; i++)
			{
				sprintf_s(val_char_1, 3, "%02X", PeekMemory(dis_pc++));

				trace_string.append(val_char_1, 2);
				trace_string.append(" ");
			}

			while (trace_string.length() < 18)
			{
				trace_string.append(" ");
			}

			trace_string.append(disasm);

			while (trace_string.length() < 38)
			{
				trace_string.append(" ");
			}

			return trace_string;
		}

		string CPURegisterState()
		{
			string trace_string = " ";

			val_char_1 = replacer;

			trace_string.append(" A:");
			sprintf_s(val_char_1, 3, "%02X", A);
			trace_string.append(val_char_1, 2);

			trace_string.append(" X:");
			sprintf_s(val_char_1, 3, "%02X", X);
			trace_string.append(val_char_1, 2);

			trace_string.append(" Y:");
			sprintf_s(val_char_1, 3, "%02X", Y);
			trace_string.append(val_char_1, 2);

			trace_string.append(" P:");
			sprintf_s(val_char_1, 3, "%02X", P);
			trace_string.append(val_char_1, 2);

			trace_string.append(" SP:");
			sprintf_s(val_char_1, 3, "%02X", S);
			trace_string.append(val_char_1, 2);

			trace_string.append("  ");
			trace_string.append(FlagNget() ? "N" : "n");
			trace_string.append(FlagVget() ? "V" : "v");
			trace_string.append(FlagPget() ? "P" : "p");
			trace_string.append(FlagBget() ? "B" : "b");
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
			sprintf_s(val_char_1, 4, "%3u", *Core_status_sl);
			trace_string.append(val_char_1, 3);

			trace_string.append(" F-Cy:");
			sprintf_s(val_char_1, 17, "%16lld", FrameCycle);
			trace_string.append(val_char_1, 16);

			while (trace_string.length() < 91)
			{
				trace_string.append(" ");
			}

			return trace_string;
		}

		string Disassemble(uint16_t pc)
		{
			uint16_t diff = pc;

			uint8_t op = PeekMemory(pc++);

			string ret = "";

			val_char_2 = replacer;

			switch (op)
			{
				case 0x00: sprintf_s(val_char_2, 40, "BRK"); break;
				case 0x01: sprintf_s(val_char_2, 40, "ORA ($%02X,X)", PeekMemory(pc++)); break;
				case 0x04: sprintf_s(val_char_2, 40, "NOP $%02X", PeekMemory(pc++)); break;
				case 0x05: sprintf_s(val_char_2, 40, "ORA $%02X", PeekMemory(pc++)); break;
				case 0x06: sprintf_s(val_char_2, 40, "ASL $%02X", PeekMemory(pc++)); break;
				case 0x08: sprintf_s(val_char_2, 40, "PHP"); break;
				case 0x09: sprintf_s(val_char_2, 40, "ORA #$%02X", PeekMemory(pc++)); break;
				case 0x0A: sprintf_s(val_char_2, 40, "ASL A"); break;
				case 0x0C: sprintf_s(val_char_2, 40, "NOP ($%04X)", Peek_Memory_16(pc++)); pc++; break;
				case 0x0D: sprintf_s(val_char_2, 40, "ORA $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x0E: sprintf_s(val_char_2, 40, "ASL $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x10: sprintf_s(val_char_2, 40, "BPL $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0x11: sprintf_s(val_char_2, 40, "ORA ($%02X),Y *", PeekMemory(pc++)); break;
				case 0x14: sprintf_s(val_char_2, 40, "NOP $%02X,X", PeekMemory(pc++)); break;
				case 0x15: sprintf_s(val_char_2, 40, "ORA $%02X,X", PeekMemory(pc++)); break;
				case 0x16: sprintf_s(val_char_2, 40, "ASL $%02X,X", PeekMemory(pc++)); break;
				case 0x18: sprintf_s(val_char_2, 40, "CLC"); break;
				case 0x19: sprintf_s(val_char_2, 40, "ORA $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0x1A: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0x1C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", PeekMemory(pc++)); break;
				case 0x1D: sprintf_s(val_char_2, 40, "ORA $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0x1E: sprintf_s(val_char_2, 40, "ASL $%04X,X", Peek_Memory_16(pc++)); pc++; break;
				case 0x20: sprintf_s(val_char_2, 40, "JSR $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x21: sprintf_s(val_char_2, 40, "AND ($%02X,X)", PeekMemory(pc++)); break;
				case 0x24: sprintf_s(val_char_2, 40, "BIT $%02X", PeekMemory(pc++)); break;
				case 0x25: sprintf_s(val_char_2, 40, "AND $%02X", PeekMemory(pc++)); break;
				case 0x26: sprintf_s(val_char_2, 40, "ROL $%02X", PeekMemory(pc++)); break;
				case 0x28: sprintf_s(val_char_2, 40, "PLP"); break;
				case 0x29: sprintf_s(val_char_2, 40, "AND #$%02X", PeekMemory(pc++)); break;
				case 0x2A: sprintf_s(val_char_2, 40, "ROL A"); break;
				case 0x2C: sprintf_s(val_char_2, 40, "BIT $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x2D: sprintf_s(val_char_2, 40, "AND $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x2E: sprintf_s(val_char_2, 40, "ROL $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x30: sprintf_s(val_char_2, 40, "BMI $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0x31: sprintf_s(val_char_2, 40, "AND ($%02X),Y *", PeekMemory(pc++)); break;
				case 0x34: sprintf_s(val_char_2, 40, "NOP $%02X,X", PeekMemory(pc++)); break;
				case 0x35: sprintf_s(val_char_2, 40, "AND $%02X,X", PeekMemory(pc++)); break;
				case 0x36: sprintf_s(val_char_2, 40, "ROL $%02X,X", PeekMemory(pc++)); break;
				case 0x38: sprintf_s(val_char_2, 40, "SEC"); break;
				case 0x39: sprintf_s(val_char_2, 40, "AND $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0x3A: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0x3C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", PeekMemory(pc++)); break;
				case 0x3D: sprintf_s(val_char_2, 40, "AND $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0x3E: sprintf_s(val_char_2, 40, "ROL $%04X,X", Peek_Memory_16(pc++)); pc++; break;
				case 0x40: sprintf_s(val_char_2, 40, "RTI"); break;
				case 0x41: sprintf_s(val_char_2, 40, "EOR ($%02X,X)", PeekMemory(pc++)); break;
				case 0x44: sprintf_s(val_char_2, 40, "NOP $%02X", PeekMemory(pc++)); break;
				case 0x45: sprintf_s(val_char_2, 40, "EOR $%02X", PeekMemory(pc++)); break;
				case 0x46: sprintf_s(val_char_2, 40, "LSR $%02X", PeekMemory(pc++)); break;
				case 0x48: sprintf_s(val_char_2, 40, "PHA"); break;
				case 0x49: sprintf_s(val_char_2, 40, "EOR #$%02X", PeekMemory(pc++)); break;
				case 0x4A: sprintf_s(val_char_2, 40, "LSR A"); break;
				case 0x4C: sprintf_s(val_char_2, 40, "JMP $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x4D: sprintf_s(val_char_2, 40, "EOR $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x4E: sprintf_s(val_char_2, 40, "LSR $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x50: sprintf_s(val_char_2, 40, "BVC $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0x51: sprintf_s(val_char_2, 40, "EOR ($%02X),Y *", PeekMemory(pc++)); break;
				case 0x54: sprintf_s(val_char_2, 40, "NOP $%02X,X", PeekMemory(pc++)); break;
				case 0x55: sprintf_s(val_char_2, 40, "EOR $%02X,X", PeekMemory(pc++)); break;
				case 0x56: sprintf_s(val_char_2, 40, "LSR $%02X,X", PeekMemory(pc++)); break;
				case 0x58: sprintf_s(val_char_2, 40, "CLI"); break;
				case 0x59: sprintf_s(val_char_2, 40, "EOR $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0x5A: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0x5C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", PeekMemory(pc++)); break;
				case 0x5D: sprintf_s(val_char_2, 40, "EOR $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0x5E: sprintf_s(val_char_2, 40, "LSR $%04X,X", Peek_Memory_16(pc++)); pc++; break;
				case 0x60: sprintf_s(val_char_2, 40, "RTS"); break;
				case 0x61: sprintf_s(val_char_2, 40, "ADC ($%02X,X)", PeekMemory(pc++)); break;
				case 0x64: sprintf_s(val_char_2, 40, "NOP $%02X", PeekMemory(pc++)); break;
				case 0x65: sprintf_s(val_char_2, 40, "ADC $%02X", PeekMemory(pc++)); break;
				case 0x66: sprintf_s(val_char_2, 40, "ROR $%02X", PeekMemory(pc++)); break;
				case 0x68: sprintf_s(val_char_2, 40, "PLA"); break;
				case 0x69: sprintf_s(val_char_2, 40, "ADC #$%02X", PeekMemory(pc++)); break;
				case 0x6A: sprintf_s(val_char_2, 40, "ROR A"); break;
				case 0x6C: sprintf_s(val_char_2, 40, "JMP ($%04X)", Peek_Memory_16(pc++)); pc++; break;
				case 0x6D: sprintf_s(val_char_2, 40, "ADC $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x6E: sprintf_s(val_char_2, 40, "ROR $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x70: sprintf_s(val_char_2, 40, "BVS $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0x71: sprintf_s(val_char_2, 40, "ADC ($%02X),Y *", PeekMemory(pc++)); break;
				case 0x74: sprintf_s(val_char_2, 40, "NOP $%02X,X", PeekMemory(pc++)); break;
				case 0x75: sprintf_s(val_char_2, 40, "ADC $%02X,X", PeekMemory(pc++)); break;
				case 0x76: sprintf_s(val_char_2, 40, "ROR $%02X,X", PeekMemory(pc++)); break;
				case 0x78: sprintf_s(val_char_2, 40, "SEI"); break;
				case 0x79: sprintf_s(val_char_2, 40, "ADC $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0x7A: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0x7C: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", PeekMemory(pc++)); break;
				case 0x7D: sprintf_s(val_char_2, 40, "ADC $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0x7E: sprintf_s(val_char_2, 40, "ROR $%04X,X", Peek_Memory_16(pc++)); pc++; break;
				case 0x80: sprintf_s(val_char_2, 40, "NOP #$%02X", PeekMemory(pc++)); break;
				case 0x81: sprintf_s(val_char_2, 40, "STA ($%02X,X)", PeekMemory(pc++)); break;
				case 0x82: sprintf_s(val_char_2, 40, "NOP #$%02X", PeekMemory(pc++)); break;
				case 0x84: sprintf_s(val_char_2, 40, "STY $%02X", PeekMemory(pc++)); break;
				case 0x85: sprintf_s(val_char_2, 40, "STA $%02X", PeekMemory(pc++)); break;
				case 0x86: sprintf_s(val_char_2, 40, "STX $%02X", PeekMemory(pc++)); break;
				case 0x88: sprintf_s(val_char_2, 40, "DEY"); break;
				case 0x89: sprintf_s(val_char_2, 40, "NOP #$%02X", PeekMemory(pc++)); break;
				case 0x8A: sprintf_s(val_char_2, 40, "TXA"); break;
				case 0x8C: sprintf_s(val_char_2, 40, "STY $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x8D: sprintf_s(val_char_2, 40, "STA $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x8E: sprintf_s(val_char_2, 40, "STX $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x90: sprintf_s(val_char_2, 40, "BCC $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0x91: sprintf_s(val_char_2, 40, "STA ($%02X),Y", PeekMemory(pc++)); break;
				case 0x94: sprintf_s(val_char_2, 40, "STY $%02X,X", PeekMemory(pc++)); break;
				case 0x95: sprintf_s(val_char_2, 40, "STA $%02X,X", PeekMemory(pc++)); break;
				case 0x96: sprintf_s(val_char_2, 40, "STX $%02X,Y", PeekMemory(pc++)); break;
				case 0x98: sprintf_s(val_char_2, 40, "TYA"); break;
				case 0x99: sprintf_s(val_char_2, 40, "STA $%04X,Y", Peek_Memory_16(pc++)); pc++; break;
				case 0x9A: sprintf_s(val_char_2, 40, "TXS"); break;
				case 0x9D: sprintf_s(val_char_2, 40, "STA $%04X,X", Peek_Memory_16(pc++)); pc++; break;
				case 0xA0: sprintf_s(val_char_2, 40, "LDY #$%02X", PeekMemory(pc++)); break;
				case 0xA1: sprintf_s(val_char_2, 40, "LDA ($%02X,X)", PeekMemory(pc++)); break;
				case 0xA2: sprintf_s(val_char_2, 40, "LDX #$%02X", PeekMemory(pc++)); break;
				case 0xA4: sprintf_s(val_char_2, 40, "LDY $%02X", PeekMemory(pc++)); break;
				case 0xA5: sprintf_s(val_char_2, 40, "LDA $%02X", PeekMemory(pc++)); break;
				case 0xA6: sprintf_s(val_char_2, 40, "LDX $%02X", PeekMemory(pc++)); break;
				case 0xA8: sprintf_s(val_char_2, 40, "TAY"); break;
				case 0xA9: sprintf_s(val_char_2, 40, "LDA #$%02X", PeekMemory(pc++)); break;
				case 0xAA: sprintf_s(val_char_2, 40, "TAX"); break;
				case 0xAC: sprintf_s(val_char_2, 40, "LDY $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xAD: sprintf_s(val_char_2, 40, "LDA $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xAE: sprintf_s(val_char_2, 40, "LDX $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xB0: sprintf_s(val_char_2, 40, "BCS $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0xB1: sprintf_s(val_char_2, 40, "LDA ($%02X),Y *", PeekMemory(pc++)); break;
				case 0xB3: sprintf_s(val_char_2, 40, "LAX ($%02X),Y *", PeekMemory(pc++)); break;
				case 0xB4: sprintf_s(val_char_2, 40, "LDY $%02X,X", PeekMemory(pc++)); break;
				case 0xB5: sprintf_s(val_char_2, 40, "LDA $%02X,X", PeekMemory(pc++)); break;
				case 0xB6: sprintf_s(val_char_2, 40, "LDX $%02X,Y", PeekMemory(pc++)); break;
				case 0xB8: sprintf_s(val_char_2, 40, "CLV"); break;
				case 0xB9: sprintf_s(val_char_2, 40, "LDA $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0xBA: sprintf_s(val_char_2, 40, "TXS"); break;
				case 0xBC: sprintf_s(val_char_2, 40, "LDY $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0xBD: sprintf_s(val_char_2, 40, "LDA $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0xBE: sprintf_s(val_char_2, 40, "LDX $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0xC0: sprintf_s(val_char_2, 40, "CPY #$%02X", PeekMemory(pc++)); break;
				case 0xC1: sprintf_s(val_char_2, 40, "CMP ($%02X,X)", PeekMemory(pc++)); break;
				case 0xC2: sprintf_s(val_char_2, 40, "NOP #$%02X", PeekMemory(pc++)); break;
				case 0xC4: sprintf_s(val_char_2, 40, "CPY $%02X", PeekMemory(pc++)); break;
				case 0xC5: sprintf_s(val_char_2, 40, "CMP $%02X", PeekMemory(pc++)); break;
				case 0xC6: sprintf_s(val_char_2, 40, "DEC $%02X", PeekMemory(pc++)); break;
				case 0xC8: sprintf_s(val_char_2, 40, "INY"); break;
				case 0xC9: sprintf_s(val_char_2, 40, "CMP #$%02X", PeekMemory(pc++)); break;
				case 0xCA: sprintf_s(val_char_2, 40, "DEX"); break;
				case 0xCB: sprintf_s(val_char_2, 40, "AXS $%02X", PeekMemory(pc++)); break;
				case 0xCC: sprintf_s(val_char_2, 40, "CPY $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xCD: sprintf_s(val_char_2, 40, "CMP $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xCE: sprintf_s(val_char_2, 40, "DEC $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xD0: sprintf_s(val_char_2, 40, "BNE $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0xD1: sprintf_s(val_char_2, 40, "CMP ($%02X),Y *", PeekMemory(pc++)); break;
				case 0xD4: sprintf_s(val_char_2, 40, "NOP $%02X,X", PeekMemory(pc++)); break;
				case 0xD5: sprintf_s(val_char_2, 40, "CMP $%02X,X", PeekMemory(pc++)); break;
				case 0xD6: sprintf_s(val_char_2, 40, "DEC $%02X,X", PeekMemory(pc++)); break;
				case 0xD8: sprintf_s(val_char_2, 40, "CLD"); break;
				case 0xD9: sprintf_s(val_char_2, 40, "CMP $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0xDA: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0xDC: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", PeekMemory(pc++)); break;
				case 0xDD: sprintf_s(val_char_2, 40, "CMP $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0xDE: sprintf_s(val_char_2, 40, "DEC $%04X,X", Peek_Memory_16(pc++)); pc++; break;
				case 0xE0: sprintf_s(val_char_2, 40, "CPX #$%02X", PeekMemory(pc++)); break;
				case 0xE1: sprintf_s(val_char_2, 40, "SBC ($%02X,X)", PeekMemory(pc++)); break;
				case 0xE2: sprintf_s(val_char_2, 40, "NOP #$%02X", PeekMemory(pc++)); break;
				case 0xE4: sprintf_s(val_char_2, 40, "CPX $%02X", PeekMemory(pc++)); break;
				case 0xE5: sprintf_s(val_char_2, 40, "SBC $%02X", PeekMemory(pc++)); break;
				case 0xE6: sprintf_s(val_char_2, 40, "INC $%02X", PeekMemory(pc++)); break;
				case 0xE8: sprintf_s(val_char_2, 40, "INX"); break;
				case 0xE9: sprintf_s(val_char_2, 40, "SBC #$%02X", PeekMemory(pc++)); break;
				case 0xEA: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0xEC: sprintf_s(val_char_2, 40, "CPX $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xED: sprintf_s(val_char_2, 40, "SBC $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xEE: sprintf_s(val_char_2, 40, "INC $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xF0: sprintf_s(val_char_2, 40, "BEQ $%04X", Peek_Memory_8_Branch(pc++)); break;
				case 0xF1: sprintf_s(val_char_2, 40, "SBC ($%02X),Y *", PeekMemory(pc++)); break;
				case 0xF4: sprintf_s(val_char_2, 40, "NOP $%02X,X", PeekMemory(pc++)); break;
				case 0xF5: sprintf_s(val_char_2, 40, "SBC $%02X,X", PeekMemory(pc++)); break;
				case 0xF6: sprintf_s(val_char_2, 40, "INC $%02X,X", PeekMemory(pc++)); break;
				case 0xF8: sprintf_s(val_char_2, 40, "SED"); break;
				case 0xF9: sprintf_s(val_char_2, 40, "SBC $%04X,Y *", Peek_Memory_16(pc++)); pc++; break;
				case 0xFA: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0xFC: sprintf_s(val_char_2, 40, "NOP ($%02X,X)", PeekMemory(pc++)); break;
				case 0xFD: sprintf_s(val_char_2, 40, "SBC $%04X,X *", Peek_Memory_16(pc++)); pc++; break;
				case 0xFE: sprintf_s(val_char_2, 40, "INC $%04X,X", Peek_Memory_16(pc++)); pc++; break;
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

	#pragma endregion

	#pragma region CPU State Save / Load

		uint8_t* cpu_SaveState(uint8_t* saver)
		{
			saver = byte_saver(A, saver);
			saver = byte_saver(X, saver);
			saver = byte_saver(Y, saver);
			saver = byte_saver(P, saver);
			saver = short_saver(PC, saver);
			saver = short_saver(D, saver);
			saver = short_saver(S, saver);
			saver = short_saver(Index_Add, saver);

			saver = bool_saver(NMI, saver);
			saver = bool_saver(IRQ, saver);
			saver = bool_saver(RDY, saver);
			saver = bool_saver(NMI_Br, saver);
			saver = bool_saver(IRQ_Br, saver);

			saver = long_saver(TotalExecutedCycles, saver);
			saver = long_saver(Total_CPU_Clock_Cycles, saver);

			saver = bool_saver(iflag_pending, saver);
			saver = bool_saver(RDY_Freeze, saver);
			saver = bool_saver(branch_irq_hack, saver);
			saver = bool_saver(cpu_First_Check, saver);

			saver = byte_saver(opcode2, saver);
			saver = byte_saver(opcode3, saver);
			saver = byte_saver(opcode4, saver);
			saver = byte_saver(H, saver);

			saver = short_saver(address_bus, saver);

			saver = int_saver(opcode, saver);
			saver = int_saver(ea, saver);
			saver = int_saver(alu_temp, saver);
			saver = int_saver(alu_temp_hi, saver);
			saver = int_saver(alu_temp_16, saver);

			saver = int_saver(Fetch_Cnt, saver);
			saver = int_saver(Fetch_Wait, saver);
			saver = int_saver(Fetch_Op, saver);

			saver = byte_saver(value8, saver);
			saver = byte_saver(temp8, saver);
			saver = short_saver(value16, saver);
			saver = bool_saver(branch_taken, saver);
			saver = bool_saver(my_iflag, saver);
			saver = bool_saver(booltemp, saver);
			saver = int_saver(tempint, saver);
			saver = int_saver(lo, saver);
			saver = int_saver(hi, saver);

			saver = short_saver(IRQ_Type, saver);
			saver = short_saver(Instr_Cycle, saver);

			saver = int_saver((uint32_t)Instr_Type, saver);
			saver = int_saver((uint32_t)ALU_Type, saver);

			return saver;
		}

		uint8_t* cpu_LoadState(uint8_t* loader)
		{
			loader = byte_loader(&A, loader);
			loader = byte_loader(&X, loader);
			loader = byte_loader(&Y, loader);
			loader = byte_loader(&P, loader);
			loader = short_loader(&PC, loader);
			loader = short_loader(&D, loader);
			loader = short_loader(&S, loader);
			loader = short_loader(&Index_Add, loader);

			loader = bool_loader(&NMI, loader);
			loader = bool_loader(&IRQ, loader);
			loader = bool_loader(&RDY, loader);
			loader = bool_loader(&NMI_Br, loader);
			loader = bool_loader(&IRQ_Br, loader);

			loader = long_loader(&TotalExecutedCycles, loader);
			loader = long_loader(&Total_CPU_Clock_Cycles, loader);

			loader = bool_loader(&iflag_pending, loader);
			loader = bool_loader(&RDY_Freeze, loader);
			loader = bool_loader(&branch_irq_hack, loader);
			loader = bool_loader(&cpu_First_Check, loader);

			loader = byte_loader(&opcode2, loader);
			loader = byte_loader(&opcode3, loader);
			loader = byte_loader(&opcode4, loader);
			loader = byte_loader(&H, loader);

			loader = short_loader(&address_bus, loader);

			loader = int_loader(&opcode, loader);
			loader = int_loader(&ea, loader);
			loader = int_loader(&alu_temp, loader);
			loader = int_loader(&alu_temp_hi, loader);
			loader = int_loader(&alu_temp_16, loader);

			loader = int_loader(&Fetch_Cnt, loader);
			loader = int_loader(&Fetch_Wait, loader);
			loader = int_loader(&Fetch_Op, loader);

			loader = byte_loader(&value8, loader);
			loader = byte_loader(&temp8, loader);
			loader = short_loader(&value16, loader);
			loader = bool_loader(&branch_taken, loader);
			loader = bool_loader(&my_iflag, loader);
			loader = bool_loader(&booltemp, loader);
			loader = sint_loader(&tempint, loader);
			loader = int_loader(&lo, loader);
			loader = int_loader(&hi, loader);

			loader = short_loader(&IRQ_Type, loader);
			loader = short_loader(&Instr_Cycle, loader);

			loader = int_loader(&Instr_Type_Save, loader);
			loader = int_loader(&ALU_Type_Save, loader);

			Instr_Type = static_cast<OpT>(Instr_Type_Save);
			ALU_Type = static_cast<ALU>(ALU_Type_Save);

			return loader;
		}

	#pragma endregion

	#pragma endregion

	};
}

#endif