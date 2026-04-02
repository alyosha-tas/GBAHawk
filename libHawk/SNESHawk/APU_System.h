#pragma once

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
*	it operates mostly as an independent console, only communicating with the SNES through certain registers
* 
*   The SPC700 has interrupts but they are unused so not implemented here
*/

//Message_String = "Uop " + to_string((int)uop) + " cyc: " + to_string(TotalExecutedCycles);

//MessageCallback(Message_String.length());

namespace SNESHawk
{
	class APU_System
	{
	public:

	#pragma region General APU System

		// loaded at power on
		uint8_t IPL_ROM[64] = { };

		bool IPL_Enable;

		uint8_t Undoc_Reg;
		uint8_t Ctrl_Reg;
		uint8_t DSP_Reg_Addr;
		uint8_t DSP_Reg_Data;
		uint8_t Port_0_Reg, Port_1_Reg, Port_2_Reg, Port_3_Reg;
		uint8_t Timer_0_Reg, Timer_1_Reg, Timer_2_Reg;
		uint8_t Counter_0_Reg, Counter_1_Reg, Counter_2_Reg;
		uint8_t RAM_F8, RAM_F9;

		uint8_t RAM[0x10000] = { };

		inline void Single_Step();

		uint8_t ReadMemory(uint32_t addr);

		uint8_t ReadMemoryDMA(uint32_t addr);

		void WriteMemory(uint32_t addr, uint8_t value);

		uint8_t PeekMemory(uint32_t addr);

		uint16_t Peek_Memory_16(uint32_t addr);

		uint16_t Peek_Memory_8_Branch(uint32_t addr);

		uint8_t PeekReg(uint32_t addr);

		uint8_t ReadReg(uint32_t addr);

		void WriteReg(uint32_t addr, uint8_t value);

		void Reset()
		{
			IPL_Enable = true;
			
			Undoc_Reg = 0xA0;
			Ctrl_Reg = 0xB0;
			DSP_Reg_Addr = 0;
			DSP_Reg_Data = 0;
			Port_0_Reg = Port_1_Reg = Port_2_Reg = Port_3_Reg = 0;
			Timer_0_Reg = Timer_1_Reg = Timer_2_Reg = 0;
			Counter_0_Reg = Counter_1_Reg = Counter_2_Reg = 0;
			RAM_F8 = RAM_F9 = 0;
		}

	#pragma endregion


	#pragma region SPC700
	#pragma region Variables

		uint16_t spc_Instr_Cycle;
		uint16_t spc_IRQ_Type;

		uint32_t spc_Instr_Type_Save;
		uint32_t spc_ALU_Type_Save;

		uint8_t A;
		uint8_t X;
		uint8_t Y;
		uint8_t P;
		uint16_t PC;
		uint8_t S;

		uint64_t TotalExecutedCycles;
		uint64_t Total_spc_Clock_Cycles;

		uint8_t opcode2, opcode3;
		uint8_t H;
		uint8_t Bit;

		uint16_t address_bus;

		uint32_t opcode;
		uint32_t ea, alu_temp;

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

		void spc_Reset()
		{
			A = 0;
			X = 0;
			Y = 0;
			P = 0;
			S = 0;
			PC = 0;
			TotalExecutedCycles = 0;

			spc_Instr_Type = OpT::DRMI;
			spc_IRQ_Type = 2;
			spc_ALU_Type = ALU::NOP;
			spc_Instr_Cycle = 0;

			opcode = 0;
		}

		inline bool spc_FlagCget() { return (P & 0x01) != 0; }
		inline void spc_FlagCset(bool value) { P = (uint8_t)((P & ~0x01) | (value ? 0x01 : 0x00)); }

		inline bool spc_FlagZget() { return (P & 0x02) != 0; }
		inline void spc_FlagZset(bool value) { P = (uint8_t)((P & ~0x02) | (value ? 0x02 : 0x00)); }

		inline bool spc_FlagIget() { return (P & 0x04) != 0; }
		inline void spc_FlagIset(bool value) { P = (uint8_t)((P & ~0x04) | (value ? 0x04 : 0x00)); }

		inline bool spc_FlagHget() { return (P & 0x08) != 0; }
		inline void spc_FlagHset(bool value) { P = (uint8_t)((P & ~0x08) | (value ? 0x08 : 0x00)); }

		inline bool spc_FlagBget() { return (P & 0x10) != 0; }
		inline void spc_FlagBset(bool value) { P = (uint8_t)((P & ~0x10) | (value ? 0x10 : 0x00)); }

		inline bool spc_FlagPget() { return (P & 0x20) != 0; }
		inline void spc_FlagPset(bool value) { P = (uint8_t)((P & ~0x20) | (value ? 0x20 : 0x00)); }

		inline bool spc_FlagVget() { return (P & 0x40) != 0; }
		inline void spc_FlagVset(bool value) { P = (uint8_t)((P & ~0x40) | (value ? 0x40 : 0x00)); }

		inline bool spc_FlagNget() { return (P & 0x80) != 0; }
		inline void spc_FlagNset(bool value) { P = (uint8_t)((P & ~0x80) | (value ? 0x80 : 0x00)); }

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
			AIUW,		// addr,(X,Y) [absolute indexed WRITE Unofficial]

			// Call
			TCAL,		// TCALL

			// Bit sets and branches
			SECLB,
			BBSC,

			INT,		// Interrupts
			DRMI,		// Dummy reads for interrupts
			FONI		// Fetch opcode no interrupts
		};

		OpT spc_Instr_Type;

		OpT spc_Instr_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			OpT::Imp  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PH   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::Imp  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PL   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,

			OpT::Imp  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PH   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::JMP  , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::CSI  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::Imp  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PL   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::JMPI , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::CSI  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,

			OpT::Imp  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPW  , OpT::ZPW  , OpT::ZPW  , OpT::ZPW  , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsW , OpT::AbsW , OpT::AbsW , OpT::AbsW ,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXW , OpT::ZPXW , OpT::ZPYW , OpT::ZPYW , OpT::Imp  , OpT::AIYW , OpT::Imp  , OpT::AIUW , OpT::AIUW , OpT::AIXW , OpT::AIUW , OpT::AIUW ,
			OpT::CSI  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPR  , OpT::ZPR  , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsR , OpT::AbsR ,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPYR , OpT::ZPYR , OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYR , OpT::AIXR , OpT::AIXR , OpT::AIYR , OpT::AIYR ,

			OpT::CSI  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::Imp  , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::TCAL , OpT::SECLB, OpT::BBSC , OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
		};

		enum class ALU
		{
			// regular ops
			NOP, BIT, AND, EOR, ORA, ADC, CMP, CPY, CPX, ASL, SBC, ROL,
			LSR, ASR, ROR, RRA, DEX, DEY, TXA, TYA, TXS, TAY, TAX, CLV,
			TSX, DEC, INY, CLD, INC, INX, SEC, SEI, SEP, CLC, CLI, CLP,
			CLV, MOV,

			// int value 33
			// A implied
			ASLA, ROLA, LSRA, RORA,

			// Branch conditions
			BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ,

			// push pull op
			PLP, PLA, PHA, PHP,

			// loads / stores
			STA, STX, STY, LDA, LDX, LDY,

			SLP, STP,

		};

		ALU spc_ALU_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ASLA , ALU::ASL  , ALU::ASL  , ALU::ORA  , ALU::ASL  , ALU::NOP  ,
			ALU::BPL  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::ORA  , ALU::NOP  , ALU::ASL  , ALU::ASL  , ALU::DEC  , ALU::ASL  , ALU::NOP  ,
			ALU::CLP  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::ROLA , ALU::ROL  , ALU::ROL  , ALU::ROL  , ALU::ROL  , ALU::NOP  ,
			ALU::BMI  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::AND  , ALU::NOP  , ALU::ROL  , ALU::ROL  , ALU::INC  , ALU::ROL  , ALU::NOP  ,

			ALU::SEP  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::LSRA , ALU::LSR  , ALU::LSR  , ALU::LSR  , ALU::LSR  , ALU::NOP  ,
			ALU::BVC  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::EOR  , ALU::NOP  , ALU::LSR  , ALU::LSR  , ALU::MOV  , ALU::LSR  , ALU::NOP  ,
			ALU::CLC  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::RORA , ALU::ROR  , ALU::ROR  , ALU::ROR  , ALU::ROR  , ALU::RRA  ,
			ALU::BVS  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::CMP  , ALU::NOP  , ALU::ROR  , ALU::ROR  , ALU::MOV  , ALU::ROR  , ALU::RRA  ,

			ALU::SEC  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::TXA  , ALU::DEC  , ALU::DEC  , ALU::STA  , ALU::STX  , ALU::NOP  ,
			ALU::BCC  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::ADC  , ALU::TXS  , ALU::DEC  , ALU::DEC  , ALU::MOV  , ALU::NOP  , ALU::NOP  ,
			ALU::SEI  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::TAX  , ALU::INC  , ALU::INC  , ALU::LDA  , ALU::LDX  , ALU::NOP  ,
			ALU::BCS  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::SBC  , ALU::TSX  , ALU::INC  , ALU::INC  , ALU::MOV  , ALU::LDX  , ALU::NOP  ,

			ALU::CLI  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::CPX  , ALU::MOV  , ALU::DEX  , ALU::MOV  , ALU::MOV  , ALU::CMP  , ALU::DEC  , ALU::NOP  ,
			ALU::BNE  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::NOP  , ALU::MOV  , ALU::DEC  , ALU::MOV  , ALU::DEC  , ALU::NOP  ,
			ALU::CLV  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::NOP  , ALU::MOV  , ALU::MOV  , ALU::SBC  , ALU::INC  , ALU::SLP  ,
			ALU::BEQ  , ALU::NOP  , ALU::NOP  , ALU::NOP  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::MOV  , ALU::NOP  , ALU::MOV  , ALU::INC  , ALU::MOV  , ALU::INC  , ALU::STP  ,
		};

		ALU spc_ALU_Type;

#pragma endregion

#pragma region M6502 functions

		void NZ_A() { P = (uint8_t)((P & 0x7D) | TableNZ[A]); }
		void NZ_X() { P = (uint8_t)((P & 0x7D) | TableNZ[X]); }
		void NZ_Y() { P = (uint8_t)((P & 0x7D) | TableNZ[Y]); }

		void spc_Decode(uint8_t opcode)
		{
			spc_Instr_Type = spc_Instr_Type_List[opcode];
			spc_ALU_Type = spc_ALU_Type_List[opcode];
		}

		void spc_ALU_Operation();

		void spc_Write_Operation();

		void Execute(int cycles);

		void Fetch_Dummy_Interrupt();

		void Fetch_Opcode_No_Interrupt();

		void Fetch1();

		void Fetch1_Branch();

		void Fetch2();

		void Fetch3();

		void ExecuteOneOp();

		void End_SuppressInterrupt()
		{
			spc_Instr_Type = OpT::FONI;
			Fetch_Opcode_No_Interrupt();
		}

		void End()
		{
			Fetch1();
		}

		void End_Branch()
		{
			Fetch1_Branch();
		}

		void ExecuteOne()
		{
			ExecuteOneOp();
			TotalExecutedCycles++;
			Total_spc_Clock_Cycles++;
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

		string CPUDMAStateOAM()
		{
			val_char_1 = replacer;

			string reg_state = "  OAM ST ADDR: ";

			sprintf_s(val_char_1, 5, "%04X", oam_dma_addr);
			reg_state.append(val_char_1, 4);

			reg_state.append("  OAM ADDR: ");

			sprintf_s(val_char_1, 3, "%02X", reg_2003);
			reg_state.append(val_char_1, 2);

			while (reg_state.length() < 87)
			{
				reg_state.append(" ");
			}

			return reg_state;
		}

		string CPUDMAStateDMC()
		{
			val_char_1 = replacer;

			string reg_state = "  DMC ST ADDR: ";

			sprintf_s(val_char_1, 5, "%04X", apu_DMC_Sample_Address);
			reg_state.append(val_char_1, 4);

			reg_state.append("  DMC Length: ");

			sprintf_s(val_char_1, 5, "%04d", apu_DMC_Sample_Length);
			reg_state.append(val_char_1, 4);

			while (reg_state.length() < 87)
			{
				reg_state.append(" ");
			}

			return reg_state;
		}

		string CPUDisassembly()
		{
			string trace_string = "";

			string disasm = spc_Disassemble(PC);

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
			trace_string.append(spc_FlagNget() ? "N" : "n");
			trace_string.append(spc_FlagVget() ? "V" : "v");
			trace_string.append(spc_FlagPget() ? "P" : "p");
			trace_string.append(spc_FlagBget() ? "B" : "b");
			trace_string.append(spc_FlagVget() ? "D" : "d");
			trace_string.append(spc_FlagIget() ? "I" : "i");
			trace_string.append(spc_FlagZget() ? "Z" : "z");
			trace_string.append(spc_FlagCget() ? "C" : "c");
			trace_string.append("  ");

			trace_string.append("Cy:");
			sprintf_s(val_char_1, 17, "%16lld", TotalExecutedCycles);
			trace_string.append(val_char_1, 16);

			trace_string.append(" LY:");
			sprintf_s(val_char_1, 4, "%3u", status_sl);
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

		string spc_Disassemble(uint16_t pc)
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

		uint8_t* spc_SaveState(uint8_t* saver)
		{
			saver = byte_saver(A, saver);
			saver = byte_saver(X, saver);
			saver = byte_saver(Y, saver);
			saver = byte_saver(P, saver);
			saver = short_saver(PC, saver);
			saver = byte_saver(S, saver);

			saver = long_saver(TotalExecutedCycles, saver);
			saver = long_saver(Total_spc_Clock_Cycles, saver);

			saver = byte_saver(opcode2, saver);
			saver = byte_saver(opcode3, saver);
			saver = byte_saver(H, saver);
			saver = byte_saver(Bit, saver);

			saver = short_saver(address_bus, saver);

			saver = int_saver(opcode, saver);
			saver = int_saver(ea, saver);
			saver = int_saver(alu_temp, saver);

			saver = byte_saver(value8, saver);
			saver = byte_saver(temp8, saver);
			saver = short_saver(value16, saver);
			saver = bool_saver(branch_taken, saver);
			saver = bool_saver(my_iflag, saver);
			saver = bool_saver(booltemp, saver);
			saver = int_saver(tempint, saver);
			saver = int_saver(lo, saver);
			saver = int_saver(hi, saver);

			saver = short_saver(spc_IRQ_Type, saver);
			saver = short_saver(spc_Instr_Cycle, saver);

			saver = int_saver((uint32_t)spc_Instr_Type, saver);
			saver = int_saver((uint32_t)spc_ALU_Type, saver);

			return saver;
		}

		uint8_t* spc_LoadState(uint8_t* loader)
		{
			loader = byte_loader(&A, loader);
			loader = byte_loader(&X, loader);
			loader = byte_loader(&Y, loader);
			loader = byte_loader(&P, loader);
			loader = short_loader(&PC, loader);
			loader = byte_loader(&S, loader);

			loader = long_loader(&TotalExecutedCycles, loader);
			loader = long_loader(&Total_spc_Clock_Cycles, loader);

			loader = byte_loader(&opcode2, loader);
			loader = byte_loader(&opcode3, loader);
			loader = byte_loader(&H, loader);
			loader = byte_loader(&Bit, loader);

			loader = short_loader(&address_bus, loader);

			loader = int_loader(&opcode, loader);
			loader = int_loader(&ea, loader);
			loader = int_loader(&alu_temp, loader);

			loader = byte_loader(&value8, loader);
			loader = byte_loader(&temp8, loader);
			loader = short_loader(&value16, loader);
			loader = bool_loader(&branch_taken, loader);
			loader = bool_loader(&my_iflag, loader);
			loader = bool_loader(&booltemp, loader);
			loader = sint_loader(&tempint, loader);
			loader = int_loader(&lo, loader);
			loader = int_loader(&hi, loader);

			loader = short_loader(&spc_IRQ_Type, loader);
			loader = short_loader(&spc_Instr_Cycle, loader);

			loader = int_loader(&spc_Instr_Type_Save, loader);
			loader = int_loader(&spc_ALU_Type_Save, loader);

			spc_Instr_Type = static_cast<OpT>(spc_Instr_Type_Save);
			spc_ALU_Type = static_cast<ALU>(spc_ALU_Type_Save);

			return loader;
		}

	#pragma endregion

	#pragma endregion


	#pragma region savestate
		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(IPL_Enable, saver);
			
			saver = byte_saver(Undoc_Reg, saver);
			saver = byte_saver(Ctrl_Reg, saver);
			saver = byte_saver(DSP_Reg_Addr, saver);
			saver = byte_saver(DSP_Reg_Data, saver);
			saver = byte_saver(Port_0_Reg, saver);
			saver = byte_saver(Port_1_Reg, saver);
			saver = byte_saver(Port_2_Reg, saver);
			saver = byte_saver(Port_3_Reg, saver);
			saver = byte_saver(Timer_0_Reg, saver);
			saver = byte_saver(Timer_1_Reg, saver);
			saver = byte_saver(Timer_2_Reg, saver);
			saver = byte_saver(Counter_0_Reg, saver);
			saver = byte_saver(Counter_1_Reg, saver);
			saver = byte_saver(Counter_2_Reg, saver);
			saver = byte_saver(RAM_F8, saver);
			saver = byte_saver(RAM_F9, saver);

			saver = byte_array_saver(RAM, saver, 0x10000);

			saver = spc_SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&IPL_Enable, loader);
			
			loader = byte_loader(&Undoc_Reg, loader);
			loader = byte_loader(&Ctrl_Reg, loader);
			loader = byte_loader(&DSP_Reg_Addr, loader);
			loader = byte_loader(&DSP_Reg_Data, loader);
			loader = byte_loader(&Port_0_Reg, loader);
			loader = byte_loader(&Port_1_Reg, loader);
			loader = byte_loader(&Port_2_Reg, loader);
			loader = byte_loader(&Port_3_Reg, loader);
			loader = byte_loader(&Timer_0_Reg, loader);
			loader = byte_loader(&Timer_1_Reg, loader);
			loader = byte_loader(&Timer_2_Reg, loader);
			loader = byte_loader(&Counter_0_Reg, loader);
			loader = byte_loader(&Counter_1_Reg, loader);
			loader = byte_loader(&Counter_2_Reg, loader);
			loader = byte_loader(&RAM_F8, loader);
			loader = byte_loader(&RAM_F9, loader);

			loader = byte_array_loader(RAM, loader, 0x10000);
			
			loader = spc_LoadState(loader);

			return loader;
		}

	#pragma endregion



	};

}