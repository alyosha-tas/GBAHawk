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

		bool AB_Connected;
		bool BC_Connected;
		bool CD_Connected;
		bool AD_Connected;
		bool AC_Connected;
		bool BD_Connected;
		bool X4_Connected;
		bool do_Opp_Next;
		bool do_Opp_Next_2;

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

		void Set_RTC(int32_t val, uint32_t param, uint32_t console_num)
		{
			if (console_num < Num_ROMs)
			{
				GBL[console_num].Set_RTC(val, param);
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
			AB_Connected = false;
			BC_Connected = false;
			CD_Connected = false;
			AD_Connected = false;
			AC_Connected = false;
			BD_Connected = false;
			X4_Connected = false;

			do_Opp_Next = false;
			do_Opp_Next_2 = false;
			
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

			for (int i = 0; i < Num_ROMs; i++)
			{
				if (GBL[i].GB.ppu_pntr->clear_screen)
				{
					GBL[i].GB.clear_screen_func();
				}
			}

			return false;
		}

		void Linking2x()
		{
			if (AB_Connected)
			{
				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[0].GB.ser_Clock == 1) || (GBL[0].GB.ser_Clock == 2)) && (GBL[0].GB.ser_Clk_Rate > 0) && !do_Opp_Next)
				{
					GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);

					if ((GBL[1].GB.ser_Clk_Rate == -1) && GBL[0].GB.ser_Can_Pulse)
					{
						GBL[1].GB.ser_Clock = GBL[0].GB.ser_Clock;
						GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
						GBL[1].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					}

					GBL[0].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					GBL[0].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[1].GB.ser_Clock == 1) || (GBL[1].GB.ser_Clock == 2)) && (GBL[1].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next = false;

					GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);

					if ((GBL[0].GB.ser_Clk_Rate == -1) && GBL[1].GB.ser_Can_Pulse)
					{
						GBL[0].GB.ser_Clock = GBL[1].GB.ser_Clock;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					}

					GBL[1].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					GBL[1].GB.ser_Can_Pulse = false;

					if (GBL[1].GB.ser_Clock == 2) { do_Opp_Next = true; }
				}
				else
				{
					do_Opp_Next = false;
				}

				// do IR transfer
				if (GBL[0].GB.IR_write > 0)
				{
					GBL[0].GB.IR_write--;
					if (GBL[0].GB.IR_write == 0)
					{
						GBL[1].GB.IR_receive = GBL[0].GB.IR_signal;
						if ((GBL[1].GB.IR_self & GBL[1].GB.IR_receive) == 2) { GBL[1].GB.IR_reg |= 2; }
						else { GBL[1].GB.IR_reg &= 0xFD; }
						if ((GBL[0].GB.IR_self & GBL[0].GB.IR_receive) == 2) { GBL[0].GB.IR_reg |= 2; }
						else { GBL[0].GB.IR_reg &= 0xFD; }
					}
				}

				if (GBL[1].GB.IR_write > 0)
				{
					GBL[1].GB.IR_write--;
					if (GBL[1].GB.IR_write == 0)
					{
						GBL[0].GB.IR_receive = GBL[1].GB.IR_signal;
						if ((GBL[0].GB.IR_self & GBL[0].GB.IR_receive) == 2) { GBL[0].GB.IR_reg |= 2; }
						else { GBL[0].GB.IR_reg &= 0xFD; }
						if ((GBL[1].GB.IR_self & GBL[1].GB.IR_receive) == 2) { GBL[1].GB.IR_reg |= 2; }
						else { GBL[1].GB.IR_reg &= 0xFD; }
					}
				}
			}
		}

		void Linking3x()
		{
			if (AC_Connected)
			{
				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[0].GB.ser_Clock == 1) || (GBL[0].GB.ser_Clock == 2)) && (GBL[0].GB.ser_Clk_Rate > 0) && !do_Opp_Next)
				{
					GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);

					if ((GBL[2].GB.ser_Clk_Rate == -1) && GBL[0].GB.ser_Can_Pulse)
					{
						GBL[2].GB.ser_Clock = GBL[0].GB.ser_Clock;
						GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
						GBL[2].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					}

					GBL[0].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					GBL[0].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[2].GB.ser_Clock == 1) || (GBL[2].GB.ser_Clock == 2)) && (GBL[2].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next = false;

					GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);

					if ((GBL[0].GB.ser_Clk_Rate == -1) && GBL[2].GB.ser_Can_Pulse)
					{
						GBL[0].GB.ser_Clock = GBL[2].GB.ser_Clock;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					}

					GBL[2].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					GBL[2].GB.ser_Can_Pulse = false;

					if (GBL[2].GB.ser_Clock == 2) { do_Opp_Next = true; }
				}
				else
				{
					do_Opp_Next = false;
				}

				// do IR transfer
				if (GBL[0].GB.IR_write > 0)
				{
					GBL[0].GB.IR_write--;
					if (GBL[0].GB.IR_write == 0)
					{
						GBL[2].GB.IR_receive = GBL[0].GB.IR_signal;
						if ((GBL[2].GB.IR_self & GBL[2].GB.IR_receive) == 2) { GBL[2].GB.IR_reg |= 2; }
						else { GBL[2].GB.IR_reg &= 0xFD; }
						if ((GBL[0].GB.IR_self & GBL[0].GB.IR_receive) == 2) { GBL[0].GB.IR_reg |= 2; }
						else { GBL[0].GB.IR_reg &= 0xFD; }
					}
				}

				if (GBL[2].GB.IR_write > 0)
				{
					GBL[2].GB.IR_write--;
					if (GBL[2].GB.IR_write == 0)
					{
						GBL[0].GB.IR_receive = GBL[2].GB.IR_signal;
						if ((GBL[0].GB.IR_self & GBL[0].GB.IR_receive) == 2) { GBL[0].GB.IR_reg |= 2; }
						else { GBL[0].GB.IR_reg &= 0xFD; }
						if ((GBL[2].GB.IR_self & GBL[2].GB.IR_receive) == 2) { GBL[2].GB.IR_reg |= 2; }
						else { GBL[2].GB.IR_reg &= 0xFD; }
					}
				}
			}
			else if (BC_Connected)
			{
				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[2].GB.ser_Clock == 1) || (GBL[2].GB.ser_Clock == 2)) && (GBL[2].GB.ser_Clk_Rate > 0) && !do_Opp_Next)
				{
					GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);

					if ((GBL[1].GB.ser_Clk_Rate == -1) && GBL[2].GB.ser_Can_Pulse)
					{
						GBL[1].GB.ser_Clock = GBL[2].GB.ser_Clock;
						GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
						GBL[1].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					}

					GBL[2].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					GBL[2].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[1].GB.ser_Clock == 1) || (GBL[1].GB.ser_Clock == 2)) && (GBL[1].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next = false;

					GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);

					if ((GBL[2].GB.ser_Clk_Rate == -1) && GBL[1].GB.ser_Can_Pulse)
					{
						GBL[2].GB.ser_Clock = GBL[1].GB.ser_Clock;
						GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
						GBL[2].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					}

					GBL[1].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					GBL[1].GB.ser_Can_Pulse = false;

					if (GBL[1].GB.ser_Clock == 2) { do_Opp_Next = true; }
				}
				else
				{
					do_Opp_Next = false;
				}

				// do IR transfer
				if (GBL[2].GB.IR_write > 0)
				{
					GBL[2].GB.IR_write--;
					if (GBL[2].GB.IR_write == 0)
					{
						GBL[1].GB.IR_receive = GBL[2].GB.IR_signal;
						if ((GBL[1].GB.IR_self & GBL[1].GB.IR_receive) == 2) { GBL[1].GB.IR_reg |= 2; }
						else { GBL[1].GB.IR_reg &= 0xFD; }
						if ((GBL[2].GB.IR_self & GBL[2].GB.IR_receive) == 2) { GBL[2].GB.IR_reg |= 2; }
						else { GBL[2].GB.IR_reg &= 0xFD; }
					}
				}

				if (GBL[1].GB.IR_write > 0)
				{
					GBL[1].GB.IR_write--;
					if (GBL[1].GB.IR_write == 0)
					{
						GBL[2].GB.IR_receive = GBL[1].GB.IR_signal;
						if ((GBL[2].GB.IR_self & GBL[2].GB.IR_receive) == 2) { GBL[2].GB.IR_reg |= 2; }
						else { GBL[2].GB.IR_reg &= 0xFD; }
						if ((GBL[1].GB.IR_self & GBL[1].GB.IR_receive) == 2) { GBL[1].GB.IR_reg |= 2; }
						else { GBL[1].GB.IR_reg &= 0xFD; }
					}
				}
			}
			else if (AB_Connected)
			{
				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[1].GB.ser_Clock == 1) || (GBL[1].GB.ser_Clock == 2)) && (GBL[1].GB.ser_Clk_Rate > 0) && !do_Opp_Next)
				{
					GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);

					if ((GBL[0].GB.ser_Clk_Rate == -1) && GBL[1].GB.ser_Can_Pulse)
					{
						GBL[0].GB.ser_Clock = GBL[1].GB.ser_Clock;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					}

					GBL[1].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					GBL[1].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[0].GB.ser_Clock == 1) || (GBL[0].GB.ser_Clock == 2)) && (GBL[0].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next = false;

					GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);

					if ((GBL[1].GB.ser_Clk_Rate == -1) && GBL[0].GB.ser_Can_Pulse)
					{
						GBL[1].GB.ser_Clock = GBL[0].GB.ser_Clock;
						GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
						GBL[1].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					}

					GBL[0].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					GBL[0].GB.ser_Can_Pulse = false;

					if (GBL[0].GB.ser_Clock == 2) { do_Opp_Next = true; }
				}
				else
				{
					do_Opp_Next = false;
				}

				// do IR transfer
				if (GBL[1].GB.IR_write > 0)
				{
					GBL[1].GB.IR_write--;
					if (GBL[1].GB.IR_write == 0)
					{
						GBL[0].GB.IR_receive = GBL[1].GB.IR_signal;
						if ((GBL[0].GB.IR_self & GBL[0].GB.IR_receive) == 2) { GBL[0].GB.IR_reg |= 2; }
						else { GBL[0].GB.IR_reg &= 0xFD; }
						if ((GBL[1].GB.IR_self & GBL[1].GB.IR_receive) == 2) { GBL[1].GB.IR_reg |= 2; }
						else { GBL[1].GB.IR_reg &= 0xFD; }
					}
				}

				if (GBL[0].GB.IR_write > 0)
				{
					GBL[0].GB.IR_write--;
					if (GBL[0].GB.IR_write == 0)
					{
						GBL[1].GB.IR_receive = GBL[0].GB.IR_signal;
						if ((GBL[1].GB.IR_self & GBL[1].GB.IR_receive) == 2) { GBL[1].GB.IR_reg |= 2; }
						else { GBL[1].GB.IR_reg &= 0xFD; }
						if ((GBL[0].GB.IR_self & GBL[0].GB.IR_receive) == 2) { GBL[0].GB.IR_reg |= 2; }
						else { GBL[0].GB.IR_reg &= 0xFD; }
					}
				}
			}
		}

		void Linking4x()
		{

		}

		void Change_Linking(bool link_status, uint32_t link_type)
		{
			if (link_type == 0)
			{
				// 2 player linking
				AB_Connected = link_status;

				if (!AB_Connected) { do_Opp_Next = false; }
			}
			else if (link_type == 1)
			{
				// 3 or 4 player disconnect all connections
				AB_Connected = false;
				BC_Connected = false;
				CD_Connected = false;
				AD_Connected = false;
				AC_Connected = false;
				BD_Connected = false;
				X4_Connected = false;

				do_Opp_Next = false;
				do_Opp_Next_2 = false;
			}
			else if (link_type == 2)
			{
				// 3 player set AB
				AB_Connected = true;
			}
			else if (link_type == 3)
			{
				// 3 player set BC
				BC_Connected = true;
			}
			else if (link_type == 4)
			{
				// 3 player set AC
				AC_Connected = true;
			}
			else if (link_type == 5)
			{
				// 4 player set AB, CD
				AB_Connected = true;
				CD_Connected = true;
			}
			else if (link_type == 6)
			{
				// 4 player set AC, BD
				BD_Connected = true;
				AC_Connected = true;
			}
			else if (link_type == 7)
			{
				// 4 player set AD, BC
				AD_Connected = true;
				BC_Connected = true;
			}
			else if (link_type == 8)
			{
				// 4 player set 4x
				X4_Connected = true;
			}
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
			saver = bool_saver(AB_Connected, saver);
			saver = bool_saver(BC_Connected, saver);
			saver = bool_saver(CD_Connected, saver);
			saver = bool_saver(AD_Connected, saver);
			saver = bool_saver(AC_Connected, saver);
			saver = bool_saver(BD_Connected, saver);
			saver = bool_saver(X4_Connected, saver);

			saver = bool_saver(do_Opp_Next, saver);
			saver = bool_saver(do_Opp_Next_2, saver);

			for (int i = 0; i < Num_ROMs; i++)
			{
				saver = GBL[i].SaveState(saver);
			}
		}

		void LoadState(uint8_t* loader)
		{
			loader = bool_loader(&AB_Connected, loader);
			loader = bool_loader(&BC_Connected, loader);
			loader = bool_loader(&CD_Connected, loader);
			loader = bool_loader(&AD_Connected, loader);
			loader = bool_loader(&AC_Connected, loader);
			loader = bool_loader(&BD_Connected, loader);
			loader = bool_loader(&X4_Connected, loader);

			loader = bool_loader(&do_Opp_Next, loader);
			loader = bool_loader(&do_Opp_Next_2, loader);

			for (int i = 0; i < Num_ROMs; i++)
			{
				loader = GBL[i].LoadState(loader);
			}
		}

		uint8_t* bool_saver(bool to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save ? 1 : 0); saver++;

			return saver;
		}

		uint8_t* bool_loader(bool* to_load, uint8_t* loader)
		{
			to_load[0] = *loader == 1; loader++;

			return loader;
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

