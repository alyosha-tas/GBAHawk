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
#include "Mappers.h"

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
			Mapper = nullptr;
		};

		SNES_System SNES;
		Mappers* Mapper;
		APU APU;

		void Load_IPL(uint8_t* ipl)
		{
			std::memcpy(APU.IPL, ipl, 0x40);
		}

		// NOTE: Front end only sends power of 2 sized ROMs here
		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint8_t* ext_header, uint32_t apu_freq, uint32_t ppu_h_pos, uint32_t ppu_v_pos, uint32_t dram_pos)
		{
			SNES.APU_Frequency = apu_freq;
			SNES.PPU_H_Pos_Reset = ppu_h_pos;		
			SNES.PPU_V_Pos_Reset = ppu_v_pos;
			SNES.DRAM_Refresh_Pos = dram_pos;

			std::memcpy(SNES.Header, ext_header, 0x40);

			uint16_t mapper_num = (SNES.Header[0x15] & 0xF);

			SNES.ROM_Speed = ((SNES.Header[0x15] >> 4) & 1);

			uint32_t rom_divs = 0;

			int k = 0;

			if (mapper_num == 0)
			{
				// Lo ROM
				Mapper = new Mapper_Lo_ROM;

				SNES.ROM_Length = 0x400000;

				SNES.ROM = new uint8_t[SNES.ROM_Length];

				rom_divs = SNES.ROM_Length / ext_rom_size;
				
				for (int i = 0; i < rom_divs; i++)
				{
					for (int j = 0; j < ext_rom_size; j++)
					{
						SNES.ROM[k] = ext_rom[j];
						k++;
					}
				}

				SNES.Is_Lo_ROM = true;
			}
			else if (mapper_num == 1)
			{
				// Hi ROM
				Mapper = new Mapper_Hi_ROM;

				SNES.ROM_Length = 0x400000;

				SNES.ROM = new uint8_t[SNES.ROM_Length];

				rom_divs = SNES.ROM_Length / ext_rom_size;

				for (int i = 0; i < rom_divs; i++)
				{
					for (int j = 0; j < ext_rom_size; j++)
					{
						SNES.ROM[k] = ext_rom[j];
						k++;
					}
				}

				SNES.Is_Lo_ROM = false;
			}
			else if (mapper_num == 5)
			{
				// Ex Hi ROM
				Mapper = new Mapper_Ex_Hi_ROM;

				SNES.ROM_Length = 0x800000;

				SNES.ROM = new uint8_t[SNES.ROM_Length];

				rom_divs = SNES.ROM_Length / ext_rom_size;

				for (int i = 0; i < rom_divs; i++)
				{
					for (int j = 0; j < ext_rom_size; j++)
					{
						SNES.ROM[k] = ext_rom[j];
						k++;
					}
				}

				uint8_t temp_val = 0;

				// upper and lower halves are swapped relative to address space
				for (int i = 0; i < 0x400000; i++)
				{
					temp_val = SNES.ROM[i];
					SNES.ROM[i] = SNES.ROM[i + 0x400000];
					SNES.ROM[i + 0x400000] = temp_val;
				}

				SNES.Is_Lo_ROM = false;
			}

			SNES.mapper_pntr = &Mapper[0];

			Mapper->Core_ROM_Length = ext_rom_size;
			Mapper->Core_ROM = &SNES.ROM[0];
			Mapper->Core_RAM = &SNES.RAM[0];

			Mapper->Core_DB = &SNES.Data_Bus;

			Mapper->Core_Message_String = &SNES.Message_String;
			Mapper->MessageCallback = SNES.MessageCallback;

			Mapper->Reset();

			SNES.Cart_RAM_Length = 0;
			Mapper->Core_Cart_RAM_Length = 0;

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

			Mapper->Core_Cart_RAM_Length = ext_sram_size;

			Mapper->Core_Cart_RAM = &SNES.Cart_RAM[0];

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
			Mapper->Reset();
			
			SNES.HardReset();
		}

		void Soft_Reset()
		{
			APU.SoftReset();
			Mapper->Reset();

			SNES.cpu_SoftReset();
			SNES.ppu_SoftReset();
		}

		bool FrameAdvance(bool render, bool rendersound)
		{
			APU.Audio_Sample_Clock = 0;

			APU.Audio_Num_Samples = 0;

			SNES.Is_Lag = true;

			SNES.Frame_Advance();

			return SNES.Is_Lag;
		}

		bool SubFrameAdvance( bool render, bool rendersound, bool do_reset, uint32_t reset_cycle)
		{
			APU.Audio_Sample_Clock = 0;

			APU.Audio_Num_Samples = 0;

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
			int32_t* src = APU.Audio_Samples;
			int32_t* dst = dest;

			std::memcpy(dst, src, sizeof int32_t * APU.Audio_Num_Samples * 2);
			n_samp[0] = APU.Audio_Num_Samples;

			uint32_t temp_int = APU.Audio_Sample_Clock;
			
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
			return SNES.PeekMemory(addr & 0xFFFFFF);
		}

		uint8_t GetRAM(uint32_t addr)
		{
			return SNES.RAM[addr & 0x1FFFF];
		}

		uint8_t GetVRAM(uint32_t addr) 
		{

			return SNES.VRAM[(addr & 0xFFFF)];
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
			if (addr < 544)
			{
				return SNES.OAM[addr & 0xFF];
			}
			return 0;
		}

		uint8_t GetPALRAM(uint32_t addr)
		{
			return SNES.PALRAM[addr & 0x1FF];
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
