#ifndef CORE_H
#define CORE_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "GB_System.h"
#include "Memory.h"
#include "Mappers.h"

using namespace std;

namespace GBHawk
{
	class GBCore
	{
	public:
		GBCore() 
		{
			Mapper = nullptr;
			GB.MessageCallback = nullptr;
			GB.RumbleCallback = nullptr;
			GB.TraceCallback = nullptr;
			GB.System_Reset();
		};

		GB_System GB;
		Mappers* Mapper;

		void Load_BIOS(uint8_t* bios)
		{
			std::memcpy(GB.BIOS, bios, 0x4000);
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint32_t mapper, uint64_t datetime, bool rtc_functional, int16_t EEPROM_offset, uint16_t flash_type_64_value,
					  uint16_t flash_type_128_value, int16_t flash_write_offset, int32_t flash_sector_offset, int32_t flash_chip_offset, bool is_GBP)
		{
			std::memcpy(GB.ROM, ext_rom, 0x6000000);

			if ((mapper == 0) || (mapper == 1))
			{
				GB.Cart_RAM_Present = false;
				GB.Is_EEPROM = false;
			}
			else if ((mapper == 2) || (mapper == 3))
			{
				GB.Cart_RAM_Present = true;
				GB.Is_EEPROM = false;
			}
			else if (mapper == 4)
			{
				GB.Cart_RAM_Present = true;
				GB.Is_EEPROM = true;

				if (ext_rom_size <= 0x1000000)
				{
					GB.EEPROM_Wiring = true;
				}
				else
				{
					GB.EEPROM_Wiring = false;
				}
			}
			else if (mapper == 5)
			{
				GB.Cart_RAM_Present = true;
				GB.Is_EEPROM = true;

				if (ext_rom_size <= 0x1000000)
				{
					GB.EEPROM_Wiring = true;
				}
				else
				{
					GB.EEPROM_Wiring = false;
				}
			}
			else if (mapper == 6)
			{
				GB.Cart_RAM_Present = true;
				GB.Is_EEPROM = true;

				if (ext_rom_size <= 0x1000000)
				{
					GB.EEPROM_Wiring = true;
				}
				else
				{
					GB.EEPROM_Wiring = false;
				}
			}
			else if ((mapper == 7) || (mapper == 8))
			{
				GB.Cart_RAM_Present = true;
				GB.Is_EEPROM = false;
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
				if (flash_type_64_value == 0x3D1F) { Mapper = new Mapper_FLASH_Atmel(); }
				else { Mapper = new Mapper_FLASH(); }
			}
			else if (mapper == 8)
			{
				Mapper = new Mapper_FLASH_RTC();
			}

			GB.mapper_pntr = &Mapper[0];

			Mapper->Core_Cycle_Count = &GB.CycleCount;

			Mapper->Core_Clock_Update_Cycle = &GB.Clock_Update_Cycle;

			Mapper->Core_Acc_X = &GB.New_Acc_X;

			Mapper->Core_Acc_Y = &GB.New_Acc_Y;

			Mapper->Core_Solar = &GB.New_Solar;

			Mapper->Core_ROM = &GB.ROM[0];

			Mapper->ROM_C4 = GB.ROM[0xC4];
			Mapper->ROM_C5 = GB.ROM[0xC5];
			Mapper->ROM_C6 = GB.ROM[0xC6];
			Mapper->ROM_C7 = GB.ROM[0xC7];
			Mapper->ROM_C8 = GB.ROM[0xC8];
			Mapper->ROM_C9 = GB.ROM[0xC9];

			Mapper->Current_C4 = GB.ROM[0xC4];
			Mapper->Current_C5 = GB.ROM[0xC5];
			Mapper->Current_C6 = GB.ROM[0xC6];
			Mapper->Current_C7 = GB.ROM[0xC7];
			Mapper->Current_C8 = GB.ROM[0xC8];
			Mapper->Current_C9 = GB.ROM[0xC9];

			// only reset RTC on initial power on
			Mapper->Reset_RTC = true;
			Mapper->Reset();
			Mapper->Reset_RTC = false;

			Mapper->Flash_Type_64_Value = flash_type_64_value;
			Mapper->Flash_Type_128_Value = flash_type_128_value;

			Mapper->RTC_Functional = rtc_functional;

			Mapper->Solar_Functional = mapper == 6;

			Mapper->Reg_Second = (uint8_t)datetime;
			Mapper->Reg_Minute = (uint8_t)(datetime >> 8);
			Mapper->Reg_Hour = (uint8_t)(datetime >> 16);
			Mapper->Reg_Week = (uint8_t)(datetime >> 24);
			Mapper->Reg_Day = (uint8_t)(datetime >> 32);
			Mapper->Reg_Month = (uint8_t)(datetime >> 40);
			Mapper->Reg_Year = (uint8_t)(datetime >> 48);
			Mapper->CTRL_Reg = (uint8_t)(datetime >> 56);

			Mapper->RTC_24_Hour = (Mapper->CTRL_Reg & 0x40) == 0x40;

			Mapper->EEPROM_Offset = EEPROM_offset;
			Mapper->Flash_Write_Offset = flash_write_offset;
			Mapper->Flash_Sector_Erase_Offset = flash_sector_offset;
			Mapper->Flash_Chip_Erase_Offset = flash_chip_offset;

			Mapper->Core_Message_String = &GB.Message_String;
			Mapper->RumbleCallback = GB.RumbleCallback;
			Mapper->MessageCallback = GB.MessageCallback;

			// Only reset cycle count on initial power on, not power cycles
			GB.CycleCount = 0;
			GB.Clock_Update_Cycle = 0;

			GB.Is_GBP = is_GBP;
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			GB.Cart_RAM = new uint8_t[ext_sram_size];

			GB.Cart_RAM_Length = ext_sram_size;

			std::memcpy(GB.Cart_RAM, ext_sram, ext_sram_size);

			Mapper->Cart_RAM = &GB.Cart_RAM[0];

			Mapper->Size_Mask = ext_sram_size - 1;
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			std::memcpy(GB.Cart_RAM, ext_sram, ext_sram_size);
		}

		void Hard_Reset() 
		{
			Mapper->Reset();
			
			GB.System_Reset();
		}

		void Set_GBP_Enable()
		{
			GB.GBP_Mode_Enabled = true;
		}

		bool FrameAdvance(uint16_t controller_1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool rendersound)
		{
			GB.New_Controller = controller_1;
			GB.New_Acc_X = accx;
			GB.New_Acc_Y = accy;
			GB.New_Solar = solar;

			// update the controller state
			GB.controller_state_old = GB.controller_state;
			GB.controller_state = GB.New_Controller;

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (GB.VBlank_Rise || GB.stopped)
			{
				// check if controller state caused interrupt
				GB.do_controller_check(false);
			}

			GB.snd_Master_Clock = 0;

			GB.num_samples_L = 0;
			GB.num_samples_R = 0;

			GB.Is_Lag = true;

			GB.VBlank_Rise = false;

			if (!GB.stopped)
			{
				GB.Frame_Advance();
			}
			else
			{
				if ((GB.INT_EN & GB.INT_Flags_Gather & 0x1000) == 0x1000)
				{
					GB.stopped = false;
				}
			}

			return GB.Is_Lag;
		}

		bool SubFrameAdvance(uint16_t controller_1, uint16_t accx, uint16_t accy, uint8_t solar, bool render, bool rendersound, bool do_reset, uint32_t reset_cycle)
		{
			GB.New_Controller = controller_1;
			GB.New_Acc_X = accx;
			GB.New_Acc_Y = accy;
			GB.New_Solar = solar;

			// update the controller state
			GB.controller_state_old = GB.controller_state;
			GB.controller_state = GB.New_Controller;

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (GB.VBlank_Rise || GB.stopped)
			{
				// check if controller state caused interrupt
				GB.do_controller_check(false);
			}

			GB.snd_Master_Clock = 0;

			GB.num_samples_L = 0;
			GB.num_samples_R = 0;

			GB.Is_Lag = true;

			GB.VBlank_Rise = false;

			bool reset_was_done = false;

			if (!do_reset) { reset_cycle = -1; }

			if (!GB.stopped)
			{
				reset_was_done = GB.SubFrame_Advance(reset_cycle);
			}
			else
			{
				if ((GB.INT_EN & GB.INT_Flags & 0x1000) == 0x1000)
				{
					GB.stopped = false;
				}
			}

			if (reset_was_done)
			{
				Hard_Reset();
			}

			return GB.Is_Lag;
		}

		void GetVideo(uint32_t* dest) 
		{
			uint32_t* src = GB.video_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof (uint32_t) * 240 * 160);

			// blank the screen
			for (int i = 0; i < 240 * 160; i++)
			{
				GB.video_buffer[i] = 0xFFF8F8F8;
			}
		}

		uint32_t GetAudio(int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
		{
			int32_t* src = GB.samples_L;
			int32_t* dst = dest_L;

			std::memcpy(dst, src, sizeof int32_t * GB.num_samples_L * 2);
			n_samp_L[0] = GB.num_samples_L;

			src = GB.samples_R;
			dst = dest_R;

			std::memcpy(dst, src, sizeof int32_t * GB.num_samples_R * 2);
			n_samp_R[0] = GB.num_samples_R;

			uint32_t temp_int = GB.snd_Master_Clock;
			
			return temp_int;
		}

		#pragma region State Save / Load

		void SaveState(uint8_t* saver)
		{	
			saver = GB.SaveState(saver);
			saver = Mapper->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = GB.LoadState(loader);
			loader = Mapper->LoadState(loader);
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return GB.Peek_Memory_8(addr);
		}

		uint8_t GetVRAM(uint32_t addr) 
		{
			if (addr < 0x18000)
			{
				return GB.VRAM[addr];
			}

			return GB.VRAM[(addr & 0x7FFF) | 0x10000];
		}

		uint8_t GetWRAM(uint32_t addr)
		{
			return GB.WRAM[addr & 0x3FFFF];
		}

		uint8_t GetIWRAM(uint32_t addr)
		{
			return GB.IWRAM[addr & 0x7FFF];
		}

		uint8_t GetOAM(uint32_t addr)
		{
			return GB.OAM[addr & 0x3FF];
		}

		uint8_t GetPALRAM(uint32_t addr)
		{
			return GB.PALRAM[addr & 0x3FF];
		}

		uint8_t GetRegisters(uint32_t addr)
		{
			return GB.Get_Registers_Internal(addr);
		}

		uint8_t GetSRAM(uint32_t addr)
		{
			if (GB.Cart_RAM_Length != 0) 
			{
				return GB.Cart_RAM[addr & (GB.Cart_RAM_Length - 1)];
			}
			
			return 0;
		}

		#pragma endregion

		#pragma region Tracer

		void SetTraceCallback(void (*callback)(int))
		{
			GB.TraceCallback = callback;
		}

		void SetRumbleCallback(void (*callback)(bool))
		{
			GB.RumbleCallback = callback;
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
			std::memcpy(h, GB.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(r, GB.CPURegisterState().c_str(), l);
			}
			else if (t < 5)
			{
				std::memcpy(r, GB.No_Reg, l);
			}
			else
			{
				// DMA info
				std::memcpy(r, GB.CPUDMAState().c_str(), l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(d, GB.CPUDisassembly().c_str(), l);
			}
			else if (t == 1)
			{
				std::memcpy(d, GB.SWI_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, GB.UDF_event, l);
			}
			else if (t == 3)
			{
				std::memcpy(d, GB.IRQ_event, l);
			}
			else if (t == 4)
			{
				std::memcpy(d, GB.HALT_event, l);
			}
			else
			{
				std::memcpy(d, GB.DMA_event, l);
			}
		}

		#pragma endregion

		void SetMessageCallback(void (*callback)(int))
		{
			GB.MessageCallback = callback;
		}

		void GetMessage(char* d)
		{
			std::memcpy(d, GB.Message_String.c_str(), GB.Message_String.length() + 1);
		}

		uint8_t* Get_PPU_Pointers(int sel)
		{
			switch (sel)
			{
				case 0: return GB.VRAM; break;
				case 1: return GB.OAM; break;
				case 2: return GB.PALRAM; break;
				case 3:
					for (int i = 0; i < 0x60; i++)
					{
						GB.PPU_IO[i] = GB.ppu_Read_Reg_8(i);
					}
					return GB.PPU_IO; break;
			}

			return nullptr;
		}

		void SetScanlineCallback(void (*callback)(void), int sl)
		{
			GB.ScanlineCallback = callback;
			GB.Callback_Scanline = sl;
		}

	};
}

#endif
