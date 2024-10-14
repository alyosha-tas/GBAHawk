#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "GBA_System.h"
#include "Memory.h"
#include "Mappers.h"

using namespace std;

namespace GBAHawk
{
	class GBACore
	{
	public:
		GBACore() 
		{
			Mapper = nullptr;
			GBA.System_Reset();
		};

		GBA_System GBA;
		Mappers* Mapper;

		void Load_BIOS(uint8_t* bios)
		{
			std::memcpy(GBA.BIOS, bios, 0x4000);
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint32_t mapper, uint64_t datetime, bool rtc_functional, int16_t EEPROM_offset, 
					  int16_t flash_write_offset, int32_t flash_sector_offset, int32_t flash_chip_offset, bool is_GBP)
		{
			std::memcpy(GBA.ROM, ext_rom, 0x6000000);

			if ((mapper == 0) || (mapper == 1))
			{
				GBA.Cart_RAM_Present = false;
				GBA.Is_EEPROM = false;
			}
			else if ((mapper == 2) || (mapper == 3))
			{
				GBA.Cart_RAM_Present = true;
				GBA.Is_EEPROM = false;
			}
			else if (mapper == 4)
			{
				GBA.Cart_RAM_Present = true;
				GBA.Is_EEPROM = true;

				if (ext_rom_size <= 0x1000000)
				{
					GBA.EEPROM_Wiring = true;
				}
				else
				{
					GBA.EEPROM_Wiring = false;
				}
			}
			else if (mapper == 5)
			{
				GBA.Cart_RAM_Present = true;
				GBA.Is_EEPROM = true;

				if (ext_rom_size <= 0x1000000)
				{
					GBA.EEPROM_Wiring = true;
				}
				else
				{
					GBA.EEPROM_Wiring = false;
				}
			}
			else if (mapper == 6)
			{
				GBA.Cart_RAM_Present = true;
				GBA.Is_EEPROM = true;

				if (ext_rom_size <= 0x1000000)
				{
					GBA.EEPROM_Wiring = true;
				}
				else
				{
					GBA.EEPROM_Wiring = false;
				}
			}
			else if ((mapper == 7) || (mapper == 8))
			{
				GBA.Cart_RAM_Present = true;
				GBA.Is_EEPROM = false;
			}

			if (mapper == 0)
			{
				Mapper = new Mapper_Default();
			}
			else if (mapper == 1)
			{
				Mapper = new Mapper_DefaultRTC();
			}
			else if (mapper == 2)
			{
				Mapper = new Mapper_SRAM();
			}
			else if (mapper == 3)
			{
				Mapper = new Mapper_SRAM_Gyro();
			}
			else if (mapper == 4)
			{
				Mapper = new Mapper_EEPROM();
			}
			else if (mapper == 5)
			{
				Mapper = new Mapper_EEPROM_Tilt();
			}
			else if (mapper == 6)
			{
				Mapper = new Mapper_EEPROM_Solar();
			}
			else if (mapper == 7)
			{
				Mapper = new Mapper_FLASH();
			}
			else if (mapper == 8)
			{
				Mapper = new Mapper_FLASH_RTC();
			}

			GBA.mapper_pntr = &Mapper[0];

			Mapper->Core_Cycle_Count = &GBA.CycleCount;

			Mapper->Core_Clock_Update_Cycle = &GBA.Clock_Update_Cycle;

			Mapper->Core_Acc_X = &GBA.New_Acc_X;

			Mapper->Core_Acc_Y = &GBA.New_Acc_Y;

			Mapper->Core_Solar = &GBA.New_Solar;

			Mapper->Core_ROM = &GBA.ROM[0];

			Mapper->ROM_C4 = GBA.ROM[0xC4];
			Mapper->ROM_C5 = GBA.ROM[0xC5];
			Mapper->ROM_C6 = GBA.ROM[0xC6];
			Mapper->ROM_C7 = GBA.ROM[0xC7];
			Mapper->ROM_C8 = GBA.ROM[0xC8];
			Mapper->ROM_C9 = GBA.ROM[0xC9];

			Mapper->Current_C4 = GBA.ROM[0xC4];
			Mapper->Current_C5 = GBA.ROM[0xC5];
			Mapper->Current_C6 = GBA.ROM[0xC6];
			Mapper->Current_C7 = GBA.ROM[0xC7];
			Mapper->Current_C8 = GBA.ROM[0xC8];
			Mapper->Current_C9 = GBA.ROM[0xC9];

			// only reset RTC on initial power on
			Mapper->Reset_RTC = true;
			Mapper->Reset();
			Mapper->Reset_RTC = false;

			Mapper->RTC_Functional = rtc_functional;

			Mapper->Solar_Functional = mapper == 6;

			Mapper->Reg_Second = (uint8_t)datetime;
			Mapper->Reg_Minute = (uint8_t)(datetime >> 8);
			Mapper->Reg_Hour = (uint8_t)(datetime >> 16);
			Mapper->Reg_Week = (uint8_t)(datetime >> 24);
			Mapper->Reg_Day = (uint8_t)(datetime >> 32);
			Mapper->Reg_Month = (uint8_t)(datetime >> 40);
			Mapper->Reg_Year = (uint8_t)(datetime >> 48);
			Mapper->Reg_Ctrl = (uint8_t)(datetime >> 56);

			Mapper->RTC_24_Hour = (Mapper->Reg_Ctrl & 0x40) == 0x40;

			Mapper->EEPROM_Offset = EEPROM_offset;
			Mapper->Flash_Write_Offset = flash_write_offset;
			Mapper->Flash_Sector_Erase_Offset = flash_sector_offset;
			Mapper->Flash_Chip_Erase_Offset = flash_chip_offset;

			Mapper->RumbleCallback = GBA.RumbleCallback;

			// Only reset cycle count on initial power on, not power cycles
			GBA.CycleCount = 0;
			GBA.Clock_Update_Cycle = 0;

			GBA.Is_GBP = is_GBP;
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			GBA.Cart_RAM = new uint8_t[ext_sram_size];

			GBA.Cart_RAM_Length = ext_sram_size;

			std::memcpy(GBA.Cart_RAM, ext_sram, ext_sram_size);

			Mapper->Cart_RAM = &GBA.Cart_RAM[0];

			Mapper->Size_Mask = ext_sram_size - 1;
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			std::memcpy(GBA.Cart_RAM, ext_sram, ext_sram_size);
		}

		void Hard_Reset() 
		{
			Mapper->Reset();
			
			GBA.System_Reset();
		}

		void Set_GBP_Enable()
		{
			GBA.GBP_Mode_Enabled = true;
		}

		bool FrameAdvance(uint16_t controller_1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool rendersound)
		{
			GBA.New_Controller = controller_1;
			GBA.New_Acc_X = accx;
			GBA.New_Acc_Y = accy;
			GBA.New_Solar = solar;

			// update the controller state
			GBA.controller_state_old = GBA.controller_state;
			GBA.controller_state = GBA.New_Controller;

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (GBA.VBlank_Rise || GBA.stopped)
			{
				// check if controller state caused interrupt
				GBA.do_controller_check(false);
			}

			GBA.snd_Master_Clock = 0;

			GBA.num_samples_L = 0;
			GBA.num_samples_R = 0;

			GBA.Is_Lag = true;

			GBA.VBlank_Rise = false;

			if (!GBA.stopped)
			{
				GBA.Frame_Advance();
			}
			else
			{
				if ((GBA.INT_EN & GBA.INT_Flags & 0x1000) == 0x1000)
				{
					GBA.stopped = false;
				}
			}

			return GBA.Is_Lag;
		}

		bool SubFrameAdvance(uint16_t controller_1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool rendersound, bool do_reset, uint32_t reset_cycle)
		{
			GBA.New_Controller = controller_1;
			GBA.New_Acc_X = accx;
			GBA.New_Acc_Y = accy;
			GBA.New_Solar = solar;

			// update the controller state
			GBA.controller_state_old = GBA.controller_state;
			GBA.controller_state = GBA.New_Controller;

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (GBA.VBlank_Rise || GBA.stopped)
			{
				// check if controller state caused interrupt
				GBA.do_controller_check(false);
			}

			GBA.snd_Master_Clock = 0;

			GBA.num_samples_L = 0;
			GBA.num_samples_R = 0;

			GBA.Is_Lag = true;

			GBA.VBlank_Rise = false;

			bool reset_was_done = false;

			if (!do_reset) { reset_cycle = -1; }

			if (!GBA.stopped)
			{
				reset_was_done = GBA.SubFrame_Advance(reset_cycle);
			}
			else
			{
				if ((GBA.INT_EN & GBA.INT_Flags & 0x1000) == 0x1000)
				{
					GBA.stopped = false;
				}
			}

			if (reset_was_done)
			{
				Hard_Reset();
			}

			return GBA.Is_Lag;
		}

		void GetVideo(uint32_t* dest) 
		{
			uint32_t* src = GBA.video_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof (uint32_t) * 240 * 160);

			// blank the screen
			for (int i = 0; i < 240 * 160; i++)
			{
				GBA.video_buffer[i] = 0xFFF8F8F8;
			}
		}

		uint32_t GetAudio(int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
		{
			int32_t* src = GBA.samples_L;
			int32_t* dst = dest_L;

			std::memcpy(dst, src, sizeof int32_t * GBA.num_samples_L * 2);
			n_samp_L[0] = GBA.num_samples_L;

			src = GBA.samples_R;
			dst = dest_R;

			std::memcpy(dst, src, sizeof int32_t * GBA.num_samples_R * 2);
			n_samp_R[0] = GBA.num_samples_R;

			uint32_t temp_int = GBA.snd_Master_Clock;
			
			return temp_int;
		}

		#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{	
			saver = GBA.SaveState(saver);
			saver = Mapper->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = GBA.LoadState(loader);
			loader = Mapper->LoadState(loader);
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return GBA.Peek_Memory_8(addr);
		}

		uint8_t GetVRAM(uint32_t addr) 
		{
			if (addr < 0x18000)
			{
				return GBA.VRAM[addr];
			}

			return GBA.VRAM[(addr & 0x7FFF) | 0x10000];
		}

		uint8_t GetWRAM(uint32_t addr)
		{
			return GBA.WRAM[addr & 0x3FFFF];
		}

		uint8_t GetIWRAM(uint32_t addr)
		{
			return GBA.IWRAM[addr & 0x7FFF];
		}

		uint8_t GetOAM(uint32_t addr)
		{
			return GBA.OAM[addr & 0x3FF];
		}

		uint8_t GetPALRAM(uint32_t addr)
		{
			return GBA.PALRAM[addr & 0x3FF];
		}

		uint8_t GetSRAM(uint32_t addr)
		{
			if (GBA.Cart_RAM_Length != 0) 
			{
				return GBA.Cart_RAM[addr & (GBA.Cart_RAM_Length - 1)];
			}
			
			return 0;
		}

		#pragma endregion

		#pragma region Tracer

		void SetTraceCallback(void (*callback)(int))
		{
			GBA.TraceCallback = callback;
		}

		void SetRumbleCallback(void (*callback)(bool))
		{
			GBA.RumbleCallback = callback;
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
			std::memcpy(h, GBA.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(r, GBA.CPURegisterState().c_str(), l);
			}
			else
			{
				std::memcpy(r, GBA.No_Reg, l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(d, GBA.CPUDisassembly().c_str(), l);
			}
			else if (t == 1)
			{
				std::memcpy(d, GBA.SWI_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, GBA.UDF_event, l);
			}
			else if (t == 3)
			{
				std::memcpy(d, GBA.IRQ_event, l);
			}
			else
			{
				std::memcpy(d, GBA.HALT_event, l);
			}
		}

		#pragma endregion

		void SetMessageCallback(void (*callback)(int))
		{
			GBA.MessageCallback = callback;
		}

		void GetMessage(char* d, int l)
		{
			std::memcpy(d, GBA.Message_String.c_str(), GBA.Message_String.length() + 1);
		}

		uint8_t* Get_PPU_Pointers(int sel)
		{
			switch (sel)
			{
				case 0: return GBA.VRAM; break;
				case 1: return GBA.OAM; break;
				case 2: return GBA.PALRAM; break;
				case 3:
					for (int i = 0; i < 0x60; i++)
					{
						GBA.PPU_IO[i] = GBA.ppu_Read_Reg_8(i);
					}
					return GBA.PPU_IO; break;
			}

			return nullptr;
		}

		void SetScanlineCallback(void (*callback)(void), int sl)
		{
			GBA.ScanlineCallback = callback;
			GBA.Callback_Scanline = sl;
		}

	};
}

#endif
