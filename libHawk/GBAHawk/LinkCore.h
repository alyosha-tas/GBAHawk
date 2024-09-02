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
		GBALinkCore()
		{
			L.GBA.System_Reset();
			L.Mapper = nullptr;

			R.GBA.System_Reset();
			R.Mapper = nullptr;
		};

		GBACore L;
		GBACore R;

		void Load_BIOS(uint8_t* bios)
		{
			std::memcpy(L.GBA.BIOS, bios, 0x4000);
			std::memcpy(R.GBA.BIOS, bios, 0x4000);
		}

		void Load_ROM(uint8_t* ext_rom_0, uint32_t ext_rom_size_0, uint32_t mapper_0,
						uint8_t* ext_rom_1, uint32_t ext_rom_size_1, uint32_t mapper_1,
						uint64_t datetime_0, bool rtc_functional_0,
						uint64_t datetime_1, bool rtc_functional_1,
						int16_t EEPROM_offset_0, int16_t EEPROM_offset_1,
						int16_t flash_write_offset_0, int16_t flash_write_offset_1,
						int16_t flash_sector_offset_0, int16_t flash_sector_offset_1,
						int16_t flash_chip_offset_0, int16_t flash_chip_offset_1,
						bool is_GBP_0, bool is_GBP_1)
		{
			L.Load_ROM(ext_rom_0, ext_rom_size_0, mapper_0, datetime_0, rtc_functional_0, EEPROM_offset_0, flash_write_offset_0, flash_sector_offset_0, flash_chip_offset_0, is_GBP_0);
			R.Load_ROM(ext_rom_1, ext_rom_size_1, mapper_1, datetime_1, rtc_functional_1, EEPROM_offset_1, flash_write_offset_1, flash_sector_offset_1, flash_chip_offset_1, is_GBP_1);
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
				std::memcpy(L.GBA.Cart_RAM, ext_sram, ext_sram_size);
			}
			else
			{
				std::memcpy(R.GBA.Cart_RAM, ext_sram, ext_sram_size);
			}	
		}

		void Hard_Reset()
		{
			L.Mapper->Reset();
			R.Mapper->Reset();

			L.GBA.System_Reset();
			R.GBA.System_Reset();

			// system starts connected
			L.GBA.ext_num = 1;
			R.GBA.ext_num = 2;

			L.GBA.is_linked_system = true;
			R.GBA.is_linked_system = true;

			// change ser control state since its plugged in
			L.GBA.ser_CTRL = 0;
			R.GBA.ser_CTRL = 0;
		}

		void Set_GBP_Enable(int num)
		{
			if (num == 0)
			{
				L.GBA.GBP_Mode_Enabled = true;
			}
			else
			{
				R.GBA.GBP_Mode_Enabled = true;
			}
		}

		bool FrameAdvance(uint16_t controller_0, uint16_t accx_0, uint16_t accy_0, uint8_t solar_0, bool render_0, bool rendersound_0,
						  uint16_t controller_1, uint16_t accx_1, uint16_t accy_1, uint8_t solar_1, bool render_1, bool rendersound_1,
						  bool l_reset, bool r_reset)
		{
			L.GBA.New_Controller = controller_0;
			L.GBA.New_Acc_X = accx_0;
			L.GBA.New_Acc_Y = accy_0;
			L.GBA.New_Solar = solar_0;

			// update the controller state
			L.GBA.controller_state_old = L.GBA.controller_state;
			L.GBA.controller_state = L.GBA.New_Controller;

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (L.GBA.VBlank_Rise || L.GBA.stopped)
			{
				// check if controller state caused interrupt
				L.GBA.do_controller_check(false);
			}

			L.GBA.snd_Master_Clock = 0;

			L.GBA.num_samples_L = 0;
			L.GBA.num_samples_R = 0;

			L.GBA.Is_Lag = true;

			L.GBA.VBlank_Rise = false;

			R.GBA.New_Controller = controller_1;
			R.GBA.New_Acc_X = accx_1;
			R.GBA.New_Acc_Y = accy_1;
			R.GBA.New_Solar = solar_1;

			// update the controller state
			R.GBA.controller_state_old = R.GBA.controller_state;
			R.GBA.controller_state = R.GBA.New_Controller;

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (R.GBA.VBlank_Rise || R.GBA.stopped)
			{
				// check if controller state caused interrupt
				R.GBA.do_controller_check(false);
			}

			R.GBA.snd_Master_Clock = 0;

			R.GBA.num_samples_L = 0;
			R.GBA.num_samples_R = 0;

			R.GBA.Is_Lag = true;

			R.GBA.VBlank_Rise = false;

			if (l_reset)
			{
				L.Mapper->Reset();
				L.GBA.System_Reset();
				L.GBA.ser_CTRL = 0;

				for (int i = 0; i < 80081; i++)
				{
					R.GBA.Single_Step();

					// sync up state bits
					if (L.GBA.ser_Ext_Update)
					{
						if (L.GBA.ser_Mode_State == 3)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else if (L.GBA.ser_Mode_State == 2)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else
						{
							if (L.GBA.ser_Ctrl_Mode_State == 3)
							{
								// uart
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
							else if (L.GBA.ser_Ctrl_Mode_State == 2)
							{
								// multiplayer
								if ((R.GBA.ser_Mode_State < 2) && (R.GBA.ser_Ctrl_Mode_State == 2))
								{
									L.GBA.ser_CTRL |= 8;
									R.GBA.ser_CTRL |= 8;

									if ((L.GBA.ser_CTRL & 0x80) == 0x80)
									{
										R.GBA.ser_CTRL |= 0x80;
									}
									else
									{
										R.GBA.ser_CTRL &= 0xFF7F;
									}
								}
								else
								{
									L.GBA.ser_CTRL &= 0xFFF7;
									R.GBA.ser_CTRL &= 0xFFF7;
								}
							}
							else
							{
								// normal
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
						}

						L.GBA.ser_Ext_Update = false;
					}

					if (R.GBA.ser_Ext_Update)
					{
						if (R.GBA.ser_Mode_State == 3)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else if (R.GBA.ser_Mode_State == 2)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else
						{
							if (R.GBA.ser_Ctrl_Mode_State == 3)
							{
								// uart
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
							else if (R.GBA.ser_Ctrl_Mode_State == 2)
							{
								// multiplayer
								if ((L.GBA.ser_Mode_State < 2) && (L.GBA.ser_Ctrl_Mode_State == 2))
								{
									L.GBA.ser_CTRL |= 8;
									R.GBA.ser_CTRL |= 8;
									/*
									if ((L.ser_CTRL & 0x80) == 0x80)
									{
										R.ser_CTRL |= 0x80;
									}
									*/
								}
								else
								{
									L.GBA.ser_CTRL &= 0xFFF7;
									R.GBA.ser_CTRL &= 0xFFF7;
								}
							}
							else
							{
								// normal
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
						}

						R.GBA.ser_Ext_Update = false;
					}

					// transfer a bit
					if (L.GBA.ser_Ext_Tick)
					{
						if (L.GBA.ser_Ctrl_Mode_State != 2)
						{
							uint16_t temp_t = L.GBA.ser_Data_0;
							L.GBA.ser_Data_0 = R.GBA.ser_Data_0;
							R.GBA.ser_Data_0 = temp_t;

							temp_t = L.GBA.ser_Data_1;
							L.GBA.ser_Data_1 = R.GBA.ser_Data_1;
							R.GBA.ser_Data_1 = temp_t;

							L.GBA.ser_CTRL &= 0xFF7F;
							R.GBA.ser_CTRL &= 0xFF7F;

							// trigger interrupt if needed
							if ((R.GBA.ser_CTRL & 0x4000) == 0x4000)
							{
								R.GBA.Trigger_IRQ(7);
							}

							L.GBA.ser_Ext_Tick = false;
						}
						else
						{
							L.GBA.ser_Data_0 = L.GBA.ser_Data_M;
							R.GBA.ser_Data_0 = L.GBA.ser_Data_M;

							L.GBA.ser_Data_1 = R.GBA.ser_Data_M;
							R.GBA.ser_Data_1 = R.GBA.ser_Data_M;

							L.GBA.ser_Data_2 = 0xFFFF;
							R.GBA.ser_Data_2 = 0xFFFF;

							L.GBA.ser_Data_3 = 0xFFFF;
							R.GBA.ser_Data_3 = 0xFFFF;

							L.GBA.ser_CTRL &= 0xFF7F;
							R.GBA.ser_CTRL &= 0xFF7F;

							R.GBA.ser_CTRL |= 0x10;

							// trigger interrupt if needed
							if ((R.GBA.ser_CTRL & 0x4000) == 0x4000)
							{
								R.GBA.Trigger_IRQ(7);
							}

							L.GBA.ser_Ext_Tick = false;
						}
					}
				}
			}

			if (r_reset)
			{
				R.Mapper->Reset();
				R.GBA.System_Reset();
				R.GBA.ser_CTRL = 0;

				for (int i = 0; i < 80081; i++)
				{
					L.GBA.Single_Step();

					// sync up state bits
					if (L.GBA.ser_Ext_Update)
					{
						if (L.GBA.ser_Mode_State == 3)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else if (L.GBA.ser_Mode_State == 2)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else
						{
							if (L.GBA.ser_Ctrl_Mode_State == 3)
							{
								// uart
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
							else if (L.GBA.ser_Ctrl_Mode_State == 2)
							{
								// multiplayer
								if ((R.GBA.ser_Mode_State < 2) && (R.GBA.ser_Ctrl_Mode_State == 2))
								{
									L.GBA.ser_CTRL |= 8;
									R.GBA.ser_CTRL |= 8;

									if ((L.GBA.ser_CTRL & 0x80) == 0x80)
									{
										R.GBA.ser_CTRL |= 0x80;
									}
									else
									{
										R.GBA.ser_CTRL &= 0xFF7F;
									}
								}
								else
								{
									L.GBA.ser_CTRL &= 0xFFF7;
									R.GBA.ser_CTRL &= 0xFFF7;
								}
							}
							else
							{
								// normal
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
						}

						L.GBA.ser_Ext_Update = false;
					}

					if (R.GBA.ser_Ext_Update)
					{
						if (R.GBA.ser_Mode_State == 3)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else if (R.GBA.ser_Mode_State == 2)
						{
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else
						{
							if (R.GBA.ser_Ctrl_Mode_State == 3)
							{
								// uart
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
							else if (R.GBA.ser_Ctrl_Mode_State == 2)
							{
								// multiplayer
								if ((L.GBA.ser_Mode_State < 2) && (L.GBA.ser_Ctrl_Mode_State == 2))
								{
									L.GBA.ser_CTRL |= 8;
									R.GBA.ser_CTRL |= 8;
									/*
									if ((L.ser_CTRL & 0x80) == 0x80)
									{
										R.ser_CTRL |= 0x80;
									}
									*/
								}
								else
								{
									L.GBA.ser_CTRL &= 0xFFF7;
									R.GBA.ser_CTRL &= 0xFFF7;
								}
							}
							else
							{
								// normal
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
						}

						R.GBA.ser_Ext_Update = false;
					}

					// transfer a bit
					if (L.GBA.ser_Ext_Tick)
					{
						if (L.GBA.ser_Ctrl_Mode_State != 2)
						{
							uint16_t temp_t = L.GBA.ser_Data_0;
							L.GBA.ser_Data_0 = R.GBA.ser_Data_0;
							R.GBA.ser_Data_0 = temp_t;

							temp_t = L.GBA.ser_Data_1;
							L.GBA.ser_Data_1 = R.GBA.ser_Data_1;
							R.GBA.ser_Data_1 = temp_t;

							L.GBA.ser_CTRL &= 0xFF7F;
							R.GBA.ser_CTRL &= 0xFF7F;

							// trigger interrupt if needed
							if ((R.GBA.ser_CTRL & 0x4000) == 0x4000)
							{
								R.GBA.Trigger_IRQ(7);
							}

							L.GBA.ser_Ext_Tick = false;
						}
						else
						{
							L.GBA.ser_Data_0 = L.GBA.ser_Data_M;
							R.GBA.ser_Data_0 = L.GBA.ser_Data_M;

							L.GBA.ser_Data_1 = R.GBA.ser_Data_M;
							R.GBA.ser_Data_1 = R.GBA.ser_Data_M;

							L.GBA.ser_Data_2 = 0xFFFF;
							R.GBA.ser_Data_2 = 0xFFFF;

							L.GBA.ser_Data_3 = 0xFFFF;
							R.GBA.ser_Data_3 = 0xFFFF;

							L.GBA.ser_CTRL &= 0xFF7F;
							R.GBA.ser_CTRL &= 0xFF7F;

							R.GBA.ser_CTRL |= 0x10;

							// trigger interrupt if needed
							if ((R.GBA.ser_CTRL & 0x4000) == 0x4000)
							{
								R.GBA.Trigger_IRQ(7);
							}

							L.GBA.ser_Ext_Tick = false;
						}
					}
				}
			}

			// NOTE: the two cores will always be in sync in terms of vblank rise
			while (!L.GBA.VBlank_Rise)
			{
				L.GBA.Single_Step();
				R.GBA.Single_Step();

				// sync up state bits
				if (L.GBA.ser_Ext_Update)
				{
					if (L.GBA.ser_Mode_State == 3)
					{
						L.GBA.ser_CTRL &= 0xFFF7;
						R.GBA.ser_CTRL &= 0xFFF7;
					}
					else if (L.GBA.ser_Mode_State == 2)
					{
						L.GBA.ser_CTRL &= 0xFFF7;
						R.GBA.ser_CTRL &= 0xFFF7;
					}
					else
					{
						if (L.GBA.ser_Ctrl_Mode_State == 3)
						{
							// uart
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else if (L.GBA.ser_Ctrl_Mode_State == 2)
						{
							// multiplayer
							if ((R.GBA.ser_Mode_State < 2) && (R.GBA.ser_Ctrl_Mode_State == 2))
							{
								L.GBA.ser_CTRL |= 8;
								R.GBA.ser_CTRL |= 8;

								if ((L.GBA.ser_CTRL & 0x80) == 0x80)
								{
									R.GBA.ser_CTRL |= 0x80;
								}
								else
								{
									R.GBA.ser_CTRL &= 0xFF7F;
								}
							}
							else
							{
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
						}
						else
						{
							// normal
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
					}

					L.GBA.ser_Ext_Update = false;
				}

				if (R.GBA.ser_Ext_Update)
				{
					if (R.GBA.ser_Mode_State == 3)
					{
						L.GBA.ser_CTRL &= 0xFFF7;
						R.GBA.ser_CTRL &= 0xFFF7;
					}
					else if (R.GBA.ser_Mode_State == 2)
					{
						L.GBA.ser_CTRL &= 0xFFF7;
						R.GBA.ser_CTRL &= 0xFFF7;
					}
					else
					{
						if (R.GBA.ser_Ctrl_Mode_State == 3)
						{
							// uart
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
						else if (R.GBA.ser_Ctrl_Mode_State == 2)
						{
							// multiplayer
							if ((L.GBA.ser_Mode_State < 2) && (L.GBA.ser_Ctrl_Mode_State == 2))
							{
								L.GBA.ser_CTRL |= 8;
								R.GBA.ser_CTRL |= 8;
								/*
								if ((L.ser_CTRL & 0x80) == 0x80)
								{
									R.ser_CTRL |= 0x80;
								}
								*/
							}
							else
							{
								L.GBA.ser_CTRL &= 0xFFF7;
								R.GBA.ser_CTRL &= 0xFFF7;
							}
						}
						else
						{
							// normal
							L.GBA.ser_CTRL &= 0xFFF7;
							R.GBA.ser_CTRL &= 0xFFF7;
						}
					}

					R.GBA.ser_Ext_Update = false;
				}

				// transfer a bit
				if (L.GBA.ser_Ext_Tick)
				{
					if (L.GBA.ser_Ctrl_Mode_State != 2)
					{
						uint16_t temp_t = L.GBA.ser_Data_0;
						L.GBA.ser_Data_0 = R.GBA.ser_Data_0;
						R.GBA.ser_Data_0 = temp_t;

						temp_t = L.GBA.ser_Data_1;
						L.GBA.ser_Data_1 = R.GBA.ser_Data_1;
						R.GBA.ser_Data_1 = temp_t;

						L.GBA.ser_CTRL &= 0xFF7F;
						R.GBA.ser_CTRL &= 0xFF7F;

						// trigger interrupt if needed
						if ((R.GBA.ser_CTRL & 0x4000) == 0x4000)
						{
							R.GBA.Trigger_IRQ(7);
						}

						L.GBA.ser_Ext_Tick = false;
					}
					else
					{
						L.GBA.ser_Data_0 = L.GBA.ser_Data_M;
						R.GBA.ser_Data_0 = L.GBA.ser_Data_M;

						L.GBA.ser_Data_1 = R.GBA.ser_Data_M;
						R.GBA.ser_Data_1 = R.GBA.ser_Data_M;

						L.GBA.ser_Data_2 = 0xFFFF;
						R.GBA.ser_Data_2 = 0xFFFF;

						L.GBA.ser_Data_3 = 0xFFFF;
						R.GBA.ser_Data_3 = 0xFFFF;

						L.GBA.ser_CTRL &= 0xFF7F;
						R.GBA.ser_CTRL &= 0xFF7F;

						R.GBA.ser_CTRL |= 0x10;

						// trigger interrupt if needed
						if ((R.GBA.ser_CTRL & 0x4000) == 0x4000)
						{
							R.GBA.Trigger_IRQ(7);
						}

						L.GBA.ser_Ext_Tick = false;
					}
				}

			}

			return L.GBA.Is_Lag && R.GBA.Is_Lag;
		}

		void GetVideo(uint32_t* dest, uint32_t num)
		{
			if (num == 0)
			{
				uint32_t* src = L.GBA.video_buffer;
				uint32_t* dst = dest;

				std::memcpy(dst, src, sizeof(uint32_t) * 240 * 160);

				// blank the screen
				for (int i = 0; i < 240 * 160; i++)
				{
					L.GBA.video_buffer[i] = 0xFFF8F8F8;
				}
			}
			else
			{
				uint32_t* src = R.GBA.video_buffer;
				uint32_t* dst = dest;

				std::memcpy(dst, src, sizeof(uint32_t) * 240 * 160);

				// blank the screen
				for (int i = 0; i < 240 * 160; i++)
				{
					R.GBA.video_buffer[i] = 0xFFF8F8F8;
				}
			}		
		}

		uint32_t GetAudio(int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R, uint32_t num)
		{
			if (num == 0)
			{
				int32_t* src = L.GBA.samples_L;
				int32_t* dst = dest_L;

				std::memcpy(dst, src, sizeof int32_t * L.GBA.num_samples_L * 2);
				n_samp_L[0] = L.GBA.num_samples_L;

				src = L.GBA.samples_R;
				dst = dest_R;

				std::memcpy(dst, src, sizeof int32_t * L.GBA.num_samples_R * 2);
				n_samp_R[0] = L.GBA.num_samples_R;

				uint32_t temp_int = L.GBA.snd_Master_Clock;

				return temp_int;
			}
			else if (num == 1)
			{
				int32_t* src = R.GBA.samples_L;
				int32_t* dst = dest_L;

				std::memcpy(dst, src, sizeof int32_t * R.GBA.num_samples_L * 2);
				n_samp_L[0] = R.GBA.num_samples_L;

				src = R.GBA.samples_R;
				dst = dest_R;

				std::memcpy(dst, src, sizeof int32_t * R.GBA.num_samples_R * 2);
				n_samp_R[0] = R.GBA.num_samples_R;

				uint32_t temp_int = R.GBA.snd_Master_Clock;

				return temp_int;
			}
			else
			{
				int32_t* src = L.GBA.samples_L;
				int32_t* dst = dest_L;

				std::memcpy(dst, src, sizeof int32_t * L.GBA.num_samples_L * 2);
				n_samp_L[0] = L.GBA.num_samples_L;

				src = R.GBA.samples_R;
				dst = dest_R;

				std::memcpy(dst, src, sizeof int32_t * R.GBA.num_samples_R * 2);
				n_samp_R[0] = R.GBA.num_samples_R;

				uint32_t temp_int = (R.GBA.snd_Master_Clock > L.GBA.snd_Master_Clock) ? R.GBA.snd_Master_Clock : L.GBA.snd_Master_Clock;

				return temp_int;
			}
		}

#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{
			saver = L.GBA.SaveState(saver);
			saver = L.Mapper->SaveState(saver);

			saver = R.GBA.SaveState(saver);
			saver = R.Mapper->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = L.GBA.LoadState(loader);
			loader = L.Mapper->LoadState(loader);

			loader = R.GBA.LoadState(loader);
			loader = R.Mapper->LoadState(loader);
		}

		void SetRumbleCallback(void (*callback)(bool), uint32_t num)
		{
			if (num == 0)
			{
				L.GBA.RumbleCallback = callback;
			}
			else
			{
				R.GBA.RumbleCallback = callback;
			}
		}

#pragma endregion

#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GBA.Peek_Memory_8(addr);
			}
			else
			{
				return R.GBA.Peek_Memory_8(addr);
			}
		}

		uint8_t GetVRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				if (addr < 0x18000)
				{
					return L.GBA.VRAM[addr];
				}

				return L.GBA.VRAM[(addr & 0x7FFF) | 0x10000];
			}
			else
			{
				if (addr < 0x18000)
				{
					return R.GBA.VRAM[addr];
				}

				return R.GBA.VRAM[(addr & 0x7FFF) | 0x10000];
			}			
		}

		uint8_t GetWRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GBA.WRAM[addr & 0x3FFFF];
			}
			else
			{
				return R.GBA.WRAM[addr & 0x3FFFF];
			}
		}

		uint8_t GetIWRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GBA.IWRAM[addr & 0x7FFF];
			}
			else
			{
				return R.GBA.IWRAM[addr & 0x7FFF];
			}
		}

		uint8_t GetOAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GBA.OAM[addr & 0x3FF];
			}
			else
			{
				return R.GBA.OAM[addr & 0x3FF];
			}
		}

		uint8_t GetPALRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				return L.GBA.PALRAM[addr & 0x3FF];
			}
			else
			{
				return R.GBA.PALRAM[addr & 0x3FF];
			}
		}

		uint8_t GetSRAM(uint32_t addr, uint32_t num)
		{
			if (num == 0)
			{
				if (L.GBA.Cart_RAM_Length != 0)
				{
					return L.GBA.Cart_RAM[addr & (L.GBA.Cart_RAM_Length - 1)];
				}

				return 0;
			}
			else
			{
				if (R.GBA.Cart_RAM_Length != 0)
				{
					return R.GBA.Cart_RAM[addr & (R.GBA.Cart_RAM_Length - 1)];
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
				L.GBA.TraceCallback = callback;
				R.GBA.TraceCallback = nullptr;
			}
			else
			{
				R.GBA.TraceCallback = callback;
				L.GBA.TraceCallback = nullptr;
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
			std::memcpy(h, L.GBA.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l, uint32_t num)
		{
			if (num == 0)
			{
				if (t == 0)
				{
					std::memcpy(r, L.GBA.CPURegisterState().c_str(), l);
				}
				else
				{
					std::memcpy(r, L.GBA.No_Reg, l);
				}
			}
			else
			{
				if (t == 0)
				{
					std::memcpy(r, R.GBA.CPURegisterState().c_str(), l);
				}
				else
				{
					std::memcpy(r, R.GBA.No_Reg, l);
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
					std::memcpy(d, L.GBA.CPUDisassembly().c_str(), l);
				}
				else if (t == 1)
				{
					std::memcpy(d, L.GBA.SWI_event, l);
				}
				else if (t == 2)
				{
					std::memcpy(d, L.GBA.UDF_event, l);
				}
				else if (t == 3)
				{
					std::memcpy(d, L.GBA.IRQ_event, l);
				}
				else
				{
					std::memcpy(d, L.GBA.HALT_event, l);
				}
			}
			else
			{
				if (t == 0)
				{
					std::memcpy(d, R.GBA.CPUDisassembly().c_str(), l);
				}
				else if (t == 1)
				{
					std::memcpy(d, R.GBA.SWI_event, l);
				}
				else if (t == 2)
				{
					std::memcpy(d, R.GBA.UDF_event, l);
				}
				else if (t == 3)
				{
					std::memcpy(d, R.GBA.IRQ_event, l);
				}
				else
				{
					std::memcpy(d, R.GBA.HALT_event, l);
				}
			}	
		}

#pragma endregion

	};
}

#endif

