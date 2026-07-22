#pragma once

#ifndef SNES_CPU_H
#define SNES_CPU_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>
#include <inttypes.h>
#include <cmath>

#include "../Common/Savestate.h"
#include "../Common/Common_Ops.h"

#include "SNES_System.h"

#ifndef _WIN32
#define sprintf_s snprintf
#endif

using namespace std;

namespace SNESHawk
{
	class SNES_System;

	class R5A22
	{
	public:

		SNES_System* Sys_pntr = nullptr;

	#pragma region DMA Controller





	#pragma region DMA State Save / Load

		uint8_t* DMA_SaveState(uint8_t* saver)
		{

			return saver;
		}

		uint8_t* DMA_LoadState(uint8_t* loader)
		{

			return loader;
		}

	#pragma endregion
		

	#pragma endregion





	#pragma region W65C816
	#pragma region Variables

		uint16_t Instr_Cycle;
		uint16_t IRQ_Type;
		uint16_t Instr_Skip;

		uint32_t Instr_Type_Save;
		uint32_t ALU_Type_Save;
		uint32_t Cycle_Type_Save;
		uint32_t RW_Size_Save;

		uint32_t Fetch_Cnt, Fetch_Wait, Fetch_Op;

		bool BCD_Enabled = false;
		bool debug = false;
		bool Is_Index_16;
		bool Is_Acc_16;
		bool IRQ;
		bool NMI;
		bool RDY;
		bool Flag_E;
		bool Flag_B;
		bool ALU_Op_Size_8;

		uint8_t P;
		uint8_t DBR;
		uint8_t PBR;
		uint16_t PC;
		uint16_t S;
		uint16_t D;
		uint16_t A;
		uint16_t X;
		uint16_t Y;
		uint16_t ea;

		uint64_t TotalExecutedCycles;
		uint64_t Total_CPU_Clock_Cycles;

		bool iflag_pending;

		uint8_t opcode2, opcode3, opcode4;
		uint8_t H;

		uint32_t Push_Shift;
		uint32_t address_bus;

		uint32_t opcode;
		uint32_t alu_temp, alu_temp_hi;

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

		void HardReset()
		{
			A = 0;
			X = 0;
			Y = 0;
			P = 0x20; // 5th bit always set
			S = 0x100;
			PC = 0;
			TotalExecutedCycles = 0;

			Instr_Type = OpT::INT;
			IRQ_Type = 2;
			ALU_Type = ALU::NOP;
			Instr_Cycle = -1;
			RW_Size_Op = RW_Size::NA;
			Cycle_Type = CPU_Cycle_Type::Fetch_Reset;

			Push_Shift = 0;
			address_bus = 0;

			Instr_Skip = 1;
			ALU_Op_Size_8 = true;

			opcode = 0;
			iflag_pending = true;
			RDY = true;
			BCD_Enabled = false;

			Is_Acc_16 = false;
			Is_Index_16 = false;

			IRQ = false;
			NMI = false;

			Fetch_Cnt = 0;
			Fetch_Op = 1;
			Fetch_Wait = 6;

			Flag_E = true;
			Flag_B = false;
		}

		void SoftReset()
		{
			Instr_Type = OpT::INT;
			IRQ_Type = 2;
			ALU_Type = ALU::NOP;
			Instr_Cycle = -1;
			RW_Size_Op = RW_Size::NA;
			Cycle_Type = CPU_Cycle_Type::Fetch_Reset;

			Push_Shift = 0;
			address_bus = 0;

			opcode = 0;

			iflag_pending = true;
			FlagIset(true);

			RDY = true;
			BCD_Enabled = false;

			Instr_Skip = 1;
			ALU_Op_Size_8 = true;

			Fetch_Cnt = 0;
			Fetch_Op = 1;
			Fetch_Wait = 6;

			Flag_E = true;
			Flag_B = false;

		}

		inline bool FlagCget() { return (P & 0x01) != 0; }
		inline void FlagCset(bool value) { P = (uint8_t)((P & ~0x01) | (value ? 0x01 : 0x00)); }

		inline bool FlagZget() { return (P & 0x02) != 0; }
		inline void FlagZset(bool value) { P = (uint8_t)((P & ~0x02) | (value ? 0x02 : 0x00)); }

		inline bool FlagIget() { return (P & 0x04) != 0; }
		inline void FlagIset(bool value) { P = (uint8_t)((P & ~0x04) | (value ? 0x04 : 0x00)); }

		inline bool FlagDget() { return (P & 0x08) != 0; }
		inline void FlagDset(bool value) { P = (uint8_t)((P & ~0x08) | (value ? 0x08 : 0x00)); }

		inline bool FlagXget() { return (P & 0x10) != 0; }
		inline void FlagXset(bool value)
		{
			if (Flag_E)
			{
				P |= 0x10;
			}
			else
			{
				P = (uint8_t)((P & ~0x10) | (value ? 0x10 : 0x00));
			}
		}

		inline bool FlagMget() { return (P & 0x20) != 0; }
		inline void FlagMset(bool value)
		{
			if (Flag_E)
			{
				P |= 0x20;
			}
			else
			{
				P = (uint8_t)((P & ~0x20) | (value ? 0x20 : 0x00));
			}
		}

		inline bool FlagVget() { return (P & 0x40) != 0; }
		inline void FlagVset(bool value) { P = (uint8_t)((P & ~0x40) | (value ? 0x40 : 0x00)); }

		inline bool FlagNget() { return (P & 0x80) != 0; }
		inline void FlagNset(bool value) { P = (uint8_t)((P & ~0x80) | (value ? 0x80 : 0x00)); }

		// SO pin
		inline void SetOverflow() { FlagVset(true); }

		uint8_t TableNZ[256] =
		{
			0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
		};

	#pragma endregion

	#pragma region Constant Declarations
		enum class OpT
		{
			Br,			// Branch
			JSR,		// JSR
			JMP,		// Jump
			JMPI,		// Jump Indirect
			JMPX,		// Jump Indirect,x
			Imp,		// Implied
			Imm,		// Immediate
			Acc,		// Accumulator
			PL,			// Pull
			PH,			// Push
			RTS,		// RTS
			RTI,		// RTI
			CSI,		// CLI, SEI
			BRK,		// Break

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

			// int value 22
			ZPR,		// [zero page READ]
			ZPW,		// [zero page WRITE]
			ZPRW,		// [zero page RMW]

			ZPXR,		// zp,X [zero page indexed READ X]
			ZPYR,		// zp,Y [zero page indexed READ Y]
			ZPXW,		// zp,X [zero page indexed WRITE X]
			ZPYW,		// zp,Y [zero page indexed WRITE Y]
			ZPXRW,		// zp,X [zero page indexed RMW]

			// int value 30
			AIXR,		// addr,X [absolute indexed READ X]
			AIYR,		// addr,Y [absolute indexed READ Y]
			AIXW,		// addr,X [absolute indexed WRITE X]
			AIYW,		// addr,Y [absolute indexed WRITE Y]
			AIXRW,		// addr,X [absolute indexed RMW X]
			AIYRW,		// addr,Y [absolute indexed RMW Y]

			Jam,		// Jam
			INT,		// Interrupts
		};

		OpT Instr_Type;

		OpT Instr_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			OpT::BRK  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PH   , OpT::Imm  , OpT::Acc  , OpT::PH   , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::JSR  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PL   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,

			OpT::RTI  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PH   , OpT::Imm  , OpT::Acc  , OpT::PH   , OpT::JMP  , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::CSI  , OpT::AIYR , OpT::PH   , OpT::AIYRW, OpT::JMP  , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::RTS  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PL   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::JMPI , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::CSI  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::JMPX , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,

			OpT::Imm  , OpT::AdXW , OpT::Imm  , OpT::AdXW , OpT::ZPW  , OpT::ZPW  , OpT::ZPW  , OpT::ZPW  , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::PH   , OpT::AbsW , OpT::AbsW , OpT::AbsW , OpT::AbsW ,
			OpT::Br   , OpT::IIYW , OpT::Jam  , OpT::IIYW , OpT::ZPXW , OpT::ZPXW , OpT::ZPYW , OpT::ZPYW , OpT::Imp  , OpT::AIYW , OpT::Imp  , OpT::AbsW , OpT::AbsW , OpT::AIXW , OpT::AbsW , OpT::AbsW ,
			OpT::Imm  , OpT::AdXR , OpT::Imm  , OpT::AdXR , OpT::ZPR  , OpT::ZPR  , OpT::ZPR  , OpT::ZPR  , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsR , OpT::AbsR ,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYR , OpT::ZPXR , OpT::ZPXR , OpT::ZPYR , OpT::ZPYR , OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYR , OpT::AIXR , OpT::AIXR , OpT::AIYR , OpT::AIYR ,

			OpT::Imm  , OpT::AdXR , OpT::Imm  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::PH   , OpT::AIYRW, OpT::JMPI , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::Imm  , OpT::AdXR , OpT::Imm  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
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
			PC_Change_Cycle,
			Fetch_Reset,
		};

		CPU_Cycle_Type Cycle_Type;

		enum class ALU
		{
			// regular ops
			NOP, SEC, SEI, CLC, CLI, BIT, AND, EOR, ORA, ADC, CMP, CPY, CPX, ASL, SBC, ROL,
			LSR, ASR, ROR, RRA, DEX, DEY, TXA, TYA, TXS, TAY, TAX, CLV, TSX, DEC, INY, CLD,
			INC, INX,

			// int value 33
			// A implied
			ASLA, ROLA, LSRA, RORA,

			// 65C816 misc.
			SED, WAI, STP, XBA, XCE, TCS, TSC, TCD, RTL, TDC, TXY, TYX,

			// Branch conditions
			BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ,

			// push pull op
			PHA, PHP, PHD, PHK, PHB, PHX, PHY, PLA, PLP, PLD, PLB, PLX, PLY,

			// loads / stores
			STA, STX, STY, STZ, LDA, LDX, LDY,

			// reserved area
			RA_1, RA_2,

			// Now duplicate the whole thing for 16 bit versions
			// these are chosen in the decode step based on emulation mode / processor flags

			// regular ops
			NOP_16, SEC_16, SEI_16, CLC_16, CLI_16, BIT_16, AND_16, EOR_16, ORA_16, ADC_16, CMP_16, CPY_16, CPX_16, ASL_16, SBC_16, ROL_16,
			LSR_16, ASR_16, ROR_16, RRA_16, DEX_16, DEY_16, TXA_16, TYA_16, TXS_16, TAY_16, TAX_16, CLV_16, TSX_16, DEC_16, INY_16, CLD_16,
			INC_16, INX_16,

			// int value 33
			// A implied
			ASLA_16, ROLA_16, LSRA_16, RORA_16,

			// 65C816 misc.
			SED_16, WAI_16, STP_16, XBA_16, XCE_16, TCS_16, TSC_16, TCD_16, RTL_16, TDC_16, TXY_16, TYX_16,

			// Branch conditions
			BPL_16, BMI_16, BVC_16, BVS_16, BCC_16, BCS_16, BNE_16, BEQ_16,

			// push pull op
			PHA_16, PHP_16, PHD_16, PHK_16, PHB_16, PHX_16, PHY_16, PLA_16, PLP_16, PLD_16, PLB_16, PLX_16, PLY_16,

			// loads / stores
			STA_16, STX_16, STY_16, STZ_16, LDA_16, LDX_16, LDY_16,

			// reserved area
			RA_1_16, RA_2_16,
		};

		ALU ALU_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			ALU::NOP  , ALU::ORA  , ALU::NOP  , ALU::ORA  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::ORA  , ALU::PHP  , ALU::ORA  , ALU::ASLA , ALU::PHD  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::ORA  ,
			ALU::BPL  , ALU::ORA  , ALU::NOP  , ALU::ORA  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::ORA  , ALU::CLC  , ALU::ORA  , ALU::NOP  , ALU::TCS  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::ORA  ,
			ALU::NOP  , ALU::AND  , ALU::NOP  , ALU::AND  , ALU::BIT  , ALU::AND  , ALU::ROL  , ALU::AND  , ALU::PLP  , ALU::AND  , ALU::ROLA , ALU::PLD  , ALU::BIT  , ALU::AND  , ALU::ROL  , ALU::AND  ,
			ALU::BMI  , ALU::AND  , ALU::NOP  , ALU::AND  , ALU::NOP  , ALU::AND  , ALU::ROL  , ALU::AND  , ALU::SEC  , ALU::AND  , ALU::NOP  , ALU::TSC  , ALU::NOP  , ALU::AND  , ALU::ROL  , ALU::AND  ,

			ALU::NOP  , ALU::EOR  , ALU::NOP  , ALU::EOR  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::EOR  , ALU::PHA  , ALU::EOR  , ALU::LSRA , ALU::PHK  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::EOR  ,
			ALU::BVC  , ALU::EOR  , ALU::NOP  , ALU::EOR  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::EOR  , ALU::CLI  , ALU::EOR  , ALU::PHY  , ALU::TCD  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::EOR  ,
			ALU::NOP  , ALU::ADC  , ALU::NOP  , ALU::RRA  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  , ALU::PLA  , ALU::ADC  , ALU::RORA , ALU::RTL  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  ,
			ALU::BVS  , ALU::ADC  , ALU::NOP  , ALU::RRA  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  , ALU::SEI  , ALU::ADC  , ALU::PLY  , ALU::TDC  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  ,

			ALU::NOP  , ALU::STA  , ALU::NOP  , ALU::STA  , ALU::STY  , ALU::STA  , ALU::STX  , ALU::STA  , ALU::DEY  , ALU::NOP  , ALU::TXA  , ALU::PHB  , ALU::STY  , ALU::STA  , ALU::STX  , ALU::STA  ,
			ALU::BCC  , ALU::STA  , ALU::NOP  , ALU::STA  , ALU::STY  , ALU::STA  , ALU::STX  , ALU::STA  , ALU::TYA  , ALU::STA  , ALU::TXS  , ALU::TXY  , ALU::STZ  , ALU::STA  , ALU::STZ  , ALU::STA  ,
			ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LDA  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LDA  , ALU::TAY  , ALU::LDA  , ALU::TAX  , ALU::PLB  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LDA  ,
			ALU::BCS  , ALU::LDA  , ALU::NOP  , ALU::LDA  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LDA  , ALU::CLV  , ALU::LDA  , ALU::TSX  , ALU::TYX  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LDA  ,

			ALU::CPY  , ALU::CMP  , ALU::NOP  , ALU::CMP  , ALU::CPY  , ALU::CMP  , ALU::DEC  , ALU::CMP  , ALU::INY  , ALU::CMP  , ALU::DEX  , ALU::WAI  , ALU::CPY  , ALU::CMP  , ALU::DEC  , ALU::CMP  ,
			ALU::BNE  , ALU::CMP  , ALU::NOP  , ALU::CMP  , ALU::NOP  , ALU::CMP  , ALU::DEC  , ALU::CMP  , ALU::CLD  , ALU::CMP  , ALU::PHX  , ALU::STP  , ALU::NOP  , ALU::CMP  , ALU::DEC  , ALU::CMP  ,
			ALU::CPX  , ALU::SBC  , ALU::NOP  , ALU::SBC  , ALU::CPX  , ALU::SBC  , ALU::INC  , ALU::SBC  , ALU::INX  , ALU::SBC  , ALU::NOP  , ALU::XBA  , ALU::CPX  , ALU::SBC  , ALU::INC  , ALU::SBC  ,
			ALU::BEQ  , ALU::SBC  , ALU::NOP  , ALU::SBC  , ALU::NOP  , ALU::SBC  , ALU::INC  , ALU::SBC  , ALU::SED  , ALU::SBC  , ALU::PLX  , ALU::XCE  , ALU::NOP  , ALU::SBC  , ALU::INC  , ALU::SBC  ,
		};

		ALU ALU_Type;

		enum class RW_Size
		{
			NA,	// Not Applicable (includes 8 bit only operations like STZ)
			Acc,// Depends on size of accumulator
			IXY,// Depends on size of X,Y regs
			A16,// Always 16 bits (ex PLD) or long jumps
		};

		RW_Size RW_Size_List[256] =
		{
			//  0			   1			  2				 3			    4			   5			  6				 7				8			   9			  A				 B				C			   D			  E				 F
			RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::A16 , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::A16 , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,

			RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::NA  , RW_Size::A16 , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,

			RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::IXY , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc ,

			RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::NA  , RW_Size::A16 , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::NA  , RW_Size::IXY , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
			RW_Size::NA  , RW_Size::Acc , RW_Size::Acc , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc , RW_Size::IXY , RW_Size::NA  , RW_Size::NA  , RW_Size::Acc , RW_Size::NA  , RW_Size::Acc ,
		};

		RW_Size RW_Size_Op;

	#pragma endregion

	#pragma region M6502 functions

		void NZ_ALU() { P = (uint8_t)((P & 0x7D) | TableNZ[alu_temp]); }

		void Decode(uint8_t opcode)
		{
			Instr_Type = Instr_Type_List[opcode];
			ALU_Type = ALU_Type_List[opcode];
			RW_Size_Op = RW_Size_List[opcode];

			// determine whether one or two reads or writes occurs depending on processor state
			if (RW_Size_Op == RW_Size::NA)
			{
				Instr_Skip = 1;
				ALU_Op_Size_8 = true;
			}
			else if (RW_Size_Op == RW_Size::Acc)
			{
				if (Flag_E || FlagMget())
				{
					Instr_Skip = 1;
					ALU_Op_Size_8 = true;
				}
				else
				{
					Instr_Skip = 0;
					ALU_Op_Size_8 = false;
				}
			}
			else if (RW_Size_Op == RW_Size::IXY)
			{
				if (Flag_E || FlagXget())
				{
					Instr_Skip = 1;
					ALU_Op_Size_8 = true;
				}
				else
				{
					Instr_Skip = 0;
					ALU_Op_Size_8 = false;
				}
			}
			else
			{
				Instr_Skip = 0;
				ALU_Op_Size_8 = false;
			}

			// Now we have know what size operation to do, choose ALU op accordingly
			if (!ALU_Op_Size_8)
			{
				ALU_Type = static_cast<ALU>(76 + (uint32_t)ALU_Type);
			}
		}

		uint32_t Calculate_Wait_States()
		{
			return 1;
		}

		void OnExecFetch(uint16_t addr);

		void ALU_Operation();

		void Execute(int cycles);

		void Fetch_Dummy_Interrupt();

		void Fetch_Opcode_No_Interrupt();

		void Fetch1();

		void ExecuteOneOp();

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
			ExecuteOneOp();
			TotalExecutedCycles++;
			Total_CPU_Clock_Cycles++;
		}

		void Get_Push_Value()
		{

		}

		inline uint32_t get_PC_Addr()
		{
			return (((uint32_t)PBR) << 16) | PC;
		}

		inline uint16_t Dec_S()
		{
			S--;
			if (Flag_E)
			{
				S &= 0xFF;
				S |= 0x100;
			}
		}

		inline uint16_t Inc_S()
		{
			S++;
			if (Flag_E)
			{
				S &= 0xFF;
				S |= 0x100;
			}
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

				case ALU::PHD:
					value8 = D;
					break;

				case ALU::PHK:
					value8 = PBR;
					break;

				case ALU::PHB:
					value8 = DBR;
					break;

				case ALU::PHX:
					value8 = X;
					break;

				case ALU::PHY:
					value8 = Y;
					break;

				case ALU::PHA_16:
					value8 = A >> Push_Shift;
					break;

				case ALU::PHP_16:
					value8 = P >> Push_Shift;
					break;

				case ALU::PHD_16:
					value8 = D >> Push_Shift;
					break;

				case ALU::PHK_16:
					value8 = PBR >> Push_Shift;
					break;

				case ALU::PHB_16:
					value8 = DBR >> Push_Shift;
					break;

				case ALU::PHX_16:
					value8 = X >> Push_Shift;
					break;

				case ALU::PHY_16:
					value8 = Y >> Push_Shift;
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

		string CPUDMAStateOAM();

		string CPUDisassembly();

		string CPURegisterState();

		string Disassemble(uint16_t pc);

	#pragma endregion

	#pragma region CPU State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = byte_saver(P, saver);
			saver = byte_saver(DBR, saver);
			saver = byte_saver(PBR, saver);
			saver = short_saver(PC, saver);
			saver = short_saver(S, saver);
			saver = short_saver(D, saver);
			saver = short_saver(A, saver);
			saver = short_saver(X, saver);
			saver = short_saver(Y, saver);
			saver = short_saver(ea, saver);

			saver = bool_saver(NMI, saver);
			saver = bool_saver(IRQ, saver);
			saver = bool_saver(RDY, saver);
			saver = bool_saver(Is_Index_16, saver);
			saver = bool_saver(Is_Acc_16, saver);
			saver = bool_saver(Flag_E, saver);
			saver = bool_saver(Flag_B, saver);
			saver = bool_saver(ALU_Op_Size_8, saver);
			saver = bool_saver(iflag_pending, saver);

			saver = long_saver(TotalExecutedCycles, saver);
			saver = long_saver(Total_CPU_Clock_Cycles, saver);

			saver = byte_saver(opcode2, saver);
			saver = byte_saver(opcode3, saver);
			saver = byte_saver(opcode4, saver);
			saver = byte_saver(H, saver);

			saver = int_saver(Push_Shift, saver);
			saver = int_saver(address_bus, saver);
			saver = int_saver(opcode, saver);
			saver = int_saver(alu_temp, saver);
			saver = int_saver(alu_temp_hi, saver);

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
			saver = short_saver(Instr_Skip, saver);

			saver = int_saver((uint32_t)Instr_Type, saver);
			saver = int_saver((uint32_t)ALU_Type, saver);
			saver = int_saver((uint32_t)Cycle_Type, saver);
			saver = int_saver((uint32_t)RW_Size_Op, saver);

			saver = int_saver(Fetch_Cnt, saver);
			saver = int_saver(Fetch_Wait, saver);
			saver = int_saver(Fetch_Op, saver);

			saver = DMA_SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = byte_loader(&P, loader);
			loader = byte_loader(&DBR, loader);
			loader = byte_loader(&PBR, loader);
			loader = short_loader(&PC, loader);
			loader = short_loader(&S, loader);
			loader = short_loader(&D, loader);
			loader = short_loader(&A, loader);
			loader = short_loader(&X, loader);
			loader = short_loader(&Y, loader);
			loader = short_loader(&ea, loader);

			loader = bool_loader(&NMI, loader);
			loader = bool_loader(&IRQ, loader);
			loader = bool_loader(&RDY, loader);
			loader = bool_loader(&Is_Index_16, loader);
			loader = bool_loader(&Is_Acc_16, loader);
			loader = bool_loader(&Flag_E, loader);
			loader = bool_loader(&Flag_B, loader);
			loader = bool_loader(&ALU_Op_Size_8, loader);
			loader = bool_loader(&iflag_pending, loader);

			loader = long_loader(&TotalExecutedCycles, loader);
			loader = long_loader(&Total_CPU_Clock_Cycles, loader);

			loader = byte_loader(&opcode2, loader);
			loader = byte_loader(&opcode3, loader);
			loader = byte_loader(&opcode4, loader);
			loader = byte_loader(&H, loader);

			loader = int_loader(&Push_Shift, loader);
			loader = int_loader(&address_bus, loader);
			loader = int_loader(&opcode, loader);
			loader = int_loader(&alu_temp, loader);
			loader = int_loader(&alu_temp_hi, loader);

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
			loader = short_loader(&Instr_Skip, loader);

			loader = int_loader(&Instr_Type_Save, loader);
			loader = int_loader(&ALU_Type_Save, loader);
			loader = int_loader(&Cycle_Type_Save, loader);
			loader = int_loader(&RW_Size_Save, loader);

			Instr_Type = static_cast<OpT>(Instr_Type_Save);
			ALU_Type = static_cast<ALU>(ALU_Type_Save);
			Cycle_Type = static_cast<CPU_Cycle_Type>(Cycle_Type_Save);
			RW_Size_Op = static_cast<RW_Size>(RW_Size_Save);

			loader = int_loader(&Fetch_Cnt, loader);
			loader = int_loader(&Fetch_Wait, loader);
			loader = int_loader(&Fetch_Op, loader);

			loader = DMA_LoadState(loader);

			return loader;
		}

	#pragma endregion
	#pragma endregion
	};
}

#endif