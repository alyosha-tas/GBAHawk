#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "SNES_System.h"
#include "APU.h"
#include "Memory.h"

#pragma region coprocessor list includes

#pragma endregion


using namespace std;

namespace SNESHawk
{
	class SNESCore
	{
	public:
		SNESCore() 
		{
			SNES.MessageCallback = nullptr;
			SNES.TraceCallback = nullptr;
			SNES.HardReset();
			APU.HardReset();
		};

		SNES_System SNES;
		APU APU;

		void Load_IPL(uint8_t* ipl)
		{
			std::memcpy(APU.IPL, ipl, 0x40);
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint8_t* ext_header, uint32_t apu_freq, uint32_t ppu_h_pos, uint32_t ppu_v_pos, uint32_t dram_pos)
		{
			SNES.APU_Frequency = apu_freq;
			SNES.PPU_H_Pos_Reset = ppu_h_pos;		
			SNES.PPU_V_Pos_Reset = ppu_v_pos;
			SNES.DRAM_Refresh_Pos = dram_pos;

			SNES.ROM_Length = ext_rom_size;

			SNES.ROM = new uint8_t[SNES.ROM_Length];

			std::memcpy(SNES.ROM, ext_rom, SNES.ROM_Length);

			std::memcpy(SNES.Header, ext_header, 0x40);

			uint16_t mapper_num = (SNES.Header[0x15] & 0xF);

			if (mapper_num == 0)
			{
				// Lo ROM
				SNES.ReadMemory = &SNES_System::ReadMemory_Lo_ROM;
				SNES.PeekMemory = &SNES_System::PeekMemory_Lo_ROM;
				SNES.WriteMemory = &SNES_System::WriteMemory_Lo_ROM;
			}
			else if (mapper_num == 1)
			{
				// Hi ROM
				SNES.ReadMemory = &SNES_System::ReadMemory_Hi_ROM;
				SNES.PeekMemory = &SNES_System::PeekMemory_Hi_ROM;
				SNES.WriteMemory = &SNES_System::WriteMemory_Hi_ROM;
			}
			else if (mapper_num == 5)
			{
				// Ex Hi ROM
				SNES.ReadMemory = &SNES_System::ReadMemory_Ex_Hi_ROM;
				SNES.PeekMemory = &SNES_System::PeekMemory_Ex_Hi_ROM;
				SNES.WriteMemory = &SNES_System::WriteMemory_Ex_Hi_ROM;
			}

			SNES.Cart_RAM_Length = 0;

			// Only reset cycle count on initial power on, not power cycles
			SNES.Cycle_Count = 0;
			SNES.Clock_Update_Cycle = 0;

			// Only certain carts require ppu clocking
			SNES.ppu_HasClockPPU = false;

			if ((mapper_num == 4) || (mapper_num == 1) || (mapper_num == 5))
			{
				SNES.ppu_HasClockPPU = true;
			}

			SNES.Mapper_Number = mapper_num;

			SNES.Total_CPU_Clock_Cycles = 0;
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			SNES.Cart_RAM = new uint8_t[ext_sram_size];

			SNES.Cart_RAM_Length = ext_sram_size;

			std::memcpy(SNES.Cart_RAM, ext_sram, ext_sram_size);
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
			APU.HardReset();
			
			SNES.HardReset();
		}

		void Soft_Reset()
		{
			APU.SoftReset();

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
			saver = APU.apu_SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = SNES.LoadState(loader);
			loader = APU.apu_LoadState(loader);
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return (SNES.*SNES.PeekMemory)(addr & 0xFFFF);
		}

		uint8_t GetVRAM(uint32_t addr) 
		{

			return SNES.VRAM[(addr & 0xFFFF)];
		}

		uint8_t GetCHR_ROM(uint32_t addr)
		{

			return 0;
		}

		uint8_t GetRAM(uint32_t addr)
		{
			return SNES.RAM[addr & 0x7FF];
		}

		uint8_t GetAPURAM(uint32_t addr)
		{
			return APU.RAM[addr & 0xFFFF];
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
				case 3: return false;
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
			return 0;
		}

		uint8_t* Get_PPU_Pointers(int sel)
		{
			switch (sel)
			{
				case 0: return SNES.CHR_ROM; break;
				case 1: return SNES.OAM; break;
				case 2: return SNES.PALRAM; break;
				case 3: return 0; break;
				case 4: return 0; break;
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
