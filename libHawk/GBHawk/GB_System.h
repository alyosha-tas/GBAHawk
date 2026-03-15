#ifndef GB_System_H
#define GB_System_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>
#include <inttypes.h>
#include <cmath>

#ifndef _WIN32
#define sprintf_s snprintf
#endif

using namespace std;

/*
 * VRAM is arranged as:
 * 0x1800 Tiles
 * 0x400 BG Map 1
 * 0x400 BG Map 2
 * 0x1800 Tiles
 * 0x400 CA Map 1
 * 0x400 CA Map 2
 * Only the top set is available in GB (i.e. VRAM_Bank = 0)
 */

// TODO:
/* 
* The check when read on opcode 0xE8 ADD SP is happening
* 
* 
*/

//Message_String = "Complete: " + to_string(ser_GBP_Transfer_Count) + " " + to_string(ser_Data_0 & 0xFF) + " " + to_string(CycleCount);

//MessageCallback(Message_String.length());

namespace GBHawk
{
	class Mappers;
	class PPUs;

	class GB_System
	{
	public:
		
		Mappers* mapper_pntr = nullptr;
		PPUs* ppu_pntr = nullptr;

		uint8_t* Cart_RAM = nullptr;
		uint8_t* Cart_RAM_vbls = nullptr;
		uint32_t Cart_RAM_Length = 0;
		string Message_String = "";

		void (*MessageCallback)(int);
		void (*ScanlineCallback)(uint8_t);

		int ScanlineCallbackLine = 0;

		// external pointers and functions
		uint8_t* ppu_LY_pntr;
		bool* PPU_Pal_Change_Blocked = nullptr;

		uint8_t (PPUs::*PPU_Read_Regs)(uint16_t addr);
		void (PPUs::*PPU_Write_Regs)(uint16_t addr, uint8_t value);

		const uint32_t color_palette_BW[4] = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };
		const uint32_t color_palette_Gr[4] = { 0xFFA4C505, 0xFF88A905, 0xFF1D551D, 0xFF052505 };
		uint32_t color_palette[4] = { };

	# pragma region General System and Prefetch

		uint32_t video_buffer[160 * 144] = { };
		uint32_t frame_buffer[160 * 144] = { };

		void Frame_Advance();
		bool SubFrame_Advance(uint32_t reset_cycle);
		inline void Single_Step();

		uint8_t Read_Memory(uint16_t addr);

		void Write_Memory(uint16_t addr, uint8_t value);

		uint8_t Peek_Memory(uint16_t addr);
		int8_t Peek_Memory_Signed(uint16_t addr);
		uint16_t Peek_Memory_16(uint16_t addr);

		// General Variables
		bool Is_Lag;
		bool VBlank_Rise;
		bool GBC_Compat;

		uint8_t ext_num = 0; // zero here means disconnected

		// this register controls whether or not the GB BIOS is mapped into memory
		uint8_t GB_bios_register;

		uint8_t input_register;

		// The unused bits in this register are still read/writable
		uint8_t REG_FFFF;
		// The unused bits in this register (interrupt flags) are always set
		uint8_t REG_FF0F = 0xE0;
		// Updating reg FF0F seems to be delayed by one cycle,needs more testing
		uint8_t REG_FF0F_OLD = 0xE0;
		
		// several undocumented GBC Registers
		uint8_t undoc_6C, undoc_72, undoc_73, undoc_74, undoc_75, undoc_76, undoc_77;

		// other system state
		bool controller_was_checked;
		bool delays_to_process;
		bool DIV_falling_edge, DIV_edge_old;
		bool Double_Speed;
		bool speed_switch;
		bool HDMA_Transfer; // stalls CPU when in progress
		bool In_Vblank_old;
		bool Sync_Domains_VBL;

		uint8_t bus_value; // we need the last value on the bus for proper emulation of blocked SRAM
		uint8_t VRAM_Bank;
		uint8_t IR_reg, IR_mask, IR_signal, IR_receive, IR_self;
		uint8_t controller_state;
		uint8_t multi_core_controller_byte;
		uint8_t New_Controller;

		uint16_t New_Acc_X, New_Acc_Y;
		uint16_t addr_access;
		uint16_t Acc_X_state;
		uint16_t Acc_Y_state;

		uint32_t IR_write;
		uint32_t RAM_Bank;
		uint32_t RAM_Bank_ret;
		uint32_t controller_delay_cd;
		uint32_t clear_counter;

		uint64_t bus_access_time; // also need to keep track of the time of the access since it doesn't last very long
		uint64_t Cycle_Count;
		uint64_t Frame_Cycle;

		uint8_t RAM[0x8000] = { }; // only 0x2000 available to GB
		uint8_t ZP_RAM[0x80] = { };
		uint8_t PALRAM[0x400] = { };
		uint8_t VRAM[0x4000] = { };
		uint8_t OAM[0xA0] = { };

		uint8_t RAM_vbls[0x8000] = { };
		uint8_t ZP_RAM_vbls[0x80] = { };
		uint8_t PALRAM_vbls[0x400] = { };
		uint8_t VRAM_vbls[0x4000] = { };
		uint8_t OAM_vbls[0xA0] = { };

		// not stated, controlled on system load
		uint8_t BIOS[0x900] = { };
		uint8_t ROM[0x2000000] = { };

		uint32_t ROM_Length;

		bool is_linked_system = false;
		bool Is_GBC = false;
		bool Is_GBC_GBA = false;
		bool Use_MT = false;

		// only used by ppu viewer
		uint8_t PPU_IO[0x60] = { };

		GB_System()
		{
			System_Reset();
		}

		void System_Reset() 
		{
			controller_was_checked = false;
			DIV_falling_edge = DIV_edge_old = false;
			speed_switch = false;
			HDMA_Transfer = false;
			In_Vblank_old = true;

			bus_value = 0;
			IR_reg = IR_mask = IR_signal = IR_receive = IR_self = 0;
			controller_state = 0;
			multi_core_controller_byte = 0;

			addr_access = 0;
			Acc_X_state = 0;
			Acc_Y_state = 0;

			IR_write = 0;

			GB_bios_register = 0; // bios enable
			GBC_Compat = true;
			Double_Speed = false;
			VRAM_Bank = 0;
			RAM_Bank = 1; // RAM bank always starts as 1 (even writing zero still sets 1)
			RAM_Bank_ret = 0; // return value can still be zero even though the bank itself cannot be
			delays_to_process = false;
			controller_delay_cd = 0;
			clear_counter = 0;

			Cycle_Count = 0;
			Frame_Cycle = 0;

			input_register = 0xCF; // not reading any input

			REG_FFFF = 0;
			REG_FF0F = 0xE0;
			REG_FF0F_OLD = 0xE0;

			//undocumented registers
			undoc_6C = 0xFE;
			undoc_72 = 0;
			undoc_73 = 0;
			undoc_74 = 0;
			undoc_75 = 0x8F;
			undoc_76 = 0;
			undoc_77 = 0;

			snd_Reset();
			ser_Reset();
			tim_Reset();
			cpu_Reset();

			uint32_t startup_color = 0xFFF8F8F8;

			for (int i = 0; i < 160*144; i++)
			{
				video_buffer[i] = startup_color;
			}

			for (int i = 0; i < 0x80; i++)
			{
				ZP_RAM[i] = 0;
			}

			if (Is_GBC_GBA)
			{
				// on GBA, initial RAM is mostly random, choosing 0 allows for stable clear and hotswap for games that encounter
				// uninitialized RAM
				for (int i = 0; i < 0x8000; i++)
				{
					RAM[i] = 0;//GBA_Init_RAM[i];
				}
			}
			else
			{
				for (int i = 0; i < 0x800; i++)
				{
					if ((i & 0xF) < 8)
					{
						RAM[i] = 0xFF;
						RAM[i + 0x1000] = 0xFF;
						RAM[i + 0x2000] = 0xFF;
						RAM[i + 0x3000] = 0xFF;
						RAM[i + 0x4000] = 0xFF;
						RAM[i + 0x5000] = 0xFF;
						RAM[i + 0x6000] = 0xFF;
						RAM[i + 0x7000] = 0xFF;

						RAM[i + 0x800] = 0;
						RAM[i + 0x1800] = 0;
						RAM[i + 0x2800] = 0;
						RAM[i + 0x3800] = 0;
						RAM[i + 0x4800] = 0;
						RAM[i + 0x5800] = 0;
						RAM[i + 0x6800] = 0;
						RAM[i + 0x7800] = 0;
					}
					else
					{
						RAM[i] = 0;
						RAM[i + 0x1000] = 0;
						RAM[i + 0x2000] = 0;
						RAM[i + 0x3000] = 0;
						RAM[i + 0x4000] = 0;
						RAM[i + 0x5000] = 0;
						RAM[i + 0x6000] = 0;
						RAM[i + 0x7000] = 0;

						RAM[i + 0x800] = 0xFF;
						RAM[i + 0x1800] = 0xFF;
						RAM[i + 0x2800] = 0xFF;
						RAM[i + 0x3800] = 0xFF;
						RAM[i + 0x4800] = 0xFF;
						RAM[i + 0x5800] = 0xFF;
						RAM[i + 0x6800] = 0xFF;
						RAM[i + 0x7800] = 0xFF;
					}
				}

				// some bytes are like this is Gambatte, hardware anomoly? Is it consistent across versions?
				/*
				for (int i = 0; i < 16; i++)
				{
					RAM[0xE02 + (16 * i)] = 0;
					RAM[0xE0A + (16 * i)] = 0xFF;

					RAM[0x1E02 + (16 * i)] = 0;
					RAM[0x1E0A + (16 * i)] = 0xFF;

					RAM[0x2E02 + (16 * i)] = 0;
					RAM[0x2E0A + (16 * i)] = 0xFF;

					RAM[0x3E02 + (16 * i)] = 0;
					RAM[0x3E0A + (16 * i)] = 0xFF;

					RAM[0x4E02 + (16 * i)] = 0;
					RAM[0x4E0A + (16 * i)] = 0xFF;

					RAM[0x5E02 + (16 * i)] = 0;
					RAM[0x5E0A + (16 * i)] = 0xFF;

					RAM[0x6E02 + (16 * i)] = 0;
					RAM[0x6E0A + (16 * i)] = 0xFF;

					RAM[0x7E02 + (16 * i)] = 0;
					RAM[0x7E0A + (16 * i)] = 0xFF;
				}
				*/
			}
		}

		void Sync_Domains_VBL_Func()
		{
			for (int j = 0; j < 0x8000; j++) { RAM_vbls[j] = RAM[j]; }
			for (int j = 0; j < 0x4000; j++) { VRAM_vbls[j] = VRAM[j]; }
			for (int j = 0; j < 0x400; j++) { PALRAM_vbls[j] = PALRAM[j]; }
			for (int j = 0; j < 0x80; j++) { ZP_RAM_vbls[j] = ZP_RAM[j]; }
			for (int j = 0; j < 0xA0; j++) { OAM_vbls[j] = OAM[j]; }

			if (Cart_RAM_Length != 0)
			{
				for (int j = 0; j < Cart_RAM_Length != 0; j++) { Cart_RAM_vbls[j] = Cart_RAM[j]; }
			}
		}

		void Get_Controller_State()
		{
			controller_state = New_Controller;
			Acc_X_state = New_Acc_X;
			Acc_Y_state = New_Acc_Y;
		}

		void do_controller_check()
		{
			// check if new input changed the input register and triggered IRQ
			uint8_t contr_prev = input_register;

			input_register &= 0xF0;
			if ((input_register & 0x30) == 0x20)
			{
				input_register |= (uint8_t)(controller_state & 0xF);
			}
			else if ((input_register & 0x30) == 0x10)
			{
				input_register |= (uint8_t)((controller_state & 0xF0) >> 4);
			}
			else if ((input_register & 0x30) == 0x00)
			{
				// if both polls are set, then a bit is zero if either or both pins are zero
				uint8_t temp = (uint8_t)((controller_state & 0xF) & ((controller_state & 0xF0) >> 4));
				input_register |= temp;
			}
			else
			{
				input_register |= 0xF;
			}

			// check for interrupts			
			if (((contr_prev & 8) > 0) && ((input_register & 8) == 0) ||
				((contr_prev & 4) > 0) && ((input_register & 4) == 0) ||
				((contr_prev & 2) > 0) && ((input_register & 2) == 0) ||
				((contr_prev & 1) > 0) && ((input_register & 1) == 0))
			{
				if ((REG_FFFF& 0x10) == 0x10) { cpu_FlagI = true; }
				REG_FF0F |= 0x10;
			}
		}

		void HDMA_start_stop(bool hdma_start)
		{
			// put the cpu into a wait state when HDMA starts
			// restore it when HDMA ends
			HDMA_Transfer = hdma_start;

			if (hdma_start)
			{
				cpu_State_Hold = cpu_Instr_Type;
				cpu_Instr_Type = OpT::WAIT;

				cpu_Instr_Cycle_Hold = cpu_Instr_Cycle;
			}
			else
			{
				cpu_Instr_Type = cpu_State_Hold;
				cpu_Instr_Cycle = cpu_Instr_Cycle_Hold;
			}
		}

		void process_delays()
		{
			// triggering an interrupt with a write to the control register takes 4 cycles to trigger interrupt
			controller_delay_cd--;
			if (controller_delay_cd == 0)
			{
				if ((REG_FFFF & 0x10) == 0x10) { cpu_FlagI = true; }
				REG_FF0F |= 0x10;
				delays_to_process = false;
			}
		}

		uint8_t GetButtons(uint16_t r)
		{
			return input_register;
		}

		uint8_t GetIntRegs(uint16_t r)
		{
			if (r == 0)
			{
				return REG_FF0F;
			}
			else
			{
				return REG_FFFF;
			}
		}

		void clear_screen_func();

		void Send_Video_Buffer();

		void SetIntRegs(uint8_t r)
		{
			// For timer interrupts or serial interrupts that occur on the same cycle as the IRQ clear
			// the clear wins on GB and GBA (tested on GBP.) Assuming true for GBC E too.
			// but only in single speed
			if (((REG_FF0F & 4) == 4) && ((r & 4) == 0) && tim_IRQ_Block && !Double_Speed) { r |= 4; }
			if (((REG_FF0F & 8) == 8) && ((r & 8) == 0) && ser_IRQ_Block && !Double_Speed) { r |= 8; }
			REG_FF0F = r;
		}

		void On_VBlank()
		{
			// things to do on vblank
		}

	#pragma endregion

	#pragma region HW Registers

		uint8_t Read_Registers(uint16_t addr)
		{
			uint8_t ret = 0;
			uint8_t ret1 = 0;
			uint8_t ret2 = 0;
			uint8_t retN = 0;
			uint8_t retW = 0;

			switch (addr)
			{
				// Read Input
				case 0xFF00:
					Is_Lag = false;
					ret = input_register;
					break;

					// Serial data port
				case 0xFF01:
					ret = ser_Read_Reg(addr);
					break;

					// Serial port control
				case 0xFF02:
					ret = ser_Read_Reg(addr);
					break;

					// Timer Registers
				case 0xFF04:
				case 0xFF05:
				case 0xFF06:
				case 0xFF07:
					ret = tim_Read_Reg(addr);
					break;

					// Interrupt flags
				case 0xFF0F:
					//Console.WriteLine("FF0F " + cpu.TotalExecutedCycles);
					ret = REG_FF0F_OLD;
					break;

					// audio regs
				case 0xFF10:
				case 0xFF11:
				case 0xFF12:
				case 0xFF13:
				case 0xFF14:
				case 0xFF16:
				case 0xFF17:
				case 0xFF18:
				case 0xFF19:
				case 0xFF1A:
				case 0xFF1B:
				case 0xFF1C:
				case 0xFF1D:
				case 0xFF1E:
				case 0xFF20:
				case 0xFF21:
				case 0xFF22:
				case 0xFF23:
				case 0xFF24:
				case 0xFF25:
				case 0xFF26:
				case 0xFF30:
				case 0xFF31:
				case 0xFF32:
				case 0xFF33:
				case 0xFF34:
				case 0xFF35:
				case 0xFF36:
				case 0xFF37:
				case 0xFF38:
				case 0xFF39:
				case 0xFF3A:
				case 0xFF3B:
				case 0xFF3C:
				case 0xFF3D:
				case 0xFF3E:
				case 0xFF3F:
					ret = snd_Read_Reg(addr);
					break;

					// PPU Regs
				case 0xFF40:
				case 0xFF41:
				case 0xFF42:
				case 0xFF43:
				case 0xFF44:
				case 0xFF45:
				case 0xFF46:
				case 0xFF47:
				case 0xFF48:
				case 0xFF49:
				case 0xFF4A:
				case 0xFF4B:
					ret = (ppu_pntr->*PPU_Read_Regs)(addr);
					break;

					// Speed Control for GBC
				case 0xFF4D:
					if (GBC_Compat)
					{
						ret = (uint8_t)(((Double_Speed ? 1 : 0) << 7) | (speed_switch ? 1 : 0) | 0x7E);
					}
					else
					{
						ret = 0xFF;
					}
					break;

				case 0xFF4F: // VBK
					if (Is_GBC)
					{
						ret = (uint8_t)(0xFE | VRAM_Bank);
					}
					else
					{
						ret = 0xFF;
					}
					break;

					// Bios control register. Not sure if it is readable
				case 0xFF50:
					ret = 0xFF;
					break;

					// PPU Regs for GBC
				case 0xFF51:
				case 0xFF52:
				case 0xFF53:
				case 0xFF54:
				case 0xFF55:
					if (GBC_Compat)
					{
						ret = (ppu_pntr->*PPU_Read_Regs)(addr);
					}
					else
					{
						ret = 0xFF;
					}
					break;

				case 0xFF56:
					if (GBC_Compat)
					{
						// can receive data
						if ((IR_reg & 0xC0) == 0xC0)
						{
							ret = IR_reg;
						}
						else
						{
							ret = (uint8_t)(IR_reg | 2);
						}
					}
					else
					{
						ret = 0xFF;
					}
					break;

				case 0xFF68:
				case 0xFF69:
				case 0xFF6A:
				case 0xFF6B:
					if (Is_GBC)
					{
						ret = (ppu_pntr->*PPU_Read_Regs)(addr);
					}
					else
					{
						ret = 0xFF;
					}
					break;

					// Ram bank for GBC
				case 0xFF70:
					if (GBC_Compat)
					{
						ret = (uint8_t)(0xF8 | RAM_Bank_ret);
					}
					else
					{
						ret = 0xFF;
					}
					break;

				case 0xFF6C:
					if (GBC_Compat) { ret = undoc_6C; }
					else { ret = 0xFF; }
					break;

				case 0xFF72:
					ret = undoc_72;
					break;

				case 0xFF73:
					ret = undoc_73;
					break;

				case 0xFF74:
					if (GBC_Compat) { ret = undoc_74; }
					else { ret = 0xFF; }
					break;

				case 0xFF75:
					ret = undoc_75;
					break;

				case 0xFF76:
					ret1 = snd_SQ1_output >= snd_DAC_OFST
						? (uint8_t)(snd_SQ1_output - snd_DAC_OFST)
						: (uint8_t)0;
					ret2 = snd_SQ2_output >= snd_DAC_OFST
						? (uint8_t)(snd_SQ2_output - snd_DAC_OFST)
						: (uint8_t)0;
					ret = (uint8_t)(ret1 | (ret2 << 4));
					break;

				case 0xFF77:
					retN = snd_NOISE_output >= snd_DAC_OFST
						? (uint8_t)(snd_NOISE_output - snd_DAC_OFST)
						: (uint8_t)0;
					retW = snd_WAVE_output >= snd_DAC_OFST
						? (uint8_t)(snd_WAVE_output - snd_DAC_OFST)
						: (uint8_t)0;
					ret = (uint8_t)(retN | (retW << 4));
					break;

					// interrupt control register
				case 0xFFFF:
					ret = REG_FFFF;
					break;

				default:
					ret = 0xFF;
					break;

			}
			return ret;
		}

		void Write_Registers(uint16_t addr, uint8_t value)
		{
			uint8_t contr_prev = 0;
			
			switch (addr)
			{
				// select input
				case 0xFF00:
					input_register &= 0xCF;
					input_register |= (uint8_t)(value & 0x30); // top 2 bits always 1

					// check for high to low transitions that trigger IRQs
					contr_prev = input_register;

					input_register &= 0xF0;
					if ((input_register & 0x30) == 0x20)
					{
						input_register |= (uint8_t)(controller_state & 0xF);
					}
					else if ((input_register & 0x30) == 0x10)
					{
						input_register |= (uint8_t)((controller_state & 0xF0) >> 4);
					}
					else if ((input_register & 0x30) == 0x00)
					{
						// if both polls are set, then a bit is zero if either or both pins are zero
						uint8_t temp = (uint8_t)((controller_state & 0xF) & ((controller_state & 0xF0) >> 4));
						input_register |= temp;
					}
					else
					{
						input_register |= 0xF;
					}

					// check for interrupts
					// if an interrupt is triggered, it is delayed by 4 cycles
					if (((contr_prev & 8) > 0) && ((input_register & 8) == 0) ||
						((contr_prev & 4) > 0) && ((input_register & 4) == 0) ||
						((contr_prev & 2) > 0) && ((input_register & 2) == 0) ||
						((contr_prev & 1) > 0) && ((input_register & 1) == 0))
					{
						controller_delay_cd = 4; delays_to_process = true;
					}

					break;

					// Serial data port
				case 0xFF01:
					ser_Write_Reg(addr, value);
					break;

					// Serial port control
				case 0xFF02:
					ser_Write_Reg(addr, value);
					break;

					// Timer Registers
				case 0xFF04:
				case 0xFF05:
				case 0xFF06:
				case 0xFF07:
					tim_Write_Reg(addr, value);
					break;

					// Interrupt flags
				case 0xFF0F:
					REG_FF0F = (uint8_t)(0xE0 | value);

					// check if enabling any of the bits triggered an IRQ
					for (int i = 0; i < 5; i++)
					{
						if (((REG_FFFF >> i) & (REG_FF0F >> i) & 1) == 1)
						{
							cpu_FlagI = true;
						}
					}

					// if no bits are in common between flags and enables, de-assert the IRQ
					if (((REG_FF0F & 0x1F) & REG_FFFF) == 0) { cpu_FlagI = false; }
					break;

					// audio regs
				case 0xFF10:
				case 0xFF11:
				case 0xFF12:
				case 0xFF13:
				case 0xFF14:
				case 0xFF16:
				case 0xFF17:
				case 0xFF18:
				case 0xFF19:
				case 0xFF1A:
				case 0xFF1B:
				case 0xFF1C:
				case 0xFF1D:
				case 0xFF1E:
				case 0xFF20:
				case 0xFF21:
				case 0xFF22:
				case 0xFF23:
				case 0xFF24:
				case 0xFF25:
				case 0xFF26:
				case 0xFF30:
				case 0xFF31:
				case 0xFF32:
				case 0xFF33:
				case 0xFF34:
				case 0xFF35:
				case 0xFF36:
				case 0xFF37:
				case 0xFF38:
				case 0xFF39:
				case 0xFF3A:
				case 0xFF3B:
				case 0xFF3C:
				case 0xFF3D:
				case 0xFF3E:
				case 0xFF3F:
					snd_WriteReg(addr, value);
					break;

					// PPU Regs
				case 0xFF40:
				case 0xFF41:
				case 0xFF42:
				case 0xFF43:
				case 0xFF44:
				case 0xFF45:
				case 0xFF46:
				case 0xFF47:
				case 0xFF48:
				case 0xFF49:
				case 0xFF4A:
				case 0xFF4B:
					(ppu_pntr->*PPU_Write_Regs)(addr, value);
					break;

					// GBC compatibility register (I think)
				case 0xFF4C:
					if ((value != 0xC0) && (value != 0x80) && (GB_bios_register == 0))// && (value != 0xFF) && (value != 0x04))
					{
						GBC_Compat = false;
					}

					//Message_String = "GBC Compatibility? " + to_string(value);
					//MessageCallback(Message_String.length());
					break;

					// Speed Control for GBC
				case 0xFF4D:
					if (GBC_Compat)
					{
						speed_switch = (value & 1) > 0;
					}
					break;

					// VBK
				case 0xFF4F:
					if (Is_GBC/* && !ppu.HDMA_active*/)
					{
						VRAM_Bank = (uint8_t)(value & 1);
					}
					break;

					// Bios control register. Writing 1 permanently disables BIOS until a power cycle occurs
				case 0xFF50:
					// Console.WriteLine(value);
					if (GB_bios_register == 0)
					{
						GB_bios_register = value;
						if (!GBC_Compat) { *PPU_Pal_Change_Blocked = true; RAM_Bank = 1; RAM_Bank_ret = 0; }
					}
					break;

					// PPU Regs for GBC
				case 0xFF51:
				case 0xFF52:
				case 0xFF53:
				case 0xFF54:
				case 0xFF55:
					if (GBC_Compat)
					{
						(ppu_pntr->*PPU_Write_Regs)(addr, value);
					}
					break;

				case 0xFF56:
					IR_reg = (uint8_t)((value & 0xC1) | (IR_reg & 0x3E));

					// send IR signal out
					if ((IR_reg & 0x1) == 0x1) { IR_signal = (uint8_t)(0 | IR_mask); }
					else { IR_signal = 2; }

					// receive own signal if IR on and receive on
					if ((IR_reg & 0xC1) == 0xC1) { IR_self = (uint8_t)(0 | IR_mask); }
					else { IR_self = 2; }

					IR_write = 8;
					break;

				case 0xFF68:
				case 0xFF69:
				case 0xFF6A:
				case 0xFF6B:
					if (Is_GBC)
					{
						(ppu_pntr->*PPU_Write_Regs)(addr, value);
					}
					break;

					// RAM Bank in GBC mode
				case 0xFF70:
					if (GBC_Compat)
					{
						RAM_Bank = value & 7;
						RAM_Bank_ret = RAM_Bank;
						if (RAM_Bank == 0) { RAM_Bank = 1; }
					}
					break;

				case 0xFF6C:
					if (GBC_Compat) { undoc_6C |= (uint8_t)(value & 1); }
					break;

				case 0xFF72:
					if (Is_GBC) { undoc_72 = value; }
					break;

				case 0xFF73:
					if (Is_GBC) { undoc_73 = value; }
					break;

				case 0xFF74:
					if (GBC_Compat) { undoc_74 = value; }
					break;

				case 0xFF75:
					if (Is_GBC) { undoc_75 |= (uint8_t)(value & 0x70); }
					break;

				case 0xFF76:
					// read only
					break;

				case 0xFF77:
					// read only
					break;

					// interrupt control register
				case 0xFFFF:
					REG_FFFF = value;

					// check if enabling any of the bits triggered an IRQ
					for (int i = 0; i < 5; i++)
					{
						if (((REG_FFFF >> i) & (REG_FF0F >> i) & 1) == 1)
						{
							cpu_FlagI = true;
						}
					}

					// if no bits are in common between flags and enables, de-assert the IRQ
					if (((REG_FF0F & 0x1F) & REG_FFFF) == 0) { cpu_FlagI = false; }
					break;

				default:
					//Console.WriteLine(addr + " " + value);
					break;
			}
		}

		void Register_Reset()
		{
			input_register = 0xCF; // not reading any input

			REG_FFFF = 0;
			REG_FF0F = 0xE0;
			REG_FF0F_OLD = 0xE0;

			//undocumented registers
			undoc_6C = 0xFE;
			undoc_72 = 0;
			undoc_73 = 0;
			undoc_74 = 0;
			undoc_75 = 0x8F;
			undoc_76 = 0;
			undoc_77 = 0;
		}

		uint8_t Get_Registers_Internal(uint16_t addr)
		{
			uint8_t ret = 0;

			return ret;
		}

		uint8_t Peek_Registers_8(uint16_t addr)
		{
			uint8_t ret = 0;

			return ret;
		}
	# pragma endregion

	#pragma region LR 35902
		// TODO: STOP for second byte nonzero

		#pragma region Variables
		// General Execution
		bool cpu_was_FlagI, cpu_FlagI;
		bool cpu_Jammed, cpu_Stopped, cpu_Halted;
		bool cpu_CB_Prefix;

		bool cpu_Interrupts_Enabled;

		bool cpu_I_Use; // in halt mode, the I flag is checked earlier then when deicision to IRQ is taken
		bool cpu_Skip_Once;
		bool cpu_Halt_bug_2;
		bool cpu_Halt_bug_3;
		bool cpu_Halt_bug_4;
		bool cpu_Halt_bug_5;
		bool cpu_Stop_Check;

		uint8_t cpu_Int_Clear;

		uint16_t cpu_Opcode, cpu_Instr_Cycle, cpu_Instr_Cycle_Hold;

		uint16_t cpu_Int_Src;

		uint32_t cpu_EI_Pending;

		uint32_t cpu_Instr_Type_Save;
		uint32_t cpu_State_Hold_Save;

		uint64_t cpu_Instruction_Start;

		uint64_t cpu_Stop_Time;

		uint8_t cpu_Regs[14] = { };

		inline bool cpu_FlagCget() { return (cpu_Regs[5] & 0x10) != 0; }
		inline void cpu_FlagCset(bool value) { cpu_Regs[5] = (uint16_t)((cpu_Regs[5] & ~0x10) | (value ? 0x10 : 0x00)); }

		inline bool cpu_FlagHget() { return (cpu_Regs[5] & 0x20) != 0; }
		inline void cpu_FlagHset(bool value) { cpu_Regs[5] = (uint16_t)((cpu_Regs[5] & ~0x20) | (value ? 0x20 : 0x00)); }

		inline bool cpu_FlagNget() { return (cpu_Regs[5] & 0x40) != 0; }
		inline void cpu_FlagNset(bool value) { cpu_Regs[5] = (uint16_t)((cpu_Regs[5] & ~0x40) | (value ? 0x40 : 0x00)); }

		inline bool cpu_FlagZget() { return (cpu_Regs[5] & 0x80) != 0; }
		inline void cpu_FlagZset(bool value) { cpu_Regs[5] = (uint16_t)((cpu_Regs[5] & ~0x80) | (value ? 0x80 : 0x00)); }

		inline uint16_t cpu_RegPCget() { return ((uint16_t)(cpu_Regs[0] | (cpu_Regs[1] << 8))); }
		inline void cpu_RegPCset(uint16_t value) { cpu_Regs[0] = (uint16_t)(value & 0xFF); cpu_Regs[1] = (uint16_t)((value >> 8) & 0xFF); }

		inline uint16_t cpu_RegBCget() { return ((uint16_t)(cpu_Regs[cpu_C] | (cpu_Regs[cpu_B] << 8))); }
		inline void cpu_RegBCset(uint16_t value) { cpu_Regs[cpu_C] = (uint16_t)(value & 0xFF); cpu_Regs[cpu_B] = (uint16_t)((value >> 8) & 0xFF); }
		
		inline uint16_t cpu_RegDEget() { return ((uint16_t)(cpu_Regs[cpu_E] | (cpu_Regs[cpu_D] << 8))); }
		inline void cpu_RegDEset(uint16_t value) { cpu_Regs[cpu_E] = (uint16_t)(value & 0xFF); cpu_Regs[cpu_D] = (uint16_t)((value >> 8) & 0xFF); }

		inline uint16_t cpu_RegHLget() { return ((uint16_t)(cpu_Regs[cpu_L] | (cpu_Regs[cpu_H] << 8))); }
		inline void cpu_RegHLset(uint16_t value) { cpu_Regs[cpu_L] = (uint16_t)(value & 0xFF); cpu_Regs[cpu_H] = (uint16_t)((value >> 8) & 0xFF); }

		inline uint16_t cpu_RegSPget() { return ((uint16_t)(cpu_Regs[cpu_SPl] | (cpu_Regs[cpu_SPh] << 8))); }
		inline void cpu_RegSPset(uint16_t value) { cpu_Regs[cpu_SPl] = (uint16_t)(value & 0xFF); cpu_Regs[cpu_SPh] = (uint16_t)((value >> 8) & 0xFF); }

		inline uint16_t cpu_RegWZget() { return ((uint16_t)(cpu_Regs[cpu_Z] | (cpu_Regs[cpu_W] << 8))); }
		inline void cpu_RegWZset(uint16_t value) { cpu_Regs[cpu_Z] = (uint16_t)(value & 0xFF); cpu_Regs[cpu_W] = (uint16_t)((value >> 8) & 0xFF); }


		// local variables for operations, not stated
		uint32_t cpu_Reg16_dt, cpu_Reg16_st, cpu_ct;
		uint16_t cpu_ans, cpu_ans_l, cpu_ans_h, cpu_temp;
		uint8_t cpu_a_d;
		bool cpu_imm;

		#pragma endregion

		#pragma region Constant Declarations
		// registers
		const static uint16_t cpu_PCl = 0;
		const static uint16_t cpu_PCh = 1;
		const static uint16_t cpu_SPl = 2;
		const static uint16_t cpu_SPh = 3;
		const static uint16_t cpu_A = 4;
		const static uint16_t cpu_F = 5;
		const static uint16_t cpu_B = 6;
		const static uint16_t cpu_C = 7;
		const static uint16_t cpu_D = 8;
		const static uint16_t cpu_E = 9;
		const static uint16_t cpu_H = 10;
		const static uint16_t cpu_L = 11;
		const static uint16_t cpu_W = 12;
		const static uint16_t cpu_Z = 13;
		const static uint16_t cpu_Aim = 14; // use this indicator for RLCA etc., since the Z flag is reset on those

		uint8_t INT_vectors[6] = { 0x40, 0x48, 0x50, 0x58, 0x60, 0x00 };

		// Instruction types
		enum class OpT
		{
			INT_OP,
			REG_OP,
			REG_OP_IND,
			LD_IND_16,
			LD_8_IND,
			INC_DEC_16,
			LD_IND_8_INC,
			LD_R_IM,
			ADD_16,
			STOP,
			JR_COND,
			LD_8_IND_INC_DEC,
			LD_IND_8_INC_DEC_HL,
			INC_DEC_8_IND,
			LD_8_IND_IND,		
			RET_COND,
			POP,
			JP_COND,
			CALL_COND,
			PUSH,
			REG_OP_IND_INC,
			RST,
			RET,
			PREFIX,
			JAM,
			RETI,
			LD_FF_IND_8,
			LD_FFC_IND_8,
			ADD_SP,
			JP_HL,
			LD_FF_IND_16,
			LD_8_IND_FF,
			LD_8_IND_FFC,
			EI_DI,
			LD_HL_SPn,
			LD_SP_HL,
			LD_16_IND_FF,
			INT_OP_IND,
			BIT_OP,
			BIT_TE_IND,
			BIT_OP_IND,

			RESET,
			EXIT,
			SKIP,
			HALT,
			GBC_HALT,
			SPC_HALT,
			STOP_LOOP,
			INTRPT,
			INTRPT_GBC,
			WAIT,
			SPD_CHG
		};

		OpT cpu_Instr_Type;

		OpT cpu_State_Hold;

		OpT cpu_Instr_Type_List[512 + 10] =
		{
			/*0x00*/ OpT::INT_OP,							// NOP
			/*0x01*/ OpT::LD_IND_16,						// LD BC, nn
			/*0x02*/ OpT::LD_8_IND,							// LD (BC), A
			/*0x03*/ OpT::INC_DEC_16,						// INC BC
			/*0x04*/ OpT::INT_OP,							// INC B
			/*0x05*/ OpT::INT_OP,							// DEC B
			/*0x06*/ OpT::LD_IND_8_INC,						// LD B, n
			/*0x07*/ OpT::INT_OP,							// RLCA
			/*0x08*/ OpT::LD_R_IM,							// LD (imm), SP
			/*0x09*/ OpT::ADD_16,							// ADD HL, BC
			/*0x0A*/ OpT::REG_OP_IND,						// LD A, (BC)
			/*0x0B*/ OpT::INC_DEC_16,						// DEC BC
			/*0x0C*/ OpT::INT_OP,							// INC C
			/*0x0D*/ OpT::INT_OP,							// DEC C
			/*0x0E*/ OpT::LD_IND_8_INC,						// LD C, n
			/*0x0F*/ OpT::INT_OP,							// RRCA
			/*0x10*/ OpT::STOP,								// STOP
			/*0x11*/ OpT::LD_IND_16,						// LD DE, nn
			/*0x12*/ OpT::LD_8_IND,							// LD (DE), A
			/*0x13*/ OpT::INC_DEC_16,						// INC DE
			/*0x14*/ OpT::INT_OP,							// INC D
			/*0x15*/ OpT::INT_OP,							// DEC D
			/*0x16*/ OpT::LD_IND_8_INC,						// LD D, n
			/*0x17*/ OpT::INT_OP,							// RLA
			/*0x18*/ OpT::JR_COND,							// JR, r8
			/*0x19*/ OpT::ADD_16,							// ADD HL, DE
			/*0x1A*/ OpT::REG_OP_IND,						// LD A, (DE)
			/*0x1B*/ OpT::INC_DEC_16,						// DEC DE
			/*0x1C*/ OpT::INT_OP,							// INC E
			/*0x1D*/ OpT::INT_OP,							// DEC E
			/*0x1E*/ OpT::LD_IND_8_INC,						// LD E, n
			/*0x1F*/ OpT::INT_OP,							// RRA
			/*0x20*/ OpT::JR_COND,							// JR NZ, r8
			/*0x21*/ OpT::LD_IND_16,						// LD HL, nn
			/*0x22*/ OpT::LD_8_IND_INC_DEC,					// LD (HL+), A
			/*0x23*/ OpT::INC_DEC_16,						// INC HL
			/*0x24*/ OpT::INT_OP,							// INC H
			/*0x25*/ OpT::INT_OP,							// DEC H
			/*0x26*/ OpT::LD_IND_8_INC,						// LD H, n
			/*0x27*/ OpT::INT_OP,							// DAA
			/*0x28*/ OpT::JR_COND,							// JR Z, r8
			/*0x29*/ OpT::ADD_16,							// ADD HL, HL
			/*0x2A*/ OpT::LD_IND_8_INC_DEC_HL,				// LD A, (HL+)
			/*0x2B*/ OpT::INC_DEC_16,						// DEC HL
			/*0x2C*/ OpT::INT_OP,							// INC L
			/*0x2D*/ OpT::INT_OP,							// DEC L
			/*0x2E*/ OpT::LD_IND_8_INC,						// LD L, n
			/*0x2F*/ OpT::INT_OP,							// CPL
			/*0x30*/ OpT::JR_COND,							// JR NC, r8
			/*0x31*/ OpT::LD_IND_16,						// LD SP, nn
			/*0x32*/ OpT::LD_8_IND_INC_DEC,					// LD (HL-), A
			/*0x33*/ OpT::INC_DEC_16,						// INC SP
			/*0x34*/ OpT::INC_DEC_8_IND,					// INC (HL)
			/*0x35*/ OpT::INC_DEC_8_IND,					// DEC (HL)
			/*0x36*/ OpT::LD_8_IND_IND,						// LD (HL), n
			/*0x37*/ OpT::INT_OP,							// SCF
			/*0x38*/ OpT::JR_COND,							// JR C, r8
			/*0x39*/ OpT::ADD_16,							// ADD HL, SP
			/*0x3A*/ OpT::LD_IND_8_INC_DEC_HL,				// LD A, (HL-)
			/*0x3B*/ OpT::INC_DEC_16,						// DEC SP
			/*0x3C*/ OpT::INT_OP,							// INC A
			/*0x3D*/ OpT::INT_OP,							// DEC A
			/*0x3E*/ OpT::LD_IND_8_INC,						// LD A, n
			/*0x3F*/ OpT::INT_OP,							// CCF
			/*0x40*/ OpT::REG_OP,							// LD B, B
			/*0x41*/ OpT::REG_OP,							// LD B, C
			/*0x42*/ OpT::REG_OP,							// LD B, D
			/*0x43*/ OpT::REG_OP,							// LD B, E
			/*0x44*/ OpT::REG_OP,							// LD B, H
			/*0x45*/ OpT::REG_OP,							// LD B, L
			/*0x46*/ OpT::REG_OP_IND,						// LD B, (HL)
			/*0x47*/ OpT::REG_OP,							// LD B, A
			/*0x48*/ OpT::REG_OP,							// LD C, B
			/*0x49*/ OpT::REG_OP,							// LD C, C
			/*0x4A*/ OpT::REG_OP,							// LD C, D
			/*0x4B*/ OpT::REG_OP,							// LD C, E
			/*0x4C*/ OpT::REG_OP,							// LD C, H
			/*0x4D*/ OpT::REG_OP,							// LD C, L
			/*0x4E*/ OpT::REG_OP_IND,						// LD C, (HL)
			/*0x4F*/ OpT::REG_OP,							// LD C, A
			/*0x50*/ OpT::REG_OP,							// LD D, B
			/*0x51*/ OpT::REG_OP,							// LD D, C
			/*0x52*/ OpT::REG_OP,							// LD D, D
			/*0x53*/ OpT::REG_OP,							// LD D, E
			/*0x54*/ OpT::REG_OP,							// LD D, H
			/*0x55*/ OpT::REG_OP,							// LD D, L
			/*0x56*/ OpT::REG_OP_IND,						// LD D, (HL)
			/*0x57*/ OpT::REG_OP,							// LD D, A
			/*0x58*/ OpT::REG_OP,							// LD E, B
			/*0x59*/ OpT::REG_OP,							// LD E, C
			/*0x5A*/ OpT::REG_OP,							// LD E, D
			/*0x5B*/ OpT::REG_OP,							// LD E, E
			/*0x5C*/ OpT::REG_OP,							// LD E, H
			/*0x5D*/ OpT::REG_OP,							// LD E, L
			/*0x5E*/ OpT::REG_OP_IND,						// LD E, (HL)
			/*0x5F*/ OpT::REG_OP,							// LD E, A
			/*0x60*/ OpT::REG_OP,							// LD H, B
			/*0x61*/ OpT::REG_OP,							// LD H, C
			/*0x62*/ OpT::REG_OP,							// LD H, D
			/*0x63*/ OpT::REG_OP,							// LD H, E
			/*0x64*/ OpT::REG_OP,							// LD H, H
			/*0x65*/ OpT::REG_OP,							// LD H, L
			/*0x66*/ OpT::REG_OP_IND,						// LD H, (HL)
			/*0x67*/ OpT::REG_OP,							// LD H, A
			/*0x68*/ OpT::REG_OP,							// LD L, B
			/*0x69*/ OpT::REG_OP,							// LD L, C
			/*0x6A*/ OpT::REG_OP,							// LD L, D
			/*0x6B*/ OpT::REG_OP,							// LD L, E
			/*0x6C*/ OpT::REG_OP,							// LD L, H
			/*0x6D*/ OpT::REG_OP,							// LD L, L
			/*0x6E*/ OpT::REG_OP_IND,						// LD L, (HL)
			/*0x6F*/ OpT::REG_OP,							// LD L, A
			/*0x70*/ OpT::LD_8_IND,							// LD (HL), B
			/*0x71*/ OpT::LD_8_IND,							// LD (HL), C
			/*0x72*/ OpT::LD_8_IND,							// LD (HL), D
			/*0x73*/ OpT::LD_8_IND,							// LD (HL), E
			/*0x74*/ OpT::LD_8_IND,							// LD (HL), H
			/*0x75*/ OpT::LD_8_IND,							// LD (HL), L
			/*0x76*/ OpT::HALT,								// HALT
			/*0x77*/ OpT::LD_8_IND,							// LD (HL), A
			/*0x78*/ OpT::REG_OP,							// LD A, B
			/*0x79*/ OpT::REG_OP,							// LD A, C
			/*0x7A*/ OpT::REG_OP,							// LD A, D
			/*0x7B*/ OpT::REG_OP,							// LD A, E
			/*0x7C*/ OpT::REG_OP,							// LD A, H
			/*0x7D*/ OpT::REG_OP,							// LD A, L
			/*0x7E*/ OpT::REG_OP_IND,						// LD A, (HL)
			/*0x7F*/ OpT::REG_OP,							// LD A, A
			/*0x80*/ OpT::REG_OP,							// ADD A, B
			/*0x81*/ OpT::REG_OP,							// ADD A, C
			/*0x82*/ OpT::REG_OP,							// ADD A, D
			/*0x83*/ OpT::REG_OP,							// ADD A, E
			/*0x84*/ OpT::REG_OP,							// ADD A, H
			/*0x85*/ OpT::REG_OP,							// ADD A, L
			/*0x86*/ OpT::REG_OP_IND,						// ADD A, (HL)
			/*0x87*/ OpT::REG_OP,							// ADD A, A
			/*0x88*/ OpT::REG_OP,							// ADC A, B
			/*0x89*/ OpT::REG_OP,							// ADC A, C
			/*0x8A*/ OpT::REG_OP,							// ADC A, D
			/*0x8B*/ OpT::REG_OP,							// ADC A, E
			/*0x8C*/ OpT::REG_OP,							// ADC A, H
			/*0x8D*/ OpT::REG_OP,							// ADC A, L
			/*0x8E*/ OpT::REG_OP_IND,						// ADC A, (HL)
			/*0x8F*/ OpT::REG_OP,							// ADC A, A
			/*0x90*/ OpT::REG_OP,							// SUB A, B
			/*0x91*/ OpT::REG_OP,							// SUB A, C
			/*0x92*/ OpT::REG_OP,							// SUB A, D
			/*0x93*/ OpT::REG_OP,							// SUB A, E
			/*0x94*/ OpT::REG_OP,							// SUB A, H
			/*0x95*/ OpT::REG_OP,							// SUB A, L
			/*0x96*/ OpT::REG_OP_IND,						// SUB A, (HL)
			/*0x97*/ OpT::REG_OP,							// SUB A, A
			/*0x98*/ OpT::REG_OP,							// SBC A, B
			/*0x99*/ OpT::REG_OP,							// SBC A, C
			/*0x9A*/ OpT::REG_OP,							// SBC A, D
			/*0x9B*/ OpT::REG_OP,							// SBC A, E
			/*0x9C*/ OpT::REG_OP,							// SBC A, H
			/*0x9D*/ OpT::REG_OP,							// SBC A, L
			/*0x9E*/ OpT::REG_OP_IND,						// SBC A, (HL)
			/*0x9F*/ OpT::REG_OP,							// SBC A, A
			/*0xA0*/ OpT::REG_OP,							// AND A, B
			/*0xA1*/ OpT::REG_OP,							// AND A, C
			/*0xA2*/ OpT::REG_OP,							// AND A, D
			/*0xA3*/ OpT::REG_OP,							// AND A, E
			/*0xA4*/ OpT::REG_OP,							// AND A, H
			/*0xA5*/ OpT::REG_OP,							// AND A, L
			/*0xA6*/ OpT::REG_OP_IND,						// AND A, (HL)
			/*0xA7*/ OpT::REG_OP,							// AND A, A
			/*0xA8*/ OpT::REG_OP,							// XOR A, B
			/*0xA9*/ OpT::REG_OP,							// XOR A, C
			/*0xAA*/ OpT::REG_OP,							// XOR A, D
			/*0xAB*/ OpT::REG_OP,							// XOR A, E
			/*0xAC*/ OpT::REG_OP,							// XOR A, H
			/*0xAD*/ OpT::REG_OP,							// XOR A, L
			/*0xAE*/ OpT::REG_OP_IND,						// XOR A, (HL)
			/*0xAF*/ OpT::REG_OP,							// XOR A, A
			/*0xB0*/ OpT::REG_OP,							// OR A, B
			/*0xB1*/ OpT::REG_OP,							// OR A, C
			/*0xB2*/ OpT::REG_OP,							// OR A, D
			/*0xB3*/ OpT::REG_OP,							// OR A, E
			/*0xB4*/ OpT::REG_OP,							// OR A, H
			/*0xB5*/ OpT::REG_OP,							// OR A, L
			/*0xB6*/ OpT::REG_OP_IND,						// OR A, (HL)
			/*0xB7*/ OpT::REG_OP,							// OR A, A
			/*0xB8*/ OpT::REG_OP,							// CP A, B
			/*0xB9*/ OpT::REG_OP,							// CP A, C
			/*0xBA*/ OpT::REG_OP,							// CP A, D
			/*0xBB*/ OpT::REG_OP,							// CP A, E
			/*0xBC*/ OpT::REG_OP,							// CP A, H
			/*0xBD*/ OpT::REG_OP,							// CP A, L
			/*0xBE*/ OpT::REG_OP_IND,						// CP A, (HL)
			/*0xBF*/ OpT::REG_OP,							// CP A, A
			/*0xC0*/ OpT::RET_COND,							// Ret NZ
			/*0xC1*/ OpT::POP,								// POP BC
			/*0xC2*/ OpT::JP_COND,							// JP NZ
			/*0xC3*/ OpT::JP_COND,							// JP
			/*0xC4*/ OpT::CALL_COND,						// CALL NZ
			/*0xC5*/ OpT::PUSH,								// PUSH BC
			/*0xC6*/ OpT::REG_OP_IND_INC,					// ADD A, n
			/*0xC7*/ OpT::RST,								// RST 0
			/*0xC8*/ OpT::RET_COND,							// RET Z
			/*0xC9*/ OpT::RET,								// RET
			/*0xCA*/ OpT::JP_COND,							// JP Z
			/*0xCB*/ OpT::PREFIX,							// PREFIX
			/*0xCC*/ OpT::CALL_COND,						// CALL Z
			/*0xCD*/ OpT::CALL_COND,						// CALL
			/*0xCE*/ OpT::REG_OP_IND_INC,					// ADC A, n
			/*0xCF*/ OpT::RST,								// RST 0x08
			/*0xD0*/ OpT::RET_COND,							// Ret NC
			/*0xD1*/ OpT::POP,								// POP DE
			/*0xD2*/ OpT::JP_COND,							// JP NC
			/*0xD3*/ OpT::JAM,								// JAM
			/*0xD4*/ OpT::CALL_COND,						// CALL NC
			/*0xD5*/ OpT::PUSH,								// PUSH DE
			/*0xD6*/ OpT::REG_OP_IND_INC,					// SUB A, n
			/*0xD7*/ OpT::RST,								// RST 0x10
			/*0xD8*/ OpT::RET_COND,							// RET C
			/*0xD9*/ OpT::RETI,								// RETI
			/*0xDA*/ OpT::JP_COND,							// JP C
			/*0xDB*/ OpT::JAM,								// JAM
			/*0xDC*/ OpT::CALL_COND,						// CALL C
			/*0xDD*/ OpT::JAM,								// JAM
			/*0xDE*/ OpT::REG_OP_IND_INC,					// SBC A, n
			/*0xDF*/ OpT::RST,								// RST 0x18
			/*0xE0*/ OpT::LD_FF_IND_8,						// LD(n), A
			/*0xE1*/ OpT::POP,								// POP HL
			/*0xE2*/ OpT::LD_FFC_IND_8,						// LD(C), A
			/*0xE3*/ OpT::JAM,								// JAM
			/*0xE4*/ OpT::JAM,								// JAM
			/*0xE5*/ OpT::PUSH,								// PUSH HL
			/*0xE6*/ OpT::REG_OP_IND_INC,					// AND A, n
			/*0xE7*/ OpT::RST,								// RST 0x20
			/*0xE8*/ OpT::ADD_SP,							// ADD SP,n
			/*0xE9*/ OpT::JP_HL,							// JP (HL)
			/*0xEA*/ OpT::LD_FF_IND_16,						// LD(nn), A
			/*0xEB*/ OpT::JAM,								// JAM
			/*0xEC*/ OpT::JAM,								// JAM
			/*0xED*/ OpT::JAM,								// JAM
			/*0xEE*/ OpT::REG_OP_IND_INC,					// XOR A, n
			/*0xEF*/ OpT::RST,								// RST 0x28
			/*0xF0*/ OpT::LD_8_IND_FF,						// A, LD(n)
			/*0xF1*/ OpT::POP,								// POP AF
			/*0xF2*/ OpT::LD_8_IND_FFC,						// A, LD(C)
			/*0xF3*/ OpT::EI_DI,							// DI
			/*0xF4*/ OpT::JAM,								// JAM
			/*0xF5*/ OpT::PUSH,								// PUSH AF
			/*0xF6*/ OpT::REG_OP_IND_INC,					// OR A, n
			/*0xF7*/ OpT::RST,								// RST 0x30
			/*0xF8*/ OpT::LD_HL_SPn,						// LD HL, SP+n
			/*0xF9*/ OpT::LD_SP_HL,							// LD, SP, HL
			/*0xFA*/ OpT::LD_16_IND_FF,						// A, LD(nn)
			/*0xFB*/ OpT::EI_DI,							// EI
			/*0xFC*/ OpT::JAM,								// JAM
			/*0xFD*/ OpT::JAM,								// JAM
			/*0xFE*/ OpT::REG_OP_IND_INC,					// CP A, n
			/*0xFF*/ OpT::RST,								// RST 0x38

			// Prefix opcodes
			/*0x00*/ OpT::INT_OP,							// RLC B
			/*0x01*/ OpT::INT_OP,							// RLC C
			/*0x02*/ OpT::INT_OP,							// RLC D
			/*0x03*/ OpT::INT_OP,							// RLC E
			/*0x04*/ OpT::INT_OP,							// RLC H
			/*0x05*/ OpT::INT_OP,							// RLC L
			/*0x06*/ OpT::INT_OP_IND,						// RLC (HL)
			/*0x07*/ OpT::INT_OP,							// RLC A
			/*0x08*/ OpT::INT_OP,							// RRC B
			/*0x09*/ OpT::INT_OP,							// RRC C
			/*0x0A*/ OpT::INT_OP,							// RRC D
			/*0x0B*/ OpT::INT_OP,							// RRC E
			/*0x0C*/ OpT::INT_OP,							// RRC H
			/*0x0D*/ OpT::INT_OP,							// RRC L
			/*0x0E*/ OpT::INT_OP_IND,						// RRC (HL)
			/*0x0F*/ OpT::INT_OP,							// RRC A
			/*0x10*/ OpT::INT_OP,							// RL B
			/*0x11*/ OpT::INT_OP,							// RL C
			/*0x12*/ OpT::INT_OP,							// RL D
			/*0x13*/ OpT::INT_OP,							// RL E
			/*0x14*/ OpT::INT_OP,							// RL H
			/*0x15*/ OpT::INT_OP,							// RL L
			/*0x16*/ OpT::INT_OP_IND,						// RL (HL)
			/*0x17*/ OpT::INT_OP,							// RL A
			/*0x18*/ OpT::INT_OP,							// RR B
			/*0x19*/ OpT::INT_OP,							// RR C
			/*0x1A*/ OpT::INT_OP,							// RR D
			/*0x1B*/ OpT::INT_OP,							// RR E
			/*0x1C*/ OpT::INT_OP,							// RR H
			/*0x1D*/ OpT::INT_OP,							// RR L
			/*0x1E*/ OpT::INT_OP_IND,						// RR (HL)
			/*0x1F*/ OpT::INT_OP,							// RR A
			/*0x20*/ OpT::INT_OP,							// SLA B
			/*0x21*/ OpT::INT_OP,							// SLA C
			/*0x22*/ OpT::INT_OP,							// SLA D
			/*0x23*/ OpT::INT_OP,							// SLA E
			/*0x24*/ OpT::INT_OP,							// SLA H
			/*0x25*/ OpT::INT_OP,							// SLA L
			/*0x26*/ OpT::INT_OP_IND,						// SLA (HL)
			/*0x27*/ OpT::INT_OP,							// SLA A
			/*0x28*/ OpT::INT_OP,							// SRA B
			/*0x29*/ OpT::INT_OP,							// SRA C
			/*0x2A*/ OpT::INT_OP,							// SRA D
			/*0x2B*/ OpT::INT_OP,							// SRA E
			/*0x2C*/ OpT::INT_OP,							// SRA H
			/*0x2D*/ OpT::INT_OP,							// SRA L
			/*0x2E*/ OpT::INT_OP_IND,						// SRA (HL)
			/*0x2F*/ OpT::INT_OP,							// SRA A
			/*0x30*/ OpT::INT_OP,							// SWAP B
			/*0x31*/ OpT::INT_OP,							// SWAP C
			/*0x32*/ OpT::INT_OP,							// SWAP D
			/*0x33*/ OpT::INT_OP,							// SWAP E
			/*0x34*/ OpT::INT_OP,							// SWAP H
			/*0x35*/ OpT::INT_OP,							// SWAP L
			/*0x36*/ OpT::INT_OP_IND,						// SWAP (HL)
			/*0x37*/ OpT::INT_OP,							// SWAP A
			/*0x38*/ OpT::INT_OP,							// SRL B
			/*0x39*/ OpT::INT_OP,							// SRL C
			/*0x3A*/ OpT::INT_OP,							// SRL D
			/*0x3B*/ OpT::INT_OP,							// SRL E
			/*0x3C*/ OpT::INT_OP,							// SRL H
			/*0x3D*/ OpT::INT_OP,							// SRL L
			/*0x3E*/ OpT::INT_OP_IND,						// SRL (HL)
			/*0x3F*/ OpT::INT_OP,							// SRL A
			/*0x40*/ OpT::BIT_OP,							// BIT 0, B
			/*0x41*/ OpT::BIT_OP,							// BIT 0, C
			/*0x42*/ OpT::BIT_OP,							// BIT 0, D
			/*0x43*/ OpT::BIT_OP,							// BIT 0, E
			/*0x44*/ OpT::BIT_OP,							// BIT 0, H
			/*0x45*/ OpT::BIT_OP,							// BIT 0, L
			/*0x46*/ OpT::BIT_TE_IND,						// BIT 0, (HL)
			/*0x47*/ OpT::BIT_OP,							// BIT 0, A
			/*0x48*/ OpT::BIT_OP,							// BIT 1, B
			/*0x49*/ OpT::BIT_OP,							// BIT 1, C
			/*0x4A*/ OpT::BIT_OP,							// BIT 1, D
			/*0x4B*/ OpT::BIT_OP,							// BIT 1, E
			/*0x4C*/ OpT::BIT_OP,							// BIT 1, H
			/*0x4D*/ OpT::BIT_OP,							// BIT 1, L
			/*0x4E*/ OpT::BIT_TE_IND,						// BIT 1, (HL)
			/*0x4F*/ OpT::BIT_OP,							// BIT 1, A
			/*0x50*/ OpT::BIT_OP,							// BIT 2, B
			/*0x51*/ OpT::BIT_OP,							// BIT 2, C
			/*0x52*/ OpT::BIT_OP,							// BIT 2, D
			/*0x53*/ OpT::BIT_OP,							// BIT 2, E
			/*0x54*/ OpT::BIT_OP,							// BIT 2, H
			/*0x55*/ OpT::BIT_OP,							// BIT 2, L
			/*0x56*/ OpT::BIT_TE_IND,						// BIT 2, (HL)
			/*0x57*/ OpT::BIT_OP,							// BIT 2, A
			/*0x58*/ OpT::BIT_OP,							// BIT 3, B
			/*0x59*/ OpT::BIT_OP,							// BIT 3, C
			/*0x5A*/ OpT::BIT_OP,							// BIT 3, D
			/*0x5B*/ OpT::BIT_OP,							// BIT 3, E
			/*0x5C*/ OpT::BIT_OP,							// BIT 3, H
			/*0x5D*/ OpT::BIT_OP,							// BIT 3, L
			/*0x5E*/ OpT::BIT_TE_IND,						// BIT 3, (HL)
			/*0x5F*/ OpT::BIT_OP,							// BIT 3, A
			/*0x60*/ OpT::BIT_OP,							// BIT 4, B
			/*0x61*/ OpT::BIT_OP,							// BIT 4, C
			/*0x62*/ OpT::BIT_OP,							// BIT 4, D
			/*0x63*/ OpT::BIT_OP,							// BIT 4, E
			/*0x64*/ OpT::BIT_OP,							// BIT 4, H
			/*0x65*/ OpT::BIT_OP,							// BIT 4, L
			/*0x66*/ OpT::BIT_TE_IND,						// BIT 4, (HL)
			/*0x67*/ OpT::BIT_OP,							// BIT 4, A
			/*0x68*/ OpT::BIT_OP,							// BIT 5, B
			/*0x69*/ OpT::BIT_OP,							// BIT 5, C
			/*0x6A*/ OpT::BIT_OP,							// BIT 5, D
			/*0x6B*/ OpT::BIT_OP,							// BIT 5, E
			/*0x6C*/ OpT::BIT_OP,							// BIT 5, H
			/*0x6D*/ OpT::BIT_OP,							// BIT 5, L
			/*0x6E*/ OpT::BIT_TE_IND,						// BIT 5, (HL)
			/*0x6F*/ OpT::BIT_OP,							// BIT 5, A
			/*0x70*/ OpT::BIT_OP,							// BIT 6, B
			/*0x71*/ OpT::BIT_OP,							// BIT 6, C
			/*0x72*/ OpT::BIT_OP,							// BIT 6, D
			/*0x73*/ OpT::BIT_OP,							// BIT 6, E
			/*0x74*/ OpT::BIT_OP,							// BIT 6, H
			/*0x75*/ OpT::BIT_OP,							// BIT 6, L
			/*0x76*/ OpT::BIT_TE_IND,						// BIT 6, (HL)
			/*0x77*/ OpT::BIT_OP,							// BIT 6, A
			/*0x78*/ OpT::BIT_OP,							// BIT 7, B
			/*0x79*/ OpT::BIT_OP,							// BIT 7, C
			/*0x7A*/ OpT::BIT_OP,							// BIT 7, D
			/*0x7B*/ OpT::BIT_OP,							// BIT 7, E
			/*0x7C*/ OpT::BIT_OP,							// BIT 7, H
			/*0x7D*/ OpT::BIT_OP,							// BIT 7, L
			/*0x7E*/ OpT::BIT_TE_IND,						// BIT 7, (HL)
			/*0x7F*/ OpT::BIT_OP,							// BIT 7, A
			/*0x80*/ OpT::BIT_OP,							// RES 0, B
			/*0x81*/ OpT::BIT_OP,							// RES 0, C
			/*0x82*/ OpT::BIT_OP,							// RES 0, D
			/*0x83*/ OpT::BIT_OP,							// RES 0, E
			/*0x84*/ OpT::BIT_OP,							// RES 0, H
			/*0x85*/ OpT::BIT_OP,							// RES 0, L
			/*0x86*/ OpT::BIT_OP_IND,						// RES 0, (HL)
			/*0x87*/ OpT::BIT_OP,							// RES 0, A
			/*0x88*/ OpT::BIT_OP,							// RES 1, B
			/*0x89*/ OpT::BIT_OP,							// RES 1, C
			/*0x8A*/ OpT::BIT_OP,							// RES 1, D
			/*0x8B*/ OpT::BIT_OP,							// RES 1, E
			/*0x8C*/ OpT::BIT_OP,							// RES 1, H
			/*0x8D*/ OpT::BIT_OP,							// RES 1, L
			/*0x8E*/ OpT::BIT_OP_IND,						// RES 1, (HL)
			/*0x8F*/ OpT::BIT_OP,							// RES 1, A
			/*0x90*/ OpT::BIT_OP,							// RES 2, B
			/*0x91*/ OpT::BIT_OP,							// RES 2, C
			/*0x92*/ OpT::BIT_OP,							// RES 2, D
			/*0x93*/ OpT::BIT_OP,							// RES 2, E
			/*0x94*/ OpT::BIT_OP,							// RES 2, H
			/*0x95*/ OpT::BIT_OP,							// RES 2, L
			/*0x96*/ OpT::BIT_OP_IND,						// RES 2, (HL)
			/*0x97*/ OpT::BIT_OP,							// RES 2, A
			/*0x98*/ OpT::BIT_OP,							// RES 3, B
			/*0x99*/ OpT::BIT_OP,							// RES 3, C
			/*0x9A*/ OpT::BIT_OP,							// RES 3, D
			/*0x9B*/ OpT::BIT_OP,							// RES 3, E
			/*0x9C*/ OpT::BIT_OP,							// RES 3, H
			/*0x9D*/ OpT::BIT_OP,							// RES 3, L
			/*0x9E*/ OpT::BIT_OP_IND,						// RES 3, (HL)
			/*0x9F*/ OpT::BIT_OP,							// RES 3, A
			/*0xA0*/ OpT::BIT_OP,							// RES 4, B
			/*0xA1*/ OpT::BIT_OP,							// RES 4, C
			/*0xA2*/ OpT::BIT_OP,							// RES 4, D
			/*0xA3*/ OpT::BIT_OP,							// RES 4, E
			/*0xA4*/ OpT::BIT_OP,							// RES 4, H
			/*0xA5*/ OpT::BIT_OP,							// RES 4, L
			/*0xA6*/ OpT::BIT_OP_IND,						// RES 4, (HL)
			/*0xA7*/ OpT::BIT_OP,							// RES 4, A
			/*0xA8*/ OpT::BIT_OP,							// RES 5, B
			/*0xA9*/ OpT::BIT_OP,							// RES 5, C
			/*0xAA*/ OpT::BIT_OP,							// RES 5, D
			/*0xAB*/ OpT::BIT_OP,							// RES 5, E
			/*0xAC*/ OpT::BIT_OP,							// RES 5, H
			/*0xAD*/ OpT::BIT_OP,							// RES 5, L
			/*0xAE*/ OpT::BIT_OP_IND,						// RES 5, (HL)
			/*0xAF*/ OpT::BIT_OP,							// RES 5, A
			/*0xB0*/ OpT::BIT_OP,							// RES 6, B
			/*0xB1*/ OpT::BIT_OP,							// RES 6, C
			/*0xB2*/ OpT::BIT_OP,							// RES 6, D
			/*0xB3*/ OpT::BIT_OP,							// RES 6, E
			/*0xB4*/ OpT::BIT_OP,							// RES 6, H
			/*0xB5*/ OpT::BIT_OP,							// RES 6, L
			/*0xB6*/ OpT::BIT_OP_IND,						// RES 6, (HL)
			/*0xB7*/ OpT::BIT_OP,							// RES 6, A
			/*0xB8*/ OpT::BIT_OP,							// RES 7, B
			/*0xB9*/ OpT::BIT_OP,							// RES 7, C
			/*0xBA*/ OpT::BIT_OP,							// RES 7, D
			/*0xBB*/ OpT::BIT_OP,							// RES 7, E
			/*0xBC*/ OpT::BIT_OP,							// RES 7, H
			/*0xBD*/ OpT::BIT_OP,							// RES 7, L
			/*0xBE*/ OpT::BIT_OP_IND,						// RES 7, (HL)
			/*0xBF*/ OpT::BIT_OP,							// RES 7, A
			/*0xC0*/ OpT::BIT_OP,							// SET 0, B
			/*0xC1*/ OpT::BIT_OP,							// SET 0, C
			/*0xC2*/ OpT::BIT_OP,							// SET 0, D
			/*0xC3*/ OpT::BIT_OP,							// SET 0, E
			/*0xC4*/ OpT::BIT_OP,							// SET 0, H
			/*0xC5*/ OpT::BIT_OP,							// SET 0, L
			/*0xC6*/ OpT::BIT_OP_IND,						// SET 0, (HL)
			/*0xC7*/ OpT::BIT_OP,							// SET 0, A
			/*0xC8*/ OpT::BIT_OP,							// SET 1, B
			/*0xC9*/ OpT::BIT_OP,							// SET 1, C
			/*0xCA*/ OpT::BIT_OP,							// SET 1, D
			/*0xCB*/ OpT::BIT_OP,							// SET 1, E
			/*0xCC*/ OpT::BIT_OP,							// SET 1, H
			/*0xCD*/ OpT::BIT_OP,							// SET 1, L
			/*0xCE*/ OpT::BIT_OP_IND,						// SET 1, (HL)
			/*0xCF*/ OpT::BIT_OP,							// SET 1, A
			/*0xD0*/ OpT::BIT_OP,							// SET 2, B
			/*0xD1*/ OpT::BIT_OP,							// SET 2, C
			/*0xD2*/ OpT::BIT_OP,							// SET 2, D
			/*0xD3*/ OpT::BIT_OP,							// SET 2, E
			/*0xD4*/ OpT::BIT_OP,							// SET 2, H
			/*0xD5*/ OpT::BIT_OP,							// SET 2, L
			/*0xD6*/ OpT::BIT_OP_IND,						// SET 2, (HL)
			/*0xD7*/ OpT::BIT_OP,							// SET 2, A
			/*0xD8*/ OpT::BIT_OP,							// SET 3, B
			/*0xD9*/ OpT::BIT_OP,							// SET 3, C
			/*0xDA*/ OpT::BIT_OP,							// SET 3, D
			/*0xDB*/ OpT::BIT_OP,							// SET 3, E
			/*0xDC*/ OpT::BIT_OP,							// SET 3, H
			/*0xDD*/ OpT::BIT_OP,							// SET 3, L
			/*0xDE*/ OpT::BIT_OP_IND,						// SET 3, (HL)
			/*0xDF*/ OpT::BIT_OP,							// SET 3, A
			/*0xE0*/ OpT::BIT_OP,							// SET 4, B
			/*0xE1*/ OpT::BIT_OP,							// SET 4, C
			/*0xE2*/ OpT::BIT_OP,							// SET 4, D
			/*0xE3*/ OpT::BIT_OP,							// SET 4, E
			/*0xE4*/ OpT::BIT_OP,							// SET 4, H
			/*0xE5*/ OpT::BIT_OP,							// SET 4, L
			/*0xE6*/ OpT::BIT_OP_IND,						// SET 4, (HL)
			/*0xE7*/ OpT::BIT_OP,							// SET 4, A
			/*0xE8*/ OpT::BIT_OP,							// SET 5, B
			/*0xE9*/ OpT::BIT_OP,							// SET 5, C
			/*0xEA*/ OpT::BIT_OP,							// SET 5, D
			/*0xEB*/ OpT::BIT_OP,							// SET 5, E
			/*0xEC*/ OpT::BIT_OP,							// SET 5, H
			/*0xED*/ OpT::BIT_OP,							// SET 5, L
			/*0xEE*/ OpT::BIT_OP_IND,						// SET 5, (HL)
			/*0xEF*/ OpT::BIT_OP,							// SET 5, A
			/*0xF0*/ OpT::BIT_OP,							// SET 6, B
			/*0xF1*/ OpT::BIT_OP,							// SET 6, C
			/*0xF2*/ OpT::BIT_OP,							// SET 6, D
			/*0xF3*/ OpT::BIT_OP,							// SET 6, E
			/*0xF4*/ OpT::BIT_OP,							// SET 6, H
			/*0xF5*/ OpT::BIT_OP,							// SET 6, L
			/*0xF6*/ OpT::BIT_OP_IND,						// SET 6, (HL)
			/*0xF7*/ OpT::BIT_OP,							// SET 6, A
			/*0xF8*/ OpT::BIT_OP,							// SET 7, B
			/*0xF9*/ OpT::BIT_OP,							// SET 7, C
			/*0xFA*/ OpT::BIT_OP,							// SET 7, D
			/*0xFB*/ OpT::BIT_OP,							// SET 7, E
			/*0xFC*/ OpT::BIT_OP,							// SET 7, H
			/*0xFD*/ OpT::BIT_OP,							// SET 7, L
			/*0xFE*/ OpT::BIT_OP_IND,						// SET 7, (HL)
			/*0xFF*/ OpT::BIT_OP,							// SET 7, A

			// IRQ and misc
			OpT::RESET,
			OpT::EXIT,
			OpT::SKIP,
			OpT::GBC_HALT,
			OpT::SPC_HALT,
			OpT::STOP_LOOP,
			OpT::INTRPT,
			OpT::INTRPT_GBC,
			OpT::WAIT,
			OpT::SPD_CHG
		};

		#pragma endregion

		#pragma region LR35902 functions
		void cpu_Exec_INT_Func()
		{
			switch (cpu_Opcode)
			{
				case 0x00: break;
				// LD BC, nn
				// LD (BC), A
				// INC_Func(cpu_B); break;C
				case 0x04: cpu_INC8_Func(cpu_B); break;
				case 0x05: cpu_DEC8_Func(cpu_B); break;
				// LD B, n
				case 0x07: cpu_RLC_Func(cpu_Aim); break;
				// LD (imm), SP
				// ADD HL, BC
				// LD A, (BC)
				// DEC_Func(cpu_B); break;C
				case 0x0C: cpu_INC8_Func(cpu_C); break;
				case 0x0D: cpu_DEC8_Func(cpu_C); break;
				// LD C, n
				case 0x0F: cpu_RRC_Func(cpu_Aim); break;
				// STOP
				// LD DE, nn
				// LD (DE), A
				// INC_Func(cpu_D); break;E
				case 0x14: cpu_INC8_Func(cpu_D); break;
				case 0x15: cpu_DEC8_Func(cpu_D); break;
				// LD D, n
				case 0x17: cpu_RL_Func(cpu_Aim); break;
				// JR, r8
				// ADD HL, DE
				// LD A, (DE)
				// DEC_Func(cpu_D); break;E
				case 0x1C: cpu_INC8_Func(cpu_E); break;
				case 0x1D: cpu_DEC8_Func(cpu_E); break;
				// LD E, n
				case 0x1F: cpu_RR_Func(cpu_Aim); break;
				// JR NZ, r8
				// LD HL, nn
				// LD (HL+), A
				// INC_Func(cpu_H); break;L
				case 0x24: cpu_INC8_Func(cpu_H); break;
				case 0x25: cpu_DEC8_Func(cpu_H); break;
				// LD H, n
				case 0x27: cpu_DA_Func(cpu_A); break;
				// JR Z, r8
				// ADD HL, HL
				// LD A, (HL+)
				// DEC_Func(cpu_H); break;L
				case 0x2C: cpu_INC8_Func(cpu_L); break;
				case 0x2D: cpu_DEC8_Func(cpu_L); break;
				// LD L, n
				case 0x2F: cpu_CPL_Func(cpu_A); break;
				// JR NC, r8
				// LD SP, nn
				// LD (HL-), A
				// INC_Func(cpu_SP
				// INC_Func(cpu_(HL)
				// DEC_Func(cpu_(HL)
				// LD (HL), n
				case 0x37: cpu_SCF_Func(cpu_A); break;
				// JR C, r8
				// ADD HL, SP
				// LD A, (HL-)
				// DEC_Func(cpu_SP
				case 0x3C: cpu_INC8_Func(cpu_A); break;
				case 0x3D: cpu_DEC8_Func(cpu_A); break;
				// LD A, n
				case 0x3F: cpu_CCF_Func(cpu_A); break;

				// Prefix opcodes
				case 0x100: cpu_RLC_Func(cpu_B); break;
				case 0x101: cpu_RLC_Func(cpu_C); break;
				case 0x102: cpu_RLC_Func(cpu_D); break;
				case 0x103: cpu_RLC_Func(cpu_E); break;
				case 0x104: cpu_RLC_Func(cpu_H); break;
				case 0x105: cpu_RLC_Func(cpu_L); break;
				// RLC_Func(cpu_(HL)
				case 0x107: cpu_RLC_Func(cpu_A); break;
				case 0x108: cpu_RRC_Func(cpu_B); break;
				case 0x109: cpu_RRC_Func(cpu_C); break;
				case 0x10A: cpu_RRC_Func(cpu_D); break;
				case 0x10B: cpu_RRC_Func(cpu_E); break;
				case 0x10C: cpu_RRC_Func(cpu_H); break;
				case 0x10D: cpu_RRC_Func(cpu_L); break;
				// RRC_Func(cpu_(HL)
				case 0x10F: cpu_RRC_Func(cpu_A); break;
				case 0x110: cpu_RL_Func(cpu_B); break;
				case 0x111: cpu_RL_Func(cpu_C); break;
				case 0x112: cpu_RL_Func(cpu_D); break;
				case 0x113: cpu_RL_Func(cpu_E); break;
				case 0x114: cpu_RL_Func(cpu_H); break;
				case 0x115: cpu_RL_Func(cpu_L); break;
				// RL_Func(cpu_(HL)
				case 0x117: cpu_RL_Func(cpu_A); break;
				case 0x118: cpu_RR_Func(cpu_B); break;
				case 0x119: cpu_RR_Func(cpu_C); break;
				case 0x11A: cpu_RR_Func(cpu_D); break;
				case 0x11B: cpu_RR_Func(cpu_E); break;
				case 0x11C: cpu_RR_Func(cpu_H); break;
				case 0x11D: cpu_RR_Func(cpu_L); break;
				// RR_Func(cpu_(HL)
				case 0x11F: cpu_RR_Func(cpu_A); break;
				case 0x120: cpu_SLA_Func(cpu_B); break;
				case 0x121: cpu_SLA_Func(cpu_C); break;
				case 0x122: cpu_SLA_Func(cpu_D); break;
				case 0x123: cpu_SLA_Func(cpu_E); break;
				case 0x124: cpu_SLA_Func(cpu_H); break;
				case 0x125: cpu_SLA_Func(cpu_L); break;
				// SLA_Func(cpu_(HL)
				case 0x127: cpu_SLA_Func(cpu_A); break;
				case 0x128: cpu_SRA_Func(cpu_B); break;
				case 0x129: cpu_SRA_Func(cpu_C); break;
				case 0x12A: cpu_SRA_Func(cpu_D); break;
				case 0x12B: cpu_SRA_Func(cpu_E); break;
				case 0x12C: cpu_SRA_Func(cpu_H); break;
				case 0x12D: cpu_SRA_Func(cpu_L); break;
				// SRA_Func(cpu_(HL)
				case 0x12F: cpu_SRA_Func(cpu_A); break;
				case 0x130: cpu_SWAP_Func(cpu_B); break;
				case 0x131: cpu_SWAP_Func(cpu_C); break;
				case 0x132: cpu_SWAP_Func(cpu_D); break;
				case 0x133: cpu_SWAP_Func(cpu_E); break;
				case 0x134: cpu_SWAP_Func(cpu_H); break;
				case 0x135: cpu_SWAP_Func(cpu_L); break;
				// SWAP_Func(cpu_(HL)
				case 0x137: cpu_SWAP_Func(cpu_A); break;
				case 0x138: cpu_SRL_Func(cpu_B); break;
				case 0x139: cpu_SRL_Func(cpu_C); break;
				case 0x13A: cpu_SRL_Func(cpu_D); break;
				case 0x13B: cpu_SRL_Func(cpu_E); break;
				case 0x13C: cpu_SRL_Func(cpu_H); break;
				case 0x13D: cpu_SRL_Func(cpu_L); break;
				// SRL_Func(cpu_(HL)
				case 0x13F: cpu_SRL_Func(cpu_A); break;
			}
		}

		void cpu_Exec_REG_Func()
		{
			switch (cpu_Opcode)
			{
				case 0x40: cpu_TR_Func(cpu_B, cpu_B); break;
				case 0x41: cpu_TR_Func(cpu_B, cpu_C); break;
				case 0x42: cpu_TR_Func(cpu_B, cpu_D); break;
				case 0x43: cpu_TR_Func(cpu_B, cpu_E); break;
				case 0x44: cpu_TR_Func(cpu_B, cpu_H); break;
				case 0x45: cpu_TR_Func(cpu_B, cpu_L); break;
				// LD_Func(cpu_B, (HL)
				case 0x47: cpu_TR_Func(cpu_B, cpu_A); break;
				case 0x48: cpu_TR_Func(cpu_C, cpu_B); break;
				case 0x49: cpu_TR_Func(cpu_C, cpu_C); break;
				case 0x4A: cpu_TR_Func(cpu_C, cpu_D); break;
				case 0x4B: cpu_TR_Func(cpu_C, cpu_E); break;
				case 0x4C: cpu_TR_Func(cpu_C, cpu_H); break;
				case 0x4D: cpu_TR_Func(cpu_C, cpu_L); break;
				// LD_Func(cpu_C, (HL)
				case 0x4F: cpu_TR_Func(cpu_C, cpu_A); break;
				case 0x50: cpu_TR_Func(cpu_D, cpu_B); break;
				case 0x51: cpu_TR_Func(cpu_D, cpu_C); break;
				case 0x52: cpu_TR_Func(cpu_D, cpu_D); break;
				case 0x53: cpu_TR_Func(cpu_D, cpu_E); break;
				case 0x54: cpu_TR_Func(cpu_D, cpu_H); break;
				case 0x55: cpu_TR_Func(cpu_D, cpu_L); break;
				// LD_Func(cpu_D, (HL)
				case 0x57: cpu_TR_Func(cpu_D, cpu_A); break;
				case 0x58: cpu_TR_Func(cpu_E, cpu_B); break;
				case 0x59: cpu_TR_Func(cpu_E, cpu_C); break;
				case 0x5A: cpu_TR_Func(cpu_E, cpu_D); break;
				case 0x5B: cpu_TR_Func(cpu_E, cpu_E); break;
				case 0x5C: cpu_TR_Func(cpu_E, cpu_H); break;
				case 0x5D: cpu_TR_Func(cpu_E, cpu_L); break;
				// LD_Func(cpu_E, (HL)
				case 0x5F: cpu_TR_Func(cpu_E, cpu_A); break;
				case 0x60: cpu_TR_Func(cpu_H, cpu_B); break;
				case 0x61: cpu_TR_Func(cpu_H, cpu_C); break;
				case 0x62: cpu_TR_Func(cpu_H, cpu_D); break;
				case 0x63: cpu_TR_Func(cpu_H, cpu_E); break;
				case 0x64: cpu_TR_Func(cpu_H, cpu_H); break;
				case 0x65: cpu_TR_Func(cpu_H, cpu_L); break;
				// LD_Func(cpu_H, (HL)
				case 0x67: cpu_TR_Func(cpu_H, cpu_A); break;
				case 0x68: cpu_TR_Func(cpu_L, cpu_B); break;
				case 0x69: cpu_TR_Func(cpu_L, cpu_C); break;
				case 0x6A: cpu_TR_Func(cpu_L, cpu_D); break;
				case 0x6B: cpu_TR_Func(cpu_L, cpu_E); break;
				case 0x6C: cpu_TR_Func(cpu_L, cpu_H); break;
				case 0x6D: cpu_TR_Func(cpu_L, cpu_L); break;
				// LD_Func(cpu_L, (HL)
				case 0x6F: cpu_TR_Func(cpu_L, cpu_A); break;
				// LD (HL), cpu_B); break;
				// LD (HL), cpu_C); break;
				// LD (HL), cpu_D); break;
				// LD (HL), cpu_E); break;
				// LD (HL), cpu_H); break;
				// LD (HL), cpu_L); break;
				// HALT
				// LD (HL), cpu_A); break;
				case 0x78: cpu_TR_Func(cpu_A, cpu_B); break;
				case 0x79: cpu_TR_Func(cpu_A, cpu_C); break;
				case 0x7A: cpu_TR_Func(cpu_A, cpu_D); break;
				case 0x7B: cpu_TR_Func(cpu_A, cpu_E); break;
				case 0x7C: cpu_TR_Func(cpu_A, cpu_H); break;
				case 0x7D: cpu_TR_Func(cpu_A, cpu_L); break;
				// LD_Func(cpu_A, (HL)
				case 0x7F: cpu_TR_Func(cpu_A, cpu_A); break;
				case 0x80: cpu_ADD8_Func(cpu_A, cpu_B); break;
				case 0x81: cpu_ADD8_Func(cpu_A, cpu_C); break;
				case 0x82: cpu_ADD8_Func(cpu_A, cpu_D); break;
				case 0x83: cpu_ADD8_Func(cpu_A, cpu_E); break;
				case 0x84: cpu_ADD8_Func(cpu_A, cpu_H); break;
				case 0x85: cpu_ADD8_Func(cpu_A, cpu_L); break;
				// ADD_Func(cpu_A, (HL)
				case 0x87: cpu_ADD8_Func(cpu_A, cpu_A); break;
				case 0x88: cpu_ADC8_Func(cpu_A, cpu_B); break;
				case 0x89: cpu_ADC8_Func(cpu_A, cpu_C); break;
				case 0x8A: cpu_ADC8_Func(cpu_A, cpu_D); break;
				case 0x8B: cpu_ADC8_Func(cpu_A, cpu_E); break;
				case 0x8C: cpu_ADC8_Func(cpu_A, cpu_H); break;
				case 0x8D: cpu_ADC8_Func(cpu_A, cpu_L); break;
				// ADC_Func(cpu_A, (HL)
				case 0x8F: cpu_ADC8_Func(cpu_A, cpu_A); break;
				case 0x90: cpu_SUB8_Func(cpu_A, cpu_B); break;
				case 0x91: cpu_SUB8_Func(cpu_A, cpu_C); break;
				case 0x92: cpu_SUB8_Func(cpu_A, cpu_D); break;
				case 0x93: cpu_SUB8_Func(cpu_A, cpu_E); break;
				case 0x94: cpu_SUB8_Func(cpu_A, cpu_H); break;
				case 0x95: cpu_SUB8_Func(cpu_A, cpu_L); break;
				// SUB_Func(cpu_A, (HL)
				case 0x97: cpu_SUB8_Func(cpu_A, cpu_A); break;
				case 0x98: cpu_SBC8_Func(cpu_A, cpu_B); break;
				case 0x99: cpu_SBC8_Func(cpu_A, cpu_C); break;
				case 0x9A: cpu_SBC8_Func(cpu_A, cpu_D); break;
				case 0x9B: cpu_SBC8_Func(cpu_A, cpu_E); break;
				case 0x9C: cpu_SBC8_Func(cpu_A, cpu_H); break;
				case 0x9D: cpu_SBC8_Func(cpu_A, cpu_L); break;
				// SBC_Func(cpu_A, (HL)
				case 0x9F: cpu_SBC8_Func(cpu_A, cpu_A); break;
				case 0xA0: cpu_AND8_Func(cpu_A, cpu_B); break;
				case 0xA1: cpu_AND8_Func(cpu_A, cpu_C); break;
				case 0xA2: cpu_AND8_Func(cpu_A, cpu_D); break;
				case 0xA3: cpu_AND8_Func(cpu_A, cpu_E); break;
				case 0xA4: cpu_AND8_Func(cpu_A, cpu_H); break;
				case 0xA5: cpu_AND8_Func(cpu_A, cpu_L); break;
				// AND_Func(cpu_A, (HL)
				case 0xA7: cpu_AND8_Func(cpu_A, cpu_A); break;
				case 0xA8: cpu_XOR8_Func(cpu_A, cpu_B); break;
				case 0xA9: cpu_XOR8_Func(cpu_A, cpu_C); break;
				case 0xAA: cpu_XOR8_Func(cpu_A, cpu_D); break;
				case 0xAB: cpu_XOR8_Func(cpu_A, cpu_E); break;
				case 0xAC: cpu_XOR8_Func(cpu_A, cpu_H); break;
				case 0xAD: cpu_XOR8_Func(cpu_A, cpu_L); break;
				// XOR_Func(cpu_A, (HL)
				case 0xAF: cpu_XOR8_Func(cpu_A, cpu_A); break;
				case 0xB0: cpu_OR8_Func(cpu_A, cpu_B); break;
				case 0xB1: cpu_OR8_Func(cpu_A, cpu_C); break;
				case 0xB2: cpu_OR8_Func(cpu_A, cpu_D); break;
				case 0xB3: cpu_OR8_Func(cpu_A, cpu_E); break;
				case 0xB4: cpu_OR8_Func(cpu_A, cpu_H); break;
				case 0xB5: cpu_OR8_Func(cpu_A, cpu_L); break;
				// OR_Func(cpu_A, (HL)
				case 0xB7: cpu_OR8_Func(cpu_A, cpu_A); break;
				case 0xB8: cpu_CP8_Func(cpu_A, cpu_B); break;
				case 0xB9: cpu_CP8_Func(cpu_A, cpu_C); break;
				case 0xBA: cpu_CP8_Func(cpu_A, cpu_D); break;
				case 0xBB: cpu_CP8_Func(cpu_A, cpu_E); break;
				case 0xBC: cpu_CP8_Func(cpu_A, cpu_H); break;
				case 0xBD: cpu_CP8_Func(cpu_A, cpu_L); break;
				// CP_Func(cpu_A, (HL)
				case 0xBF: cpu_CP8_Func(cpu_A, cpu_A); break;
			}
		}

		void cpu_Exec_Bit_Func()
		{
			switch (cpu_Opcode)
			{
				case 0x140: cpu_BIT_Func(0, cpu_B); break;
				case 0x141: cpu_BIT_Func(0, cpu_C); break;
				case 0x142: cpu_BIT_Func(0, cpu_D); break;
				case 0x143: cpu_BIT_Func(0, cpu_E); break;
				case 0x144: cpu_BIT_Func(0, cpu_H); break;
				case 0x145: cpu_BIT_Func(0, cpu_L); break;
				// cpu_BIT_Func(0, (HL)
				case 0x147: cpu_BIT_Func(0, cpu_A); break;
				case 0x148: cpu_BIT_Func(1, cpu_B); break;
				case 0x149: cpu_BIT_Func(1, cpu_C); break;
				case 0x14A: cpu_BIT_Func(1, cpu_D); break;
				case 0x14B: cpu_BIT_Func(1, cpu_E); break;
				case 0x14C: cpu_BIT_Func(1, cpu_H); break;
				case 0x14D: cpu_BIT_Func(1, cpu_L); break;
				// cpu_BIT_Func(1, (HL)
				case 0x14F: cpu_BIT_Func(1, cpu_A); break;
				case 0x150: cpu_BIT_Func(2, cpu_B); break;
				case 0x151: cpu_BIT_Func(2, cpu_C); break;
				case 0x152: cpu_BIT_Func(2, cpu_D); break;
				case 0x153: cpu_BIT_Func(2, cpu_E); break;
				case 0x154: cpu_BIT_Func(2, cpu_H); break;
				case 0x155: cpu_BIT_Func(2, cpu_L); break;
				// cpu_BIT_Func(2, (HL)
				case 0x157: cpu_BIT_Func(2, cpu_A); break;
				case 0x158: cpu_BIT_Func(3, cpu_B); break;
				case 0x159: cpu_BIT_Func(3, cpu_C); break;
				case 0x15A: cpu_BIT_Func(3, cpu_D); break;
				case 0x15B: cpu_BIT_Func(3, cpu_E); break;
				case 0x15C: cpu_BIT_Func(3, cpu_H); break;
				case 0x15D: cpu_BIT_Func(3, cpu_L); break;
				// cpu_BIT_Func(3, (HL)
				case 0x15F: cpu_BIT_Func(3, cpu_A); break;
				case 0x160: cpu_BIT_Func(4, cpu_B); break;
				case 0x161: cpu_BIT_Func(4, cpu_C); break;
				case 0x162: cpu_BIT_Func(4, cpu_D); break;
				case 0x163: cpu_BIT_Func(4, cpu_E); break;
				case 0x164: cpu_BIT_Func(4, cpu_H); break;
				case 0x165: cpu_BIT_Func(4, cpu_L); break;
				// cpu_BIT_Func(4, (HL)
				case 0x167: cpu_BIT_Func(4, cpu_A); break;
				case 0x168: cpu_BIT_Func(5, cpu_B); break;
				case 0x169: cpu_BIT_Func(5, cpu_C); break;
				case 0x16A: cpu_BIT_Func(5, cpu_D); break;
				case 0x16B: cpu_BIT_Func(5, cpu_E); break;
				case 0x16C: cpu_BIT_Func(5, cpu_H); break;
				case 0x16D: cpu_BIT_Func(5, cpu_L); break;
				// cpu_BIT_Func(5, (HL)
				case 0x16F: cpu_BIT_Func(5, cpu_A); break;
				case 0x170: cpu_BIT_Func(6, cpu_B); break;
				case 0x171: cpu_BIT_Func(6, cpu_C); break;
				case 0x172: cpu_BIT_Func(6, cpu_D); break;
				case 0x173: cpu_BIT_Func(6, cpu_E); break;
				case 0x174: cpu_BIT_Func(6, cpu_H); break;
				case 0x175: cpu_BIT_Func(6, cpu_L); break;
				// cpu_BIT_Func(6, (HL)
				case 0x177: cpu_BIT_Func(6, cpu_A); break;
				case 0x178: cpu_BIT_Func(7, cpu_B); break;
				case 0x179: cpu_BIT_Func(7, cpu_C); break;
				case 0x17A: cpu_BIT_Func(7, cpu_D); break;
				case 0x17B: cpu_BIT_Func(7, cpu_E); break;
				case 0x17C: cpu_BIT_Func(7, cpu_H); break;
				case 0x17D: cpu_BIT_Func(7, cpu_L); break;
				// cpu_BIT_Func(7, (HL)
				case 0x17F: cpu_BIT_Func(7, cpu_A); break;
				case 0x180: cpu_RES_Func(0, cpu_B); break;
				case 0x181: cpu_RES_Func(0, cpu_C); break;
				case 0x182: cpu_RES_Func(0, cpu_D); break;
				case 0x183: cpu_RES_Func(0, cpu_E); break;
				case 0x184: cpu_RES_Func(0, cpu_H); break;
				case 0x185: cpu_RES_Func(0, cpu_L); break;
				// cpu_RES_Func(0, (HL)
				case 0x187: cpu_RES_Func(0, cpu_A); break;
				case 0x188: cpu_RES_Func(1, cpu_B); break;
				case 0x189: cpu_RES_Func(1, cpu_C); break;
				case 0x18A: cpu_RES_Func(1, cpu_D); break;
				case 0x18B: cpu_RES_Func(1, cpu_E); break;
				case 0x18C: cpu_RES_Func(1, cpu_H); break;
				case 0x18D: cpu_RES_Func(1, cpu_L); break;
				// cpu_RES_Func(1, (HL)
				case 0x18F: cpu_RES_Func(1, cpu_A); break;
				case 0x190: cpu_RES_Func(2, cpu_B); break;
				case 0x191: cpu_RES_Func(2, cpu_C); break;
				case 0x192: cpu_RES_Func(2, cpu_D); break;
				case 0x193: cpu_RES_Func(2, cpu_E); break;
				case 0x194: cpu_RES_Func(2, cpu_H); break;
				case 0x195: cpu_RES_Func(2, cpu_L); break;
				// cpu_RES_Func(2, (HL)
				case 0x197: cpu_RES_Func(2, cpu_A); break;
				case 0x198: cpu_RES_Func(3, cpu_B); break;
				case 0x199: cpu_RES_Func(3, cpu_C); break;
				case 0x19A: cpu_RES_Func(3, cpu_D); break;
				case 0x19B: cpu_RES_Func(3, cpu_E); break;
				case 0x19C: cpu_RES_Func(3, cpu_H); break;
				case 0x19D: cpu_RES_Func(3, cpu_L); break;
				// cpu_RES_Func(3, (HL)
				case 0x19F: cpu_RES_Func(3, cpu_A); break;
				case 0x1A0: cpu_RES_Func(4, cpu_B); break;
				case 0x1A1: cpu_RES_Func(4, cpu_C); break;
				case 0x1A2: cpu_RES_Func(4, cpu_D); break;
				case 0x1A3: cpu_RES_Func(4, cpu_E); break;
				case 0x1A4: cpu_RES_Func(4, cpu_H); break;
				case 0x1A5: cpu_RES_Func(4, cpu_L); break;
				// cpu_RES_Func(4, (HL)
				case 0x1A7: cpu_RES_Func(4, cpu_A); break;
				case 0x1A8: cpu_RES_Func(5, cpu_B); break;
				case 0x1A9: cpu_RES_Func(5, cpu_C); break;
				case 0x1AA: cpu_RES_Func(5, cpu_D); break;
				case 0x1AB: cpu_RES_Func(5, cpu_E); break;
				case 0x1AC: cpu_RES_Func(5, cpu_H); break;
				case 0x1AD: cpu_RES_Func(5, cpu_L); break;
				// cpu_RES_Func(5, (HL)
				case 0x1AF: cpu_RES_Func(5, cpu_A); break;
				case 0x1B0: cpu_RES_Func(6, cpu_B); break;
				case 0x1B1: cpu_RES_Func(6, cpu_C); break;
				case 0x1B2: cpu_RES_Func(6, cpu_D); break;
				case 0x1B3: cpu_RES_Func(6, cpu_E); break;
				case 0x1B4: cpu_RES_Func(6, cpu_H); break;
				case 0x1B5: cpu_RES_Func(6, cpu_L); break;
				// cpu_RES_Func(6, (HL)
				case 0x1B7: cpu_RES_Func(6, cpu_A); break;
				case 0x1B8: cpu_RES_Func(7, cpu_B); break;
				case 0x1B9: cpu_RES_Func(7, cpu_C); break;
				case 0x1BA: cpu_RES_Func(7, cpu_D); break;
				case 0x1BB: cpu_RES_Func(7, cpu_E); break;
				case 0x1BC: cpu_RES_Func(7, cpu_H); break;
				case 0x1BD: cpu_RES_Func(7, cpu_L); break;
				// cpu_RES_Func(7, (HL)
				case 0x1BF: cpu_RES_Func(7, cpu_A); break;
				case 0x1C0: cpu_SET_Func(0, cpu_B); break;
				case 0x1C1: cpu_SET_Func(0, cpu_C); break;
				case 0x1C2: cpu_SET_Func(0, cpu_D); break;
				case 0x1C3: cpu_SET_Func(0, cpu_E); break;
				case 0x1C4: cpu_SET_Func(0, cpu_H); break;
				case 0x1C5: cpu_SET_Func(0, cpu_L); break;
				// cpu_SET_Func(0, (HL)
				case 0x1C7: cpu_SET_Func(0, cpu_A); break;
				case 0x1C8: cpu_SET_Func(1, cpu_B); break;
				case 0x1C9: cpu_SET_Func(1, cpu_C); break;
				case 0x1CA: cpu_SET_Func(1, cpu_D); break;
				case 0x1CB: cpu_SET_Func(1, cpu_E); break;
				case 0x1CC: cpu_SET_Func(1, cpu_H); break;
				case 0x1CD: cpu_SET_Func(1, cpu_L); break;
				// cpu_SET_Func(1, (HL)
				case 0x1CF: cpu_SET_Func(1, cpu_A); break;
				case 0x1D0: cpu_SET_Func(2, cpu_B); break;
				case 0x1D1: cpu_SET_Func(2, cpu_C); break;
				case 0x1D2: cpu_SET_Func(2, cpu_D); break;
				case 0x1D3: cpu_SET_Func(2, cpu_E); break;
				case 0x1D4: cpu_SET_Func(2, cpu_H); break;
				case 0x1D5: cpu_SET_Func(2, cpu_L); break;
				// cpu_SET_Func(2, (HL)
				case 0x1D7: cpu_SET_Func(2, cpu_A); break;
				case 0x1D8: cpu_SET_Func(3, cpu_B); break;
				case 0x1D9: cpu_SET_Func(3, cpu_C); break;
				case 0x1DA: cpu_SET_Func(3, cpu_D); break;
				case 0x1DB: cpu_SET_Func(3, cpu_E); break;
				case 0x1DC: cpu_SET_Func(3, cpu_H); break;
				case 0x1DD: cpu_SET_Func(3, cpu_L); break;
				// cpu_SET_Func(3, (HL)
				case 0x1DF: cpu_SET_Func(3, cpu_A); break;
				case 0x1E0: cpu_SET_Func(4, cpu_B); break;
				case 0x1E1: cpu_SET_Func(4, cpu_C); break;
				case 0x1E2: cpu_SET_Func(4, cpu_D); break;
				case 0x1E3: cpu_SET_Func(4, cpu_E); break;
				case 0x1E4: cpu_SET_Func(4, cpu_H); break;
				case 0x1E5: cpu_SET_Func(4, cpu_L); break;
				// cpu_SET_Func(4, (HL)
				case 0x1E7: cpu_SET_Func(4, cpu_A); break;
				case 0x1E8: cpu_SET_Func(5, cpu_B); break;
				case 0x1E9: cpu_SET_Func(5, cpu_C); break;
				case 0x1EA: cpu_SET_Func(5, cpu_D); break;
				case 0x1EB: cpu_SET_Func(5, cpu_E); break;
				case 0x1EC: cpu_SET_Func(5, cpu_H); break;
				case 0x1ED: cpu_SET_Func(5, cpu_L); break;
				// cpu_SET_Func(5, (HL)
				case 0x1EF: cpu_SET_Func(5, cpu_A); break;
				case 0x1F0: cpu_SET_Func(6, cpu_B); break;
				case 0x1F1: cpu_SET_Func(6, cpu_C); break;
				case 0x1F2: cpu_SET_Func(6, cpu_D); break;
				case 0x1F3: cpu_SET_Func(6, cpu_E); break;
				case 0x1F4: cpu_SET_Func(6, cpu_H); break;
				case 0x1F5: cpu_SET_Func(6, cpu_L); break;
				// cpu_SET_Func(6, (HL)
				case 0x1F7: cpu_SET_Func(6, cpu_A); break;
				case 0x1F8: cpu_SET_Func(7, cpu_B); break;
				case 0x1F9: cpu_SET_Func(7, cpu_C); break;
				case 0x1FA: cpu_SET_Func(7, cpu_D); break;
				case 0x1FB: cpu_SET_Func(7, cpu_E); break;
				case 0x1FC: cpu_SET_Func(7, cpu_H); break;
				case 0x1FD: cpu_SET_Func(7, cpu_L); break;
				// cpu_SET_Func(7, (HL)
				case 0x1FF: cpu_SET_Func(7, cpu_A); break;
			}
		}

		inline uint32_t SpeedFunc(uint32_t val);

		void cpu_Reset()
		{
			cpu_was_FlagI = cpu_FlagI = false;
			cpu_Jammed = cpu_Stopped = cpu_Halted = false;
			cpu_CB_Prefix = false;
			cpu_Interrupts_Enabled = false;

			cpu_I_Use = false;
			cpu_Skip_Once = false;
			cpu_Halt_bug_2 = false;
			cpu_Halt_bug_3 = false;
			cpu_Halt_bug_4 = false;
			cpu_Halt_bug_5 = false;

			cpu_Opcode = 0;
			cpu_Instr_Cycle = 0;
			cpu_Instr_Cycle_Hold = 0;
			cpu_Instr_Type = OpT::RESET;


			cpu_Int_Src = 5;
			cpu_Int_Clear = 0;

			cpu_Instruction_Start = 0;
			
			for (int i = 0; i < 14; i++)
			{
				cpu_Regs[i] = 0;
			}
		}

		inline void cpu_Halt_Check_Func()
		{
			cpu_I_Use = cpu_FlagI;
			if (cpu_Halt_bug_2 && cpu_I_Use)
			{
				cpu_RegPCset(cpu_RegPCget() - 1);
				cpu_Halt_bug_3 = true;
				//Console.WriteLine("Halt_bug_3");
				//Console.WriteLine(TotalExecutedCycles);
			}

			cpu_Halt_bug_2 = false;
		}

		inline void cpu_Op_Func()
		{
			// Read the opcode of the next instruction
			if (cpu_EI_Pending > 0 && !cpu_CB_Prefix)
			{
				cpu_EI_Pending--;
				if (cpu_EI_Pending == 0)
				{
					cpu_Interrupts_Enabled = true;
				}
			}

			if (cpu_I_Use && cpu_Interrupts_Enabled && !cpu_CB_Prefix && !cpu_Jammed)
			{
				cpu_Interrupts_Enabled = false;

				if (TraceCallback) TraceCallback(3); // IRQ

				// call interrupt processor
				// lowest bit set is highest priority
				cpu_Instr_Type = OpT::INTRPT; // point to Interrupt
			}
			else
			{
				if (TraceCallback && !cpu_CB_Prefix) TraceCallback(0);

				cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget()));
				cpu_RegPCset((uint16_t)(cpu_RegPCget() + 1));
				cpu_Instr_Cycle = -1;
			}

			cpu_Instruction_Start = Cycle_Count + 1;
			cpu_I_Use = false;
		}

		inline void cpu_Op_G_Func()
		{
			if (TraceCallback) TraceCallback(0);

			cpu_FetchInstruction(GB_System::Read_Memory(cpu_RegPCget()));
			// note no increment
			cpu_Instr_Cycle = -1;
		}

		inline void cpu_FetchInstruction(int op)
		{
			cpu_Opcode = op;
			cpu_Instr_Type = cpu_Instr_Type_List[op];

			if (cpu_CB_Prefix)
			{
				cpu_Opcode += 256;
				cpu_Instr_Type = cpu_Instr_Type_List[cpu_Opcode];
			}

			cpu_CB_Prefix = false;

			cpu_was_FlagI = cpu_FlagI;
		}

		inline void cpu_Halt_Enter();

		inline void cpu_Halt_Ex(uint8_t param);

		void cpu_Execute_One();

		inline void cpu_STOP_Ex();

		void cpu_TR_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = cpu_Regs[src];
		}

		void cpu_ADD8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[dest];
			cpu_Reg16_dt += cpu_Regs[src];

			cpu_FlagCset(((cpu_Reg16_dt & 0x100) == 0x100));
			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			cpu_ans = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[dest] & 0xF;
			cpu_Reg16_dt += (cpu_Regs[src] & 0xF);

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));

			cpu_FlagNset(false);

			cpu_Regs[dest] = (uint8_t) cpu_ans;
		}

		void cpu_SUB8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[dest];
			cpu_Reg16_dt -= cpu_Regs[src];

			cpu_FlagCset(((cpu_Reg16_dt & 0x100) == 0x100));
			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			cpu_ans = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[dest] & 0xF;
			cpu_Reg16_dt -= (cpu_Regs[src] & 0xF);

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));
			cpu_FlagNset(true);

			cpu_Regs[dest] = (uint8_t) cpu_ans;
		}

		void cpu_BIT_Func(uint16_t bit, uint16_t src)
		{
			cpu_FlagZset(!((cpu_Regs[src] & (1 << bit)) == (1 << bit)));
			cpu_FlagHset(true);
			cpu_FlagNset(false);
		}

		void cpu_SET_Func(uint16_t bit, uint16_t src)
		{
			cpu_Regs[src] |= (uint8_t)(1 << bit);
		}

		void cpu_RES_Func(uint16_t bit, uint16_t src)
		{
			cpu_Regs[src] &= (uint8_t)(0xFF - (1 << bit));
		}

		void cpu_ASGN_Func(uint16_t src, uint16_t val)
		{
			cpu_Regs[src] = (uint8_t) val;
		}

		void cpu_SWAP_Func(uint16_t src)
		{
			cpu_temp = (uint16_t)((cpu_Regs[src] << 4) & 0xF0);
			cpu_Regs[src] = (uint8_t)(cpu_temp | (cpu_Regs[src] >> 4));

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
			cpu_FlagCset(false);
		}

		void cpu_SLA_Func(uint16_t src)
		{
			cpu_FlagCset(((cpu_Regs[src] & 0x80) == 0x80));

			cpu_Regs[src] = (uint8_t)((cpu_Regs[src] << 1) & 0xFF);

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_SRA_Func(uint16_t src)
		{
			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_temp = (uint16_t)(cpu_Regs[src] & 0x80); // MSB doesn't change in this operation

			cpu_Regs[src] = (uint8_t)((cpu_Regs[src] >> 1) | cpu_temp);

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_SRL_Func(uint16_t src)
		{
			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_Regs[src] = (uint8_t)(cpu_Regs[src] >> 1);

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_CPL_Func(uint16_t src)
		{
			cpu_Regs[src] = (uint8_t)((~cpu_Regs[src]) & 0xFF);

			cpu_FlagHset(true);
			cpu_FlagNset(true);
		}

		void cpu_CCF_Func(uint16_t src)
		{
			cpu_FlagCset(!cpu_FlagCget());
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_SCF_Func(uint16_t src)
		{
			cpu_FlagCset(true);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_AND8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = (uint8_t)(cpu_Regs[dest] & cpu_Regs[src]);

			cpu_FlagZset(cpu_Regs[dest] == 0);
			cpu_FlagCset(false);
			cpu_FlagHset(true);
			cpu_FlagNset(false);
		}

		void cpu_OR8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = (uint8_t)(cpu_Regs[dest] | cpu_Regs[src]);

			cpu_FlagZset(cpu_Regs[dest] == 0);
			cpu_FlagCset(false);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_XOR8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = (uint8_t)(cpu_Regs[dest] ^ cpu_Regs[src]);

			cpu_FlagZset(cpu_Regs[dest] == 0);
			cpu_FlagCset(false);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_CP8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[dest];
			cpu_Reg16_dt -= cpu_Regs[src];

			cpu_FlagCset(((cpu_Reg16_dt & 0x100) == 0x100));
			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[dest] & 0xF;
			cpu_Reg16_dt -= (cpu_Regs[src] & 0xF);

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));

			cpu_FlagNset(true);
		}

		void cpu_RRC_Func(uint16_t src)
		{
			cpu_imm = (src == cpu_Aim);
			if (cpu_imm) { src = cpu_A; }

			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_Regs[src] = (uint16_t)((cpu_FlagCget() ? 0x80 : 0) | (cpu_Regs[src] >> 1));

			cpu_FlagZset(cpu_imm ? false : (cpu_Regs[src] == 0));
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_RR_Func(uint16_t src)
		{
			cpu_imm = (src == cpu_Aim);
			if (cpu_imm) { src = cpu_A; }

			cpu_ct = cpu_FlagCget() ? 0x80 : 0;

			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_Regs[src] = (uint8_t)(cpu_ct | (cpu_Regs[src] >> 1));

			cpu_FlagZset(cpu_imm ? false : (cpu_Regs[src] == 0));
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}


		void cpu_RL_Func(uint16_t src)
		{
			cpu_imm = src == cpu_Aim;
			if (cpu_imm) { src = cpu_A; }

			cpu_ct = cpu_FlagCget() ? 1 : 0;
			cpu_FlagCset(((cpu_Regs[src] & 0x80) == 0x80));

			cpu_Regs[src] = (uint8_t)(((cpu_Regs[src] << 1) & 0xFF) | cpu_ct);

			cpu_FlagZset(cpu_imm ? false : (cpu_Regs[src] == 0));
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_ADC8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[dest];
			cpu_ct = cpu_FlagCget() ? 1 : 0;

			cpu_Reg16_dt += (cpu_Regs[src] + cpu_ct);

			cpu_FlagCset(((cpu_Reg16_dt & 0x100) == 0x100));
			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			cpu_ans = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[dest] & 0xF;
			cpu_Reg16_dt += ((cpu_Regs[src] & 0xF) + cpu_ct);

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));
			cpu_FlagNset(false);

			cpu_Regs[dest] = (uint8_t)cpu_ans;
		}

		void cpu_SBC8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[dest];
			cpu_ct = cpu_FlagCget() ? 1 : 0;

			cpu_Reg16_dt -= (cpu_Regs[src] + cpu_ct);

			cpu_FlagCset(((cpu_Reg16_dt & 0x100) == 0x100));
			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			cpu_ans = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[dest] & 0xF;
			cpu_Reg16_dt -= ((cpu_Regs[src] & 0xF) + cpu_ct);

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));
			cpu_FlagNset(true);

			cpu_Regs[dest] = (uint8_t)cpu_ans;
		}

		// DA code courtesy of AWJ: http://forums.nesdev.com/viewtopic.php?f=20&t=15944
		void cpu_DA_Func(uint16_t src)
		{
			cpu_a_d = cpu_Regs[src];

			if (!cpu_FlagNget())
			{  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
				if (cpu_FlagCget() || cpu_a_d > 0x99) { cpu_a_d += 0x60; cpu_FlagCset(true); }
				if (cpu_FlagHget() || (cpu_a_d & 0x0f) > 0x09) { cpu_a_d += 0x6; }
			}
			else
			{  // after a subtraction, only adjust if (half-)carry occurred
				if (cpu_FlagCget()) { cpu_a_d -= 0x60; }
				if (cpu_FlagHget()) { cpu_a_d -= 0x6; }
			}

			cpu_a_d &= 0xFF;

			cpu_Regs[src] = cpu_a_d;

			cpu_FlagZset(cpu_a_d == 0);
			cpu_FlagHset(false);
		}

		// used for signed operations
		void cpu_ADDS_Func(uint16_t dest_l, uint16_t dest_h, uint16_t src_l, uint16_t src_h)
		{
			cpu_Reg16_dt = cpu_Regs[dest_l];
			cpu_Reg16_st = cpu_Regs[src_l];

			cpu_Reg16_dt += cpu_Reg16_st;

			cpu_temp = 0;

			// since this is signed addition, calculate the high byte carry appropriately
			if (((cpu_Reg16_st & 0x80) == 0x80))
			{
				if (((cpu_Reg16_dt & 0xFF) >= cpu_Regs[dest_l]))
				{
					cpu_temp = 0xFF;
				}
				else
				{
					cpu_temp = 0;
				}
			}
			else
			{
				cpu_temp = (uint16_t)(((cpu_Reg16_dt & 0x100) == 0x100) ? 1 : 0);
			}

			cpu_ans_l = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// JR operations do not effect flags
			if (dest_l != cpu_PCl)
			{
				cpu_FlagCset(((cpu_Reg16_dt & 0x100) == 0x100));

				// redo for half carry flag
				cpu_Reg16_dt = cpu_Regs[dest_l] & 0xF;
				cpu_Reg16_dt += cpu_Regs[src_l] & 0xF;

				cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));
				cpu_FlagNset(false);
				cpu_FlagZset(false);
			}

			cpu_Regs[dest_l] = cpu_ans_l;
			cpu_Regs[dest_h] += cpu_temp;
			cpu_Regs[dest_h] &= 0xFF;

		}

		void cpu_ADD16_Func(uint16_t dest_l, uint16_t dest_h, uint16_t src_l, uint16_t src_h)
		{
			cpu_Reg16_dt = cpu_Regs[dest_l] | (cpu_Regs[dest_h] << 8);
			cpu_Reg16_st = cpu_Regs[src_l] | (cpu_Regs[src_h] << 8);

			cpu_Reg16_dt += cpu_Reg16_st;

			cpu_FlagCset(((cpu_Reg16_dt & 0x10000) == 0x10000));

			cpu_ans_l = (uint16_t)(cpu_Reg16_dt & 0xFF);
			cpu_ans_h = (uint16_t)((cpu_Reg16_dt & 0xFF00) >> 8);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[dest_l] | ((cpu_Regs[dest_h] & 0x0F) << 8);
			cpu_Reg16_st = cpu_Regs[src_l] | ((cpu_Regs[src_h] & 0x0F) << 8);

			cpu_Reg16_dt += cpu_Reg16_st;

			cpu_FlagHset(((cpu_Reg16_dt & 0x1000) == 0x1000));
			cpu_FlagNset(false);

			cpu_Regs[dest_l] = cpu_ans_l;
			cpu_Regs[dest_h] = cpu_ans_h;
		}


		void cpu_INC8_Func(uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[src];
			cpu_Reg16_dt += 1;

			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			cpu_ans = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[src] & 0xF;
			cpu_Reg16_dt += 1;

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));
			cpu_FlagNset(false);

			cpu_Regs[src] = (uint8_t) cpu_ans;
		}

		void cpu_DEC8_Func(uint16_t src)
		{
			cpu_Reg16_dt = cpu_Regs[src];
			cpu_Reg16_dt -= 1;

			cpu_FlagZset((cpu_Reg16_dt & 0xFF) == 0);

			cpu_ans = (uint16_t)(cpu_Reg16_dt & 0xFF);

			// redo for half carry flag
			cpu_Reg16_dt = cpu_Regs[src] & 0xF;
			cpu_Reg16_dt -= 1;

			cpu_FlagHset(((cpu_Reg16_dt & 0x10) == 0x10));
			cpu_FlagNset(true);

			cpu_Regs[src] = cpu_ans;
		}

		void cpu_RLC_Func(uint16_t src)
		{
			cpu_imm = (src == cpu_Aim);
			if (cpu_imm) { src = cpu_A; }

			cpu_ct = ((cpu_Regs[src]& 0x80) == 0x80) ? 1 : 0;
			cpu_FlagCset(((cpu_Regs[src] & 0x80) == 0x80));

			cpu_Regs[src] = (uint8_t)(((cpu_Regs[src] << 1) & 0xFF) | cpu_ct);

			cpu_FlagZset(cpu_imm ? false : (cpu_Regs[src] == 0));
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void cpu_IRQ_Clear_Func()
		{
			uint8_t interrupt_src_reg = GetIntRegs(0);
			uint8_t interrupt_enable_reg = GetIntRegs(1);

			if ((interrupt_src_reg & (1 <<cpu_Int_Src)) == (1 << cpu_Int_Src)) { interrupt_src_reg -= cpu_Int_Clear; }

			SetIntRegs(interrupt_src_reg);

			if ((interrupt_src_reg & (interrupt_enable_reg & 0x1F)) == 0) { cpu_FlagI = false; }
			// reset back to default state
			cpu_Int_Src = 5;
			cpu_Int_Clear = 0;
		}

		void cpu_Int_Get_Func(uint16_t reg, uint8_t value)
		{
			// check if any interrupts got cancelled along the way
			// interrupt src = 5 sets the PC to zero as observed
			// also the triggering interrupt seems like it is held low (i.e. cannot trigger I flag) until the interrupt is serviced
			uint16_t bit_check = value;
			//Console.WriteLine("int " + TotalExecutedCycles);

			uint8_t interrupt_src_reg = GetIntRegs(0);
			uint8_t interrupt_enable_reg = GetIntRegs(1);

			//if (interrupt_src_reg.Bit(bit_check) && interrupt_enable_reg.Bit(bit_check)) { cpu_Int_Src = bit_check; cpu_Int_Clear = (uint8_t)(1 << bit_check); }

			if ((interrupt_src_reg & interrupt_enable_reg & 1) == 1) { cpu_Int_Src = 0; cpu_Int_Clear = 1; }
			else if ((interrupt_src_reg & interrupt_enable_reg & 2) == 2) { cpu_Int_Src = 1; cpu_Int_Clear = 2; }
			else if ((interrupt_src_reg & interrupt_enable_reg & 4) == 4) { cpu_Int_Src = 2; cpu_Int_Clear = 4; }
			else if ((interrupt_src_reg & interrupt_enable_reg & 8) == 8) { cpu_Int_Src = 3; cpu_Int_Clear = 8; }
			else if ((interrupt_src_reg & interrupt_enable_reg & 0x10) == 0x10) { cpu_Int_Src = 4; cpu_Int_Clear = 16; }
			else { cpu_Int_Src = 5; cpu_Int_Clear = 0; }

			cpu_Regs[reg] = INT_vectors[cpu_Int_Src];

		}

		#pragma endregion

		#pragma region Disassemble

		// disassemblies will also return strings of the same length
		const char* TraceHeader = "LR35902: PC, machine code, mnemonic, operands, registers (A, F, B, C, D, E, H, L, SP), Cy, flags (ZNHCI)";
		const char* UNS_event =  "            ====Un-STOP====              ";
		const char* UNH_event =  "            ====Un-HALT====              ";
		const char* IRQ_event =  "              ====IRQ====                ";
		const char* HALT_event = "              ====HALT====               ";
		const char* DMA_event =  "              ====DMA====                ";
		const char* No_Reg = 
			"                                                                                                      ";
		const char* Reg_template = 
			"A:XX F:XX B:XX C:XX D:XX E:XX H:XX L:XX SP:XXXX Cy:XXXXXXXXXXXXXXXX LY:XXX ZNHCI F-Cy:XXXXXXXXXXXXXXXX";
		const char* Disasm_template = "PCPC:   AA BB   Di Di Di Di Di Di Di Di  ";

		char replacer[300] = {};
		char* val_char_1 = nullptr;
		char* val_char_2 = nullptr;
		uint32_t temp_reg;

		uint32_t op_size = 0;

		void (*TraceCallback)(int);
		void (*RumbleCallback)(bool);

		string CPUDMAState()
		{
			val_char_1 = replacer;

			string reg_state = "CH:";

			temp_reg = cpu_Regs[0];
			sprintf_s(val_char_1, 9, "%08X", 0);
			reg_state.append(val_char_1, 8);

			reg_state.append(" SR:");
			temp_reg = cpu_Regs[1];
			sprintf_s(val_char_1, 9, "%08X", 0);
			reg_state.append(val_char_1, 8);

			reg_state.append(" DT:");
			temp_reg = cpu_Regs[2];
			sprintf_s(val_char_1, 9, "%08X", 0);
			reg_state.append(val_char_1, 8);

			reg_state.append(" CR:");
			temp_reg = cpu_Regs[3];
			sprintf_s(val_char_1, 9, "%08X", 0);
			reg_state.append(val_char_1, 8);

			reg_state.append(" CT:");
			temp_reg = cpu_Regs[3];
			sprintf_s(val_char_1, 9, "%08X", 0);
			reg_state.append(val_char_1, 8);

			reg_state.append(" Cy:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", Cycle_Count));

			while (reg_state.length() < 103)
			{
				reg_state.append(" ");
			}

			return reg_state;
		}

		string CPURegisterState()
		{		
			val_char_1 = replacer;

			string reg_state = "A:";

			temp_reg = cpu_Regs[cpu_A];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" F:");			
			temp_reg = cpu_Regs[cpu_F];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" B:");
			temp_reg = cpu_Regs[cpu_B];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" C:");
			temp_reg = cpu_Regs[cpu_C];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" D:");
			temp_reg = cpu_Regs[cpu_D];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" E:");
			temp_reg = cpu_Regs[cpu_E];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" H:");
			temp_reg = cpu_Regs[cpu_H];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" L:");
			temp_reg = cpu_Regs[cpu_L];
			sprintf_s(val_char_1, 3, "%02X", temp_reg);
			reg_state.append(val_char_1, 2);

			reg_state.append(" SP:");
			temp_reg = cpu_RegSPget();
			sprintf_s(val_char_1, 5, "%04X", temp_reg);
			reg_state.append(val_char_1, 4);

			reg_state.append(" Cy:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", Cycle_Count));

			reg_state.append(" LY:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 4, "%3u", ppu_LY_pntr[0]));
			reg_state.append(" ");
			
			reg_state.append(cpu_FlagZget() ? "Z" : "z");
			reg_state.append(cpu_FlagNget() ? "N" : "n");
			reg_state.append(cpu_FlagHget() ? "H" : "h");
			reg_state.append(cpu_FlagCget() ? "C" : "c");		
			reg_state.append(cpu_FlagI ? "I" : "i");

			reg_state.append(" F-Cy:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", Frame_Cycle));

			return reg_state;
		}

		string CPUDisassembly()
		{
			string trace_string = "";

			string disasm = cpu_Disassemble(cpu_RegPCget());
			
			val_char_1 = replacer;

			uint16_t dis_pc = cpu_RegPCget();

			sprintf_s(val_char_1, 5, "%04X", dis_pc);
			trace_string.append(val_char_1, 4);

			trace_string.append(":  ");

			for (uint32_t i = 0; i < op_size; i++)
			{
				sprintf_s(val_char_1, 3, "%02X", Peek_Memory(dis_pc++));

				trace_string.append(val_char_1, 2);
				trace_string.append(" ");
			}

			while (trace_string.length() < 18)
			{
				trace_string.append(" ");
			}

			trace_string.append(disasm);

			while (trace_string.length() < 42)
			{
				trace_string.append(" ");
			}

			return trace_string;
		}

		string cpu_Disassemble(uint16_t pc)
		{
			uint16_t diff = pc;

			uint16_t op = Peek_Memory(pc++);

			if (op == 0xCB)
			{
				op = Peek_Memory(pc++);
				op += 256;
			}

			string ret = "";

			val_char_2 = replacer;

			switch (op)
			{
				case 0x00: sprintf_s(val_char_2, 40, "NOP"); break;
				case 0x01: sprintf_s(val_char_2, 40, "LD   BC,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x02: sprintf_s(val_char_2, 40, "LD   (BC),A"); break;
				case 0x03: sprintf_s(val_char_2, 40, "INC  BC"); break;
				case 0x04: sprintf_s(val_char_2, 40, "INC  B"); break;
				case 0x05: sprintf_s(val_char_2, 40, "DEC  B"); break;
				case 0x06: sprintf_s(val_char_2, 40, "LD   B,$%02X", Peek_Memory(pc++)); break;
				case 0x07: sprintf_s(val_char_2, 40, "RLCA"); break;
				case 0x08: sprintf_s(val_char_2, 40, "LD   ($%04X),SP", Peek_Memory_16(pc++)); pc++; break;
				case 0x09: sprintf_s(val_char_2, 40, "ADD  HL,BC"); break;
				case 0x0A: sprintf_s(val_char_2, 40, "LD   A,(BC)"); break;
				case 0x0B: sprintf_s(val_char_2, 40, "DEC  BC"); break;
				case 0x0C: sprintf_s(val_char_2, 40, "INC  C"); break;
				case 0x0D: sprintf_s(val_char_2, 40, "DEC  C"); break;
				case 0x0E: sprintf_s(val_char_2, 40, "LD   C,$%02X", Peek_Memory(pc++)); break;
				case 0x0F: sprintf_s(val_char_2, 40, "RRCA"); break;
				case 0x10: sprintf_s(val_char_2, 40, "STOP $%02X", Peek_Memory(pc++)); break;
				case 0x11: sprintf_s(val_char_2, 40, "LD   DE,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x12: sprintf_s(val_char_2, 40, "LD   (DE),A"); break;
				case 0x13: sprintf_s(val_char_2, 40, "INC  DE"); break;
				case 0x14: sprintf_s(val_char_2, 40, "INC  D"); break;
				case 0x15: sprintf_s(val_char_2, 40, "DEC  D"); break;
				case 0x16: sprintf_s(val_char_2, 40, "LD   D,$%02X", Peek_Memory(pc++)); break;
				case 0x17: sprintf_s(val_char_2, 40, "RLA"); break;
				case 0x18: sprintf_s(val_char_2, 40, "JR   $%04d", Peek_Memory_Signed(pc++)); break;
				case 0x19: sprintf_s(val_char_2, 40, "ADD  HL,DE"); break;
				case 0x1A: sprintf_s(val_char_2, 40, "LD   A,(DE)"); break;
				case 0x1B: sprintf_s(val_char_2, 40, "DEC  DE"); break;
				case 0x1C: sprintf_s(val_char_2, 40, "INC  E"); break;
				case 0x1D: sprintf_s(val_char_2, 40, "DEC  E"); break;
				case 0x1E: sprintf_s(val_char_2, 40, "LD   E,$%02X", Peek_Memory(pc++)); break;
				case 0x1F: sprintf_s(val_char_2, 40, "RRA"); break;
				case 0x20: sprintf_s(val_char_2, 40, "JR   NZ,$%04d", Peek_Memory_Signed(pc++)); break;
				case 0x21: sprintf_s(val_char_2, 40, "LD   HL,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x22: sprintf_s(val_char_2, 40, "LD   (HL+),A"); break;
				case 0x23: sprintf_s(val_char_2, 40, "INC  HL"); break;
				case 0x24: sprintf_s(val_char_2, 40, "INC  H"); break;
				case 0x25: sprintf_s(val_char_2, 40, "DEC  H"); break;
				case 0x26: sprintf_s(val_char_2, 40, "LD   H,$%02X", Peek_Memory(pc++)); break;
				case 0x27: sprintf_s(val_char_2, 40, "DAA"); break;
				case 0x28: sprintf_s(val_char_2, 40, "JR   Z,$%04d", Peek_Memory_Signed(pc++)); break;
				case 0x29: sprintf_s(val_char_2, 40, "ADD  HL,HL"); break;
				case 0x2A: sprintf_s(val_char_2, 40, "LD   A,(HL+)"); break;
				case 0x2B: sprintf_s(val_char_2, 40, "DEC  HL"); break;
				case 0x2C: sprintf_s(val_char_2, 40, "INC  L"); break;
				case 0x2D: sprintf_s(val_char_2, 40, "DEC  L"); break;
				case 0x2E: sprintf_s(val_char_2, 40, "LD   L,$%02X", Peek_Memory(pc++)); break;
				case 0x2F: sprintf_s(val_char_2, 40, "CPL"); break;
				case 0x30: sprintf_s(val_char_2, 40, "JR   NC,$%04d", Peek_Memory_Signed(pc++)); break;
				case 0x31: sprintf_s(val_char_2, 40, "LD   SP,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0x32: sprintf_s(val_char_2, 40, "LD   (HL-),A"); break;
				case 0x33: sprintf_s(val_char_2, 40, "INC  SP"); break;
				case 0x34: sprintf_s(val_char_2, 40, "INC  (HL)"); break;
				case 0x35: sprintf_s(val_char_2, 40, "DEC  (HL)"); break;
				case 0x36: sprintf_s(val_char_2, 40, "LD   (HL),$%02X", Peek_Memory(pc++)); break;
				case 0x37: sprintf_s(val_char_2, 40, "SCF"); break;
				case 0x38: sprintf_s(val_char_2, 40, "JR   C,$%04d", Peek_Memory_Signed(pc++)); break;
				case 0x39: sprintf_s(val_char_2, 40, "ADD  HL,SP"); break;
				case 0x3A: sprintf_s(val_char_2, 40, "LD   A,(HL-)"); break;
				case 0x3B: sprintf_s(val_char_2, 40, "DEC  SP"); break;
				case 0x3C: sprintf_s(val_char_2, 40, "INC  A"); break;
				case 0x3D: sprintf_s(val_char_2, 40, "DEC  A"); break;
				case 0x3E: sprintf_s(val_char_2, 40, "LD   A,$%02X", Peek_Memory(pc++)); break;
				case 0x3F: sprintf_s(val_char_2, 40, "CCF"); break;
				case 0x40: sprintf_s(val_char_2, 40, "LD   B,B"); break;
				case 0x41: sprintf_s(val_char_2, 40, "LD   B,C"); break;
				case 0x42: sprintf_s(val_char_2, 40, "LD   B,D"); break;
				case 0x43: sprintf_s(val_char_2, 40, "LD   B,E"); break;
				case 0x44: sprintf_s(val_char_2, 40, "LD   B,H"); break;
				case 0x45: sprintf_s(val_char_2, 40, "LD   B,L"); break;
				case 0x46: sprintf_s(val_char_2, 40, "LD   B,(HL)"); break;
				case 0x47: sprintf_s(val_char_2, 40, "LD   B,A"); break;
				case 0x48: sprintf_s(val_char_2, 40, "LD   C,B"); break;
				case 0x49: sprintf_s(val_char_2, 40, "LD   C,C"); break;
				case 0x4A: sprintf_s(val_char_2, 40, "LD   C,D"); break;
				case 0x4B: sprintf_s(val_char_2, 40, "LD   C,E"); break;
				case 0x4C: sprintf_s(val_char_2, 40, "LD   C,H"); break;
				case 0x4D: sprintf_s(val_char_2, 40, "LD   C,L"); break;
				case 0x4E: sprintf_s(val_char_2, 40, "LD   C,(HL)"); break;
				case 0x4F: sprintf_s(val_char_2, 40, "LD   C,A"); break;
				case 0x50: sprintf_s(val_char_2, 40, "LD   D,B"); break;
				case 0x51: sprintf_s(val_char_2, 40, "LD   D,C"); break;
				case 0x52: sprintf_s(val_char_2, 40, "LD   D,D"); break;
				case 0x53: sprintf_s(val_char_2, 40, "LD   D,E"); break;
				case 0x54: sprintf_s(val_char_2, 40, "LD   D,H"); break;
				case 0x55: sprintf_s(val_char_2, 40, "LD   D,L"); break;
				case 0x56: sprintf_s(val_char_2, 40, "LD   D,(HL)"); break;
				case 0x57: sprintf_s(val_char_2, 40, "LD   D,A"); break;
				case 0x58: sprintf_s(val_char_2, 40, "LD   E,B"); break;
				case 0x59: sprintf_s(val_char_2, 40, "LD   E,C"); break;
				case 0x5A: sprintf_s(val_char_2, 40, "LD   E,D"); break;
				case 0x5B: sprintf_s(val_char_2, 40, "LD   E,E"); break;
				case 0x5C: sprintf_s(val_char_2, 40, "LD   E,H"); break;
				case 0x5D: sprintf_s(val_char_2, 40, "LD   E,L"); break;
				case 0x5E: sprintf_s(val_char_2, 40, "LD   E,(HL)"); break;
				case 0x5F: sprintf_s(val_char_2, 40, "LD   E,A"); break;
				case 0x60: sprintf_s(val_char_2, 40, "LD   H,B"); break;
				case 0x61: sprintf_s(val_char_2, 40, "LD   H,C"); break;
				case 0x62: sprintf_s(val_char_2, 40, "LD   H,D"); break;
				case 0x63: sprintf_s(val_char_2, 40, "LD   H,E"); break;
				case 0x64: sprintf_s(val_char_2, 40, "LD   H,H"); break;
				case 0x65: sprintf_s(val_char_2, 40, "LD   H,L"); break;
				case 0x66: sprintf_s(val_char_2, 40, "LD   H,(HL)"); break;
				case 0x67: sprintf_s(val_char_2, 40, "LD   H,A"); break;
				case 0x68: sprintf_s(val_char_2, 40, "LD   L,B"); break;
				case 0x69: sprintf_s(val_char_2, 40, "LD   L,C"); break;
				case 0x6A: sprintf_s(val_char_2, 40, "LD   L,D"); break;
				case 0x6B: sprintf_s(val_char_2, 40, "LD   L,E"); break;
				case 0x6C: sprintf_s(val_char_2, 40, "LD   L,H"); break;
				case 0x6D: sprintf_s(val_char_2, 40, "LD   L,L"); break;
				case 0x6E: sprintf_s(val_char_2, 40, "LD   L,(HL)"); break;
				case 0x6F: sprintf_s(val_char_2, 40, "LD   L,A"); break;
				case 0x70: sprintf_s(val_char_2, 40, "LD   (HL),B"); break;
				case 0x71: sprintf_s(val_char_2, 40, "LD   (HL),C"); break;
				case 0x72: sprintf_s(val_char_2, 40, "LD   (HL),D"); break;
				case 0x73: sprintf_s(val_char_2, 40, "LD   (HL),E"); break;
				case 0x74: sprintf_s(val_char_2, 40, "LD   (HL),H"); break;
				case 0x75: sprintf_s(val_char_2, 40, "LD   (HL),L"); break;
				case 0x76: sprintf_s(val_char_2, 40, "HALT"); break;
				case 0x77: sprintf_s(val_char_2, 40, "LD   (HL),A"); break;
				case 0x78: sprintf_s(val_char_2, 40, "LD   A,B"); break;
				case 0x79: sprintf_s(val_char_2, 40, "LD   A,C"); break;
				case 0x7A: sprintf_s(val_char_2, 40, "LD   A,D"); break;
				case 0x7B: sprintf_s(val_char_2, 40, "LD   A,E"); break;
				case 0x7C: sprintf_s(val_char_2, 40, "LD   A,H"); break;
				case 0x7D: sprintf_s(val_char_2, 40, "LD   A,L"); break;
				case 0x7E: sprintf_s(val_char_2, 40, "LD   A,(HL)"); break;
				case 0x7F: sprintf_s(val_char_2, 40, "LD   A,A"); break;
				case 0x80: sprintf_s(val_char_2, 40, "ADD  A,B"); break;
				case 0x81: sprintf_s(val_char_2, 40, "ADD  A,C"); break;
				case 0x82: sprintf_s(val_char_2, 40, "ADD  A,D"); break;
				case 0x83: sprintf_s(val_char_2, 40, "ADD  A,E"); break;
				case 0x84: sprintf_s(val_char_2, 40, "ADD  A,H"); break;
				case 0x85: sprintf_s(val_char_2, 40, "ADD  A,L"); break;
				case 0x86: sprintf_s(val_char_2, 40, "ADD  A,(HL)"); break;
				case 0x87: sprintf_s(val_char_2, 40, "ADD  A,A"); break;
				case 0x88: sprintf_s(val_char_2, 40, "ADC  A,B"); break;
				case 0x89: sprintf_s(val_char_2, 40, "ADC  A,C"); break;
				case 0x8A: sprintf_s(val_char_2, 40, "ADC  A,D"); break;
				case 0x8B: sprintf_s(val_char_2, 40, "ADC  A,E"); break;
				case 0x8C: sprintf_s(val_char_2, 40, "ADC  A,H"); break;
				case 0x8D: sprintf_s(val_char_2, 40, "ADC  A,L"); break;
				case 0x8E: sprintf_s(val_char_2, 40, "ADC  A,(HL)"); break;
				case 0x8F: sprintf_s(val_char_2, 40, "ADC  A,A"); break;
				case 0x90: sprintf_s(val_char_2, 40, "SUB  B"); break;
				case 0x91: sprintf_s(val_char_2, 40, "SUB  C"); break;
				case 0x92: sprintf_s(val_char_2, 40, "SUB  D"); break;
				case 0x93: sprintf_s(val_char_2, 40, "SUB  E"); break;
				case 0x94: sprintf_s(val_char_2, 40, "SUB  H"); break;
				case 0x95: sprintf_s(val_char_2, 40, "SUB  L"); break;
				case 0x96: sprintf_s(val_char_2, 40, "SUB  (HL)"); break;
				case 0x97: sprintf_s(val_char_2, 40, "SUB  A"); break;
				case 0x98: sprintf_s(val_char_2, 40, "SBC  A,B"); break;
				case 0x99: sprintf_s(val_char_2, 40, "SBC  A,C"); break;
				case 0x9A: sprintf_s(val_char_2, 40, "SBC  A,D"); break;
				case 0x9B: sprintf_s(val_char_2, 40, "SBC  A,E"); break;
				case 0x9C: sprintf_s(val_char_2, 40, "SBC  A,H"); break;
				case 0x9D: sprintf_s(val_char_2, 40, "SBC  A,L"); break;
				case 0x9E: sprintf_s(val_char_2, 40, "SBC  A,(HL)"); break;
				case 0x9F: sprintf_s(val_char_2, 40, "SBC  A,A"); break;
				case 0xA0: sprintf_s(val_char_2, 40, "AND  B"); break;
				case 0xA1: sprintf_s(val_char_2, 40, "AND  C"); break;
				case 0xA2: sprintf_s(val_char_2, 40, "AND  D"); break;
				case 0xA3: sprintf_s(val_char_2, 40, "AND  E"); break;
				case 0xA4: sprintf_s(val_char_2, 40, "AND  H"); break;
				case 0xA5: sprintf_s(val_char_2, 40, "AND  L"); break;
				case 0xA6: sprintf_s(val_char_2, 40, "AND  (HL)"); break;
				case 0xA7: sprintf_s(val_char_2, 40, "AND  A"); break;
				case 0xA8: sprintf_s(val_char_2, 40, "XOR  B"); break;
				case 0xA9: sprintf_s(val_char_2, 40, "XOR  C"); break;
				case 0xAA: sprintf_s(val_char_2, 40, "XOR  D"); break;
				case 0xAB: sprintf_s(val_char_2, 40, "XOR  E"); break;
				case 0xAC: sprintf_s(val_char_2, 40, "XOR  H"); break;
				case 0xAD: sprintf_s(val_char_2, 40, "XOR  L"); break;
				case 0xAE: sprintf_s(val_char_2, 40, "XOR  (HL)"); break;
				case 0xAF: sprintf_s(val_char_2, 40, "XOR  A"); break;
				case 0xB0: sprintf_s(val_char_2, 40, "OR   B"); break;
				case 0xB1: sprintf_s(val_char_2, 40, "OR   C"); break;
				case 0xB2: sprintf_s(val_char_2, 40, "OR   D"); break;
				case 0xB3: sprintf_s(val_char_2, 40, "OR   E"); break;
				case 0xB4: sprintf_s(val_char_2, 40, "OR   H"); break;
				case 0xB5: sprintf_s(val_char_2, 40, "OR   L"); break;
				case 0xB6: sprintf_s(val_char_2, 40, "OR   (HL)"); break;
				case 0xB7: sprintf_s(val_char_2, 40, "OR   A"); break;
				case 0xB8: sprintf_s(val_char_2, 40, "CP   B"); break;
				case 0xB9: sprintf_s(val_char_2, 40, "CP   C"); break;
				case 0xBA: sprintf_s(val_char_2, 40, "CP   D"); break;
				case 0xBB: sprintf_s(val_char_2, 40, "CP   E"); break;
				case 0xBC: sprintf_s(val_char_2, 40, "CP   H"); break;
				case 0xBD: sprintf_s(val_char_2, 40, "CP   L"); break;
				case 0xBE: sprintf_s(val_char_2, 40, "CP   (HL)"); break;
				case 0xBF: sprintf_s(val_char_2, 40, "CP   A"); break;
				case 0xC0: sprintf_s(val_char_2, 40, "RET  NZ"); break;
				case 0xC1: sprintf_s(val_char_2, 40, "POP  BC"); break;
				case 0xC2: sprintf_s(val_char_2, 40, "JP   NZ,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xC3: sprintf_s(val_char_2, 40, "JP   $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xC4: sprintf_s(val_char_2, 40, "CALL NZ,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xC5: sprintf_s(val_char_2, 40, "PUSH BC"); break;
				case 0xC6: sprintf_s(val_char_2, 40, "ADD  A,$%02X", Peek_Memory(pc++)); break;
				case 0xC7: sprintf_s(val_char_2, 40, "RST  00H"); break;
				case 0xC8: sprintf_s(val_char_2, 40, "RET  Z"); break;
				case 0xC9: sprintf_s(val_char_2, 40, "RET"); break;
				case 0xCA: sprintf_s(val_char_2, 40, "JP   Z,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xCB: sprintf_s(val_char_2, 40, "PREFIX CB"); break;
				case 0xCC: sprintf_s(val_char_2, 40, "CALL Z,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xCD: sprintf_s(val_char_2, 40, "CALL $%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xCE: sprintf_s(val_char_2, 40, "ADC  A,$%02X", Peek_Memory(pc++)); break;
				case 0xCF: sprintf_s(val_char_2, 40, "RST  08H"); break;
				case 0xD0: sprintf_s(val_char_2, 40, "RET  NC"); break;
				case 0xD1: sprintf_s(val_char_2, 40, "POP  DE"); break;
				case 0xD2: sprintf_s(val_char_2, 40, "JP   NC,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xD3: sprintf_s(val_char_2, 40, "???"); break;
				case 0xD4: sprintf_s(val_char_2, 40, "CALL NC,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xD5: sprintf_s(val_char_2, 40, "PUSH DE"); break;
				case 0xD6: sprintf_s(val_char_2, 40, "SUB  $%02X", Peek_Memory(pc++)); break;
				case 0xD7: sprintf_s(val_char_2, 40, "RST  10H"); break;
				case 0xD8: sprintf_s(val_char_2, 40, "RET  C"); break;
				case 0xD9: sprintf_s(val_char_2, 40, "RETI"); break;
				case 0xDA: sprintf_s(val_char_2, 40, "JP   C,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xDB: sprintf_s(val_char_2, 40, "???"); break;
				case 0xDC: sprintf_s(val_char_2, 40, "CALL C,$%04X", Peek_Memory_16(pc++)); pc++; break;
				case 0xDD: sprintf_s(val_char_2, 40, "???"); break;
				case 0xDE: sprintf_s(val_char_2, 40, "SBC  A,$%02X", Peek_Memory(pc++)); break;
				case 0xDF: sprintf_s(val_char_2, 40, "RST  18H"); break;
				case 0xE0: sprintf_s(val_char_2, 40, "LDH  (FF$%02X),A", Peek_Memory(pc++)); break;
				case 0xE1: sprintf_s(val_char_2, 40, "POP  HL"); break;
				case 0xE2: sprintf_s(val_char_2, 40, "LD   (C),A"); break;
				case 0xE3: sprintf_s(val_char_2, 40, "???"); break;
				case 0xE4: sprintf_s(val_char_2, 40, "???"); break;
				case 0xE5: sprintf_s(val_char_2, 40, "PUSH HL"); break;
				case 0xE6: sprintf_s(val_char_2, 40, "AND  $%02X", Peek_Memory(pc++)); break;
				case 0xE7: sprintf_s(val_char_2, 40, "RST  20H"); break;
				case 0xE8: sprintf_s(val_char_2, 40, "ADD  SP,$%04d", Peek_Memory_Signed(pc++));
				case 0xE9: sprintf_s(val_char_2, 40, "JP   HL"); break;
				case 0xEA: sprintf_s(val_char_2, 40, "LD   ($%04X),A", Peek_Memory_16(pc++)); pc++; break;
				case 0xEB: sprintf_s(val_char_2, 40, "???"); break;
				case 0xEC: sprintf_s(val_char_2, 40, "???"); break;
				case 0xED: sprintf_s(val_char_2, 40, "???"); break;
				case 0xEE: sprintf_s(val_char_2, 40, "XOR  $%02X", Peek_Memory(pc++)); break;
				case 0xEF: sprintf_s(val_char_2, 40, "RST  28H"); break;
				case 0xF0: sprintf_s(val_char_2, 40, "LDH  A,(FF$%02X)", Peek_Memory(pc++)); break;
				case 0xF1: sprintf_s(val_char_2, 40, "POP  AF"); break;
				case 0xF2: sprintf_s(val_char_2, 40, "LD   A,(C)"); break;
				case 0xF3: sprintf_s(val_char_2, 40, "DI"); break;
				case 0xF4: sprintf_s(val_char_2, 40, "???"); break;
				case 0xF5: sprintf_s(val_char_2, 40, "PUSH AF"); break;
				case 0xF6: sprintf_s(val_char_2, 40, "OR   $%02X", Peek_Memory(pc++)); break;
				case 0xF7: sprintf_s(val_char_2, 40, "RST  30H"); break;
				case 0xF8: sprintf_s(val_char_2, 40, "LD   HL,SP+$%04d", Peek_Memory_Signed(pc++)); break;
				case 0xF9: sprintf_s(val_char_2, 40, "LD   SP,HL"); break;
				case 0xFA: sprintf_s(val_char_2, 40, "LD   A,($%04X)", Peek_Memory_16(pc++)); pc++; break;
				case 0xFB: sprintf_s(val_char_2, 40, "EI   "); break;
				case 0xFC: sprintf_s(val_char_2, 40, "???"); break;
				case 0xFD: sprintf_s(val_char_2, 40, "???"); break;
				case 0xFE: sprintf_s(val_char_2, 40, "CP   $%02X", Peek_Memory(pc++)); break;
				case 0xFF: sprintf_s(val_char_2, 40, "RST  38H"); break;
				// CB prefix opcode
				case 0x100: sprintf_s(val_char_2, 40, "RLC  B"); break;
				case 0x101: sprintf_s(val_char_2, 40, "RLC  C"); break;
				case 0x102: sprintf_s(val_char_2, 40, "RLC  D"); break;
				case 0x103: sprintf_s(val_char_2, 40, "RLC  E"); break;
				case 0x104: sprintf_s(val_char_2, 40, "RLC  H"); break;
				case 0x105: sprintf_s(val_char_2, 40, "RLC  L"); break;
				case 0x106: sprintf_s(val_char_2, 40, "RLC  (HL)"); break;
				case 0x107: sprintf_s(val_char_2, 40, "RLC  A"); break;
				case 0x108: sprintf_s(val_char_2, 40, "RRC  B"); break;
				case 0x109: sprintf_s(val_char_2, 40, "RRC  C"); break;
				case 0x10A: sprintf_s(val_char_2, 40, "RRC  D"); break;
				case 0x10B: sprintf_s(val_char_2, 40, "RRC  E"); break;
				case 0x10C: sprintf_s(val_char_2, 40, "RRC  H"); break;
				case 0x10D: sprintf_s(val_char_2, 40, "RRC  L"); break;
				case 0x10E: sprintf_s(val_char_2, 40, "RRC  (HL)"); break;
				case 0x10F: sprintf_s(val_char_2, 40, "RRC  A"); break;
				case 0x110: sprintf_s(val_char_2, 40, "RL   B"); break;
				case 0x111: sprintf_s(val_char_2, 40, "RL   C"); break;
				case 0x112: sprintf_s(val_char_2, 40, "RL   D"); break;
				case 0x113: sprintf_s(val_char_2, 40, "RL   E"); break;
				case 0x114: sprintf_s(val_char_2, 40, "RL   H"); break;
				case 0x115: sprintf_s(val_char_2, 40, "RL   L"); break;
				case 0x116: sprintf_s(val_char_2, 40, "RL   (HL)"); break;
				case 0x117: sprintf_s(val_char_2, 40, "RL   A"); break;
				case 0x118: sprintf_s(val_char_2, 40, "RR   B"); break;
				case 0x119: sprintf_s(val_char_2, 40, "RR   C"); break;
				case 0x11A: sprintf_s(val_char_2, 40, "RR   D"); break;
				case 0x11B: sprintf_s(val_char_2, 40, "RR   E"); break;
				case 0x11C: sprintf_s(val_char_2, 40, "RR   H"); break;
				case 0x11D: sprintf_s(val_char_2, 40, "RR   L"); break;
				case 0x11E: sprintf_s(val_char_2, 40, "RR   (HL)"); break;
				case 0x11F: sprintf_s(val_char_2, 40, "RR   A"); break;
				case 0x120: sprintf_s(val_char_2, 40, "SLA  B"); break;
				case 0x121: sprintf_s(val_char_2, 40, "SLA  C"); break;
				case 0x122: sprintf_s(val_char_2, 40, "SLA  D"); break;
				case 0x123: sprintf_s(val_char_2, 40, "SLA  E"); break;
				case 0x124: sprintf_s(val_char_2, 40, "SLA  H"); break;
				case 0x125: sprintf_s(val_char_2, 40, "SLA  L"); break;
				case 0x126: sprintf_s(val_char_2, 40, "SLA  (HL)"); break;
				case 0x127: sprintf_s(val_char_2, 40, "SLA  A"); break;
				case 0x128: sprintf_s(val_char_2, 40, "SRA  B"); break;
				case 0x129: sprintf_s(val_char_2, 40, "SRA  C"); break;
				case 0x12A: sprintf_s(val_char_2, 40, "SRA  D"); break;
				case 0x12B: sprintf_s(val_char_2, 40, "SRA  E"); break;
				case 0x12C: sprintf_s(val_char_2, 40, "SRA  H"); break;
				case 0x12D: sprintf_s(val_char_2, 40, "SRA  L"); break;
				case 0x12E: sprintf_s(val_char_2, 40, "SRA  (HL)"); break;
				case 0x12F: sprintf_s(val_char_2, 40, "SRA  A"); break;
				case 0x130: sprintf_s(val_char_2, 40, "SWAP B"); break;
				case 0x131: sprintf_s(val_char_2, 40, "SWAP C"); break;
				case 0x132: sprintf_s(val_char_2, 40, "SWAP D"); break;
				case 0x133: sprintf_s(val_char_2, 40, "SWAP E"); break;
				case 0x134: sprintf_s(val_char_2, 40, "SWAP H"); break;
				case 0x135: sprintf_s(val_char_2, 40, "SWAP L"); break;
				case 0x136: sprintf_s(val_char_2, 40, "SWAP (HL)"); break;
				case 0x137: sprintf_s(val_char_2, 40, "SWAP A"); break;
				case 0x138: sprintf_s(val_char_2, 40, "SRL  B"); break;
				case 0x139: sprintf_s(val_char_2, 40, "SRL  C"); break;
				case 0x13A: sprintf_s(val_char_2, 40, "SRL  D"); break;
				case 0x13B: sprintf_s(val_char_2, 40, "SRL  E"); break;
				case 0x13C: sprintf_s(val_char_2, 40, "SRL  H"); break;
				case 0x13D: sprintf_s(val_char_2, 40, "SRL  L"); break;
				case 0x13E: sprintf_s(val_char_2, 40, "SRL  (HL)"); break;
				case 0x13F: sprintf_s(val_char_2, 40, "SRL  A"); break;
				case 0x140: sprintf_s(val_char_2, 40, "BIT  0,B"); break;
				case 0x141: sprintf_s(val_char_2, 40, "BIT  0,C"); break;
				case 0x142: sprintf_s(val_char_2, 40, "BIT  0,D"); break;
				case 0x143: sprintf_s(val_char_2, 40, "BIT  0,E"); break;
				case 0x144: sprintf_s(val_char_2, 40, "BIT  0,H"); break;
				case 0x145: sprintf_s(val_char_2, 40, "BIT  0,L"); break;
				case 0x146: sprintf_s(val_char_2, 40, "BIT  0,(HL)"); break;
				case 0x147: sprintf_s(val_char_2, 40, "BIT  0,A"); break;
				case 0x148: sprintf_s(val_char_2, 40, "BIT  1,B"); break;
				case 0x149: sprintf_s(val_char_2, 40, "BIT  1,C"); break;
				case 0x14A: sprintf_s(val_char_2, 40, "BIT  1,D"); break;
				case 0x14B: sprintf_s(val_char_2, 40, "BIT  1,E"); break;
				case 0x14C: sprintf_s(val_char_2, 40, "BIT  1,H"); break;
				case 0x14D: sprintf_s(val_char_2, 40, "BIT  1,L"); break;
				case 0x14E: sprintf_s(val_char_2, 40, "BIT  1,(HL)"); break;
				case 0x14F: sprintf_s(val_char_2, 40, "BIT  1,A"); break;
				case 0x150: sprintf_s(val_char_2, 40, "BIT  2,B"); break;
				case 0x151: sprintf_s(val_char_2, 40, "BIT  2,C"); break;
				case 0x152: sprintf_s(val_char_2, 40, "BIT  2,D"); break;
				case 0x153: sprintf_s(val_char_2, 40, "BIT  2,E"); break;
				case 0x154: sprintf_s(val_char_2, 40, "BIT  2,H"); break;
				case 0x155: sprintf_s(val_char_2, 40, "BIT  2,L"); break;
				case 0x156: sprintf_s(val_char_2, 40, "BIT  2,(HL)"); break;
				case 0x157: sprintf_s(val_char_2, 40, "BIT  2,A"); break;
				case 0x158: sprintf_s(val_char_2, 40, "BIT  3,B"); break;
				case 0x159: sprintf_s(val_char_2, 40, "BIT  3,C"); break;
				case 0x15A: sprintf_s(val_char_2, 40, "BIT  3,D"); break;
				case 0x15B: sprintf_s(val_char_2, 40, "BIT  3,E"); break;
				case 0x15C: sprintf_s(val_char_2, 40, "BIT  3,H"); break;
				case 0x15D: sprintf_s(val_char_2, 40, "BIT  3,L"); break;
				case 0x15E: sprintf_s(val_char_2, 40, "BIT  3,(HL)"); break;
				case 0x15F: sprintf_s(val_char_2, 40, "BIT  3,A"); break;
				case 0x160: sprintf_s(val_char_2, 40, "BIT  4,B"); break;
				case 0x161: sprintf_s(val_char_2, 40, "BIT  4,C"); break;
				case 0x162: sprintf_s(val_char_2, 40, "BIT  4,D"); break;
				case 0x163: sprintf_s(val_char_2, 40, "BIT  4,E"); break;
				case 0x164: sprintf_s(val_char_2, 40, "BIT  4,H"); break;
				case 0x165: sprintf_s(val_char_2, 40, "BIT  4,L"); break;
				case 0x166: sprintf_s(val_char_2, 40, "BIT  4,(HL)"); break;
				case 0x167: sprintf_s(val_char_2, 40, "BIT  4,A"); break;
				case 0x168: sprintf_s(val_char_2, 40, "BIT  5,B"); break;
				case 0x169: sprintf_s(val_char_2, 40, "BIT  5,C"); break;
				case 0x16A: sprintf_s(val_char_2, 40, "BIT  5,D"); break;
				case 0x16B: sprintf_s(val_char_2, 40, "BIT  5,E"); break;
				case 0x16C: sprintf_s(val_char_2, 40, "BIT  5,H"); break;
				case 0x16D: sprintf_s(val_char_2, 40, "BIT  5,L"); break;
				case 0x16E: sprintf_s(val_char_2, 40, "BIT  5,(HL)"); break;
				case 0x16F: sprintf_s(val_char_2, 40, "BIT  5,A"); break;
				case 0x170: sprintf_s(val_char_2, 40, "BIT  6,B"); break;
				case 0x171: sprintf_s(val_char_2, 40, "BIT  6,C"); break;
				case 0x172: sprintf_s(val_char_2, 40, "BIT  6,D"); break;
				case 0x173: sprintf_s(val_char_2, 40, "BIT  6,E"); break;
				case 0x174: sprintf_s(val_char_2, 40, "BIT  6,H"); break;
				case 0x175: sprintf_s(val_char_2, 40, "BIT  6,L"); break;
				case 0x176: sprintf_s(val_char_2, 40, "BIT  6,(HL)"); break;
				case 0x177: sprintf_s(val_char_2, 40, "BIT  6,A"); break;
				case 0x178: sprintf_s(val_char_2, 40, "BIT  7,B"); break;
				case 0x179: sprintf_s(val_char_2, 40, "BIT  7,C"); break;
				case 0x17A: sprintf_s(val_char_2, 40, "BIT  7,D"); break;
				case 0x17B: sprintf_s(val_char_2, 40, "BIT  7,E"); break;
				case 0x17C: sprintf_s(val_char_2, 40, "BIT  7,H"); break;
				case 0x17D: sprintf_s(val_char_2, 40, "BIT  7,L"); break;
				case 0x17E: sprintf_s(val_char_2, 40, "BIT  7,(HL)"); break;
				case 0x17F: sprintf_s(val_char_2, 40, "BIT  7,A"); break;
				case 0x180: sprintf_s(val_char_2, 40, "RES  0,B"); break;
				case 0x181: sprintf_s(val_char_2, 40, "RES  0,C"); break;
				case 0x182: sprintf_s(val_char_2, 40, "RES  0,D"); break;
				case 0x183: sprintf_s(val_char_2, 40, "RES  0,E"); break;
				case 0x184: sprintf_s(val_char_2, 40, "RES  0,H"); break;
				case 0x185: sprintf_s(val_char_2, 40, "RES  0,L"); break;
				case 0x186: sprintf_s(val_char_2, 40, "RES  0,(HL)"); break;
				case 0x187: sprintf_s(val_char_2, 40, "RES  0,A"); break;
				case 0x188: sprintf_s(val_char_2, 40, "RES  1,B"); break;
				case 0x189: sprintf_s(val_char_2, 40, "RES  1,C"); break;
				case 0x18A: sprintf_s(val_char_2, 40, "RES  1,D"); break;
				case 0x18B: sprintf_s(val_char_2, 40, "RES  1,E"); break;
				case 0x18C: sprintf_s(val_char_2, 40, "RES  1,H"); break;
				case 0x18D: sprintf_s(val_char_2, 40, "RES  1,L"); break;
				case 0x18E: sprintf_s(val_char_2, 40, "RES  1,(HL)"); break;
				case 0x18F: sprintf_s(val_char_2, 40, "RES  1,A"); break;
				case 0x190: sprintf_s(val_char_2, 40, "RES  2,B"); break;
				case 0x191: sprintf_s(val_char_2, 40, "RES  2,C"); break;
				case 0x192: sprintf_s(val_char_2, 40, "RES  2,D"); break;
				case 0x193: sprintf_s(val_char_2, 40, "RES  2,E"); break;
				case 0x194: sprintf_s(val_char_2, 40, "RES  2,H"); break;
				case 0x195: sprintf_s(val_char_2, 40, "RES  2,L"); break;
				case 0x196: sprintf_s(val_char_2, 40, "RES  2,(HL)"); break;
				case 0x197: sprintf_s(val_char_2, 40, "RES  2,A"); break;
				case 0x198: sprintf_s(val_char_2, 40, "RES  3,B"); break;
				case 0x199: sprintf_s(val_char_2, 40, "RES  3,C"); break;
				case 0x19A: sprintf_s(val_char_2, 40, "RES  3,D"); break;
				case 0x19B: sprintf_s(val_char_2, 40, "RES  3,E"); break;
				case 0x19C: sprintf_s(val_char_2, 40, "RES  3,H"); break;
				case 0x19D: sprintf_s(val_char_2, 40, "RES  3,L"); break;
				case 0x19E: sprintf_s(val_char_2, 40, "RES  3,(HL)"); break;
				case 0x19F: sprintf_s(val_char_2, 40, "RES  3,A"); break;
				case 0x1A0: sprintf_s(val_char_2, 40, "RES  4,B"); break;
				case 0x1A1: sprintf_s(val_char_2, 40, "RES  4,C"); break;
				case 0x1A2: sprintf_s(val_char_2, 40, "RES  4,D"); break;
				case 0x1A3: sprintf_s(val_char_2, 40, "RES  4,E"); break;
				case 0x1A4: sprintf_s(val_char_2, 40, "RES  4,H"); break;
				case 0x1A5: sprintf_s(val_char_2, 40, "RES  4,L"); break;
				case 0x1A6: sprintf_s(val_char_2, 40, "RES  4,(HL)"); break;
				case 0x1A7: sprintf_s(val_char_2, 40, "RES  4,A"); break;
				case 0x1A8: sprintf_s(val_char_2, 40, "RES  5,B"); break;
				case 0x1A9: sprintf_s(val_char_2, 40, "RES  5,C"); break;
				case 0x1AA: sprintf_s(val_char_2, 40, "RES  5,D"); break;
				case 0x1AB: sprintf_s(val_char_2, 40, "RES  5,E"); break;
				case 0x1AC: sprintf_s(val_char_2, 40, "RES  5,H"); break;
				case 0x1AD: sprintf_s(val_char_2, 40, "RES  5,L"); break;
				case 0x1AE: sprintf_s(val_char_2, 40, "RES  5,(HL)"); break;
				case 0x1AF: sprintf_s(val_char_2, 40, "RES  5,A"); break;
				case 0x1B0: sprintf_s(val_char_2, 40, "RES  6,B"); break;
				case 0x1B1: sprintf_s(val_char_2, 40, "RES  6,C"); break;
				case 0x1B2: sprintf_s(val_char_2, 40, "RES  6,D"); break;
				case 0x1B3: sprintf_s(val_char_2, 40, "RES  6,E"); break;
				case 0x1B4: sprintf_s(val_char_2, 40, "RES  6,H"); break;
				case 0x1B5: sprintf_s(val_char_2, 40, "RES  6,L"); break;
				case 0x1B6: sprintf_s(val_char_2, 40, "RES  6,(HL)"); break;
				case 0x1B7: sprintf_s(val_char_2, 40, "RES  6,A"); break;
				case 0x1B8: sprintf_s(val_char_2, 40, "RES  7,B"); break;
				case 0x1B9: sprintf_s(val_char_2, 40, "RES  7,C"); break;
				case 0x1BA: sprintf_s(val_char_2, 40, "RES  7,D"); break;
				case 0x1BB: sprintf_s(val_char_2, 40, "RES  7,E"); break;
				case 0x1BC: sprintf_s(val_char_2, 40, "RES  7,H"); break;
				case 0x1BD: sprintf_s(val_char_2, 40, "RES  7,L"); break;
				case 0x1BE: sprintf_s(val_char_2, 40, "RES  7,(HL)"); break;
				case 0x1BF: sprintf_s(val_char_2, 40, "RES  7,A"); break;
				case 0x1C0: sprintf_s(val_char_2, 40, "SET  0,B"); break;
				case 0x1C1: sprintf_s(val_char_2, 40, "SET  0,C"); break;
				case 0x1C2: sprintf_s(val_char_2, 40, "SET  0,D"); break;
				case 0x1C3: sprintf_s(val_char_2, 40, "SET  0,E"); break;
				case 0x1C4: sprintf_s(val_char_2, 40, "SET  0,H"); break;
				case 0x1C5: sprintf_s(val_char_2, 40, "SET  0,L"); break;
				case 0x1C6: sprintf_s(val_char_2, 40, "SET  0,(HL)"); break;
				case 0x1C7: sprintf_s(val_char_2, 40, "SET  0,A"); break;
				case 0x1C8: sprintf_s(val_char_2, 40, "SET  1,B"); break;
				case 0x1C9: sprintf_s(val_char_2, 40, "SET  1,C"); break;
				case 0x1CA: sprintf_s(val_char_2, 40, "SET  1,D"); break;
				case 0x1CB: sprintf_s(val_char_2, 40, "SET  1,E"); break;
				case 0x1CC: sprintf_s(val_char_2, 40, "SET  1,H"); break;
				case 0x1CD: sprintf_s(val_char_2, 40, "SET  1,L"); break;
				case 0x1CE: sprintf_s(val_char_2, 40, "SET  1,(HL)"); break;
				case 0x1CF: sprintf_s(val_char_2, 40, "SET  1,A"); break;
				case 0x1D0: sprintf_s(val_char_2, 40, "SET  2,B"); break;
				case 0x1D1: sprintf_s(val_char_2, 40, "SET  2,C"); break;
				case 0x1D2: sprintf_s(val_char_2, 40, "SET  2,D"); break;
				case 0x1D3: sprintf_s(val_char_2, 40, "SET  2,E"); break;
				case 0x1D4: sprintf_s(val_char_2, 40, "SET  2,H"); break;
				case 0x1D5: sprintf_s(val_char_2, 40, "SET  2,L"); break;
				case 0x1D6: sprintf_s(val_char_2, 40, "SET  2,(HL)"); break;
				case 0x1D7: sprintf_s(val_char_2, 40, "SET  2,A"); break;
				case 0x1D8: sprintf_s(val_char_2, 40, "SET  3,B"); break;
				case 0x1D9: sprintf_s(val_char_2, 40, "SET  3,C"); break;
				case 0x1DA: sprintf_s(val_char_2, 40, "SET  3,D"); break;
				case 0x1DB: sprintf_s(val_char_2, 40, "SET  3,E"); break;
				case 0x1DC: sprintf_s(val_char_2, 40, "SET  3,H"); break;
				case 0x1DD: sprintf_s(val_char_2, 40, "SET  3,L"); break;
				case 0x1DE: sprintf_s(val_char_2, 40, "SET  3,(HL)"); break;
				case 0x1DF: sprintf_s(val_char_2, 40, "SET  3,A"); break;
				case 0x1E0: sprintf_s(val_char_2, 40, "SET  4,B"); break;
				case 0x1E1: sprintf_s(val_char_2, 40, "SET  4,C"); break;
				case 0x1E2: sprintf_s(val_char_2, 40, "SET  4,D"); break;
				case 0x1E3: sprintf_s(val_char_2, 40, "SET  4,E"); break;
				case 0x1E4: sprintf_s(val_char_2, 40, "SET  4,H"); break;
				case 0x1E5: sprintf_s(val_char_2, 40, "SET  4,L"); break;
				case 0x1E6: sprintf_s(val_char_2, 40, "SET  4,(HL)"); break;
				case 0x1E7: sprintf_s(val_char_2, 40, "SET  4,A"); break;
				case 0x1E8: sprintf_s(val_char_2, 40, "SET  5,B"); break;
				case 0x1E9: sprintf_s(val_char_2, 40, "SET  5,C"); break;
				case 0x1EA: sprintf_s(val_char_2, 40, "SET  5,D"); break;
				case 0x1EB: sprintf_s(val_char_2, 40, "SET  5,E"); break;
				case 0x1EC: sprintf_s(val_char_2, 40, "SET  5,H"); break;
				case 0x1ED: sprintf_s(val_char_2, 40, "SET  5,L"); break;
				case 0x1EE: sprintf_s(val_char_2, 40, "SET  5,(HL)"); break;
				case 0x1EF: sprintf_s(val_char_2, 40, "SET  5,A"); break;
				case 0x1F0: sprintf_s(val_char_2, 40, "SET  6,B"); break;
				case 0x1F1: sprintf_s(val_char_2, 40, "SET  6,C"); break;
				case 0x1F2: sprintf_s(val_char_2, 40, "SET  6,D"); break;
				case 0x1F3: sprintf_s(val_char_2, 40, "SET  6,E"); break;
				case 0x1F4: sprintf_s(val_char_2, 40, "SET  6,H"); break;
				case 0x1F5: sprintf_s(val_char_2, 40, "SET  6,L"); break;
				case 0x1F6: sprintf_s(val_char_2, 40, "SET  6,(HL)"); break;
				case 0x1F7: sprintf_s(val_char_2, 40, "SET  6,A"); break;
				case 0x1F8: sprintf_s(val_char_2, 40, "SET  7,B"); break;
				case 0x1F9: sprintf_s(val_char_2, 40, "SET  7,C"); break;
				case 0x1FA: sprintf_s(val_char_2, 40, "SET  7,D"); break;
				case 0x1FB: sprintf_s(val_char_2, 40, "SET  7,E"); break;
				case 0x1FC: sprintf_s(val_char_2, 40, "SET  7,H"); break;
				case 0x1FD: sprintf_s(val_char_2, 40, "SET  7,L"); break;
				case 0x1FE: sprintf_s(val_char_2, 40, "SET  7,(HL)"); break;
				case 0x1FF: sprintf_s(val_char_2, 40, "SET  7,A"); break;
			}

			if (pc > diff)
			{
				op_size = pc - diff;
			}
			else
			{
				uint32_t h_diff = (uint32_t)pc + 0x10000;

				op_size = h_diff - diff;
			}

			return std::string(val_char_2, 20);
		}

		#pragma endregion

		#pragma region CPU State Save / Load

		uint8_t* cpu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(cpu_was_FlagI, saver);
			saver = bool_saver(cpu_FlagI, saver);
			saver = bool_saver(cpu_Jammed, saver);
			saver = bool_saver(cpu_Stopped, saver);
			saver = bool_saver(cpu_Halted, saver);
			saver = bool_saver(cpu_CB_Prefix, saver);

			saver = bool_saver(cpu_Interrupts_Enabled, saver);

			saver = bool_saver(cpu_I_Use, saver);
			saver = bool_saver(cpu_Skip_Once, saver);
			saver = bool_saver(cpu_Halt_bug_2, saver);
			saver = bool_saver(cpu_Halt_bug_3, saver);
			saver = bool_saver(cpu_Halt_bug_4, saver);
			saver = bool_saver(cpu_Halt_bug_5, saver);
			saver = bool_saver(cpu_Stop_Check, saver);

			saver = byte_saver(cpu_Int_Clear, saver);

			saver = short_saver(cpu_Opcode, saver);
			saver = short_saver(cpu_Instr_Cycle, saver);
			saver = short_saver(cpu_Instr_Cycle_Hold, saver);

			saver = short_saver(cpu_Int_Src, saver);

			saver = int_saver(cpu_EI_Pending, saver);

			saver = int_saver((uint32_t)cpu_Instr_Type, saver);
			saver = int_saver((uint32_t)cpu_State_Hold, saver);

			saver = long_saver(cpu_Instruction_Start, saver);
			saver = long_saver(cpu_Stop_Time, saver);

			saver = byte_array_saver(cpu_Regs, saver, 14);

			return saver;
		}

		uint8_t* cpu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&cpu_was_FlagI, loader);
			loader = bool_loader(&cpu_FlagI, loader);
			loader = bool_loader(&cpu_Jammed, loader);
			loader = bool_loader(&cpu_Stopped, loader);
			loader = bool_loader(&cpu_Halted, loader);
			loader = bool_loader(&cpu_CB_Prefix, loader);

			loader = bool_loader(&cpu_Interrupts_Enabled, loader);

			loader = bool_loader(&cpu_I_Use, loader);
			loader = bool_loader(&cpu_Skip_Once, loader);
			loader = bool_loader(&cpu_Halt_bug_2, loader);
			loader = bool_loader(&cpu_Halt_bug_3, loader);
			loader = bool_loader(&cpu_Halt_bug_4, loader);
			loader = bool_loader(&cpu_Halt_bug_5, loader);
			loader = bool_loader(&cpu_Stop_Check, loader);

			loader = byte_loader(&cpu_Int_Clear, loader);

			loader = short_loader(&cpu_Opcode, loader);
			loader = short_loader(&cpu_Instr_Cycle, loader);
			loader = short_loader(&cpu_Instr_Cycle_Hold, loader);

			loader = short_loader(&cpu_Int_Src, loader);

			loader = int_loader(&cpu_EI_Pending, loader);

			loader = int_loader(&cpu_Instr_Type_Save, loader);
			loader = int_loader(&cpu_State_Hold_Save, loader);

			loader = long_loader(&cpu_Instruction_Start, loader);
			loader = long_loader(&cpu_Stop_Time, loader);

			loader = byte_array_loader(cpu_Regs, loader, 14);

			cpu_Instr_Type = static_cast<OpT>(cpu_Instr_Type_Save);
			cpu_State_Hold = static_cast<OpT>(cpu_State_Hold_Save);

			return loader;
		}

		#pragma endregion

	#pragma endregion

	#pragma region Serial port

		bool ser_Can_Pulse;
		bool ser_IRQ_Block;

		uint8_t ser_Control;
		uint8_t ser_Data;
		uint8_t ser_Going_Out;
		uint8_t ser_Coming_In;

		uint32_t ser_Clock;
		uint32_t ser_Bits;
		uint32_t ser_Clk_Rate;

		uint8_t ser_Read_Reg(int addr)
		{
			switch (addr)
			{
				case 0xFF01:
					return ser_Data;
				case 0xFF02:
					return ser_Control;
			}

			return 0xFF;
		}

		void ser_Write_Reg(int addr, uint8_t value)
		{
			switch (addr)
			{
			case 0xFF01:
				ser_Data = value;
				break;

			case 0xFF02:
				if ((value & 0x80) == 0x80)
				{
					if ((value & 0x01) == 0x01)
					{
						if (((value & 2) > 0) && Is_GBC)
						{
							ser_Clk_Rate = 16;
							ser_Clock = 16 - (uint32_t)(tim_Divider_Reg % 8) - 1;

							// if the clock rate is changing and it's on a GBA/C, the parity of (cpu.totalexecutedcycles & 512) effects the first bit
							// Not sure exactly how yet
						}
						else
						{
							ser_Clk_Rate = 512;
							ser_Clock = 512 - (uint32_t)(tim_Divider_Reg % 256) - 1;

							// there seems to be some clock inverting happening on some transfers
							// not sure of the exact nature of it, here is one method that gives correct result on one test rom but not others
							/*
							if (Core._syncSettings.GBACGB && Core.is_GBC)
							{
								if ((Core.TotalExecutedCycles % 256) > 127)
								{
									serial_clock = (8 - (int)(Core.cpu.TotalExecutedCycles % 8)) + 1;
								}
							}
							*/
						}

						ser_Can_Pulse = true;
						ser_Bits = 8;
					}
					else
					{
						ser_Clk_Rate = -1;
						ser_Can_Pulse = false;
						ser_Bits = 8;
					}
				}
				else
				{
					ser_Bits = 8;
					ser_Clk_Rate = -1;
					ser_Clock = ser_Clk_Rate;
					ser_Can_Pulse = false;
				}

				if (Is_GBC)
				{
					ser_Control = (uint8_t)(0x7C | (value & 0x83)); // extra CGB bit
				}
				else
				{
					ser_Control = (uint8_t)(0x7E | (value & 0x81)); // middle six bits always 1
				}

				break;
			}
		}

		void ser_Tick()
		{
			ser_IRQ_Block = false;

			if (ser_Clock > 0)
			{
				ser_Clock--;

				if (ser_Clock == 0)
				{
					if (ser_Bits > 0)
					{
						ser_Data = (uint8_t)((ser_Data << 1) | ser_Coming_In);

						ser_Bits--;

						if (ser_Bits == 0)
						{
							ser_Control &= 0x7F;

							if ((REG_FFFF & 0x8) == 0x8) { cpu_FlagI = true; }
							REG_FF0F |= 0x08;
							//Console.WriteLine("SIRQ " + Core.cpu.TotalExecutedCycles);
							ser_IRQ_Block = true;
						}
						else
						{
							ser_Clock = ser_Clk_Rate;
							if (ser_Clk_Rate > 0) { ser_Can_Pulse = true; }
						}
					}
				}
			}
		}

		void ser_Reset()
		{
			ser_Control = 0x7E;
			ser_Data = 0x00;
			ser_Clock = -1;
			ser_Bits = 8;
			ser_Clk_Rate = -1;
			ser_Going_Out = 0;
			ser_Coming_In = 1;
			ser_Can_Pulse = false;
			ser_IRQ_Block = false;
		}

		uint8_t* ser_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ser_Can_Pulse, saver);
			saver = bool_saver(ser_IRQ_Block, saver);

			saver = byte_saver(ser_Control, saver);
			saver = byte_saver(ser_Data, saver);
			saver = byte_saver(ser_Going_Out, saver);
			saver = byte_saver(ser_Coming_In, saver);

			saver = int_saver(ser_Clock, saver);
			saver = int_saver(ser_Bits, saver);
			saver = int_saver(ser_Clk_Rate, saver);

			return saver;
		}

		uint8_t* ser_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ser_Can_Pulse, loader);
			loader = bool_loader(&ser_IRQ_Block, loader);

			loader = byte_loader(&ser_Control, loader);
			loader = byte_loader(&ser_Data, loader);
			loader = byte_loader(&ser_Going_Out, loader);
			loader = byte_loader(&ser_Coming_In, loader);

			loader = int_loader(&ser_Clock, loader);
			loader = int_loader(&ser_Bits, loader);
			loader = int_loader(&ser_Clk_Rate, loader);

			return loader;
		}

	#pragma endregion

	#pragma region Timer
		
		// Timer Emulation
		// NOTES: 
		//
		// Currently, a starting value of 0xFFFE passes all tests. GBA is not explicitly tested but for now is set to 0xFFFE as well.
		//
		// Some additional glitches happen on GBC, but they are non-deterministic and not emulated here
		//
		// TODO: On GBA models, there is a race condition when enabling with a change in bit check
		// that would result in a state change that is not consistent in all models, see tac_set_disabled.gbc
		//
		// TODO: On GBA only, there is a glitch where if the current timer control is 7 and the written value is 7 and
		// there is a coincident timer increment, there will be an additional increment along with this write.
		// not sure it effects all models or of exact details, see test tac_set_timer_disabled.gbc

		bool tim_IRQ_Block; // if the timer IRQ happens on the same cycle as a previous one was cleared, the IRQ is set
		bool tim_Old_State;
		bool tim_State;
		bool tim_Reload_Block;
		uint8_t tim_Reload;
		uint8_t tim_Timer;
		uint8_t tim_Timer_Old;
		uint8_t tim_Control;
		uint8_t tim_Pending_Reload;
		uint16_t tim_Divider_Reg;
		uint64_t tim_Next_Free_Cycle;

		uint8_t tim_Read_Reg(uint16_t addr)
		{
			uint8_t ret = 0;

			switch (addr)
			{
				case 0xFF04: ret = (uint8_t)(tim_Divider_Reg >> 8); 	break; // DIV register
				case 0xFF05: ret = tim_Timer;						break; // TIMA (Timer Counter)
				case 0xFF06: ret = tim_Reload;						break; // TMA (Timer Modulo)
				case 0xFF07: ret = tim_Control;					break; // TAC (Timer Control)
			}

			return ret;
		}

		void tim_Write_Reg(uint16_t addr, uint8_t value)
		{
			switch (addr)
			{
				// DIV register
				case 0xFF04:
					// NOTE: even though there is an automatic increment directly after the CPU loop, 
					// it is still expected that 0 is written here
					tim_Divider_Reg = 0;
					break;

					// TIMA (Timer Counter)
				case 0xFF05:
					if (Cycle_Count >= tim_Next_Free_Cycle)
					{
						tim_Timer_Old = tim_Timer;
						tim_Timer = value;
						tim_Reload_Block = true;
					}
					break;

					// TMA (Timer Modulo)
				case 0xFF06:
					tim_Reload = value;
					if (Cycle_Count < tim_Next_Free_Cycle)
					{
						tim_Timer = tim_Reload;
						tim_Timer_Old = tim_Timer;
					}
					break;

					// TAC (Timer Control)
				case 0xFF07:
					uint8_t timer_control_old = tim_Control;

					// Console.WriteLine("tac: " + timer_control + " " + value + " " + timer + " " + divider_reg);
					tim_Control = (uint8_t)((tim_Control & 0xf8) | (value & 0x7)); // only bottom 3 bits function

					if (!((timer_control_old & 4) == 4) && ((tim_Control & 4) == 4) && Is_GBC_GBA)
					{
						bool temp_check_old = false;
						bool temp_check = false;

						switch (timer_control_old & 3)
						{
							case 0:
								temp_check_old = ((tim_Divider_Reg & 0x100) == 0x100);
								break;
							case 1:
								temp_check_old = ((tim_Divider_Reg & 8) == 8);
								break;
							case 2:
								temp_check_old = ((tim_Divider_Reg & 0x20) == 0x20);
								break;
							case 3:
								temp_check_old = ((tim_Divider_Reg & 0x80) == 0x80);
								break;
						}

						switch (tim_Control & 3)
						{
							case 0:
								temp_check = ((tim_Divider_Reg & 0x100) == 0x100);
								break;
							case 1:
								temp_check = ((tim_Divider_Reg & 8) == 8);
								break;
							case 2:
								temp_check = ((tim_Divider_Reg & 0x20) == 0x20);
								break;
							case 3:
								temp_check = ((tim_Divider_Reg & 0x80) == 0x80);
								break;
						}

						if (temp_check_old && !temp_check)
						{
							tim_Timer_Old = tim_Timer;
							tim_Timer++;


							Message_String = "Timer glitch.";

							MessageCallback(Message_String.length());


							// if overflow happens, set the interrupt flag and reload the timer (if applicable)
							if (tim_Timer < tim_Timer_Old)
							{
								tim_Pending_Reload = 4;
								tim_Reload_Block = false;
							}
						}
					}

					break;
			}
		}

		void tim_Tick()
		{
			tim_IRQ_Block = false;

			// pick a bit to test based on the current value of timer control
			switch (tim_Control & 3)
			{
				case 0:
					tim_State = ((tim_Divider_Reg & 0x100) == 0x100);
					break;
				case 1:
					tim_State = ((tim_Divider_Reg & 8) == 8);
					break;
				case 2:
					tim_State = ((tim_Divider_Reg & 0x20) == 0x20);
					break;
				case 3:
					tim_State = ((tim_Divider_Reg & 0x80) == 0x80);
					break;
			}

			// And it with the state of the timer on/off bit
			tim_State &= ((tim_Control & 4) == 4);

			// this procedure allows several glitchy timer ticks, since it only measures falling edge of the state
			// so things like turning the timer off and resetting the divider will tick the timer
			if (tim_Old_State && !tim_State)
			{
				tim_Timer_Old = tim_Timer;
				tim_Timer++;

				// if overflow happens, set the interrupt flag and reload the timer (if applicable)
				if (tim_Timer < tim_Timer_Old)
				{
					if (((tim_Control & 4) == 4))
					{
						tim_Pending_Reload = 4;
						tim_Reload_Block = false;
					}
					else
					{
						tim_Pending_Reload = 3;
						tim_Reload_Block = false;
					}
				}
			}

			tim_Old_State = tim_State;

			if (tim_Pending_Reload > 0)
			{
				tim_Pending_Reload--;
				if (tim_Pending_Reload == 0 && !tim_Reload_Block)
				{
					tim_Timer = tim_Reload;
					tim_Timer_Old = tim_Timer;

					tim_Next_Free_Cycle = 4 + Cycle_Count;

					// set interrupts
					if ((REG_FFFF & 4) == 4) { cpu_FlagI = true; }
					//Console.WriteLine("timer " + Core.cpu.TotalExecutedCycles);
					REG_FF0F |= 0x04;
					tim_IRQ_Block = true;
				}
			}
		}

		void tim_Reset()
		{
			tim_Divider_Reg = 0xFFFE;
			tim_Reload = 0;
			tim_Timer = 0;
			tim_Timer_Old = 0;
			tim_Control = 0xF8;
			tim_Pending_Reload = 0;
			tim_IRQ_Block = false;
			tim_Old_State = false;
			tim_State = false;
			tim_Reload_Block = false;
			tim_Next_Free_Cycle = 0;
		}

		uint8_t* tim_SaveState(uint8_t* saver)
		{
			saver = bool_saver(tim_IRQ_Block, saver);
			saver = bool_saver(tim_Old_State, saver);
			saver = bool_saver(tim_State, saver);
			saver = bool_saver(tim_Reload_Block, saver);

			saver = byte_saver(tim_Reload, saver);
			saver = byte_saver(tim_Timer, saver);
			saver = byte_saver(tim_Timer_Old, saver);
			saver = byte_saver(tim_Control, saver);
			saver = byte_saver(tim_Pending_Reload, saver);

			saver = short_saver(tim_Divider_Reg, saver);

			saver = long_saver(tim_Next_Free_Cycle, saver);

			return saver;
		}

		uint8_t* tim_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&tim_IRQ_Block, loader);
			loader = bool_loader(&tim_Old_State, loader);
			loader = bool_loader(&tim_State, loader);
			loader = bool_loader(&tim_Reload_Block, loader);

			loader = byte_loader(&tim_Reload, loader);
			loader = byte_loader(&tim_Timer, loader);
			loader = byte_loader(&tim_Timer_Old, loader);
			loader = byte_loader(&tim_Control, loader);
			loader = byte_loader(&tim_Pending_Reload, loader);

			loader = short_loader(&tim_Divider_Reg, loader);

			loader = long_loader(&tim_Next_Free_Cycle, loader);

			return loader;
		}

	#pragma endregion

	#pragma region Audio

		bool snd_Duty_Cycles[32] = {false, false, false, false, false, false, false, true,
									true, false, false, false, false, false, false, true,
									true, false, false, false, false, true, true, true,
									false, true, true, true, true, true, true, false };

		const static uint16_t snd_NR10 = 0;
		const static uint16_t snd_NR11 = 1;
		const static uint16_t snd_NR12 = 2;
		const static uint16_t snd_NR13 = 3;
		const static uint16_t snd_NR14 = 4;
		const static uint16_t snd_NR21 = 5;
		const static uint16_t snd_NR22 = 6;
		const static uint16_t snd_NR23 = 7;
		const static uint16_t snd_NR24 = 8;
		const static uint16_t snd_NR30 = 9;
		const static uint16_t snd_NR31 = 10;
		const static uint16_t snd_NR32 = 11;
		const static uint16_t snd_NR33 = 12;
		const static uint16_t snd_NR34 = 13;
		const static uint16_t snd_NR41 = 14;
		const static uint16_t snd_NR42 = 15;
		const static uint16_t snd_NR43 = 16;
		const static uint16_t snd_NR44 = 17;
		const static uint16_t snd_NR50 = 18;
		const static uint16_t snd_NR51 = 19;
		const static uint16_t snd_NR52 = 20;

		uint32_t snd_Divisor[8] = {8, 16, 32, 48, 64, 80, 96, 112};

		uint32_t snd_Chan_Mult_Table[4] = {1, 2, 4, 4};

		uint8_t snd_Audio_Regs[21] = { };

		uint8_t snd_Wave_RAM[16] = { };

		uint32_t snd_DAC_Offset = 8;
		uint32_t snd_Wave_Decay_cnt;
		uint32_t snd_Internal_cnt;
		uint32_t snd_Divider;

		bool snd_Wave_Decay_Done;

		// Audio Variables
		// derived
		bool														snd_WAVE_DAC_pow;
		bool																				snd_NOISE_wdth_md;
		bool		snd_SQ1_negate;
		bool		snd_SQ1_trigger,		snd_SQ2_trigger,		snd_WAVE_trigger,		snd_NOISE_trigger;
		bool		snd_SQ1_len_en,			snd_SQ2_len_en,			snd_WAVE_len_en,		snd_NOISE_len_en;
		bool		snd_SQ1_env_add,		snd_SQ2_env_add,								snd_NOISE_env_add;
		uint8_t														snd_WAVE_vol_code;
		uint8_t																				snd_NOISE_clk_shft;
		uint8_t																				snd_NOISE_div_code;
		uint8_t		snd_SQ1_shift;
		uint8_t		snd_SQ1_duty,			snd_SQ2_duty;
		uint8_t		snd_SQ1_st_vol,			snd_SQ2_st_vol,									snd_NOISE_st_vol;
		uint8_t		snd_SQ1_per,			snd_SQ2_per,									snd_NOISE_per;
		uint8_t		snd_SQ1_swp_prd;
		uint32_t	snd_SQ1_frq,			snd_SQ2_frq,			snd_WAVE_frq;
		uint16_t	snd_SQ1_length,			snd_SQ2_length,			snd_WAVE_length,		snd_NOISE_length;
		// state
		bool														snd_WAVE_can_get;
		bool		snd_SQ1_calc_done;
		bool		snd_SQ1_swp_enable;
		bool		snd_SQ1_vol_done,		snd_SQ2_vol_done,								snd_NOISE_vol_done;
		bool		snd_SQ1_enable,			snd_SQ2_enable,			snd_WAVE_enable,		snd_NOISE_enable;
		uint8_t		snd_SQ1_vol_state,		snd_SQ2_vol_state,								snd_NOISE_vol_state;
		uint8_t		snd_SQ1_duty_cntr,		snd_SQ2_duty_cntr;
		uint8_t														snd_WAVE_wave_cntr;
		uint32_t	snd_SQ1_frq_shadow;
		uint32_t	snd_SQ1_intl_cntr,		snd_SQ2_intl_cntr,		snd_WAVE_intl_cntr,		snd_NOISE_intl_cntr;
		uint32_t	snd_SQ1_vol_per,		snd_SQ2_vol_per,								snd_NOISE_vol_per;
		uint32_t	snd_SQ1_intl_swp_cnt;
		uint32_t																			snd_NOISE_LFSR;
		uint16_t	snd_SQ1_len_cntr,		snd_SQ2_len_cntr,		snd_WAVE_len_cntr,		snd_NOISE_len_cntr;

		// GB specific registers
		uint32_t snd_Wave_Bank, snd_Wave_Bank_Playing;
		uint32_t snd_Chan_Mult;
		uint8_t snd_CTRL_GB_Low, snd_CTRL_GB_High;
		uint8_t snd_Bias_Low, snd_Bias_High;
		bool snd_Wave_Size;
		bool snd_Wave_Vol_Force;

		// computed
		int snd_SQ1_output, snd_SQ2_output, snd_WAVE_output, snd_NOISE_output;

		// Contol Variables
		bool snd_CTRL_sq1_L_en;
		bool snd_CTRL_sq2_L_en;
		bool snd_CTRL_wave_L_en;
		bool snd_CTRL_noise_L_en;
		bool snd_CTRL_sq1_R_en;
		bool snd_CTRL_sq2_R_en;
		bool snd_CTRL_wave_R_en;
		bool snd_CTRL_noise_R_en;
		bool snd_CTRL_power;
		bool snd_CTRL_vin_L_en;
		bool snd_CTRL_vin_R_en;

		uint8_t snd_CTRL_vol_L;
		uint8_t snd_CTRL_vol_R;

		uint32_t snd_Sequencer_len, snd_Sequencer_vol, snd_Sequencer_swp;
		uint32_t snd_Sequencer_reset_cd;

		uint8_t snd_Sample;

		uint32_t snd_Master_Clock;

		int32_t snd_Latched_Sample_L, snd_Latched_Sample_R;
		int32_t snd_BIAS_Offset, snd_Sample_Rate;

		uint32_t num_samples_L, num_samples_R;
		int32_t samples_L[25000] = {};
		int32_t samples_R[25000] = {};

		uint8_t snd_Unused_Bits[21] = { 0x80, 0x3F, 0x00, 0xFF, 0xBF,
											  0x3F, 0x00, 0xFF, 0xBF,
										0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
											  0xFF, 0x00, 0x00, 0xBF,
										0x00, 0x00, 0x70 };

		uint32_t snd_DAC_OFST = 8;
		uint32_t snd_WAVE_decay_counter;
		bool snd_WAVE_decay_done;

		bool snd_Update_Needed;

		uint8_t snd_WAVE_RAM_INI[16] = { 0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C,
									0x60, 0x59, 0x59, 0xB0, 0x34, 0xB8, 0x2E, 0xDA };

		uint8_t snd_Read_Reg(uint32_t addr)
		{
			uint8_t ret = 0;

			switch (addr)
			{
			case 0xFF10: ret = (uint8_t)(snd_Audio_Regs[snd_NR10] | snd_Unused_Bits[snd_NR10]); break; // NR10 (sweep)
			case 0xFF11: ret = (uint8_t)(snd_Audio_Regs[snd_NR11] | snd_Unused_Bits[snd_NR11]); break; // NR11 (sound length / wave pattern duty %)
			case 0xFF12: ret = (uint8_t)(snd_Audio_Regs[snd_NR12] | snd_Unused_Bits[snd_NR12]); break; // NR12 (envelope)
			case 0xFF13: ret = (uint8_t)(snd_Audio_Regs[snd_NR13] | snd_Unused_Bits[snd_NR13]); break; // NR13 (freq low)
			case 0xFF14: ret = (uint8_t)(snd_Audio_Regs[snd_NR14] | snd_Unused_Bits[snd_NR14]); break; // NR14 (freq hi)
			case 0xFF16: ret = (uint8_t)(snd_Audio_Regs[snd_NR21] | snd_Unused_Bits[snd_NR21]); break; // NR21 (sound length / wave pattern duty %)
			case 0xFF17: ret = (uint8_t)(snd_Audio_Regs[snd_NR22] | snd_Unused_Bits[snd_NR22]); break; // NR22 (envelope)
			case 0xFF18: ret = (uint8_t)(snd_Audio_Regs[snd_NR23] | snd_Unused_Bits[snd_NR23]); break; // NR23 (freq low)
			case 0xFF19: ret = (uint8_t)(snd_Audio_Regs[snd_NR24] | snd_Unused_Bits[snd_NR24]); break; // NR24 (freq hi)
			case 0xFF1A: ret = (uint8_t)(snd_Audio_Regs[snd_NR30] | snd_Unused_Bits[snd_NR30]); break; // NR30 (on/off)
			case 0xFF1B: ret = (uint8_t)(snd_Audio_Regs[snd_NR31] | snd_Unused_Bits[snd_NR31]); break; // NR31 (length)
			case 0xFF1C: ret = (uint8_t)(snd_Audio_Regs[snd_NR32] | snd_Unused_Bits[snd_NR32]); break; // NR32 (level output)
			case 0xFF1D: ret = (uint8_t)(snd_Audio_Regs[snd_NR33] | snd_Unused_Bits[snd_NR33]); break; // NR33 (freq low)
			case 0xFF1E: ret = (uint8_t)(snd_Audio_Regs[snd_NR34] | snd_Unused_Bits[snd_NR34]); break; // NR34 (freq hi)
			case 0xFF20: ret = (uint8_t)(snd_Audio_Regs[snd_NR41] | snd_Unused_Bits[snd_NR41]); break; // NR41 (length)
			case 0xFF21: ret = (uint8_t)(snd_Audio_Regs[snd_NR42] | snd_Unused_Bits[snd_NR42]); break; // NR42 (envelope)
			case 0xFF22: ret = (uint8_t)(snd_Audio_Regs[snd_NR43] | snd_Unused_Bits[snd_NR43]); break; // NR43 (shift)
			case 0xFF23: ret = (uint8_t)(snd_Audio_Regs[snd_NR44] | snd_Unused_Bits[snd_NR44]); break; // NR44 (trigger)
			case 0xFF24: ret = (uint8_t)(snd_Audio_Regs[snd_NR50] | snd_Unused_Bits[snd_NR50]); break; // NR50 (ctrl)
			case 0xFF25: ret = (uint8_t)(snd_Audio_Regs[snd_NR51] | snd_Unused_Bits[snd_NR51]); break; // NR51 (ctrl)
			case 0xFF26: ret = (uint8_t)(snd_Read_NR52() | snd_Unused_Bits[snd_NR52]); break; // NR52 (ctrl)

				// wave ram table
			case 0xFF30:
			case 0xFF31:
			case 0xFF32:
			case 0xFF33:
			case 0xFF34:
			case 0xFF35:
			case 0xFF36:
			case 0xFF37:
			case 0xFF38:
			case 0xFF39:
			case 0xFF3A:
			case 0xFF3B:
			case 0xFF3C:
			case 0xFF3D:
			case 0xFF3E:
			case 0xFF3F:
				if (snd_WAVE_enable)
				{
					if (snd_WAVE_can_get || Is_GBC) { ret = snd_Wave_RAM[snd_WAVE_wave_cntr >> 1]; }
					else { ret = 0xFF; }
				}
				else { ret = snd_Wave_RAM[addr & 0x0F]; }

				break;
			}

			return ret;
		}

		void snd_WriteReg(uint16_t addr, uint8_t value)
		{
			// while power is on, everything is writable
			//Console.WriteLine((addr & 0xFF) + " " + value);
			if (snd_CTRL_power)
			{
				switch (addr)
				{
					case 0xFF10:                                        // NR10 (sweep)
						snd_Audio_Regs[snd_NR10] = value;
						snd_SQ1_swp_prd = (uint8_t)((value & 0x70) >> 4);
						snd_SQ1_negate = (value & 8) > 0;
						snd_SQ1_shift = (uint8_t)(value & 7);

						if (!snd_SQ1_negate && snd_SQ1_calc_done) { snd_SQ1_enable = false; snd_calculate_bias_gain_1(); }
						break;
					case 0xFF11:                                        // NR11 (sound length / wave pattern duty %)
						snd_Audio_Regs[snd_NR11] = value;
						snd_SQ1_duty = (uint8_t)((value & 0xC0) >> 6);
						snd_SQ1_length = (uint16_t)(64 - (value & 0x3F));
						snd_SQ1_len_cntr = snd_SQ1_length;
						break;
					case 0xFF12:                                        // NR12 (envelope)
						snd_SQ1_st_vol = (uint8_t)((value & 0xF0) >> 4);
						snd_SQ1_env_add = (value & 8) > 0;
						snd_SQ1_per = (uint8_t)(value & 7);

						// several glitchy effects happen when writing to NRx2 during audio playing
						if (((snd_Audio_Regs[snd_NR12] & 7) == 0) && !snd_SQ1_vol_done) { snd_SQ1_vol_state++; }
						else if ((snd_Audio_Regs[snd_NR12] & 8) == 0) { snd_SQ1_vol_state += 2; }

						if (((snd_Audio_Regs[snd_NR12] ^ value) & 8) > 0) { snd_SQ1_vol_state = (uint8_t)(0x10 - snd_SQ1_vol_state); }

						snd_SQ1_vol_state &= 0xF;

						if ((value & 0xF8) == 0) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
						snd_Audio_Regs[snd_NR12] = value;

						snd_calculate_bias_gain_1();
						break;
					case 0xFF13:                                        // NR13 (freq low)
						snd_Audio_Regs[snd_NR13] = value;
						snd_SQ1_frq &= 0x700;
						snd_SQ1_frq |= value;
						break;
					case 0xFF14:                                        // NR14 (freq hi)
						snd_Audio_Regs[snd_NR14] = value;
						snd_SQ1_trigger = (value & 0x80) > 0;
						snd_SQ1_frq &= 0xFF;
						snd_SQ1_frq |= (uint16_t)((value & 7) << 8);

						if (((snd_Sequencer_len & 1) == 0))
						{
							if (!snd_SQ1_len_en && ((value & 0x40) > 0) && (snd_SQ1_len_cntr > 0))
							{
								snd_SQ1_len_cntr--;
								if ((snd_SQ1_len_cntr == 0) && !snd_SQ1_trigger) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
							}
						}

						if (snd_SQ1_trigger)
						{
							snd_SQ1_enable = true;
							snd_SQ1_vol_done = false;
							snd_SQ1_duty_cntr = 0;

							if (snd_SQ1_len_cntr == 0)
							{
								snd_SQ1_len_cntr = 64;
								if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_SQ1_len_cntr--; }
							}
							snd_SQ1_vol_state = snd_SQ1_st_vol;
							snd_SQ1_vol_per = (snd_SQ1_per > 0) ? snd_SQ1_per : 8;
							if (snd_Sequencer_vol == 4) { snd_SQ1_vol_per++; }
							snd_SQ1_frq_shadow = snd_SQ1_frq;
							snd_SQ1_intl_cntr = ((2048 - snd_SQ1_frq_shadow) * 4) | (snd_SQ1_intl_cntr & 3);

							snd_SQ1_intl_swp_cnt = snd_SQ1_swp_prd > 0 ? snd_SQ1_swp_prd : 8;
							snd_SQ1_calc_done = false;

							if ((snd_SQ1_shift > 0) || (snd_SQ1_swp_prd > 0))
							{
								snd_SQ1_swp_enable = true;
							}
							else
							{
								snd_SQ1_swp_enable = false;
							}

							if (snd_SQ1_shift > 0)
							{
								int shadow_frq = snd_SQ1_frq_shadow;
								shadow_frq >>= snd_SQ1_shift;
								if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
								shadow_frq += snd_SQ1_frq_shadow;

								// disable channel if overflow
								if ((uint32_t)shadow_frq > 2047)
								{
									snd_SQ1_enable = snd_SQ1_swp_enable = false;
								}

								// set negate mode flag that disables channel is negate clerar
								if (snd_SQ1_negate) { snd_SQ1_calc_done = true; }
							}

							if ((snd_SQ1_vol_state == 0) && !snd_SQ1_env_add) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
						}

						snd_calculate_bias_gain_1();
						snd_SQ1_len_en = (value & 0x40) > 0;
						break;
					case 0xFF16:                                        // NR21 (sound length / wave pattern duty %)
						snd_Audio_Regs[snd_NR21] = value;
						snd_SQ2_duty = (uint8_t)((value & 0xC0) >> 6);
						snd_SQ2_length = (uint16_t)(64 - (value & 0x3F));
						snd_SQ2_len_cntr = snd_SQ2_length;
						break;
					case 0xFF17:                                        // NR22 (envelope)
						snd_SQ2_st_vol = (uint8_t)((value & 0xF0) >> 4);
						snd_SQ2_env_add = (value & 8) > 0;
						snd_SQ2_per = (uint8_t)(value & 7);

						// several glitchy effects happen when writing to NRx2 during audio playing
						if (((snd_Audio_Regs[snd_NR22] & 7) == 0) && !snd_SQ2_vol_done) { snd_SQ2_vol_state++; }
						else if ((snd_Audio_Regs[snd_NR22] & 8) == 0) { snd_SQ2_vol_state += 2; }

						if (((snd_Audio_Regs[snd_NR22] ^ value) & 8) > 0) { snd_SQ2_vol_state = (uint8_t)(0x10 - snd_SQ2_vol_state); }

						snd_SQ2_vol_state &= 0xF;
						if ((value & 0xF8) == 0) { snd_SQ2_enable = false; }
						snd_Audio_Regs[snd_NR22] = value;

						snd_calculate_bias_gain_2();
						break;
					case 0xFF18:                                        // NR23 (freq low)
						snd_Audio_Regs[snd_NR23] = value;
						snd_SQ2_frq &= 0x700;
						snd_SQ2_frq |= value;
						break;
					case 0xFF19:                                        // NR24 (freq hi)
						snd_Audio_Regs[snd_NR24] = value;
						snd_SQ2_trigger = (value & 0x80) > 0;
						snd_SQ2_frq &= 0xFF;
						snd_SQ2_frq |= (uint16_t)((value & 7) << 8);

						if ((snd_Sequencer_len & 1) == 0)
						{
							if (!snd_SQ2_len_en && ((value & 0x40) > 0) && (snd_SQ2_len_cntr > 0))
							{
								snd_SQ2_len_cntr--;
								if ((snd_SQ2_len_cntr == 0) && !snd_SQ2_trigger) { snd_SQ2_enable = false; }
							}
						}

						if (snd_SQ2_trigger)
						{
							snd_SQ2_enable = true;
							snd_SQ2_vol_done = false;
							snd_SQ2_duty_cntr = 0;

							if (snd_SQ2_len_cntr == 0)
							{
								snd_SQ2_len_cntr = 64;
								if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_SQ2_len_cntr--; }
							}
							snd_SQ2_intl_cntr = ((2048 - snd_SQ2_frq) * 4) | (snd_SQ2_intl_cntr & 3);
							snd_SQ2_vol_state = snd_SQ2_st_vol;
							snd_SQ2_vol_per = (snd_SQ2_per > 0) ? snd_SQ2_per : 8;
							if (snd_Sequencer_vol == 4) { snd_SQ2_vol_per++; }
							if ((snd_SQ2_vol_state == 0) && !snd_SQ2_env_add) { snd_SQ2_enable = false; }
						}
						snd_calculate_bias_gain_2();
						snd_SQ2_len_en = (value & 0x40) > 0;
						break;
					case 0xFF1A:                                        // NR30 (on/off)
						snd_Audio_Regs[snd_NR30] = value;
						snd_WAVE_DAC_pow = (value & 0x80) > 0;
						if (!snd_WAVE_DAC_pow) { snd_WAVE_enable = false; }
						snd_calculate_bias_gain_w();
						break;
					case 0xFF1B:                                        // NR31 (length)
						snd_Audio_Regs[snd_NR31] = value;
						snd_WAVE_length = (uint16_t)(256 - value);
						snd_WAVE_len_cntr = snd_WAVE_length;
						break;
					case 0xFF1C:                                        // NR32 (level output)
						snd_Audio_Regs[snd_NR32] = value;
						snd_WAVE_vol_code = (uint8_t)((value & 0x60) >> 5);
						break;
					case 0xFF1D:                                        // NR33 (freq low)
						snd_Audio_Regs[snd_NR33] = value;
						snd_WAVE_frq &= 0x700;
						snd_WAVE_frq |= value;
						break;
					case 0xFF1E:                                        // NR34 (freq hi)
						snd_Audio_Regs[snd_NR34] = value;
						snd_WAVE_trigger = (value & 0x80) > 0;
						snd_WAVE_frq &= 0xFF;
						snd_WAVE_frq |= (uint16_t)((value & 7) << 8);

						if ((snd_Sequencer_len & 1) == 0)
						{

							if (!snd_WAVE_len_en && ((value & 0x40) > 0) && (snd_WAVE_len_cntr > 0))
							{
								snd_WAVE_len_cntr--;
								if ((snd_WAVE_len_cntr == 0) && !snd_WAVE_trigger) { snd_WAVE_enable = false; }
							}
						}

						if (snd_WAVE_trigger)
						{
							// some corruption occurs if triggering while reading
							if (snd_WAVE_enable && (snd_WAVE_intl_cntr == 2) && !Is_GBC)
							{
								// we want to use the previous wave cntr value since it was just incremented
								int t_wave_cntr = (snd_WAVE_wave_cntr + 1) & 31;
								if ((t_wave_cntr >> 1) < 4)
								{
									snd_Wave_RAM[0] = snd_Wave_RAM[t_wave_cntr >> 1];
								}
								else
								{
									snd_Wave_RAM[0] = snd_Wave_RAM[(t_wave_cntr >> 3) * 4];
									snd_Wave_RAM[1] = snd_Wave_RAM[(t_wave_cntr >> 3) * 4 + 1];
									snd_Wave_RAM[2] = snd_Wave_RAM[(t_wave_cntr >> 3) * 4 + 2];
									snd_Wave_RAM[3] = snd_Wave_RAM[(t_wave_cntr >> 3) * 4 + 3];
								}
							}

							snd_WAVE_enable = true;

							if (snd_WAVE_len_cntr == 0)
							{
								snd_WAVE_len_cntr = 256;
								if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_WAVE_len_cntr--; }
							}
							snd_WAVE_intl_cntr = (2048 - snd_WAVE_frq) * 2 + 6; // trigger delay for wave channel

							snd_WAVE_wave_cntr = 0;
							if (!snd_WAVE_DAC_pow) { snd_WAVE_enable = false; }
						}

						snd_calculate_bias_gain_w();
						snd_WAVE_len_en = (value & 0x40) > 0;
						break;
					case 0xFF20:                                        // NR41 (length)
						snd_Audio_Regs[snd_NR41] = value;
						snd_NOISE_length = (uint16_t)(64 - (value & 0x3F));
						snd_NOISE_len_cntr = snd_NOISE_length;
						break;
					case 0xFF21:                                        // NR42 (envelope)
						snd_NOISE_st_vol = (uint8_t)((value & 0xF0) >> 4);
						snd_NOISE_env_add = (value & 8) > 0;
						snd_NOISE_per = (uint8_t)(value & 7);

						// several glitchy effects happen when writing to NRx2 during audio playing
						if (((snd_Audio_Regs[snd_NR42] & 7) == 0) && !snd_NOISE_vol_done) { snd_NOISE_vol_state++; }
						else if ((snd_Audio_Regs[snd_NR42] & 8) == 0) { snd_NOISE_vol_state += 2; }

						if (((snd_Audio_Regs[snd_NR42] ^ value) & 8) > 0) { snd_NOISE_vol_state = (uint8_t)(0x10 - snd_NOISE_vol_state); }

						snd_NOISE_vol_state &= 0xF;
						if ((value & 0xF8) == 0) { snd_NOISE_enable = false; }
						snd_Audio_Regs[snd_NR42] = value;

						snd_calculate_bias_gain_n();
						break;
					case 0xFF22:                                        // NR43 (shift)
						snd_Audio_Regs[snd_NR43] = value;
						snd_NOISE_clk_shft = (uint8_t)((value & 0xF0) >> 4);
						snd_NOISE_wdth_md = (value & 8) > 0;
						snd_NOISE_div_code = (uint8_t)(value & 7);
						// Mickey's Dangerous Chase requires writes here to take effect immediately (for sound of taking damage)
						snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
						break;
					case 0xFF23:                                        // NR44 (trigger)
						snd_Audio_Regs[snd_NR44] = value;
						snd_NOISE_trigger = (value & 0x80) > 0;

						if ((snd_Sequencer_len & 1) == 0)
						{
							if (!snd_NOISE_len_en && ((value & 0x40) > 0) && (snd_NOISE_len_cntr > 0))
							{
								snd_NOISE_len_cntr--;
								if ((snd_NOISE_len_cntr == 0) && !snd_NOISE_trigger) { snd_NOISE_enable = false; }
							}
						}

						if (snd_NOISE_trigger)
						{
							snd_NOISE_enable = true;
							snd_NOISE_vol_done = false;

							if (snd_NOISE_len_cntr == 0)
							{
								snd_NOISE_len_cntr = 64;
								if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_NOISE_len_cntr--; }
							}
							snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
							snd_NOISE_vol_state = snd_NOISE_st_vol;
							snd_NOISE_vol_per = (snd_NOISE_per > 0) ? snd_NOISE_per : 8;
							if (snd_Sequencer_vol == 4) { snd_NOISE_vol_per++; }
							snd_NOISE_LFSR = 0x7FFF;
							if ((snd_NOISE_vol_state == 0) && !snd_NOISE_env_add) { snd_NOISE_enable = false; }
						}

						snd_calculate_bias_gain_n();
						snd_NOISE_len_en = (value & 0x40) > 0;
						break;
					case 0xFF24:                                        // NR50 (ctrl)
						snd_Audio_Regs[snd_NR50] = value;
						snd_CTRL_vin_L_en = (value & 0x80) > 0;
						snd_CTRL_vol_L = (uint8_t)((value & 0x70) >> 4);
						snd_CTRL_vin_R_en = (value & 8) > 0;
						snd_CTRL_vol_R = (uint8_t)(value & 7);

						snd_calculate_bias_gain_a();
						break;
					case 0xFF25:                                        // NR51 (ctrl)
						snd_Audio_Regs[snd_NR51] = value;
						snd_CTRL_noise_L_en = (value & 0x80) > 0;
						snd_CTRL_wave_L_en = (value & 0x40) > 0;
						snd_CTRL_sq2_L_en = (value & 0x20) > 0;
						snd_CTRL_sq1_L_en = (value & 0x10) > 0;
						snd_CTRL_noise_R_en = (value & 8) > 0;
						snd_CTRL_wave_R_en = (value & 4) > 0;
						snd_CTRL_sq2_R_en = (value & 2) > 0;
						snd_CTRL_sq1_R_en = (value & 1) > 0;

						snd_calculate_bias_gain_a();
						break;
					case 0xFF26:                                        // NR52 (ctrl)
						// NOTE: Make sure to do the power off first since it will call the write_reg function again
						if ((value & 0x80) == 0) { snd_power_off(); }
						snd_CTRL_power = (value & 0x80) > 0;
						break;

						// wave ram table
					case 0xFF30:
					case 0xFF31:
					case 0xFF32:
					case 0xFF33:
					case 0xFF34:
					case 0xFF35:
					case 0xFF36:
					case 0xFF37:
					case 0xFF38:
					case 0xFF39:
					case 0xFF3A:
					case 0xFF3B:
					case 0xFF3C:
					case 0xFF3D:
					case 0xFF3E:
					case 0xFF3F:
						if (snd_WAVE_enable)
						{
							if (snd_WAVE_can_get || Is_GBC) { snd_Wave_RAM[snd_WAVE_wave_cntr >> 1] = value; }
						}
						else
						{
							snd_Wave_RAM[addr & 0xF] = value;
						}

						break;
				}
			}
			// when power is off, only length counters and waveRAM are effected by writes
			// ON GBC, length counters cannot be written to either
			else
			{
				switch (addr)
				{
					case 0xFF11:                                        // NR11 (sound length / wave pattern duty %)
						if (!Is_GBC)
						{
							snd_SQ1_length = (uint16_t)(64 - (value & 0x3F));
							snd_SQ1_len_cntr = snd_SQ1_length;
						}
						break;
					case 0xFF16:                                        // NR21 (sound length / wave pattern duty %)
						if (!Is_GBC)
						{
							snd_SQ2_length = (uint16_t)(64 - (value & 0x3F));
							snd_SQ2_len_cntr = snd_SQ2_length;
						}
						break;
					case 0xFF1B:                                        // NR31 (length)
						if (!Is_GBC)
						{
							snd_WAVE_length = (uint16_t)(256 - value);
							snd_WAVE_len_cntr = snd_WAVE_length;
						}
						break;
					case 0xFF20:                                        // NR41 (length)
						if (!Is_GBC)
						{
							snd_NOISE_length = (uint16_t)(64 - (value & 0x3F));
							snd_NOISE_len_cntr = snd_NOISE_length;
						}
						break;
					case 0xFF26:                                        // NR52 (ctrl)
						snd_CTRL_power = (value & 0x80) > 0;
						if (snd_CTRL_power)
						{
							snd_Sequencer_reset_cd = 4;
						}
						break;

						// wave ram table
					case 0xFF30:
					case 0xFF31:
					case 0xFF32:
					case 0xFF33:
					case 0xFF34:
					case 0xFF35:
					case 0xFF36:
					case 0xFF37:
					case 0xFF38:
					case 0xFF39:
					case 0xFF3A:
					case 0xFF3B:
					case 0xFF3C:
					case 0xFF3D:
					case 0xFF3E:
					case 0xFF3F:
						snd_Wave_RAM[addr & 0x0F] = value;
						break;
				}
			}
		}

		void snd_Tick()
		{
			// calculate square1's output
			if (snd_SQ1_enable)
			{
				snd_SQ1_intl_cntr--;
				if (snd_SQ1_intl_cntr == 0)
				{
					snd_SQ1_intl_cntr = (2048 - snd_SQ1_frq) * 4;
					snd_SQ1_duty_cntr++;
					snd_SQ1_duty_cntr &= 7;

					snd_SQ1_output = snd_Duty_Cycles[snd_SQ1_duty * 8 + snd_SQ1_duty_cntr] ? (snd_SQ1_vol_state + snd_DAC_OFST) : snd_DAC_OFST;

					snd_Update_Needed = true;
				}
			}

			// calculate square2's output
			if (snd_SQ2_enable)
			{
				snd_SQ2_intl_cntr--;
				if (snd_SQ2_intl_cntr == 0)
				{
					snd_SQ2_intl_cntr = (2048 - snd_SQ2_frq) * 4;
					snd_SQ2_duty_cntr++;
					snd_SQ2_duty_cntr &= 7;

					snd_SQ2_output = snd_Duty_Cycles[snd_SQ2_duty * 8 + snd_SQ2_duty_cntr] ? (snd_SQ2_vol_state + snd_DAC_OFST) : snd_DAC_OFST;

					snd_Update_Needed = true;
				}
			}

			// calculate wave output
			snd_WAVE_can_get = false;
			if (snd_WAVE_enable)
			{
				snd_WAVE_intl_cntr--;

				if (snd_WAVE_intl_cntr == 0)
				{
					snd_WAVE_can_get = true;

					snd_WAVE_intl_cntr = (2048 - snd_WAVE_frq) * 2;

					if ((snd_WAVE_wave_cntr & 1) == 0)
					{
						snd_Sample = (uint8_t)(snd_Sample >> 4);
					}

					if (snd_WAVE_vol_code == 0)
					{
						snd_Sample = (uint8_t)((snd_Sample & 0xF) >> 4);
					}
					else if (snd_WAVE_vol_code == 1)
					{
						snd_Sample = (uint8_t)(snd_Sample & 0xF);
					}
					else if (snd_WAVE_vol_code == 2)
					{
						snd_Sample = (uint8_t)((snd_Sample & 0xF) >> 1);
					}
					else
					{
						snd_Sample = (uint8_t)((snd_Sample & 0xF) >> 2);
					}

					snd_WAVE_output = snd_Sample + snd_DAC_OFST;

					// NOTE: The sample buffer is only reloaded after the current sample is played, even if just triggered
					snd_WAVE_wave_cntr++;
					snd_WAVE_wave_cntr &= 0x1F;
					snd_Sample = snd_Wave_RAM[snd_WAVE_wave_cntr >> 1];

					snd_Update_Needed = true;
				}
			}
			else if (!snd_WAVE_decay_done && (++snd_WAVE_decay_counter == 200))
			{
				snd_WAVE_decay_counter = 0;

				// wave state must decay slow enough that games that turn on and off the wave channel to fill wave RAM don't buzz too much
				if (!snd_WAVE_DAC_pow)
				{
					if (snd_WAVE_output > 0) { snd_WAVE_output--; snd_Update_Needed = true; }
					else { snd_WAVE_decay_done = true; }
				}
				else
				{
					if (snd_WAVE_output > snd_DAC_OFST)
					{
						snd_WAVE_output--; snd_Update_Needed = true;
					}
					else if (snd_WAVE_output < snd_DAC_OFST)
					{
						snd_WAVE_output++; snd_Update_Needed = true;
					}
					else { snd_WAVE_decay_done = true; }
				}
			}

			// calculate noise output
			if (snd_NOISE_enable)
			{
				snd_NOISE_intl_cntr--;
				if (snd_NOISE_intl_cntr == 0)
				{
					snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
					int bit_lfsr = (snd_NOISE_LFSR & 1) ^ ((snd_NOISE_LFSR & 2) >> 1);

					snd_NOISE_LFSR = (snd_NOISE_LFSR >> 1) & 0x3FFF;
					snd_NOISE_LFSR |= (bit_lfsr << 14);

					if (snd_NOISE_wdth_md)
					{
						snd_NOISE_LFSR &= 0x7FBF;
						snd_NOISE_LFSR |= (bit_lfsr << 6);
					}

					snd_NOISE_output = (snd_NOISE_LFSR & 1) > 0 ? snd_DAC_OFST : (snd_NOISE_vol_state + snd_DAC_OFST);
					snd_Update_Needed = true;
				}
			}

			// frame snd_Sequencer ticks at a rate of 512 hz (or every time a 13 bit counter rolls over)
			// the snd_Sequencer is actually the timer DIV register
			// so if it's constantly written to, these values won't update

			if (DIV_falling_edge && snd_CTRL_power)
			{
				snd_Sequencer_vol++; snd_Sequencer_vol &= 0x7;
				snd_Sequencer_len++; snd_Sequencer_len &= 0x7;
				snd_Sequencer_swp++; snd_Sequencer_swp &= 0x7;

				// clock the lengths
				if ((snd_Sequencer_len & 1) == 0)
				{
					if (snd_SQ1_len_en && snd_SQ1_len_cntr > 0)
					{
						snd_SQ1_len_cntr--;
						if (snd_SQ1_len_cntr == 0) { snd_SQ1_enable = snd_SQ1_swp_enable = false; snd_calculate_bias_gain_1(); }
					}
					if (snd_SQ2_len_en && snd_SQ2_len_cntr > 0)
					{
						snd_SQ2_len_cntr--;
						if (snd_SQ2_len_cntr == 0) { snd_SQ2_enable = false; snd_calculate_bias_gain_2(); }
					}
					if (snd_WAVE_len_en && snd_WAVE_len_cntr > 0)
					{
						snd_WAVE_len_cntr--;
						if (snd_WAVE_len_cntr == 0) { snd_WAVE_enable = false; snd_calculate_bias_gain_w(); }
					}
					if (snd_NOISE_len_en && snd_NOISE_len_cntr > 0)
					{
						snd_NOISE_len_cntr--;
						if (snd_NOISE_len_cntr == 0) { snd_NOISE_enable = false; snd_calculate_bias_gain_n(); }
					}
				}

				// clock the sweep
				if ((snd_Sequencer_swp == 0) || (snd_Sequencer_swp == 4))
				{
					snd_SQ1_intl_swp_cnt--;
					if ((snd_SQ1_intl_swp_cnt == 0) && snd_SQ1_swp_enable)
					{
						snd_SQ1_intl_swp_cnt = snd_SQ1_swp_prd > 0 ? snd_SQ1_swp_prd : 8;

						if ((snd_SQ1_swp_prd > 0))
						{
							int shadow_frq = snd_SQ1_frq_shadow;
							shadow_frq >>= snd_SQ1_shift;
							if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
							shadow_frq += snd_SQ1_frq_shadow;

							// set negate mode flag that disables channel is negate clerar
							if (snd_SQ1_negate) { snd_SQ1_calc_done = true; }

							// disable channel if overflow
							if ((uint32_t)shadow_frq > 2047)
							{
								snd_SQ1_enable = snd_SQ1_swp_enable = false; snd_calculate_bias_gain_1();
							}
							else
							{
								if (snd_SQ1_shift > 0)
								{
									shadow_frq &= 0x7FF;
									snd_SQ1_frq = shadow_frq;
									snd_SQ1_frq_shadow = shadow_frq;

									// note that we also write back the frequency to the actual register
									snd_Audio_Regs[snd_NR13] = (uint8_t)(snd_SQ1_frq & 0xFF);
									snd_Audio_Regs[snd_NR14] &= 0xF8;
									snd_Audio_Regs[snd_NR14] |= (uint8_t)((snd_SQ1_frq >> 8) & 7);

									// after writing, we repeat the process and do another overflow check
									shadow_frq = snd_SQ1_frq_shadow;
									shadow_frq >>= snd_SQ1_shift;
									if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
									shadow_frq += snd_SQ1_frq_shadow;

									if ((uint32_t)shadow_frq > 2047)
									{
										snd_SQ1_enable = snd_SQ1_swp_enable = false; snd_calculate_bias_gain_1();
									}
								}
							}
						}
					}
				}

				// clock the volume envelope
				if (snd_Sequencer_vol == 5)
				{
					if (snd_SQ1_per > 0)
					{
						snd_SQ1_vol_per--;
						if (snd_SQ1_vol_per == 0)
						{
							snd_SQ1_vol_per = (snd_SQ1_per > 0) ? snd_SQ1_per : 8;
							if (!snd_SQ1_vol_done)
							{
								if (snd_SQ1_env_add)
								{
									if (snd_SQ1_vol_state < 15) { snd_SQ1_vol_state++; snd_Update_Needed = true; }
									else { snd_SQ1_vol_done = true; }
								}
								else
								{
									if (snd_SQ1_vol_state >= 1) { snd_SQ1_vol_state--; snd_Update_Needed = true; }
									else { snd_SQ1_vol_done = true; }
								}
							}
						}
					}

					if (snd_SQ2_per > 0)
					{
						snd_SQ2_vol_per--;
						if (snd_SQ2_vol_per == 0)
						{
							snd_SQ2_vol_per = (snd_SQ2_per > 0) ? snd_SQ2_per : 8;
							if (!snd_SQ2_vol_done)
							{
								if (snd_SQ2_env_add)
								{
									if (snd_SQ2_vol_state < 15) { snd_SQ2_vol_state++; snd_Update_Needed = true; }
									else { snd_SQ2_vol_done = true; }
								}
								else
								{
									if (snd_SQ2_vol_state >= 1) { snd_SQ2_vol_state--; snd_Update_Needed = true; }
									else { snd_SQ2_vol_done = true; }
								}
							}
						}
					}

					if (snd_NOISE_per > 0)
					{
						snd_NOISE_vol_per--;
						if (snd_NOISE_vol_per == 0)
						{
							snd_NOISE_vol_per = (snd_NOISE_per > 0) ? snd_NOISE_per : 8;
							if (!snd_NOISE_vol_done)
							{
								if (snd_NOISE_env_add)
								{
									if (snd_NOISE_vol_state < 15) { snd_NOISE_vol_state++; snd_Update_Needed = true; }
									else { snd_NOISE_vol_done = true; }
								}
								else
								{
									if (snd_NOISE_vol_state >= 1) { snd_NOISE_vol_state--; snd_Update_Needed = true; }
									else { snd_NOISE_vol_done = true; }
								}
							}
						}
					}
				}
			}

			DIV_falling_edge = false;

			if (snd_Sequencer_reset_cd > 0)
			{
				snd_Sequencer_reset_cd--;

				if (snd_Sequencer_reset_cd == 0)
				{
					// seems to be off by one issues here, hard to tell since the write takes place in the cpu loop
					// but the effect takes place in the sound loop
					if (Double_Speed)
					{
						snd_Sequencer_len = ((tim_Divider_Reg - 1) & 0x2000) ? 0 : 1;
						snd_Sequencer_vol = ((tim_Divider_Reg - 1) & 0x2000) ? 0 : 1;
						snd_Sequencer_swp = ((tim_Divider_Reg - 1) & 0x2000) ? 0 : 1;
					}
					else
					{
						snd_Sequencer_len = ((tim_Divider_Reg - 1) & 0x1000) ? 0 : 1;
						snd_Sequencer_vol = ((tim_Divider_Reg - 1) & 0x1000) ? 0 : 1;
						snd_Sequencer_swp = ((tim_Divider_Reg - 1) & 0x1000) ? 0 : 1;
					}
				}
			}

			if (snd_Update_Needed)
			{
				// add up components to each channel
				int L_final = 0;
				int R_final = 0;

				if (snd_CTRL_sq1_L_en) { L_final += snd_SQ1_output; }
				if (snd_CTRL_sq2_L_en) { L_final += snd_SQ2_output; }
				if (snd_CTRL_wave_L_en) { L_final += snd_WAVE_output; }
				if (snd_CTRL_noise_L_en) { L_final += snd_NOISE_output; }

				if (snd_CTRL_sq1_R_en) { R_final += snd_SQ1_output; }
				if (snd_CTRL_sq2_R_en) { R_final += snd_SQ2_output; }
				if (snd_CTRL_wave_R_en) { R_final += snd_WAVE_output; }
				if (snd_CTRL_noise_R_en) { R_final += snd_NOISE_output; }

				L_final *= (snd_CTRL_vol_L + 1) * 40;
				R_final *= (snd_CTRL_vol_R + 1) * 40;

				if (L_final != snd_Latched_Sample_L)
				{
					samples_L[num_samples_L * 2] = snd_Master_Clock;
					samples_L[num_samples_L * 2 + 1] = L_final - snd_Latched_Sample_L;

					num_samples_L += 1;

					snd_Latched_Sample_L = L_final;
				}

				if (R_final != snd_Latched_Sample_R)
				{
					samples_R[num_samples_R * 2] = snd_Master_Clock;
					samples_R[num_samples_R * 2 + 1] = R_final - snd_Latched_Sample_R;

					num_samples_R += 1;

					snd_Latched_Sample_R = R_final;
				}
			}

			snd_Master_Clock++;
			snd_Update_Needed = false;
		}


		void snd_power_off()
		{
			for (uint32_t i = 0; i < 0x16; i++)
			{
				snd_WriteReg(0xFF10 + i, 0);
			}

			snd_calculate_bias_gain_a();

			// duty and length are reset
			snd_SQ1_duty_cntr = snd_SQ2_duty_cntr = 0;

			// reset state variables
			snd_SQ1_enable = snd_SQ1_swp_enable = snd_SQ2_enable = snd_WAVE_enable = snd_NOISE_enable = false;

			snd_SQ1_len_en = snd_SQ2_len_en = snd_WAVE_len_en = snd_NOISE_len_en = false;

			snd_SQ1_output = snd_SQ2_output = snd_WAVE_output = snd_NOISE_output = 0;

			// on GBC, lengths are also reset
			if (Is_GBC)
			{
				snd_SQ1_length = snd_SQ2_length = snd_WAVE_length = snd_NOISE_length = 0;
				snd_SQ1_len_cntr = snd_SQ2_len_cntr = snd_WAVE_len_cntr = snd_NOISE_len_cntr = 0;
			}
		}

		void snd_Reset()
		{
			if (Is_GBC)
			{
				for (int i = 0; i < 16; i++)
				{
					snd_Wave_RAM[i] = (uint8_t)(((i & 1) == 0) ? 0 : 0xFF);
				}
			}
			else
			{
				for (int i = 0; i < 16; i++)
				{
					snd_Wave_RAM[i] = snd_WAVE_RAM_INI[i];
				}			
			}

			for (int i = 0; i < 21; i++)
			{
				snd_Audio_Regs[i] = 0;
			}

			for (int i = 0; i < 0x16; i++)
			{
				snd_WriteReg(0xFF10 + i, 0);
			}

			snd_calculate_bias_gain_a();

			snd_SQ1_duty_cntr = snd_SQ2_duty_cntr = 0;

			snd_SQ1_enable = snd_SQ1_swp_enable = snd_SQ2_enable = snd_WAVE_enable = snd_NOISE_enable = false;

			snd_SQ1_len_en = snd_SQ2_len_en = snd_WAVE_len_en = snd_NOISE_len_en = false;

			snd_SQ1_output = snd_SQ2_output = snd_WAVE_output = snd_NOISE_output = 0;

			snd_SQ1_length = snd_SQ2_length = snd_WAVE_length = snd_NOISE_length = 0;
			snd_SQ1_len_cntr = snd_SQ2_len_cntr = snd_WAVE_len_cntr = snd_NOISE_len_cntr = 0;

			snd_Master_Clock = 0;

			snd_Internal_cnt = snd_Divider = 0;

			snd_Sequencer_len = 0;
			snd_Sequencer_swp = 0;
			snd_Sequencer_vol = 0;
			snd_Sequencer_reset_cd = 0;

			snd_Sample = 0;

			snd_CTRL_GB_Low = snd_CTRL_GB_High = 0;
			snd_Bias_Low = snd_Bias_High = 0;

			snd_Wave_Bank = snd_Wave_Bank_Playing = 0;

			snd_BIAS_Offset = 0;

			snd_Sample_Rate = 0x1FF;
		}

		uint8_t snd_Read_NR52()
		{
			return (uint8_t)(
				((snd_CTRL_power ? 1 : 0) << 7) |
				(snd_SQ1_enable ? 1 : 0) |
				((snd_SQ2_enable ? 1 : 0) << 1) |
				((snd_WAVE_enable ? 1 : 0) << 2) |
				((snd_NOISE_enable ? 1 : 0) << 3));
		}

		void snd_calculate_bias_gain_a()
		{
			if (!snd_SQ1_enable && ((snd_Audio_Regs[snd_NR12] & 0xF8) > 0)) { snd_SQ1_output = snd_DAC_OFST; }
			else if ((snd_Audio_Regs[snd_NR12] & 0xF8) == 0) { snd_SQ1_output = 0; }

			if (!snd_SQ2_enable && ((snd_Audio_Regs[snd_NR22] & 0xF8) > 0)) { snd_SQ2_output = snd_DAC_OFST; }
			else if ((snd_Audio_Regs[snd_NR22] & 0xF8) == 0) { snd_SQ2_output = 0; }

			if (!snd_WAVE_enable && snd_WAVE_DAC_pow) { snd_WAVE_decay_counter = 0; snd_WAVE_decay_done = false; }
			else if (!snd_WAVE_DAC_pow) { snd_WAVE_decay_counter = 0; snd_WAVE_decay_done = false; }

			if (!snd_NOISE_enable && ((snd_Audio_Regs[snd_NR42] & 0xF8) > 0)) { snd_NOISE_output = snd_DAC_OFST; }
			else if ((snd_Audio_Regs[snd_NR42] & 0xF8) == 0) { snd_NOISE_output = 0; }

			snd_Update_Needed = true;
		}

		void snd_calculate_bias_gain_1()
		{
			if (!snd_SQ1_enable && ((snd_Audio_Regs[snd_NR12] & 0xF8) > 0)) { snd_SQ1_output = snd_DAC_OFST; }
			else if ((snd_Audio_Regs[snd_NR12] & 0xF8) == 0) { snd_SQ1_output = 0; }

			snd_Update_Needed = true;
		}

		void snd_calculate_bias_gain_2()
		{
			if (!snd_SQ2_enable && ((snd_Audio_Regs[snd_NR22] & 0xF8) > 0)) { snd_SQ2_output = snd_DAC_OFST; }
			else if ((snd_Audio_Regs[snd_NR22] & 0xF8) == 0) { snd_SQ2_output = 0; }

			snd_Update_Needed = true;
		}

		void snd_calculate_bias_gain_w()
		{
			if (!snd_WAVE_enable && snd_WAVE_DAC_pow) { snd_WAVE_decay_counter = 0; snd_WAVE_decay_done = false; }
			else if (!snd_WAVE_DAC_pow) { snd_WAVE_decay_counter = 0; snd_WAVE_decay_done = false; }

			snd_Update_Needed = true;
		}

		void snd_calculate_bias_gain_n()
		{
			if (!snd_NOISE_enable && ((snd_Audio_Regs[snd_NR42] & 0xF8) > 0)) { snd_NOISE_output = snd_DAC_OFST; }
			else if ((snd_Audio_Regs[snd_NR42] & 0xF8) == 0) { snd_NOISE_output = 0; }

			snd_Update_Needed = true;
		}

		uint8_t* snd_SaveState(uint8_t* saver)
		{
			saver = byte_array_saver(snd_Audio_Regs, saver, 21);

			saver = byte_array_saver(snd_Wave_RAM, saver, 16);

			saver = int_saver(snd_DAC_Offset, saver);
			saver = int_saver(snd_Wave_Decay_cnt, saver);
			saver = int_saver(snd_Internal_cnt, saver);
			saver = int_saver(snd_Divider, saver);

			saver = bool_saver(snd_Wave_Decay_Done, saver);

			// Audio Variables
			// derived
			saver = bool_saver(snd_WAVE_DAC_pow, saver);
			saver = bool_saver(snd_NOISE_wdth_md, saver);
			saver = bool_saver(snd_SQ1_negate, saver);
			saver = bool_saver(snd_SQ1_trigger, saver); 
			saver = bool_saver(snd_SQ2_trigger, saver);
			saver = bool_saver(snd_WAVE_trigger, saver);
			saver = bool_saver(snd_NOISE_trigger, saver);
			saver = bool_saver(snd_SQ1_len_en, saver);
			saver = bool_saver(snd_SQ2_len_en, saver);
			saver = bool_saver(snd_WAVE_len_en, saver);
			saver = bool_saver(snd_NOISE_len_en, saver);
			saver = bool_saver(snd_SQ1_env_add, saver);
			saver = bool_saver(snd_SQ2_env_add, saver);
			saver = bool_saver(snd_NOISE_env_add, saver);
			saver = byte_saver(snd_WAVE_vol_code, saver);
			saver = byte_saver(snd_NOISE_clk_shft, saver);
			saver = byte_saver(snd_NOISE_div_code, saver);
			saver = byte_saver(snd_SQ1_shift, saver);
			saver = byte_saver(snd_SQ1_duty, saver);
			saver = byte_saver(snd_SQ2_duty, saver);
			saver = byte_saver(snd_SQ1_st_vol, saver);
			saver = byte_saver(snd_SQ2_st_vol, saver); 
			saver = byte_saver(snd_NOISE_st_vol, saver);
			saver = byte_saver(snd_SQ1_per, saver);
			saver = byte_saver(snd_SQ2_per, saver); 
			saver = byte_saver(snd_NOISE_per, saver);
			saver = byte_saver(snd_SQ1_swp_prd, saver);

			saver = int_saver(snd_SQ1_frq, saver);
			saver = int_saver(snd_SQ2_frq, saver);
			saver = int_saver(snd_WAVE_frq, saver);

			saver = short_saver(snd_SQ1_length, saver);
			saver = short_saver(snd_SQ2_length, saver);
			saver = short_saver(snd_WAVE_length, saver);
			saver = short_saver(snd_NOISE_length, saver);
			// state
			saver = bool_saver(snd_WAVE_can_get, saver);
			saver = bool_saver(snd_SQ1_calc_done, saver);
			saver = bool_saver(snd_SQ1_swp_enable, saver);
			saver = bool_saver(snd_SQ1_vol_done, saver);
			saver = bool_saver(snd_SQ2_vol_done, saver);
			saver = bool_saver(snd_NOISE_vol_done, saver);
			saver = bool_saver(snd_SQ1_enable, saver);
			saver = bool_saver(snd_SQ2_enable, saver);
			saver = bool_saver(snd_WAVE_enable, saver);
			saver = bool_saver(snd_NOISE_enable, saver);
			saver = byte_saver(snd_SQ1_vol_state, saver);
			saver = byte_saver(snd_SQ2_vol_state, saver);
			saver = byte_saver(snd_NOISE_vol_state, saver);
			saver = byte_saver(snd_SQ1_duty_cntr, saver);
			saver = byte_saver(snd_SQ2_duty_cntr, saver);
			saver = byte_saver(snd_WAVE_wave_cntr, saver);
			saver = int_saver(snd_SQ1_frq_shadow, saver);
			saver = int_saver(snd_SQ1_intl_cntr, saver);
			saver = int_saver(snd_SQ2_intl_cntr, saver);
			saver = int_saver(snd_WAVE_intl_cntr, saver);
			saver = int_saver(snd_NOISE_intl_cntr, saver);
			saver = int_saver(snd_SQ1_vol_per, saver);
			saver = int_saver(snd_SQ2_vol_per, saver);
			saver = int_saver(snd_NOISE_vol_per, saver);
			saver = int_saver(snd_SQ1_intl_swp_cnt, saver);
			saver = int_saver(snd_NOISE_LFSR, saver);
			saver = short_saver(snd_SQ1_len_cntr, saver);
			saver = short_saver(snd_SQ2_len_cntr, saver);
			saver = short_saver(snd_WAVE_len_cntr, saver);
			saver = short_saver(snd_NOISE_len_cntr, saver);

			// GB specific registers
			saver = int_saver(snd_Wave_Bank, saver);
			saver = int_saver(snd_Wave_Bank_Playing, saver);
			saver = int_saver(snd_Chan_Mult, saver);
			saver = byte_saver(snd_CTRL_GB_Low, saver);
			saver = byte_saver(snd_CTRL_GB_High, saver);
			saver = byte_saver(snd_Bias_Low, saver);
			saver = byte_saver(snd_Bias_High, saver);
			saver = bool_saver(snd_Wave_Size, saver);
			saver = bool_saver(snd_Wave_Vol_Force, saver);

			// computed
			saver = int_saver(snd_SQ1_output, saver);
			saver = int_saver(snd_SQ2_output, saver);
			saver = int_saver(snd_WAVE_output, saver);
			saver = int_saver(snd_NOISE_output, saver);

			// Contol Variables
			saver = bool_saver(snd_CTRL_sq1_L_en, saver);
			saver = bool_saver(snd_CTRL_sq2_L_en, saver);
			saver = bool_saver(snd_CTRL_wave_L_en, saver);
			saver = bool_saver(snd_CTRL_noise_L_en, saver);
			saver = bool_saver(snd_CTRL_sq1_R_en, saver);
			saver = bool_saver(snd_CTRL_sq2_R_en, saver);
			saver = bool_saver(snd_CTRL_wave_R_en, saver);
			saver = bool_saver(snd_CTRL_noise_R_en, saver);
			saver = bool_saver(snd_CTRL_power, saver);
			saver = bool_saver(snd_CTRL_vin_L_en, saver);
			saver = bool_saver(snd_CTRL_vin_R_en, saver);

			saver = byte_saver(snd_CTRL_vol_L, saver);
			saver = byte_saver(snd_CTRL_vol_R, saver);

			saver = int_saver(snd_Sequencer_len, saver);
			saver = int_saver(snd_Sequencer_vol, saver);
			saver = int_saver(snd_Sequencer_swp, saver);
			saver = int_saver(snd_Sequencer_reset_cd, saver);

			saver = byte_saver(snd_Sample, saver);

			saver = int_saver(snd_Master_Clock, saver);

			saver = int_saver(snd_Latched_Sample_L, saver);
			saver = int_saver(snd_Latched_Sample_R, saver);
			saver = int_saver(snd_BIAS_Offset, saver);
			saver = int_saver(snd_Sample_Rate, saver);

			saver = int_saver(snd_WAVE_decay_counter, saver);
			saver = bool_saver(snd_WAVE_decay_done, saver);
			saver = bool_saver(snd_Update_Needed, saver);

			return saver;
		}

		uint8_t* snd_LoadState(uint8_t* loader)
		{
			loader = byte_array_loader(snd_Audio_Regs, loader, 21);

			loader = byte_array_loader(snd_Wave_RAM, loader, 16);

			loader = int_loader(&snd_DAC_Offset, loader);
			loader = int_loader(&snd_Wave_Decay_cnt, loader);
			loader = int_loader(&snd_Internal_cnt, loader);
			loader = int_loader(&snd_Divider, loader);

			loader = bool_loader(&snd_Wave_Decay_Done, loader);

			// Audio Variables
			// derived
			loader = bool_loader(&snd_WAVE_DAC_pow, loader);
			loader = bool_loader(&snd_NOISE_wdth_md, loader);
			loader = bool_loader(&snd_SQ1_negate, loader);
			loader = bool_loader(&snd_SQ1_trigger, loader);
			loader = bool_loader(&snd_SQ2_trigger, loader);
			loader = bool_loader(&snd_WAVE_trigger, loader);
			loader = bool_loader(&snd_NOISE_trigger, loader);
			loader = bool_loader(&snd_SQ1_len_en, loader);
			loader = bool_loader(&snd_SQ2_len_en, loader);
			loader = bool_loader(&snd_WAVE_len_en, loader);
			loader = bool_loader(&snd_NOISE_len_en, loader);
			loader = bool_loader(&snd_SQ1_env_add, loader);
			loader = bool_loader(&snd_SQ2_env_add, loader);
			loader = bool_loader(&snd_NOISE_env_add, loader);
			loader = byte_loader(&snd_WAVE_vol_code, loader);
			loader = byte_loader(&snd_NOISE_clk_shft, loader);
			loader = byte_loader(&snd_NOISE_div_code, loader);
			loader = byte_loader(&snd_SQ1_shift, loader);
			loader = byte_loader(&snd_SQ1_duty, loader);
			loader = byte_loader(&snd_SQ2_duty, loader);
			loader = byte_loader(&snd_SQ1_st_vol, loader);
			loader = byte_loader(&snd_SQ2_st_vol, loader);
			loader = byte_loader(&snd_NOISE_st_vol, loader);
			loader = byte_loader(&snd_SQ1_per, loader);
			loader = byte_loader(&snd_SQ2_per, loader);
			loader = byte_loader(&snd_NOISE_per, loader);
			loader = byte_loader(&snd_SQ1_swp_prd, loader);

			loader = int_loader(&snd_SQ1_frq, loader);
			loader = int_loader(&snd_SQ2_frq, loader);
			loader = int_loader(&snd_WAVE_frq, loader);

			loader = short_loader(&snd_SQ1_length, loader);
			loader = short_loader(&snd_SQ2_length, loader);
			loader = short_loader(&snd_WAVE_length, loader);
			loader = short_loader(&snd_NOISE_length, loader);
			// state
			loader = bool_loader(&snd_WAVE_can_get, loader);
			loader = bool_loader(&snd_SQ1_calc_done, loader);
			loader = bool_loader(&snd_SQ1_swp_enable, loader);
			loader = bool_loader(&snd_SQ1_vol_done, loader);
			loader = bool_loader(&snd_SQ2_vol_done, loader);
			loader = bool_loader(&snd_NOISE_vol_done, loader);
			loader = bool_loader(&snd_SQ1_enable, loader);
			loader = bool_loader(&snd_SQ2_enable, loader);
			loader = bool_loader(&snd_WAVE_enable, loader);
			loader = bool_loader(&snd_NOISE_enable, loader);
			loader = byte_loader(&snd_SQ1_vol_state, loader);
			loader = byte_loader(&snd_SQ2_vol_state, loader);
			loader = byte_loader(&snd_NOISE_vol_state, loader);
			loader = byte_loader(&snd_SQ1_duty_cntr, loader);
			loader = byte_loader(&snd_SQ2_duty_cntr, loader);
			loader = byte_loader(&snd_WAVE_wave_cntr, loader);
			loader = int_loader(&snd_SQ1_frq_shadow, loader);
			loader = int_loader(&snd_SQ1_intl_cntr, loader);
			loader = int_loader(&snd_SQ2_intl_cntr, loader);
			loader = int_loader(&snd_WAVE_intl_cntr, loader);
			loader = int_loader(&snd_NOISE_intl_cntr, loader);
			loader = int_loader(&snd_SQ1_vol_per, loader);
			loader = int_loader(&snd_SQ2_vol_per, loader);
			loader = int_loader(&snd_NOISE_vol_per, loader);
			loader = int_loader(&snd_SQ1_intl_swp_cnt, loader);
			loader = int_loader(&snd_NOISE_LFSR, loader);

			loader = short_loader(&snd_SQ1_len_cntr, loader);
			loader = short_loader(&snd_SQ2_len_cntr, loader);
			loader = short_loader(&snd_WAVE_len_cntr, loader);
			loader = short_loader(&snd_NOISE_len_cntr, loader);

			// GB specific registers
			loader = int_loader(&snd_Wave_Bank, loader);
			loader = int_loader(&snd_Wave_Bank_Playing, loader);
			loader = int_loader(&snd_Chan_Mult, loader);
			loader = byte_loader(&snd_CTRL_GB_Low, loader);
			loader = byte_loader(&snd_CTRL_GB_High, loader);
			loader = byte_loader(&snd_Bias_Low, loader);
			loader = byte_loader(&snd_Bias_High, loader);
			loader = bool_loader(&snd_Wave_Size, loader);
			loader = bool_loader(&snd_Wave_Vol_Force, loader);

			// computed
			loader = sint_loader(&snd_SQ1_output, loader);
			loader = sint_loader(&snd_SQ2_output, loader);
			loader = sint_loader(&snd_WAVE_output, loader);
			loader = sint_loader(&snd_NOISE_output, loader);

			// Contol Variables
			loader = bool_loader(&snd_CTRL_sq1_L_en, loader);
			loader = bool_loader(&snd_CTRL_sq2_L_en, loader);
			loader = bool_loader(&snd_CTRL_wave_L_en, loader);
			loader = bool_loader(&snd_CTRL_noise_L_en, loader);
			loader = bool_loader(&snd_CTRL_sq1_R_en, loader);
			loader = bool_loader(&snd_CTRL_sq2_R_en, loader);
			loader = bool_loader(&snd_CTRL_wave_R_en, loader);
			loader = bool_loader(&snd_CTRL_noise_R_en, loader);
			loader = bool_loader(&snd_CTRL_power, loader);
			loader = bool_loader(&snd_CTRL_vin_L_en, loader);
			loader = bool_loader(&snd_CTRL_vin_R_en, loader);

			loader = byte_loader(&snd_CTRL_vol_L, loader);
			loader = byte_loader(&snd_CTRL_vol_R, loader);

			loader = int_loader(&snd_Sequencer_len, loader);
			loader = int_loader(&snd_Sequencer_vol, loader);
			loader = int_loader(&snd_Sequencer_swp, loader);
			loader = int_loader(&snd_Sequencer_reset_cd, loader);

			loader = byte_loader(&snd_Sample, loader);

			loader = int_loader(&snd_Master_Clock, loader);

			loader = sint_loader(&snd_Latched_Sample_L, loader);
			loader = sint_loader(&snd_Latched_Sample_R, loader);
			loader = sint_loader(&snd_BIAS_Offset, loader);
			loader = sint_loader(&snd_Sample_Rate, loader);

			loader = int_loader(&snd_WAVE_decay_counter, loader);
			loader = bool_loader(&snd_WAVE_decay_done, loader);
			loader = bool_loader(&snd_Update_Needed, loader);

			return loader;
		}

	#pragma endregion

	#pragma region State Save / Load for System
		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Is_Lag, saver);
			saver = bool_saver(VBlank_Rise, saver);
			saver = bool_saver(GBC_Compat, saver);

			saver = byte_saver(ext_num, saver);

			saver = byte_saver(GB_bios_register, saver);
			saver = byte_saver(input_register, saver);
			saver = byte_saver(REG_FFFF, saver);
			saver = byte_saver(REG_FF0F, saver);
			saver = byte_saver(REG_FF0F_OLD, saver);

			saver = byte_saver(undoc_6C, saver);
			saver = byte_saver(undoc_72, saver);
			saver = byte_saver(undoc_73, saver);
			saver = byte_saver(undoc_74, saver);
			saver = byte_saver(undoc_75, saver);
			saver = byte_saver(undoc_76, saver);
			saver = byte_saver(undoc_77, saver);

			saver = bool_saver(controller_was_checked, saver);
			saver = bool_saver(delays_to_process, saver);
			saver = bool_saver(DIV_falling_edge, saver);
			saver = bool_saver(DIV_edge_old, saver);
			saver = bool_saver(Double_Speed, saver);
			saver = bool_saver(speed_switch, saver);
			saver = bool_saver(HDMA_Transfer, saver);
			saver = bool_saver(In_Vblank_old, saver);
			saver = bool_saver(Sync_Domains_VBL, saver);

			saver = byte_saver(controller_state, saver);
			saver = byte_saver(multi_core_controller_byte, saver);
			saver = byte_saver(bus_value, saver);
			saver = byte_saver(VRAM_Bank, saver);
			saver = byte_saver(IR_reg, saver);
			saver = byte_saver(IR_mask, saver);
			saver = byte_saver(IR_signal, saver);
			saver = byte_saver(IR_receive, saver);
			saver = byte_saver(IR_self, saver);

			saver = short_saver(Acc_X_state, saver);
			saver = short_saver(Acc_Y_state, saver);
			saver = short_saver(addr_access, saver);

			saver = int_saver(controller_delay_cd, saver);
			saver = int_saver(clear_counter, saver);
			saver = int_saver(IR_write, saver);
			saver = int_saver(RAM_Bank, saver);
			saver = int_saver(RAM_Bank_ret, saver);

			saver = long_saver(bus_access_time, saver);
			saver = long_saver(Cycle_Count, saver);
			saver = long_saver(Frame_Cycle, saver);

			saver = byte_array_saver(RAM, saver, 0x8000);
			saver = byte_array_saver(ZP_RAM, saver, 0x80);
			saver = byte_array_saver(PALRAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x4000);
			saver = byte_array_saver(OAM, saver, 0xA0);

			saver = byte_array_saver(RAM_vbls, saver, 0x8000);
			saver = byte_array_saver(ZP_RAM_vbls, saver, 0x80);
			saver = byte_array_saver(PALRAM_vbls, saver, 0x400);
			saver = byte_array_saver(VRAM_vbls, saver, 0x4000);
			saver = byte_array_saver(OAM_vbls, saver, 0xA0);

			saver = int_array_saver(color_palette, saver, 4);

			if (Cart_RAM_Length != 0)
			{
				saver = byte_array_saver(Cart_RAM, saver, Cart_RAM_Length);
			}

			saver = snd_SaveState(saver);
			saver = ser_SaveState(saver);
			saver = tim_SaveState(saver);
			saver = cpu_SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Is_Lag, loader);
			loader = bool_loader(&VBlank_Rise, loader);
			loader = bool_loader(&GBC_Compat, loader);

			loader = byte_loader(&ext_num, loader);

			loader = byte_loader(&GB_bios_register, loader);
			loader = byte_loader(&input_register, loader);
			loader = byte_loader(&REG_FFFF, loader);
			loader = byte_loader(&REG_FF0F, loader);
			loader = byte_loader(&REG_FF0F_OLD, loader);

			loader = byte_loader(&undoc_6C, loader);
			loader = byte_loader(&undoc_72, loader);
			loader = byte_loader(&undoc_73, loader);
			loader = byte_loader(&undoc_74, loader);
			loader = byte_loader(&undoc_75, loader);
			loader = byte_loader(&undoc_76, loader);
			loader = byte_loader(&undoc_77, loader);

			loader = bool_loader(&controller_was_checked, loader);
			loader = bool_loader(&delays_to_process, loader);
			loader = bool_loader(&DIV_falling_edge, loader);
			loader = bool_loader(&DIV_edge_old, loader);
			loader = bool_loader(&Double_Speed, loader);
			loader = bool_loader(&speed_switch, loader);
			loader = bool_loader(&HDMA_Transfer, loader);
			loader = bool_loader(&In_Vblank_old, loader);
			loader = bool_loader(&Sync_Domains_VBL, loader);

			loader = byte_loader(&controller_state, loader);
			loader = byte_loader(&multi_core_controller_byte, loader);
			loader = byte_loader(&bus_value, loader);
			loader = byte_loader(&VRAM_Bank, loader);
			loader = byte_loader(&IR_reg, loader);
			loader = byte_loader(&IR_mask, loader);
			loader = byte_loader(&IR_signal, loader);
			loader = byte_loader(&IR_receive, loader);
			loader = byte_loader(&IR_self, loader);

			loader = short_loader(&Acc_X_state, loader);
			loader = short_loader(&Acc_Y_state, loader);
			loader = short_loader(&addr_access, loader);

			loader = int_loader(&controller_delay_cd, loader);
			loader = int_loader(&clear_counter, loader);
			loader = int_loader(&IR_write, loader);
			loader = int_loader(&RAM_Bank, loader);
			loader = int_loader(&RAM_Bank_ret, loader);

			loader = long_loader(&bus_access_time, loader);
			loader = long_loader(&Cycle_Count, loader);
			loader = long_loader(&Frame_Cycle, loader);
			
			loader = byte_array_loader(RAM, loader, 0x8000);
			loader = byte_array_loader(ZP_RAM, loader, 0x80);
			loader = byte_array_loader(PALRAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x4000);
			loader = byte_array_loader(OAM, loader, 0xA0);

			loader = byte_array_loader(RAM_vbls, loader, 0x8000);
			loader = byte_array_loader(ZP_RAM_vbls, loader, 0x80);
			loader = byte_array_loader(PALRAM_vbls, loader, 0x400);
			loader = byte_array_loader(VRAM_vbls, loader, 0x4000);
			loader = byte_array_loader(OAM_vbls, loader, 0xA0);

			loader = int_array_loader(color_palette, loader, 4);

			if (Cart_RAM_Length != 0)
			{	
				loader = byte_array_loader(Cart_RAM, loader, Cart_RAM_Length);
			}

			loader = snd_LoadState(loader);
			loader = ser_LoadState(loader);
			loader = tim_LoadState(loader);
			loader = cpu_LoadState(loader);

			return loader;
		}

	#pragma endregion

	#pragma region State Save / Load Functions
		uint8_t* bool_saver(bool to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save ? 1 : 0); saver++;

			return saver;
		}

		uint8_t* byte_saver(uint8_t to_save, uint8_t* saver)
		{
			*saver = to_save; saver++;

			return saver;
		}

		uint8_t* short_saver(uint16_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;

			return saver;
		}

		uint8_t* int_saver(uint32_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;

			return saver;
		}

		uint8_t* long_saver(uint64_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 40) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 56) & 0xFF); saver++;

			return saver;
		}

		uint8_t* bool_array_saver(bool* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++) { *saver = (uint8_t)(to_save[i] ? 1 : 0); saver++; }

			return saver;
		}

		uint8_t* byte_array_saver(uint8_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++) { *saver = to_save[i]; saver++; }

			return saver;
		}

		uint8_t* short_array_saver(uint16_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++)
			{ 
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* int_array_saver(uint32_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++)
			{
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* long_array_saver(uint64_t* to_save, uint8_t* saver, int length)
		{
			for (int i = 0; i < length; i++)
			{
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 40) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 56) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* bool_loader(bool* to_load, uint8_t* loader)
		{
			to_load[0] = *loader == 1; loader++;

			return loader;
		}

		uint8_t* byte_loader(uint8_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;

			return loader;
		}

		uint8_t* short_loader(uint16_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;
			to_load[0] |= ((uint16_t)(*loader) << 8); loader++;

			return loader;
		}

		uint8_t* int_loader(uint32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= ((uint32_t)(*loader) << 8); loader++;
			to_load[0] |= ((uint32_t)(*loader) << 16); loader++; to_load[0] |= ((uint32_t)(*loader) << 24); loader++;

			return loader;
		}

		uint8_t* sint_loader(int32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= ((int32_t)(*loader) << 8); loader++;
			to_load[0] |= ((int32_t)(*loader) << 16); loader++; to_load[0] |= ((int32_t)(*loader) << 24); loader++;

			return loader;
		}

		uint8_t* long_loader(uint64_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (uint64_t)(* loader) << 8; loader++;
			to_load[0] |= (uint64_t)(*loader) << 16; loader++; to_load[0] |= (uint64_t)(*loader) << 24; loader++;
			to_load[0] |= (uint64_t)(*loader) << 32; loader++; to_load[0] |= (uint64_t)(*loader) << 40; loader++;
			to_load[0] |= (uint64_t)(*loader) << 48; loader++; to_load[0] |= (uint64_t)(*loader) << 56; loader++;

			return loader;
		}

		uint8_t* slong_loader(int64_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (int64_t)(*loader) << 8; loader++;
			to_load[0] |= (int64_t)(*loader) << 16; loader++; to_load[0] |= (int64_t)(*loader) << 24; loader++;
			to_load[0] |= (int64_t)(*loader) << 32; loader++; to_load[0] |= (int64_t)(*loader) << 40; loader++;
			to_load[0] |= (int64_t)(*loader) << 48; loader++; to_load[0] |= (int64_t)(*loader) << 56; loader++;

			return loader;
		}

		uint8_t* bool_array_loader(bool* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++) { to_load[i] = *loader == 1; loader++; }

			return loader;
		}

		uint8_t* byte_array_loader(uint8_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++) { to_load[i] = *loader; loader++; }

			return loader;
		}

		uint8_t* short_array_loader(uint16_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= ((uint16_t)(*loader) << 8); loader++;
			}

			return loader;
		}

		uint8_t* int_array_loader(uint32_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= ((uint32_t)(*loader) << 8); loader++;
				to_load[i] |= ((uint32_t)(*loader) << 16); loader++; to_load[i] |= ((uint32_t)(*loader) << 24); loader++;
			}

			return loader;
		}

		uint8_t* long_array_loader(uint64_t* to_load, uint8_t* loader, int length)
		{
			for (int i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= (uint64_t)(*loader) << 8; loader++;
				to_load[i] |= (uint64_t)(*loader) << 16; loader++; to_load[i] |= (uint64_t)(*loader) << 24; loader++;
				to_load[i] |= (uint64_t)(*loader) << 32; loader++; to_load[i] |= (uint64_t)(*loader) << 40; loader++;
				to_load[i] |= (uint64_t)(*loader) << 48; loader++; to_load[i] |= (uint64_t)(*loader) << 56; loader++;
			}

			return loader;
		}

	#pragma endregion
	};
}

#endif