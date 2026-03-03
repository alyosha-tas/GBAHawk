#ifndef NES_System_H
#define NES_System_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>
#include <inttypes.h>
#include <cmath>

#ifndef _WIN32
#define sprintf_s snprintf
#endif

using namespace std;

// Notes:

/*
*	Follows NESDev documentation, including code for scrolling
* 
*/

//Message_String = "Uop " + to_string((int)uop) + " cyc: " + to_string(TotalExecutedCycles);

//MessageCallback(Message_String.length());

namespace NESHawk
{
	class MemoryManager;
	class Mappers;

	class NES_System
	{
	public:
		
		Mappers* mapper_pntr = nullptr;

		// Various sync settings and revision dependent behavior
		bool Use_APU_Test_Regs = false;
		bool CPU_Zero_Set_Reset = false;

		uint8_t* Cart_RAM = nullptr;
		uint8_t* ROM = nullptr;
		uint8_t* CHR_ROM = nullptr;
		uint32_t Cart_RAM_Length = 0;
		string Message_String = "";

		uint32_t ROM_Length;
		uint32_t CHR_ROM_Length;

		uint32_t Mapper_Number = 0;

		void (*MessageCallback)(int);
		void (*InputPollCallback)();

		uint8_t(*ReadController)(bool);
		void(*StrobeController)(uint8_t, uint8_t);

	# pragma region General System

		uint32_t RAM_Start_Up[256] =  { 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0 };
		
		uint32_t video_buffer[256 * 240] = { };

		//user configuration 
		uint32_t Compiled_Palette[512] = { };

		void Frame_Advance();

		bool SubFrame_Advance(uint32_t reset_cycle);

		// this function will run one step of the ppu 
		// it will return whether the controller is read or not.
		inline void Single_Step();

		void HardReset();

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

		void RunCpuOne();

		bool Is_Lag;
		bool _irq_apu;
		bool frame_is_done;
		bool Settings_DispBackground = true;
		bool Settings_DispSprites = true;
		bool DMC_DMA_Exec;
		bool Controller_Strobed = false;

		// these variables are for subframe input control
		bool controller_was_latched;
		bool current_strobe;
		bool new_strobe;
		bool alt_lag;
		bool OAM_DMA_Exec = false;
		bool dmc_realign;

		uint8_t DB; //old data bus values from previous reads
		uint8_t oam_dma_byte;
		uint8_t latched4016;
		uint8_t Controller_Strobed_Value;
		uint8_t Previous_Controller_Latch_1;
		uint8_t Previous_Controller_Latch_2;

		uint16_t oam_dma_addr;

		uint32_t sprdma_countdown;
		uint32_t cpu_deadcounter;
		uint32_t oam_dma_index;

		int32_t old_s = 0;

		// General Variables
		uint64_t FrameCycle;
		uint64_t Clock_Update_Cycle;
		uint64_t Cycle_Count;
		uint64_t Last_Controller_Poll_1;
		uint64_t Last_Controller_Poll_2;

		//hardware/state
		uint8_t RAM[0x800] = { };
		uint8_t CIRAM[0x800] = { }; //AKA nametables

		uint8_t Header[0x10] = { };

		NES_System()
		{
			HardReset();
		}

		void On_VBlank()
		{
			// things to do on vblank
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Is_Lag, saver);
			saver = bool_saver(_irq_apu, saver);
			saver = bool_saver(frame_is_done, saver);
			saver = bool_saver(Settings_DispBackground, saver);
			saver = bool_saver(Settings_DispSprites, saver);
			saver = bool_saver(DMC_DMA_Exec, saver);
			saver = bool_saver(Controller_Strobed, saver);

			saver = bool_saver(controller_was_latched, saver);
			saver = bool_saver(current_strobe, saver);
			saver = bool_saver(new_strobe, saver);
			saver = bool_saver(alt_lag, saver);
			saver = bool_saver(OAM_DMA_Exec, saver);
			saver = bool_saver(dmc_realign, saver);

			saver = byte_saver(DB, saver);
			saver = byte_saver(oam_dma_byte, saver);
			saver = byte_saver(latched4016, saver);
			saver = byte_saver(Controller_Strobed_Value, saver);
			saver = byte_saver(Previous_Controller_Latch_1, saver);
			saver = byte_saver(Previous_Controller_Latch_2, saver);

			saver = short_saver(oam_dma_addr, saver);

			saver = int_saver(sprdma_countdown, saver);
			saver = int_saver(cpu_deadcounter, saver);
			saver = int_saver(oam_dma_index, saver);

			saver = int_saver(old_s, saver);

			saver = long_saver(FrameCycle, saver);
			saver = long_saver(Clock_Update_Cycle, saver);
			saver = long_saver(Cycle_Count, saver);
			saver = long_saver(Last_Controller_Poll_1, saver);
			saver = long_saver(Last_Controller_Poll_2, saver);
			
			saver = byte_array_saver(RAM, saver, 0x800);
			saver = byte_array_saver(CIRAM, saver, 0x800);

			if (Cart_RAM_Length != 0)
			{
				saver = byte_array_saver(Cart_RAM, saver, Cart_RAM_Length);
			}

			saver = apu_SaveState(saver);
			saver = ppu_SaveState(saver);
			saver = cpu_SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Is_Lag, loader);
			loader = bool_loader(&_irq_apu, loader);
			loader = bool_loader(&frame_is_done, loader);
			loader = bool_loader(&Settings_DispBackground, loader);
			loader = bool_loader(&Settings_DispSprites, loader);
			loader = bool_loader(&DMC_DMA_Exec, loader);
			loader = bool_loader(&Controller_Strobed, loader);

			loader = bool_loader(&controller_was_latched, loader);
			loader = bool_loader(&current_strobe, loader);
			loader = bool_loader(&new_strobe, loader);
			loader = bool_loader(&alt_lag, loader);
			loader = bool_loader(&OAM_DMA_Exec, loader);
			loader = bool_loader(&dmc_realign, loader);

			loader = byte_loader(&DB, loader);
			loader = byte_loader(&oam_dma_byte, loader);
			loader = byte_loader(&latched4016, loader);
			loader = byte_loader(&Controller_Strobed_Value, loader);
			loader = byte_loader(&Previous_Controller_Latch_1, loader);
			loader = byte_loader(&Previous_Controller_Latch_2, loader);

			loader = short_loader(&oam_dma_addr, loader);

			loader = int_loader(&sprdma_countdown, loader);
			loader = int_loader(&cpu_deadcounter, loader);
			loader = int_loader(&oam_dma_index, loader);

			loader = sint_loader(&old_s, loader);

			loader = long_loader(&FrameCycle, loader);
			loader = long_loader(&Clock_Update_Cycle, loader);
			loader = long_loader(&Cycle_Count, loader);
			loader = long_loader(&Last_Controller_Poll_1, loader);
			loader = long_loader(&Last_Controller_Poll_2, loader);

			loader = byte_array_loader(RAM, loader, 0x800);
			loader = byte_array_loader(CIRAM, loader, 0x800);

			if (Cart_RAM_Length != 0)
			{
				loader = byte_array_loader(Cart_RAM, loader, Cart_RAM_Length);
			}

			loader = apu_LoadState(loader);
			loader = ppu_LoadState(loader);
			loader = cpu_LoadState(loader);

			return loader;
		}

	#pragma endregion

	#pragma region M6502
		#pragma region Variables

		uint16_t cpu_Instr_Cycle;
		uint16_t cpu_IRQ_Type;

		uint32_t cpu_Instr_Type_Save;
		uint32_t cpu_ALU_Type_Save;

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
		uint8_t S;

		uint64_t TotalExecutedCycles;
		uint64_t Total_CPU_Clock_Cycles;

		bool iflag_pending;
		bool RDY_Freeze;
		bool branch_irq_hack;
		bool cpu_First_Check;

		uint8_t opcode2, opcode3;
		uint8_t H;

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

		void cpu_Reset()
		{
			A = 0;
			X = 0;
			Y = 0;
			P = 0x20; // 5th bit always set
			S = 0;
			PC = 0;
			TotalExecutedCycles = 0;

			// revision dependent
			if (CPU_Zero_Set_Reset) { cpu_FlagZset(true); }

			cpu_Instr_Type = OpT::DRMI;
			cpu_IRQ_Type = 2;
			cpu_ALU_Type = ALU::NOP;
			cpu_Instr_Cycle = 0;

			opcode = 0;
			iflag_pending = true;
			RDY = true;
			BCD_Enabled = false;
			cpu_First_Check = false;

			IRQ = false;
			NMI = false;

			IRQ_Br = false;
			NMI_Br = false;
		}

		void cpu_SoftReset()
		{
			cpu_Instr_Type = OpT::DRMI;
			cpu_IRQ_Type = 2;
			cpu_ALU_Type = ALU::NOP;
			cpu_Instr_Cycle = 0;

			opcode = 0;

			iflag_pending = true;
			cpu_FlagIset(true);
		}

		inline bool cpu_FlagCget() { return (P & 0x01) != 0; }
		inline void cpu_FlagCset(bool value) { P = (uint8_t)((P & ~0x01) | (value ? 0x01 : 0x00)); }
		
		inline bool cpu_FlagZget() { return (P & 0x02) != 0; }
		inline void cpu_FlagZset(bool value) { P = (uint8_t)((P & ~0x02) | (value ? 0x02 : 0x00)); }

		inline bool cpu_FlagIget() { return (P & 0x04) != 0; }
		inline void cpu_FlagIset(bool value) { P = (uint8_t)((P & ~0x04) | (value ? 0x04 : 0x00)); }

		inline bool cpu_FlagDget() { return (P & 0x08) != 0; }
		inline void cpu_FlagDset(bool value) { P = (uint8_t)((P & ~0x08) | (value ? 0x08 : 0x00)); }

		inline bool cpu_FlagBget() { return (P & 0x10) != 0; }
		inline void cpu_FlagBset(bool value) { P = (uint8_t)((P & ~0x10) | (value ? 0x10 : 0x00)); }

		inline bool cpu_FlagTget() { return (P & 0x20) != 0; }
		inline void cpu_FlagTset(bool value) { P = (uint8_t)((P & ~0x20) | (value ? 0x20 : 0x00)); }

		inline bool cpu_FlagVget() { return (P & 0x40) != 0; }
		inline void cpu_FlagVset(bool value) { P = (uint8_t)((P & ~0x40) | (value ? 0x40 : 0x00)); }

		inline bool cpu_FlagNget() { return (P & 0x80) != 0; }
		inline void cpu_FlagNset(bool value) { P = (uint8_t)((P & ~0x80) | (value ? 0x80 : 0x00)); }

		// SO pin
		inline void SetOverflow() { cpu_FlagVset(true); }

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
			
			Jam,		// Jam
			INT,		// Interrupts
			DRMI,		// Dummy reads for interrupts
			FONI		// Fetch opcode no interrupts
		};

		OpT cpu_Instr_Type;

		OpT cpu_Instr_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			OpT::BRK  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PH   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::JSR  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PL   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,

			OpT::RTI  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PH   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::JMP  , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::CSI  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::RTS  , OpT::AdXR , OpT::Jam  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::PL   , OpT::Imm  , OpT::Acc  , OpT::Imm  , OpT::JMPI , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::CSI  , OpT::AIYR , OpT::Acc  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,

			OpT::Imm  , OpT::AdXW , OpT::Imm  , OpT::AdXW , OpT::ZPW  , OpT::ZPW  , OpT::ZPW  , OpT::ZPW  , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsW , OpT::AbsW , OpT::AbsW , OpT::AbsW ,
			OpT::Br   , OpT::IIYW , OpT::Jam  , OpT::IIYW , OpT::ZPXW , OpT::ZPXW , OpT::ZPYW , OpT::ZPYW , OpT::Imp  , OpT::AIYW , OpT::Imp  , OpT::AIUW , OpT::AIUW , OpT::AIXW , OpT::AIUW , OpT::AIUW ,
			OpT::Imm  , OpT::AdXR , OpT::Imm  , OpT::AdXR , OpT::ZPR  , OpT::ZPR  , OpT::ZPR  , OpT::ZPR  , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsR , OpT::AbsR ,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYR , OpT::ZPXR , OpT::ZPXR , OpT::ZPYR , OpT::ZPYR , OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYR , OpT::AIXR , OpT::AIXR , OpT::AIYR , OpT::AIYR ,

			OpT::Imm  , OpT::AdXR , OpT::Imm  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
			OpT::Imm  , OpT::AdXR , OpT::Imm  , OpT::AdXRW, OpT::ZPR  , OpT::ZPR  , OpT::ZPRW , OpT::ZPRW , OpT::Imp  , OpT::Imm  , OpT::Imp  , OpT::Imm  , OpT::AbsR , OpT::AbsR , OpT::AbsRW, OpT::AbsRW,
			OpT::Br   , OpT::IIYR , OpT::Jam  , OpT::IIYRW, OpT::ZPXR , OpT::ZPXR , OpT::ZPXRW, OpT::ZPXRW, OpT::Imp  , OpT::AIYR , OpT::Imp  , OpT::AIYRW, OpT::AIXR , OpT::AIXR , OpT::AIXRW, OpT::AIXRW,
		};

		enum class ALU
		{
			// regular ops
			NOP, SEC, SEI, CLC, CLI, BIT, AND, EOR, ORA, ADC, CMP, CPY, CPX, ASL, SBC, ROL,
			LSR, ASR, ROR, RRA, DEX, DEY, TXA, TYA, TXS, TAY, TAX, CLV, TSX, DEC, INY, CLD,
			INC, INX, 

			// int value 33
			// A implied
			ASLA, ROLA, LSRA, RORA,
			
			// unofficial
			LAX, SLO, ANC, RLA, SRE, ARR, SAX, ANE, SHA, SHS, SHY, SHX, LXA,
			LAS, DCP, AXS, ISC, SED,

			// Branch conditions
			BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ,

			// push pull op
			PLP, PLA, PHA, PHP,

			// loads / stores
			STA, STX, STY, LDA, LDX, LDY,

		};

		ALU cpu_ALU_Type_List[256] =
		{
			//  0			1			2			3			4			5			6			7			8			9			A			B			C			D			E			F
			ALU::NOP  , ALU::ORA  , ALU::NOP  , ALU::SLO  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::SLO  , ALU::PHP  , ALU::ORA  , ALU::ASLA , ALU::ANC  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::SLO  ,
			ALU::BPL  , ALU::ORA  , ALU::NOP  , ALU::SLO  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::SLO  , ALU::CLC  , ALU::ORA  , ALU::NOP  , ALU::SLO  , ALU::NOP  , ALU::ORA  , ALU::ASL  , ALU::SLO  ,
			ALU::NOP  , ALU::AND  , ALU::NOP  , ALU::RLA  , ALU::BIT  , ALU::AND  , ALU::ROL  , ALU::RLA  , ALU::PLP  , ALU::AND  , ALU::ROLA , ALU::ANC  , ALU::BIT  , ALU::AND  , ALU::ROL  , ALU::RLA  ,
			ALU::BMI  , ALU::AND  , ALU::NOP  , ALU::RLA  , ALU::NOP  , ALU::AND  , ALU::ROL  , ALU::RLA  , ALU::SEC  , ALU::AND  , ALU::NOP  , ALU::RLA  , ALU::NOP  , ALU::AND  , ALU::ROL  , ALU::RLA  ,

			ALU::NOP  , ALU::EOR  , ALU::NOP  , ALU::SRE  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::SRE  , ALU::PHA  , ALU::EOR  , ALU::LSRA , ALU::ASR  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::SRE  ,
			ALU::BVC  , ALU::EOR  , ALU::NOP  , ALU::SRE  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::SRE  , ALU::CLI  , ALU::EOR  , ALU::NOP  , ALU::SRE  , ALU::NOP  , ALU::EOR  , ALU::LSR  , ALU::SRE  ,
			ALU::NOP  , ALU::ADC  , ALU::NOP  , ALU::RRA  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  , ALU::PLA  , ALU::ADC  , ALU::RORA , ALU::ARR  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  ,
			ALU::BVS  , ALU::ADC  , ALU::NOP  , ALU::RRA  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  , ALU::SEI  , ALU::ADC  , ALU::NOP  , ALU::RRA  , ALU::NOP  , ALU::ADC  , ALU::ROR  , ALU::RRA  ,

			ALU::NOP  , ALU::STA  , ALU::NOP  , ALU::SAX  , ALU::STY  , ALU::STA  , ALU::STX  , ALU::SAX  , ALU::DEY  , ALU::NOP  , ALU::TXA  , ALU::ANE  , ALU::STY  , ALU::STA  , ALU::STX  , ALU::SAX  ,
			ALU::BCC  , ALU::STA  , ALU::NOP  , ALU::SHA  , ALU::STY  , ALU::STA  , ALU::STX  , ALU::SAX  , ALU::TYA  , ALU::STA  , ALU::TXS  , ALU::SHS  , ALU::SHY  , ALU::STA  , ALU::SHX  , ALU::SHA  ,
			ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LAX  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LAX  , ALU::TAY  , ALU::LDA  , ALU::TAX  , ALU::LXA  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LAX  ,
			ALU::BCS  , ALU::LDA  , ALU::NOP  , ALU::LAX  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LAX  , ALU::CLV  , ALU::LDA  , ALU::TSX  , ALU::LAS  , ALU::LDY  , ALU::LDA  , ALU::LDX  , ALU::LAX  ,

			ALU::CPY  , ALU::CMP  , ALU::NOP  , ALU::DCP  , ALU::CPY  , ALU::CMP  , ALU::DEC  , ALU::DCP  , ALU::INY  , ALU::CMP  , ALU::DEX  , ALU::AXS  , ALU::CPY  , ALU::CMP  , ALU::DEC  , ALU::DCP  ,
			ALU::BNE  , ALU::CMP  , ALU::NOP  , ALU::DCP  , ALU::NOP  , ALU::CMP  , ALU::DEC  , ALU::DCP  , ALU::CLD  , ALU::CMP  , ALU::NOP  , ALU::DCP  , ALU::NOP  , ALU::CMP  , ALU::DEC  , ALU::DCP  ,
			ALU::CPX  , ALU::SBC  , ALU::NOP  , ALU::ISC  , ALU::CPX  , ALU::SBC  , ALU::INC  , ALU::ISC  , ALU::INX  , ALU::SBC  , ALU::NOP  , ALU::SBC  , ALU::CPX  , ALU::SBC  , ALU::INC  , ALU::ISC  ,
			ALU::BEQ  , ALU::SBC  , ALU::NOP  , ALU::ISC  , ALU::NOP  , ALU::SBC  , ALU::INC  , ALU::ISC  , ALU::SED  , ALU::SBC  , ALU::NOP  , ALU::ISC  , ALU::NOP  , ALU::SBC  , ALU::INC  , ALU::ISC  ,
		};

		ALU cpu_ALU_Type;

		#pragma endregion

		#pragma region M6502 functions

		void NZ_A() { P = (uint8_t)((P & 0x7D) | TableNZ[A]); }
		void NZ_X() { P = (uint8_t)((P & 0x7D) | TableNZ[X]); }
		void NZ_Y() { P = (uint8_t)((P & 0x7D) | TableNZ[Y]); }

		void cpu_Decode(uint8_t opcode)
		{
			cpu_Instr_Type = cpu_Instr_Type_List[opcode];
			cpu_ALU_Type = cpu_ALU_Type_List[opcode];
		}

		void cpu_ALU_Operation();

		void cpu_Write_Operation();

		void Execute(int cycles);

		void Fetch_Dummy_Interrupt();

		void Fetch_Opcode_No_Interrupt();

		void Fetch1();

		void Fetch1_Branch();

		void Fetch2();

		void Fetch3();

		void ExecuteOneOp();

		void End_ISpecial()
		{
			// no irq flag check here
			Fetch1();
		}

		void End_SuppressInterrupt()
		{
			cpu_Instr_Type = OpT::FONI;
			Fetch_Opcode_No_Interrupt();
		}

		void End()
		{
			iflag_pending = cpu_FlagIget();
			Fetch1();
		}

		void End_Branch()
		{
			iflag_pending = cpu_FlagIget();
			Fetch1_Branch();
		}

		void ExecuteOne()
		{
			RDY_Freeze = false;

			ExecuteOneOp();
			TotalExecutedCycles++;
			Total_CPU_Clock_Cycles++;
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

			string disasm = cpu_Disassemble(PC);
			
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
			trace_string.append(cpu_FlagNget() ? "N" : "n");
			trace_string.append(cpu_FlagVget() ? "V" : "v");
			trace_string.append(cpu_FlagTget() ? "T" : "t");
			trace_string.append(cpu_FlagBget() ? "B" : "b");
			trace_string.append(cpu_FlagVget() ? "D" : "d");
			trace_string.append(cpu_FlagIget() ? "I" : "i");
			trace_string.append(cpu_FlagZget() ? "Z" : "z");
			trace_string.append(cpu_FlagCget() ? "C" : "c");
			trace_string.append(RDY ? "R" : "r");
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

		string cpu_Disassemble(uint16_t pc)
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
			saver = byte_saver(S, saver);

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
			saver = byte_saver(H, saver);

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

			saver = short_saver(cpu_IRQ_Type, saver);
			saver = short_saver(cpu_Instr_Cycle, saver);

			saver = int_saver((uint32_t)cpu_Instr_Type, saver);
			saver = int_saver((uint32_t)cpu_ALU_Type, saver);

			return saver;
		}

		uint8_t* cpu_LoadState(uint8_t* loader)
		{
			loader = byte_loader(&A, loader);
			loader = byte_loader(&X, loader);
			loader = byte_loader(&Y, loader);
			loader = byte_loader(&P, loader);
			loader = short_loader(&PC, loader);
			loader = byte_loader(&S, loader);

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
			loader = byte_loader(&H, loader);

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

			loader = short_loader(&cpu_IRQ_Type, loader);
			loader = short_loader(&cpu_Instr_Cycle, loader);

			loader = int_loader(&cpu_Instr_Type_Save, loader);
			loader = int_loader(&cpu_ALU_Type_Save, loader);

			cpu_Instr_Type = static_cast<OpT>(cpu_Instr_Type_Save);
			cpu_ALU_Type = static_cast<ALU>(cpu_ALU_Type_Save);

			return loader;
		}

		#pragma endregion

	#pragma endregion

	#pragma region PPU
		
		//blargg: Reading from $2007 when the VRAM address is $3fxx will fill the internal read buffer with the contents at VRAM address $3fxx, in addition to reading the palette RAM. 

		//static const uint8_t powerUpPalette[] =
		//{
		//    0x3F,0x01,0x00,0x01, 0x00,0x02,0x02,0x0D, 0x08,0x10,0x08,0x24, 0x00,0x00,0x04,0x2C,
		//    0x09,0x01,0x34,0x03, 0x00,0x04,0x00,0x14, 0x08,0x3A,0x00,0x02, 0x00,0x20,0x2C,0x08
		//};

		uint32_t  POWER_ON_PALETTE[32] = { 0x09,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
										   0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08 };

		const static uint32_t PPU_PHASE_VBL = 0;
		const static uint32_t PPU_PHASE_BG = 1;
		const static uint32_t PPU_PHASE_OBJ = 2;

		// other values of action are reserved for possibly needed expansions, but this passes
		// ppu_open_bus for now.
		const static uint32_t DecayType_None = 0; // if there is no action, decrement the timer
		const static uint32_t DecayType_All = 1; // reset the timer for all bits (reg 2004 / 2007 (non-palette)
		const static uint32_t DecayType_High = 2; // reset the timer for high 3 bits (reg 2002)
		const static uint32_t DecayType_Low = 3; // reset the timer for all low 6 bits (reg 2007 (palette))

		inline bool PPUON() { return show_bg_new || show_obj_new; }
		inline bool ppu_Is_Rendering() { return (status_sl == 261) || (status_sl < 240); }

		bool ppu_HasClockPPU;
		bool ppu_Chop_Dot;
		bool ppu_IdleSynch;
		bool ppu_Dot_Was_Skipped;
		bool ppu_Toggle_w;

		bool ppu_Color_Disable; //Color disable (0: normal color; 1: AND all palette entries with 110000, effectively producing a monochrome display)
		bool ppu_Show_BG_Leftmost; //Show leftmost 8 pixels of background
		bool ppu_Show_OBJ_Leftmost; //Show sprites in leftmost 8 pixels
		bool ppu_Show_BG; //Show background
		bool ppu_Show_OBJ; //Show sprites
		bool ppu_Intense_Green; //Intensify greens (and darken other colors)
		bool ppu_Intense_Blue; //Intensify blues (and darken other colors)
		bool ppu_Intense_Red; //Intensify reds (and darken other colors)

		bool ppu_Vram_Incr32; //(0: increment by 1, going across; 1: increment by 32, going down)
		bool ppu_OBJ_Pattern_High; //Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000)
		bool ppu_BG_Pattern_High; //Background pattern table address (0: $0000; 1: $1000)
		bool ppu_OBJ_Size_16; //Sprite size (0: 8x8 sprites; 1: 8x16 sprites)
		bool ppu_Layer; //PPU layer select (should always be 0 in the NES; some Nintendo arcade boards presumably had two PPUs)
		bool ppu_Vblank_NMI_Gen; //Vertical blank NMI generation (0: off; 1: on)

		bool ppu_Reg2002_objoverflow;  //Sprite overflow. The PPU can handle only eight sprites on one scanline and sets this bit if it starts drawing sprites.
		bool ppu_Reg2002_objhit; //Sprite 0 overlap.  Set when a nonzero pixel of sprite 0 is drawn overlapping a nonzero background pixel.  Used for raster timing.
		bool ppu_Reg2002_vblank_active;  //Vertical blank start (0: has not started; 1: has started)
		bool ppu_Reg2002_vblank_active_pending; //set if Reg2002_vblank_active is pending
		bool ppu_Reg2002_vblank_clear_pending; //ppu's clear of vblank flag is pending

		bool sprite_eval_write;
		bool sprite_zero_in_range;
		bool sprite_zero_go;
		bool ppu_was_on;
		bool race_2006;
		bool show_bg_new; //Show background
		bool show_obj_new; //Show sprites
		bool do_vbl;
		bool do_active_sl;
		bool do_pre_vbl;
		bool nmi_destiny;
		bool evenOddDestiny;
		bool last_pipeline;
		bool ppu_Commit_Read;
		bool ppu_Can_Corrupt;
		bool ppu_Sprite_Draw_Glitch;

		uint8_t ppu_OAM_Corrupt_Addr;
		uint8_t VRAMBuffer;
		uint8_t read_value;
		uint8_t reg_2003;
		uint8_t glitch_2007_iter;
		uint8_t glitch_2007_addr;
		uint8_t ppu_temp_oam_y;
		uint8_t ppu_temp_oam_ind = 0;
		uint8_t ppu_BG_NT_Addr;

		// this uint8_t is used to simulate open bus reads and writes
		// it should be modified by every read and write to a ppu register
		uint8_t ppu_Open_Bus;

		uint16_t ppu_Reg_v;
		uint16_t ppu_Reg_t;
		uint16_t ppu_Scroll_x;
		uint16_t ppu_Raster_Pos;
		uint16_t ppu_VRAM_Address;
		uint16_t ppu_BG_Pattern_0;
		uint16_t ppu_BG_Pattern_1;
		uint16_t ppu_Next_BG_Pt_0;
		uint16_t ppu_Next_BG_Pt_1;

		uint32_t cpu_step, cpu_stepcounter;
		uint32_t ppuphase;
		uint32_t ppudead;
		uint32_t NMI_PendingInstructions;
		uint32_t intensity_lsl_6;
		uint32_t status_sl;
		uint32_t status_cycle;
		uint32_t spr_true_count;
		uint32_t soam_index;
		uint32_t oam_index;
		uint32_t yp;
		uint32_t yp_sp;
		uint32_t target;
		uint32_t spriteHeight;
		uint32_t install_2006;
		uint32_t install_2001;
		uint32_t start_up_offset;
		uint32_t NMI_offset;
		uint32_t yp_shift;
		uint32_t sprite_eval_cycle;
		uint32_t xt;
		uint32_t xp;
		uint32_t xstart;
		uint32_t s;
		uint32_t ppu_aux_index;
		uint32_t line;
		uint32_t patternNumber;
		uint32_t patternAddress;
		uint32_t ppu_Sprite_Draw_Cycle;

		uint32_t pixelcolor_latch_1;
		uint32_t pixelcolor_latch_2;

		// sequential reads and RMW instructions cause glitches (but not STA instructions)
		uint64_t double_2007_read;

		uint64_t Total_PPU_Clock_Cycles;

		uint8_t glitchy_reads_2003[8] = { };
		uint8_t OAM[0x100] = { };
		uint8_t PALRAM[0x20] = { };

		uint8_t ppu_BG_Attr[3] = { };

		uint8_t soam[256] = { };

		struct Sprite_Shifter_Struct
		{
			bool X_Start;
			uint16_t X;
			uint8_t Attr;
			uint8_t Pattern_0;
			uint8_t Pattern_1;
		} ppu_Sprite_Shifters[8];

		uint16_t xbuf[256 * 240] = { };

		uint32_t ppu_open_bus_decay_timer[8] = { };

		void (*NTViewCallback)(void);
		void (*PPUViewCallback)(void);

		int NTView_Scanline = 0;
		int PPUView_Scanline = 0;

		uint8_t ppubus_read(uint32_t addr);

		void ppubus_clock(uint32_t addr);

		uint8_t ppubus_peek(uint32_t addr);

		void ppu_Run();

		void ppu_SoftReset()
		{
			//this hasn't been brought up to date since NEShawk was first made.
			//in particular http://wiki.nesdev.com/w/index.php/PPU_power_up_state should be studied, but theres no use til theres test cases
			ppu_Reset();
		}

		void ppu_Reset()
		{
			ppu_Chop_Dot = true;
			ppu_Dot_Was_Skipped = false;
			ppudead = 1;
			ppu_IdleSynch = true;
			ppu_Open_Bus = 0;

			ppu_Reg_v = 0;
			ppu_Reg_t = 0;
			ppu_Scroll_x = 0;
			ppu_Raster_Pos = 0;
			ppu_VRAM_Address = 0;
			ppu_BG_Pattern_0 = 0;
			ppu_BG_Pattern_1 = 0;
			ppu_Next_BG_Pt_0 = 0;
			ppu_Next_BG_Pt_1 = 0;
			ppu_BG_Attr[0] = 0;
			ppu_BG_Attr[1] = 0;
			ppu_BG_Attr[2] = 0;

			ppu_temp_oam_y = 0;
			ppu_temp_oam_ind = 0;
			ppu_BG_NT_Addr = 0;

			double_2007_read = 0;
			start_up_offset = 5;

			ppu_Color_Disable = false;
			ppu_Show_BG_Leftmost = false;
			ppu_Show_OBJ_Leftmost = false;
			ppu_Show_BG = false;
			ppu_Show_OBJ = false;
			ppu_Intense_Green = false;
			ppu_Intense_Blue = false;
			ppu_Intense_Red = false;

			ppu_Vram_Incr32 = false;
			ppu_OBJ_Pattern_High = false;
			ppu_BG_Pattern_High = false;
			ppu_OBJ_Size_16 = false;
			ppu_Layer = false;
			ppu_Vblank_NMI_Gen = false;
			ppu_Commit_Read = false;
			ppu_Can_Corrupt = false;
			ppu_Sprite_Draw_Glitch = false;

			ppu_OAM_Corrupt_Addr = 0;
			intensity_lsl_6 = 0;

			ppu_Reg2002_objoverflow = false;
			ppu_Reg2002_objhit = false;
			ppu_Reg2002_vblank_active = false;
			reg_2003 = 0;
			glitch_2007_iter = 0;
			glitch_2007_addr = 0;
			ppu_Toggle_w = false;
			VRAMBuffer = 0;

			status_cycle = 0;
			status_sl = 0;

			cpu_step = 0;
			cpu_stepcounter = 0;

			NMI_PendingInstructions = 0;

			spr_true_count = 0;
			sprite_eval_write = false;
			read_value = 0;
			sprite_zero_go = false;
			sprite_zero_in_range = false;
			soam_index = 0;
			oam_index = 0;
			yp = 0;
			yp_sp = 0;
			target = 0;
			ppu_was_on = false;
			spriteHeight = 0;
			install_2006 = 0;
			race_2006 = false;
			install_2001 = 0;
			show_bg_new = false;
			show_obj_new = false;
			evenOddDestiny = false;

			ppuphase = 0;

			ppu_Reg2002_vblank_active_pending = 0;
			ppu_Reg2002_vblank_clear_pending = 0;

			Total_PPU_Clock_Cycles = 0;
			do_vbl = 0;
			do_active_sl = 0;
			do_pre_vbl = 0;

			nmi_destiny = 0;
			NMI_offset = 0;
			yp_shift = 0;
			sprite_eval_cycle = 0;
			xt = 0;
			xp = 0;
			xstart = 0;

			s = 0;
			ppu_aux_index = 0;
			line = 0;
			patternNumber = 0;
			patternAddress = 0;
			ppu_Sprite_Draw_Cycle = 0;

			for (int i = 0; i < 8; i++)
			{
				ppu_open_bus_decay_timer[i] = 0;
			}

			for (int i = 0; i < 8; i++)
			{
				glitchy_reads_2003[i] = 0;
			}

			for (int i = 0; i < 256; i++)
			{
				OAM[i] = 0;
				soam[i] = 0xFF;
			}

			for (int i = 0; i < 32; i++)
			{
				PALRAM[i] = POWER_ON_PALETTE[i];
			}

			for (int i = 0; i < 8; i++)
			{
				ppu_Sprite_Shifters[i].X_Start = false;
				ppu_Sprite_Shifters[i].X = 0;
				ppu_Sprite_Shifters[i].Attr = 0;
				ppu_Sprite_Shifters[i].Pattern_0 = 0;
				ppu_Sprite_Shifters[i].Pattern_1 = 0;
			}
		}

		void ppu_Install_Latches()
		{
			ppu_Reg_v = ppu_Reg_t;
		}

		void ppu_Install_h_Latches()
		{
			ppu_Reg_v &= 0xFBE0;
			ppu_Reg_v |= (ppu_Reg_t & 0x41F);
		}

		void ppu_Install_v_Latches()
		{
			ppu_Reg_v &= 0x41F;
			ppu_Reg_v |= (ppu_Reg_t & 0xFBE0);
		}

		void ppu_Increment_hsc()
		{
			if ((ppu_Reg_v & 0x001F) == 31) // if coarse X == 31
			{
				ppu_Reg_v &= ~0x001F;         // coarse X = 0
				ppu_Reg_v ^= 0x0400;           // switch horizontal nametable
			}
			else
			{
				ppu_Reg_v += 1;                // increment coarse X
			}

			ppu_Reg_v &= 0x7FFF;
		}

		void ppu_Increment_vs()
		{
			if ((ppu_Reg_v & 0x7000) != 0x7000)			// if fine Y < 7
			{
				ppu_Reg_v += 0x1000;					// increment fine Y
			}
			else
			{
				ppu_Reg_v &= ~0x7000;					// fine Y = 0
				int y = (ppu_Reg_v & 0x03E0) >> 5;      // let y = coarse Y
				if (y == 29)
				{
					y = 0;								// coarse Y = 0
					ppu_Reg_v ^= 0x0800;				// switch vertical nametable
				}
				else if (y == 31)
				{
					y = 0;								// coarse Y = 0, nametable not switched
				}
				else
				{
					y += 1;								// increment coarse Y
				}

				ppu_Reg_v = (ppu_Reg_v & ~0x03E0) | (y << 5); // put coarse Y back into v
			}

			ppu_Reg_v &= 0x7FFF;
		}

		int ppu_Get_NT_Read()
		{
			return 0x2000 | (ppu_Reg_v & 0x0FFF);
		}

		int ppu_Get_AT_Read()
		{
			return 0x23C0 | (ppu_Reg_v & 0x0C00) | ((ppu_Reg_v >> 4) & 0x38) | ((ppu_Reg_v >> 2) & 0x07);
		}

		void ppu_Increment_2007(bool rendering, bool by32)
		{
			// Glitch: both horizontal and vertical increment triggered
			if (rendering)
			{
				ppu_Increment_hsc();
				ppu_Increment_vs();
				return;
			}

			if (by32)
			{
				ppu_Reg_v += 32;
			}
			else
			{
				ppu_Reg_v += 1;
			}

			ppu_Reg_v &= 0x7FFF;
		}

		uint8_t ppu_ReadReg(int addr)                                                   // Register Reads
		{
			uint8_t ret_spec;
			switch (addr)
			{
				case 0: return read_2000();
				case 1: return read_2001();
				case 2: return read_2002();
				case 3: return read_2003();
				case 4: return read_2004();
				case 5: return read_2005();
				case 6: return read_2006();
				case 7:
					{
						if (TotalExecutedCycles == double_2007_read)
						{
							ret_spec = ppu_Open_Bus;
						}
						else
						{
							ret_spec = read_2007();
						}

						//Message_String = "rd " + to_string(ppu_VRAM_Address) + " cyc: " + to_string(TotalExecutedCycles);

						//MessageCallback(Message_String.length());

						glitch_2007_iter = 0;

						double_2007_read = TotalExecutedCycles + 1;

						return ret_spec;
					}
				default: throw new exception();
			}
		}

		uint8_t read_2000() { return ppu_Open_Bus; }

		uint8_t read_2001() { return ppu_Open_Bus; }

		uint8_t read_2002()
		{
			uint8_t ret = peek_2002();
			/*
			if (do_the_reread_2002 > 0)
			{
				if (Reg2002_vblank_active || Reg2002_vblank_active_pending)
					Console.WriteLine("reread 2002");
			}
			*/

			// reading from $2002 resets the destination for $2005 and $2006 writes
			ppu_Toggle_w = false;
			ppu_Reg2002_vblank_active = false;
			ppu_Reg2002_vblank_active_pending = false;

			// update the open bus here
			ppu_Open_Bus = ret;
			PpuOpenBusDecay(DecayType_High);
			return ret;
		}

		uint8_t read_2003() { return ppu_Open_Bus; }

		uint8_t read_2004()
		{
			uint8_t ret;

			// behaviour depends on whether things are being rendered or not
			if (PPUON())
			{
				if (ppu_Is_Rendering())
				{
					if (status_cycle == 0)
					{
						ret = soam[0];
					}
					else if (status_cycle <= 64)
					{
						ret = 0xFF; // during this time all reads return FF
					}
					else if (status_cycle <= 256)
					{
						ret = read_value;
					}
					else if (status_cycle <= 320)
					{
						ret = read_value;
					}
					else
					{
						ret = soam[0];
					}
				}
				else
				{
					ret = OAM[reg_2003];
				}
			}
			else
			{
				ret = OAM[reg_2003];
			}

			ppu_Open_Bus = ret;
			PpuOpenBusDecay(DecayType_All);
			return ret;
		}

		uint8_t read_2005() { return ppu_Open_Bus; }

		uint8_t read_2006() { return ppu_Open_Bus; }

		uint8_t read_2007();

		void ppu_WriteReg(int addr, uint8_t value)                                       // Register Writes
		{
			ppu_Open_Bus = value;

			switch (addr & 0x07)
			{
				case 0: write_2000(value); break;
				case 1: write_2001(value); break;
				case 2: write_2002(value); break;
				case 3: write_2003(addr, value); break;
				case 4: write_2004(value); break;
				case 5: write_2005(value); break;
				case 6: write_2006(value); break;
				case 7:
					if (TotalExecutedCycles == double_2007_read)
					{
						// The ppu address bus also doubles as the data bus.
						// when RMW instructions are used, the address behaves in a glitchy manner
						// this is console / revision	 depdendent
						// ordinary write instructions are not effected

						if ((cpu_Instr_Type == OpT::AbsRW) || (cpu_Instr_Type == OpT::AdXRW) || (cpu_Instr_Type == OpT::AIXRW) ||
							(cpu_Instr_Type == OpT::AIYRW) || (cpu_Instr_Type == OpT::IIYRW))
						{
							write_2007_Glitch(value);

							glitch_2007_iter = 1;
						}
						else
						{
							write_2007(value);
						}
					}
					else
					{
						write_2007(value);
					}

					double_2007_read = TotalExecutedCycles + 1;

					break;
				default: throw new exception();
			}
		}

		void write_2000(uint8_t value)
		{
			if (!ppu_Vblank_NMI_Gen && ((value & 0x80) != 0) && (ppu_Reg2002_vblank_active) && !ppu_Reg2002_vblank_clear_pending)
			{
				//if we just unleashed the vblank interrupt then activate it now
				//if (ppudead != 1)
				NMI_PendingInstructions = 2;
			}

			ppu_Vram_Incr32 = ((value >> 2) & 1) == 1;
			ppu_OBJ_Pattern_High = ((value >> 3) & 1) == 1;
			ppu_BG_Pattern_High = ((value >> 4) & 1) == 1;
			ppu_OBJ_Size_16 = ((value >> 5) & 1) == 1;
			ppu_Layer = ((value >> 6) & 1) == 1;
			ppu_Vblank_NMI_Gen = ((value >> 7) & 1) == 1;

			ppu_Reg_t &= 0xF3FF;
			ppu_Reg_t |= ((uint16_t)value & 3) << 10;
		}

		void write_2001(uint8_t value)
		{
			ppu_Color_Disable = (value & 1) == 1;
			ppu_Show_BG_Leftmost = ((value >> 1) & 1) == 1;
			ppu_Show_OBJ_Leftmost = ((value >> 2) & 1) == 1;
			ppu_Show_BG = ((value >> 3) & 1) == 1;
			ppu_Show_OBJ = ((value >> 4) & 1) == 1;
			ppu_Intense_Blue = ((value >> 6) & 1) == 1;
			ppu_Intense_Red = ((value >> 7) & 1) == 1;
			ppu_Intense_Green = ((value >> 5) & 1) == 1;
			intensity_lsl_6 = ((value >> 5) & 7) << 6;

			install_2001 = 1;
		}

		void write_2002(uint8_t value) {}

		void write_2003(int addr, uint8_t value)
		{
			// in NTSC this does several glitchy things to corrupt OAM
			// commented out for now until better understood
			uint8_t temp = (uint8_t)(reg_2003 & 0xF8);
			uint8_t temp_2 = (uint8_t)(addr >> 16 & 0xF8);
			/*
			for (int i=0;i<8;i++)
			{
				glitchy_reads_2003[i] = OAM[temp + i];
				//OAM[temp_2 + i] = glitchy_reads_2003[i];
			}
			*/
			reg_2003 = value;
		}

		void write_2004(uint8_t value)
		{
			if ((reg_2003 & 3) == 2)
			{
				//some of the OAM bits are unwired so we mask them out here
				//otherwise we just write this value and move on to the next oam byte
				value &= 0xE3;
			}
			if (ppu_Is_Rendering())
			{
				// don't write to OAM if the screen is on and we are in the active display area
				// this impacts sprite evaluation
				if (show_bg_new || show_obj_new)
				{
					// glitchy increment of OAM index
					oam_index += 4;
					reg_2003 += 4;

					oam_index &= 0xFFC;
					reg_2003 &= 0xFC;
				}
				else
				{
					OAM[reg_2003] = value;
					reg_2003++;
				}
			}
			else
			{
				OAM[reg_2003] = value;
				reg_2003++;
			}
		}

		void write_2005(uint8_t value)
		{
			if (!ppu_Toggle_w)
			{
				ppu_Reg_t &= 0xFFE0;
				ppu_Reg_t |= ((uint16_t)(value >> 3) & 0x1F);
				ppu_Scroll_x = value & 7;
			}
			else
			{
				ppu_Reg_t &= 0x0C1F;
				ppu_Reg_t |= (((uint16_t)(value >> 3) & 0x1F) << 5);
				ppu_Reg_t |= (((uint16_t)value & 7) << 12);
			}

			ppu_Toggle_w ^= true;
		}

		void write_2006(uint8_t value)
		{

			if (!ppu_Toggle_w)
			{
				// Note: upper bit of address cleared (7 vits cleared even though we only write 6)
				ppu_Reg_t &= 0xFF;
				ppu_Reg_t |= (((uint16_t)value & 0x3F) << 8);
			}
			else
			{
				ppu_Reg_t &= 0xFF00;
				ppu_Reg_t |= value;

				// testing indicates that this operation is delayed by 3 pixels
				//install_latches();				
				install_2006 = 3;
			}

			ppu_Toggle_w ^= true;
		}

		void write_2007(uint8_t value);

		void write_2007_Glitch(uint8_t value);

		uint8_t ppu_PeekReg(int addr)
		{
			switch (addr)
			{
				case 0: return peek_2000(); case 1: return peek_2001(); case 2: return peek_2002(); case 3: return peek_2003();
				case 4: return peek_2004(); case 5: return peek_2005(); case 6: return peek_2006(); case 7: return peek_2007();
				default: throw new exception();
			}
		}

		uint8_t peek_2000() { return ppu_Open_Bus; }

		uint8_t peek_2001() { return ppu_Open_Bus; }

		uint8_t peek_2002()
		{
			uint8_t ret = (uint8_t)(ppu_Open_Bus & 0x1F);

			if (ppu_Reg2002_objoverflow)
			{
				ret |= 0x20;
			}

			if (ppu_Reg2002_objhit)
			{
				ret |= 0x40;
			}

			if (ppu_Reg2002_vblank_active)
			{
				ret |= 0x80;
			}

			return ret;
		}

		uint8_t peek_2003() { return ppu_Open_Bus; }

		uint8_t peek_2004() { return OAM[reg_2003]; }

		uint8_t peek_2005() { return ppu_Open_Bus; }

		uint8_t peek_2006() { return ppu_Open_Bus; }

		uint8_t peek_2007()
		{
			int addr = ppu_Reg_v;

			//ordinarily we return the buffered values
			uint8_t ret = VRAMBuffer;

			//in any case, we read from the ppu bus
			// can't do this in peek; updates the value that will be used later
			// VRAMBuffer = ppubus_peek(addr);

			//but reads from the palette are implemented in the PPU and return immediately
			if ((addr & 0x3F00) == 0x3F00)
			{
				//TODO apply greyscale shit?
				ret = PALRAM[addr & 0x1F];
			}

			return ret;
		}

		void PpuOpenBusDecay(uint32_t action)
		{
			switch (action)
			{
				case DecayType_None:
					for (int i = 0; i < 8; i++)
					{
						if (ppu_open_bus_decay_timer[i] == 0)
						{
							ppu_Open_Bus = (uint8_t)(ppu_Open_Bus & (0xff - (1 << i)));
							ppu_open_bus_decay_timer[i] = 1786840; // about 1 second worth of cycles
						}
						else
						{
							ppu_open_bus_decay_timer[i]--;
						}
					}
					break;
				case DecayType_All:
					for (int i = 0; i < 8; i++)
					{
						ppu_open_bus_decay_timer[i] = 1786840;
					}
					break;
				case DecayType_High:
					ppu_open_bus_decay_timer[7] = 1786840;
					ppu_open_bus_decay_timer[6] = 1786840;
					ppu_open_bus_decay_timer[5] = 1786840;
					break;
				case DecayType_Low:
					for (int i = 0; i < 6; i++)
					{
						ppu_open_bus_decay_timer[i] = 1786840;
					}
					break;
			}
		}

		void pipeline(int pixelcolor, int row_check)
		{
			if (row_check > 1)
			{
				if (ppu_Color_Disable)
					pixelcolor_latch_2 &= 0x30;

				xbuf[target-2] = (uint16_t)(pixelcolor_latch_2 | intensity_lsl_6);
			}

			pixelcolor_latch_2 = pixelcolor_latch_1;
			pixelcolor_latch_1 = pixelcolor;
		}

		//address line 3 relates to the pattern table fetch occuring (the PPU always makes them in pairs).
		int get_ptread(int par)
		{
			int hi = ppu_BG_Pattern_High ? 1 : 0;
			return (hi << 0xC) | (par << 0x4) | ((ppu_Reg_v >> 12) & 7);
		}

		void Read_bgdata(int cycle, int i)
		{
			uint8_t at = 0;
			
			switch (cycle)
			{
				case 0:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (ppu_Get_NT_Read() & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 1:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = ppu_Get_NT_Read();
						ppu_BG_NT_Addr = ppubus_read(ppu_VRAM_Address);
					}
					break;
				case 2:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (ppu_Get_AT_Read() & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 3:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = ppu_Get_AT_Read();

						at = ppubus_read(ppu_VRAM_Address);

						// modify at to get appropriate palette shift
						if ((ppu_Reg_v & 0x40) != 0) at >>= 4;
						if ((ppu_Reg_v & 2) != 0) at >>= 2;
						at &= 0x03;
						at <<= 2;
						ppu_BG_Attr[2] = at;
					}
					break;
				case 4:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (get_ptread(ppu_BG_NT_Addr) & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 5:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = get_ptread(ppu_BG_NT_Addr);
						ppu_Next_BG_Pt_0 = ppubus_read(ppu_VRAM_Address);
					}
					break;
				case 6:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (get_ptread(ppu_BG_NT_Addr) & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 7:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = get_ptread(ppu_BG_NT_Addr);
						ppu_VRAM_Address |= 8;
						ppu_Next_BG_Pt_1 = ppubus_read(ppu_VRAM_Address);
					}
					break;
			}
		}

		void TickPPU(bool);

		//not quite emulating all the NES power up behavior
		//since it is known that the NES ignores writes to some
		//register before around a full frame, but no games
		//should write to those regs during that time, it needs
		//to wait for vblank
		void NewDeadPPU(uint32_t cycles_to_run)
		{
			for (uint32_t i = 0; i < cycles_to_run; i++)
			{
				ppu_Run();

				status_cycle += 1;

				if (status_cycle == 241 * 341 - start_up_offset)
				{
					ppudead--;

					status_cycle = 0;

					status_sl = 241;

					do_vbl = true;

					frame_is_done = true;
				}

				FrameCycle += 1;
			}
		}

		uint8_t BitReverse(uint8_t b)
		{
			uint8_t ret = (uint8_t)(b >> 7);

			uint8_t bit_0 = (uint8_t)(b & 1);
			uint8_t bit_1 = (uint8_t)(b & 2);
			uint8_t bit_2 = (uint8_t)(b & 4);
			uint8_t bit_3 = (uint8_t)(b & 8);
			uint8_t bit_4 = (uint8_t)(b & 0x10);
			uint8_t bit_5 = (uint8_t)(b & 0x20);
			uint8_t bit_6 = (uint8_t)(b & 0x40);

			ret |= (uint8_t)(bit_0 << 7);
			ret |= (uint8_t)(bit_1 << 5);
			ret |= (uint8_t)(bit_2 << 3);
			ret |= (uint8_t)(bit_3 << 1);
			ret |= (uint8_t)(bit_4 >> 1);
			ret |= (uint8_t)(bit_5 >> 3);
			ret |= (uint8_t)(bit_6 >> 5);

			return ret;
		}

		uint8_t* ppu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ppu_HasClockPPU, saver);
			saver = bool_saver(ppu_Chop_Dot, saver);
			saver = bool_saver(ppu_IdleSynch, saver);
			saver = bool_saver(ppu_Dot_Was_Skipped, saver);
			saver = bool_saver(ppu_Toggle_w, saver);

			saver = bool_saver(ppu_Color_Disable, saver);
			saver = bool_saver(ppu_Show_BG_Leftmost, saver);
			saver = bool_saver(ppu_Show_OBJ_Leftmost, saver);
			saver = bool_saver(ppu_Show_BG, saver);
			saver = bool_saver(ppu_Show_OBJ, saver);
			saver = bool_saver(ppu_Intense_Green, saver);
			saver = bool_saver(ppu_Intense_Blue, saver);
			saver = bool_saver(ppu_Intense_Red, saver);

			saver = bool_saver(ppu_Vram_Incr32, saver);
			saver = bool_saver(ppu_OBJ_Pattern_High, saver);
			saver = bool_saver(ppu_BG_Pattern_High, saver);
			saver = bool_saver(ppu_OBJ_Size_16, saver);
			saver = bool_saver(ppu_Layer, saver);
			saver = bool_saver(ppu_Vblank_NMI_Gen, saver);

			saver = bool_saver(ppu_Reg2002_objoverflow, saver);
			saver = bool_saver(ppu_Reg2002_objhit, saver);
			saver = bool_saver(ppu_Reg2002_vblank_active, saver);
			saver = bool_saver(ppu_Reg2002_vblank_active_pending, saver);
			saver = bool_saver(ppu_Reg2002_vblank_clear_pending, saver);

			saver = bool_saver(sprite_eval_write, saver);
			saver = bool_saver(sprite_zero_in_range, saver);
			saver = bool_saver(sprite_zero_go, saver);
			saver = bool_saver(ppu_was_on, saver);
			saver = bool_saver(race_2006, saver);
			saver = bool_saver(show_bg_new, saver);
			saver = bool_saver(show_obj_new, saver);
			saver = bool_saver(do_vbl, saver);
			saver = bool_saver(do_active_sl, saver);
			saver = bool_saver(do_pre_vbl, saver);
			saver = bool_saver(nmi_destiny, saver);
			saver = bool_saver(evenOddDestiny, saver);
			saver = bool_saver(last_pipeline, saver);
			saver = bool_saver(ppu_Commit_Read, saver);
			saver = bool_saver(ppu_Can_Corrupt, saver);
			saver = bool_saver(ppu_Sprite_Draw_Glitch, saver);

			saver = byte_saver(ppu_OAM_Corrupt_Addr, saver);
			saver = byte_saver(VRAMBuffer, saver);
			saver = byte_saver(read_value, saver);
			saver = byte_saver(reg_2003, saver);
			saver = byte_saver(glitch_2007_iter, saver);
			saver = byte_saver(glitch_2007_addr, saver);
			saver = byte_saver(ppu_temp_oam_y, saver);
			saver = byte_saver(ppu_temp_oam_ind, saver);
			saver = byte_saver(ppu_BG_NT_Addr, saver);

			saver = byte_saver(ppu_Open_Bus, saver);

			saver = short_saver(ppu_Reg_v, saver);
			saver = short_saver(ppu_Reg_t, saver);
			saver = short_saver(ppu_Scroll_x, saver);
			saver = short_saver(ppu_Raster_Pos, saver);
			saver = short_saver(ppu_VRAM_Address, saver);
			saver = short_saver(ppu_BG_Pattern_0, saver);
			saver = short_saver(ppu_BG_Pattern_1, saver);
			saver = short_saver(ppu_Next_BG_Pt_0, saver);
			saver = short_saver(ppu_Next_BG_Pt_1, saver);

			saver = int_saver(cpu_step, saver);
			saver = int_saver(cpu_stepcounter, saver);
			saver = int_saver(ppuphase, saver);
			saver = int_saver(ppudead, saver);
			saver = int_saver(NMI_PendingInstructions, saver);
			saver = int_saver(intensity_lsl_6, saver);
			saver = int_saver(status_sl, saver);
			saver = int_saver(status_cycle, saver);
			saver = int_saver(spr_true_count, saver);
			saver = int_saver(soam_index, saver);
			saver = int_saver(oam_index, saver);
			saver = int_saver(yp, saver);
			saver = int_saver(yp_sp, saver);
			saver = int_saver(target, saver);
			saver = int_saver(spriteHeight, saver);
			saver = int_saver(install_2006, saver);
			saver = int_saver(install_2001, saver);
			saver = int_saver(start_up_offset, saver);
			saver = int_saver(NMI_offset, saver);
			saver = int_saver(yp_shift, saver);
			saver = int_saver(sprite_eval_cycle, saver);
			saver = int_saver(xt, saver);
			saver = int_saver(xp, saver);
			saver = int_saver(xstart, saver);
			saver = int_saver(s, saver);
			saver = int_saver(ppu_aux_index, saver);
			saver = int_saver(line, saver);
			saver = int_saver(patternNumber, saver);
			saver = int_saver(patternAddress, saver);
			saver = int_saver(ppu_Sprite_Draw_Cycle, saver);
			saver = int_saver(pixelcolor_latch_1, saver);
			saver = int_saver(pixelcolor_latch_2, saver);

			saver = long_saver(double_2007_read, saver);
			saver = long_saver(Total_PPU_Clock_Cycles, saver);

			saver = byte_array_saver(glitchy_reads_2003, saver, 8);
			saver = byte_array_saver(OAM, saver, 256);
			saver = byte_array_saver(PALRAM, saver, 32);
			saver = byte_array_saver(ppu_BG_Attr, saver, 3);
			saver = byte_array_saver(soam, saver, 256);

			saver = short_array_saver(xbuf, saver, 256 * 240);

			saver = int_array_saver(ppu_open_bus_decay_timer, saver, 8);

			for (int i = 0; i < 8; i++)
			{
				saver = bool_saver(ppu_Sprite_Shifters[i].X_Start, saver);
				saver = short_saver(ppu_Sprite_Shifters[i].X, saver);
				saver = byte_saver(ppu_Sprite_Shifters[i].Attr, saver);
				saver = byte_saver(ppu_Sprite_Shifters[i].Pattern_0, saver);
				saver = byte_saver(ppu_Sprite_Shifters[i].Pattern_1, saver);
			}

			return saver;
		}

		uint8_t* ppu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ppu_HasClockPPU, loader);
			loader = bool_loader(&ppu_Chop_Dot, loader);
			loader = bool_loader(&ppu_IdleSynch, loader);
			loader = bool_loader(&ppu_Dot_Was_Skipped, loader);
			loader = bool_loader(&ppu_Toggle_w, loader);

			loader = bool_loader(&ppu_Color_Disable, loader);
			loader = bool_loader(&ppu_Show_BG_Leftmost, loader);
			loader = bool_loader(&ppu_Show_OBJ_Leftmost, loader);
			loader = bool_loader(&ppu_Show_BG, loader);
			loader = bool_loader(&ppu_Show_OBJ, loader);
			loader = bool_loader(&ppu_Intense_Green, loader);
			loader = bool_loader(&ppu_Intense_Blue, loader);
			loader = bool_loader(&ppu_Intense_Red, loader);

			loader = bool_loader(&ppu_Vram_Incr32, loader);
			loader = bool_loader(&ppu_OBJ_Pattern_High, loader);
			loader = bool_loader(&ppu_BG_Pattern_High, loader);
			loader = bool_loader(&ppu_OBJ_Size_16, loader);
			loader = bool_loader(&ppu_Layer, loader);
			loader = bool_loader(&ppu_Vblank_NMI_Gen, loader);

			loader = bool_loader(&ppu_Reg2002_objoverflow, loader);
			loader = bool_loader(&ppu_Reg2002_objhit, loader);
			loader = bool_loader(&ppu_Reg2002_vblank_active, loader);
			loader = bool_loader(&ppu_Reg2002_vblank_active_pending, loader);
			loader = bool_loader(&ppu_Reg2002_vblank_clear_pending, loader);

			loader = bool_loader(&sprite_eval_write, loader);
			loader = bool_loader(&sprite_zero_in_range, loader);
			loader = bool_loader(&sprite_zero_go, loader);
			loader = bool_loader(&ppu_was_on, loader);
			loader = bool_loader(&race_2006, loader);
			loader = bool_loader(&show_bg_new, loader);
			loader = bool_loader(&show_obj_new, loader);
			loader = bool_loader(&do_vbl, loader);
			loader = bool_loader(&do_active_sl, loader);
			loader = bool_loader(&do_pre_vbl, loader);
			loader = bool_loader(&nmi_destiny, loader);
			loader = bool_loader(&evenOddDestiny, loader);
			loader = bool_loader(&last_pipeline, loader);
			loader = bool_loader(&ppu_Commit_Read, loader);
			loader = bool_loader(&ppu_Can_Corrupt, loader);
			loader = bool_loader(&ppu_Sprite_Draw_Glitch, loader);

			loader = byte_loader(&ppu_OAM_Corrupt_Addr, loader);
			loader = byte_loader(&VRAMBuffer, loader);
			loader = byte_loader(&read_value, loader);
			loader = byte_loader(&reg_2003, loader);
			loader = byte_loader(&glitch_2007_iter, loader);
			loader = byte_loader(&glitch_2007_addr, loader);
			loader = byte_loader(&ppu_temp_oam_y, loader);
			loader = byte_loader(&ppu_temp_oam_ind, loader);
			loader = byte_loader(&ppu_BG_NT_Addr, loader);

			loader = byte_loader(&ppu_Open_Bus, loader);

			loader = short_loader(&ppu_Reg_v, loader);
			loader = short_loader(&ppu_Reg_t, loader);
			loader = short_loader(&ppu_Scroll_x, loader);
			loader = short_loader(&ppu_Raster_Pos, loader);
			loader = short_loader(&ppu_VRAM_Address, loader);
			loader = short_loader(&ppu_BG_Pattern_0, loader);
			loader = short_loader(&ppu_BG_Pattern_1, loader);
			loader = short_loader(&ppu_Next_BG_Pt_0, loader);
			loader = short_loader(&ppu_Next_BG_Pt_1, loader);

			loader = int_loader(&cpu_step, loader);
			loader = int_loader(&cpu_stepcounter, loader);
			loader = int_loader(&ppuphase, loader);
			loader = int_loader(&ppudead, loader);
			loader = int_loader(&NMI_PendingInstructions, loader);
			loader = int_loader(&intensity_lsl_6, loader);
			loader = int_loader(&status_sl, loader);
			loader = int_loader(&status_cycle, loader);
			loader = int_loader(&spr_true_count, loader);
			loader = int_loader(&soam_index, loader);
			loader = int_loader(&oam_index, loader);
			loader = int_loader(&yp, loader);
			loader = int_loader(&yp_sp, loader);
			loader = int_loader(&target, loader);
			loader = int_loader(&spriteHeight, loader);
			loader = int_loader(&install_2006, loader);
			loader = int_loader(&install_2001, loader);
			loader = int_loader(&start_up_offset, loader);
			loader = int_loader(&NMI_offset, loader);
			loader = int_loader(&yp_shift, loader);
			loader = int_loader(&sprite_eval_cycle, loader);
			loader = int_loader(&xt, loader);
			loader = int_loader(&xp, loader);
			loader = int_loader(&xstart, loader);
			loader = int_loader(&s, loader);
			loader = int_loader(&ppu_aux_index, loader);
			loader = int_loader(&line, loader);
			loader = int_loader(&patternNumber, loader);
			loader = int_loader(&patternAddress, loader);
			loader = int_loader(&ppu_Sprite_Draw_Cycle, loader);
			loader = int_loader(&pixelcolor_latch_1, loader);
			loader = int_loader(&pixelcolor_latch_2, loader);

			loader = long_loader(&double_2007_read, loader);
			loader = long_loader(&Total_PPU_Clock_Cycles, loader);

			loader = byte_array_loader(glitchy_reads_2003, loader, 8);
			loader = byte_array_loader(OAM, loader, 256);
			loader = byte_array_loader(PALRAM, loader, 32);
			loader = byte_array_loader(ppu_BG_Attr, loader, 3);
			loader = byte_array_loader(soam, loader, 256);

			loader = short_array_loader(xbuf, loader, 256 * 240);

			loader = int_array_loader(ppu_open_bus_decay_timer, loader, 8);

			for (int i = 0; i < 8; i++)
			{	
				loader = bool_loader(&ppu_Sprite_Shifters[i].X_Start, loader);
				loader = short_loader(&ppu_Sprite_Shifters[i].X, loader);
				loader = byte_loader(&ppu_Sprite_Shifters[i].Attr, loader);
				loader = byte_loader(&ppu_Sprite_Shifters[i].Pattern_0, loader);
				loader = byte_loader(&ppu_Sprite_Shifters[i].Pattern_1, loader);
			}

			return loader;
		}

	#pragma endregion

	#pragma region Audio

		uint32_t LENGTH_TABLE[32] = { 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };

		uint32_t PULSE_DUTY[4][8] = {
			{0,1,0,0,0,0,0,0}, // (12.5%)
			{0,1,1,0,0,0,0,0}, // (25%)
			{0,1,1,1,1,0,0,0}, // (50%)
			{1,0,0,1,1,1,1,1}, // (25% negated (75%))
		};

		uint32_t TRIANGLE_TABLE[32] =
		{
			15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
			0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
		};

		uint32_t  NOISE_TABLE[16] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };

		uint32_t DMC_RATE[16] = { 428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54 };

		uint32_t sequencer_lut[2][5] = {
			{7456,14912,22370,29828, 0},
			{7456,14912,22370,29828,37280}
		};

		bool apu_Recalculate;
		bool apu_Len_Clock_Active;
		bool apu_Get_Cycle;

		uint8_t apu_Write_Value;

		uint32_t apu_Old_Cart_Sound;

		uint32_t apu_Audio_Sample_Clock;
		uint32_t apu_Write_Countdown;
		uint32_t apu_Write_Address;

		uint32_t apu_Audio_Num_Samples;
		int32_t apu_Audio_Samples[25000] = {};
		int32_t apu_Old_Mix;
		int32_t apu_Cart_Sound;

		// Sequencer
		bool apu_Sequencer_IRQ;
		bool apu_Sequencer_IRQ_Flag;
		bool apu_Sequencer_Inhibit;
		bool apu_Sequencer_Mode;
		bool apu_Sequencer_Half;
		bool apu_Sequencer_Quarter;
		bool apu_Sequencer_Flag_Trigger;

		uint8_t apu_Sequencer_Clear_Pending;
		uint8_t apu_Sequencer_Value;

		uint32_t apu_Sequencer_Counter;
		uint32_t apu_Sequencer_Step;
		uint32_t apu_Sequencer_Write_Countdown;
		uint32_t apu_Sequencer_IRQ_Assert;

		// Pulse_0
		bool apu_Pulse_0_Sweep_Reload;
		bool apu_Pulse_0_Sweep_Silence;
		bool apu_Pulse_0_Duty_Value;

		uint32_t apu_Pulse_0_Duty_Count, apu_Pulse_0_Env_Loop, apu_Pulse_0_Env_Constant, apu_Pulse_0_Env_Count_Value;
		uint32_t apu_Pulse_0_Sweep_Enable, apu_Pulse_0_Sweep_Divider_Count, apu_Pulse_0_Sweep_Negate, apu_Pulse_0_Sweep_Shiftcount;
		uint32_t apu_Pulse_0_Len_Count;
		uint32_t apu_Pulse_0_Timer_Raw_Reload_Value, apu_Pulse_0_Timer_Reload_Value;
		uint32_t apu_Pulse_0_Lenctr_Enable;
		uint32_t apu_Pulse_0_Sweep_Divider_Counter;
		uint32_t apu_Pulse_0_Duty_Step;
		uint32_t apu_Pulse_0_Timer_Counter;
		uint32_t apu_Pulse_0_Sample;
		uint32_t apu_Pulse_0_Env_Start_Flag, apu_Pulse_0_Env_Divider, apu_Pulse_0_Env_Counter;
		uint32_t apu_Pulse_0_Env_Output;

		// Pulse_1
		bool apu_Pulse_1_Sweep_Reload;
		bool apu_Pulse_1_Sweep_Silence;
		bool apu_Pulse_1_Duty_Value;

		uint32_t apu_Pulse_1_Duty_Count, apu_Pulse_1_Env_Loop, apu_Pulse_1_Env_Constant, apu_Pulse_1_Env_Count_Value;
		uint32_t apu_Pulse_1_Sweep_Enable, apu_Pulse_1_Sweep_Divider_Count, apu_Pulse_1_Sweep_Negate, apu_Pulse_1_Sweep_Shiftcount;
		uint32_t apu_Pulse_1_Len_Count;
		uint32_t apu_Pulse_1_Timer_Raw_Reload_Value, apu_Pulse_1_Timer_Reload_Value;
		uint32_t apu_Pulse_1_Lenctr_Enable;
		uint32_t apu_Pulse_1_Sweep_Divider_Counter;
		uint32_t apu_Pulse_1_Duty_Step;
		uint32_t apu_Pulse_1_Timer_Counter;
		uint32_t apu_Pulse_1_Sample;
		uint32_t apu_Pulse_1_Env_Start_Flag, apu_Pulse_1_Env_Divider, apu_Pulse_1_Env_Counter;
		uint32_t apu_Pulse_1_Env_Output;

		// Noise
		bool apu_Noise_Noise_Bit;

		uint32_t apu_Noise_Env_Count_Value, apu_Noise_Env_Loop, apu_Noise_Env_Constant;
		uint32_t apu_Noise_Mode_Count, apu_Noise_Period_Count;
		uint32_t apu_Noise_Len_Count;
		uint32_t apu_Noise_Lenctr_Enable;
		uint32_t apu_Noise_Shift_Register;
		uint32_t apu_Noise_Timer_Counter;
		uint32_t apu_Noise_Sample;
		uint32_t Noise_env_output, Noise_env_start_flag, Noise_env_divider, Noise_env_counter;

		// Triangle
		uint32_t apu_Triangle_Linear_Counter_Reload, apu_Triangle_Control_Flag;
		uint32_t apu_Triangle_Timer_Count, apu_Triangle_Reload_Flag, apu_Triangle_Len_Count;
		uint32_t apu_Triangle_Lenctr_Enable;
		uint32_t apu_Triangle_Linear_Counter, apu_Triangle_Timer, apu_Triangle_Timer_Count_Reload;
		uint32_t apu_Triangle_Seq;
		uint32_t apu_Triangle_Sample;

		// DMC
		bool apu_DMC_IRQ;
		bool apu_DMC_IRQ_Flag;
		bool apu_DMC_IRQ_Enabled;
		bool apu_DMC_Loop_Flag;
		bool apu_DMC_Out_Silence;
		bool apu_DMC_Sample_Buffer_Filled;
		bool apu_DMC_Call_From_Write;
		bool apu_DMC_Fill_Glitch_1;
		bool apu_DMC_Fill_Glitch_2;
		bool apu_DMC_Fill_Glitch_3;

		uint32_t apu_DMC_Timer_Reload;
		// dmc delay per visual 2a03
		uint32_t apu_DMC_Delay;
		// this timer never stops, ever, so it is convenient to use for even/odd timing used elsewhere
		uint32_t apu_DMC_Timer;
		uint32_t apu_DMC_User_Address;
		uint32_t apu_DMC_Sample_Address, apu_DMC_Sample_Buffer;
		uint32_t apu_DMC_Out_Shift, apu_DMC_Out_Bits_Remaining, apu_DMC_Out_Deltacounter;

		uint32_t apu_DMC_Sample_Length;
		uint32_t apu_DMC_User_Length;
		uint32_t apu_DMC_Length_Countdown;

		int32_t apu_DMC_RDY_Check;
		int32_t apu_DMC_DMA_Countdown;


		void apu_Pulse_0_WriteReg(uint32_t addr, uint8_t val)
		{
			// Console.WriteLine("write pulse {0:X} {1:X}", addr, val);
			switch (addr)
			{
				case 0:
					apu_Pulse_0_Env_Count_Value = val & 0xF;
					apu_Pulse_0_Env_Constant = (val >> 4) & 1;
					apu_Pulse_0_Env_Loop = (val >> 5) & 1;
					apu_Pulse_0_Duty_Count = (val >> 6) & 3;
					break;
				case 1:
					apu_Pulse_0_Sweep_Shiftcount = val & 7;
					apu_Pulse_0_Sweep_Negate = (val >> 3) & 1;
					apu_Pulse_0_Sweep_Divider_Count = (val >> 4) & 7;
					apu_Pulse_0_Sweep_Enable = (val >> 7) & 1;
					apu_Pulse_0_Sweep_Reload = true;
					break;
				case 2:
					apu_Pulse_0_Timer_Reload_Value = (apu_Pulse_0_Timer_Reload_Value & 0x700) | val;
					apu_Pulse_0_Timer_Raw_Reload_Value = apu_Pulse_0_Timer_Reload_Value * 2 + 2;
					// if (unit == 1) Console.WriteLine("{0} Pulse_0_timer_reload_value: {1}", unit, Pulse_0_timer_reload_value);
					break;
				case 3:					
					if (apu_Len_Clock_Active)
					{			
						if (apu_Pulse_0_Len_Count == 0)
						{
							apu_Pulse_0_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
						}
					}
					else
					{
						apu_Pulse_0_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
					}

					apu_Pulse_0_Timer_Reload_Value = (apu_Pulse_0_Timer_Reload_Value & 0xFF) | ((val & 0x07) << 8);
					apu_Pulse_0_Timer_Raw_Reload_Value = apu_Pulse_0_Timer_Reload_Value * 2 + 2;
					apu_Pulse_0_Duty_Step = 0;
					apu_Pulse_0_Env_Start_Flag = 1;

					// allow the lenctr_en to kill the len_cnt
					apu_Pulse_0_set_lenctr_en(apu_Pulse_0_Lenctr_Enable);

					// serves as a useful note-on diagnostic
					// if(unit==1) Console.WriteLine("{0} Pulse_0_timer_reload_value: {1}", unit, Pulse_0_timer_reload_value);
					break;
			}
		}

		void apu_Pulse_0_set_lenctr_en(uint32_t value)
		{
			apu_Pulse_0_Lenctr_Enable = value;
			// if the length counter is not enabled, then we must disable the length system in this way
			if (apu_Pulse_0_Lenctr_Enable == 0) apu_Pulse_0_Len_Count = 0;
		}

		void apu_Pulse_0_clock_length_and_sweep()
		{
			// this should be optimized to update only when `Pulse_0_timer_reload_value` changes
			int sweep_shifter = apu_Pulse_0_Timer_Reload_Value >> apu_Pulse_0_Sweep_Shiftcount;
			if (apu_Pulse_0_Sweep_Negate == 1)
				sweep_shifter = -sweep_shifter;
			sweep_shifter += apu_Pulse_0_Timer_Reload_Value;

			// this sweep logic is always enabled:
			apu_Pulse_0_Sweep_Silence = (apu_Pulse_0_Timer_Reload_Value < 8 || (sweep_shifter > 0x7FF)); // && Pulse_0_sweep_negate == 0));

			// does enable only block the pitch bend? does the clocking proceed?
			if (apu_Pulse_0_Sweep_Enable == 1)
			{
				// clock divider
				if (apu_Pulse_0_Sweep_Divider_Counter != 0) apu_Pulse_0_Sweep_Divider_Counter--;
				if (apu_Pulse_0_Sweep_Divider_Counter == 0)
				{
					apu_Pulse_0_Sweep_Divider_Counter = apu_Pulse_0_Sweep_Divider_Count + 1;

					// divider was clocked: process sweep pitch bend
					if (apu_Pulse_0_Sweep_Shiftcount != 0 && !apu_Pulse_0_Sweep_Silence)
					{
						apu_Pulse_0_Timer_Reload_Value = sweep_shifter;
						apu_Pulse_0_Timer_Raw_Reload_Value = (apu_Pulse_0_Timer_Reload_Value << 1) + 2;
					}
					// TODO - does this change the user's reload value or the latched reload value?
				}

				// handle divider reload, after clocking happens
				if (apu_Pulse_0_Sweep_Reload)
				{
					apu_Pulse_0_Sweep_Divider_Counter = apu_Pulse_0_Sweep_Divider_Count + 1;
					apu_Pulse_0_Sweep_Reload = false;
				}
			}

			// env_loop doubles as "halt length counter"
			if ((apu_Pulse_0_Env_Loop == 0) && apu_Pulse_0_Len_Count > 0)
				apu_Pulse_0_Len_Count--;
		}

		void apu_Pulse_0_clock_env()
		{
			if (apu_Pulse_0_Env_Start_Flag == 1)
			{
				apu_Pulse_0_Env_Start_Flag = 0;
				apu_Pulse_0_Env_Divider = apu_Pulse_0_Env_Count_Value;
				apu_Pulse_0_Env_Counter = 15;
			}
			else
			{
				if (apu_Pulse_0_Env_Divider != 0)
				{
					apu_Pulse_0_Env_Divider--;
				}
				else if (apu_Pulse_0_Env_Divider == 0)
				{
					apu_Pulse_0_Env_Divider = apu_Pulse_0_Env_Count_Value;
					if (apu_Pulse_0_Env_Counter == 0)
					{
						if (apu_Pulse_0_Env_Loop == 1)
						{
							apu_Pulse_0_Env_Counter = 15;
						}
					}
					else apu_Pulse_0_Env_Counter--;
				}
			}
		}

		void apu_Pulse_0_Run()
		{
			if (apu_Pulse_0_Env_Constant == 1)
				apu_Pulse_0_Env_Output = apu_Pulse_0_Env_Count_Value;
			else apu_Pulse_0_Env_Output = apu_Pulse_0_Env_Counter;

			if (apu_Pulse_0_Timer_Counter > 0) apu_Pulse_0_Timer_Counter--;
			if (apu_Pulse_0_Timer_Counter == 0 && apu_Pulse_0_Timer_Raw_Reload_Value != 0)
			{
				if (apu_Pulse_0_Duty_Step == 7)
				{
					apu_Pulse_0_Duty_Step = 0;
				}
				else
				{
					apu_Pulse_0_Duty_Step++;
				}
				apu_Pulse_0_Duty_Value = PULSE_DUTY[apu_Pulse_0_Duty_Count][apu_Pulse_0_Duty_Step] == 1;
				// reload Triangle_timer
				apu_Pulse_0_Timer_Counter = apu_Pulse_0_Timer_Raw_Reload_Value;
			}

			int newsample;

			if (apu_Pulse_0_Duty_Value) // high state of duty cycle
			{
				newsample = apu_Pulse_0_Env_Output;
				if (apu_Pulse_0_Sweep_Silence || apu_Pulse_0_Len_Count == 0)
					newsample = 0; // silenced
			}
			else
				newsample = 0; // duty cycle is 0, silenced.

			// newsample -= env_output >> 1; //unbias
			if (newsample != apu_Pulse_0_Sample)
			{
				apu_Recalculate = true;
				apu_Pulse_0_Sample = newsample;
			}
		}

		void apu_Pulse_1_WriteReg(uint32_t addr, uint8_t val)
		{
			// Console.WriteLine("write pulse {0:X} {1:X}", addr, val);
			switch (addr)
			{
				case 0:
					apu_Pulse_1_Env_Count_Value = val & 0xF;
					apu_Pulse_1_Env_Constant = (val >> 4) & 1;
					apu_Pulse_1_Env_Loop = (val >> 5) & 1;
					apu_Pulse_1_Duty_Count = (val >> 6) & 3;
					break;
				case 1:
					apu_Pulse_1_Sweep_Shiftcount = val & 7;
					apu_Pulse_1_Sweep_Negate = (val >> 3) & 1;
					apu_Pulse_1_Sweep_Divider_Count = (val >> 4) & 7;
					apu_Pulse_1_Sweep_Enable = (val >> 7) & 1;
					apu_Pulse_1_Sweep_Reload = true;
					break;
				case 2:
					apu_Pulse_1_Timer_Reload_Value = (apu_Pulse_1_Timer_Reload_Value & 0x700) | val;
					apu_Pulse_1_Timer_Raw_Reload_Value = apu_Pulse_1_Timer_Reload_Value * 2 + 2;
					// if (unit == 1) Console.WriteLine("{0} Pulse_1_timer_reload_value: {1}", unit, Pulse_1_timer_reload_value);
					break;
				case 3:
					if (apu_Len_Clock_Active)
					{
						if (apu_Pulse_1_Len_Count == 0)
						{
							apu_Pulse_1_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
						}
					}
					else
					{
						apu_Pulse_1_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
					}

					apu_Pulse_1_Timer_Reload_Value = (apu_Pulse_1_Timer_Reload_Value & 0xFF) | ((val & 0x07) << 8);
					apu_Pulse_1_Timer_Raw_Reload_Value = apu_Pulse_1_Timer_Reload_Value * 2 + 2;
					apu_Pulse_1_Duty_Step = 0;
					apu_Pulse_1_Env_Start_Flag = 1;

					// allow the lenctr_en to kill the len_cnt
					apu_Pulse_1_set_lenctr_en(apu_Pulse_1_Lenctr_Enable);

					// serves as a useful note-on diagnostic
					// if(unit==1) Console.WriteLine("{0} Pulse_1_timer_reload_value: {1}", unit, Pulse_1_timer_reload_value);
					break;
			}
		}

		void apu_Pulse_1_set_lenctr_en(uint32_t value)
		{
			apu_Pulse_1_Lenctr_Enable = value;
			// if the length counter is not enabled, then we must disable the length system in this way
			if (apu_Pulse_1_Lenctr_Enable == 0) apu_Pulse_1_Len_Count = 0;
		}

		void apu_Pulse_1_clock_length_and_sweep()
		{
			// this should be optimized to update only when `Pulse_1_timer_reload_value` changes
			int sweep_shifter = apu_Pulse_1_Timer_Reload_Value >> apu_Pulse_1_Sweep_Shiftcount;
			if (apu_Pulse_1_Sweep_Negate == 1)
				sweep_shifter = -sweep_shifter - 1;
			sweep_shifter += apu_Pulse_1_Timer_Reload_Value;

			// this sweep logic is always enabled:
			apu_Pulse_1_Sweep_Silence = (apu_Pulse_1_Timer_Reload_Value < 8 || (sweep_shifter > 0x7FF)); // && Pulse_1_sweep_negate == 0));

			// does enable only block the pitch bend? does the clocking proceed?
			if (apu_Pulse_1_Sweep_Enable == 1)
			{
				// clock divider
				if (apu_Pulse_1_Sweep_Divider_Counter != 0) apu_Pulse_1_Sweep_Divider_Counter--;
				if (apu_Pulse_1_Sweep_Divider_Counter == 0)
				{
					apu_Pulse_1_Sweep_Divider_Counter = apu_Pulse_1_Sweep_Divider_Count + 1;

					// divider was clocked: process sweep pitch bend
					if (apu_Pulse_1_Sweep_Shiftcount != 0 && !apu_Pulse_1_Sweep_Silence)
					{
						apu_Pulse_1_Timer_Reload_Value = sweep_shifter;
						apu_Pulse_1_Timer_Raw_Reload_Value = (apu_Pulse_1_Timer_Reload_Value << 1) + 2;
					}
					// TODO - does this change the user's reload value or the latched reload value?
				}

				// handle divider reload, after clocking happens
				if (apu_Pulse_1_Sweep_Reload)
				{
					apu_Pulse_1_Sweep_Divider_Counter = apu_Pulse_1_Sweep_Divider_Count + 1;
					apu_Pulse_1_Sweep_Reload = false;
				}
			}

			// env_loop doubles as "halt length counter"
			if ((apu_Pulse_1_Env_Loop == 0) && apu_Pulse_1_Len_Count > 0)
				apu_Pulse_1_Len_Count--;
		}

		void apu_Pulse_1_clock_env()
		{
			if (apu_Pulse_1_Env_Start_Flag == 1)
			{
				apu_Pulse_1_Env_Start_Flag = 0;
				apu_Pulse_1_Env_Divider = apu_Pulse_1_Env_Count_Value;
				apu_Pulse_1_Env_Counter = 15;
			}
			else
			{
				if (apu_Pulse_1_Env_Divider != 0)
				{
					apu_Pulse_1_Env_Divider--;
				}
				else if (apu_Pulse_1_Env_Divider == 0)
				{
					apu_Pulse_1_Env_Divider = apu_Pulse_1_Env_Count_Value;
					if (apu_Pulse_1_Env_Counter == 0)
					{
						if (apu_Pulse_1_Env_Loop == 1)
						{
							apu_Pulse_1_Env_Counter = 15;
						}
					}
					else apu_Pulse_1_Env_Counter--;
				}
			}
		}

		void apu_Pulse_1_Run()
		{
			if (apu_Pulse_1_Env_Constant == 1)
				apu_Pulse_1_Env_Output = apu_Pulse_1_Env_Count_Value;
			else apu_Pulse_1_Env_Output = apu_Pulse_1_Env_Counter;

			if (apu_Pulse_1_Timer_Counter > 0) apu_Pulse_1_Timer_Counter--;
			if (apu_Pulse_1_Timer_Counter == 0 && apu_Pulse_1_Timer_Raw_Reload_Value != 0)
			{
				if (apu_Pulse_1_Duty_Step == 7)
				{
					apu_Pulse_1_Duty_Step = 0;
				}
				else
				{
					apu_Pulse_1_Duty_Step++;
				}
				apu_Pulse_1_Duty_Value = PULSE_DUTY[apu_Pulse_1_Duty_Count][apu_Pulse_1_Duty_Step] == 1;
				// reload Triangle_timer
				apu_Pulse_1_Timer_Counter = apu_Pulse_1_Timer_Raw_Reload_Value;
			}

			int newsample;

			if (apu_Pulse_1_Duty_Value) // high state of duty cycle
			{
				newsample = apu_Pulse_1_Env_Output;
				if (apu_Pulse_1_Sweep_Silence || apu_Pulse_1_Len_Count == 0)
					newsample = 0; // silenced
			}
			else
				newsample = 0; // duty cycle is 0, silenced.

			// newsample -= env_output >> 1; //unbias
			if (newsample != apu_Pulse_1_Sample)
			{
				apu_Recalculate = true;
				apu_Pulse_1_Sample = newsample;
			}
		}


		void apu_Noise_WriteReg(uint32_t addr, uint8_t val)
		{
			switch (addr)
			{
				case 0:
					apu_Noise_Env_Count_Value = val & 0xF;
					apu_Noise_Env_Constant = (val >> 4) & 1;
					apu_Noise_Env_Loop = (val >> 5) & 1;
					break;
				case 1:
					break;
				case 2:
					apu_Noise_Period_Count = NOISE_TABLE[val & 0xF];
					apu_Noise_Mode_Count = (val >> 7) & 1;
					break;
				case 3:
					if (apu_Len_Clock_Active)
					{
						if (apu_Noise_Len_Count == 0)
						{
							apu_Noise_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
						}
					}
					else
					{
						apu_Noise_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
					}

					apu_Noise_set_lenctr_en(apu_Noise_Lenctr_Enable);
					Noise_env_start_flag = 1;
					break;
			}
		}

		void apu_Noise_set_lenctr_en(uint32_t value)
		{
			apu_Noise_Lenctr_Enable = value;

			// if the length counter is not enabled, then we must disable the length system in this way
			if (apu_Noise_Lenctr_Enable == 0) apu_Noise_Len_Count = 0;
		}

		void apu_Noise_clock_env()
		{
			if (Noise_env_start_flag == 1)
			{
				Noise_env_start_flag = 0;
				Noise_env_divider = (apu_Noise_Env_Count_Value + 1);
				Noise_env_counter = 15;
			}
			else
			{
				if (Noise_env_divider != 0) Noise_env_divider--;
				if (Noise_env_divider == 0)
				{
					Noise_env_divider = (apu_Noise_Env_Count_Value + 1);
					if (Noise_env_counter == 0)
					{
						if (apu_Noise_Env_Loop == 1)
						{
							Noise_env_counter = 15;
						}
					}
					else Noise_env_counter--;
				}
			}
		}

		void apu_Noise_clock_length_and_sweep()
		{

			if (apu_Noise_Len_Count > 0 && (apu_Noise_Env_Loop == 0))
				apu_Noise_Len_Count--;
		}

		void apu_Noise_Run()
		{
			if (apu_Noise_Env_Constant == 1)
				Noise_env_output = apu_Noise_Env_Count_Value;
			else Noise_env_output = Noise_env_counter;

			if (apu_Noise_Timer_Counter > 0) apu_Noise_Timer_Counter--;
			if (apu_Noise_Timer_Counter == 0 && apu_Noise_Period_Count != 0)
			{
				// reload Noise timer
				apu_Noise_Timer_Counter = apu_Noise_Period_Count;
				int feedback_bit;
				if (apu_Noise_Mode_Count == 1) feedback_bit = (apu_Noise_Shift_Register >> 6) & 1;
				else feedback_bit = (apu_Noise_Shift_Register >> 1) & 1;
				int feedback = feedback_bit ^ (apu_Noise_Shift_Register & 1);
				apu_Noise_Shift_Register >>= 1;
				apu_Noise_Shift_Register &= ~(1 << 14);
				apu_Noise_Shift_Register |= (feedback << 14);
				apu_Noise_Noise_Bit = (apu_Noise_Shift_Register & 1) != 0;
			}

			int newsample;
			if (apu_Noise_Len_Count == 0) newsample = 0;
			else if (apu_Noise_Noise_Bit) newsample = Noise_env_output; // switched, was 0?
			else newsample = 0;
			if (newsample != apu_Noise_Sample)
			{
				apu_Recalculate = true;
				apu_Noise_Sample = newsample;
			}
		}


		void apu_Triangle_set_lenctr_en(uint32_t value)
		{
			apu_Triangle_Lenctr_Enable = value;
			// if the length counter is not enabled, then we must disable the length system in this way
			if (apu_Triangle_Lenctr_Enable == 0) apu_Triangle_Len_Count = 0;
		}

		void apu_Triangle_WriteReg(uint32_t addr, uint8_t val)
		{
			switch (addr)
			{
				case 0:
					apu_Triangle_Linear_Counter_Reload = (val & 0x7F);
					apu_Triangle_Control_Flag = (val >> 7) & 1;
					break;
				case 1: break;
				case 2:
					apu_Triangle_Timer_Count = (apu_Triangle_Timer_Count & ~0xFF) | val;
					apu_Triangle_Timer_Count_Reload = apu_Triangle_Timer_Count + 1;
					break;
				case 3:
					apu_Triangle_Timer_Count = (apu_Triangle_Timer_Count & 0xFF) | ((val & 0x7) << 8);
					apu_Triangle_Timer_Count_Reload = apu_Triangle_Timer_Count + 1;
					if (apu_Len_Clock_Active)
					{
						if (apu_Triangle_Len_Count == 0)
						{
							apu_Triangle_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
						}
					}
					else
					{
						apu_Triangle_Len_Count = LENGTH_TABLE[(val >> 3) & 0x1F];
					}
					apu_Triangle_Reload_Flag = 1;

					// allow the lenctr_en to kill the len_cnt
					apu_Triangle_set_lenctr_en(apu_Triangle_Lenctr_Enable);
					break;
			}
		}

		void apu_Triangle_Run()
		{
			// when clocked by Triangle_timer, Triangle_seq steps forward
			// except when linear counter or length counter is 0 
			bool en = apu_Triangle_Len_Count != 0 && apu_Triangle_Linear_Counter != 0;

			bool do_clock = false;
			if (apu_Triangle_Timer > 0) apu_Triangle_Timer--;
			if (apu_Triangle_Timer == 0)
			{
				do_clock = true;
				apu_Triangle_Timer = apu_Triangle_Timer_Count_Reload;
			}

			if (en && do_clock)
			{
				int newsample;

				apu_Triangle_Seq = (apu_Triangle_Seq + 1) & 0x1F;

				newsample = TRIANGLE_TABLE[apu_Triangle_Seq];

				// special hack: frequently, games will use the maximum frequency triangle in order to mute it
				// apparently this results in the DAC for the triangle wave outputting a steady level at about 7.5
				// so we'll emulate it at the digital level
				if (apu_Triangle_Timer_Count_Reload == 1) newsample = 8;

				if (newsample != apu_Triangle_Sample)
				{
					apu_Recalculate = true;
					apu_Triangle_Sample = newsample;
				}
			}
		}

		void apu_Triangle_clock_length_and_sweep()
		{
			// env_loopdoubles as "halt length counter"
			if (apu_Triangle_Len_Count > 0 && apu_Triangle_Control_Flag == 0)
				apu_Triangle_Len_Count--;
		}

		void apu_Triangle_clock_linear_counter()
		{
			if (apu_Triangle_Reload_Flag == 1)
			{
				apu_Triangle_Linear_Counter = apu_Triangle_Linear_Counter_Reload;
			}
			else if (apu_Triangle_Linear_Counter != 0)
			{
				apu_Triangle_Linear_Counter--;
			}

			if (apu_Triangle_Control_Flag == 0) { apu_Triangle_Reload_Flag = 0; }
		}

		void apu_DMC_Run()
		{
			if (apu_DMC_Timer > 0)
			{
				apu_DMC_Timer--;
			}

			if (apu_DMC_Timer == 0)
			{
				apu_DMC_Timer = apu_DMC_Timer_Reload;
				apu_DMC_Clock();
			}

			// Any time the sample buffer is in an empty state and bytes remaining is not zero, the following occur: 
			// also note that the halt for DMC DMA occurs on APU cycles only (hence the timer check)
			if (!apu_DMC_Sample_Buffer_Filled && ((apu_DMC_Sample_Length > 0) || apu_DMC_Fill_Glitch_2) && (apu_DMC_DMA_Countdown == -1) && (apu_DMC_Delay == 0))
			{
				if (!apu_DMC_Call_From_Write)
				{
					// when called due to empty buffer while DMC running, there is no delay
					apu_DMC_DMA_Countdown = 4;
					apu_DMC_RDY_Check = 2;

					if (TraceCallback) TraceCallback(4);
				}
				else
				{
					// when called from write, either a 2 or 3 cycle delay in activation.
					if (apu_Get_Cycle)
					{
						apu_DMC_Delay = 3;
					}
					else
					{
						apu_DMC_Delay = 2;
					}
				}
			}

			// VisualNES and test roms verify that DMC DMA is 1 cycle shorter for calls from writes
			// however, if the third cycle lands on a write, there is a 2 cycle delay even if the instruction only has a single write
			// therefore, the RDY_check is 2 in both cases.
			if (apu_DMC_Delay != 0)
			{
				apu_DMC_Delay--;
				if (apu_DMC_Delay == 0)
				{
					apu_DMC_DMA_Countdown = 3;
					apu_DMC_RDY_Check = 2;
					apu_DMC_Call_From_Write = false;

					if (TraceCallback) TraceCallback(4);
				}
			}

			if (apu_DMC_Length_Countdown > 0)
			{
				apu_DMC_Length_Countdown--;

				if (apu_DMC_Length_Countdown == 2)
				{
					apu_DMC_Sample_Length -= 1;

					if (apu_DMC_Sample_Length == 0)
					{
						if (apu_DMC_Loop_Flag)
						{
							apu_DMC_Sample_Address = apu_DMC_User_Address;
							apu_DMC_Sample_Length = apu_DMC_User_Length;
						}

						if (!apu_DMC_Loop_Flag && apu_DMC_IRQ_Enabled)
						{
							apu_DMC_IRQ_Flag = true;
						}
					}
				}

				if (apu_DMC_Length_Countdown == 0)
				{
					if (!apu_DMC_Loop_Flag && apu_DMC_IRQ_Enabled && (apu_DMC_Sample_Length == 0))
					{
						apu_DMC_IRQ = true;
					}
				}
			}
		}

		void apu_DMC_Clock()
		{
			// If the silence flag is clear, bit 0 of the shift register is applied to the counter as follows: 
			// if bit 0 is clear and the delta-counter is greater than 1, the counter is decremented by 2; 
			// otherwise, if bit 0 is set and the delta-counter is less than 126, the counter is incremented by 2
			if (!apu_DMC_Out_Silence)
			{
				// apply current sample bit to delta counter
				if ((apu_DMC_Out_Shift & 1) == 1)
				{
					if (apu_DMC_Out_Deltacounter < 126)
						apu_DMC_Out_Deltacounter += 2;
				}
				else
				{
					if (apu_DMC_Out_Deltacounter > 1)
						apu_DMC_Out_Deltacounter -= 2;
				}

				apu_Recalculate = true;
			}

			// The right shift register is clocked. 
			apu_DMC_Out_Shift >>= 1;

			// The bits-remaining counter is decremented. If it becomes zero, a new cycle is started. 
			if (apu_DMC_Out_Bits_Remaining == 0)
			{
				// The bits-remaining counter is loaded with 8. 
				apu_DMC_Out_Bits_Remaining = 7;

				// in this case we end up in a case of being silenced for a sample legnth but with a full buffer
				if (apu_DMC_Fill_Glitch_1)
				{
					apu_DMC_Out_Silence = true;

					apu_DMC_Fill_Glitch_1 = false;
				}
				else
				{
					if (!apu_DMC_Sample_Buffer_Filled)
					{
						// If the sample buffer is empty then the silence flag is set
						apu_DMC_Out_Silence = true;
					}
					else
					{
						// otherwise, the silence flag is cleared and the sample buffer is emptied into the shift register. 
						apu_DMC_Out_Silence = false;
						apu_DMC_Out_Shift = apu_DMC_Sample_Buffer;
						apu_DMC_Sample_Buffer_Filled = false;
					}
				}
			}
			else
			{
				apu_DMC_Out_Bits_Remaining--;
			}
		}

		void apu_DMC_set_lenctr_en(bool en)
		{
			if (!en)
			{
				// in the first case, the disable happens right as the reload begins, it is cancelled but one cycle still runs
				// in the second case, it is too late to cancel the DMA but IRQs can still be cancelled
				if ((apu_DMC_Timer == 1) && (apu_DMC_Out_Bits_Remaining == 0) && (apu_DMC_Sample_Length != 0))
				{
					apu_DMC_Fill_Glitch_2 = true;
				}
				else if (apu_DMC_DMA_Countdown != -1)
				{
					apu_DMC_Fill_Glitch_3 = true;
				}
				else
				{
					apu_DMC_Sample_Length = 0;
				}
			}
			else
			{
				// can't call from write if currently enabled already, even if buffer empty (which indicates a DMA just started)
				bool able_to_call_from_write = false;
				
				// only start playback if playback is stopped
				if (apu_DMC_Sample_Length == 0)
				{
					able_to_call_from_write = true;
					
					apu_DMC_Sample_Address = apu_DMC_User_Address;
					apu_DMC_Sample_Length = apu_DMC_User_Length;
				}

				if (!apu_DMC_Sample_Buffer_Filled)
				{
					if (able_to_call_from_write)
					{
						// apparently the dmc is different if called from a cpu write, let's try
						apu_DMC_Call_From_Write = true;
					}
				}
				else if ((apu_DMC_Out_Bits_Remaining == 0) && (apu_DMC_Timer == 1))
				{
					if (able_to_call_from_write)
					{
						// this case also misses being an ordinary DMA and becomes a write enable DMA
						apu_DMC_Call_From_Write = true;
					}
				}
			}

			// irq is acknowledged or sure to be clear, in either case
			apu_DMC_IRQ = false;
			apu_DMC_IRQ_Flag = false;
		}

		void apu_DMC_WriteReg(uint32_t addr, uint8_t val)
		{
			switch (addr)
			{
				case 0:
					apu_DMC_IRQ_Enabled = ((val & 0x80) == 0x80);
					apu_DMC_Loop_Flag = ((val & 0x40) == 0x40);
					apu_DMC_Timer_Reload = DMC_RATE[val & 0xF];
					if (!apu_DMC_IRQ_Enabled)
					{
						apu_DMC_IRQ = false;
						apu_DMC_IRQ_Flag = false;
					}
					break;

				case 1:
					apu_DMC_Out_Deltacounter = val & 0x7F;
					apu_Recalculate = true;
					break;

				case 2:
					apu_DMC_User_Address = 0xC000 | (val << 6);
					break;

				case 3:
					apu_DMC_User_Length = ((uint32_t)val << 4) + 1;
					break;
			}
		}

		void apu_DMC_Fetch()
		{
			if (apu_DMC_Sample_Length != 0)
			{
				apu_DMC_Sample_Buffer = ReadMemoryDMA(apu_DMC_Sample_Address);
				apu_DMC_Sample_Buffer_Filled = true;
				apu_DMC_Sample_Address = (uint32_t)(apu_DMC_Sample_Address + 1);
				apu_DMC_Sample_Address &= 0xFFFF;

				//sample address wraps to 0x8000, even though this cannot be reached by write to address reg
				if (apu_DMC_Sample_Address == 0) { apu_DMC_Sample_Address = 0x8000; }

				apu_DMC_Length_Countdown = 3;
			}
		}

		void apu_Sequencer_Tick()
		{
			apu_Sequencer_Counter++;

			if (!apu_Sequencer_Mode)
			{
				if (apu_Sequencer_Step == 1)
				{
					if (apu_Sequencer_Counter == sequencer_lut[0][1])
					{
						apu_Sequencer_Half = true;
					}
				}
				else if (apu_Sequencer_Step == 3)
				{
					if (apu_Sequencer_Counter == sequencer_lut[0][3] - 1)
					{
						apu_Sequencer_Flag_Trigger = true;
					}

					if (apu_Sequencer_Counter == sequencer_lut[0][3])
					{
						apu_Sequencer_Half = true;
					}
				}

				if (sequencer_lut[0][apu_Sequencer_Step] == apu_Sequencer_Counter)
				{
					apu_Sequencer_Quarter = true;

					apu_Sequencer_Step += 1;

					if (apu_Sequencer_Step == 4)
					{
						apu_Sequencer_Step = 0;
						apu_Sequencer_Counter = -2;
					}
				}
			}
			else
			{
				if (apu_Sequencer_Step == 1)
				{
					if (apu_Sequencer_Counter == sequencer_lut[1][1])
					{
						apu_Sequencer_Half = true;
					}
				}
				else if (apu_Sequencer_Step == 4)
				{
					if (apu_Sequencer_Counter == sequencer_lut[1][4])
					{
						apu_Sequencer_Half = true;
					}
				}

				if (sequencer_lut[1][apu_Sequencer_Step] == apu_Sequencer_Counter)
				{
					// third step doesn't do a quarter clock
					if (apu_Sequencer_Step != 3)
					{
						apu_Sequencer_Quarter = true;
					}

					apu_Sequencer_Step += 1;

					if (apu_Sequencer_Step == 5)
					{
						apu_Sequencer_Step = 0;
						apu_Sequencer_Counter = -2;
					}
				}
			}
			
			if (apu_Sequencer_Write_Countdown > 0)
			{
				apu_Sequencer_Write_Countdown--;

				if (apu_Sequencer_Write_Countdown == 1)
				{
					// clock if mode flag will be set, but don't double clock
					if (((apu_Sequencer_Value & 0x80) == 0x80))
					{
						apu_Sequencer_Quarter = true;
						apu_Sequencer_Half = true;
					}
				}
				else if (apu_Sequencer_Write_Countdown == 0)
				{
					apu_Sequencer_Inhibit = ((apu_Sequencer_Value & 0x40) == 0x40);
					apu_Sequencer_Mode = ((apu_Sequencer_Value & 0x80) == 0x80);

					if (apu_Sequencer_Inhibit)
					{
						apu_Sequencer_IRQ = false;
						apu_Sequencer_IRQ_Flag = false;
						apu_Sequencer_Flag_Trigger = false;
					}

					apu_Sequencer_Counter = -1;
					apu_Sequencer_Step = 0;
				}
			}

			if (apu_Sequencer_Quarter)
			{
				apu_QuarterFrame();
				apu_Sequencer_Quarter = false;
			}

			if (apu_Sequencer_Half)
			{
				apu_HalfFrame();
				apu_Sequencer_Half = false;
			}

			if (apu_Sequencer_Clear_Pending > 0)
			{
				apu_Sequencer_Clear_Pending--;
				if (apu_Sequencer_Clear_Pending == 0)
				{
					apu_Sequencer_IRQ_Flag = false;
				}
			}

			if (apu_Sequencer_Flag_Trigger)
			{
				apu_Sequencer_IRQ_Flag = true;

				if (apu_Sequencer_Counter == -1)
				{
					apu_Sequencer_Flag_Trigger = false;

					if (apu_Sequencer_Inhibit)
					{
						apu_Sequencer_IRQ_Flag = false;
					}
					else
					{
						apu_Sequencer_IRQ = true;
					}
				}
			}
		}

		void apu_HalfFrame()
		{
			apu_Len_Clock_Active = true;
			apu_Pulse_0_clock_length_and_sweep();
			apu_Pulse_1_clock_length_and_sweep();
			apu_Triangle_clock_length_and_sweep();
			apu_Noise_clock_length_and_sweep();
		}

		void apu_QuarterFrame()
		{
			apu_Pulse_0_clock_env();
			apu_Pulse_1_clock_env();
			apu_Triangle_clock_linear_counter();
			apu_Noise_clock_env();
		}

		uint8_t apu_ReadReg(uint32_t addr)
		{
			switch (addr)
			{
				case 0x4015:
				{
					uint8_t ret = apu_PeekReg(0x4015);

					if (!apu_Get_Cycle)
					{
						apu_Sequencer_IRQ_Flag = false;
					}
					else
					{
						apu_Sequencer_Clear_Pending = 2;
					}

					apu_Sequencer_IRQ = false;
					return ret;
				}
				default:
					return DB;
			}
		}

		void apu_WriteReg(uint32_t addr, uint8_t val)
		{
			// clock in writes only on even cycles
			switch (addr)
			{
				// these cases are needed to pass blargg's apu tests or accuracy coin tests
				case 0x4000:
				case 0x4004:
				case 0x4008:
				case 0x400C:
				case 0x4015:
				case 0x4017:
					if (!apu_Get_Cycle)
					{
						apu_Write_Countdown = 2;
					}
					else
					{
						apu_Write_Countdown = 3;
					}
					break;

				// these cases are needed to pass blargg's apu tests, they match reloads with length clock timing (test 11 len rld timing)
				case 0x4003:
				case 0x4007:
				case 0x400B:
				case 0x400F:
					apu_Write_Countdown = 2;
					break;

				// needed for castlevania 3 tas to sync (write on same cycle as reload of buffer should use new clock rate)
				case 0x4010:
					apu_Write_Countdown = 1;
					break;

				// currently unknown, not needed for any test
				default:
					if (!apu_Get_Cycle)
					{
						apu_Write_Countdown = 2;
					}
					else
					{
						apu_Write_Countdown = 3;
					}
					break;
			}

			apu_Write_Address = addr;
			apu_Write_Value = val;
		}

		void apu_Internal_WriteReg(uint32_t addr, uint8_t val)
		{
			//Console.WriteLine("{0:X4} = {1:X2}", addr, val);
			uint32_t index = addr - 0x4000;
			uint32_t reg = index & 3;
			uint32_t channel = index >> 2;
			switch (channel)
			{
				case 0:
					apu_Pulse_0_WriteReg(reg, val);
					break;
				case 1:
					apu_Pulse_1_WriteReg(reg, val);
					break;
				case 2:
					apu_Triangle_WriteReg(reg, val);
					break;
				case 3:
					apu_Noise_WriteReg(reg, val);
					break;
				case 4:
					apu_DMC_WriteReg(reg, val);
					break;
				case 5:
					if (addr == 0x4015)
					{
						apu_Pulse_0_set_lenctr_en(val & 1);
						apu_Pulse_1_set_lenctr_en((val >> 1) & 1);
						apu_Triangle_set_lenctr_en((val >> 2) & 1);
						apu_Noise_set_lenctr_en((val >> 3) & 1);
						apu_DMC_set_lenctr_en((val & 0x10) == 0x10);

					}
					else if (addr == 0x4017)
					{
						apu_Sequencer_Value = val;

						apu_Sequencer_Write_Countdown = 2;
					}
					break;
			}
		}

		uint8_t apu_PeekReg(uint32_t addr)
		{
			switch (addr)
			{
				case 0x4015:
				{
					bool dmc_nz_bool = false;

					dmc_nz_bool = apu_DMC_Sample_Length > 0;

					int dmc_nonzero = dmc_nz_bool ? 1 : 0;
					int noise_nonzero = (apu_Noise_Len_Count > 0) ? 1 : 0;
					int tri_nonzero = (apu_Triangle_Len_Count > 0) ? 1 : 0;
					int pulse1_nonzero = (apu_Pulse_1_Len_Count > 0) ? 1 : 0;
					int pulse0_nonzero = (apu_Pulse_0_Len_Count > 0) ? 1 : 0;
					int ret = ((apu_DMC_IRQ_Flag ? 1 : 0) << 7) | ((apu_Sequencer_IRQ_Flag ? 1 : 0) << 6) | (dmc_nonzero << 4) | (noise_nonzero << 3) | (tri_nonzero << 2) | (pulse1_nonzero << 1) | (pulse0_nonzero);
					return (uint8_t)ret;
				}
				default:
					return DB;
			}
		}

		/// <summary>only call in board.ClockCPU()</summary>
		void apu_ExternalQueue(uint32_t value)
		{
			apu_Cart_Sound = value + apu_Old_Cart_Sound;

			if (apu_Cart_Sound != apu_Old_Cart_Sound)
			{
				apu_Recalculate = true;
				apu_Old_Cart_Sound = apu_Cart_Sound;
			}
		}

		int32_t apu_EmitSample()
		{
			if (apu_Recalculate)
			{
				apu_Recalculate = false;

				int s_pulse0 = apu_Pulse_0_Sample;
				int s_pulse1 = apu_Pulse_1_Sample;
				int s_tri = apu_Triangle_Sample;
				int s_noise = apu_Noise_Sample;
				int s_dmc = apu_DMC_Out_Deltacounter;

				// more properly correct
				float pulse_out = s_pulse0 == 0 && s_pulse1 == 0
					? 0
					: 95.88f / ((8128.0f / (s_pulse0 + s_pulse1)) + 100.0f);

				float tnd_out = s_tri == 0 && s_noise == 0 && s_dmc == 0
					? 0
					: 159.79f / (1 / ((s_tri / 8227.0f) + (s_noise / 12241.0f /* * NOISEADJUST*/) + (s_dmc / 22638.0f)) + 100);


				float output = pulse_out + tnd_out;

				// output = output * 2 - 1;
				// this needs to leave enough headroom for straying DC bias due to the DMC unit getting stuck outputs. smb3 is bad about that. 
				apu_Old_Mix = (int32_t)(20000 * output * (1 + 1 / 5)) + apu_Cart_Sound;
			}

			return apu_Old_Mix;
		}


		void apu_SoftReset()
		{
			// need to study what happens to apu and stuff..
			apu_Sequencer_IRQ = false;
			apu_Sequencer_IRQ_Flag = false;
			apu_Internal_WriteReg(0x4015, 0);

			// for 4017, its as if the last value written gets rewritten
			apu_Sequencer_Inhibit = ((apu_Sequencer_Value & 0x40) == 0x40);
			apu_Sequencer_Mode = ((apu_Sequencer_Value & 0x80) == 0x80);

			apu_Sequencer_Counter = 0;
			apu_Sequencer_Step = 0;

			apu_DMC_Fill_Glitch_1 = false;
			apu_DMC_Fill_Glitch_2 = false;
			apu_DMC_Fill_Glitch_3 = false;
		}

		void apu_HardReset()
		{
			// "at power on it is as if $00 was written to $4017 9-12 cycles before the reset vector"
			// DMC seems to run for a couple cycles after reset, so aim for the upper end of that range (12)
			apu_Sequencer_Counter = 0;
			apu_Sequencer_Step = 0;

			apu_DMC_Fill_Glitch_1 = false;
			apu_DMC_Fill_Glitch_2 = false;
			apu_DMC_Fill_Glitch_3 = false;
		}

		void apu_Reset()
		{
			// NOTE: Must match parity of DMC Timer
			apu_Get_Cycle = true;
			
			apu_Audio_Sample_Clock = 0;
			apu_Write_Countdown = 0;
			apu_Write_Address = 0;
			apu_Write_Value = 0;

			apu_Sequencer_IRQ = false;

			apu_Sequencer_Value = 0;
			apu_Sequencer_IRQ_Flag = false;
			apu_Sequencer_Inhibit = false;
			apu_Sequencer_Mode = false;
			apu_Sequencer_Half = false;
			apu_Sequencer_Quarter = false;
			apu_Sequencer_Flag_Trigger = false;

			apu_Sequencer_Clear_Pending = 0;
			apu_Sequencer_Counter = 0;
			apu_Sequencer_Step = 0;
			apu_Sequencer_Write_Countdown = 0;
			apu_Sequencer_IRQ_Assert = 0;

			apu_Len_Clock_Active = false;

			apu_Old_Mix = 0;
			apu_Cart_Sound = 0;
			apu_Old_Cart_Sound = 0;

			apu_Pulse_0_Duty_Count = 0;
			apu_Pulse_0_Env_Loop = 0;
			apu_Pulse_0_Env_Constant = 0;
			apu_Pulse_0_Env_Count_Value = 0;

			apu_Pulse_0_Sweep_Enable = 0;
			apu_Pulse_0_Sweep_Divider_Count = 0;
			apu_Pulse_0_Sweep_Negate = 0;
			apu_Pulse_0_Sweep_Shiftcount = 0;
			apu_Pulse_0_Sweep_Reload = false;

			apu_Pulse_0_Len_Count = 0;
			apu_Pulse_0_Timer_Raw_Reload_Value = 0;
			apu_Pulse_0_Timer_Reload_Value = 0;

			apu_Pulse_0_Lenctr_Enable = 0;

			apu_Pulse_0_Sweep_Divider_Counter = 0;
			apu_Pulse_0_Sweep_Silence = false;
			apu_Pulse_0_Duty_Step = 0;
			apu_Pulse_0_Timer_Counter = 0;
			apu_Pulse_0_Sample = 0;
			apu_Pulse_0_Duty_Value = false;

			apu_Pulse_0_Env_Start_Flag = 0;
			apu_Pulse_0_Env_Divider = 0;
			apu_Pulse_0_Env_Counter = 0;
			apu_Pulse_0_Env_Output = 0;

			apu_Pulse_1_Duty_Count = 0;
			apu_Pulse_1_Env_Loop = 0;
			apu_Pulse_1_Env_Constant = 0;
			apu_Pulse_1_Env_Count_Value = 0;

			apu_Pulse_1_Sweep_Enable = 0;
			apu_Pulse_1_Sweep_Divider_Count = 0;
			apu_Pulse_1_Sweep_Negate = 0;
			apu_Pulse_1_Sweep_Shiftcount = 0;
			apu_Pulse_1_Sweep_Reload = false;

			apu_Pulse_1_Len_Count = 0;
			apu_Pulse_1_Timer_Raw_Reload_Value = 0;
			apu_Pulse_1_Timer_Reload_Value = 0;

			apu_Pulse_1_Lenctr_Enable = 0;

			apu_Pulse_1_Sweep_Divider_Counter = 0;
			apu_Pulse_1_Sweep_Silence = false;
			apu_Pulse_1_Duty_Step = 0;
			apu_Pulse_1_Timer_Counter = 0;
			apu_Pulse_1_Sample = 0;
			apu_Pulse_1_Duty_Value = false;

			apu_Pulse_1_Env_Start_Flag = 0;
			apu_Pulse_1_Env_Divider = 0;
			apu_Pulse_1_Env_Counter = 0;
			apu_Pulse_1_Env_Output = 0;

			apu_Noise_Env_Count_Value = 0;
			apu_Noise_Env_Loop = 0;
			apu_Noise_Env_Constant = 0;
			apu_Noise_Mode_Count = 0;
			apu_Noise_Period_Count = 0;

			apu_Noise_Len_Count = 0;
			apu_Noise_Lenctr_Enable = 0;

			apu_Noise_Shift_Register = 1;
			apu_Noise_Timer_Counter = 0;
			apu_Noise_Sample = 0;

			Noise_env_output = 0;
			Noise_env_start_flag = 0;
			Noise_env_divider = 0;
			Noise_env_counter = 0;
			apu_Noise_Noise_Bit = true;

			apu_Triangle_Linear_Counter_Reload = 0;
			apu_Triangle_Control_Flag = 0;
			apu_Triangle_Timer_Count = 0;
			apu_Triangle_Reload_Flag = 0;
			apu_Triangle_Len_Count = 0;

			apu_Triangle_Lenctr_Enable = 0;
			apu_Triangle_Linear_Counter = 0;
			apu_Triangle_Timer = 0;
			apu_Triangle_Timer_Count_Reload = 0;
			apu_Triangle_Seq = 0;
			apu_Triangle_Sample = 0;

			apu_DMC_IRQ = false;
			apu_DMC_IRQ_Flag = false;
			apu_DMC_IRQ_Enabled = false;
			apu_DMC_Loop_Flag = false;
			apu_DMC_Call_From_Write = false;
			apu_DMC_Fill_Glitch_1 = false;
			apu_DMC_Fill_Glitch_2 = false;
			apu_DMC_Fill_Glitch_3 = false;

			apu_DMC_Timer_Reload = DMC_RATE[0];

			apu_DMC_Timer = 1020; // confirmed in VisualNES, on console it seems the APU runs a couple cycles before CPU exits reset
			apu_DMC_User_Address = 0xC000;
			apu_DMC_User_Length = 1;

			apu_DMC_Sample_Address = 0xC000;
			apu_DMC_Sample_Length = 0;
			apu_DMC_Sample_Buffer = 0;
			apu_DMC_Sample_Buffer_Filled = false;

			apu_DMC_Out_Shift = 0;
			apu_DMC_Out_Bits_Remaining = 7; // confirmed in VisualNES
			apu_DMC_Out_Deltacounter = 64;
			apu_DMC_Out_Silence = true;

			apu_DMC_Delay = 0;
			apu_DMC_Length_Countdown = 0;

			apu_DMC_RDY_Check = 0;
			apu_DMC_DMA_Countdown = -1;
		}

		uint8_t* apu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(apu_Recalculate, saver);
			saver = bool_saver(apu_Len_Clock_Active, saver);
			saver = bool_saver(apu_Get_Cycle, saver);

			// Sequencer
			saver = bool_saver(apu_Sequencer_IRQ, saver);
			saver = bool_saver(apu_Sequencer_IRQ_Flag, saver);
			saver = bool_saver(apu_Sequencer_Inhibit, saver);
			saver = bool_saver(apu_Sequencer_Mode, saver);
			saver = bool_saver(apu_Sequencer_Half, saver);
			saver = bool_saver(apu_Sequencer_Quarter, saver);
			saver = bool_saver(apu_Sequencer_Flag_Trigger, saver);

			saver = byte_saver(apu_Sequencer_Clear_Pending, saver);		
			saver = byte_saver(apu_Sequencer_Value, saver);
			saver = byte_saver(apu_Write_Value, saver);

			saver = int_saver(apu_Sequencer_Counter, saver);
			saver = int_saver(apu_Sequencer_Step, saver);
			saver = int_saver(apu_Sequencer_Write_Countdown, saver);
			saver = int_saver(apu_Sequencer_IRQ_Assert, saver);

			saver = int_saver(apu_Old_Cart_Sound, saver);
			saver = int_saver(apu_Audio_Sample_Clock, saver);
			saver = int_saver(apu_Write_Countdown, saver);
			saver = int_saver(apu_Write_Address, saver);

			saver = int_saver(apu_Old_Mix, saver);
			saver = int_saver(apu_Cart_Sound, saver);

			// Pulse_0
			saver = bool_saver(apu_Pulse_0_Sweep_Reload, saver);
			saver = bool_saver(apu_Pulse_0_Sweep_Silence, saver);
			saver = bool_saver(apu_Pulse_0_Duty_Value, saver);

			saver = int_saver(apu_Pulse_0_Duty_Count, saver);
			saver = int_saver(apu_Pulse_0_Env_Loop, saver);
			saver = int_saver(apu_Pulse_0_Env_Constant, saver);
			saver = int_saver(apu_Pulse_0_Env_Count_Value, saver);
			saver = int_saver(apu_Pulse_0_Sweep_Enable, saver);
			saver = int_saver(apu_Pulse_0_Sweep_Divider_Count, saver);
			saver = int_saver(apu_Pulse_0_Sweep_Negate, saver);
			saver = int_saver(apu_Pulse_0_Sweep_Shiftcount, saver);
			saver = int_saver(apu_Pulse_0_Len_Count, saver);
			saver = int_saver(apu_Pulse_0_Timer_Raw_Reload_Value, saver);
			saver = int_saver(apu_Pulse_0_Timer_Reload_Value, saver);
			saver = int_saver(apu_Pulse_0_Lenctr_Enable, saver);
			saver = int_saver(apu_Pulse_0_Sweep_Divider_Counter, saver);
			saver = int_saver(apu_Pulse_0_Duty_Step, saver);
			saver = int_saver(apu_Pulse_0_Timer_Counter, saver);
			saver = int_saver(apu_Pulse_0_Sample, saver);
			saver = int_saver(apu_Pulse_0_Env_Start_Flag, saver);
			saver = int_saver(apu_Pulse_0_Env_Divider, saver);
			saver = int_saver(apu_Pulse_0_Env_Counter, saver);
			saver = int_saver(apu_Pulse_0_Env_Output, saver);

			// Pulse_1
			saver = bool_saver(apu_Pulse_1_Sweep_Reload, saver);
			saver = bool_saver(apu_Pulse_1_Sweep_Silence, saver);
			saver = bool_saver(apu_Pulse_1_Duty_Value, saver);

			saver = int_saver(apu_Pulse_1_Duty_Count, saver);
			saver = int_saver(apu_Pulse_1_Env_Loop, saver);
			saver = int_saver(apu_Pulse_1_Env_Constant, saver);
			saver = int_saver(apu_Pulse_1_Env_Count_Value, saver);
			saver = int_saver(apu_Pulse_1_Sweep_Enable, saver);
			saver = int_saver(apu_Pulse_1_Sweep_Divider_Count, saver);
			saver = int_saver(apu_Pulse_1_Sweep_Negate, saver);
			saver = int_saver(apu_Pulse_1_Sweep_Shiftcount, saver);
			saver = int_saver(apu_Pulse_1_Len_Count, saver);
			saver = int_saver(apu_Pulse_1_Timer_Raw_Reload_Value, saver);
			saver = int_saver(apu_Pulse_1_Timer_Reload_Value, saver);
			saver = int_saver(apu_Pulse_1_Lenctr_Enable, saver);
			saver = int_saver(apu_Pulse_1_Sweep_Divider_Counter, saver);
			saver = int_saver(apu_Pulse_1_Duty_Step, saver);
			saver = int_saver(apu_Pulse_1_Timer_Counter, saver);
			saver = int_saver(apu_Pulse_1_Sample, saver);
			saver = int_saver(apu_Pulse_1_Env_Start_Flag, saver);
			saver = int_saver(apu_Pulse_1_Env_Divider, saver);
			saver = int_saver(apu_Pulse_1_Env_Counter, saver);
			saver = int_saver(apu_Pulse_1_Env_Output, saver);

			// Noise
			saver = bool_saver(apu_Noise_Noise_Bit, saver);

			saver = int_saver(apu_Noise_Env_Count_Value, saver);
			saver = int_saver(apu_Noise_Env_Loop, saver);
			saver = int_saver(apu_Noise_Env_Constant, saver);
			saver = int_saver(apu_Noise_Mode_Count, saver);
			saver = int_saver(apu_Noise_Period_Count, saver);
			saver = int_saver(apu_Noise_Len_Count, saver);
			saver = int_saver(apu_Noise_Lenctr_Enable, saver);
			saver = int_saver(apu_Noise_Shift_Register, saver);
			saver = int_saver(apu_Noise_Timer_Counter, saver);
			saver = int_saver(apu_Noise_Sample, saver);
			saver = int_saver(Noise_env_output, saver);
			saver = int_saver(Noise_env_start_flag, saver);
			saver = int_saver(Noise_env_divider, saver);
			saver = int_saver(Noise_env_counter, saver);

			// Triangle
			saver = int_saver(apu_Triangle_Linear_Counter_Reload, saver);
			saver = int_saver(apu_Triangle_Control_Flag, saver);
			saver = int_saver(apu_Triangle_Timer_Count, saver);
			saver = int_saver(apu_Triangle_Reload_Flag, saver);
			saver = int_saver(apu_Triangle_Len_Count, saver);
			saver = int_saver(apu_Triangle_Lenctr_Enable, saver);
			saver = int_saver(apu_Triangle_Linear_Counter, saver);
			saver = int_saver(apu_Triangle_Timer, saver);
			saver = int_saver(apu_Triangle_Timer_Count_Reload, saver);
			saver = int_saver(apu_Triangle_Seq, saver);
			saver = int_saver(apu_Triangle_Sample, saver);

			// DMC
			saver = bool_saver(apu_DMC_IRQ, saver);
			saver = bool_saver(apu_DMC_IRQ_Flag, saver);
			saver = bool_saver(apu_DMC_IRQ_Enabled, saver);
			saver = bool_saver(apu_DMC_Loop_Flag, saver);
			saver = bool_saver(apu_DMC_Out_Silence, saver);
			saver = bool_saver(apu_DMC_Sample_Buffer_Filled, saver);
			saver = bool_saver(apu_DMC_Call_From_Write, saver);
			saver = bool_saver(apu_DMC_Fill_Glitch_1, saver);
			saver = bool_saver(apu_DMC_Fill_Glitch_2, saver);
			saver = bool_saver(apu_DMC_Fill_Glitch_3, saver);

			saver = int_saver(apu_DMC_Timer_Reload, saver);
			saver = int_saver(apu_DMC_Delay, saver);
			saver = int_saver(apu_DMC_Timer, saver);
			saver = int_saver(apu_DMC_User_Address, saver);
			saver = int_saver(apu_DMC_Sample_Address, saver);
			saver = int_saver(apu_DMC_Sample_Buffer, saver);
			saver = int_saver(apu_DMC_Out_Shift, saver);
			saver = int_saver(apu_DMC_Out_Bits_Remaining, saver);
			saver = int_saver(apu_DMC_Out_Deltacounter, saver);

			saver = int_saver(apu_DMC_User_Length, saver);
			saver = int_saver(apu_DMC_Sample_Length, saver);
			saver = int_saver(apu_DMC_Length_Countdown, saver);

			saver = int_saver(apu_DMC_RDY_Check, saver);
			saver = int_saver(apu_DMC_DMA_Countdown, saver);
			
			return saver;
		}

		uint8_t* apu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&apu_Recalculate, loader);
			loader = bool_loader(&apu_Len_Clock_Active, loader);
			loader = bool_loader(&apu_Get_Cycle, loader);

			// Sequencer
			loader = bool_loader(&apu_Sequencer_IRQ, loader);
			loader = bool_loader(&apu_Sequencer_IRQ_Flag, loader);
			loader = bool_loader(&apu_Sequencer_Inhibit, loader);
			loader = bool_loader(&apu_Sequencer_Mode, loader);
			loader = bool_loader(&apu_Sequencer_Half, loader);
			loader = bool_loader(&apu_Sequencer_Quarter, loader);
			loader = bool_loader(&apu_Sequencer_Flag_Trigger, loader);

			loader = byte_loader(&apu_Sequencer_Clear_Pending, loader);
			loader = byte_loader(&apu_Sequencer_Value, loader);
			loader = byte_loader(&apu_Write_Value, loader);

			loader = int_loader(&apu_Sequencer_Counter, loader);
			loader = int_loader(&apu_Sequencer_Step, loader);
			loader = int_loader(&apu_Sequencer_Write_Countdown, loader);
			loader = int_loader(&apu_Sequencer_IRQ_Assert, loader);

			loader = int_loader(&apu_Old_Cart_Sound, loader);
			loader = int_loader(&apu_Audio_Sample_Clock, loader);
			loader = int_loader(&apu_Write_Countdown, loader);
			loader = int_loader(&apu_Write_Address, loader);

			loader = sint_loader(&apu_Old_Mix, loader);
			loader = sint_loader(&apu_Cart_Sound, loader);

			// Pulse_0
			loader = bool_loader(&apu_Pulse_0_Sweep_Reload, loader);
			loader = bool_loader(&apu_Pulse_0_Sweep_Silence, loader);
			loader = bool_loader(&apu_Pulse_0_Duty_Value, loader);

			loader = int_loader(&apu_Pulse_0_Duty_Count, loader);
			loader = int_loader(&apu_Pulse_0_Env_Loop, loader);
			loader = int_loader(&apu_Pulse_0_Env_Constant, loader);
			loader = int_loader(&apu_Pulse_0_Env_Count_Value, loader);
			loader = int_loader(&apu_Pulse_0_Sweep_Enable, loader);
			loader = int_loader(&apu_Pulse_0_Sweep_Divider_Count, loader);
			loader = int_loader(&apu_Pulse_0_Sweep_Negate, loader);
			loader = int_loader(&apu_Pulse_0_Sweep_Shiftcount, loader);
			loader = int_loader(&apu_Pulse_0_Len_Count, loader);
			loader = int_loader(&apu_Pulse_0_Timer_Raw_Reload_Value, loader);
			loader = int_loader(&apu_Pulse_0_Timer_Reload_Value, loader);
			loader = int_loader(&apu_Pulse_0_Lenctr_Enable, loader);
			loader = int_loader(&apu_Pulse_0_Sweep_Divider_Counter, loader);
			loader = int_loader(&apu_Pulse_0_Duty_Step, loader);
			loader = int_loader(&apu_Pulse_0_Timer_Counter, loader);
			loader = int_loader(&apu_Pulse_0_Sample, loader);
			loader = int_loader(&apu_Pulse_0_Env_Start_Flag, loader);
			loader = int_loader(&apu_Pulse_0_Env_Divider, loader);
			loader = int_loader(&apu_Pulse_0_Env_Counter, loader);
			loader = int_loader(&apu_Pulse_0_Env_Output, loader);

			// Pulse_1
			loader = bool_loader(&apu_Pulse_1_Sweep_Reload, loader);
			loader = bool_loader(&apu_Pulse_1_Sweep_Silence, loader);
			loader = bool_loader(&apu_Pulse_1_Duty_Value, loader);

			loader = int_loader(&apu_Pulse_1_Duty_Count, loader);
			loader = int_loader(&apu_Pulse_1_Env_Loop, loader);
			loader = int_loader(&apu_Pulse_1_Env_Constant, loader);
			loader = int_loader(&apu_Pulse_1_Env_Count_Value, loader);
			loader = int_loader(&apu_Pulse_1_Sweep_Enable, loader);
			loader = int_loader(&apu_Pulse_1_Sweep_Divider_Count, loader);
			loader = int_loader(&apu_Pulse_1_Sweep_Negate, loader);
			loader = int_loader(&apu_Pulse_1_Sweep_Shiftcount, loader);
			loader = int_loader(&apu_Pulse_1_Len_Count, loader);
			loader = int_loader(&apu_Pulse_1_Timer_Raw_Reload_Value, loader);
			loader = int_loader(&apu_Pulse_1_Timer_Reload_Value, loader);
			loader = int_loader(&apu_Pulse_1_Lenctr_Enable, loader);
			loader = int_loader(&apu_Pulse_1_Sweep_Divider_Counter, loader);
			loader = int_loader(&apu_Pulse_1_Duty_Step, loader);
			loader = int_loader(&apu_Pulse_1_Timer_Counter, loader);
			loader = int_loader(&apu_Pulse_1_Sample, loader);
			loader = int_loader(&apu_Pulse_1_Env_Start_Flag, loader);
			loader = int_loader(&apu_Pulse_1_Env_Divider, loader);
			loader = int_loader(&apu_Pulse_1_Env_Counter, loader);
			loader = int_loader(&apu_Pulse_1_Env_Output, loader);

			// Noise
			loader = bool_loader(&apu_Noise_Noise_Bit, loader);

			loader = int_loader(&apu_Noise_Env_Count_Value, loader);
			loader = int_loader(&apu_Noise_Env_Loop, loader);
			loader = int_loader(&apu_Noise_Env_Constant, loader);
			loader = int_loader(&apu_Noise_Mode_Count, loader);
			loader = int_loader(&apu_Noise_Period_Count, loader);
			loader = int_loader(&apu_Noise_Len_Count, loader);
			loader = int_loader(&apu_Noise_Lenctr_Enable, loader);
			loader = int_loader(&apu_Noise_Shift_Register, loader);
			loader = int_loader(&apu_Noise_Timer_Counter, loader);
			loader = int_loader(&apu_Noise_Sample, loader);
			loader = int_loader(&Noise_env_output, loader);
			loader = int_loader(&Noise_env_start_flag, loader);
			loader = int_loader(&Noise_env_divider, loader);
			loader = int_loader(&Noise_env_counter, loader);

			// Triangle
			loader = int_loader(&apu_Triangle_Linear_Counter_Reload, loader);
			loader = int_loader(&apu_Triangle_Control_Flag, loader);
			loader = int_loader(&apu_Triangle_Timer_Count, loader);
			loader = int_loader(&apu_Triangle_Reload_Flag, loader);
			loader = int_loader(&apu_Triangle_Len_Count, loader);
			loader = int_loader(&apu_Triangle_Lenctr_Enable, loader);
			loader = int_loader(&apu_Triangle_Linear_Counter, loader);
			loader = int_loader(&apu_Triangle_Timer, loader);
			loader = int_loader(&apu_Triangle_Timer_Count_Reload, loader);
			loader = int_loader(&apu_Triangle_Seq, loader);
			loader = int_loader(&apu_Triangle_Sample, loader);

			// DMC
			loader = bool_loader(&apu_DMC_IRQ, loader);
			loader = bool_loader(&apu_DMC_IRQ_Flag, loader);
			loader = bool_loader(&apu_DMC_IRQ_Enabled, loader);
			loader = bool_loader(&apu_DMC_Loop_Flag, loader);
			loader = bool_loader(&apu_DMC_Out_Silence, loader);
			loader = bool_loader(&apu_DMC_Sample_Buffer_Filled, loader);
			loader = bool_loader(&apu_DMC_Call_From_Write, loader);
			loader = bool_loader(&apu_DMC_Fill_Glitch_1, loader);
			loader = bool_loader(&apu_DMC_Fill_Glitch_2, loader);
			loader = bool_loader(&apu_DMC_Fill_Glitch_3, loader);

			loader = int_loader(&apu_DMC_Timer_Reload, loader);
			loader = int_loader(&apu_DMC_Delay, loader);
			loader = int_loader(&apu_DMC_Timer, loader);
			loader = int_loader(&apu_DMC_User_Address, loader);
			loader = int_loader(&apu_DMC_Sample_Address, loader);
			loader = int_loader(&apu_DMC_Sample_Buffer, loader);
			loader = int_loader(&apu_DMC_Out_Shift, loader);
			loader = int_loader(&apu_DMC_Out_Bits_Remaining, loader);
			loader = int_loader(&apu_DMC_Out_Deltacounter, loader);

			loader = int_loader(&apu_DMC_User_Length, loader);
			loader = int_loader(&apu_DMC_Sample_Length, loader);
			loader = int_loader(&apu_DMC_Length_Countdown, loader);

			loader = sint_loader(&apu_DMC_RDY_Check, loader);
			loader = sint_loader(&apu_DMC_DMA_Countdown, loader);

			return loader;
		}

	#pragma endregion

	#pragma region State Save / Load Functions
		uint8_t* bool_saver(bool to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save ? 1 : 0); saver++;

			return saver;
		}

		uint8_t* byte_saver(uint8_t to_save, uint8_t* saver)
		{
			*saver = to_save; saver++;

			return saver;
		}

		uint8_t* short_saver(uint16_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;

			return saver;
		}

		uint8_t* int_saver(uint32_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;

			return saver;
		}

		uint8_t* long_saver(uint64_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 40) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 56) & 0xFF); saver++;

			return saver;
		}

		uint8_t* bool_array_saver(bool* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++) { *saver = (uint8_t)(to_save[i] ? 1 : 0); saver++; }

			return saver;
		}

		uint8_t* byte_array_saver(uint8_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++) { *saver = to_save[i]; saver++; }

			return saver;
		}

		uint8_t* short_array_saver(uint16_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++)
			{ 
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* int_array_saver(uint32_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++)
			{
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* long_array_saver(uint64_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++)
			{
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 40) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 56) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* bool_loader(bool* to_load, uint8_t* loader)
		{
			to_load[0] = *loader == 1; loader++;

			return loader;
		}

		uint8_t* byte_loader(uint8_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;

			return loader;
		}

		uint8_t* short_loader(uint16_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;
			to_load[0] |= ((uint16_t)(*loader) << 8); loader++;

			return loader;
		}

		uint8_t* int_loader(uint32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= ((uint32_t)(*loader) << 8); loader++;
			to_load[0] |= ((uint32_t)(*loader) << 16); loader++; to_load[0] |= ((uint32_t)(*loader) << 24); loader++;

			return loader;
		}

		uint8_t* sint_loader(int32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= ((int32_t)(*loader) << 8); loader++;
			to_load[0] |= ((int32_t)(*loader) << 16); loader++; to_load[0] |= ((int32_t)(*loader) << 24); loader++;

			return loader;
		}

		uint8_t* long_loader(uint64_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (uint64_t)(* loader) << 8; loader++;
			to_load[0] |= (uint64_t)(*loader) << 16; loader++; to_load[0] |= (uint64_t)(*loader) << 24; loader++;
			to_load[0] |= (uint64_t)(*loader) << 32; loader++; to_load[0] |= (uint64_t)(*loader) << 40; loader++;
			to_load[0] |= (uint64_t)(*loader) << 48; loader++; to_load[0] |= (uint64_t)(*loader) << 56; loader++;

			return loader;
		}

		uint8_t* slong_loader(int64_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (int64_t)(*loader) << 8; loader++;
			to_load[0] |= (int64_t)(*loader) << 16; loader++; to_load[0] |= (int64_t)(*loader) << 24; loader++;
			to_load[0] |= (int64_t)(*loader) << 32; loader++; to_load[0] |= (int64_t)(*loader) << 40; loader++;
			to_load[0] |= (int64_t)(*loader) << 48; loader++; to_load[0] |= (int64_t)(*loader) << 56; loader++;

			return loader;
		}

		uint8_t* bool_array_loader(bool* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++) { to_load[i] = *loader == 1; loader++; }

			return loader;
		}

		uint8_t* byte_array_loader(uint8_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++) { to_load[i] = *loader; loader++; }

			return loader;
		}

		uint8_t* short_array_loader(uint16_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= ((uint16_t)(*loader) << 8); loader++;
			}

			return loader;
		}

		uint8_t* int_array_loader(uint32_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= ((uint32_t)(*loader) << 8); loader++;
				to_load[i] |= ((uint32_t)(*loader) << 16); loader++; to_load[i] |= ((uint32_t)(*loader) << 24); loader++;
			}

			return loader;
		}

		uint8_t* long_array_loader(uint64_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= (uint64_t)(*loader) << 8; loader++;
				to_load[i] |= (uint64_t)(*loader) << 16; loader++; to_load[i] |= (uint64_t)(*loader) << 24; loader++;
				to_load[i] |= (uint64_t)(*loader) << 32; loader++; to_load[i] |= (uint64_t)(*loader) << 40; loader++;
				to_load[i] |= (uint64_t)(*loader) << 48; loader++; to_load[i] |= (uint64_t)(*loader) << 56; loader++;
			}

			return loader;
		}

	#pragma endregion
	};
}

#endif