#ifndef MAPPERS_H
#define MAPPERS_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "../Common/Savestate.h"

using namespace std;

//Core_Message_String->assign("Bank: " + to_string(PRG_Bank) + " Mask: " + to_string(PRG_Mask) + " len: " + to_string(*Core_Cycle_Count));

//MessageCallback(Core_Message_String->length());

namespace NESHawk
{
	class Mappers
	{
	public:
	#pragma region mapper base
 
		// not stated, initialized with power on		
		// stated
		bool Mirroring;
		bool Bus_Conflicts;
		bool IRQ_Pending;
		bool IRQ_Enable;
		bool IRQ_Audio;
		bool IRQ_Reload_Flag;
		bool PRG_Mode;
		bool PRG_Slot;
		bool CHR_Mode;
		bool Just_Cleared;
		bool Just_Cleared_Pending;
		bool WRAM_Enable;
		bool WRAM_Write_Protect;
		bool Old_IRQ_Type;
		bool Alt_Mirroring;
		bool In_Frame;

		bool Chip_ID_Active;

		uint8_t Command;
		uint8_t IRQ_Reload;
		uint8_t IRQ_Counter;
		uint8_t IRQ_Target;
		uint8_t MMC1_CHR_0;
		uint8_t MMC1_CHR_1;
		uint8_t Serial_Shift_Count;
		uint8_t Serial_Shift_Value;
		uint8_t Multiplicand;
		uint8_t Multiplier;
		uint8_t Product_Low;
		uint8_t Product_High;
		uint8_t NT_Fill_Tile;
		uint8_t NT_Fill_Attrib;
		uint8_t MMC2_Latch_0;
		uint8_t MMC2_Latch_1;

		uint32_t CHR_Bank;
		uint32_t CHR_Bank_2;
		uint32_t CHR_Bank_3;
		uint32_t CHR_Bank_4;
		uint32_t CHR_Mask;
		uint32_t PRG_Bank;
		uint32_t PRG_Block;
		uint32_t PRG_Mask;
		uint32_t WRAM_Bank;
		uint32_t WRAM_Mask;
		uint32_t Last_NT_Read;

		uint32_t Size_Mask;
		uint32_t Mirror_Mode;
		uint32_t Reg_Addr;
		uint32_t Separator_Counter;
		uint32_t IRQ_Countdown;
		uint32_t A12_Old;
		uint32_t MMC1_PPU_Clock;
		uint32_t MMC5_PRG_Mode;
		uint32_t MMC5_CHR_Mode;
		uint32_t MMC5_ExRAM_Mode;
		uint32_t MMC5_AB_Mode;
		uint32_t MMC5_CHR_Reg_High;

		uint64_t Next_Ready_Cycle;

		uint8_t MMC3_Regs[8] = { };
		uint8_t MMC3_CHR_Regs_1K[8] = { };
		uint8_t MMC1_CHR_Regs_4K[2] = { };

		uint32_t MMC5_Regs_A[8] = { };
		uint32_t MMC5_Regs_B[4] = { };
		uint32_t MMC5_Regs_PRG[4] = { };
		uint32_t MMC5_NT_Modes[4] = { };
		uint32_t MMC5_A_Banks_1K[8] = { };
		uint32_t MMC5_B_Banks_1K[8] = { };
		uint32_t MMC5_PRG_Banks_8K[4] = { };

		// cartridge audio
		uint32_t MMC5_Audio_Frame_Reload = 7458; // ???
		
		bool MMC5_Audio_PCM_Read;
		bool MMC5_Audio_PCM_Enable_IRQ;
		bool MMC5_Audio_PCM_IRQ_Triggered;

		bool MMC5_Pulse_0_LenCntDisable;
		bool MMC5_Pulse_0_ConstantVolume;
		bool MMC5_Pulse_0_Enable;
		bool MMC5_Pulse_0_estart;

		bool MMC5_Pulse_1_LenCntDisable;
		bool MMC5_Pulse_1_ConstantVolume;
		bool MMC5_Pulse_1_Enable;
		bool MMC5_Pulse_1_estart;

		uint8_t MMC5_Audio_PCM_Val;
		uint8_t MMC5_Audio_PCM_NextVal;

		uint32_t MMC5_Audio_Frame;

		uint32_t MMC5_Pulse_0_V;
		uint32_t MMC5_Pulse_0_T;
		uint32_t MMC5_Pulse_0_L;
		uint32_t MMC5_Pulse_0_D;
		uint32_t MMC5_Pulse_0_etime;
		uint32_t MMC5_Pulse_0_ecount;
		uint32_t MMC5_Pulse_0_length;
		uint32_t MMC5_Pulse_0_sequence;
		uint32_t MMC5_Pulse_0_clock;
		uint32_t MMC5_Pulse_0_output;

		uint32_t MMC5_Pulse_1_V;
		uint32_t MMC5_Pulse_1_T;
		uint32_t MMC5_Pulse_1_L;
		uint32_t MMC5_Pulse_1_D;
		uint32_t MMC5_Pulse_1_etime;
		uint32_t MMC5_Pulse_1_ecount;
		uint32_t MMC5_Pulse_1_length;
		uint32_t MMC5_Pulse_1_sequence;
		uint32_t MMC5_Pulse_1_clock;
		uint32_t MMC5_Pulse_1_output;

		int32_t Cart_Audio_Output = 0;


		uint8_t Ex_RAM[0x400] = { };

		// always 32 k available, but most games only have access to 8
		uint8_t VRAM[0x8000] = { };

		// used by ex mmc3 in 4 screen mode, replaces internal CIRAM
		uint8_t EXT_CIRAM[0x2000] = { };

		bool* Core_show_bg_new = nullptr;

		bool* Core_show_obj_new = nullptr;

		bool* Core_ppu_OBJ_Size_16 = nullptr;

		uint64_t* Core_Cycle_Count = nullptr;

		uint64_t* Core_Clock_Update_Cycle = nullptr;

		uint8_t* Core_Cart_RAM = nullptr;

		uint32_t* Core_Cart_RAM_Length = nullptr;

		uint8_t* Core_ROM[4] = { nullptr, nullptr, nullptr, nullptr };

		uint8_t* Core_ROM_Base = nullptr;

		uint8_t* Core_CIRAM_Base = nullptr;

		uint32_t* Core_ROM_Length = nullptr;

		uint32_t* Core_status_sl = nullptr;

		uint32_t* Core_status_cycle = nullptr;

		uint32_t* Core_PPU_Phase = nullptr;

		uint8_t* Core_DB = nullptr;

		uint8_t* Core_CHR_ROM = nullptr;

		uint8_t* Core_CIRAM[4] = { nullptr, nullptr, nullptr, nullptr };

		uint32_t* Core_CHR_ROM_Length = nullptr;

		string* Core_Message_String = nullptr;

		void (*RumbleCallback)(bool);

		void (*MessageCallback)(int);
		
		Mappers()
		{
			Reset();
		}

		virtual uint8_t ReadPRG(uint32_t addr) { return Core_ROM_Base[addr]; }

		virtual uint8_t ReadExp(uint32_t addr) { return *Core_DB; }

		virtual uint8_t ReadWRAM(uint32_t addr)
		{
			if (*Core_Cart_RAM_Length > 0)
			{
				return Core_Cart_RAM[addr];
			}

			// not entirely accurate and hardware dependent
			// works for Battletoads and Castlevania III
			
			if ((addr & 4) == 0)
			{
				return (*Core_DB & 0x7F);
			}
			else
			{
				return (*Core_DB | 0x80);
			}
			
			//return *Core_DB;
		}

		virtual uint8_t Peek_Memory(uint32_t addr) { return 0; }

		virtual void Remap_ROM() { }

		virtual void WritePRG(uint32_t addr, uint8_t value){ }

		virtual void WriteExp(uint32_t addr, uint8_t value) { }

		virtual void WriteWRAM(uint32_t addr, uint8_t value)
		{
			if (*Core_Cart_RAM_Length > 0)
			{
				Core_Cart_RAM[addr] = value;
			}
		}

		virtual void Poke_Memory(uint32_t addr, uint8_t value) { }

		virtual void Dispose() { }

		virtual void Reset() { }

		virtual void Update_State() { }

		virtual void AddressPPU(uint32_t addr) { }

		virtual uint8_t ReadPPU(uint32_t addr)
		{ 
			if (addr < 0x2000)
			{
				if (*Core_CHR_ROM_Length > 0)
				{
					return Core_CHR_ROM[addr];
				}
				else
				{
					return VRAM[addr];
				}
			}

			uint32_t ofs = addr & 0x3FF;
			uint32_t block = (addr >> 10) & 3;

			return Core_CIRAM[block][ofs];
		}

		virtual uint8_t PeekPPU(uint32_t addr) { return ReadPPU(addr); }

		virtual uint8_t PeekReg2xxx(uint32_t addr) { return 0; }

		virtual uint8_t PeekCart(uint32_t addr)
		{
			if (addr >= 0x8000) { return ReadPRG(addr - 0x8000); }

			return *Core_DB;
		}

		virtual void WritePPU(uint32_t addr, uint8_t value)
		{
			if (addr >= 0x2000)
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				Core_CIRAM[block][ofs] = value;
			}
			else if (*Core_CHR_ROM_Length == 0)
			{
				VRAM[addr] = value;
			}
		}

		virtual void ClockCPU() {}

		virtual void ClockPPU() {}

		virtual void AtVsyncNmi() {}

		virtual bool IrqSignal() { return false; }

	#pragma endregion

	#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Mirroring, saver);
			saver = bool_saver(Bus_Conflicts, saver);
			saver = bool_saver(IRQ_Pending, saver);
			saver = bool_saver(IRQ_Enable, saver);
			saver = bool_saver(IRQ_Audio, saver);
			saver = bool_saver(IRQ_Reload_Flag, saver);
			saver = bool_saver(PRG_Mode, saver);
			saver = bool_saver(PRG_Slot, saver);
			saver = bool_saver(CHR_Mode, saver);
			saver = bool_saver(Just_Cleared, saver);
			saver = bool_saver(Just_Cleared_Pending, saver);
			saver = bool_saver(WRAM_Enable, saver);
			saver = bool_saver(WRAM_Write_Protect, saver);
			saver = bool_saver(Old_IRQ_Type, saver);
			saver = bool_saver(Alt_Mirroring, saver);
			saver = bool_saver(In_Frame, saver);
			saver = bool_saver(Chip_ID_Active, saver);

			saver = byte_saver(Command, saver);
			saver = byte_saver(IRQ_Reload, saver);
			saver = byte_saver(IRQ_Counter, saver);
			saver = byte_saver(IRQ_Target, saver);
			saver = byte_saver(MMC1_CHR_0, saver);
			saver = byte_saver(MMC1_CHR_1, saver);
			saver = byte_saver(Serial_Shift_Count, saver);
			saver = byte_saver(Serial_Shift_Value, saver);
			saver = byte_saver(Multiplicand, saver);
			saver = byte_saver(Multiplier, saver);
			saver = byte_saver(Product_Low, saver);
			saver = byte_saver(Product_High, saver);
			saver = byte_saver(NT_Fill_Tile, saver);
			saver = byte_saver(NT_Fill_Attrib, saver);
			saver = byte_saver(MMC2_Latch_0, saver);
			saver = byte_saver(MMC2_Latch_1, saver);

			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Bank_2, saver);
			saver = int_saver(CHR_Bank_3, saver);
			saver = int_saver(CHR_Bank_4, saver);
			saver = int_saver(CHR_Mask, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Block, saver);
			saver = int_saver(PRG_Mask, saver);
			saver = int_saver(WRAM_Bank, saver);
			saver = int_saver(WRAM_Mask, saver);
			saver = int_saver(Size_Mask, saver);
			saver = int_saver(Mirror_Mode, saver);
			saver = int_saver(Reg_Addr, saver);
			saver = int_saver(Separator_Counter, saver);
			saver = int_saver(IRQ_Countdown, saver);
			saver = int_saver(A12_Old, saver);
			saver = int_saver(MMC1_PPU_Clock, saver);
			saver = int_saver(MMC5_PRG_Mode, saver);
			saver = int_saver(MMC5_CHR_Mode, saver);
			saver = int_saver(MMC5_ExRAM_Mode, saver);
			saver = int_saver(MMC5_AB_Mode, saver);
			saver = int_saver(MMC5_CHR_Reg_High, saver);
			saver = int_saver(Last_NT_Read, saver);

			saver = long_saver(Next_Ready_Cycle, saver);

			saver = byte_array_saver(MMC3_Regs, saver, 8);
			saver = byte_array_saver(MMC3_CHR_Regs_1K, saver, 8);
			saver = byte_array_saver(MMC1_CHR_Regs_4K, saver, 2);

			saver = byte_array_saver(Ex_RAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x8000);
			saver = byte_array_saver(EXT_CIRAM, saver, 0x2000);

			saver = int_array_saver(MMC5_Regs_A, saver, 8);
			saver = int_array_saver(MMC5_Regs_B, saver, 4);
			saver = int_array_saver(MMC5_Regs_PRG, saver, 4);
			saver = int_array_saver(MMC5_NT_Modes, saver, 4);
			saver = int_array_saver(MMC5_A_Banks_1K, saver, 8);
			saver = int_array_saver(MMC5_B_Banks_1K, saver, 8);
			saver = int_array_saver(MMC5_PRG_Banks_8K, saver, 4);

			// cartridge audio
			saver = bool_saver(MMC5_Audio_PCM_Read, saver);
			saver = bool_saver(MMC5_Audio_PCM_Enable_IRQ, saver);
			saver = bool_saver(MMC5_Audio_PCM_IRQ_Triggered, saver);

			saver = byte_saver(MMC5_Audio_PCM_Val, saver);
			saver = byte_saver(MMC5_Audio_PCM_NextVal, saver);

			saver = int_saver(MMC5_Audio_Frame, saver);

			saver = bool_saver(MMC5_Pulse_0_LenCntDisable, saver);
			saver = bool_saver(MMC5_Pulse_0_ConstantVolume, saver);
			saver = bool_saver(MMC5_Pulse_0_Enable, saver);
			saver = bool_saver(MMC5_Pulse_0_estart, saver);
			saver = bool_saver(MMC5_Pulse_1_LenCntDisable, saver);
			saver = bool_saver(MMC5_Pulse_1_ConstantVolume, saver);
			saver = bool_saver(MMC5_Pulse_1_Enable, saver);
			saver = bool_saver(MMC5_Pulse_1_estart, saver);

			saver = int_saver(MMC5_Pulse_0_V, saver);
			saver = int_saver(MMC5_Pulse_0_T, saver);
			saver = int_saver(MMC5_Pulse_0_L, saver);
			saver = int_saver(MMC5_Pulse_0_D, saver);
			saver = int_saver(MMC5_Pulse_0_etime, saver);
			saver = int_saver(MMC5_Pulse_0_ecount, saver);
			saver = int_saver(MMC5_Pulse_0_length, saver);
			saver = int_saver(MMC5_Pulse_0_sequence, saver);
			saver = int_saver(MMC5_Pulse_0_clock, saver);
			saver = int_saver(MMC5_Pulse_0_output, saver);
			saver = int_saver(MMC5_Pulse_1_V, saver);
			saver = int_saver(MMC5_Pulse_1_T, saver);
			saver = int_saver(MMC5_Pulse_1_L, saver);
			saver = int_saver(MMC5_Pulse_1_D, saver);
			saver = int_saver(MMC5_Pulse_1_etime, saver);
			saver = int_saver(MMC5_Pulse_1_ecount, saver);
			saver = int_saver(MMC5_Pulse_1_length, saver);
			saver = int_saver(MMC5_Pulse_1_sequence, saver);
			saver = int_saver(MMC5_Pulse_1_clock, saver);
			saver = int_saver(MMC5_Pulse_1_output, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Mirroring, loader);
			loader = bool_loader(&Bus_Conflicts, loader);
			loader = bool_loader(&IRQ_Pending, loader);
			loader = bool_loader(&IRQ_Enable, loader);
			loader = bool_loader(&IRQ_Audio, loader);
			loader = bool_loader(&IRQ_Reload_Flag, loader);
			loader = bool_loader(&PRG_Mode, loader);
			loader = bool_loader(&PRG_Slot, loader);
			loader = bool_loader(&CHR_Mode, loader);
			loader = bool_loader(&Just_Cleared, loader);
			loader = bool_loader(&Just_Cleared_Pending, loader);
			loader = bool_loader(&WRAM_Enable, loader);
			loader = bool_loader(&WRAM_Write_Protect, loader);
			loader = bool_loader(&Old_IRQ_Type, loader);
			loader = bool_loader(&Alt_Mirroring, loader);
			loader = bool_loader(&In_Frame, loader);
			loader = bool_loader(&Chip_ID_Active, loader);

			loader = byte_loader(&Command, loader);
			loader = byte_loader(&IRQ_Reload, loader);
			loader = byte_loader(&IRQ_Counter, loader);
			loader = byte_loader(&IRQ_Target, loader);
			loader = byte_loader(&MMC1_CHR_0, loader);
			loader = byte_loader(&MMC1_CHR_1, loader);
			loader = byte_loader(&Serial_Shift_Count, loader);
			loader = byte_loader(&Serial_Shift_Value, loader);
			loader = byte_loader(&Multiplicand, loader);
			loader = byte_loader(&Multiplier, loader);
			loader = byte_loader(&Product_Low, loader);
			loader = byte_loader(&Product_High, loader);
			loader = byte_loader(&NT_Fill_Tile, loader);
			loader = byte_loader(&NT_Fill_Attrib, loader);
			loader = byte_loader(&MMC2_Latch_0, loader);
			loader = byte_loader(&MMC2_Latch_1, loader);

			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Bank_2, loader);
			loader = int_loader(&CHR_Bank_3, loader);
			loader = int_loader(&CHR_Bank_4, loader);
			loader = int_loader(&CHR_Mask, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Block, loader);
			loader = int_loader(&PRG_Mask, loader);
			loader = int_loader(&WRAM_Bank, loader);
			loader = int_loader(&WRAM_Mask, loader);
			loader = int_loader(&Size_Mask, loader);
			loader = int_loader(&Mirror_Mode, loader);
			loader = int_loader(&Reg_Addr, loader);
			loader = int_loader(&Separator_Counter, loader);
			loader = int_loader(&IRQ_Countdown, loader);
			loader = int_loader(&A12_Old, loader);
			loader = int_loader(&MMC1_PPU_Clock, loader);
			loader = int_loader(&MMC5_PRG_Mode, loader);
			loader = int_loader(&MMC5_CHR_Mode, loader);
			loader = int_loader(&MMC5_ExRAM_Mode, loader);
			loader = int_loader(&MMC5_AB_Mode, loader);
			loader = int_loader(&MMC5_CHR_Reg_High, loader);
			loader = int_loader(&Last_NT_Read, loader);

			loader = long_loader(&Next_Ready_Cycle, loader);

			loader = byte_array_loader(MMC3_Regs, loader, 8);
			loader = byte_array_loader(MMC3_CHR_Regs_1K, loader, 8);
			loader = byte_array_loader(MMC1_CHR_Regs_4K, loader, 2);

			loader = byte_array_loader(Ex_RAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x8000);
			loader = byte_array_loader(EXT_CIRAM, loader, 0x2000);

			loader = int_array_loader(MMC5_Regs_A, loader, 8);
			loader = int_array_loader(MMC5_Regs_B, loader, 4);
			loader = int_array_loader(MMC5_Regs_PRG, loader, 4);
			loader = int_array_loader(MMC5_NT_Modes, loader, 4);
			loader = int_array_loader(MMC5_A_Banks_1K, loader, 8);
			loader = int_array_loader(MMC5_B_Banks_1K, loader, 8);
			loader = int_array_loader(MMC5_PRG_Banks_8K, loader, 4);

			// cartridge audio
			loader = bool_loader(&MMC5_Audio_PCM_Read, loader);
			loader = bool_loader(&MMC5_Audio_PCM_Enable_IRQ, loader);
			loader = bool_loader(&MMC5_Audio_PCM_IRQ_Triggered, loader);

			loader = byte_loader(&MMC5_Audio_PCM_Val, loader);
			loader = byte_loader(&MMC5_Audio_PCM_NextVal, loader);

			loader = int_loader(&MMC5_Audio_Frame, loader);

			loader = bool_loader(&MMC5_Pulse_0_LenCntDisable, loader);
			loader = bool_loader(&MMC5_Pulse_0_ConstantVolume, loader);
			loader = bool_loader(&MMC5_Pulse_0_Enable, loader);
			loader = bool_loader(&MMC5_Pulse_0_estart, loader);
			loader = bool_loader(&MMC5_Pulse_1_LenCntDisable, loader);
			loader = bool_loader(&MMC5_Pulse_1_ConstantVolume, loader);
			loader = bool_loader(&MMC5_Pulse_1_Enable, loader);
			loader = bool_loader(&MMC5_Pulse_1_estart, loader);

			loader = int_loader(&MMC5_Pulse_0_V, loader);
			loader = int_loader(&MMC5_Pulse_0_T, loader);
			loader = int_loader(&MMC5_Pulse_0_L, loader);
			loader = int_loader(&MMC5_Pulse_0_D, loader);
			loader = int_loader(&MMC5_Pulse_0_etime, loader);
			loader = int_loader(&MMC5_Pulse_0_ecount, loader);
			loader = int_loader(&MMC5_Pulse_0_length, loader);
			loader = int_loader(&MMC5_Pulse_0_sequence, loader);
			loader = int_loader(&MMC5_Pulse_0_clock, loader);
			loader = int_loader(&MMC5_Pulse_0_output, loader);
			loader = int_loader(&MMC5_Pulse_1_V, loader);
			loader = int_loader(&MMC5_Pulse_1_T, loader);
			loader = int_loader(&MMC5_Pulse_1_L, loader);
			loader = int_loader(&MMC5_Pulse_1_D, loader);
			loader = int_loader(&MMC5_Pulse_1_etime, loader);
			loader = int_loader(&MMC5_Pulse_1_ecount, loader);
			loader = int_loader(&MMC5_Pulse_1_length, loader);
			loader = int_loader(&MMC5_Pulse_1_sequence, loader);
			loader = int_loader(&MMC5_Pulse_1_clock, loader);
			loader = int_loader(&MMC5_Pulse_1_output, loader);

			Remap_ROM();

			return loader;
		}

	#pragma endregion

	};

	#pragma region NROM

	class Mapper_NROM : public Mappers
	{
	public:

	};

	#pragma endregion
}

#endif