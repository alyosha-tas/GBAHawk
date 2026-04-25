#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "NES_System.h"
#include "Memory.h"
#include "Mappers.h"

#pragma region Mapper list includes
#include "CNROM.h"
#include "UxROM.h"
#include "AxROM.h"
#include "MMC5.h"
#include "MMC3.h"
#include "MMC2.h"
#include "MMC1.h"
#include "Mapper_030.h"
#include "Mapper_232.h"
#include "NROM_RetroCoders.h"
#pragma endregion


using namespace std;

namespace NESHawk
{
	class NESCore
	{
	public:
		NESCore() 
		{
			Mapper = nullptr;
			NES.MessageCallback = nullptr;
			NES.TraceCallback = nullptr;
			NES.HardReset();
		};

		NES_System NES;
		Mappers* Mapper;

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint8_t* ext_header, bool mmc3_old_irq, bool mapper_bus_conflicts, bool apu_test_regs, bool cpu_zero_reset, uint8_t special_flag)
		{
			NES.Use_APU_Test_Regs = apu_test_regs;
			
			NES.CPU_Zero_Set_Reset = cpu_zero_reset;

			if (NES.CPU_Zero_Set_Reset)
			{
				NES.cpu_FlagZset(true);
			}

			std::memcpy(NES.Header, ext_header, 0x10);

			uint16_t mapper_num = (NES.Header[6] >> 4);

			mapper_num |= (NES.Header[7] & 0xF0);

			mapper_num |= ((uint16_t)NES.Header[8] & 0xF) << 8;

			NES.ROM_Length = ((uint32_t)NES.Header[4] + (((uint32_t)NES.Header[9] & 0xF) << 8)) * 0x4000;

			NES.CHR_ROM_Length = ((uint32_t)NES.Header[5] + (((uint32_t)NES.Header[9] & 0xF0) << 4)) * 0x2000;

			int ofst_to_chr = NES.ROM_Length;

			if (NES.ROM_Length == 0x4000)
			{
				// copy to 32kb if only 16 kb present
				NES.ROM_Length = 0x8000;

				NES.ROM = new uint8_t[NES.ROM_Length];

				std::memcpy(NES.ROM, ext_rom, 0x4000);

				std::memcpy(NES.ROM + 0x4000, ext_rom, 0x4000);
			}
			else
			{
				NES.ROM = new uint8_t[NES.ROM_Length];

				std::memcpy(NES.ROM, ext_rom, NES.ROM_Length);
			}

			if (NES.CHR_ROM_Length > 0)
			{
				NES.CHR_ROM = new uint8_t[NES.CHR_ROM_Length];

				std::memcpy(NES.CHR_ROM, ext_rom + ofst_to_chr, NES.CHR_ROM_Length);
			}

			if (special_flag == 0)
			{
				switch (mapper_num)
				{
				case 0x00: Mapper = new Mapper_NROM(); break;
				case 0x01: Mapper = new Mapper_MMC1(); break;
				case 0x02: Mapper = new Mapper_UxROM(); break;
				case 0x03: Mapper = new Mapper_CNROM(); break;
				case 0x04: Mapper = new Mapper_MMC3(); break;
				case 0x05: Mapper = new Mapper_MMC5(); break;
				case 0x07: Mapper = new Mapper_AxROM(); break;
				case 0x09: Mapper = new Mapper_MMC2(); break;
				case 0x1E: Mapper = new Mapper_030(); break;
				case 0xE8: Mapper = new Mapper_232(); break;

				default: Mapper = new Mapper_NROM(); break;
				}
			}
			else
			{
				if (special_flag == 1)
				{
					Mapper = new NROM_RetroCoders(); // ROM expects to be able to read chip ID
				}
			}

			Mapper->Mirroring = ((NES.Header[6] & 0x1) == 1);

			NES.mapper_pntr = &Mapper[0];

			Mapper->Core_Cycle_Count = &NES.TotalExecutedCycles;

			Mapper->Core_Clock_Update_Cycle = &NES.Clock_Update_Cycle;

			Mapper->Core_ROM_Length = &NES.ROM_Length;

			// default mapping
			Mapper->Core_ROM_Base = &NES.ROM[0];
			Mapper->Core_CIRAM_Base = &NES.CIRAM[0];

			Mapper->Core_CHR_ROM_Length = &NES.CHR_ROM_Length;

			Mapper->Core_CHR_ROM = &NES.CHR_ROM[0];

			if (Mapper->Mirroring)
			{
				Mapper->Core_CIRAM[0] = &NES.CIRAM[0];
				Mapper->Core_CIRAM[1] = &NES.CIRAM[0x400];
				Mapper->Core_CIRAM[2] = &NES.CIRAM[0];
				Mapper->Core_CIRAM[3] = &NES.CIRAM[0x400];
			}
			else
			{
				Mapper->Core_CIRAM[0] = &NES.CIRAM[0];
				Mapper->Core_CIRAM[1] = &NES.CIRAM[0];
				Mapper->Core_CIRAM[2] = &NES.CIRAM[0x400];
				Mapper->Core_CIRAM[3] = &NES.CIRAM[0x400];
			}

			Mapper->Core_DB = &NES.DB_Ext;

			Mapper->Core_Message_String = &NES.Message_String;
			Mapper->MessageCallback = NES.MessageCallback;

			Mapper->Core_show_bg_new = &NES.show_bg_new;
			Mapper->Core_show_obj_new = &NES.show_bg_new;

			Mapper->Core_status_sl = &NES.status_sl;
			Mapper->Core_status_cycle = &NES.status_cycle;
			Mapper->Core_PPU_Phase = &NES.ppuphase;
			Mapper->Core_ppu_OBJ_Size_16 = &NES.ppu_OBJ_Size_16;

			Mapper->Reset();

			// set MMC3 IRQ Type
			Mapper->Old_IRQ_Type = mmc3_old_irq;

			// Bus conflicts for CNROM, UxROM, AxROM
			Mapper->Bus_Conflicts = mapper_bus_conflicts;

			NES.Cart_RAM_Length = 0;

			Mapper->Core_Cart_RAM_Length = &NES.Cart_RAM_Length;

			// Only reset cycle count on initial power on, not power cycles
			NES.Cycle_Count = 0;
			NES.Clock_Update_Cycle = 0;

			// Only certain carts require ppu clocking
			NES.ppu_HasClockPPU = false;

			if ((mapper_num == 4) || (mapper_num == 1) || (mapper_num == 5))
			{
				NES.ppu_HasClockPPU = true;
			}

			// Use alternate mirroring layout when applicable
			if ((mapper_num == 4) && ((NES.Header[6] & 0x8) == 0x8))
			{
				Mapper->Alt_Mirroring = true;
				
				Mapper->Core_CIRAM[0] = &Mapper->EXT_CIRAM[0];
				Mapper->Core_CIRAM[1] = &Mapper->EXT_CIRAM[0x400];
				Mapper->Core_CIRAM[2] = &Mapper->EXT_CIRAM[0x800];
				Mapper->Core_CIRAM[3] = &Mapper->EXT_CIRAM[0xC00];
			}

			NES.Mapper_Number = mapper_num;

			NES.Total_CPU_Clock_Cycles = 0;
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			NES.Cart_RAM = new uint8_t[ext_sram_size];

			NES.Cart_RAM_Length = ext_sram_size;

			std::memcpy(NES.Cart_RAM, ext_sram, ext_sram_size);

			Mapper->Core_Cart_RAM = &NES.Cart_RAM[0];

			Mapper->Size_Mask = ext_sram_size - 1;
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			std::memcpy(NES.Cart_RAM, ext_sram, ext_sram_size);
		}

		void Load_Palette(uint8_t* ext_palette)
		{
			std::memcpy(NES.Compiled_Palette, ext_palette, sizeof int32_t * 512);
		}

		void Hard_Reset() 
		{
			Mapper->Reset();
			
			NES.HardReset();
		}

		void Soft_Reset()
		{
			Mapper->Reset();

			NES.cpu_SoftReset();
			NES.apu_SoftReset();
			NES.ppu_SoftReset();
		}

		bool FrameAdvance(bool render, bool rendersound)
		{
			NES.apu_Audio_Sample_Clock = 0;

			NES.apu_Audio_Num_Samples = 0;

			NES.Is_Lag = true;

			NES.Frame_Advance();

			return NES.Is_Lag;
		}

		bool SubFrameAdvance( bool render, bool rendersound, bool do_reset, uint32_t reset_cycle)
		{
			NES.apu_Audio_Sample_Clock = 0;

			NES.apu_Audio_Num_Samples = 0;

			NES.frame_is_done = false;

			if (!do_reset) { reset_cycle = -1; }

			bool pass_a_frame = false;

			pass_a_frame = NES.SubFrame_Advance(reset_cycle);

			return pass_a_frame;
		}

		void SetControllerCallback(uint8_t (*callback)(bool))
		{
			NES.ReadController = callback;
		}

		void SetStrobeCallback(void(*callback)(uint8_t, uint8_t))
		{
			NES.StrobeController = callback;
		}

		uint8_t GetZapperState(int x, int y)
		{
			// assume radius 8 for now
			int radius = 8;

			int rad = 0;

			int bright = 0;

			int total = 0;

			int max_line = 0;

			int min_line = 0;

			if (NES.status_sl < 239)
			{
				max_line = (int)NES.status_sl;
			}
			else
			{
				max_line = 239;
			}

			min_line = (int)NES.status_sl - 10;

			if (min_line < 0) { min_line = 0; }

			for (int i = -radius; i <= radius; i++)
			{
				for (int j = -radius; j <= radius; j++)
				{
					if (((x + i) >= 0) && ((x + i) <= 255))
					{
						if (((y + j) >= min_line) && ((y + j) <= max_line))
						{
							rad = NES.Compiled_Palette[NES.xbuf[(y + j) * 256 + (x + i)]];

							bright = rad & 0xFF;
							bright += (rad >> 8) & 0xFF;
							bright += (rad >> 16) & 0xFF;

							total += bright;
						}
					}
				}
			}

			if (total > 20000)
			{
				return 0;
			}
			
			return 0x08;
		}

		void GetVideo(uint32_t* dest) 
		{
			for (int i = 0; i < 256 * 240; i++)
			{
				NES.video_buffer[i] = NES.Compiled_Palette[NES.xbuf[i] & 0x1FF];
			}
			
			uint32_t* src = NES.video_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof (uint32_t) * 256 * 240);

			// blank the screen
			for (int i = 0; i < 256 * 240; i++)
			{
				NES.video_buffer[i] = 0xFF000000;
				//NES.xbuf[i] = 0;
			}
		}

		uint32_t GetAudio(int32_t* dest, int32_t* n_samp)
		{
			int32_t* src = NES.apu_Audio_Samples;
			int32_t* dst = dest;

			std::memcpy(dst, src, sizeof int32_t * NES.apu_Audio_Num_Samples * 2);
			n_samp[0] = NES.apu_Audio_Num_Samples;

			uint32_t temp_int = NES.apu_Audio_Sample_Clock;
			
			return temp_int;
		}

		#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{	
			saver = NES.SaveState(saver);
			saver = Mapper->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = NES.LoadState(loader);
			loader = Mapper->LoadState(loader);
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return NES.PeekMemory(addr & 0xFFFF);
		}

		uint8_t GetVRAM(uint32_t addr) 
		{

			return Mapper->VRAM[(addr & 0x7FFF)];
		}

		uint8_t GetCHR_ROM(uint32_t addr)
		{
			if (NES.CHR_ROM_Length != 0)
			{
				return NES.CHR_ROM[addr & (NES.CHR_ROM_Length - 1)];
			}

			return 0;
		}

		uint8_t GetRAM(uint32_t addr)
		{
			return NES.RAM[addr & 0x7FF];
		}

		uint8_t GetROM(uint32_t addr)
		{
			return NES.ROM[addr & (NES.ROM_Length - 1)];
		}

		uint8_t GetOAM(uint32_t addr)
		{
			return NES.OAM[addr & 0xFF];
		}

		uint8_t GetPALRAM(uint32_t addr)
		{
			return NES.PALRAM[addr & 0x1F];
		}

		uint8_t GetSRAM(uint32_t addr)
		{
			if (NES.Cart_RAM_Length != 0) 
			{
				return NES.Cart_RAM[addr & (NES.Cart_RAM_Length - 1)];
			}
			
			return 0;
		}

		#pragma endregion

		#pragma region Tracer

		void SetTraceCallback(void (*callback)(int))
		{
			NES.TraceCallback = callback;
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
			std::memcpy(h, NES.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(r, NES.CPURegisterState().c_str(), l);
			}
			else if (t < 3)
			{
				std::memcpy(r, NES.Reg_Blank, l);
			}
			else if (t == 3)
			{
				// DMA info OAM
				std::memcpy(r, NES.CPUDMAStateOAM().c_str(), l);
			}
			else
			{
				// DMA info DMC
				std::memcpy(r, NES.CPUDMAStateDMC().c_str(), l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(d, NES.CPUDisassembly().c_str(), l);
			}
			else if (t == 1)
			{
				std::memcpy(d, NES.NMI_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, NES.IRQ_event, l);
			}
			else
			{
				std::memcpy(d, NES.DMA_event, l);
			}
		}

		#pragma endregion

		void SetMessageCallback(void (*callback)(int))
		{
			NES.MessageCallback = callback;
		}

		void SetInputPollCallback(void (*callback)())
		{
			NES.InputPollCallback = callback;
		}

		void GetMessage(char* d)
		{
			std::memcpy(d, NES.Message_String.c_str(), NES.Message_String.length() + 1);
		}

		bool Get_PPU_Values(int sel)
		{
			switch (sel)
			{
				case 0: return NES.ppu_BG_Pattern_High;
				case 1: return NES.ppu_OBJ_Pattern_High;
				case 2: return NES.ppu_OBJ_Size_16;
				case 3:
					if (NES.Mapper_Number == 5)
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
			return NES.ppubus_peek(addr);
		}

		uint8_t Get_PPU_Board_Peek(uint32_t addr)
		{
			return Mapper->PeekPPU(addr);
		}

		uint8_t* Get_PPU_Pointers(int sel)
		{
			switch (sel)
			{
				case 0: return NES.CHR_ROM; break;
				case 1: return NES.OAM; break;
				case 2: return NES.PALRAM; break;
				case 3: return Mapper->Ex_RAM; break;
				case 4: return Mapper->VRAM; break;
			}

			return nullptr;
		}

		void SetNTVCallback(void (*callback)(void), int sl)
		{
			NES.NTViewCallback = callback;
			NES.NTView_Scanline = sl;
		}

		void SetPPUCallback(void (*callback)(void), int sl)
		{
			NES.PPUViewCallback = callback;
			NES.PPUView_Scanline = sl;
		}
	};
}

#endif
