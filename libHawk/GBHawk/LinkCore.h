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
			MessageCallback = nullptr;
		};

		GBCore* GBL;
		uint32_t Num_ROMs;

		string Message_String = "";

		void (*MessageCallback)(int);

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

		void Set_Palette(bool palette, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Set_Palette(palette);
			}
		}

		void Sync_Domain_VBL(bool on_vbl, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Sync_Domain_VBL(on_vbl);
			}
		}

		void Hard_Reset(uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Hard_Reset();
			}
		}

		bool FrameAdvance(uint8_t* ctrls, uint16_t* accxs, uint16_t* accys, bool* renders, bool* sounds, bool* resets)
		{
			for (int i = 0; i < Num_ROMs; i++)
			{
				GBL[i].GB.New_Controller = ctrls[i];
				GBL[i].GB.New_Acc_X = accxs[i];
				GBL[i].GB.New_Acc_Y = accys[i];

				GBL[i].GB.snd_Master_Clock = 0;

				GBL[i].GB.num_samples_L = 0;
				GBL[i].GB.num_samples_R = 0;

				GBL[i].GB.Is_Lag = true;

				GBL[i].GB.VBlank_Rise = false;

				if (resets[i]) { GBL[i].Hard_Reset(); }

				GBL[i].GB.Frame_Cycle = 0;
			}

			// do a full frame of cycles for all consoles
			for (int i = 0; i < 70224; i++)
			{
				for (int i = 0; i < Num_ROMs; i++)
				{
					GBL[i].GB.Single_Step();
					GBL[i].GB.Frame_Cycle += 1;
				}

				if (Num_ROMs == 2)
				{
					Linking2x();
				}
				else if (Num_ROMs == 3)
				{
					Linking3x();
				}
				else
				{
					Linking4x();
				}
			}
			
			return false;
		}

		void Linking2x()
		{

		}

		void Linking3x()
		{

		}

		void Linking4x()
		{

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

		void SetMessageCallback(void (*callback)(int))
		{
			MessageCallback = callback;
		}

		void GetMessage(char* d)
		{
			std::memcpy(d, Message_String.c_str(), Message_String.length() + 1);
		}

	};
}

#endif

