#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "SNES_System.h"
#include "Memory.h"
#include "Mappers.h"
#include "APU_System.h"

#pragma region Mapper list includes

#pragma endregion


using namespace std;

namespace SNESHawk
{
	class SNESCore
	{
	public:
		SNESCore() 
		{
			Mapper = nullptr;
			SNES.MessageCallback = nullptr;
			SNES.TraceCallback = nullptr;
			SNES.HardReset();
		};

		SNES_System SNES;
		Mappers* Mapper;
		APU_System APU;

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint8_t* ext_header, bool mmc3_old_irq, bool mapper_bus_conflicts, bool apu_test_regs, bool cpu_zero_reset)
		{
			SNES.Use_APU_Test_Regs = apu_test_regs;
			
			SNES.CPU_Zero_Set_Reset = cpu_zero_reset;

			if (SNES.CPU_Zero_Set_Reset)
			{
				SNES.cpu_FlagZset(true);
			}

			std::memcpy(SNES.Header, ext_header, 0x10);

			uint16_t mapper_num = (SNES.Header[6] >> 4);

			mapper_num |= (SNES.Header[7] & 0xF0);

			SNES.ROM_Length = (uint32_t)SNES.Header[4] * 0x4000;

			SNES.CHR_ROM_Length = (uint32_t)SNES.Header[5] * 0x2000;

			int ofst_to_chr = SNES.ROM_Length;

			if (SNES.ROM_Length == 0x4000)
			{
				// copy to 32kb if only 16 kb present
				SNES.ROM_Length = 0x8000;

				SNES.ROM = new uint8_t[SNES.ROM_Length];

				std::memcpy(SNES.ROM, ext_rom, 0x4000);

				std::memcpy(SNES.ROM + 0x4000, ext_rom, 0x4000);
			}
			else
			{
				SNES.ROM = new uint8_t[SNES.ROM_Length];

				std::memcpy(SNES.ROM, ext_rom, SNES.ROM_Length);
			}

			if (SNES.CHR_ROM_Length > 0)
			{
				SNES.CHR_ROM = new uint8_t[SNES.CHR_ROM_Length];

				std::memcpy(SNES.CHR_ROM, ext_rom + ofst_to_chr, SNES.CHR_ROM_Length);
			}

			switch (mapper_num)
			{
				case 0x00: Mapper = new Mapper_NROM(); break;
				
				default: Mapper = new Mapper_NROM(); break;
			}

			Mapper->Mirroring = ((SNES.Header[6] & 0x1) == 1);

			SNES.mapper_pntr = &Mapper[0];

			Mapper->Core_Cycle_Count = &SNES.TotalExecutedCycles;

			Mapper->Core_Clock_Update_Cycle = &SNES.Clock_Update_Cycle;

			Mapper->Core_ROM_Length = &SNES.ROM_Length;

			// default mapping
			Mapper->Core_ROM_Base = &SNES.ROM[0];
			Mapper->Core_CIRAM_Base = &SNES.CIRAM[0];

			Mapper->Core_CHR_ROM_Length = &SNES.CHR_ROM_Length;

			Mapper->Core_CHR_ROM = &SNES.CHR_ROM[0];

			if (Mapper->Mirroring)
			{
				Mapper->Core_CIRAM[0] = &SNES.CIRAM[0];
				Mapper->Core_CIRAM[1] = &SNES.CIRAM[0x400];
				Mapper->Core_CIRAM[2] = &SNES.CIRAM[0];
				Mapper->Core_CIRAM[3] = &SNES.CIRAM[0x400];
			}
			else
			{
				Mapper->Core_CIRAM[0] = &SNES.CIRAM[0];
				Mapper->Core_CIRAM[1] = &SNES.CIRAM[0];
				Mapper->Core_CIRAM[2] = &SNES.CIRAM[0x400];
				Mapper->Core_CIRAM[3] = &SNES.CIRAM[0x400];
			}

			Mapper->Core_DB = &SNES.DB;

			Mapper->Core_Message_String = &SNES.Message_String;
			Mapper->MessageCallback = SNES.MessageCallback;

			Mapper->Core_show_bg_new = &SNES.show_bg_new;
			Mapper->Core_show_obj_new = &SNES.show_bg_new;

			Mapper->Core_status_sl = &SNES.status_sl;
			Mapper->Core_status_cycle = &SNES.status_cycle;
			Mapper->Core_PPU_Phase = &SNES.ppuphase;
			Mapper->Core_ppu_OBJ_Size_16 = &SNES.ppu_OBJ_Size_16;

			Mapper->Reset();

			// set MMC3 IRQ Type
			Mapper->Old_IRQ_Type = mmc3_old_irq;

			// Bus conflicts for CNROM, UxROM, AxROM
			Mapper->Bus_Conflicts = mapper_bus_conflicts;

			SNES.Cart_RAM_Length = 0;

			Mapper->Core_Cart_RAM_Length = &SNES.Cart_RAM_Length;

			// Only reset cycle count on initial power on, not power cycles
			SNES.Cycle_Count = 0;
			SNES.Clock_Update_Cycle = 0;

			// Only certain carts require ppu clocking
			SNES.ppu_HasClockPPU = false;

			if ((mapper_num == 4) || (mapper_num == 1) || (mapper_num == 5))
			{
				SNES.ppu_HasClockPPU = true;
			}

			// Use alternate mirroring layout when applicable
			if ((mapper_num == 4) && ((SNES.Header[6] & 0x8) == 0x8))
			{
				Mapper->Alt_Mirroring = true;
				
				Mapper->Core_CIRAM[0] = &Mapper->EXT_CIRAM[0];
				Mapper->Core_CIRAM[1] = &Mapper->EXT_CIRAM[0x400];
				Mapper->Core_CIRAM[2] = &Mapper->EXT_CIRAM[0x800];
				Mapper->Core_CIRAM[3] = &Mapper->EXT_CIRAM[0xC00];
			}

			SNES.Mapper_Number = mapper_num;

			SNES.Total_CPU_Clock_Cycles = 0;
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			SNES.Cart_RAM = new uint8_t[ext_sram_size];

			SNES.Cart_RAM_Length = ext_sram_size;

			std::memcpy(SNES.Cart_RAM, ext_sram, ext_sram_size);

			Mapper->Core_Cart_RAM = &SNES.Cart_RAM[0];

			Mapper->Size_Mask = ext_sram_size - 1;
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			std::memcpy(SNES.Cart_RAM, ext_sram, ext_sram_size);
		}

		void Load_Palette(uint8_t* ext_palette)
		{
			std::memcpy(SNES.Compiled_Palette, ext_palette, sizeof int32_t * 512);
		}

		void Hard_Reset() 
		{
			Mapper->Reset();
			
			SNES.HardReset();
		}

		void Soft_Reset()
		{
			Mapper->Reset();

			SNES.cpu_SoftReset();
			SNES.apu_SoftReset();
			SNES.ppu_SoftReset();
		}

		bool FrameAdvance(bool render, bool rendersound)
		{
			SNES.apu_Audio_Sample_Clock = 0;

			SNES.apu_Audio_Num_Samples = 0;

			SNES.Is_Lag = true;

			SNES.Frame_Advance();

			return SNES.Is_Lag;
		}

		bool SubFrameAdvance( bool render, bool rendersound, bool do_reset, uint32_t reset_cycle)
		{
			SNES.apu_Audio_Sample_Clock = 0;

			SNES.apu_Audio_Num_Samples = 0;

			SNES.frame_is_done = false;

			if (!do_reset) { reset_cycle = -1; }

			bool pass_a_frame = false;

			pass_a_frame = SNES.SubFrame_Advance(reset_cycle);

			return pass_a_frame;
		}

		void SetControllerCallback(uint8_t (*callback)(bool))
		{
			SNES.ReadController = callback;
		}

		void SetStrobeCallback(void(*callback)(uint8_t, uint8_t))
		{
			SNES.StrobeController = callback;
		}

		void GetVideo(uint32_t* dest) 
		{
			for (int i = 0; i < 256 * 240; i++)
			{
				SNES.video_buffer[i] = SNES.Compiled_Palette[SNES.xbuf[i] & 0x1FF];
			}
			
			uint32_t* src = SNES.video_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof (uint32_t) * 256 * 240);

			// blank the screen
			for (int i = 0; i < 256 * 240; i++)
			{
				SNES.video_buffer[i] = 0xFF000000;
			}
		}

		uint32_t GetAudio(int32_t* dest, int32_t* n_samp)
		{
			int32_t* src = SNES.apu_Audio_Samples;
			int32_t* dst = dest;

			std::memcpy(dst, src, sizeof int32_t * SNES.apu_Audio_Num_Samples * 2);
			n_samp[0] = SNES.apu_Audio_Num_Samples;

			uint32_t temp_int = SNES.apu_Audio_Sample_Clock;
			
			return temp_int;
		}

		#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{	
			saver = SNES.SaveState(saver);
			saver = Mapper->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = SNES.LoadState(loader);
			loader = Mapper->LoadState(loader);
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return SNES.PeekMemory(addr & 0xFFFF);
		}

		uint8_t GetOAMBus(uint32_t addr)
		{
			return APU.PeekMemory(addr & 0xFFFF);
		}

		uint8_t GetVRAM(uint32_t addr) 
		{

			return Mapper->VRAM[(addr & 0x7FFF)];
		}

		uint8_t GetCHR_ROM(uint32_t addr)
		{
			if (SNES.CHR_ROM_Length != 0)
			{
				return SNES.CHR_ROM[addr & (SNES.CHR_ROM_Length - 1)];
			}

			return 0;
		}

		uint8_t GetRAM(uint32_t addr)
		{
			return SNES.RAM[addr & 0x7FF];
		}

		uint8_t GetROM(uint32_t addr)
		{
			return SNES.ROM[addr & (SNES.ROM_Length - 1)];
		}

		uint8_t GetOAM(uint32_t addr)
		{
			return SNES.OAM[addr & 0xFF];
		}

		uint8_t GetPALRAM(uint32_t addr)
		{
			return SNES.PALRAM[addr & 0x1F];
		}

		uint8_t GetSRAM(uint32_t addr)
		{
			if (SNES.Cart_RAM_Length != 0) 
			{
				return SNES.Cart_RAM[addr & (SNES.Cart_RAM_Length - 1)];
			}
			
			return 0;
		}

		#pragma endregion

		#pragma region Tracer

		void SetTraceCallback(void (*callback)(int))
		{
			SNES.TraceCallback = callback;
		}

		int GetHeaderLength()
		{
			return 124 + 1;
		}

		int GetDisasmLength()
		{
			return 38 + 1;
		}

		int GetRegStringLength()
		{
			return 91 + 1;
		}

		void GetHeader(char* h, int l)
		{
			std::memcpy(h, SNES.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(r, SNES.CPURegisterState().c_str(), l);
			}
			else if (t < 3)
			{
				std::memcpy(r, SNES.Reg_Blank, l);
			}
			else if (t == 3)
			{
				// DMA info OAM
				std::memcpy(r, SNES.CPUDMAStateOAM().c_str(), l);
			}
			else
			{
				// DMA info DMC
				std::memcpy(r, SNES.CPUDMAStateDMC().c_str(), l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(d, SNES.CPUDisassembly().c_str(), l);
			}
			else if (t == 1)
			{
				std::memcpy(d, SNES.NMI_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, SNES.IRQ_event, l);
			}
			else
			{
				std::memcpy(d, SNES.DMA_event, l);
			}
		}

		#pragma endregion

		void SetMessageCallback(void (*callback)(int))
		{
			SNES.MessageCallback = callback;
		}

		void SetInputPollCallback(void (*callback)())
		{
			SNES.InputPollCallback = callback;
		}

		void GetMessage(char* d)
		{
			std::memcpy(d, SNES.Message_String.c_str(), SNES.Message_String.length() + 1);
		}

		bool Get_PPU_Values(int sel)
		{
			switch (sel)
			{
				case 0: return SNES.ppu_BG_Pattern_High;
				case 1: return SNES.ppu_OBJ_Pattern_High;
				case 2: return SNES.ppu_OBJ_Size_16;
				case 3:
					if (SNES.Mapper_Number == 5)
					{
						return Mapper->MMC5_ExRAM_Mode == 1;
					}

					return false;
				default: return false;
			}
			
			return false;
		}

		uint8_t Get_PPU_Bus_Peek(uint32_t addr)
		{
			return SNES.ppubus_peek(addr);
		}

		uint8_t Get_PPU_Board_Peek(uint32_t addr)
		{
			return Mapper->PeekPPU(addr);
		}

		uint8_t* Get_PPU_Pointers(int sel)
		{
			switch (sel)
			{
				case 0: return SNES.CHR_ROM; break;
				case 1: return SNES.OAM; break;
				case 2: return SNES.PALRAM; break;
				case 3: return Mapper->Ex_RAM; break;
				case 4: return Mapper->VRAM; break;
			}

			return nullptr;
		}

		void SetNTVCallback(void (*callback)(void), int sl)
		{
			SNES.NTViewCallback = callback;
			SNES.NTView_Scanline = sl;
		}

		void SetPPUCallback(void (*callback)(void), int sl)
		{
			SNES.PPUViewCallback = callback;
			SNES.PPUView_Scanline = sl;
		}
	};
}

#endif
