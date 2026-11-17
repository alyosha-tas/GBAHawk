#ifndef LINKCORE_H
#define LINKCORE_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "Core.h"

using namespace std;

namespace GBAHawk
{
	class GBALinkCore
	{
	public:
		GBALinkCore(uint32_t num_roms)
		{
			GBAL = new GBACore[num_roms];
			Num_ROMs = num_roms;
			MessageCallback = nullptr;

			for (int i = 0; i < Num_ROMs; i++)
			{
				GBAL[i].GBA.ext_num = (uint8_t)(i + 1);
				GBAL[i].Mapper = nullptr;
				GBAL[i].GBA.System_Reset();
			}

			Link_Reset();
		};

		GBACore* GBAL;
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

		uint8_t LP1, LP2, LP3, LP4;

		void Link_Reset()
		{
			AB_Connected = false;
			BC_Connected = false;
			CD_Connected = false;
			AD_Connected = false;
			AC_Connected = false;
			BD_Connected = false;
			X4_Connected = false;

			LP1 = LP2 = LP3 = LP4 = 0;
		}

		void Load_BIOS(uint8_t* bios)
		{
			for (int i = 0; i < Num_ROMs; i++)
			{
				std::memcpy(GBAL[i].GBA.BIOS, bios, 0x4000);
			}
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint32_t mapper,
						uint64_t datetime, bool rtc_functional, int16_t EEPROM_offset,
						uint16_t flash_type_64_value, uint16_t flash_type_128_value,
						int16_t flash_write_offset, int16_t flash_sector_offset,
						int16_t flash_chip_offset, bool is_GBP, uint32_t num)
		{
			if (num < Num_ROMs)
			{
				GBAL[num].Load_ROM(ext_rom, ext_rom_size, mapper, datetime, rtc_functional, EEPROM_offset,
					flash_type_64_value, flash_type_128_value, flash_write_offset, flash_sector_offset, flash_chip_offset, is_GBP);
			}
		}

		void Load_Video_ROM(uint8_t* rom, uint32_t num)
		{
			GBAL[num].Load_Video_ROM(rom);
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size, uint32_t num)
		{
			GBAL[num].Create_SRAM(ext_sram, ext_sram_size);
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size, uint32_t num)
		{
			std::memcpy(GBAL[num].GBA.Cart_RAM, ext_sram, ext_sram_size);	
		}

		void Hard_Reset()
		{
			for (int i = 0; i < Num_ROMs; i++)
			{
				GBAL[i].Mapper->Reset();
				GBAL[i].GBA.System_Reset();
				GBAL[i].GBA.is_linked_system = true;
			}

			Link_Reset();

			// system starts in left - right linked state
			// set extrenal pins based on linked state
			if (Num_ROMs < 4)
			{
				GBAL[0].GBA.ext_num = 1;
				GBAL[1].GBA.ext_num = 2;

				// override pointers to serial port pins
				GBAL[0].GBA.Ext_SI = &GBAL[1].GBA.ser_SO;
				GBAL[0].GBA.Ext_SO = &GBAL[1].GBA.ser_SI;
				GBAL[0].GBA.Ext_SC = &GBAL[1].GBA.ser_SC;
				GBAL[0].GBA.Ext_SD = &GBAL[1].GBA.ser_SD;

				GBAL[1].GBA.Ext_SI = &GBAL[0].GBA.ser_SO;
				GBAL[1].GBA.Ext_SO = &GBAL[0].GBA.ser_SI;
				GBAL[1].GBA.Ext_SC = &GBAL[0].GBA.ser_SC;
				GBAL[1].GBA.Ext_SD = &GBAL[0].GBA.ser_SD;

				GBAL[0].GBA.Ext_Multi_Start = &GBAL[1].GBA.ser_Multi_Start;
				GBAL[1].GBA.Ext_Multi_Start = &GBAL[0].GBA.ser_Multi_Start;
			}
			else
			{
				GBAL[0].GBA.ext_num = 1;
				GBAL[1].GBA.ext_num = 2;

				GBAL[2].GBA.ext_num = 1;
				GBAL[3].GBA.ext_num = 2;

				// override pointers to serial port pins
				GBAL[0].GBA.Ext_SI = &GBAL[1].GBA.ser_SO;
				GBAL[0].GBA.Ext_SO = &GBAL[1].GBA.ser_SI;
				GBAL[0].GBA.Ext_SC = &GBAL[1].GBA.ser_SC;
				GBAL[0].GBA.Ext_SD = &GBAL[1].GBA.ser_SD;

				GBAL[1].GBA.Ext_SI = &GBAL[0].GBA.ser_SO;
				GBAL[1].GBA.Ext_SO = &GBAL[0].GBA.ser_SI;
				GBAL[1].GBA.Ext_SC = &GBAL[0].GBA.ser_SC;
				GBAL[1].GBA.Ext_SD = &GBAL[0].GBA.ser_SD;

				GBAL[0].GBA.Ext_Multi_Start = &GBAL[1].GBA.ser_Multi_Start;
				GBAL[1].GBA.Ext_Multi_Start = &GBAL[0].GBA.ser_Multi_Start;

				// override pointers to serial port pins
				GBAL[2].GBA.Ext_SI = &GBAL[3].GBA.ser_SO;
				GBAL[2].GBA.Ext_SO = &GBAL[3].GBA.ser_SI;
				GBAL[2].GBA.Ext_SC = &GBAL[3].GBA.ser_SC;
				GBAL[2].GBA.Ext_SD = &GBAL[3].GBA.ser_SD;

				GBAL[3].GBA.Ext_SI = &GBAL[2].GBA.ser_SO;
				GBAL[3].GBA.Ext_SO = &GBAL[2].GBA.ser_SI;
				GBAL[3].GBA.Ext_SC = &GBAL[2].GBA.ser_SC;
				GBAL[3].GBA.Ext_SD = &GBAL[2].GBA.ser_SD;

				GBAL[2].GBA.Ext_Multi_Start = &GBAL[3].GBA.ser_Multi_Start;
				GBAL[3].GBA.Ext_Multi_Start = &GBAL[2].GBA.ser_Multi_Start;
			}
		}

		void Set_GBP_Enable(int num)
		{
			if (num < Num_ROMs)
			{
				GBAL[num].GBA.GBP_Mode_Enabled = true;
			}
		}

		bool FrameAdvance(uint16_t* controller, uint16_t* accx, uint16_t* accy, uint8_t* solar,
						  bool* render, bool* rendersound, bool* reset)
		{
			bool any_reset = false;
			bool any_lags = true;
			
			for (int i = 0; i < Num_ROMs; i++)
			{
				GBAL[i].GBA.New_Controller = controller[i];
				GBAL[i].GBA.New_Acc_X = accx[i];
				GBAL[i].GBA.New_Acc_Y = accy[i];
				GBAL[i].GBA.New_Solar = solar[i];

				// update the controller state
				GBAL[i].GBA.controller_state_old = GBAL[i].GBA.controller_state;
				GBAL[i].GBA.controller_state = GBAL[i].GBA.New_Controller;

				// as long as not in stop mode, vblank will occur and the controller will be checked
				if (GBAL[i].GBA.VBlank_Rise || GBAL[i].GBA.stopped)
				{
					// check if controller state caused interrupt
					GBAL[i].GBA.do_controller_check(false);
				}

				GBAL[i].GBA.snd_Master_Clock = 0;

				GBAL[i].GBA.num_samples_L = 0;
				GBAL[i].GBA.num_samples_R = 0;

				GBAL[i].GBA.Is_Lag = true;

				GBAL[i].GBA.VBlank_Rise = false;

				any_reset |= reset[i];

				if (reset[i])
				{ 
					GBAL[i].Hard_Reset();	
				}
			}

			// always keep vblanks in sync for resetting cores
			if (any_reset)
			{
				if (Num_ROMs == 2)
				{
					for (int i = 0; i < 80081; i++)
					{
						if (!reset[0]) { GBAL[0].GBA.Single_Step(); }
						if (!reset[1]) { GBAL[1].GBA.Single_Step(); }

						// second of a linked pair will never be 0 (the A console)
						if (LP2 != 0) { Do_Linking_2_3x(); }
					}

				}
				else if (Num_ROMs == 3)
				{
					for (int i = 0; i < 80081; i++)
					{
						if (!reset[0]) { GBAL[0].GBA.Single_Step(); }
						if (!reset[1]) { GBAL[1].GBA.Single_Step(); }
						if (!reset[2]) { GBAL[2].GBA.Single_Step(); }

						// second of a linked pair will never be 0 (the A console)
						if (LP2 != 0) { Do_Linking_2_3x(); }
					}
				}
				else
				{
					for (int i = 0; i < 80081; i++)
					{
						if (!reset[0]) { GBAL[0].GBA.Single_Step(); }
						if (!reset[1]) { GBAL[1].GBA.Single_Step(); }
						if (!reset[2]) { GBAL[2].GBA.Single_Step(); }
						if (!reset[3]) { GBAL[3].GBA.Single_Step(); }

						// second of a linked pair will never be 0 (the A console)
						if (LP2 != 0) { Do_Linking_4x(); }
					}
				}
			}

			// NOTE: the two cores will always be in sync in terms of vblank rise
			if (Num_ROMs == 2)
			{
				while (!GBAL[0].GBA.VBlank_Rise)
				{
					GBAL[0].GBA.Single_Step();
					GBAL[1].GBA.Single_Step();

					// second of a linked pair will never be 0 (the A console)
					if (LP2 != 0) { Do_Linking_2_3x(); }
				}
			}
			else if (Num_ROMs == 3)
			{
				while (!GBAL[0].GBA.VBlank_Rise)
				{
					GBAL[0].GBA.Single_Step();
					GBAL[1].GBA.Single_Step();
					GBAL[2].GBA.Single_Step();

					// second of a linked pair will never be 0 (the A console)
					if (LP2 != 0) { Do_Linking_2_3x(); }
				}
			}
			else
			{
				while (!GBAL[0].GBA.VBlank_Rise)
				{
					GBAL[0].GBA.Single_Step();
					GBAL[1].GBA.Single_Step();
					GBAL[2].GBA.Single_Step();
					GBAL[3].GBA.Single_Step();

					// second of a linked pair will never be 0 (the A console)
					if (LP2 != 0) { Do_Linking_4x(); }
				}
			}

			for (int i = 0; i < Num_ROMs; i++)
			{
				any_lags &= GBAL[i].GBA.Is_Lag;
			}

			return any_lags;
		}

		void Do_Linking_2_3x()
		{
			if (GBAL[LP1].GBA.ser_Ext_Tick)
			{
				if (GBAL[LP1].GBA.ser_Ctrl_Mode_State != 2)
				{
					if (GBAL[LP1].GBA.ser_Bit_Total == 32)
					{
						uint16_t temp_t = GBAL[LP1].GBA.ser_Data_0;
						GBAL[LP1].GBA.ser_Data_0 = GBAL[LP2].GBA.ser_Data_0;
						GBAL[LP2].GBA.ser_Data_0 = temp_t;

						temp_t = GBAL[LP1].GBA.ser_Data_1;
						GBAL[LP1].GBA.ser_Data_1 = GBAL[LP2].GBA.ser_Data_1;
						GBAL[LP2].GBA.ser_Data_1 = temp_t;
					}
					else
					{
						uint16_t temp_t8 = (GBAL[LP1].GBA.ser_Data_M & 0xFF);
						GBAL[LP1].GBA.ser_Data_M &= 0xFF00;
						GBAL[LP1].GBA.ser_Data_M |= GBAL[LP2].GBA.ser_Data_M & 0xFF;
						GBAL[LP2].GBA.ser_Data_M &= 0xFF00;
						GBAL[LP2].GBA.ser_Data_M |= temp_t8;
					}

					GBAL[LP1].GBA.ser_CTRL &= 0xFF7F;
					GBAL[LP2].GBA.ser_CTRL &= 0xFF7F;

					GBAL[LP2].GBA.ser_Start = false;

					// set SO to expected state
					GBAL[LP2].GBA.ser_SO = GBAL[LP2].GBA.ser_SO_Inactive;
					GBAL[LP1].GBA.ser_SO = GBAL[LP1].GBA.ser_SO_Inactive;

					// trigger interrupt if needed
					if ((GBAL[LP2].GBA.ser_CTRL & 0x4000) == 0x4000)
					{
						GBAL[LP2].GBA.Trigger_IRQ(7);
					}

					GBAL[LP1].GBA.ser_Ext_Tick = false;
				}
				else
				{
					GBAL[LP1].GBA.ser_Data_0 = GBAL[LP1].GBA.ser_Data_M;
					GBAL[LP2].GBA.ser_Data_0 = GBAL[LP1].GBA.ser_Data_M;

					GBAL[LP1].GBA.ser_Data_1 = GBAL[LP2].GBA.ser_Data_M;
					GBAL[LP2].GBA.ser_Data_1 = GBAL[LP2].GBA.ser_Data_M;

					GBAL[LP1].GBA.ser_Data_2 = 0xFFFF;
					GBAL[LP2].GBA.ser_Data_2 = 0xFFFF;

					GBAL[LP1].GBA.ser_Data_3 = 0xFFFF;
					GBAL[LP2].GBA.ser_Data_3 = 0xFFFF;

					GBAL[LP1].GBA.ser_CTRL &= 0xFF7F;
					GBAL[LP2].GBA.ser_CTRL &= 0xFF7F;

					GBAL[LP2].GBA.ser_Multi_ID = 1;

					// trigger interrupt if needed
					if ((GBAL[LP2].GBA.ser_CTRL & 0x4000) == 0x4000)
					{
						GBAL[LP2].GBA.Trigger_IRQ(7);
					}

					GBAL[LP1].GBA.ser_Ext_Tick = false;
				}
			}
		}

		void Do_Linking_4x()
		{
			if (GBAL[LP1].GBA.ser_Ext_Tick)
			{
				if (GBAL[LP1].GBA.ser_Ctrl_Mode_State != 2)
				{
					if (GBAL[LP1].GBA.ser_Bit_Total == 32)
					{
						uint16_t temp_t = GBAL[LP1].GBA.ser_Data_0;
						GBAL[LP1].GBA.ser_Data_0 = GBAL[LP2].GBA.ser_Data_0;
						GBAL[LP2].GBA.ser_Data_0 = temp_t;

						temp_t = GBAL[LP1].GBA.ser_Data_1;
						GBAL[LP1].GBA.ser_Data_1 = GBAL[LP2].GBA.ser_Data_1;
						GBAL[LP2].GBA.ser_Data_1 = temp_t;
					}
					else
					{
						uint16_t temp_t8 = (GBAL[LP1].GBA.ser_Data_M & 0xFF);
						GBAL[LP1].GBA.ser_Data_M &= 0xFF00;
						GBAL[LP1].GBA.ser_Data_M |= GBAL[LP2].GBA.ser_Data_M & 0xFF;
						GBAL[LP2].GBA.ser_Data_M &= 0xFF00;
						GBAL[LP2].GBA.ser_Data_M |= temp_t8;
					}

					GBAL[LP1].GBA.ser_CTRL &= 0xFF7F;
					GBAL[LP2].GBA.ser_CTRL &= 0xFF7F;

					GBAL[LP2].GBA.ser_Start = false;

					// set SO to expected state
					GBAL[LP2].GBA.ser_SO = GBAL[LP2].GBA.ser_SO_Inactive;
					GBAL[LP1].GBA.ser_SO = GBAL[LP1].GBA.ser_SO_Inactive;

					// trigger interrupt if needed
					if ((GBAL[LP2].GBA.ser_CTRL & 0x4000) == 0x4000)
					{
						GBAL[LP2].GBA.Trigger_IRQ(7);
					}

					GBAL[LP1].GBA.ser_Ext_Tick = false;
				}
				else
				{
					GBAL[LP1].GBA.ser_Data_0 = GBAL[LP1].GBA.ser_Data_M;
					GBAL[LP2].GBA.ser_Data_0 = GBAL[LP1].GBA.ser_Data_M;

					GBAL[LP1].GBA.ser_Data_1 = GBAL[LP2].GBA.ser_Data_M;
					GBAL[LP2].GBA.ser_Data_1 = GBAL[LP2].GBA.ser_Data_M;

					GBAL[LP1].GBA.ser_Data_2 = 0xFFFF;
					GBAL[LP2].GBA.ser_Data_2 = 0xFFFF;

					GBAL[LP1].GBA.ser_Data_3 = 0xFFFF;
					GBAL[LP2].GBA.ser_Data_3 = 0xFFFF;

					GBAL[LP1].GBA.ser_CTRL &= 0xFF7F;
					GBAL[LP2].GBA.ser_CTRL &= 0xFF7F;

					GBAL[LP2].GBA.ser_Multi_ID = 1;

					// trigger interrupt if needed
					if ((GBAL[LP2].GBA.ser_CTRL & 0x4000) == 0x4000)
					{
						GBAL[LP2].GBA.Trigger_IRQ(7);
					}

					GBAL[LP1].GBA.ser_Ext_Tick = false;
				}
			}

			if (GBAL[LP3].GBA.ser_Ext_Tick)
			{
				if (GBAL[LP3].GBA.ser_Ctrl_Mode_State != 2)
				{
					if (GBAL[LP3].GBA.ser_Bit_Total == 32)
					{
						uint16_t temp_t = GBAL[LP3].GBA.ser_Data_0;
						GBAL[LP3].GBA.ser_Data_0 = GBAL[LP4].GBA.ser_Data_0;
						GBAL[LP4].GBA.ser_Data_0 = temp_t;

						temp_t = GBAL[LP3].GBA.ser_Data_1;
						GBAL[LP3].GBA.ser_Data_1 = GBAL[LP4].GBA.ser_Data_1;
						GBAL[LP4].GBA.ser_Data_1 = temp_t;
					}
					else
					{
						uint16_t temp_t8 = (GBAL[LP3].GBA.ser_Data_M & 0xFF);
						GBAL[LP3].GBA.ser_Data_M &= 0xFF00;
						GBAL[LP3].GBA.ser_Data_M |= GBAL[LP4].GBA.ser_Data_M & 0xFF;
						GBAL[LP4].GBA.ser_Data_M &= 0xFF00;
						GBAL[LP4].GBA.ser_Data_M |= temp_t8;
					}

					GBAL[LP3].GBA.ser_CTRL &= 0xFF7F;
					GBAL[LP4].GBA.ser_CTRL &= 0xFF7F;

					GBAL[LP4].GBA.ser_Start = false;

					// set SO to expected state
					GBAL[LP4].GBA.ser_SO = GBAL[LP4].GBA.ser_SO_Inactive;
					GBAL[LP3].GBA.ser_SO = GBAL[LP3].GBA.ser_SO_Inactive;

					// trigger interrupt if needed
					if ((GBAL[LP4].GBA.ser_CTRL & 0x4000) == 0x4000)
					{
						GBAL[LP4].GBA.Trigger_IRQ(7);
					}

					GBAL[LP3].GBA.ser_Ext_Tick = false;
				}
				else
				{
					GBAL[LP3].GBA.ser_Data_0 = GBAL[LP3].GBA.ser_Data_M;
					GBAL[LP4].GBA.ser_Data_0 = GBAL[LP3].GBA.ser_Data_M;

					GBAL[LP3].GBA.ser_Data_1 = GBAL[LP4].GBA.ser_Data_M;
					GBAL[LP4].GBA.ser_Data_1 = GBAL[LP4].GBA.ser_Data_M;

					GBAL[LP3].GBA.ser_Data_2 = 0xFFFF;
					GBAL[LP4].GBA.ser_Data_2 = 0xFFFF;

					GBAL[LP3].GBA.ser_Data_3 = 0xFFFF;
					GBAL[LP4].GBA.ser_Data_3 = 0xFFFF;

					GBAL[LP3].GBA.ser_CTRL &= 0xFF7F;
					GBAL[LP4].GBA.ser_CTRL &= 0xFF7F;

					GBAL[LP4].GBA.ser_Multi_ID = 1;

					// trigger interrupt if needed
					if ((GBAL[LP4].GBA.ser_CTRL & 0x4000) == 0x4000)
					{
						GBAL[LP4].GBA.Trigger_IRQ(7);
					}

					GBAL[LP3].GBA.ser_Ext_Tick = false;
				}
			}
		}

		void Change_Linking(bool link_status, uint32_t link_type)
		{
			if (link_type == 0)
			{
				// 2 player linking
				AB_Connected = link_status;
				
				if (LP2 == 0)
				{
					LP1 = 0;
					LP2 = 1;
					
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
				}
				else
				{
					LP1 = 0;
					LP2 = 0;
					
					GBAL[0].GBA.ext_num = 0;
					GBAL[0].GBA.Ext_SI = &GBAL[0].GBA.Ext_Disconnect_1;
					GBAL[0].GBA.Ext_SO = &GBAL[0].GBA.Ext_Disconnect_1;
					GBAL[0].GBA.Ext_SC = &GBAL[0].GBA.Ext_Disconnect_1;
					GBAL[0].GBA.Ext_SD = &GBAL[0].GBA.Ext_Disconnect_0;
					GBAL[0].GBA.Ext_Multi_Start = &GBAL[0].GBA.ser_Multi_Start;

					GBAL[1].GBA.ext_num = 0;
					GBAL[1].GBA.Ext_SI = &GBAL[1].GBA.Ext_Disconnect_1;
					GBAL[1].GBA.Ext_SO = &GBAL[1].GBA.Ext_Disconnect_1;
					GBAL[1].GBA.Ext_SC = &GBAL[1].GBA.Ext_Disconnect_1;
					GBAL[1].GBA.Ext_SD = &GBAL[1].GBA.Ext_Disconnect_0;
					GBAL[1].GBA.Ext_Multi_Start = &GBAL[1].GBA.ser_Multi_Start;
				}
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

				LP1 = 0;
				LP2 = 0;
				LP3 = 0;
				LP4 = 0;

				GBAL[0].GBA.ext_num = 0;
				GBAL[0].GBA.Ext_SI = &GBAL[0].GBA.Ext_Disconnect_1;
				GBAL[0].GBA.Ext_SO = &GBAL[0].GBA.Ext_Disconnect_1;
				GBAL[0].GBA.Ext_SC = &GBAL[0].GBA.Ext_Disconnect_1;
				GBAL[0].GBA.Ext_SD = &GBAL[0].GBA.Ext_Disconnect_0;
				GBAL[0].GBA.Ext_Multi_Start = &GBAL[0].GBA.ser_Multi_Start;

				GBAL[1].GBA.ext_num = 0;
				GBAL[1].GBA.Ext_SI = &GBAL[1].GBA.Ext_Disconnect_1;
				GBAL[1].GBA.Ext_SO = &GBAL[1].GBA.Ext_Disconnect_1;
				GBAL[1].GBA.Ext_SC = &GBAL[1].GBA.Ext_Disconnect_1;
				GBAL[1].GBA.Ext_SD = &GBAL[1].GBA.Ext_Disconnect_0;
				GBAL[1].GBA.Ext_Multi_Start = &GBAL[1].GBA.ser_Multi_Start;

				GBAL[2].GBA.ext_num = 0;
				GBAL[2].GBA.Ext_SI = &GBAL[2].GBA.Ext_Disconnect_1;
				GBAL[2].GBA.Ext_SO = &GBAL[2].GBA.Ext_Disconnect_1;
				GBAL[2].GBA.Ext_SC = &GBAL[2].GBA.Ext_Disconnect_1;
				GBAL[2].GBA.Ext_SD = &GBAL[2].GBA.Ext_Disconnect_0;
				GBAL[2].GBA.Ext_Multi_Start = &GBAL[2].GBA.ser_Multi_Start;

				if (Num_ROMs > 3)
				{
					GBAL[3].GBA.ext_num = 0;
					GBAL[3].GBA.Ext_SI = &GBAL[3].GBA.Ext_Disconnect_1;
					GBAL[3].GBA.Ext_SO = &GBAL[3].GBA.Ext_Disconnect_1;
					GBAL[3].GBA.Ext_SC = &GBAL[3].GBA.Ext_Disconnect_1;
					GBAL[3].GBA.Ext_SD = &GBAL[3].GBA.Ext_Disconnect_0;
					GBAL[3].GBA.Ext_Multi_Start = &GBAL[3].GBA.ser_Multi_Start;
				}
			}
			else if (link_type == 2)
			{
				// 3 player set AB
				AB_Connected = true;

				LP1 = 0;
				LP2 = 1;

				GBAL[LP1].GBA.ext_num = 1;
				GBAL[LP2].GBA.ext_num = 2;

				Link_Console_Pair(LP1, LP2);

			}
			else if (link_type == 3)
			{
				// 3 player set BC
				BC_Connected = true;

				LP1 = 1;
				LP2 = 2;

				GBAL[LP1].GBA.ext_num = 1;
				GBAL[LP2].GBA.ext_num = 2;

				Link_Console_Pair(LP1, LP2);
			}
			else if (link_type == 4)
			{
				// 3 player set AC
				AC_Connected = true;

				LP1 = 0;
				LP2 = 2;

				GBAL[LP1].GBA.ext_num = 1;
				GBAL[LP2].GBA.ext_num = 2;

				Link_Console_Pair(LP1, LP2);
			}
			else if (link_type == 5)
			{
				// 4 player set AB, CD
				AB_Connected = true;
				CD_Connected = true;

				LP1 = 0;
				LP2 = 1;
				LP3 = 2;
				LP4 = 3;

				GBAL[LP1].GBA.ext_num = 1;
				GBAL[LP2].GBA.ext_num = 2;
				GBAL[LP3].GBA.ext_num = 1;
				GBAL[LP4].GBA.ext_num = 2;

				Link_Console_Pair(LP1, LP2);
				Link_Console_Pair(LP3, LP4);
			}
			else if (link_type == 6)
			{
				// 4 player set AC, BD
				BD_Connected = true;
				AC_Connected = true;

				LP1 = 0;
				LP2 = 2;
				LP3 = 1;
				LP4 = 3;

				GBAL[LP1].GBA.ext_num = 1;
				GBAL[LP2].GBA.ext_num = 2;
				GBAL[LP3].GBA.ext_num = 1;
				GBAL[LP4].GBA.ext_num = 2;

				Link_Console_Pair(LP1, LP2);
				Link_Console_Pair(LP3, LP4);
			}
			else if (link_type == 7)
			{
				// 4 player set AD, BC
				AD_Connected = true;
				BC_Connected = true;

				LP1 = 0;
				LP2 = 3;
				LP3 = 1;
				LP4 = 2;

				GBAL[LP1].GBA.ext_num = 1;
				GBAL[LP2].GBA.ext_num = 2;
				GBAL[LP3].GBA.ext_num = 1;
				GBAL[LP4].GBA.ext_num = 2;

				Link_Console_Pair(LP1, LP2);
				Link_Console_Pair(LP3, LP4);
			}
			else if (link_type == 8)
			{
				// 4 player set 4x
				X4_Connected = true;
			}
		}

		void Link_Console_Pair(uint8_t l1, uint8_t l2)
		{
			// override pointers to serial port pins
			GBAL[l1].GBA.Ext_SI = &GBAL[l2].GBA.ser_SO;
			GBAL[l1].GBA.Ext_SO = &GBAL[l2].GBA.ser_SI;
			GBAL[l1].GBA.Ext_SC = &GBAL[l2].GBA.ser_SC;
			GBAL[l1].GBA.Ext_SD = &GBAL[l2].GBA.ser_SD;

			GBAL[l2].GBA.Ext_SI = &GBAL[l1].GBA.ser_SO;
			GBAL[l2].GBA.Ext_SO = &GBAL[l1].GBA.ser_SI;
			GBAL[l2].GBA.Ext_SC = &GBAL[l1].GBA.ser_SC;
			GBAL[l2].GBA.Ext_SD = &GBAL[l1].GBA.ser_SD;

			GBAL[l1].GBA.Ext_Multi_Start = &GBAL[l2].GBA.ser_Multi_Start;
			GBAL[l2].GBA.Ext_Multi_Start = &GBAL[l1].GBA.ser_Multi_Start;
		}

		void GetVideo(uint32_t* dest, uint32_t num)
		{
			uint32_t* src = GBAL[num].GBA.video_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof(uint32_t) * 240 * 160);

			// blank the screen
			for (int i = 0; i < 240 * 160; i++)
			{
				GBAL[num].GBA.video_buffer[i] = 0xFFF8F8F8;
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
				return GBAL[one_en].GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0);
			}

			if (counts == 2)
			{
				// send left audio to console 1, right to console 2
				GBAL[one_en].GetAudio(dest_L_0, n_samp_L_0, dest_R_0, n_samp_R_0);

				return GBAL[two_en].GetAudio(dest_L_1, n_samp_L_1, dest_R_1, n_samp_R_1);
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

			saver = byte_saver(LP1, saver);
			saver = byte_saver(LP2, saver);
			saver = byte_saver(LP3, saver);
			saver = byte_saver(LP4, saver);
			
			for (int i = 0; i < Num_ROMs; i++)
			{
				saver = GBAL[i].GBA.SaveState(saver);
				saver = GBAL[i].Mapper->SaveState(saver);
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

			loader = byte_loader(&LP1, loader);
			loader = byte_loader(&LP2, loader);
			loader = byte_loader(&LP3, loader);
			loader = byte_loader(&LP4, loader);
			
			for (int i = 0; i < Num_ROMs; i++)
			{
				loader = GBAL[i].GBA.LoadState(loader);
				loader = GBAL[i].Mapper->LoadState(loader);
			}

			// update pointers
			// first assume everything is disconnected
			GBAL[0].GBA.ext_num = 0;
			GBAL[0].GBA.Ext_SI = &GBAL[0].GBA.Ext_Disconnect_1;
			GBAL[0].GBA.Ext_SO = &GBAL[0].GBA.Ext_Disconnect_1;
			GBAL[0].GBA.Ext_SC = &GBAL[0].GBA.Ext_Disconnect_1;
			GBAL[0].GBA.Ext_SD = &GBAL[0].GBA.Ext_Disconnect_0;
			GBAL[0].GBA.Ext_Multi_Start = &GBAL[0].GBA.ser_Multi_Start;

			GBAL[1].GBA.ext_num = 0;
			GBAL[1].GBA.Ext_SI = &GBAL[1].GBA.Ext_Disconnect_1;
			GBAL[1].GBA.Ext_SO = &GBAL[1].GBA.Ext_Disconnect_1;
			GBAL[1].GBA.Ext_SC = &GBAL[1].GBA.Ext_Disconnect_1;
			GBAL[1].GBA.Ext_SD = &GBAL[1].GBA.Ext_Disconnect_0;
			GBAL[1].GBA.Ext_Multi_Start = &GBAL[1].GBA.ser_Multi_Start;

			if (Num_ROMs > 2)
			{
				GBAL[2].GBA.ext_num = 0;
				GBAL[2].GBA.Ext_SI = &GBAL[2].GBA.Ext_Disconnect_1;
				GBAL[2].GBA.Ext_SO = &GBAL[2].GBA.Ext_Disconnect_1;
				GBAL[2].GBA.Ext_SC = &GBAL[2].GBA.Ext_Disconnect_1;
				GBAL[2].GBA.Ext_SD = &GBAL[2].GBA.Ext_Disconnect_0;
				GBAL[2].GBA.Ext_Multi_Start = &GBAL[2].GBA.ser_Multi_Start;
			}

			if (Num_ROMs > 3)
			{
				GBAL[3].GBA.ext_num = 0;
				GBAL[3].GBA.Ext_SI = &GBAL[3].GBA.Ext_Disconnect_1;
				GBAL[3].GBA.Ext_SO = &GBAL[3].GBA.Ext_Disconnect_1;
				GBAL[3].GBA.Ext_SC = &GBAL[3].GBA.Ext_Disconnect_1;
				GBAL[3].GBA.Ext_SD = &GBAL[3].GBA.Ext_Disconnect_0;
				GBAL[3].GBA.Ext_Multi_Start = &GBAL[3].GBA.ser_Multi_Start;
			}

			// then add connections as appropriate
			if (Num_ROMs < 4)
			{
				if (AB_Connected)
				{
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
				}
				else if (BC_Connected)
				{
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
				}
				else if (AC_Connected)
				{
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
				}
			}
			else
			{
				if (AB_Connected && CD_Connected)
				{
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;
					GBAL[LP3].GBA.ext_num = 1;
					GBAL[LP4].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
					Link_Console_Pair(LP3, LP4);
				}
				else if (BD_Connected && AC_Connected)
				{
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;
					GBAL[LP3].GBA.ext_num = 1;
					GBAL[LP4].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
					Link_Console_Pair(LP3, LP4);
				}
				else if (AD_Connected && BC_Connected)
				{
					GBAL[LP1].GBA.ext_num = 1;
					GBAL[LP2].GBA.ext_num = 2;
					GBAL[LP3].GBA.ext_num = 1;
					GBAL[LP4].GBA.ext_num = 2;

					Link_Console_Pair(LP1, LP2);
					Link_Console_Pair(LP3, LP4);
				}
			}
		}

		void SetRumbleCallback(void (*callbackA)(bool), void (*callbackB)(bool), void (*callbackC)(bool), void (*callbackD)(bool))
		{
			GBAL[0].GBA.RumbleCallback = callbackA;
			GBAL[1].GBA.RumbleCallback = callbackB;

			if (Num_ROMs > 2) { GBAL[2].GBA.RumbleCallback = callbackC; }
			if (Num_ROMs > 3) { GBAL[3].GBA.RumbleCallback = callbackD; }
		}

#pragma endregion

#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr, uint32_t num)
		{
			return GBAL[num].GBA.Peek_Memory_8(addr);
		}

		uint8_t GetVRAM(uint32_t addr, uint32_t num)
		{
			if (addr < 0x18000)
			{
				return GBAL[num].GBA.VRAM[addr];
			}

			return GBAL[num].GBA.VRAM[(addr & 0x7FFF) | 0x10000];		
		}

		uint8_t GetWRAM(uint32_t addr, uint32_t num)
		{
			return GBAL[num].GBA.WRAM[addr & 0x3FFFF];
		}

		uint8_t GetIWRAM(uint32_t addr, uint32_t num)
		{
			return GBAL[num].GBA.IWRAM[addr & 0x7FFF];
		}

		uint8_t GetOAM(uint32_t addr, uint32_t num)
		{
			return GBAL[num].GBA.OAM[addr & 0x3FF];
		}

		uint8_t GetPALRAM(uint32_t addr, uint32_t num)
		{
			return GBAL[num].GBA.PALRAM[addr & 0x3FF];
		}

		uint8_t GetSRAM(uint32_t addr, uint32_t num)
		{
			if (GBAL[num].GBA.Cart_RAM_Length != 0)
			{
				return GBAL[num].GBA.Cart_RAM[addr & (GBAL[num].GBA.Cart_RAM_Length - 1)];
			}

			return 0;
		}

#pragma endregion

#pragma region Tracer

		void SetTraceCallback(void (*callback)(int), uint32_t num)
		{
			if (num < Num_ROMs)
			{
				GBAL[num].GBA.TraceCallback = callback;
			}

			for (int i = 0; i < Num_ROMs; i++)
			{
				if (i != num)
				{
					GBAL[i].GBA.TraceCallback = nullptr;
				}
			}
		}

		int GetHeaderLength()
		{
			return 78 + 1;
		}

		int GetDisasmLength()
		{
			return 78 + 1;
		}

		int GetRegStringLength()
		{
			return 282 + 1;
		}

		void GetHeader(char* h, int l)
		{
			std::memcpy(h, GBAL[0].GBA.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l, uint32_t num)
		{
			if (t == 0)
			{
				std::memcpy(r, GBAL[num].GBA.CPURegisterState().c_str(), l);
			}
			else
			{
				std::memcpy(r, GBAL[num].GBA.No_Reg, l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l, uint32_t num)
		{
			if (t == 0)
			{
				std::memcpy(d, GBAL[num].GBA.CPUDisassembly().c_str(), l);
			}
			else if (t == 1)
			{
				std::memcpy(d, GBAL[num].GBA.SWI_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, GBAL[num].GBA.UDF_event, l);
			}
			else if (t == 3)
			{
				std::memcpy(d, GBAL[num].GBA.IRQ_event, l);
			}
			else
			{
				std::memcpy(d, GBAL[num].GBA.HALT_event, l);
			}
		}

	#pragma endregion

		void SetMessageCallback(void (*callback_L)(int), void (*callback_A)(int),
								void (*callback_B)(int), void (*callback_C)(int), void (*callback_D)(int))
		{
			MessageCallback = callback_L;
			GBAL[0].GBA.MessageCallback = callback_A;
			GBAL[1].GBA.MessageCallback = callback_B;

			if (Num_ROMs > 2) { GBAL[2].GBA.MessageCallback = callback_C; }
			if (Num_ROMs > 3) { GBAL[3].GBA.MessageCallback = callback_D; }
		}

		void GetMessage(char* d, int s)
		{
			if (s == 5)
			{
				std::memcpy(d, Message_String.c_str(), Message_String.length() + 1);
			}
			else if (s < Num_ROMs)
			{
				std::memcpy(d, GBAL[s].GBA.Message_String.c_str(), GBAL[s].GBA.Message_String.length() + 1);
			}
		}
	};
}

#endif

