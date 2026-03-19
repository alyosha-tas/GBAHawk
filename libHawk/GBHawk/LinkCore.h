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
		GBLinkCore(uint32_t num_roms)
		{
			GBL = new GBCore[num_roms];
			Num_ROMs = num_roms;
		};

		GBCore* GBL;
		uint32_t Num_ROMs;

		void Load_BIOS(uint8_t* bios, bool gbcflag, bool gbc_gba_flag, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Load_BIOS(bios, gbcflag, gbc_gba_flag);
			}
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint32_t mapper, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Load_ROM(ext_rom, ext_rom_size, mapper);
			}
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Create_SRAM(ext_sram, ext_sram_size);
			}
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				std::memcpy(GBL[console_num].GB.Cart_RAM, ext_sram, ext_sram_size);
			}
		}

		void Hard_Reset(uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Hard_Reset();
			}
		}

		bool FrameAdvance(uint16_t controller_0, uint16_t accx_0, uint16_t accy_0, bool render_0, bool rendersound_0,
						uint16_t controller_1, uint16_t accx_1, uint16_t accy_1, bool render_1, bool rendersound_1,
						bool l_reset, bool r_reset)
		{
			return false;
		}

		void GetVideo(uint32_t* dest, uint32_t num)
		{
			if (num < Num_ROMs)
			{
				GBL[num].GetVideo(dest);
			}	
		}

		uint32_t GetAudio(int32_t* dest_L_0, int32_t* n_samp_L_0, int32_t* dest_R_0, int32_t* n_samp_R_0,
						  int32_t* dest_L_1, int32_t* n_samp_L_1, int32_t* dest_R_1, int32_t* n_samp_R_1, bool* enables)
		{
			uint32_t counts = 0;
			uint32_t one_en = 5;
			uint32_t two_en = 5;

			uint32_t clock = 0;

			for (int i = 0; i < Num_ROMs; i++)
			{
				if (enables[i])
				{
					counts += 1;

					if ((one_en != 5) && (two_en == 5))
					{
						two_en = (uint32_t)i;
					}
					
					if (one_en == 5)
					{
						one_en = (uint32_t)i;
					}
				}
			}
			
			if (counts == 1)
			{
				// only one system audio enables, send to both left and right channel
				return GBL[one_en].GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0);
			}

			if (counts == 2)
			{
				// send left audio to console 1, right to console 2
				GBL[one_en].GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0);

				return GBL[two_en].GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0);
			}

			return 0;
		}

#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{
			for (int i = 0; i < Num_ROMs; i++)
			{
				saver = GBL[i].SaveState(saver);
			}
		}

		void LoadState(uint8_t* loader)
		{
			for (int i = 0; i < Num_ROMs; i++)
			{
				loader = GBL[i].LoadState(loader);
			}
		}

#pragma endregion

#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetSysBus(addr);
			}
			return 0;
		}

		uint8_t GetVRAM(uint32_t addr, bool vbl_sync, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetVRAM(addr, vbl_sync);
			}
			return 0;
		}

		uint8_t GetRAM(uint32_t addr, bool vbl_sync, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetRAM(addr, vbl_sync);
			}
			return 0;
		}

		uint8_t GetHRAM(uint32_t addr, bool vbl_sync, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetHRAM(addr, vbl_sync);
			}
			return 0;
		}

		uint8_t GetOAM(uint32_t addr, bool vbl_sync, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetOAM(addr, vbl_sync);
			}
			return 0;
		}

		uint8_t GetRegisters(uint32_t addr, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetRegisters(addr);
			}
			return 0;
		}

		uint8_t GetSRAM(uint32_t addr, bool vbl_sync, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				return GBL[console_num].GetSRAM(addr, vbl_sync);
			}

			return 0;
		}

#pragma endregion

#pragma region Tracer

		void SetTraceCallback(void (*callback)(int), uint32_t num)
		{
			if (num < Num_ROMs)
			{
				GBL[num].GB.TraceCallback = callback;
			}

			for (int i = 0; i < Num_ROMs; i++)
			{
				if (i != num)
				{
					GBL[i].GB.TraceCallback = nullptr;
				}
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
			std::memcpy(h, GBL[0].GB.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l, uint32_t num)
		{
			if (num < Num_ROMs)
			{
				GBL[num].GetRegisterState(r, t, l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l, uint32_t num)
		{
			if (num < Num_ROMs)
			{
				GBL[num].GetDisassembly(d, t, l);
			}
		}

#pragma endregion

	};
}

#endif

