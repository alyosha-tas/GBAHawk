#ifndef CORE_H
#define CORE_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "GB_System.h"
#include "Memory.h"
#include "Mappers.h"
#include "PPUs.h"

// mapper list
#include "Mappers/Mapper_Camera.h"
#include "Mappers/Mapper_Default.h"
#include "Mappers/Mapper_HuC1.h"
#include "Mappers/Mapper_HuC3.h"
#include "Mappers/Mapper_MBC1.h"
#include "Mappers/Mapper_MBC1_Multi.h"
#include "Mappers/Mapper_MBC2.h"
#include "Mappers/Mapper_MBC3.h"
#include "Mappers/Mapper_MBC5.h"
#include "Mappers/Mapper_MBC6.h"
#include "Mappers/Mapper_MBC7.h"
#include "Mappers/Mapper_MMM01.h"
#include "Mappers/Mapper_RockMan8.h"
#include "Mappers/Mapper_Sachen_MMC1.h"
#include "Mappers/Mapper_Sachen_MMC2.h"
#include "Mappers/Mapper_TAMA5.h"
#include "Mappers/Mapper_WisdomTree.h"

#include "GB_PPU.h"
#include "GBC_PPU.h"
#include "GBC_GB_PPU.h"

using namespace std;

namespace GBHawk
{
	class GBCore
	{
	public:
		GBCore() 
		{
			Mapper = nullptr;
			PPU = nullptr;
			GB.MessageCallback = nullptr;
			GB.RumbleCallback = nullptr;
			GB.TraceCallback = nullptr;
			GB.System_Reset();
		};

		GB_System GB;

		Mappers* Mapper;
		PPUs* PPU;

		void Load_BIOS(uint8_t* bios, bool cgb_flag, bool cgb_gba_flag)
		{
			if (cgb_flag)
			{
				std::memcpy(GB.BIOS, bios, 0x900);
			}
			else
			{
				std::memcpy(GB.BIOS, bios, 0x100);
			}

			GB.Is_GBC = cgb_flag;

			GB.Is_GBC_GBA = cgb_flag && cgb_gba_flag;
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint32_t mapper)
		{
			std::memcpy(GB.ROM, ext_rom, ext_rom_size);

			GB.ROM_Length = ext_rom_size;

			if (mapper == 0)
			{
				Mapper = new Mapper_Default();
			}
			else if (mapper == 1)
			{
				Mapper = new Mapper_MBC1();
			}
			else if (mapper == 2)
			{
				Mapper = new Mapper_MBC2();
			}
			else if (mapper == 3)
			{
				Mapper = new Mapper_MBC3();
			}
			else if (mapper == 4)
			{
				Mapper = new Mapper_MBC1Multi();
			}
			else if (mapper == 5)
			{
				Mapper = new Mapper_MBC5();
			}
			else if (mapper == 6)
			{
				Mapper = new Mapper_MBC6();
			}
			else if (mapper == 7)
			{
				Mapper = new Mapper_MBC7();
			}
			else if (mapper == 8)
			{
				Mapper = new Mapper_Camera();
			}
			else if (mapper == 9)
			{
				Mapper = new Mapper_TAMA5();
			}
			else if (mapper == 10)
			{
				Mapper = new Mapper_HuC3();
			}
			else if (mapper == 11)
			{
				Mapper = new Mapper_HuC1();
			}
			else if (mapper == 12)
			{
				Mapper = new Mapper_Sachen1();
			}
			else if (mapper == 13)
			{
				Mapper = new Mapper_Sachen2();
			}
			else if (mapper == 14)
			{
				Mapper = new Mapper_WisdomTree();
			}
			else if (mapper == 15)
			{
				Mapper = new Mapper_RockMan8();
			}
			else if (mapper == 16)
			{
				Mapper = new Mapper_MMM01();
			}

			GB.mapper_pntr = &Mapper[0];

			// set up all the pointers
			Mapper->Core_Acc_X_State = &GB.Acc_X_state;

			Mapper->Core_Acc_Y_State = &GB.Acc_Y_state;

			Mapper->Core_Addr_Access = &GB.addr_access;

			Mapper->Core_ROM = &GB.ROM[0];

			Mapper->Core_ROM_Length = &GB.ROM_Length;

			Mapper->Core_Bus_Value = &GB.bus_value;

			Mapper->Core_Cycle_Count = &GB.Cycle_Count;

			Mapper->Core_Bus_Access_Time = &GB.bus_access_time;

			// only set RTC at initialization
			Mapper->Reset();

			Mapper->Core_Message_String = &GB.Message_String;
			Mapper->RumbleCallback = GB.RumbleCallback;
			Mapper->MessageCallback = GB.MessageCallback;

			// repeat for the ppu
			if (GB.Is_GBC)
			{
				if ((GB.ROM[0x43] != 0x80) && (GB.ROM[0x43] != 0xC0))
				{
					PPU = new GBC_GB_PPU();
				}
				else
				{
					PPU = new GBC_PPU();
				}
			}
			else
			{
				PPU = new GB_PPU();
			}

			GB.ppu_pntr = &PPU[0];

			PPU->sys_pntr = &GB;

			// set up all the pointers
			PPU->Core_cpu_FlagI = &GB.cpu_FlagI;

			PPU->Core_GBC_compat = &GB.GBC_Compat;

			PPU->Core_Double_Speed = &GB.Double_Speed;

			PPU->Core_CPU_Halted = &GB.cpu_Halted;

			PPU->Core_CPU_Stopped = &GB.cpu_Stopped;

			PPU->Core_HDMA_Transfer = &GB.HDMA_Transfer;

			PPU->Core_REG_FFFF = &GB.REG_FFFF;

			PPU->Core_REG_FF0F = &GB.REG_FF0F;

			PPU->Core_Bus_Value = &GB.bus_value;

			PPU->Core_VRAM = &GB.VRAM[0];

			PPU->Core_VRAM_Bank = &GB.VRAM_Bank;

			PPU->Core_OAM = &GB.OAM[0];

			PPU->ScanlineCallbackLine = &GB.ScanlineCallbackLine;

			PPU->Core_Clear_Counter = &GB.clear_counter;

			PPU->Core_Color_Palette = &GB.color_palette[0];

			PPU->Core_Video_Buffer = &GB.video_buffer[0];

			PPU->Core_Cycle_Count = &GB.Cycle_Count;

			PPU->Core_Instruction_Start = &GB.cpu_Instruction_Start;

			PPU->Core_Message_String = &GB.Message_String;

			PPU->OnVBlank = &GB_System::On_VBlank;

			PPU->Core_HDMA_Start_Stop = &GB_System::HDMA_start_stop;

			PPU->MessageCallback = GB.MessageCallback;

			PPU->ScanlineCallback = GB.ScanlineCallback;

			PPU->Core_ReadMemory = &GB_System::Read_Memory;

			PPU->Core_RegPC = &GB_System::cpu_RegPCget;

			PPU->Reset();

			// finally link core pointers
			GB.ppu_LY_pntr = &PPU->LY;
			GB.PPU_Pal_Change_Blocked = &PPU->pal_change_blocked;

			GB.PPU_Read_Regs = &PPUs::ReadReg;
			GB.PPU_Write_Regs = &PPUs::WriteReg;

			GB.Message_String = "Initialization Successful";

			GB.MessageCallback(GB.Message_String.length());
		}

		void Set_RTC(int32_t val, uint32_t param)
		{
			Mapper->Set_RTC(val, param);
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			GB.Cart_RAM = new uint8_t[ext_sram_size];

			GB.Cart_RAM_Length = ext_sram_size;

			std::memcpy(GB.Cart_RAM, ext_sram, ext_sram_size);

			Mapper->Core_Cart_RAM = &GB.Cart_RAM[0];

			Mapper->Core_Cart_RAM_Length = &GB.Cart_RAM_Length;
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			std::memcpy(GB.Cart_RAM, ext_sram, ext_sram_size);
		}

		void Set_Palette(bool palette)
		{
			if (palette)
			{
				GB.color_palette[0] = GB.color_palette_BW[0];
				GB.color_palette[1] = GB.color_palette_BW[1];
				GB.color_palette[2] = GB.color_palette_BW[2];
				GB.color_palette[3] = GB.color_palette_BW[3];
			}
			else
			{
				GB.color_palette[0] = GB.color_palette_Gr[0];
				GB.color_palette[1] = GB.color_palette_Gr[1];
				GB.color_palette[2] = GB.color_palette_Gr[2];
				GB.color_palette[3] = GB.color_palette_Gr[3];
			}
		}

		void Sync_Domain_VBL(bool on_vbl)
		{
			GB.Sync_Domains_VBL = on_vbl;
		}

		void Hard_Reset() 
		{
			Mapper->Reset();
			PPU->Reset();
			
			GB.System_Reset();
		}

		bool FrameAdvance(uint8_t controller_1, uint16_t accx, uint16_t accy, bool render, bool rendersound)
		{
			GB.New_Controller = controller_1;
			GB.New_Acc_X = accx;
			GB.New_Acc_Y = accy;

			GB.snd_Master_Clock = 0;

			GB.num_samples_L = 0;
			GB.num_samples_R = 0;

			GB.Is_Lag = true;

			GB.VBlank_Rise = false;

			GB.Frame_Advance();

			// if the game is halted but controller interrupts are on, check for interrupts
			// if the game is stopped, any button press will un-stop even if interrupts are off
			if ((GB.cpu_Stopped && !GB.controller_was_checked) || (GB.cpu_Halted && ((GB.REG_FFFF & 0x10) == 0x10)))
			{
				// update the controller state on VBlank
				GB.Get_Controller_State();

				GB.do_controller_check();
			}

			return GB.Is_Lag;
		}

		bool SubFrameAdvance(uint8_t controller_1, uint16_t accx, uint16_t accy,  bool render, bool rendersound, bool do_reset, uint32_t reset_cycle)
		{
			GB.New_Controller = controller_1;
			GB.New_Acc_X = accx;
			GB.New_Acc_Y = accy;

			GB.snd_Master_Clock = 0;

			GB.num_samples_L = 0;
			GB.num_samples_R = 0;

			GB.Is_Lag = true;

			GB.VBlank_Rise = false;

			bool reset_was_done = false;

			if (!do_reset) { reset_cycle = -1; }

			reset_was_done = GB.SubFrame_Advance(reset_cycle);

			if (reset_was_done)
			{
				Hard_Reset();
			}

			return GB.Is_Lag;
		}

		void GetVideo(uint32_t* dest) 
		{
			uint32_t* src = GB.frame_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof (uint32_t) * 160 * 144);

			// blank the screen
			for (int i = 0; i < 160 * 144; i++)
			{
				GB.frame_buffer[i] = 0xFFF8F8F8;
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
			saver = PPU->SaveState(saver);
		}

		void LoadState(uint8_t* loader)
		{
			loader = GB.LoadState(loader);
			loader = Mapper->LoadState(loader);
			loader = PPU->LoadState(loader);
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return GB.Peek_Memory(addr);
		}

		uint8_t GetVRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GB.VRAM_vbls[addr & 0x3FFF];
			}
			else
			{
				return GB.VRAM[addr & 0x3FFF];
			}
		}

		uint8_t GetRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GB.RAM_vbls[addr & 0x7FFF];
			}
			else
			{
				return GB.RAM[addr & 0x7FFF];
			}
		}

		uint8_t GetHRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GB.ZP_RAM_vbls[addr & 0x7F];
			}
			else
			{
				return GB.ZP_RAM[addr & 0x7F];
			}
		}

		uint8_t GetOAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				if (addr < 0xA0) { return GB.OAM_vbls[addr]; }
			}
			else
			{
				if (addr < 0xA0) { return GB.OAM[addr]; }
			}

			return 0;
		}

		uint8_t GetPALRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GB.PALRAM_vbls[addr & 0x3FF];
			}
			else
			{
				return GB.PALRAM[addr & 0x3FF];
			}
		}

		uint8_t GetRegisters(uint32_t addr)
		{
			return GB.Get_Registers_Internal(addr);
		}

		uint8_t GetSRAM(uint32_t addr, bool vbl_sync)
		{
			if (GB.Cart_RAM_Length != 0) 
			{
				if (vbl_sync)
				{
					return GB.Cart_RAM_vbls[addr & (GB.Cart_RAM_Length - 1)];
				}
				else
				{
					return GB.Cart_RAM[addr & (GB.Cart_RAM_Length - 1)];
				}
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
				std::memcpy(d, GB.UNS_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, GB.UNH_event, l);
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
						GB.PPU_IO[i] = GB.Read_Registers(i);
					}
					return GB.PPU_IO; break;
			}

			return nullptr;
		}

		void SetScanlineCallback(void (*callback)(uint8_t), int sl)
		{
			GB.ScanlineCallback = callback;
			GB.ScanlineCallbackLine = sl;
		}

	};
}

#endif
