#ifndef LINKCORE_H
#define LINKCORE_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "Core.h"

#include "../Common/Savestate.h"

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

			for (int i = 0; i < Num_ROMs; i++)
			{
				GBL[i].Console_ID = (uint32_t)i;
			}
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

		// 4 player adapter variables
		bool is_pinging;
		bool is_transmitting;
		bool time_out_check;
		bool ready_to_transmit;
		bool buffer_parity;
		bool pre_transmit;

		uint8_t status_byte;
		uint8_t received_byte;
		uint8_t temp1_rec, temp2_rec, temp3_rec, temp4_rec;

		uint32_t x4_clock;
		uint32_t ping_player;
		uint32_t ping_byte;
		uint32_t bit_count;
		uint32_t begin_transmitting_cnt;
		uint32_t transmit_speed;
		uint32_t num_bytes_transmit;
		uint32_t transmit_byte;

		uint8_t x4_buffer[0x400 * 2] = { };

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

		void Hard_Reset()
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

			is_pinging = false;
			is_transmitting = false;
			time_out_check = false;
			ready_to_transmit = false;
			buffer_parity = false;
			pre_transmit = false;

			status_byte = 1;
			received_byte = 0;
			temp1_rec = temp2_rec = temp3_rec = temp4_rec = 0;

			x4_clock = 64;
			ping_player = 1;
			ping_byte = 0;
			bit_count = 7;
			begin_transmitting_cnt = 0;
			transmit_speed = 0;
			num_bytes_transmit = 0;
			transmit_byte = 0;

			for (int i = 0; i < 0x800; i++)
			{
				x4_buffer[i] = 0;
			}
			
			for (int i = 0; i < Num_ROMs; i++)
			{
				GBL[i].Hard_Reset();
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
					if (X4_Connected)
					{
						Linking4x();
					}
					else
					{
						Linking2x2();
					}
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

		void Linking2x2()
		{
			if (AC_Connected)
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

				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[2].GB.ser_Clock == 1) || (GBL[2].GB.ser_Clock == 2)) && (GBL[2].GB.ser_Clk_Rate > 0) && !do_Opp_Next_2)
				{
					GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);

					if ((GBL[3].GB.ser_Clk_Rate == -1) && GBL[2].GB.ser_Can_Pulse)
					{
						GBL[3].GB.ser_Clock = GBL[2].GB.ser_Clock;
						GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
						GBL[3].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					}

					GBL[2].GB.ser_Coming_In = GBL[3].GB.ser_Going_Out;
					GBL[2].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[3].GB.ser_Clock == 1) || (GBL[3].GB.ser_Clock == 2)) && (GBL[3].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next_2 = false;

					GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);

					if ((GBL[2].GB.ser_Clk_Rate == -1) && GBL[3].GB.ser_Can_Pulse)
					{
						GBL[2].GB.ser_Clock = GBL[3].GB.ser_Clock;
						GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
						GBL[2].GB.ser_Coming_In = GBL[3].GB.ser_Going_Out;
					}

					GBL[3].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					GBL[3].GB.ser_Can_Pulse = false;

					if (GBL[3].GB.ser_Clock == 2) { do_Opp_Next_2 = true; }
				}
				else
				{
					do_Opp_Next_2 = false;
				}
			}
			else if (AB_Connected)
			{
				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[0].GB.ser_Clock == 1) || (GBL[0].GB.ser_Clock == 2)) && (GBL[0].GB.ser_Clk_Rate > 0) && !do_Opp_Next)
				{
					GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);

					if ((GBL[3].GB.ser_Clk_Rate == -1) && GBL[0].GB.ser_Can_Pulse)
					{
						GBL[3].GB.ser_Clock = GBL[0].GB.ser_Clock;
						GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
						GBL[3].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					}

					GBL[0].GB.ser_Coming_In = GBL[3].GB.ser_Going_Out;
					GBL[0].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[3].GB.ser_Clock == 1) || (GBL[3].GB.ser_Clock == 2)) && (GBL[3].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next = false;

					GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);

					if ((GBL[0].GB.ser_Clk_Rate == -1) && GBL[3].GB.ser_Can_Pulse)
					{
						GBL[0].GB.ser_Clock = GBL[3].GB.ser_Clock;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = GBL[3].GB.ser_Going_Out;
					}

					GBL[3].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					GBL[3].GB.ser_Can_Pulse = false;

					if (GBL[3].GB.ser_Clock == 2) { do_Opp_Next = true; }
				}
				else
				{
					do_Opp_Next = false;
				}

				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[1].GB.ser_Clock == 1) || (GBL[1].GB.ser_Clock == 2)) && (GBL[1].GB.ser_Clk_Rate > 0) && !do_Opp_Next_2)
				{
					GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);

					if ((GBL[2].GB.ser_Clk_Rate == -1) && GBL[1].GB.ser_Can_Pulse)
					{
						GBL[2].GB.ser_Clock = GBL[1].GB.ser_Clock;
						GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
						GBL[2].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					}

					GBL[1].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					GBL[1].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[2].GB.ser_Clock == 1) || (GBL[2].GB.ser_Clock == 2)) && (GBL[2].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next_2 = false;

					GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);

					if ((GBL[1].GB.ser_Clk_Rate == -1) && GBL[2].GB.ser_Can_Pulse)
					{
						GBL[1].GB.ser_Clock = GBL[2].GB.ser_Clock;
						GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
						GBL[1].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					}

					GBL[2].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					GBL[2].GB.ser_Can_Pulse = false;

					if (GBL[2].GB.ser_Clock == 2) { do_Opp_Next_2 = true; }
				}
				else
				{
					do_Opp_Next_2 = false;
				}
			}
			else if (AD_Connected)
			{
				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[2].GB.ser_Clock == 1) || (GBL[2].GB.ser_Clock == 2)) && (GBL[2].GB.ser_Clk_Rate > 0) && !do_Opp_Next)
				{
					GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);

					if ((GBL[0].GB.ser_Clk_Rate == -1) && GBL[2].GB.ser_Can_Pulse)
					{
						GBL[0].GB.ser_Clock = GBL[2].GB.ser_Clock;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					}

					GBL[2].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					GBL[2].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[0].GB.ser_Clock == 1) || (GBL[0].GB.ser_Clock == 2)) && (GBL[0].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next = false;

					GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);

					if ((GBL[2].GB.ser_Clk_Rate == -1) && GBL[0].GB.ser_Can_Pulse)
					{
						GBL[2].GB.ser_Clock = GBL[0].GB.ser_Clock;
						GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
						GBL[2].GB.ser_Coming_In = GBL[0].GB.ser_Going_Out;
					}

					GBL[0].GB.ser_Coming_In = GBL[2].GB.ser_Going_Out;
					GBL[0].GB.ser_Can_Pulse = false;

					if (GBL[0].GB.ser_Clock == 2) { do_Opp_Next = true; }
				}
				else
				{
					do_Opp_Next = false;
				}

				// the signal to shift out a bit is when serial_clock = 1
				if (((GBL[1].GB.ser_Clock == 1) || (GBL[1].GB.ser_Clock == 2)) && (GBL[1].GB.ser_Clk_Rate > 0) && !do_Opp_Next_2)
				{
					GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);

					if ((GBL[3].GB.ser_Clk_Rate == -1) && GBL[1].GB.ser_Can_Pulse)
					{
						GBL[3].GB.ser_Clock = GBL[1].GB.ser_Clock;
						GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
						GBL[3].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					}

					GBL[1].GB.ser_Coming_In = GBL[3].GB.ser_Going_Out;
					GBL[1].GB.ser_Can_Pulse = false;
				}
				else if (((GBL[3].GB.ser_Clock == 1) || (GBL[3].GB.ser_Clock == 2)) && (GBL[3].GB.ser_Clk_Rate > 0))
				{
					do_Opp_Next_2 = false;

					GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);

					if ((GBL[1].GB.ser_Clk_Rate == -1) && GBL[3].GB.ser_Can_Pulse)
					{
						GBL[1].GB.ser_Clock = GBL[3].GB.ser_Clock;
						GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
						GBL[1].GB.ser_Coming_In = GBL[3].GB.ser_Going_Out;
					}

					GBL[3].GB.ser_Coming_In = GBL[1].GB.ser_Going_Out;
					GBL[3].GB.ser_Can_Pulse = false;

					if (GBL[3].GB.ser_Clock == 2) { do_Opp_Next_2 = true; }
				}
				else
				{
					do_Opp_Next_2 = false;
				}
			}
		}

		void Linking4x()
		{
			x4_clock--;

			if (x4_clock == 0)
			{
				if (is_transmitting)
				{
					if (ready_to_transmit)
					{
						// fill the buffer on the second pass
						GBL[0].GB.ser_Clock = 1;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = (uint8_t)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);
						temp1_rec = (uint8_t)((temp1_rec << 1) | GBL[0].GB.ser_Going_Out);

						if ((status_byte & 0x20) == 0x20)
						{
							GBL[1].GB.ser_Clock = 1;
							GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
							GBL[1].GB.ser_Coming_In = (uint8_t)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);

							temp2_rec = (uint8_t)((temp2_rec << 1) | GBL[1].GB.ser_Going_Out);
						}
						else
						{
							temp2_rec = (uint8_t)((temp2_rec << 1) | 0);
						}

						if ((status_byte & 0x40) == 0x40)
						{
							GBL[2].GB.ser_Clock = 1;
							GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
							GBL[2].GB.ser_Coming_In = (uint8_t)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);

							temp3_rec = (uint8_t)((temp3_rec << 1) | GBL[2].GB.ser_Going_Out);
						}
						else
						{
							temp3_rec = (uint8_t)((temp3_rec << 1) | 0);
						}

						if ((status_byte & 0x80) == 0x80)
						{
							GBL[3].GB.ser_Clock = 1;
							GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
							GBL[3].GB.ser_Coming_In = (uint8_t)((x4_buffer[transmit_byte + (buffer_parity ? 0x400 : 0)] >> bit_count) & 1);

							temp4_rec = (uint8_t)((temp4_rec << 1) | GBL[3].GB.ser_Going_Out);
						}
						else
						{
							temp4_rec = (uint8_t)((temp4_rec << 1) | 0);
						}

						bit_count--;
						x4_clock = 512 + transmit_speed * 8;

						if (bit_count == -1)
						{
							bit_count = 7;
							x4_clock = 64;
							ready_to_transmit = false;

							if ((transmit_byte >= 1) && (transmit_byte < (num_bytes_transmit + 1)))
							{
								x4_buffer[(buffer_parity ? 0 : 0x400) + (transmit_byte - 1)] = temp1_rec;
								x4_buffer[(buffer_parity ? 0 : 0x400) + num_bytes_transmit + (transmit_byte - 1)] = temp2_rec;
								x4_buffer[(buffer_parity ? 0 : 0x400) + num_bytes_transmit * 2 + (transmit_byte - 1)] = temp3_rec;
								x4_buffer[(buffer_parity ? 0 : 0x400) + num_bytes_transmit * 3 + (transmit_byte - 1)] = temp4_rec;
							}

							//Console.WriteLine(temp1_rec + " " + temp2_rec + " " + temp3_rec + " " + temp4_rec + " " + transmit_byte);

							transmit_byte++;

							if (transmit_byte == num_bytes_transmit * 4)
							{
								transmit_byte = 0;
								buffer_parity = !buffer_parity;
							}
						}
					}
					else
					{
						if ((GBL[0].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[0].GB.ser_Control, 7))
						{
							ready_to_transmit = true;

							if ((status_byte & 0x20) == 0x20)
							{
								if (!((GBL[1].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[1].GB.ser_Control, 7))) { ready_to_transmit = false; }
							}
							if ((status_byte & 0x40) == 0x40)
							{
								if (!((GBL[2].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[2].GB.ser_Control, 7))) { ready_to_transmit = false; }
							}
							if ((status_byte & 0x80) == 0x80)
							{
								if (!((GBL[3].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[3].GB.ser_Control, 7))) { ready_to_transmit = false; }
							}
						}

						if (ready_to_transmit)
						{
							x4_clock = 512 + transmit_speed * 8;
						}
						else
						{
							x4_clock = 64;
						}
					}
				}
				else if (is_pinging)
				{
					if (ping_byte == 0)
					{
						// first byte sent is 0xFE
						if (ping_player == 1)
						{
							if ((GBL[0].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[0].GB.ser_Control, 7))
							{
								GBL[0].GB.ser_Clock = 1;
								GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
								GBL[0].GB.ser_Coming_In = (uint8_t)((0xFE >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[0].GB.ser_Going_Out << bit_count);
						}
						else if (ping_player == 2)
						{
							if ((GBL[1].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[1].GB.ser_Control, 7))
							{
								GBL[1].GB.ser_Clock = 1;
								GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
								GBL[1].GB.ser_Coming_In = (uint8_t)((0xFE >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[1].GB.ser_Going_Out << bit_count);
						}
						else if (ping_player == 3)
						{
							if ((GBL[2].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[2].GB.ser_Control, 7))
							{
								GBL[2].GB.ser_Clock = 1;
								GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
								GBL[2].GB.ser_Coming_In = (uint8_t)((0xFE >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[2].GB.ser_Going_Out << bit_count);
						}
						else
						{
							if ((GBL[3].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[3].GB.ser_Control, 7))
							{
								GBL[3].GB.ser_Clock = 1;
								GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
								GBL[3].GB.ser_Coming_In = (uint8_t)((0xFE >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[3].GB.ser_Going_Out << bit_count);
						}

						bit_count--;
						x4_clock = 512;

						if (bit_count == -1)
						{
							// player one can start the transmission phase
							if (ping_player == 1)
							{
								begin_transmitting_cnt = 0;
								num_bytes_transmit = received_byte;
							}

							//Console.WriteLine(ping_player + " " + ping_byte + " " + status_byte + " " + received_byte);

							bit_count = 7;
							received_byte = 0;

							ping_byte++;
							x4_clock = 64;
							is_pinging = false;
						}
					}
					else
					{
						// the next 3 bytes are the status byte (which may be updated in between each transfer)
						if (ping_player == 1)
						{
							if ((GBL[0].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[0].GB.ser_Control, 7))
							{
								GBL[0].GB.ser_Clock = 1;
								GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
								GBL[0].GB.ser_Coming_In = (uint8_t)((status_byte >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[0].GB.ser_Going_Out << bit_count);
						}
						else if (ping_player == 2)
						{
							if ((GBL[1].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[1].GB.ser_Control, 7))
							{
								GBL[1].GB.ser_Clock = 1;
								GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
								GBL[1].GB.ser_Coming_In = (uint8_t)((status_byte >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[1].GB.ser_Going_Out << bit_count);
						}
						else if (ping_player == 3)
						{
							if ((GBL[2].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[2].GB.ser_Control, 7))
							{
								GBL[2].GB.ser_Clock = 1;
								GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
								GBL[2].GB.ser_Coming_In = (uint8_t)((status_byte >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[2].GB.ser_Going_Out << bit_count);
						}
						else
						{
							if ((GBL[3].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[3].GB.ser_Control, 7))
							{
								GBL[3].GB.ser_Clock = 1;
								GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
								GBL[3].GB.ser_Coming_In = (uint8_t)((status_byte >> bit_count) & 1);
							}

							received_byte |= (uint8_t)(GBL[3].GB.ser_Going_Out << bit_count);
						}

						bit_count--;
						x4_clock = 512;

						if (bit_count == -1)
						{
							is_pinging = false;
							x4_clock = 64;

							// player one can start the transmission phase
							if ((received_byte == 0xAA) && (ping_player == 1))
							{
								begin_transmitting_cnt += 1;

								if ((begin_transmitting_cnt >= 1) && (ping_byte == 3))
								{
									pre_transmit = true;
									is_pinging = false;
									ready_to_transmit = false;
									transmit_byte = 0;
									bit_count = 7;
								}
							}

							if (((received_byte & 0x88) == 0x88) && (ping_byte <= 2))
							{
								status_byte |= (uint8_t)(1 << (3 + ping_player));
							}

							if ((ping_player == 1) && (ping_byte == 3) && !pre_transmit)
							{
								transmit_speed = received_byte;
							}

							//Console.WriteLine(ping_player + " " + ping_byte + " " + status_byte + " " + received_byte);

							bit_count = 7;
							received_byte = 0;

							ping_byte++;

							if (ping_byte == 4)
							{
								ping_byte = 0;
								ping_player++;

								if (ping_player == 5) { ping_player = 1; }

								begin_transmitting_cnt = 0;

								status_byte &= 0xF0;
								status_byte |= (uint8_t)ping_player;

								time_out_check = true;
								x4_clock = 64;
							}
						}
					}
				}
				else if (pre_transmit)
				{
					if (ready_to_transmit)
					{
						// send four byte of 0xCC to signal start of transmitting

						// fill the buffer
						GBL[0].GB.ser_Clock = 1;
						GBL[0].GB.ser_Going_Out = (uint8_t)(GBL[0].GB.ser_Data >> 7);
						GBL[0].GB.ser_Coming_In = (uint8_t)((0xCC >> bit_count) & 1);

						if ((status_byte & 0x20) == 0x20)
						{
							GBL[1].GB.ser_Clock = 1;
							GBL[1].GB.ser_Going_Out = (uint8_t)(GBL[1].GB.ser_Data >> 7);
							GBL[1].GB.ser_Coming_In = (uint8_t)((0xCC >> bit_count) & 1);
						}

						if ((status_byte & 0x40) == 0x40)
						{
							GBL[2].GB.ser_Clock = 1;
							GBL[2].GB.ser_Going_Out = (uint8_t)(GBL[2].GB.ser_Data >> 7);
							GBL[2].GB.ser_Coming_In = (uint8_t)((0xCC >> bit_count) & 1);
						}

						if ((status_byte & 0x80) == 0x80)
						{
							GBL[3].GB.ser_Clock = 1;
							GBL[3].GB.ser_Going_Out = (uint8_t)(GBL[3].GB.ser_Data >> 7);
							GBL[3].GB.ser_Coming_In = (uint8_t)((0xCC >> bit_count) & 1);
						}

						bit_count--;
						x4_clock = 512;

						if (bit_count == -1)
						{
							bit_count = 7;
							x4_clock = 64;
							ready_to_transmit = false;

							transmit_byte++;

							if (transmit_byte == 4)
							{
								pre_transmit = false;
								is_transmitting = true;
								transmit_byte = 0;
								buffer_parity = false;
							}
						}
					}
					else
					{
						if ((GBL[0].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[0].GB.ser_Control, 7))
						{
							ready_to_transmit = true;

							if ((status_byte & 0x20) == 0x20)
							{
								if (!((GBL[1].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[1].GB.ser_Control, 7))) { ready_to_transmit = false; }
							}
							if ((status_byte & 0x40) == 0x40)
							{
								if (!((GBL[2].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[2].GB.ser_Control, 7))) { ready_to_transmit = false; }
							}
							if ((status_byte & 0x80) == 0x80)
							{
								if (!((GBL[3].GB.ser_Clk_Rate == -1) && Get_Bit(GBL[3].GB.ser_Control, 7))) { ready_to_transmit = false; }
							}
						}

						if (ready_to_transmit)
						{
							x4_clock = 512;
						}
						else
						{
							x4_clock = 64;
						}
					}
				}
				else
				{
					x4_clock = 64;

					// wiat for a gameboy to request a ping. Timeout and go to the next one if nothing happening for some time.
					if ((ping_player == 1) && ((GBL[0].GB.ser_Control & 0x81) == 0x80))
					{
						is_pinging = true;
						x4_clock = 512;
						time_out_check = false;
					}
					else if ((ping_player == 2) && ((GBL[1].GB.ser_Control & 0x81) == 0x80))
					{
						is_pinging = true;
						x4_clock = 512;
						time_out_check = false;
					}
					else if ((ping_player == 3) && ((GBL[2].GB.ser_Control & 0x81) == 0x80))
					{
						is_pinging = true;
						x4_clock = 512;
						time_out_check = false;
					}
					else if ((ping_player == 4) && ((GBL[3].GB.ser_Control & 0x81) == 0x80))
					{
						is_pinging = true;
						x4_clock = 512;
						time_out_check = false;
					}

					if (time_out_check)
					{
						ping_player++;

						if (ping_player == 5) { ping_player = 1; }

						status_byte &= 0xF0;
						status_byte |= (uint8_t)ping_player;

						x4_clock = 64;
					}
				}
			}
		}

		inline bool Get_Bit(uint8_t val, uint8_t bit) { return (val & (0x1 << bit)) == (0x1 << bit); }

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

				is_pinging = false;
				is_transmitting = false;
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

			saver = bool_saver(is_pinging, saver);
			saver = bool_saver(is_transmitting, saver);
			saver = bool_saver(time_out_check, saver);
			saver = bool_saver(ready_to_transmit, saver);
			saver = bool_saver(buffer_parity, saver);
			saver = bool_saver(pre_transmit, saver);

			saver = byte_saver(status_byte, saver);
			saver = byte_saver(received_byte, saver);
			saver = byte_saver(temp1_rec, saver);
			saver = byte_saver(temp2_rec, saver);
			saver = byte_saver(temp3_rec, saver);
			saver = byte_saver(temp4_rec, saver);

			saver = int_saver(x4_clock, saver);
			saver = int_saver(ping_player, saver);
			saver = int_saver(ping_byte, saver);
			saver = int_saver(bit_count, saver);
			saver = int_saver(begin_transmitting_cnt, saver);
			saver = int_saver(transmit_speed, saver);
			saver = int_saver(num_bytes_transmit, saver);
			saver = int_saver(transmit_byte, saver);

			saver = byte_array_saver(x4_buffer, saver, 0x800);

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

			loader = bool_loader(&is_pinging, loader);
			loader = bool_loader(&is_transmitting, loader);
			loader = bool_loader(&time_out_check, loader);
			loader = bool_loader(&ready_to_transmit, loader);
			loader = bool_loader(&buffer_parity, loader);
			loader = bool_loader(&pre_transmit, loader);

			loader = byte_loader(&status_byte, loader);
			loader = byte_loader(&received_byte, loader);
			loader = byte_loader(&temp1_rec, loader);
			loader = byte_loader(&temp2_rec, loader);
			loader = byte_loader(&temp3_rec, loader);
			loader = byte_loader(&temp4_rec, loader);

			loader = int_loader(&x4_clock, loader);
			loader = int_loader(&ping_player, loader);
			loader = int_loader(&ping_byte, loader);
			loader = int_loader(&bit_count, loader);
			loader = int_loader(&begin_transmitting_cnt, loader);
			loader = int_loader(&transmit_speed, loader);
			loader = int_loader(&num_bytes_transmit, loader);
			loader = int_loader(&transmit_byte, loader);

			loader = byte_array_loader(x4_buffer, loader, 0x800);

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

			for (int i = 0; i < Num_ROMs; i++)
			{
				GBL[i].SetMessageCallback(callback);
			}
		}

		void GetMessage(char* d)
		{
			std::memcpy(d, Message_String.c_str(), Message_String.length() + 1);
		}

	};
}

#endif

