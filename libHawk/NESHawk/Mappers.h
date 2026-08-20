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
 
		// variables that are common for all mappers, other variables defined per mapper
		bool Mirroring;
		bool Bus_Conflicts;
		bool Old_IRQ_Type;
		bool Alt_Mirroring;

		uint32_t Size_Mask;

		// only savestated in actual MMC5 mapper
		uint32_t MMC5_ExRAM_Mode = 0;

		// only savestated in mapper siwth audio
		int32_t Cart_Audio_Output = 0;

		// on board RAM
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

		virtual uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Mirroring, saver);
			saver = bool_saver(Bus_Conflicts, saver);
			saver = bool_saver(Old_IRQ_Type, saver);
			saver = bool_saver(Alt_Mirroring, saver);

			saver = int_saver(Size_Mask, saver);

			saver = byte_array_saver(Ex_RAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x8000);
			saver = byte_array_saver(EXT_CIRAM, saver, 0x2000);
			
			return saver;
		}

		virtual uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Mirroring, loader);
			loader = bool_loader(&Bus_Conflicts, loader);
			loader = bool_loader(&Old_IRQ_Type, loader);
			loader = bool_loader(&Alt_Mirroring, loader);

			loader = int_loader(&Size_Mask, loader);

			loader = byte_array_loader(Ex_RAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x8000);
			loader = byte_array_loader(EXT_CIRAM, loader, 0x2000);
			
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