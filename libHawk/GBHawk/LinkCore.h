#ifndef LINKCORE_H
#define LINKCORE_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "Core.h"

using namespace std;

namespace GBHawk
{
	class GBLinkCore
	{
	public:
		GBLinkCore()
		{
			L.GB.System_Reset();
			L.Mapper = nullptr;

			R.GB.System_Reset();
			R.Mapper = nullptr;
		};

		GBCore L;
		GBCore R;

		void Load_BIOS(uint8_t* bios)
		{
			std::memcpy(L.GB.BIOS, bios, 0x4000);
			std::memcpy(R.GB.BIOS, bios, 0x4000);
		}

		void Load_ROM(uint8_t* ext_rom_0, uint32_t ext_rom_size_0, uint32_t mapper_0,
						uint8_t* ext_rom_1, uint32_t ext_rom_size_1, uint32_t mapper_1)
		{
			L.Load_ROM(ext_rom_0, ext_rom_size_0, mapper_0);
			R.Load_ROM(ext_rom_1, ext_rom_size_1, mapper_1);
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size, uint32_t num)
		{
			if (num == 0)
			{
				L.Create_SRAM(ext_sram, ext_sram_size);
			}
			else
			{
				R.Create_SRAM(ext_sram, ext_sram_size);
			}
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size, uint32_t num)
		{
			if (num == 0)
			{
				std::memcpy(L.GB.Cart_RAM, ext_sram, ext_sram_size);
			}
			else
			{
				std::memcpy(R.GB.Cart_RAM, ext_sram, ext_sram_size);
			}	
		}

		void Hard_Reset()
		{
			L.Mapper->Reset();
			R.Mapper->Reset();

			L.GB.System_Reset();
			R.GB.System_Reset();

			// system starts connected
			L.GB.ext_num = 1;
			R.GB.ext_num = 2;

			L.GB.is_linked_system = true;
			R.GB.is_linked_system = true;
		}

		bool FrameAdvance(uint16_t controller_0, uint16_t accx_0, uint16_t accy_0, bool render_0, bool rendersound_0,
						uint16_t controller_1, uint16_t accx_1, uint16_t accy_1, bool render_1, bool rendersound_1,
						bool l_reset, bool r_reset)
		{
			return false;
		}

		void GetVideo(uint32_t* dest, uint32_t num)
		{
			if (num == 0)
			{
				uint32_t* src = L.GB.video_buffer;
				uint32_t* dst = dest;

				std::memcpy(dst, src, sizeof(uint32_t) * 160 * 144);

				// blank the screen
				for (int i = 0; i < 160 * 144; i++)
				{
					L.GB.video_buffer[i] = 0xFFF8F8F8;
				}
			}
			else
			{
				uint32_t* src = R.GB.video_buffer;
				uint32_t* dst = dest;

				std::memcpy(dst, src, sizeof(uint32_t) * 160 * 144);

				// blank the screen
				for (int i = 0; i < 160 * 144; i++)
				{
					R.GB.video_buffer[i] = 0xFFF8F8F8;
				}
			}		
		}

		uint32_t GetAudio(int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R, uint32_t num)
		{
			if (num == 0)
			{
				int32_t* src = L.GB.samples_L;
				int32_t* dst = dest_L;

				std::memcpy(dst, src, sizeof int32_t * L.GB.num_samples_L * 2);
				n_samp_L[0] = L.GB.num_samples_L;

				src = L.GB.samples_R;
				dst = dest_R;

				std::memcpy(dst, src, sizeof int32_t * L.GB.num_samples_R * 2);
				n_samp_R[0] = L.GB.num_samples_R;

				uint32_t temp_int = L.GB.snd_Master_Clock;

				return temp_int;
			}
			else if (num == 1)
			{
				int32_t* src = R.GB.samples_L;
				int32_t* dst = dest_L;

				std::memcpy(dst, src, sizeof int32_t * R.GB.num_samples_L * 2);
				n_samp_L[0] = R.GB.num_samples_L;

				src = R.GB.samples_R;
				dst = dest_R;

				std::memcpy(dst, src, sizeof int32_t * R.GB.num_samples_R * 2);
				n_samp_R[0] = R.GB.num_samples_R;

				uint32_t temp_int = R.GB.snd_Master_Clock;

				return temp_int;
			}
			else
			{
				int32_t* src = L.GB.samples_L;
				int32_t* dst = dest_L;

				std::memcpy(dst, src, sizeof int32_t * L.GB.num_samples_L * 2);
				n_samp_L[0] = L.GB.num_samples_L;

				src = R.GB.samples_R;
				dst = dest_R;

				std::memcpy(dst, src, sizeof int32_t * R.GB.num_samples_R * 2);
				n_samp_R[0] = R.GB.num_samples_R;

				uint32_t temp_int = (R.GB.snd_Master_Clock > L.GB.snd_Master_Clock) ? R.GB.snd_Master_Clock : L.GB.snd_Master_Clock;

				return temp_int;
			}
		}

#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{
			saver = L.GB.SaveState(saver);
			saver = L.Mapper->SaveState(saver);

			saver = R.GB.SaveState(saver);
			saver = R.Mapper->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = L.GB.LoadState(loader);
			loader = L.Mapper->LoadState(loader);

			loader = R.GB.LoadState(loader);
			loader = R.Mapper->LoadState(loader);
		}

		void SetRumbleCallback(void (*callback)(bool), uint32_t num)
		{
			if (num == 0)
			{
				L.GB.RumbleCallback = callback;
			}
			else
			{
				R.GB.RumbleCallback = callback;
			}
		}

#pragma endregion

#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GB.Peek_Memory(addr);
			}
			else
			{
				return R.GB.Peek_Memory(addr);
			}
		}

		uint8_t GetVRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GB.VRAM[(addr & 0x3FFF)];
			}
			else
			{
				return R.GB.VRAM[(addr & 0x3FFF)];
			}			
		}

		uint8_t GetRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GB.RAM[addr & 0x7FFF];
			}
			else
			{
				return R.GB.RAM[addr & 0x7FFF];
			}
		}

		uint8_t GetHRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GB.ZP_RAM[addr & 0x7F];
			}
			else
			{
				return R.GB.ZP_RAM[addr & 0x7F];
			}
		}

		uint8_t GetOAM(uint32_t addr, uint32_t num)
		{
			if (addr < 0xA0)
			{
				if (num == 0)
				{
					return L.GB.OAM[addr];
				}
				else
				{
					return R.GB.OAM[addr];
				}
			}

			return 0;
		}

		uint8_t GetSRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				if (L.GB.Cart_RAM_Length != 0)
				{
					return L.GB.Cart_RAM[addr & (L.GB.Cart_RAM_Length - 1)];
				}

				return 0;
			}
			else
			{
				if (R.GB.Cart_RAM_Length != 0)
				{
					return R.GB.Cart_RAM[addr & (R.GB.Cart_RAM_Length - 1)];
				}

				return 0;
			}
		}

#pragma endregion

#pragma region Tracer

		void SetTraceCallback(void (*callback)(int), uint32_t num)
		{
			if (num == 0)
			{
				L.GB.TraceCallback = callback;
				R.GB.TraceCallback = nullptr;
			}
			else
			{
				R.GB.TraceCallback = callback;
				L.GB.TraceCallback = nullptr;
			}		
		}

		int GetHeaderLength()
		{
			return 105 + 1;
		}

		int GetDisasmLength()
		{
			return 42 + 1;
		}

		int GetRegStringLength()
		{
			return 103 + 1;
		}

		void GetHeader(char* h, int l)
		{
			std::memcpy(h, L.GB.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l, uint32_t num)
		{
			if (num == 0)
			{
				if (t == 0)
				{
					std::memcpy(r, L.GB.CPURegisterState().c_str(), l);
				}
				else
				{
					std::memcpy(r, L.GB.No_Reg, l);
				}
			}
			else
			{
				if (t == 0)
				{
					std::memcpy(r, R.GB.CPURegisterState().c_str(), l);
				}
				else
				{
					std::memcpy(r, R.GB.No_Reg, l);
				}
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l, uint32_t num)
		{
			if (num == 0)
			{
				if (t == 0)
				{
					std::memcpy(d, L.GB.CPUDisassembly().c_str(), l);
				}
				else if (t == 1)
				{
					std::memcpy(d, L.GB.UNS_event, l);
				}
				else if (t == 2)
				{
					std::memcpy(d, L.GB.UNH_event, l);
				}
				else if (t == 3)
				{
					std::memcpy(d, L.GB.IRQ_event, l);
				}
				else if (t == 4)
				{
					std::memcpy(d, L.GB.HALT_event, l);
				}
				else if (t == 5)
				{
					std::memcpy(d, L.GB.DMA_event, l);
				}
			}
			else
			{
				if (t == 0)
				{
					std::memcpy(d, R.GB.CPUDisassembly().c_str(), l);
				}
				else if (t == 1)
				{
					std::memcpy(d, R.GB.UNS_event, l);
				}
				else if (t == 2)
				{
					std::memcpy(d, R.GB.UNH_event, l);
				}
				else if (t == 3)
				{
					std::memcpy(d, R.GB.IRQ_event, l);
				}
				else if (t == 4)
				{
					std::memcpy(d, R.GB.HALT_event, l);
				}
				else if (t == 5)
				{
					std::memcpy(d, R.GB.DMA_event, l);
				}
			}	
		}

#pragma endregion

	};
}

#endif

