#ifndef CORE_H
#define CORE_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>

#include "GBC_System.h"
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

#include "GBC_PPU.h"
#include "GBC_PPU.h"
#include "GBC_GB_PPU.h"

using namespace std;

namespace GBCHawk
{
	class GBCCore
	{
	public:
		GBCCore() 
		{
			Mapper = nullptr;
			PPU = nullptr;
			GBC.MessageCallback = nullptr;
			GBC.RumbleCallback = nullptr;
			GBC.TraceCallback = nullptr;
			GBC.System_Reset();
		};

		GBC_System GBC;

		Mappers* Mapper;
		PPUs* PPU;

		uint32_t Console_ID = 5;

		void Load_BIOS(uint8_t* bios, bool cgb_gba_flag)
		{
			std::memcpy(GBC.BIOS, bios, 0x900);

			GBC.Is_GBC_GBA = cgb_gba_flag;

			// set up IR register to off state
			GBC.IR_mask = 0;

			// Here we modify the BIOS if GBCA mode is set (credit to ExtraTricky)
			if (GBC.Is_GBC_GBA) { GBC.IR_mask = 2; }
		}

		void Load_ROM(uint8_t* ext_rom, uint32_t ext_rom_size, uint32_t mapper)
		{
			std::memcpy(GBC.ROM, ext_rom, ext_rom_size);

			GBC.ROM_Length = ext_rom_size;

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
				GBC.Use_MT = true;
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
				GBC.Use_MT = true;
			}
			else if (mapper == 10)
			{
				Mapper = new Mapper_HuC3();
				GBC.Use_MT = true;
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

			GBC.mapper_pntr = &Mapper[0];

			// set up all the pointers
			Mapper->Core_Acc_X_State = &GBC.Acc_X_state;

			Mapper->Core_Acc_Y_State = &GBC.Acc_Y_state;

			Mapper->Core_Addr_Access = &GBC.addr_access;

			Mapper->Core_ROM = &GBC.ROM[0];

			Mapper->Core_ROM_Length = &GBC.ROM_Length;

			Mapper->Core_Bus_Value = &GBC.bus_value;

			Mapper->Core_Cycle_Count = &GBC.Cycle_Count;

			Mapper->Core_Bus_Access_Time = &GBC.bus_access_time;

			// only set RTC at initialization
			Mapper->Reset();

			Mapper->Core_Message_String = &GBC.Message_String;
			Mapper->RumbleCallback = GBC.RumbleCallback;
			Mapper->MessageCallback = GBC.MessageCallback;

			// repeat for the ppu
			if ((GBC.ROM[0x143] != 0x80) && (GBC.ROM[0x143] != 0xC0))
			{
				PPU = new GBC_GB_PPU();
			}
			else
			{
				PPU = new GBC_PPU();
			}

			GBC.ppu_pntr = &PPU[0];

			PPU->sys_pntr = &GBC;

			// set up all the pointers
			PPU->Core_cpu_FlagI = &GBC.cpu_FlagI;

			PPU->Core_GBCC_compat = &GBC.GBC_Compat;

			PPU->Core_Double_Speed = &GBC.Double_Speed;

			PPU->Core_CPU_Halted = &GBC.cpu_Halted;

			PPU->Core_CPU_Stopped = &GBC.cpu_Stopped;

			PPU->Core_HDMA_Transfer = &GBC.HDMA_Transfer;

			PPU->Core_REG_FFFF = &GBC.REG_FFFF;

			PPU->Core_REG_FF0F = &GBC.REG_FF0F;

			PPU->Core_Bus_Value = &GBC.bus_value;

			PPU->Core_VRAM = &GBC.VRAM[0];

			PPU->Core_VRAM_Bank = &GBC.VRAM_Bank;

			PPU->Core_OAM = &GBC.OAM[0];

			PPU->Core_Clear_Counter = &GBC.clear_counter;

			PPU->Core_Video_Buffer = &GBC.video_buffer[0];

			PPU->Core_Cycle_Count = &GBC.Cycle_Count;

			PPU->Core_Instruction_Start = &GBC.cpu_Instruction_Start;

			PPU->Core_Message_String = &GBC.Message_String;

			PPU->OnVBlank = &GBC_System::On_VBlank;

			PPU->Core_HDMA_Start_Stop = &GBC_System::HDMA_start_stop;

			PPU->MessageCallback = GBC.MessageCallback;

			PPU->Core_DMA_OAM_Access = &GBC.dma_OAM_Access;

			PPU->Core_ReadMemory = &GBC_System::Read_Memory;

			PPU->Core_RegPC = &GBC_System::cpu_RegPCget;

			PPU->Reset();

			// finally link core pointers
			GBC.ppu_LY_pntr = &PPU->LY;
			GBC.PPU_Pal_Change_Blocked = &PPU->pal_change_blocked;

			GBC.PPU_Read_Regs = &PPUs::ReadReg;
			GBC.PPU_Write_Regs = &PPUs::WriteReg;

			if (GBC.MessageCallback)
			{
				GBC.Message_String = "Initialization Successful";

				GBC.MessageCallback(GBC.Message_String.length());
			}

			GBC.System_Reset();
		}

		void Set_RTC(int32_t val, uint32_t param)
		{
			Mapper->RTC_Set(val, param);
		}

		void Create_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			if (ext_sram_size > 0)
			{
				GBC.Cart_RAM = new uint8_t[ext_sram_size];

				GBC.Cart_RAM_Length = ext_sram_size;

				std::memcpy(GBC.Cart_RAM, ext_sram, ext_sram_size);

				Mapper->Core_Cart_RAM = &GBC.Cart_RAM[0];

				Mapper->Core_Cart_RAM_Length = &GBC.Cart_RAM_Length;
			}

			Mapper->Reset();
		}

		void Load_SRAM(uint8_t* ext_sram, uint32_t ext_sram_size)
		{
			std::memcpy(GBC.Cart_RAM, ext_sram, ext_sram_size);
		}

		void Sync_Domain_VBL(bool on_vbl)
		{
			GBC.Sync_Domains_VBL = on_vbl;
		}

		void Hard_Reset() 
		{
			Mapper->Reset();
			PPU->Reset();
			
			GBC.System_Reset();
		}

		bool FrameAdvance(uint8_t controller_1, uint16_t accx, uint16_t accy, bool render, bool rendersound)
		{
			GBC.New_Controller = controller_1;
			GBC.New_Acc_X = accx;
			GBC.New_Acc_Y = accy;

			GBC.snd_Master_Clock = 0;

			GBC.num_samples_L = 0;
			GBC.num_samples_R = 0;

			GBC.Is_Lag = true;

			GBC.VBlank_Rise = false;

			GBC.Frame_Advance();

			// if the game is halted but controller interrupts are on, check for interrupts
			// if the game is stopped, any button press will un-stop even if interrupts are off
			if ((GBC.cpu_Stopped && !GBC.controller_was_checked) || (GBC.cpu_Halted && ((GBC.REG_FFFF & 0x10) == 0x10)))
			{
				// update the controller state on VBlank
				GBC.Get_Controller_State();

				GBC.do_controller_check();
			}

			return GBC.Is_Lag;
		}

		bool SubFrameAdvance(uint8_t controller_1, uint16_t accx, uint16_t accy,  bool render, bool rendersound, bool do_reset, uint32_t input_cycle)
		{
			GBC.New_Controller = controller_1;
			GBC.New_Acc_X = accx;
			GBC.New_Acc_Y = accy;

			GBC.snd_Master_Clock = 0;

			GBC.num_samples_L = 0;
			GBC.num_samples_R = 0;

			GBC.VBlank_Rise = false;

			bool frame_was_passed = false;

			frame_was_passed = GBC.SubFrame_Advance(input_cycle);

			if (frame_was_passed)
			{
				// turn off the screen so the image doesnt persist
				// but don't turn off blank_frame yet, it still needs to be true until the next VBL
				// GBCC games need to clear slow enough that games that turn the screen off briefly for cinematics still look smooth
				if (GBC.ppu_pntr->clear_screen)
				{
					GBC.clear_screen_func();
				}
				
				GBC.Frame_Cycle = 0;
			}
			
			if (!frame_was_passed || (input_cycle == 70223))
			{
				// controller input is always applied here
				GBC.Get_Controller_State();
				GBC.do_controller_check();

				if (do_reset) { Hard_Reset(); }
			}
			
			return frame_was_passed;
		}

		void GetVideo(uint32_t* dest) 
		{
			uint32_t* src = GBC.frame_buffer;
			uint32_t* dst = dest;

			std::memcpy(dst, src, sizeof (uint32_t) * 160 * 144);

			// blank the screen
			for (int i = 0; i < 160 * 144; i++)
			{
				GBC.frame_buffer[i] = 0xFFF8F8F8;
			}
		}

		uint32_t GetAudio(int32_t* dest_L, int32_t* n_samp_L, int32_t* dest_R, int32_t* n_samp_R)
		{
			int32_t* src = GBC.samples_L;
			int32_t* dst = dest_L;

			std::memcpy(dst, src, sizeof(int32_t) * GBC.num_samples_L * 2);
			n_samp_L[0] = GBC.num_samples_L;

			src = GBC.samples_R;
			dst = dest_R;

			std::memcpy(dst, src, sizeof(int32_t) * GBC.num_samples_R * 2);
			n_samp_R[0] = GBC.num_samples_R;

			uint32_t temp_int = GBC.snd_Master_Clock;
			
			return temp_int;
		}

		uint64_t GetCycles(bool cycle_type)
		{
			if (cycle_type)
			{
				return GBC.Cycle_Count;
			}
			else
			{
				return GBC.Single_Cycle_Count;
			}
		}

		#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{	
			saver = GBC.SaveState(saver);
			saver = Mapper->SaveState(saver);
			saver = PPU->SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = GBC.LoadState(loader);
			loader = Mapper->LoadState(loader);
			loader = PPU->LoadState(loader);

			return loader;
		}

		#pragma endregion

		#pragma region Memory Domain Functions

		uint8_t GetSysBus(uint32_t addr)
		{
			return GBC.Peek_Memory(addr);
		}

		uint8_t GetVRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GBC.VRAM_vbls[addr & 0x3FFF];
			}
			else
			{
				return GBC.VRAM[addr & 0x3FFF];
			}
		}

		uint8_t GetRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GBC.RAM_vbls[addr & 0x7FFF];
			}
			else
			{
				return GBC.RAM[addr & 0x7FFF];
			}
		}

		uint8_t GetHRAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				return GBC.ZP_RAM_vbls[addr & 0x7F];
			}
			else
			{
				return GBC.ZP_RAM[addr & 0x7F];
			}
		}

		uint8_t GetOAM(uint32_t addr, bool vbl_sync)
		{
			if (vbl_sync)
			{
				if (addr < 0xA0) { return GBC.OAM_vbls[addr]; }
			}
			else
			{
				if (addr < 0xA0) { return GBC.OAM[addr]; }
			}

			return 0;
		}

		uint8_t GetRegisters(uint32_t addr)
		{
			return GBC.Get_Registers_Internal(addr);
		}

		uint8_t GetSRAM(uint32_t addr, bool vbl_sync)
		{
			if (GBC.Cart_RAM_Length != 0) 
			{
				if (vbl_sync)
				{
					return GBC.Cart_RAM_vbls[addr & (GBC.Cart_RAM_Length - 1)];
				}
				else
				{
					return GBC.Cart_RAM[addr & (GBC.Cart_RAM_Length - 1)];
				}
			}
			
			return 0;
		}

		#pragma endregion

		#pragma region Tracer

		void SetTraceCallback(void (*callback)(int))
		{
			GBC.TraceCallback = callback;
		}

		void SetRumbleCallback(void (*callback)(bool))
		{
			GBC.RumbleCallback = callback;
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
			std::memcpy(h, GBC.TraceHeader, l);
		}

		// the copy length l must be supplied ahead of time from GetRegStrngLength
		void GetRegisterState(char* r, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(r, GBC.CPURegisterState().c_str(), l);
			}
			else if (t < 5)
			{
				std::memcpy(r, GBC.No_Reg, l);
			}
			else
			{
				// DMA info
				std::memcpy(r, GBC.CPUDMAState().c_str(), l);
			}
		}

		// the copy length l must be supplied ahead of time from GetDisasmLength
		void GetDisassembly(char* d, int t, int l)
		{
			if (t == 0)
			{
				std::memcpy(d, GBC.CPUDisassembly().c_str(), l);
			}
			else if (t == 1)
			{
				std::memcpy(d, GBC.UNS_event, l);
			}
			else if (t == 2)
			{
				std::memcpy(d, GBC.UNH_event, l);
			}
			else if (t == 3)
			{
				std::memcpy(d, GBC.IRQ_event, l);
			}
			else if (t == 4)
			{
				std::memcpy(d, GBC.HALT_event, l);
			}
			else
			{
				std::memcpy(d, GBC.DMA_event, l);
			}
		}

		#pragma endregion

		void SetMessageCallback(void (*callback)(int))
		{
			GBC.MessageCallback = callback;
		}

		void GetMessage(char* d)
		{
			std::memcpy(d, GBC.Message_String.c_str(), GBC.Message_String.length() + 1);
		}

		void* Get_PPU_Pointers(int sel)
		{
			switch (sel)
			{
				case 0: return PPU->BG_palette; break;
				case 1: return PPU->OBJ_palette; break;
				case 2: return GBC.OAM; break;
				case 3: return GBC.VRAM; break;
			}

			return nullptr;
		}

		void SetScanlineCallback(void (*callback)(uint8_t), int sl)
		{
			PPU->ScanlineCallback = callback;

			PPU->ScanlineCallbackLine = sl;
		}

		void ExecuteScanlineCallback()
		{
			if (PPU->ScanlineCallback)
			{
				PPU->ScanlineCallback(PPU->LCDC);
			}
		}
	};
}

#endif
