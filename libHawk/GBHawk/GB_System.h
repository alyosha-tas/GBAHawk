#ifndef GB_System_H
#define GB_System_H

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

//Message_String = "Complete: " + to_string(ser_GBP_Transfer_Count) + " " + to_string(ser_Data_0 & 0xFF) + " " + to_string(CycleCount);

//MessageCallback(Message_String.length());

namespace GBHawk
{
	class MemoryManager;
	class Mappers;

	class GB_System
	{
	public:
		
		Mappers* mapper_pntr = nullptr;

		uint8_t* Cart_RAM = nullptr;
		uint32_t Cart_RAM_Length = 0;
		string Message_String = "";

		void (*MessageCallback)(int);

	# pragma region General System and Prefetch

		uint32_t video_buffer[160 * 144] = { };

		void Frame_Advance();
		bool SubFrame_Advance(uint32_t reset_cycle);
		inline void Single_Step();

		uint8_t Read_Memory(uint16_t addr);

		void Write_Memory(uint16_t addr, uint8_t value);

		void Read_Memory_16_DMA(uint32_t addr, uint32_t chan);
		void Read_Memory_32_DMA(uint32_t addr, uint32_t chan);

		void Write_Memory_16_DMA(uint32_t addr, uint16_t value, uint32_t chan);
		void Write_Memory_32_DMA(uint32_t addr, uint32_t value, uint32_t chan);

		uint8_t Peek_Memory(uint16_t addr);

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
		bool double_speed;
		bool speed_switch;
		bool HDMA_transfer; // stalls CPU when in progress

		uint8_t bus_value; // we need the last value on the bus for proper emulation of blocked SRAM
		uint8_t VRAM_Bank;
		uint8_t IR_reg, IR_mask, IR_signal, IR_receive, IR_self;
		uint8_t controller_state;
		uint8_t multi_core_controller_byte;

		uint16_t addr_access;
		uint16_t Acc_X_state;
		uint16_t Acc_Y_state;

		uint32_t IR_write;
		uint32_t RAM_Bank;
		uint32_t RAM_Bank_ret;
		uint32_t controller_delay_cd;
		uint32_t cpu_state_hold;
		uint32_t clear_counter;

		uint64_t bus_access_time; // also need to keep track of the time of the access since it doesn't last very long

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
		uint8_t BIOS[0x4000] = { };
		uint8_t ROM[0x6000000] = { };

		bool is_linked_system = false;
		bool Is_GBC = false;
		bool Is_GBC_GBA = false;

		// Most memory accesses from the cpu / dma are force aligned to word / half wod boundaries, 
		// so use some pointers of the appropriate memory size to simplify accesses
		uint32_t* ROM_32 = (uint32_t*)(&ROM[0]);
		uint16_t* ROM_16 = (uint16_t*)(&ROM[0]);

		uint32_t* BIOS_32 = (uint32_t*)(&BIOS[0]);
		uint16_t* BIOS_16 = (uint16_t*)(&BIOS[0]);

		uint32_t* WRAM_32 = (uint32_t*)(&WRAM[0]);
		uint16_t* WRAM_16 = (uint16_t*)(&WRAM[0]);

		uint32_t* IWRAM_32 = (uint32_t*)(&IWRAM[0]);
		uint16_t* IWRAM_16 = (uint16_t*)(&IWRAM[0]);

		uint32_t* PALRAM_32 = (uint32_t*)(&PALRAM[0]);
		uint16_t* PALRAM_16 = (uint16_t*)(&PALRAM[0]);

		uint32_t* VRAM_32 = (uint32_t*)(&VRAM[0]);
		uint16_t* VRAM_16 = (uint16_t*)(&VRAM[0]);

		uint32_t* OAM_32 = (uint32_t*)(&OAM[0]);
		uint16_t* OAM_16 = (uint16_t*)(&OAM[0]);

		// only used by ppu viewer
		uint8_t PPU_IO[0x60] = { };

		uint16_t New_Controller;
		uint16_t New_Acc_X, New_Acc_Y;

		uint8_t New_Solar;

		GB_System()
		{
			System_Reset();
		}

		void System_Reset() 
		{
			controller_was_checked = false;
			DIV_falling_edge = DIV_edge_old = false;
			speed_switch = false;
			HDMA_transfer = false;

			bus_value = 0;
			IR_reg = IR_mask = IR_signal = IR_receive = IR_self = 0;
			controller_state = 0;
			multi_core_controller_byte = 0;

			addr_access = 0;
			Acc_X_state = 0;
			Acc_Y_state = 0;

			IR_write = 0;
			cpu_state_hold = 0;

			GB_bios_register = 0; // bios enable
			GBC_Compat = true;
			double_speed = false;
			VRAM_Bank = 0;
			RAM_Bank = 1; // RAM bank always starts as 1 (even writing zero still sets 1)
			RAM_Bank_ret = 0; // return value can still be zero even though the bank itself cannot be
			delays_to_process = false;
			controller_delay_cd = 0;
			clear_counter = 0;

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
			ppu_Reset();
			dma_Reset();
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

		void do_controller_check(bool from_reg)
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
				if ((REG_FFFF& 0x10) == 0x10) { cpu_FlagIset(true); }
				REG_FF0F |= 0x10;
			}
		}

		void HDMA_start_stop(bool hdma_start)
		{
			// put the cpu into a wait state when HDMA starts
			// restore it when HDMA ends
			HDMA_transfer = hdma_start;

			if (hdma_start)
			{
				cpu_state_hold = cpu.instr_pntr;
				cpu.instr_pntr = 256 * 60 * 2 + 60 * 8;
			}
			else
			{
				cpu.instr_pntr = cpu_state_hold;
			}
		}

		void process_delays()
		{
			// triggering an interrupt with a write to the control register takes 4 cycles to trigger interrupt
			controller_delay_cd--;
			if (controller_delay_cd == 0)
			{
				if ((REG_FFFF & 0x10) == 0x10) { cpu_FlagIset(true); }
				REG_FF0F |= 0x10;
				delays_to_process = false;
			}
		}

		int SpeedFunc(int temp)
		{
			if (temp == 0)
			{
				if (speed_switch)
				{
					speed_switch = false;

					Message_String = "Speed Switch: " + to_string(CycleCount);

					MessageCallback(Message_String.length());

					int ret = double_speed ? 32770 : 32770; // actual time needs checking
					return ret;
				}

				// if we are not switching speed, return 0
				return 0;
			}
			else if (temp == 1)
			{
				// reset the divider (only way for speed_change_timing_fine.gbc and speed_change_cancel.gbc to both work)
				// Console.WriteLine("at stop " + timer.divider_reg + " " + timer.timer_control);

				// only if the timer mode is 1, an extra tick of the timer is counted before the reset
				// this varies between console revisions
				if ((tim_Control & 7) == 5)
				{
					if ((tim_Divider_Reg & 0x7) == 7)
					{
						tim_Old_State = true;
					}
				}

				tim_Divider_Reg = 0xFFFF;

				double_speed = !double_speed;

				ppu.LYC_offset = double_speed ? 1 : 2;

				ppu.LY_153_change = double_speed ? 8 : 10;

				return 0;
			}
			else
			{

				return 0;
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

		void clear_screen_func()
		{
			for (int j = 0; j < frame_buffer.Length; j++) { frame_buffer[j] = (int)(frame_buffer[j] | (0x30303 << (clear_counter * 2))); }

			clear_counter++;
			if (clear_counter == 4)
			{
				ppu.clear_screen = false;
			}
		}

		void SetIntRegs(uint8_t r)
		{
			// For timer interrupts or serial interrupts that occur on the same cycle as the IRQ clear
			// the clear wins on GB and GBA (tested on GBP.) Assuming true for GBC E too.
			// but only in single speed
			if (((REG_FF0F & 4) == 4) && ((r & 4) == 0) && tim_IRQ_Block && !double_speed) { r |= 4; }
			if (((REG_FF0F & 8) == 8) && ((r & 8) == 0) && ser_IRQ_Block && !double_speed) { r |= 8; }
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
					ret = audio.ReadReg(addr);
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
					ret = ppu.ReadReg(addr);
					break;

					// Speed Control for GBC
				case 0xFF4D:
					if (GBC_Compat)
					{
						ret = (uint8_t)(((double_speed ? 1 : 0) << 7) | (speed_switch ? 1 : 0) | 0x7E);
					}
					else
					{
						ret = 0xFF;
					}
					break;

				case 0xFF4F: // VBK
					if (is_GBC)
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
						ret = ppu.ReadReg(addr);
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
					if (is_GBC)
					{
						ret = ppu.ReadReg(addr);
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
					uint8_t ret1 = audio.SQ1_output >= Audio.DAC_OFST
						? (uint8_t)(audio.SQ1_output - Audio.DAC_OFST)
						: (uint8_t)0;
					uint8_t ret2 = audio.SQ2_output >= Audio.DAC_OFST
						? (uint8_t)(audio.SQ2_output - Audio.DAC_OFST)
						: (uint8_t)0;
					ret = (uint8_t)(ret1 | (ret2 << 4));
					break;

				case 0xFF77:
					uint8_t retN = audio.NOISE_output >= Audio.DAC_OFST
						? (uint8_t)(audio.NOISE_output - Audio.DAC_OFST)
						: (uint8_t)0;
					uint8_t retW = audio.WAVE_output >= Audio.DAC_OFST
						? (uint8_t)(audio.WAVE_output - Audio.DAC_OFST)
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
			switch (addr)
			{
				// select input
				case 0xFF00:
					input_register &= 0xCF;
					input_register |= (uint8_t)(value & 0x30); // top 2 bits always 1

					// check for high to low transitions that trigger IRQs
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
							cpu_FlagIset(true);
						}
					}

					// if no bits are in common between flags and enables, de-assert the IRQ
					if (((REG_FF0F & 0x1F) & REG_FFFF) == 0) { cpu_FlagIset(false); }
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
					audio.WriteReg(addr, value);
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
					ppu.WriteReg(addr, value);
					break;

					// GBC compatibility register (I think)
				case 0xFF4C:
					if ((value != 0xC0) && (value != 0x80) && (GB_bios_register == 0))// && (value != 0xFF) && (value != 0x04))
					{
						GBC_Compat = false;
					}
					Console.Write("GBC Compatibility? ");
					Console.WriteLine(value);
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
						if (!GBC_Compat) { ppu.pal_change_blocked = true; RAM_Bank = 1; RAM_Bank_ret = 0; }
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
						ppu.WriteReg(addr, value);
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
						ppu.WriteReg(addr, value);
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
							cpu_FlagIset(true);
						}
					}

					// if no bits are in common between flags and enables, de-assert the IRQ
					if (((REG_FF0F & 0x1F) & REG_FFFF) == 0) { cpu_FlagIset(false); }
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

		uint16_t cpu_Opcode, cpu_Instr_Type, cpu_Instr_Cycle;

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

		// Instruction types
		enum class OpT
		{
			NOP_,
			LD_IND_16,
			LD_8_IND,
			INC_16,
			INT_OP,
			LD_IND_8_INC,
			LD_R_IM,
			ADD_16,
			REG_OP_IND,
			DEC_16,
			INT_OP,
			STOP,

		};

		OpT cpu_Instr_Type;

		OpT cpu_Instr_Type_List[256] =
		{
			/*0x00 */ OpT::NOP_,							// NOP
			/*0x01 */ OpT::LD_IND_16,						// LD BC, nn
			/*0x02 */ OpT::LD_8_IND,						// LD (BC), A
			/*0x03 */ OpT::INC_16,							// INC BC
			/*0x04 */ OpT::INT_OP,							// INC B
			/*0x05 */ OpT::INT_OP,							// DEC B
			/*0x06 */ OpT::LD_IND_8_INC,					// LD B, n
			/*0x07 */ OpT::INT_OP,							// RLCA
			/*0x08 */ OpT::LD_R_IM,							// LD (imm), SP
			/*0x09 */ OpT::ADD_16,							// ADD HL, BC
			/*0x0A */ OpT::REG_OP_IND,						// LD A, (BC)
			/*0x0B */ OpT::DEC_16,							// DEC BC
			/*0x0C */ OpT::INT_OP,							// INC C
			/*0x0D */ OpT::INT_OP,							// DEC C
			/*0x0E */ OpT::LD_IND_8_INC,					// LD C, n
			/*0x0F */ OpT::INT_OP,							// RRCA
			/*0x10 */ OpT::STOP,							// STOP
			/*0x11 */ OpT::LD_IND_16,						// LD DE, nn
			/*0x12 */ OpT::LD_8_IND,						// LD (DE), A
			/*0x13 */ OpT::INC_16,							// INC DE
			/*0x14 */ OpT::INT_OP,							// INC D
			/*0x15 */ OpT::INT_OP,							// DEC D
			/*0x16 */ OpT::LD_IND_8_INC,						// LD D, n
			/*0x17 */ OpT::INT_OP,							// RLA
			/*0x18 */ OpT::JR_COND							// JR, r8
			/*0x19 */ OpT::ADD_16							// ADD HL, DE
			/*0x1A */ OpT::REG_OP,_IND						// LD A, (DE)
			/*0x1B */ OpT::DEC_16							// DEC DE
			/*0x1C */ OpT::INT_OP,							// INC E
			/*0x1D */ OpT::INT_OP,							// DEC E
			/*0x1E */ OpT::LD_IND_8_INC,						// LD E, n
			/*0x1F */ OpT::INT_OP,							// RRA
			/*0x20 */ OpT::JR_COND							// JR NZ, r8
			/*0x21 */ OpT::LD_IND_16						// LD HL, nn
			/*0x22 */ OpT::LD_8_IND_INC,						// LD (HL+), A
			/*0x23 */ OpT::INC_16							// INC HL
			/*0x24 */ OpT::INT_OP,							// INC H
			/*0x25 */ OpT::INT_OP,							// DEC H
			/*0x26 */ OpT::LD_IND_8_INC,						// LD H, n
			/*0x27 */ OpT::INT_OP,							// DAA
			/*0x28 */ OpT::JR_COND							// JR Z, r8
			/*0x29 */ OpT::ADD_16							// ADD HL, HL
			/*0x2A */ OpT::LD_IND_8_INC,_HL					// LD A, (HL+)
			/*0x2B */ OpT::DEC_16							// DEC HL
			/*0x2C */ OpT::INT_OP,							// INC L
			/*0x2D */ OpT::INT_OP,							// DEC L
			/*0x2E */ OpT::LD_IND_8_INC,						// LD L, n
			/*0x2F */ OpT::INT_OP,							// CPL
			/*0x30 */ OpT::JR_COND							// JR NC, r8
			/*0x31 */ OpT::LD_IND_16						// LD SP, nn
			/*0x32 */ OpT::LD_8_IND_DEC						// LD (HL-), A
			/*0x33 */ OpT::INC_16							// INC SP
			/*0x34 */ OpT::INC_8_IND						// INC (HL)
			/*0x35 */ OpT::DEC_8_IND						// DEC (HL)
			/*0x36 */ OpT::LD_8_IND_IND						// LD (HL), n
			/*0x37 */ OpT::INT_OP,							// SCF
			/*0x38 */ OpT::JR_COND							// JR C, r8
			/*0x39 */ OpT::ADD_16							// ADD HL, SP
			/*0x3A */ OpT::LD_IND_8_DEC_HL					// LD A, (HL-)
			/*0x3B */ OpT::DEC_16							// DEC SP
			/*0x3C */ OpT::INT_OP,							// INC A
			/*0x3D */ OpT::INT_OP,							// DEC A
			/*0x3E */ OpT::LD_IND_8_INC,						// LD A, n
			/*0x3F */ OpT::INT_OP,							// CCF
			/*0x40 */ OpT::REG_OP,							// LD B, B
			/*0x41 */ OpT::REG_OP,							// LD B, C
			/*0x42 */ OpT::REG_OP,							// LD B, D
			/*0x43 */ OpT::REG_OP,							// LD B, E
			/*0x44 */ OpT::REG_OP,							// LD B, H
			/*0x45 */ OpT::REG_OP,							// LD B, L
			/*0x46 */ OpT::REG_OP,_IND						// LD B, (HL)
			/*0x47 */ OpT::REG_OP,							// LD B, A
			/*0x48 */ OpT::REG_OP,							// LD C, B
			/*0x49 */ OpT::REG_OP,							// LD C, C
			/*0x4A */ OpT::REG_OP,							// LD C, D
			/*0x4B */ OpT::REG_OP,							// LD C, E
			/*0x4C */ OpT::REG_OP,							// LD C, H
			/*0x4D */ OpT::REG_OP,							// LD C, L
			/*0x4E */ OpT::REG_OP,_IND						// LD C, (HL)
			/*0x4F */ OpT::REG_OP,							// LD C, A
			/*0x50 */ OpT::REG_OP,							// LD D, B
			/*0x51 */ OpT::REG_OP,							// LD D, C
			/*0x52 */ OpT::REG_OP,							// LD D, D
			/*0x53 */ OpT::REG_OP,							// LD D, E
			/*0x54 */ OpT::REG_OP,							// LD D, H
			/*0x55 */ OpT::REG_OP,							// LD D, L
			/*0x56 */ OpT::REG_OP,_IND						// LD D, (HL)
			/*0x57 */ OpT::REG_OP,							// LD D, A
			/*0x58 */ OpT::REG_OP,							// LD E, B
			/*0x59 */ OpT::REG_OP,							// LD E, C
			/*0x5A */ OpT::REG_OP,							// LD E, D
			/*0x5B */ OpT::REG_OP,							// LD E, E
			/*0x5C */ OpT::REG_OP,							// LD E, H
			/*0x5D */ OpT::REG_OP,							// LD E, L
			/*0x5E */ OpT::REG_OP,_IND						// LD E, (HL)
			/*0x5F */ OpT::REG_OP,							// LD E, A
			/*0x60 */ OpT::REG_OP,							// LD H, B
			/*0x61 */ OpT::REG_OP,							// LD H, C
			/*0x62 */ OpT::REG_OP,							// LD H, D
			/*0x63 */ OpT::REG_OP,							// LD H, E
			/*0x64 */ OpT::REG_OP,							// LD H, H
			/*0x65 */ OpT::REG_OP,							// LD H, L
			/*0x66 */ OpT::REG_OP,_IND						// LD H, (HL)
			/*0x67 */ OpT::REG_OP,							// LD H, A
			/*0x68 */ OpT::REG_OP,							// LD L, B
			/*0x69 */ OpT::REG_OP,							// LD L, C
			/*0x6A */ OpT::REG_OP,							// LD L, D
			/*0x6B */ OpT::REG_OP,							// LD L, E
			/*0x6C */ OpT::REG_OP,							// LD L, H
			/*0x6D */ OpT::REG_OP,							// LD L, L
			/*0x6E */ OpT::REG_OP,_IND						// LD L, (HL)
			/*0x6F */ OpT::REG_OP,							// LD L, A
			/*0x70 */ OpT::LD_8_IND							// LD (HL), B
			/*0x71 */ OpT::LD_8_IND							// LD (HL), C
			/*0x72 */ OpT::LD_8_IND							// LD (HL), D
			/*0x73 */ OpT::LD_8_IND							// LD (HL), E
			/*0x74 */ OpT::LD_8_IND							// LD (HL), H
			/*0x75 */ OpT::LD_8_IND							// LD (HL), L
			/*0x76 */ OpT::HALT_							// HALT
			/*0x77 */ OpT::LD_8_IND							// LD (HL), A
			/*0x78 */ OpT::REG_OP,							// LD A, B
			/*0x79 */ OpT::REG_OP,							// LD A, C
			/*0x7A */ OpT::REG_OP,							// LD A, D
			/*0x7B */ OpT::REG_OP,							// LD A, E
			/*0x7C */ OpT::REG_OP,							// LD A, H
			/*0x7D */ OpT::REG_OP,							// LD A, L
			/*0x7E */ OpT::REG_OP,_IND						// LD A, (HL)
			/*0x7F */ OpT::REG_OP,							// LD A, A
			/*0x80 */ OpT::REG_OP,							// ADD A, B
			/*0x81 */ OpT::REG_OP,							// ADD A, C
			/*0x82 */ OpT::REG_OP,							// ADD A, D
			/*0x83 */ OpT::REG_OP,							// ADD A, E
			/*0x84 */ OpT::REG_OP,							// ADD A, H
			/*0x85 */ OpT::REG_OP,							// ADD A, L
			/*0x86 */ OpT::REG_OP,_IND						// ADD A, (HL)
			/*0x87 */ OpT::REG_OP,							// ADD A, A
			/*0x88 */ OpT::REG_OP,							// ADC A, B
			/*0x89 */ OpT::REG_OP,							// ADC A, C
			/*0x8A */ OpT::REG_OP,							// ADC A, D
			/*0x8B */ OpT::REG_OP,							// ADC A, E
			/*0x8C */ OpT::REG_OP,							// ADC A, H
			/*0x8D */ OpT::REG_OP,							// ADC A, L
			/*0x8E */ OpT::REG_OP,_IND						// ADC A, (HL)
			/*0x8F */ OpT::REG_OP,							// ADC A, A
			/*0x90 */ OpT::REG_OP,							// SUB A, B
			/*0x91 */ OpT::REG_OP,							// SUB A, C
			/*0x92 */ OpT::REG_OP,							// SUB A, D
			/*0x93 */ OpT::REG_OP,							// SUB A, E
			/*0x94 */ OpT::REG_OP,							// SUB A, H
			/*0x95 */ OpT::REG_OP,							// SUB A, L
			/*0x96 */ OpT::REG_OP,_IND						// SUB A, (HL)
			/*0x97 */ OpT::REG_OP,							// SUB A, A
			/*0x98 */ OpT::REG_OP,							// SBC A, B
			/*0x99 */ OpT::REG_OP,							// SBC A, C
			/*0x9A */ OpT::REG_OP,							// SBC A, D
			/*0x9B */ OpT::REG_OP,							// SBC A, E
			/*0x9C */ OpT::REG_OP,							// SBC A, H
			/*0x9D */ OpT::REG_OP,							// SBC A, L
			/*0x9E */ OpT::REG_OP,_IND						// SBC A, (HL)
			/*0x9F */ OpT::REG_OP,							// SBC A, A
			/*0xA0 */ OpT::REG_OP,							// AND A, B
			/*0xA1 */ OpT::REG_OP,							// AND A, C
			/*0xA2 */ OpT::REG_OP,							// AND A, D
			/*0xA3 */ OpT::REG_OP,							// AND A, E
			/*0xA4 */ OpT::REG_OP,							// AND A, H
			/*0xA5 */ OpT::REG_OP,							// AND A, L
			/*0xA6 */ OpT::REG_OP,_IND						// AND A, (HL)
			/*0xA7 */ OpT::REG_OP,							// AND A, A
			/*0xA8 */ OpT::REG_OP,							// XOR A, B
			/*0xA9 */ OpT::REG_OP,							// XOR A, C
			/*0xAA */ OpT::REG_OP,							// XOR A, D
			/*0xAB */ OpT::REG_OP,							// XOR A, E
			/*0xAC */ OpT::REG_OP,							// XOR A, H
			/*0xAD */ OpT::REG_OP,							// XOR A, L
			/*0xAE */ OpT::REG_OP,_IND						// XOR A, (HL)
			/*0xAF */ OpT::REG_OP,							// XOR A, A
			/*0xB0 */ OpT::REG_OP,							// OR A, B
			/*0xB1 */ OpT::REG_OP,							// OR A, C
			/*0xB2 */ OpT::REG_OP,							// OR A, D
			/*0xB3 */ OpT::REG_OP,							// OR A, E
			/*0xB4 */ OpT::REG_OP,							// OR A, H
			/*0xB5 */ OpT::REG_OP,							// OR A, L
			/*0xB6 */ OpT::REG_OP,_IND						// OR A, (HL)
			/*0xB7 */ OpT::REG_OP,							// OR A, A
			/*0xB8 */ OpT::REG_OP,							// CP A, B
			/*0xB9 */ OpT::REG_OP,							// CP A, C
			/*0xBA */ OpT::REG_OP,							// CP A, D
			/*0xBB */ OpT::REG_OP,							// CP A, E
			/*0xBC */ OpT::REG_OP,							// CP A, H
			/*0xBD */ OpT::REG_OP,							// CP A, L
			/*0xBE */ OpT::REG_OP,_IND						// CP A, (HL)
			/*0xBF */ OpT::REG_OP,							// CP A, A
			/*0xC0 */ OpT::RET_COND							// Ret NZ
			/*0xC1 */ OpT::POP_								// POP BC
			/*0xC2 */ OpT::JP_COND							// JP NZ
			/*0xC3 */ OpT::JP_COND							// JP
			/*0xC4 */ OpT::CALL_COND						// CALL NZ
			/*0xC5 */ OpT::PUSH_							// PUSH BC
			/*0xC6 */ OpT::REG_OP,_IND_INC,					// ADD A, n
			/*0xC7 */ OpT::RST_								// RST 0
			/*0xC8 */ OpT::RET_COND							// RET Z
			/*0xC9 */ OpT::RET_								// RET
			/*0xCA */ OpT::JP_COND							// JP Z
			/*0xCB */ OpT::PREFIX_							// PREFIX
			/*0xCC */ OpT::CALL_COND						// CALL Z
			/*0xCD */ OpT::CALL_COND						// CALL
			/*0xCE */ OpT::REG_OP,_IND_INC,					// ADC A, n
			/*0xCF */ OpT::RST_								// RST 0x08
			/*0xD0 */ OpT::RET_COND							// Ret NC
			/*0xD1 */ OpT::POP_								// POP DE
			/*0xD2 */ OpT::JP_COND							// JP NC
			/*0xD3 */ OpT::JAM_								// JAM
			/*0xD4 */ OpT::CALL_COND						// CALL NC
			/*0xD5 */ OpT::PUSH_							// PUSH DE
			/*0xD6 */ OpT::REG_OP,_IND_INC,					// SUB A, n
			/*0xD7 */ OpT::RST_								// RST 0x10
			/*0xD8 */ OpT::RET_COND							// RET C
			/*0xD9 */ OpT::RETI_							// RETI
			/*0xDA */ OpT::JP_COND							// JP C
			/*0xDB */ OpT::JAM_								// JAM
			/*0xDC */ OpT::CALL_COND						// CALL C
			/*0xDD */ OpT::JAM_								// JAM
			/*0xDE */ OpT::REG_OP,_IND_INC,					// SBC A, n
			/*0xDF */ OpT::RST_								// RST 0x18
			/*0xE0 */ OpT::LD_FF_IND_8						// LD(n), A
			/*0xE1 */ OpT::POP_								// POP HL
			/*0xE2 */ OpT::LD_FFC_IND_8						// LD(C), A
			/*0xE3 */ OpT::JAM_								// JAM
			/*0xE4 */ OpT::JAM_								// JAM
			/*0xE5 */ OpT::PUSH_							// PUSH HL
			/*0xE6 */ OpT::REG_OP,_IND_INC,					// AND A, n
			/*0xE7 */ OpT::RST_								// RST 0x20
			/*0xE8 */ OpT::ADD_SP							// ADD SP,n
			/*0xE9 */ OpT::JP_HL							// JP (HL)
			/*0xEA */ OpT::LD_FF_IND_16						// LD(nn), A
			/*0xEB */ OpT::JAM_								// JAM
			/*0xEC */ OpT::JAM_								// JAM
			/*0xED */ OpT::JAM_								// JAM
			/*0xEE */ OpT::REG_OP,_IND_INC,					// XOR A, n
			/*0xEF */ OpT::RST_								// RST 0x28
			/*0xF0 */ OpT::LD_8_IND_FF						// A, LD(n)
			/*0xF1 */ OpT::POP_								// POP AF
			/*0xF2 */ OpT::LD_8_IND_FFC						// A, LD(C)
			/*0xF3 */ OpT::DI_								// DI
			/*0xF4 */ OpT::JAM_								// JAM
			/*0xF5 */ OpT::PUSH_							// PUSH AF
			/*0xF6 */ OpT::REG_OP,_IND_INC,					// OR A, n
			/*0xF7 */ OpT::RST_								// RST 0x30
			/*0xF8 */ OpT::LD_HL_SPn						// LD HL, SP+n
			/*0xF9 */ OpT::LD_SP_HL							// LD, SP, HL
			/*0xFA */ OpT::LD_16_IND_FF						// A, LD(nn)
			/*0xFB */ OpT::EI_								// EI
			/*0xFC */ OpT::JAM_								// JAM
			/*0xFD */ OpT::JAM_								// JAM
			/*0xFE */ OpT::REG_OP,_IND_INC,					// CP A, n
			/*0xFF */ OpT::RST_								// RST 0x38
		};


		#pragma endregion

		#pragma region ARM7_TDMI functions

		void cpu_Reset()
		{
			cpu_was_FlagI = cpu_FlagI = false;

			cpu_Opcode = 0;
			cpu_Instr_Cycle = 0;
			
			for (int i = 0; i < 14; i++)
			{
				cpu_Regs[i] = 0;
			}
		}

		inline void cpu_Halt_Check()
		{

		}

		inline void cpu_Op_Func()
		{

		}

		inline void cpu_Execute_One();



		void Read_Func(uint16_t dest, uint16_t src_l, uint16_t src_h)
		{
			uint16_t addr = (uint16_t)(cpu_Regs[src_l] | (cpu_Regs[src_h]) << 8);
			cpu_Regs[dest] = ReadMemory(addr);
		}

		// special read for POP AF that always clears the lower 4 bits of F 
		void Read_Func_F(uint16_t dest, uint16_t src_l, uint16_t src_h)
		{
			cpu_Regs[dest] = (byte)(ReadMemory((uint16_t)(cpu_Regs[src_l] | (cpu_Regs[src_h]) << 8)) & 0xF0);
		}

		void Write_Func(uint16_t dest_l, uint16_t dest_h, uint16_t src)
		{
			uint16_t addr = (uint16_t)(cpu_Regs[dest_l] | (cpu_Regs[dest_h]) << 8);
			WriteMemory(addr, (byte)cpu_Regs[src]);
		}

		void TR_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = cpu_Regs[src];
		}

		void ADD8_Func(uint16_t dest, uint16_t src)
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

		void SUB8_Func(uint16_t dest, uint16_t src)
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

		void BIT_Func(uint16_t bit, uint16_t src)
		{
			cpu_FlagZset(!((cpu_Regs[src] & (1 << bit)) == (1 << bit)));
			cpu_FlagHset(true);
			cpu_FlagNset(false);
		}

		void SET_Func(uint16_t bit, uint16_t src)
		{
			cpu_Regs[src] |= (uint8_t)(1 << bit);
		}

		void RES_Func(uint16_t bit, uint16_t src)
		{
			cpu_Regs[src] &= (uint8_t)(0xFF - (1 << bit));
		}

		void ASGN_Func(uint16_t src, uint16_t val)
		{
			cpu_Regs[src] = (uint8_t) val;
		}

		void SWAP_Func(uint16_t src)
		{
			cpu_temp = (uint16_t)((cpu_Regs[src] << 4) & 0xF0);
			cpu_Regs[src] = (uint8_t)(cpu_temp | (cpu_Regs[src] >> 4));

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
			cpu_FlagCset(false);
		}

		void SLA_Func(uint16_t src)
		{
			cpu_FlagCset(((cpu_Regs[src] & 0x80) == 0x80));

			cpu_Regs[src] = (uint8_t)((cpu_Regs[src] << 1) & 0xFF);

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void SRA_Func(uint16_t src)
		{
			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_temp = (uint16_t)(cpu_Regs[src] & 0x80); // MSB doesn't change in this operation

			cpu_Regs[src] = (uint8_t)((cpu_Regs[src] >> 1) | cpu_temp);

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void SRL_Func(uint16_t src)
		{
			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_Regs[src] = (uint8_t)(cpu_Regs[src] >> 1);

			cpu_FlagZset(cpu_Regs[src] == 0);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void CPL_Func(uint16_t src)
		{
			cpu_Regs[src] = (uint8_t)((~cpu_Regs[src]) & 0xFF);

			cpu_FlagHset(true);
			cpu_FlagNset(true);
		}

		void CCF_Func(uint16_t src)
		{
			cpu_FlagCset(!cpu_FlagCget());
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void SCF_Func(uint16_t src)
		{
			cpu_FlagCset(true);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void AND8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = (uint8_t)(cpu_Regs[dest] & cpu_Regs[src]);

			cpu_FlagZset(cpu_Regs[dest] == 0);
			cpu_FlagCset(false);
			cpu_FlagHset(true);
			cpu_FlagNset(false);
		}

		void OR8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = (uint8_t)(cpu_Regs[dest] | cpu_Regs[src]);

			cpu_FlagZset(cpu_Regs[dest] == 0);
			cpu_FlagCset(false);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void XOR8_Func(uint16_t dest, uint16_t src)
		{
			cpu_Regs[dest] = (uint8_t)(cpu_Regs[dest] ^ cpu_Regs[src]);

			cpu_FlagZset(cpu_Regs[dest] == 0);
			cpu_FlagCset(false);
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void CP8_Func(uint16_t dest, uint16_t src)
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

		void RRC_Func(uint16_t src)
		{
			cpu_imm = (src == cpu_Aim);
			if (cpu_imm) { src = cpu_A; }

			cpu_FlagCset(((cpu_Regs[src] & 0x1) == 0x1));

			cpu_Regs[src] = (uint16_t)((cpu_FlagCget() ? 0x80 : 0) | (cpu_Regs[src] >> 1));

			cpu_FlagZset(cpu_imm ? false : (cpu_Regs[src] == 0));
			cpu_FlagHset(false);
			cpu_FlagNset(false);
		}

		void RR_Func(uint16_t src)
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


		void RL_Func(uint16_t src)
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

		void ADC8_Func(uint16_t dest, uint16_t src)
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

		void SBC8_Func(uint16_t dest, uint16_t src)
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
		void DA_Func(uint16_t src)
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
		void ADDS_Func(uint16_t dest_l, uint16_t dest_h, uint16_t src_l, uint16_t src_h)
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

		#pragma endregion

		#pragma region Disassemble

		// disassemblies will also return strings of the same length
		const char* TraceHeader = "ARM7TDMI: PC, machine code, mnemonic, operands, registers, Cy, flags (ZNHCIFE)";
		const char* SWI_event =  "                                 ====SWI====                                 ";
		const char* UDF_event =  "                                 ====UDF====                                 ";
		const char* IRQ_event =  "                                 ====IRQ====                                 ";
		const char* HALT_event = "                                 ====HALT====                                ";
		const char* DMA_event =  "                                 ====DMA====                                 ";
		const char* No_Reg = 
			"                                                                                                                                                                                                                                                                                          ";
		const char* Reg_template = 
			"R0:XXXXXXXX R1:XXXXXXXX R2:XXXXXXXX R3:XXXXXXXX R4:XXXXXXXX R5:XXXXXXXX R6:XXXXXXXX R7:XXXXXXXX R8:XXXXXXXX R9:XXXXXXXX R10:XXXXXXXX R11:XXXXXXXX R12:XXXXXXXX R13:XXXXXXXX R14:XXXXXXXX R15:XXXXXXXX CPSR:XXXXXXXX SPSR:XXXXXXXX Cy:XXXXXXXXXXXXXXXX LY:XXX NZCVIFE F-Cy:XXXXXXXXXXXXXXXX";
		const char* Disasm_template = "PCPCPCPCPC:  AABBCCDD  (fail)  Di Di Di Di Di Di Di Di                       ";

		char replacer[40] = {};
		char* val_char_1 = nullptr;
		char* val_char_2 = nullptr;
		uint32_t temp_reg;

		void (*TraceCallback)(int);
		void (*RumbleCallback)(bool);

		string CPUDMAState()
		{
			val_char_1 = replacer;

			string reg_state = "CH:";

			temp_reg = cpu_Regs[0];
			sprintf_s(val_char_1, 9, "%08X", dma_Chan_Exec);
			reg_state.append(val_char_1, 8);

			reg_state.append(" SR:");
			temp_reg = cpu_Regs[1];
			sprintf_s(val_char_1, 9, "%08X", dma_SRC_intl[dma_Chan_Exec]);
			reg_state.append(val_char_1, 8);

			reg_state.append(" DT:");
			temp_reg = cpu_Regs[2];
			sprintf_s(val_char_1, 9, "%08X", dma_DST_intl[dma_Chan_Exec]);
			reg_state.append(val_char_1, 8);

			reg_state.append(" CR:");
			temp_reg = cpu_Regs[3];
			sprintf_s(val_char_1, 9, "%08X", dma_CTRL[dma_Chan_Exec]);
			reg_state.append(val_char_1, 8);

			reg_state.append(" CT:");
			temp_reg = cpu_Regs[3];
			sprintf_s(val_char_1, 9, "%08X", dma_CNT[dma_Chan_Exec]);
			reg_state.append(val_char_1, 8);

			reg_state.append(" Cy:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", CycleCount));

			while (reg_state.length() < 282)
			{
				reg_state.append(" ");
			}

			return reg_state;
		}

		string CPURegisterState()
		{		
			val_char_1 = replacer;

			string reg_state = "R0:";

			temp_reg = cpu_Regs[0];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R1:");			
			temp_reg = cpu_Regs[1];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R2:");
			temp_reg = cpu_Regs[2];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R3:");
			temp_reg = cpu_Regs[3];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R4:");
			temp_reg = cpu_Regs[4];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R5:");
			temp_reg = cpu_Regs[5];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R6:");
			temp_reg = cpu_Regs[6];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R7:");
			temp_reg = cpu_Regs[7];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R8:");
			temp_reg = cpu_Regs[8];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R9:");
			temp_reg = cpu_Regs[9];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R10:");
			temp_reg = cpu_Regs[10];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R11:");
			temp_reg = cpu_Regs[11];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R12:");
			temp_reg = cpu_Regs[12];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R13:");
			temp_reg = cpu_Regs[13];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R14:");
			temp_reg = cpu_Regs[14];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" R15:");
			temp_reg = cpu_Regs[15];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" CPSR:");
			temp_reg = cpu_Regs[16];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" SPSR:");
			temp_reg = cpu_Regs[17];
			sprintf_s(val_char_1, 9, "%08X", temp_reg);
			reg_state.append(val_char_1, 8);

			reg_state.append(" Cy:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", CycleCount));

			reg_state.append(" LY:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 4, "%3u", ppu_LY));
			reg_state.append(" ");
			
			reg_state.append(cpu_FlagNget() ? "N" : "n");
			reg_state.append(cpu_FlagZget() ? "Z" : "z");
			reg_state.append(cpu_FlagCget() ? "C" : "c");		
			reg_state.append(cpu_FlagVget() ? "V" : "v");
			reg_state.append(cpu_FlagIget() ? "I" : "i");
			reg_state.append(cpu_FlagFget() ? "F" : "f");
			reg_state.append(INT_Master_On ? "E" : "e");

			reg_state.append(" F-Cy:");
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", FrameCycle));

			return reg_state;
		}

		string CPUDisassembly()
		{
			string byte_code = "";
			
			val_char_1 = replacer;
			
			if (cpu_Thumb_Mode)
			{
				sprintf_s(val_char_1, 9, "%08X", (cpu_Regs[15] - 4));
				byte_code.append(val_char_1, 8);
				byte_code.append(":      ");
				sprintf_s(val_char_1, 5, "%04X", cpu_Instr_TMB_2);
				byte_code.append(val_char_1, 4);
				byte_code.append("  ");
				byte_code.append(cpu_Disassemble_TMB());
			}
			else
			{
				sprintf_s(val_char_1, 9, "%08X", (cpu_Regs[15] - 8));
				byte_code.append(val_char_1, 8);
				byte_code.append(":  ");
				sprintf_s(val_char_1, 9, "%08X", cpu_Instr_ARM_2);
				byte_code.append(val_char_1, 8);
				byte_code.append("  ");
				if (!cpu_ARM_Condition_Check()) { byte_code.append("(fail)  "); }
				byte_code.append(cpu_Disassemble_ARM());
			}

			while (byte_code.length() < 78) 
			{
				byte_code.append(" ");
			}

			return byte_code;
		}

		string cpu_Disassemble_ARM()
		{
			string ret = "";

			val_char_2 = replacer;

			switch ((cpu_Instr_ARM_2 >> 25) & 7)
			{
			case 0:
				if ((cpu_Instr_ARM_2 & 0x90) == 0x90)
				{
					// miscellaneous
					if (((cpu_Instr_ARM_2 & 0xF0) == 0x90))
					{
						switch ((cpu_Instr_ARM_2 >> 22) & 0x7)
						{
							case 0x0:
								// Multiply
								if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
								{
									sprintf_s(val_char_2, 40, "MLA R%02d, R%02d * R%02d + R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 12) & 0xF));
								}
								else
								{
									sprintf_s(val_char_2, 40, "MUL R%02d, R%02d * R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF));
								}
								return std::string(val_char_2);

							case 0x1:
								// Unpredictable but still Multiply
								if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
								{
									sprintf_s(val_char_2, 40, "MLA R%02d, R%02d * R%02d + R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 12) & 0xF));
								}
								else
								{
									sprintf_s(val_char_2, 40, "MUL R%02d, R%02d * R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF));
								}
								return std::string(val_char_2);

							case 0x2:
								// Multiply Long - Unsigned
								sprintf_s(val_char_2, 40, "MUL-LU R%02dH  R%02dL, R%02d * R%02d",
									((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF));
								return std::string(val_char_2);

							case 0x3:
								// Multiply Long - Signed
								sprintf_s(val_char_2, 40, "MUL-LS R%02dH  R%02dL, R%02d * R%02d",
									((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF));
								return std::string(val_char_2);

							case 0x4:
							case 0x5:
							case 0x6:
							case 0x7:
								// Swap
								if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
								{
									sprintf_s(val_char_2, 40, "SWAPB R%02d, (R%02d), R%02d",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF));
								}
								else
								{
									sprintf_s(val_char_2, 40, "SWAP R%02d, (R%02d), R%02d",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF));
								}

								return std::string(val_char_2);
						}
					}
					else
					{
						// halfword or byte transfers
						if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
						{
							ret.append("LD");
						}
						else
						{
							ret.append("ST");
						}

						switch ((cpu_Instr_ARM_2 >> 5) & 0x3)
						{
							// 0 case is not a load store instruction
							case 0x1:
								// Unsigned halfword
								ret.append("H ");
								break;

							case 0x2:
								// Signed Byte
								ret.append("SB ");
								break;
							case 0x3:
								// Signed halfword
								ret.append("SH ");
								break;
						}

						if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
						{
							sprintf_s(val_char_2, 40, "R%02d, (R%02d, ", ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF));
							ret.append(std::string(val_char_2));

							if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
							{
								sprintf_s(val_char_2, 40, "%2X)", ((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF));
								ret.append(std::string(val_char_2));
							}
							else
							{
								sprintf_s(val_char_2, 40, "R%02d)", (cpu_Instr_ARM_2 & 0xF));
								ret.append(std::string(val_char_2));
							}
						}
						else
						{
							sprintf_s(val_char_2, 40, "(R%02d, ", ((cpu_Instr_ARM_2 >> 16) & 0xF));
							ret.append(std::string(val_char_2));

							if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
							{
								sprintf_s(val_char_2, 40, "%2X), ", ((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF));
								ret.append(std::string(val_char_2));
							}
							else
							{
								sprintf_s(val_char_2, 40, "R%02d), ", (cpu_Instr_ARM_2 & 0xF));
								ret.append(std::string(val_char_2));
							}

							sprintf_s(val_char_2, 40, "R%02d", ((cpu_Instr_ARM_2 >> 12) & 0xF));
							ret.append(std::string(val_char_2));
						}

						return ret;
					}
				}
				else
				{
					// ALU ops
					if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
					{
						// update flags
						switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
						{
							case 0x0: ret.append("AND "); break;
							case 0x1: ret.append("EOR "); break;
							case 0x2: ret.append("SUB "); break;
							case 0x3: ret.append("RSB "); break;
							case 0x4: ret.append("ADD "); break;
							case 0x5: ret.append("ADC "); break;
							case 0x6: ret.append("SBC "); break;
							case 0x7: ret.append("RSC "); break;
							case 0x8: ret.append("TST "); break;
							case 0x9: ret.append("TEQ "); break;
							case 0xA: ret.append("CMP "); break;
							case 0xB: ret.append("CMN "); break;
							case 0xC: ret.append("ORR "); break;
							case 0xD: ret.append("MOV "); break;
							case 0xE: ret.append("BIC "); break;
							case 0xF: ret.append("MVN "); break;
						}

						sprintf_s(val_char_2, 40, "R%02d, R%02d, ", ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF));
						ret.append(std::string(val_char_2));
					}
					else
					{
						// don't update flags
						switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
						{
							case 0x0: ret.append("AND "); break;
							case 0x1: ret.append("EOR "); break;
							case 0x2: ret.append("SUB "); break;
							case 0x3: ret.append("RSB "); break;
							case 0x4: ret.append("ADD "); break;
							case 0x5: ret.append("ADC "); break;
							case 0x6: ret.append("SBC "); break;
							case 0x7: ret.append("RSC "); break;
							case 0x8: sprintf_s(val_char_2, 40, "MRS R%02d, CPSR", ((cpu_Instr_ARM_2 >> 12) & 0xF)); return std::string(val_char_2);
							case 0x9:
								if ((cpu_Instr_ARM_2 & 0xFFF90) == 0xFFF10)
								{
									sprintf_s(val_char_2, 40, "Bx R%02d", (cpu_Instr_ARM_2 & 0xF));
									return std::string(val_char_2);
								}
								else
								{
									sprintf_s(val_char_2, 40, "MSR CPSR, mask:%02d, ", ((cpu_Instr_ARM_2 >> 16) & 0xF));
									ret.append(std::string(val_char_2)); break;
								}
							case 0xA: sprintf_s(val_char_2, 40, "MRS R%02d, SPSR", ((cpu_Instr_ARM_2 >> 12) & 0xF)); return std::string(val_char_2);
							case 0xB:
								if ((cpu_Instr_ARM_2 & 0xFFF90) == 0xFFF10)
								{
									sprintf_s(val_char_2, 40, "Bx R%02d", (cpu_Instr_ARM_2 & 0xF));
									return std::string(val_char_2);
								}
								else
								{
									sprintf_s(val_char_2, 40, "MSR SPSR, mask:%02d, ", ((cpu_Instr_ARM_2 >> 16) & 0xF));
									ret.append(std::string(val_char_2));
								}
								break;
							case 0xC: ret.append("ORR "); break;
							case 0xD: ret.append("MOV "); break;
							case 0xE: ret.append("BIC "); break;
							case 0xF: ret.append("MVN "); break;
						}

						if ((((cpu_Instr_ARM_2 >> 21) & 0xF) == 0x9) || (((cpu_Instr_ARM_2 >> 21) & 0xF) == 0xB))
						{
							// nothing to append
						}
						else
						{
							ret.append("(no flags) ");
							sprintf_s(val_char_2, 40, "R%02d, R%02d, ", ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF));
							ret.append(std::string(val_char_2));
						}
					}

					sprintf_s(val_char_2, 40, "(R%02d ", (cpu_Instr_ARM_2 & 0xF));
					ret.append(std::string(val_char_2));

					switch ((cpu_Instr_ARM_2 >> 5) & 3)
					{
						case 0:         // LSL
							ret.append("<< ");
							break;

						case 1:         // LSR
							ret.append(">> ");
							break;

						case 2:         // ASR
							ret.append("ASR ");
							break;

						case 3:         // RRX
							ret.append("RRX ");
						break;
					}

					if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
					{
						// immediate shift
						sprintf_s(val_char_2, 40, "%2X)", ((cpu_Instr_ARM_2 >> 7) & 0x1F));
						ret.append(std::string(val_char_2));
					}
					else
					{
						// register shift
						sprintf_s(val_char_2, 40, "R%02d & FF)", ((cpu_Instr_ARM_2 >> 8) & 0xF));
						ret.append(std::string(val_char_2));
					}

					return ret;
				}
				break;

			case 1:
				// ALU ops
				if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
				{
					// update flags
					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
						case 0x0: ret.append("AND "); break;
						case 0x1: ret.append("EOR "); break;
						case 0x2: ret.append("SUB "); break;
						case 0x3: ret.append("RSB "); break;
						case 0x4: ret.append("ADD "); break;
						case 0x5: ret.append("ADC "); break;
						case 0x6: ret.append("SBC "); break;
						case 0x7: ret.append("RSC "); break;
						case 0x8: ret.append("TST "); break;
						case 0x9: ret.append("TEQ "); break;
						case 0xA: ret.append("CMP "); break;
						case 0xB: ret.append("CMN "); break;
						case 0xC: ret.append("ORR "); break;
						case 0xD: ret.append("MOV "); break;
						case 0xE: ret.append("BIC "); break;
						case 0xF: ret.append("MVN "); break;
					}
					sprintf_s(val_char_2, 40, "R%02d, R%02d, (%2X >> %2X)",
							((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xFF), ((cpu_Instr_ARM_2 >> 7) & 0x1E));
					ret.append(std::string(val_char_2));
				}
				else
				{
					// don't update flags
					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
						case 0x0: ret.append("AND "); break;
						case 0x1: ret.append("EOR "); break;
						case 0x2: ret.append("SUB "); break;
						case 0x3: ret.append("RSB "); break;
						case 0x4: ret.append("ADD "); break;
						case 0x5: ret.append("ADC "); break;
						case 0x6: ret.append("SBC "); break;
						case 0x7: ret.append("RSC "); break;
						case 0x8:
							sprintf_s(val_char_2, 40, "MSR GLCH, mask:%02d, (%2X >> %2X)",
								((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1E));
							return std::string(val_char_2);
						case 0x9:
							sprintf_s(val_char_2, 40, "MSR CPSR, mask:%02d, (%2X >> %2X)",
									((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1E));
							return std::string(val_char_2);
						case 0xA:
							sprintf_s(val_char_2, 40, "MSR GLCH, mask:%02d, (%2X >> %2X)",
								((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1E));
							return std::string(val_char_2);
						case 0xB:
							sprintf_s(val_char_2, 40, "MSR SPSR, mask:%02d, (%2X >> %2X)",
									((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1E)); 
							return std::string(val_char_2);
						case 0xC: ret.append("ORR "); break;
						case 0xD: ret.append("MOV "); break;
						case 0xE: ret.append("BIC "); break;
						case 0xF: ret.append("MVN "); break;
					}
					ret.append("(no flags) ");
					sprintf_s(val_char_2, 40, "R%02d, R%02d, (%2X >> %2X)",
							((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xFF), ((cpu_Instr_ARM_2 >> 7) & 0x1E));
					ret.append(std::string(val_char_2));
				}
				return ret;

			case 2:
				// load / store immediate offset
				if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
				{
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
					{
						sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d, %3X)", ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xFFF));
					}
					else
					{
						sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, %3X)", ((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xFFF));
					}
				}
				else
				{
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
					{
						sprintf_s(val_char_2, 40, "STB (R%02d, %3X), R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xFFF), ((cpu_Instr_ARM_2 >> 12) & 0xF));
					}
					else
					{
						sprintf_s(val_char_2, 40, "ST (R%02d, %3X), R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xFFF), ((cpu_Instr_ARM_2 >> 12) & 0xF));
					}
				}

				return std::string(val_char_2);

			case 3:
				if ((cpu_Instr_ARM_2 & 0x10) == 0)
				{
					// load / store register offset
					if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
					{
						if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
						{
							switch ((cpu_Instr_ARM_2 >> 5) & 3)
							{
								case 0:         // LSL
									sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d, R%02d << %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
								case 1:         // LSR
									sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d, R%02d >> %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
								case 2:         // ASR
									sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d, R%02d ASR %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
								case 3:         // RRX
									sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d, R%02d RRX %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
							}
						}
						else
						{
							switch ((cpu_Instr_ARM_2 >> 5) & 3)
							{
								case 0:         // LSL
									sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d << %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
								case 1:         // LSR
									sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d >> %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
								case 2:         // ASR
									sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d ASR %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
								case 3:         // RRX
									sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d RRX %2X)",
										((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F)); break;
							}
						}
					}
					else
					{
						if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
						{
							switch ((cpu_Instr_ARM_2 >> 5) & 3)
							{
								case 0:         // LSL
									sprintf_s(val_char_2, 40, "STB (R%02d, R%02d << %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
								case 1:         // LSR
									sprintf_s(val_char_2, 40, "STB (R%02d, R%02d >> %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
								case 2:         // ASR
									sprintf_s(val_char_2, 40, "STB (R%02d, R%02d ASR %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
								case 3:         // RRX
									sprintf_s(val_char_2, 40, "STB (R%02d, R%02d RRX %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
							}
						}
						else
						{
							switch ((cpu_Instr_ARM_2 >> 5) & 3)
							{
								case 0:         // LSL
									sprintf_s(val_char_2, 40, "ST (R%02d, R%02d << %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
								case 1:         // LSR
									sprintf_s(val_char_2, 40, "ST (R%02d, R%02d >> %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
								case 2:         // ASR
									sprintf_s(val_char_2, 40, "ST (R%02d, R%02d ASR %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
								case 3:         // RRX
									sprintf_s(val_char_2, 40, "ST (R%02d, R%02d RRX %2X), R%02d",
										((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF)); break;
							}
						}
					}

					return std::string(val_char_2);
				}
				else
				{
					// Undefined Opcode Exception
					return "Undefined";
				}
				return "";

			case 4:
				// block transfer
				if ((cpu_Instr_ARM_2 & 0x100000) == 0x100000)
				{
					// user mode
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
					{
						sprintf_s(val_char_2, 40, "LDMU %8X regs:%4X", cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)], (cpu_Instr_ARM_2 & 0xFFFF));
					}
					else
					{
						sprintf_s(val_char_2, 40, "LDM %8X regs:%4X", cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)], (cpu_Instr_ARM_2 & 0xFFFF));
					}
				}
				else
				{
					// user mode
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
					{
						sprintf_s(val_char_2, 40, "STMU %8X regs:%4X", cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)], (cpu_Instr_ARM_2 & 0xFFFF));
					}
					else
					{
						sprintf_s(val_char_2, 40, "STM %8X regs:%4X", cpu_Regs[((cpu_Instr_ARM_2 >> 16) & 0xF)], (cpu_Instr_ARM_2 & 0xFFFF));
					}
				}

				return std::string(val_char_2);

			case 5:
				// branch
				if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
				{
					// Link if link bit set
					// offset is signed
					if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
					{
						sprintf_s(val_char_2, 40, "BL %8X", (((cpu_Instr_ARM_2 & 0xFFFFFF) << 2) | 0xFC000000));
					}
					else
					{
						sprintf_s(val_char_2, 40, "BL %8X", ((cpu_Instr_ARM_2 & 0xFFFFFF) << 2));
					}
				}
				else
				{
					// offset is signed
					if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
					{
						sprintf_s(val_char_2, 40, "B %8X", (((cpu_Instr_ARM_2 & 0xFFFFFF) << 2) | 0xFC000000));
					}
					else
					{
						sprintf_s(val_char_2, 40, "B %8X", ((cpu_Instr_ARM_2 & 0xFFFFFF) << 2));
					}
				}

				return std::string(val_char_2);

			case 6:
				// Coprocessor Instruction (treat as Undefined Opcode Exception)
				return "Undefined";

			case 7:
				if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
				{
					// software interrupt
					return "SWI";
				}
				else
				{
					// Coprocessor Instruction (treat as Undefined Opcode Exception)
					return "Undefined";
				}
			}

			return "";
		}

		string cpu_Disassemble_TMB()
		{
			val_char_2 = replacer;
			
			switch ((cpu_Instr_TMB_2 >> 13) & 7)
			{
			case 0:
				// shift / add / sub
				if ((cpu_Instr_TMB_2 & 0x1800) == 0x1800)
				{
					if ((cpu_Instr_TMB_2 & 0x200) == 0x200)
					{
						if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
						{
							sprintf_s(val_char_2, 40, "SUB R%02d = R%02d - %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x7));
						}
						else
						{
							sprintf_s(val_char_2, 40, "SUB R%02d = R%02d - R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
						}
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
						{
							sprintf_s(val_char_2, 40, "ADD R%02d = R%02d + %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x7));
						}
						else
						{
							sprintf_s(val_char_2, 40, "ADD R%02d = R%02d + R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
						}
					}
				}
				else
				{
					switch ((cpu_Instr_TMB_2 >> 11) & 0x3)
					{
					case 0:
						sprintf_s(val_char_2, 40, "LSL IMM R%02d = R%02d << %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F)); break;

					case 1:
						sprintf_s(val_char_2, 40, "LSR IMM R%02d = R%02d >> %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F)); break;

					case 2:
						sprintf_s(val_char_2, 40, "ROR IMM R%02d = R%02d >> %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F)); break;
					}
				}

				return std::string(val_char_2);

			case 1:
				// data ops (immedaite)
				switch ((cpu_Instr_TMB_2 >> 11) & 3)
				{
					case 0:         // MOV
						sprintf_s(val_char_2, 40, "MOV IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF)); break;

					case 1:         // CMP
						sprintf_s(val_char_2, 40, "CMP IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF)); break;

					case 2:         // ADD
						sprintf_s(val_char_2, 40, "ADD IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF)); break;

					case 3:         // SUB
						sprintf_s(val_char_2, 40, "SUB IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF)); break;
				}

				return std::string(val_char_2);

			case 2:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0x0)
				{
					if ((cpu_Instr_TMB_2 & 0x800) == 0x0)
					{
						if ((cpu_Instr_TMB_2 & 0x400) == 0x0)
						{
							// ALU Ops
							switch ((cpu_Instr_TMB_2 >> 6) & 0xF)
							{
								case 0x0: sprintf_s(val_char_2, 40, "AND R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x1: sprintf_s(val_char_2, 40, "EOR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x2: sprintf_s(val_char_2, 40, "LSL R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x3: sprintf_s(val_char_2, 40, "LSR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x4: sprintf_s(val_char_2, 40, "ASR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x5: sprintf_s(val_char_2, 40, "ADC R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x6: sprintf_s(val_char_2, 40, "SBC R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x7: sprintf_s(val_char_2, 40, "ROR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x8: sprintf_s(val_char_2, 40, "TST R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0x9: sprintf_s(val_char_2, 40, "NEG R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0xA: sprintf_s(val_char_2, 40, "CMP R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0xB: sprintf_s(val_char_2, 40, "CMN R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0xC: sprintf_s(val_char_2, 40, "ORR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0xD: sprintf_s(val_char_2, 40, "MUL R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0xE: sprintf_s(val_char_2, 40, "BIC R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
								case 0xF: sprintf_s(val_char_2, 40, "MVN R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7)); break;
							}

							return std::string(val_char_2);
						}
						else
						{
							// High Regs / Branch and exchange
							switch ((cpu_Instr_TMB_2 >> 8) & 3)
							{
								case 0:
									sprintf_s(val_char_2, 40, "ADD R%02d, R%02d", (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8), ((cpu_Instr_TMB_2 >> 3) & 0xF));
									return std::string(val_char_2);

								case 1:
									sprintf_s(val_char_2, 40, "CMP R%02d, R%02d", (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8), ((cpu_Instr_TMB_2 >> 3) & 0xF));
									return std::string(val_char_2);

								case 2:
									if ((cpu_Instr_TMB_2 & 0xC0) == 0x0)
									{
										sprintf_s(val_char_2, 40, "CPY R%02d, R%02d", (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8), ((cpu_Instr_TMB_2 >> 3) & 0xF));
										return std::string(val_char_2);
									}
									else
									{
										sprintf_s(val_char_2, 40, "MOV R%02d, R%02d", (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8), ((cpu_Instr_TMB_2 >> 3) & 0xF));
										return std::string(val_char_2);
									}

								case 3:
									sprintf_s(val_char_2, 40, "Bx (R%02d)", ((cpu_Instr_TMB_2 >> 3) & 0xF));
									return std::string(val_char_2);
							}
						}
					}
					else
					{
						// PC relative load
						sprintf_s(val_char_2, 40, "LD R%02d, (PC + %3X)", ((cpu_Instr_TMB_2 >> 8) & 7), ((cpu_Instr_TMB_2 & 0xFF) << 2));
						return std::string(val_char_2);
					}
				}
				else
				{
					// Load / store Relative offset
					switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
					{
						case 0: sprintf_s(val_char_2, 40, "ST (R%02d + R%02d), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7), (cpu_Instr_TMB_2 & 7)); break;
						case 1: sprintf_s(val_char_2, 40, "STH (R%02d + R%02d), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7), (cpu_Instr_TMB_2 & 7)); break;
						case 2: sprintf_s(val_char_2, 40, "STB (R%02d + R%02d), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7), (cpu_Instr_TMB_2 & 7)); break;
						case 3: sprintf_s(val_char_2, 40, "LDSB R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7)); break;
						case 4: sprintf_s(val_char_2, 40, "LD R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7)); break;
						case 5: sprintf_s(val_char_2, 40, "LDH R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7)); break;
						case 6: sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7)); break;
						case 7: sprintf_s(val_char_2, 40, "LDSH R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7)); break;
					}

					return std::string(val_char_2);
				}
				return "";

			case 3:
				// Load / store Immediate offset
				if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
				{
					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d + %2X)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F));
					}
					else
					{
						sprintf_s(val_char_2, 40, "STB (R%02d + %2X), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F), (cpu_Instr_TMB_2 & 7));
					}
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						sprintf_s(val_char_2, 40, "LD R%02d, (R%02d + %2X)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 4) & 0x7C));
					}
					else
					{
						sprintf_s(val_char_2, 40, "ST (R%02d + %2X), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 4) & 0x7C), (cpu_Instr_TMB_2 & 7));
					}
				}

				return std::string(val_char_2);

			case 4:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Load / store half word
					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						sprintf_s(val_char_2, 40, "LDH R%02d, (R%02d + %2X)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 5) & 0x3E));
					}
					else
					{
						sprintf_s(val_char_2, 40, "STH (R%02d + %2X), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 5) & 0x3E), (cpu_Instr_TMB_2 & 7));
					}
				}
				else
				{
					// SP relative load store
					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						sprintf_s(val_char_2, 40, "LD R%02d, (R13 + %3X)", ((cpu_Instr_TMB_2 >> 8) & 7), ((cpu_Instr_TMB_2 & 0xFF) << 2));
					}
					else
					{
						sprintf_s(val_char_2, 40, "ST (R13 + %3X), R%02d", ((cpu_Instr_TMB_2 & 0xFF) << 2), ((cpu_Instr_TMB_2 >> 8) & 7));
					}
				}

				return std::string(val_char_2);

			case 5:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Load Address
					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						sprintf_s(val_char_2, 40, "R%02d = R13 + %3X", ((cpu_Instr_TMB_2 >> 8) & 7), ((cpu_Instr_TMB_2 & 0xFF) << 2));
					}
					else
					{
						sprintf_s(val_char_2, 40, "R%02d = R15 + %3X", ((cpu_Instr_TMB_2 >> 8) & 7), ((cpu_Instr_TMB_2 & 0xFF) << 2));
					}

					return std::string(val_char_2);
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0xF00) == 0x0)
					{
						// Add offset to stack
						if ((cpu_Instr_TMB_2 & 0x80) == 0x0)
						{
							sprintf_s(val_char_2, 40, "ADD SP:%2X", (cpu_Instr_TMB_2 & 0x7F));
						}
						else
						{
							sprintf_s(val_char_2, 40, "SUB SP:%2X", (cpu_Instr_TMB_2 & 0x7F));
						}

						return std::string(val_char_2);
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0x600) == 0x400)
						{
							// Push / Pop
							if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
							{
								sprintf_s(val_char_2, 40, "Pop %8X regs:%3X", cpu_Regs[13], (cpu_Instr_TMB_2 & 0x1FF));
							}
							else
							{
								sprintf_s(val_char_2, 40, "Push %8X regs:%3X", cpu_Regs[13], (cpu_Instr_TMB_2 & 0x1FF));
							}

							return std::string(val_char_2);
						}
						else
						{
							// Undefined Opcode Exception
							return "Undefined";
						}
					}
				}

			case 6:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Multiple Load/Store
					if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
					{
						sprintf_s(val_char_2, 40, "LDM %8X regs:%2X", cpu_Regs[(cpu_Instr_TMB_2 >> 8) & 7], (cpu_Instr_TMB_2 & 0xFF));
					}
					else
					{
						sprintf_s(val_char_2, 40, "STM %8X regs:%2X", cpu_Regs[(cpu_Instr_TMB_2 >> 8) & 7], (cpu_Instr_TMB_2 & 0xFF));
					}

					return std::string(val_char_2);
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0xF00) == 0xF00)
					{
						// Software Interrupt
						return "SWI";
					}
					else if ((cpu_Instr_TMB_2 & 0xE00) == 0xE00)
					{
						// Undefined instruction
						return "Undefined";
					}
					else
					{
						// Conditional Branch
						if (cpu_TMB_Condition_Check())
						{
							if ((cpu_Instr_TMB_2 & 0x80) == 0x80)
							{
								sprintf_s(val_char_2, 40, "B (-) %2X", (cpu_Instr_TMB_2 & 0xFF));
							}
							else
							{
								sprintf_s(val_char_2, 40, "B (+) %2X", (cpu_Instr_TMB_2 & 0xFF));
							}

							return std::string(val_char_2);
						}
						else
						{
							return "B (failed)";
						}
					}
				}

			case 7:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					if ((cpu_Instr_TMB_2 & 0x800) == 0)
					{
						// Unconditional branch
						if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
						{
							sprintf_s(val_char_2, 40, "B (-) %3X", (cpu_Instr_TMB_2 & 0x7FF));
						}
						else
						{
							sprintf_s(val_char_2, 40, "B (+) %3X", (cpu_Instr_TMB_2 & 0x7FF));
						}

						return std::string(val_char_2);
					}
					else
					{
						// Undefined Opcode Exception
						return "Undefined";
					}
				}
				else
				{
					// Branch with link
					if ((cpu_Instr_TMB_2 & 0x800) == 0)
					{
						// A standard data operation assigning the upper part of the branch

						// offset is signed
						if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
						{
							sprintf_s(val_char_2, 40, "BL 1 (-) %3X", (cpu_Instr_TMB_2 & 0x7FF));
						}
						else
						{
							sprintf_s(val_char_2, 40, "BL 1 (+) %3X", (cpu_Instr_TMB_2 & 0x7FF));
						}
					}
					else
					{
						// Actual branch operation (can it occur without the first one?)
						if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
						{
							sprintf_s(val_char_2, 40, "BL 2 %3X", (cpu_Instr_TMB_2 & 0x7FF));
						}
						else
						{
							sprintf_s(val_char_2, 40, "BL 2 %3X", (cpu_Instr_TMB_2 & 0x7FF));
						}
					}

					return std::string(val_char_2);
				}
			}

			return "";
		}


		#pragma endregion

		#pragma region CPU State Save / Load

		uint8_t* cpu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(cpu_Thumb_Mode, saver);
			saver = bool_saver(cpu_ARM_Cond_Passed, saver);
			saver = bool_saver(cpu_Seq_Access, saver);
			saver = bool_saver(cpu_IRQ_Input, saver);
			saver = bool_saver(cpu_IRQ_Input_Use, saver);
			saver = bool_saver(cpu_Next_IRQ_Input, saver);
			saver = bool_saver(cpu_Next_IRQ_Input_2, saver);
			saver = bool_saver(cpu_Next_IRQ_Input_3, saver);
			saver = bool_saver(cpu_Is_Paused, saver);
			saver = bool_saver(cpu_No_IRQ_Clock, saver);
			saver = bool_saver(cpu_Restore_IRQ_Clock, saver);
			saver = bool_saver(cpu_Take_Branch, saver);
			saver = bool_saver(cpu_LS_Is_Load, saver);
			saver = bool_saver(cpu_LS_First_Access, saver);
			saver = bool_saver(cpu_Invalidate_Pipeline, saver);
			saver = bool_saver(cpu_Overwrite_Base_Reg, saver);
			saver = bool_saver(cpu_Multi_Before, saver);
			saver = bool_saver(cpu_Multi_Inc, saver);
			saver = bool_saver(cpu_Multi_S_Bit, saver);
			saver = bool_saver(cpu_ALU_S_Bit, saver);
			saver = bool_saver(cpu_Multi_Swap, saver);
			saver = bool_saver(cpu_Sign_Extend_Load, saver);
			saver = bool_saver(cpu_Dest_Is_R15, saver);
			saver = bool_saver(cpu_Swap_Store, saver);
			saver = bool_saver(cpu_Swap_Lock, saver);
			saver = bool_saver(cpu_Clear_Pipeline, saver);
			saver = bool_saver(cpu_Special_Inc, saver);
			saver = bool_saver(cpu_FlagI_Old, saver);
			saver = bool_saver(cpu_LDM_Glitch_Mode, saver);
			saver = bool_saver(cpu_LDM_Glitch_Store, saver);
			
			saver = bool_saver(stopped, saver);
			saver = bool_saver(cpu_Trigger_Unhalt, saver);
			saver = bool_saver(cpu_Trigger_Unhalt_2, saver);
			saver = bool_saver(cpu_Trigger_Unhalt_3, saver);
			saver = bool_saver(cpu_Trigger_Unhalt_4, saver);

			saver = short_saver(cpu_Exec_ARM, saver);
			saver = short_saver(cpu_Exec_TMB, saver);
			saver = short_saver(cpu_Instr_TMB_0, saver);
			saver = short_saver(cpu_Instr_TMB_1, saver);
			saver = short_saver(cpu_Instr_TMB_2, saver);
			saver = short_saver(cpu_Instr_Type, saver);
			saver = short_saver(cpu_Exception_Type, saver);
			saver = short_saver(cpu_Next_Load_Store_Type, saver);

			saver = int_saver(cpu_user_R8, saver);
			saver = int_saver(cpu_user_R9, saver);
			saver = int_saver(cpu_user_R10, saver);
			saver = int_saver(cpu_user_R11, saver);
			saver = int_saver(cpu_user_R12, saver);
			saver = int_saver(cpu_user_R13, saver);
			saver = int_saver(cpu_user_R14, saver);
			saver = int_saver(cpu_spr_R13, saver);
			saver = int_saver(cpu_spr_R14, saver);
			saver = int_saver(cpu_spr_S, saver);
			saver = int_saver(cpu_abort_R13, saver);
			saver = int_saver(cpu_abort_R14, saver);
			saver = int_saver(cpu_abort_S, saver);
			saver = int_saver(cpu_undf_R13, saver);
			saver = int_saver(cpu_undf_R14, saver);
			saver = int_saver(cpu_undf_S, saver);
			saver = int_saver(cpu_intr_R13, saver);
			saver = int_saver(cpu_intr_R14, saver);
			saver = int_saver(cpu_intr_S, saver);
			saver = int_saver(cpu_fiq_R8, saver);
			saver = int_saver(cpu_fiq_R9, saver);
			saver = int_saver(cpu_fiq_R10, saver);
			saver = int_saver(cpu_fiq_R11, saver);
			saver = int_saver(cpu_fiq_R12, saver);
			saver = int_saver(cpu_fiq_R13, saver);
			saver = int_saver(cpu_fiq_R14, saver);
			saver = int_saver(cpu_fiq_S, saver);

			saver = int_saver(cpu_Instr_ARM_0, saver);
			saver = int_saver(cpu_Instr_ARM_1, saver);
			saver = int_saver(cpu_Instr_ARM_2, saver);
			saver = int_saver(cpu_Temp_Reg, saver);
			saver = int_saver(cpu_Temp_Addr, saver);
			saver = int_saver(cpu_Temp_Data, saver);
			saver = int_saver(cpu_Temp_Mode, saver);
			saver = int_saver(cpu_Bit_To_Check, saver);
			saver = int_saver(cpu_Write_Back_Addr, saver);
			saver = int_saver(cpu_Addr_Offset, saver);
			saver = int_saver(cpu_Last_Bus_Value, saver);
			saver = int_saver(cpu_Last_Bus_Value_Old, saver);

			saver = int_saver(cpu_ALU_Temp_Val, saver);
			saver = int_saver(cpu_ALU_Temp_S_Val, saver);
			saver = int_saver(cpu_ALU_Shift_Carry, saver);

			saver = int_saver(cpu_Fetch_Cnt, saver);
			saver = int_saver(cpu_Fetch_Wait, saver);

			saver = int_saver(cpu_Multi_List_Ptr, saver);
			saver = int_saver(cpu_Multi_List_Size, saver);
			saver = int_saver(cpu_Temp_Reg_Ptr, saver);
			saver = int_saver(cpu_Base_Reg, saver);
			saver = int_saver(cpu_Base_Reg_2, saver);

			saver = int_saver(cpu_ALU_Reg_Dest, saver);
			saver = int_saver(cpu_ALU_Reg_Src, saver);

			saver = int_saver(cpu_Mul_Cycles, saver);
			saver = int_saver(cpu_Mul_Cycles_Cnt, saver);

			saver = int_saver(cpu_Shift_Imm, saver);

			saver = long_saver(cpu_ALU_Long_Result, saver);
			saver = long_saver(CycleCount, saver);
			saver = long_saver(FrameCycle, saver);
			saver = long_saver(Clock_Update_Cycle, saver);

			saver = long_saver(cpu_ALU_Signed_Long_Result, saver);

			saver = int_array_saver(cpu_Regs_To_Access, saver, 16);

			saver = int_array_saver(cpu_Regs, saver, 18);

			return saver;
		}

		uint8_t* cpu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&cpu_Thumb_Mode, loader);
			loader = bool_loader(&cpu_ARM_Cond_Passed, loader);
			loader = bool_loader(&cpu_Seq_Access, loader);
			loader = bool_loader(&cpu_IRQ_Input, loader);
			loader = bool_loader(&cpu_IRQ_Input_Use, loader);
			loader = bool_loader(&cpu_Next_IRQ_Input, loader);
			loader = bool_loader(&cpu_Next_IRQ_Input_2, loader);
			loader = bool_loader(&cpu_Next_IRQ_Input_3, loader);
			loader = bool_loader(&cpu_Is_Paused, loader);
			loader = bool_loader(&cpu_No_IRQ_Clock, loader);
			loader = bool_loader(&cpu_Restore_IRQ_Clock, loader);
			loader = bool_loader(&cpu_Take_Branch, loader);
			loader = bool_loader(&cpu_LS_Is_Load, loader);
			loader = bool_loader(&cpu_LS_First_Access, loader);
			loader = bool_loader(&cpu_Invalidate_Pipeline, loader);
			loader = bool_loader(&cpu_Overwrite_Base_Reg, loader);
			loader = bool_loader(&cpu_Multi_Before, loader);
			loader = bool_loader(&cpu_Multi_Inc, loader);
			loader = bool_loader(&cpu_Multi_S_Bit, loader);
			loader = bool_loader(&cpu_ALU_S_Bit, loader);
			loader = bool_loader(&cpu_Multi_Swap, loader);
			loader = bool_loader(&cpu_Sign_Extend_Load, loader);
			loader = bool_loader(&cpu_Dest_Is_R15, loader);
			loader = bool_loader(&cpu_Swap_Store, loader);
			loader = bool_loader(&cpu_Swap_Lock, loader);
			loader = bool_loader(&cpu_Clear_Pipeline, loader);
			loader = bool_loader(&cpu_Special_Inc, loader);
			loader = bool_loader(&cpu_FlagI_Old, loader);
			loader = bool_loader(&cpu_LDM_Glitch_Mode, loader);
			loader = bool_loader(&cpu_LDM_Glitch_Store, loader);

			loader = bool_loader(&stopped, loader);
			loader = bool_loader(&cpu_Trigger_Unhalt, loader);
			loader = bool_loader(&cpu_Trigger_Unhalt_2, loader);
			loader = bool_loader(&cpu_Trigger_Unhalt_3, loader);
			loader = bool_loader(&cpu_Trigger_Unhalt_4, loader);

			loader = short_loader(&cpu_Exec_ARM, loader);
			loader = short_loader(&cpu_Exec_TMB, loader);
			loader = short_loader(&cpu_Instr_TMB_0, loader);
			loader = short_loader(&cpu_Instr_TMB_1, loader);
			loader = short_loader(&cpu_Instr_TMB_2, loader);
			loader = short_loader(&cpu_Instr_Type, loader);
			loader = short_loader(&cpu_Exception_Type, loader);
			loader = short_loader(&cpu_Next_Load_Store_Type, loader);

			loader = int_loader(&cpu_user_R8, loader);
			loader = int_loader(&cpu_user_R9, loader);
			loader = int_loader(&cpu_user_R10, loader);
			loader = int_loader(&cpu_user_R11, loader);
			loader = int_loader(&cpu_user_R12, loader);
			loader = int_loader(&cpu_user_R13, loader);
			loader = int_loader(&cpu_user_R14, loader);
			loader = int_loader(&cpu_spr_R13, loader);
			loader = int_loader(&cpu_spr_R14, loader);
			loader = int_loader(&cpu_spr_S, loader);
			loader = int_loader(&cpu_abort_R13, loader);
			loader = int_loader(&cpu_abort_R14, loader);
			loader = int_loader(&cpu_abort_S, loader);
			loader = int_loader(&cpu_undf_R13, loader);
			loader = int_loader(&cpu_undf_R14, loader);
			loader = int_loader(&cpu_undf_S, loader);
			loader = int_loader(&cpu_intr_R13, loader);
			loader = int_loader(&cpu_intr_R14, loader);
			loader = int_loader(&cpu_intr_S, loader);
			loader = int_loader(&cpu_fiq_R8, loader);
			loader = int_loader(&cpu_fiq_R9, loader);
			loader = int_loader(&cpu_fiq_R10, loader);
			loader = int_loader(&cpu_fiq_R11, loader);
			loader = int_loader(&cpu_fiq_R12, loader);
			loader = int_loader(&cpu_fiq_R13, loader);
			loader = int_loader(&cpu_fiq_R14, loader);
			loader = int_loader(&cpu_fiq_S, loader);

			loader = int_loader(&cpu_Instr_ARM_0, loader);
			loader = int_loader(&cpu_Instr_ARM_1, loader);
			loader = int_loader(&cpu_Instr_ARM_2, loader);
			loader = int_loader(&cpu_Temp_Reg, loader);
			loader = int_loader(&cpu_Temp_Addr, loader);
			loader = int_loader(&cpu_Temp_Data, loader);
			loader = int_loader(&cpu_Temp_Mode, loader);
			loader = int_loader(&cpu_Bit_To_Check, loader);
			loader = int_loader(&cpu_Write_Back_Addr, loader);
			loader = int_loader(&cpu_Addr_Offset, loader);
			loader = int_loader(&cpu_Last_Bus_Value, loader);
			loader = int_loader(&cpu_Last_Bus_Value_Old, loader);

			loader = int_loader(&cpu_ALU_Temp_Val, loader);
			loader = int_loader(&cpu_ALU_Temp_S_Val, loader);
			loader = int_loader(&cpu_ALU_Shift_Carry, loader);

			loader = sint_loader(&cpu_Fetch_Cnt, loader);
			loader = sint_loader(&cpu_Fetch_Wait, loader);

			loader = sint_loader(&cpu_Multi_List_Ptr, loader);
			loader = sint_loader(&cpu_Multi_List_Size, loader);
			loader = sint_loader(&cpu_Temp_Reg_Ptr, loader);
			loader = sint_loader(&cpu_Base_Reg, loader);
			loader = sint_loader(&cpu_Base_Reg_2, loader);

			loader = sint_loader(&cpu_ALU_Reg_Dest, loader);
			loader = sint_loader(&cpu_ALU_Reg_Src, loader);

			loader = sint_loader(&cpu_Mul_Cycles, loader);
			loader = sint_loader(&cpu_Mul_Cycles_Cnt, loader);

			loader = sint_loader(&cpu_Shift_Imm, loader);

			loader = long_loader(&cpu_ALU_Long_Result, loader);
			loader = long_loader(&CycleCount, loader);
			loader = long_loader(&FrameCycle, loader);
			loader = long_loader(&Clock_Update_Cycle, loader);

			loader = slong_loader(&cpu_ALU_Signed_Long_Result, loader);

			loader = int_array_loader(cpu_Regs_To_Access, loader, 16);

			loader = int_array_loader(cpu_Regs, loader, 18);

			return loader;
		}

		#pragma endregion

	#pragma endregion

	#pragma region DMA

		bool dma_Seq_Access;
		bool dma_Pausable;
		bool dma_All_Off;
		bool dma_Shutdown;
		bool dma_Video_DMA_Start;
		bool dma_Video_DMA_Delay;

		uint16_t dma_TFR_HWord;
		uint16_t dma_Held_CPU_Instr;

		uint32_t dma_TFR_Word;

		uint32_t dma_Access_Cnt, dma_Access_Wait, dma_Chan_Exec;

		bool dma_Read_Cycle[4] = { };
		bool dma_Go[4] = { }; // Tell Condition checkers when the channel is on
		bool dma_Start_VBL[4] = { };
		bool dma_Start_HBL[4] = { };
		bool dma_Start_Snd_Vid[4] = { };
		bool dma_Run[4] = { }; // Actually run the DMA channel
		bool dma_Access_32[4] = { };
		bool dma_Use_ROM_Addr_SRC[4] = { };
		bool dma_Use_ROM_Addr_DST[4] = { };
		bool dma_ROM_Being_Used[4] = { };
		bool dma_ROM_Dec_Glitch_Read[4] = { };
		bool dma_ROM_Dec_Glitch_Write[4] = { };

		uint16_t dma_CNT[4] = { };
		uint16_t dma_CTRL[4] = { };

		uint32_t dma_SRC[4] = { };
		uint32_t dma_DST[4] = { };
		uint32_t dma_SRC_intl[4] = { };
		uint32_t dma_DST_intl[4] = { };
		uint32_t dma_SRC_INC[4] = { };
		uint32_t dma_DST_INC[4] = { };
		uint32_t dma_Last_Bus_Value[4] = { };

		uint32_t dma_CNT_intl[4] = { };
		uint32_t dma_ST_Time[4] = { };
		uint32_t dma_ROM_Addr[4] = { };
		uint32_t dma_Run_En_Time[4] = { };
		
		uint32_t dma_SRC_Mask[4] = {0x7FFFFFF, 0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF};

		uint32_t dma_DST_Mask[4] = {0x7FFFFFF, 0x7FFFFFF, 0x7FFFFFF, 0xFFFFFFF};

		uint32_t dma_CNT_Mask_0[4] = {0x4000, 0x4000, 0x4000, 0x10000};

		uint16_t dma_CNT_Mask[4] = {0x3FFF, 0x3FFF, 0x3FFF, 0xFFFF};


		uint8_t dma_Read_Reg_8(uint32_t addr)
		{
			uint8_t ret = 0;

			switch (addr)
			{
				case 0xB8: ret = 0; break;
				case 0xB9: ret = 0; break;
				case 0xBA: ret = (uint8_t)(dma_CTRL[0] & 0xFF); break;
				case 0xBB: ret = (uint8_t)((dma_CTRL[0] & 0xFF00) >> 8); break;

				case 0xC4: ret = 0; break;
				case 0xC5: ret = 0; break;
				case 0xC6: ret = (uint8_t)(dma_CTRL[1] & 0xFF); break;
				case 0xC7: ret = (uint8_t)((dma_CTRL[1] & 0xFF00) >> 8); break;

				case 0xD0: ret = 0; break;
				case 0xD1: ret = 0; break;
				case 0xD2: ret = (uint8_t)(dma_CTRL[2] & 0xFF); break;
				case 0xD3: ret = (uint8_t)((dma_CTRL[2] & 0xFF00) >> 8); break;

				case 0xDC: ret = 0; break;
				case 0xDD: ret = 0; break;
				case 0xDE: ret = (uint8_t)(dma_CTRL[3] & 0xFF); break;
				case 0xDF: ret = (uint8_t)((dma_CTRL[3] & 0xFF00) >> 8); break;

				default: ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (uint32_t)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		uint16_t dma_Read_Reg_16(uint32_t addr)
		{
			uint16_t ret = 0;

			switch (addr)
			{
				case 0xB8: ret = 0; break;
				case 0xBA: ret = dma_CTRL[0]; break;

				case 0xC4: ret = 0; break;
				case 0xC6: ret = dma_CTRL[1]; break;

				case 0xD0: ret = 0; break;
				case 0xD2: ret = dma_CTRL[2]; break;

				case 0xDC: ret = 0; break;
				case 0xDE: ret = dma_CTRL[3]; break;

				default: ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		uint32_t dma_Read_Reg_32(uint32_t addr)
		{
			uint32_t ret = 0;

			switch (addr)
			{
				case 0xB8: ret = (uint32_t)((dma_CTRL[0] << 16) | 0); break;

				case 0xC4: ret = (uint32_t)((dma_CTRL[1] << 16) | 0); break;

				case 0xD0: ret = (uint32_t)((dma_CTRL[2] << 16) | 0); break;

				case 0xDC: ret = (uint32_t)((dma_CTRL[3] << 16) | 0); break;

				default: ret = cpu_Last_Bus_Value; break; // open bus
			}

			return ret;
		}

		void dma_Write_Reg_8(uint32_t addr, uint8_t value)
		{
			switch (addr)
			{
				case 0xB0: dma_SRC[0] = (uint32_t)((dma_SRC[0] & 0xFFFFFF00) | value); break;
				case 0xB1: dma_SRC[0] = (uint32_t)((dma_SRC[0] & 0xFFFF00FF) | (value << 8)); break;
				case 0xB2: dma_SRC[0] = (uint32_t)((dma_SRC[0] & 0xFF00FFFF) | (value << 16)); break;
				case 0xB3: dma_SRC[0] = (uint32_t)((dma_SRC[0] & 0x00FFFFFF) | (value << 24)); break;
				case 0xB4: dma_DST[0] = (uint32_t)((dma_DST[0] & 0xFFFFFF00) | value); break;
				case 0xB5: dma_DST[0] = (uint32_t)((dma_DST[0] & 0xFFFF00FF) | (value << 8)); break;
				case 0xB6: dma_DST[0] = (uint32_t)((dma_DST[0] & 0xFF00FFFF) | (value << 16)); break;
				case 0xB7: dma_DST[0] = (uint32_t)((dma_DST[0] & 0x00FFFFFF) | (value << 24)); break;
				case 0xB8: dma_CNT[0] = (uint16_t)((dma_CNT[0] & 0xFF00) | value); break;
				case 0xB9: dma_CNT[0] = (uint16_t)((dma_CNT[0] & 0x00FF) | (value << 8)); break;
				case 0xBA: dma_Update_CTRL((uint16_t)((dma_CTRL[0] & 0xFF00) | value), 0); break;
				case 0xBB: dma_Update_CTRL((uint16_t)((dma_CTRL[0] & 0x00FF) | (value << 8)), 0); break;

				case 0xBC: dma_SRC[1] = (uint32_t)((dma_SRC[1] & 0xFFFFFF00) | value); break;
				case 0xBD: dma_SRC[1] = (uint32_t)((dma_SRC[1] & 0xFFFF00FF) | (value << 8)); break;
				case 0xBE: dma_SRC[1] = (uint32_t)((dma_SRC[1] & 0xFF00FFFF) | (value << 16)); break;
				case 0xBF: dma_SRC[1] = (uint32_t)((dma_SRC[1] & 0x00FFFFFF) | (value << 24)); break;
				case 0xC0: dma_DST[1] = (uint32_t)((dma_DST[1] & 0xFFFFFF00) | value); break;
				case 0xC1: dma_DST[1] = (uint32_t)((dma_DST[1] & 0xFFFF00FF) | (value << 8)); break;
				case 0xC2: dma_DST[1] = (uint32_t)((dma_DST[1] & 0xFF00FFFF) | (value << 16)); break;
				case 0xC3: dma_DST[1] = (uint32_t)((dma_DST[1] & 0x00FFFFFF) | (value << 24)); break;
				case 0xC4: dma_CNT[1] = (uint16_t)((dma_CNT[1] & 0xFF00) | value); break;
				case 0xC5: dma_CNT[1] = (uint16_t)((dma_CNT[1] & 0x00FF) | (value << 8)); break;
				case 0xC6: dma_Update_CTRL((uint16_t)((dma_CTRL[1] & 0xFF00) | value), 1); break;
				case 0xC7: dma_Update_CTRL((uint16_t)((dma_CTRL[1] & 0x00FF) | (value << 8)), 1); break;

				case 0xC8: dma_SRC[2] = (uint32_t)((dma_SRC[2] & 0xFFFFFF00) | value); break;
				case 0xC9: dma_SRC[2] = (uint32_t)((dma_SRC[2] & 0xFFFF00FF) | (value << 8)); break;
				case 0xCA: dma_SRC[2] = (uint32_t)((dma_SRC[2] & 0xFF00FFFF) | (value << 16)); break;
				case 0xCB: dma_SRC[2] = (uint32_t)((dma_SRC[2] & 0x00FFFFFF) | (value << 24)); break;
				case 0xCC: dma_DST[2] = (uint32_t)((dma_DST[2] & 0xFFFFFF00) | value); break;
				case 0xCD: dma_DST[2] = (uint32_t)((dma_DST[2] & 0xFFFF00FF) | (value << 8)); break;
				case 0xCE: dma_DST[2] = (uint32_t)((dma_DST[2] & 0xFF00FFFF) | (value << 16)); break;
				case 0xCF: dma_DST[2] = (uint32_t)((dma_DST[2] & 0x00FFFFFF) | (value << 24)); break;
				case 0xD0: dma_CNT[2] = (uint16_t)((dma_CNT[2] & 0xFF00) | value); break;
				case 0xD1: dma_CNT[2] = (uint16_t)((dma_CNT[2] & 0x00FF) | (value << 8)); break;
				case 0xD2: dma_Update_CTRL((uint16_t)((dma_CTRL[2] & 0xFF00) | value), 2); break;
				case 0xD3: dma_Update_CTRL((uint16_t)((dma_CTRL[2] & 0x00FF) | (value << 8)), 2); break;

				case 0xD4: dma_SRC[3] = (uint32_t)((dma_SRC[3] & 0xFFFFFF00) | value); break;
				case 0xD5: dma_SRC[3] = (uint32_t)((dma_SRC[3] & 0xFFFF00FF) | (value << 8)); break;
				case 0xD6: dma_SRC[3] = (uint32_t)((dma_SRC[3] & 0xFF00FFFF) | (value << 16)); break;
				case 0xD7: dma_SRC[3] = (uint32_t)((dma_SRC[3] & 0x00FFFFFF) | (value << 24)); break;
				case 0xD8: dma_DST[3] = (uint32_t)((dma_DST[3] & 0xFFFFFF00) | value); break;
				case 0xD9: dma_DST[3] = (uint32_t)((dma_DST[3] & 0xFFFF00FF) | (value << 8)); break;
				case 0xDA: dma_DST[3] = (uint32_t)((dma_DST[3] & 0xFF00FFFF) | (value << 16)); break;
				case 0xDB: dma_DST[3] = (uint32_t)((dma_DST[3] & 0x00FFFFFF) | (value << 24)); break;
				case 0xDC: dma_CNT[3] = (uint16_t)((dma_CNT[3] & 0xFF00) | value); break;
				case 0xDD: dma_CNT[3] = (uint16_t)((dma_CNT[3] & 0x00FF) | (value << 8)); break;
				case 0xDE: dma_Update_CTRL((uint16_t)((dma_CTRL[3] & 0xFF00) | value), 3); break;
				case 0xDF: dma_Update_CTRL((uint16_t)((dma_CTRL[3] & 0x00FF) | (value << 8)), 3); break;
			}
		}

		void dma_Write_Reg_16(uint32_t addr, uint16_t value)
		{
			switch (addr)
			{
				case 0xB0: dma_SRC[0] = (uint32_t)((dma_SRC[0] & 0xFFFF0000) | value); break;
				case 0xB2: dma_SRC[0] = (uint32_t)((dma_SRC[0] & 0x0000FFFF) | (value << 16)); break;
				case 0xB4: dma_DST[0] = (uint32_t)((dma_DST[0] & 0xFFFF0000) | value); break;
				case 0xB6: dma_DST[0] = (uint32_t)((dma_DST[0] & 0x0000FFFF) | (value << 16)); break;
				case 0xB8: dma_CNT[0] = value; break;
				case 0xBA: dma_Update_CTRL(value, 0); break;

				case 0xBC: dma_SRC[1] = (uint32_t)((dma_SRC[1] & 0xFFFF0000) | value); break;
				case 0xBE: dma_SRC[1] = (uint32_t)((dma_SRC[1] & 0x0000FFFF) | (value << 16)); break;
				case 0xC0: dma_DST[1] = (uint32_t)((dma_DST[1] & 0xFFFF0000) | value); break;
				case 0xC2: dma_DST[1] = (uint32_t)((dma_DST[1] & 0x0000FFFF) | (value << 16)); break;
				case 0xC4: dma_CNT[1] = value; break;
				case 0xC6: dma_Update_CTRL(value, 1); break;

				case 0xC8: dma_SRC[2] = (uint32_t)((dma_SRC[2] & 0xFFFF0000) | value); break;
				case 0xCA: dma_SRC[2] = (uint32_t)((dma_SRC[2] & 0x0000FFFF) | (value << 16)); break;
				case 0xCC: dma_DST[2] = (uint32_t)((dma_DST[2] & 0xFFFF0000) | value); break;
				case 0xCE: dma_DST[2] = (uint32_t)((dma_DST[2] & 0x0000FFFF) | (value << 16)); break;
				case 0xD0: dma_CNT[2] = value; break;
				case 0xD2: dma_Update_CTRL(value, 2); break;

				case 0xD4: dma_SRC[3] = (uint32_t)((dma_SRC[3] & 0xFFFF0000) | value); break;
				case 0xD6: dma_SRC[3] = (uint32_t)((dma_SRC[3] & 0x0000FFFF) | (value << 16)); break;
				case 0xD8: dma_DST[3] = (uint32_t)((dma_DST[3] & 0xFFFF0000) | value); break;
				case 0xDA: dma_DST[3] = (uint32_t)((dma_DST[3] & 0x0000FFFF) | (value << 16)); break;
				case 0xDC: dma_CNT[3] = value; break;
				case 0xDE: dma_Update_CTRL(value, 3); break;
			}
		}

		void dma_Write_Reg_32(uint32_t addr, uint32_t value)
		{
			switch (addr)
			{
				case 0xB0: dma_SRC[0] = value; break;
				case 0xB4: dma_DST[0] = value; break;
				case 0xB8: dma_CNT[0] = (uint16_t)(value & 0xFFFF);
					dma_Update_CTRL((uint16_t)((value >> 16) & 0xFFFF), 0); break;

				case 0xBC: dma_SRC[1] = value; break;
				case 0xC0: dma_DST[1] = value; break;
				case 0xC4: dma_CNT[1] = (uint16_t)(value & 0xFFFF);
					dma_Update_CTRL((uint16_t)((value >> 16) & 0xFFFF), 1); break;

				case 0xC8: dma_SRC[2] = value; break;
				case 0xCC: dma_DST[2] = value; break;
				case 0xD0: dma_CNT[2] = (uint16_t)(value & 0xFFFF);
					dma_Update_CTRL((uint16_t)((value >> 16) & 0xFFFF), 2); break;

				case 0xD4: dma_SRC[3] = value; break;
				case 0xD8: dma_DST[3] = value; break;
				case 0xDC: dma_CNT[3] = (uint16_t)(value & 0xFFFF);
					dma_Update_CTRL((uint16_t)((value >> 16) & 0xFFFF), 3); break;
			}
		}

		void dma_Update_CTRL(uint16_t value, uint32_t chan)
		{
			if (((dma_CTRL[chan] & 0x8000) == 0) && ((value & 0x8000) != 0))
			{
				dma_SRC_intl[chan] = (uint32_t)(dma_SRC[chan] & dma_SRC_Mask[chan]);

				dma_DST_intl[chan] = (uint32_t)(dma_DST[chan] & dma_DST_Mask[chan]);

				dma_CNT_intl[chan] = (dma_CNT[chan] & dma_CNT_Mask[chan]);

				if (dma_CNT_intl[chan] == 0) { dma_CNT_intl[chan] = dma_CNT_Mask_0[chan]; }

				dma_Access_32[chan] = (value & 0x400) == 0x400;

				if (dma_Access_32[chan])
				{
					if ((value & 0x60) == 0) { dma_DST_INC[chan] = 4; }
					else if ((value & 0x60) == 0x20) { dma_DST_INC[chan] = 0xFFFFFFFC; }
					else if ((value & 0x60) == 0x40) { dma_DST_INC[chan] = 0; }
					else { dma_DST_INC[chan] = 4; }

					if ((value & 0x180) == 0) { dma_SRC_INC[chan] = 4; }
					else if ((value & 0x180) == 0x80) { dma_SRC_INC[chan] = 0xFFFFFFFC; }
					else if ((value & 0x180) == 0x100) { dma_SRC_INC[chan] = 0; }
					else { dma_SRC_INC[chan] = 4; } // Prohibited?
				}
				else
				{
					if ((value & 0x60) == 0) { dma_DST_INC[chan] = 2; }
					else if ((value & 0x60) == 0x20) { dma_DST_INC[chan] = 0xFFFFFFFE; }
					else if ((value & 0x60) == 0x40) { dma_DST_INC[chan] = 0; }
					else { dma_DST_INC[chan] = 2; }

					if ((value & 0x180) == 0) { dma_SRC_INC[chan] = 2; }
					else if ((value & 0x180) == 0x80) { dma_SRC_INC[chan] = 0xFFFFFFFE; }
					else if ((value & 0x180) == 0x100) { dma_SRC_INC[chan] = 0; }
					else { dma_SRC_INC[chan] = 2; } // Prohibited?
				}

				dma_Start_VBL[chan] = dma_Start_HBL[chan] = dma_Run[chan] = false;

				dma_Start_Snd_Vid[chan] = false;
				if ((value & 0x3000) == 0x0000)
				{ 
					dma_Run_En_Time[chan] = 3;
					Misc_Delays = true;
					delays_to_process = true;
					DMA_Start_Delay[chan] = true;
					DMA_Any_Start = true;
				}
				else if ((value & 0x3000) == 0x1000) { dma_Start_VBL[chan] = true; }
				else if ((value & 0x3000) == 0x2000) { dma_Start_HBL[chan] = true; }
				else
				{
					if (chan == 0)
					{
						// Prohibited? What happens?
					}
					else
					{
						dma_Start_Snd_Vid[chan] = true;

						if ((chan == 1) || (chan == 2))
						{
							// ignore word count
							dma_CNT_intl[chan] = 4;

							// for sound FIFO DMA, always DMA to same destination
							dma_DST_INC[chan] = 0;

							// always word size accesses
							dma_Access_32[chan] = true;

							if ((value & 0x180) == 0) { dma_SRC_INC[chan] = 4; }
							else if ((value & 0x180) == 0x80) { dma_SRC_INC[chan] = 0xFFFFFFFC; }
							else if ((value & 0x180) == 0x100) { dma_SRC_INC[chan] = 0; }
							else { dma_SRC_INC[chan] = 4; } // Prohibited? 
						}

						if (chan == 3)
						{
							dma_Video_DMA_Start = false;
							dma_Video_DMA_Delay = true;
						}
					}
				}

				//Console.WriteLine(chan + " " + value);

				dma_Go[chan] = true;
			}

			if ((value & 0x8000) == 0)
			{
				// if the channel isnt currently running, turn it off
				dma_Run[chan] = false;
				dma_Go[chan] = false;

				if (dma_Chan_Exec == 4)
				{
					dma_All_Off = true;

					for (int i = 0; i < 4; i++) { dma_All_Off &= !dma_Run[i]; }

					dma_All_Off &= !dma_Shutdown;
				}
			}

			//if (!dma_All_Off) { Console.WriteLine(dma_Go[0] + " " + dma_Go[1] + " " + dma_Go[2] + " " + dma_Go[3]); }

			if (chan == 3)
			{
				dma_CTRL[chan] = (uint16_t)(value & 0xFFE0);
			}
			else
			{
				dma_CTRL[chan] = (uint16_t)(value & 0xF7E0);
			}
		}

		void dma_Reset()
		{
			for (int i = 0; i < 4; i++)
			{
				dma_CNT_intl[i] = 0;

				dma_Run_En_Time[i] = 0;

				dma_ST_Time[i] = 0;
				dma_ROM_Addr[i] = 0;

				dma_SRC[i] = 0;
				dma_DST[i] = 0;
				dma_SRC_intl[i] = 0;
				dma_DST_intl[i] = 0;
				dma_SRC_INC[i] = 0;
				dma_DST_INC[i] = 0;

				dma_Last_Bus_Value[i] = 0;

				dma_CNT[i] = 0;
				dma_CTRL[i] = 0;

				dma_Read_Cycle[i] = true;
				dma_Go[i] = false;
				dma_Start_VBL[i] = false;
				dma_Start_HBL[i] = false;
				dma_Start_Snd_Vid[i] = false;
				dma_Run[i] = false;
				dma_Access_32[i] = false;
				dma_Use_ROM_Addr_SRC[i] = false;
				dma_Use_ROM_Addr_DST[i] = false;
				dma_Use_ROM_Addr_DST[i] = false;
				dma_ROM_Being_Used[i] = false;

				dma_ROM_Dec_Glitch_Read[i] = false;
				dma_ROM_Dec_Glitch_Write[i] = false;
			}

			dma_Access_Cnt = dma_Access_Wait = 0;

			dma_Chan_Exec = 4;

			dma_TFR_Word = 0;

			dma_TFR_HWord = dma_Held_CPU_Instr = 0;

			dma_Seq_Access = false;
			dma_Pausable = true;
			dma_All_Off = true;
			dma_Shutdown = false;
			dma_Video_DMA_Start = false;
			dma_Video_DMA_Delay = false;
		}

		uint8_t* dma_SaveState(uint8_t* saver)
		{
			saver = bool_saver(dma_Seq_Access, saver);
			saver = bool_saver(dma_Pausable, saver);
			saver = bool_saver(dma_All_Off, saver);
			saver = bool_saver(dma_Shutdown, saver);
			saver = bool_saver(dma_Video_DMA_Start, saver);
			saver = bool_saver(dma_Video_DMA_Delay, saver);

			saver = short_saver(dma_TFR_HWord, saver);
			saver = short_saver(dma_Held_CPU_Instr, saver);

			saver = int_saver(dma_TFR_Word, saver);

			saver = int_saver(dma_Access_Cnt, saver);
			saver = int_saver(dma_Access_Wait, saver);
			saver = int_saver(dma_Chan_Exec, saver);

			saver = bool_array_saver(dma_Read_Cycle, saver, 4);
			saver = bool_array_saver(dma_Go, saver, 4);
			saver = bool_array_saver(dma_Start_VBL, saver, 4);
			saver = bool_array_saver(dma_Start_HBL, saver, 4);
			saver = bool_array_saver(dma_Start_Snd_Vid, saver, 4);
			saver = bool_array_saver(dma_Run, saver, 4);
			saver = bool_array_saver(dma_Access_32, saver, 4);
			saver = bool_array_saver(dma_Use_ROM_Addr_SRC, saver, 4);
			saver = bool_array_saver(dma_Use_ROM_Addr_DST, saver, 4);
			saver = bool_array_saver(dma_ROM_Being_Used, saver, 4);
			saver = bool_array_saver(dma_ROM_Dec_Glitch_Read, saver, 4);
			saver = bool_array_saver(dma_ROM_Dec_Glitch_Write, saver, 4);

			saver = short_array_saver(dma_CNT, saver, 4);
			saver = short_array_saver(dma_CTRL, saver, 4);

			saver = int_array_saver(dma_SRC, saver, 4);
			saver = int_array_saver(dma_DST, saver, 4);
			saver = int_array_saver(dma_SRC_intl, saver, 4);
			saver = int_array_saver(dma_DST_intl, saver, 4);
			saver = int_array_saver(dma_SRC_INC, saver, 4);
			saver = int_array_saver(dma_DST_INC, saver, 4);
			saver = int_array_saver(dma_Last_Bus_Value, saver, 4);

			saver = int_array_saver(dma_CNT_intl, saver, 4);
			saver = int_array_saver(dma_ST_Time, saver, 4);
			saver = int_array_saver(dma_ROM_Addr, saver, 4);
			saver = int_array_saver(dma_Run_En_Time, saver, 4);

			return saver;
		}

		uint8_t* dma_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&dma_Seq_Access, loader);
			loader = bool_loader(&dma_Pausable, loader);
			loader = bool_loader(&dma_All_Off, loader);
			loader = bool_loader(&dma_Shutdown, loader);
			loader = bool_loader(&dma_Video_DMA_Start, loader);
			loader = bool_loader(&dma_Video_DMA_Delay, loader);

			loader = short_loader(&dma_TFR_HWord, loader);
			loader = short_loader(&dma_Held_CPU_Instr, loader);

			loader = int_loader(&dma_TFR_Word, loader);

			loader = int_loader(&dma_Access_Cnt, loader);
			loader = int_loader(&dma_Access_Wait, loader);
			loader = int_loader(&dma_Chan_Exec, loader);

			loader = bool_array_loader(dma_Read_Cycle, loader, 4);
			loader = bool_array_loader(dma_Go, loader, 4);
			loader = bool_array_loader(dma_Start_VBL, loader, 4);
			loader = bool_array_loader(dma_Start_HBL, loader, 4);
			loader = bool_array_loader(dma_Start_Snd_Vid, loader, 4);
			loader = bool_array_loader(dma_Run, loader, 4);
			loader = bool_array_loader(dma_Access_32, loader, 4);
			loader = bool_array_loader(dma_Use_ROM_Addr_SRC, loader, 4);
			loader = bool_array_loader(dma_Use_ROM_Addr_DST, loader, 4);
			loader = bool_array_loader(dma_ROM_Being_Used, loader, 4);
			loader = bool_array_loader(dma_ROM_Dec_Glitch_Read, loader, 4);
			loader = bool_array_loader(dma_ROM_Dec_Glitch_Write, loader, 4);

			loader = short_array_loader(dma_CNT, loader, 4);
			loader = short_array_loader(dma_CTRL, loader, 4);

			loader = int_array_loader(dma_SRC, loader, 4);
			loader = int_array_loader(dma_DST, loader, 4);
			loader = int_array_loader(dma_SRC_intl, loader, 4);
			loader = int_array_loader(dma_DST_intl, loader, 4);
			loader = int_array_loader(dma_SRC_INC, loader, 4);
			loader = int_array_loader(dma_DST_INC, loader, 4);
			loader = int_array_loader(dma_Last_Bus_Value, loader, 4);

			loader = int_array_loader(dma_CNT_intl, loader, 4);
			loader = int_array_loader(dma_ST_Time, loader, 4);
			loader = int_array_loader(dma_ROM_Addr, loader, 4);
			loader = int_array_loader(dma_Run_En_Time, loader, 4);

			return loader;
		}

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

							if ((REG_FFFF & 0x8) == 0x8) { cpu_FlagIset(true); }
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
					if (CycleCount >= tim_Next_Free_Cycle)
					{
						tim_Timer_Old = tim_Timer;
						tim_Timer = value;
						tim_Reload_Block = true;
					}
					break;

					// TMA (Timer Modulo)
				case 0xFF06:
					tim_Reload = value;
					if (CycleCount < tim_Next_Free_Cycle)
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

		void tick()
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

					tim_Next_Free_Cycle = 4 + CycleCount;

					// set interrupts
					if ((REG_FFFF & 4) == 4) { cpu_FlagIset(true); }
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

	#pragma region PPU

		uint32_t ppu_OBJ_Sizes_X[16] = {8, 16, 8, 8,
										16, 32, 8, 8,
										32, 32, 16, 8,
										64, 64, 32, 8 };

		uint32_t ppu_OBJ_Sizes_Y[16] = {8, 8, 16, 8,
										16, 8, 32, 8,
										32, 16, 32, 8,
										64, 32, 64, 8 };

		bool ppu_In_VBlank;
		bool ppu_Delays, ppu_Latch_Delays;
		bool ppu_Sprite_Delay_Disp;
		bool ppu_Do_Green_Swap;

		bool ppu_VRAM_In_Use, ppu_VRAM_High_In_Use, ppu_PALRAM_In_Use;

		bool ppu_VRAM_High_Access;
		bool ppu_VRAM_Access;
		bool ppu_PALRAM_Access;
		bool ppu_OAM_Access;
		bool ppu_Continue_Fetch_OAM;

		bool ppu_HBL_Free, ppu_OBJ_Dim, ppu_Forced_Blank, ppu_Any_Window_On;
		bool ppu_OBJ_On, ppu_WIN0_On, ppu_WIN1_On, ppu_OBJ_WIN, ppu_OBJ_On_Disp;
		bool ppu_WIN0_Active, ppu_WIN1_Active;

		bool ppu_Ctrl_Latch_Delay;

		uint8_t ppu_STAT, ppu_LY, ppu_LYC;

		uint16_t ppu_CTRL, ppu_Green_Swap, ppu_Cycle, ppu_Display_Cycle;
		uint16_t ppu_Sprite_Eval_Time_OAM, ppu_Sprite_Eval_Time_VRAM;
		uint16_t ppu_WIN_Hor_0, ppu_WIN_Hor_1, ppu_WIN_Vert_0, ppu_WIN_Vert_1;
		uint16_t ppu_WIN_In, ppu_WIN_Out, ppu_Mosaic, ppu_Special_FX, ppu_Alpha, ppu_Bright;

		uint32_t ppu_BG_Mode, ppu_Display_Frame;
		
		uint32_t ppu_X_RS, ppu_Y_RS;

		uint32_t ppu_VBL_IRQ_cd, ppu_HBL_IRQ_cd, ppu_LYC_IRQ_cd, ppu_Sprite_Disp_cd;

		uint32_t ppu_Ctrl_Latch_cd;

		uint32_t ppu_Forced_Blank_Time;
		uint32_t ppu_OBJ_On_Time;

		uint16_t ppu_BG_CTRL[4] = { };
		uint16_t ppu_BG_X[4] = { };
		uint16_t ppu_BG_Y[4] = { };
		uint16_t ppu_BG_X_Latch[4] = { };
		uint16_t ppu_BG_Y_Latch[4] = { };

		uint16_t ppu_BG_Rot_A[4] = { };
		uint16_t ppu_BG_Rot_B[4] = { };
		uint16_t ppu_BG_Rot_C[4] = { };
		uint16_t ppu_BG_Rot_D[4] = { };

		uint16_t ppu_ROT_REF_LY[4] = { };

		uint32_t ppu_BG_Ref_X[4] = { };
		uint32_t ppu_BG_Ref_Y[4] = { };

		uint32_t ppu_BG_X_Latch_cd[4] = { };

		bool ppu_BG_On[4] = { };
		bool ppu_BG_On_Disp[4] = { };
		bool ppu_BG_On_Latch[4] = { };
		bool ppu_BG_On_Latch_2[4] = { };
		bool ppu_BG_Ref_X_Change[4] = { };
		bool ppu_BG_Ref_LY_Change[4] = { };
		bool ppu_BG_X_Latch_Delays[4] = { };
		bool ppu_BG_Line_End[4] = { };
		bool ppu_BG_Line_End_BGS5[4] = { };

		// Sprite Evaluation
		bool ppu_Rot_Scale;
		bool ppu_Rot_Scale_Temp;
		bool ppu_Fetch_OAM_0, ppu_Fetch_OAM_2, ppu_Fetch_OAM_A_D;
		bool ppu_Fetch_Sprite_VRAM;
		bool ppu_New_Sprite, ppu_Sprite_Eval_Finished;
		bool ppu_Sprite_Mosaic;

		uint8_t ppu_Sprite_LY_Check;

		uint16_t ppu_Sprite_Attr_0, ppu_Sprite_Attr_1, ppu_Sprite_Attr_2;
		uint16_t ppu_Sprite_Attr_0_Temp, ppu_Sprite_Attr_1_Temp;

		uint16_t ppu_Sprite_A_Latch, ppu_Sprite_B_Latch, ppu_Sprite_C_Latch, ppu_Sprite_D_Latch;

		uint32_t ppu_Cur_Sprite_X;
		uint32_t ppu_Cur_Sprite_Y;
		uint32_t ppu_Cur_Sprite_Y_Temp;

		uint32_t ppu_Current_Sprite;
		uint32_t ppu_Process_Sprite;
		uint32_t ppu_Process_Sprite_Temp;
		uint32_t ppu_Sprite_ofst_eval;
		uint32_t ppu_Sprite_ofst_draw;
		uint32_t ppu_Sprite_X_Pos, ppu_Sprite_Y_Pos;
		uint32_t ppu_Sprite_X_Pos_Temp, ppu_Sprite_Y_Pos_Temp;
		uint32_t ppu_Sprite_X_Size, ppu_Sprite_Y_Size;
		uint32_t ppu_Sprite_X_Size_Temp, ppu_Sprite_Y_Size_Temp;
		uint32_t ppu_Sprite_Render_Cycle;
		uint32_t ppu_Fetch_OAM_A_D_Cnt;
		uint32_t ppu_Fetch_Sprite_VRAM_Cnt;
		uint32_t ppu_Sprite_VRAM_Mod;
		uint32_t ppu_Sprite_X_Scale;
		uint32_t ppu_Sprite_Size_X_Ofst;
		uint32_t ppu_Sprite_Size_Y_Ofst;
		uint32_t ppu_Sprite_Size_X_Ofst_Temp;
		uint32_t ppu_Sprite_Size_Y_Ofst_Temp;
		uint32_t ppu_Sprite_Mode;
		uint32_t ppu_Sprite_Next_Fetch;
		uint32_t ppu_Param_Pick;
		uint32_t ppu_Sprite_Mosaic_Y_Counter;
		uint32_t ppu_Sprite_Mosaic_Y_Compare;

		bool ppu_Sprite_Pixel_Occupied[240 * 2] = { };
		bool ppu_Sprite_Semi_Transparent[240 * 2] = { };
		bool ppu_Sprite_Object_Window[240 * 2] = { };
		bool ppu_Sprite_Is_Mosaic[240 * 2] = { };

		uint32_t ppu_Sprite_Pixels[240 * 2] = { };
		uint32_t ppu_Sprite_Priority[240 * 2] = { };

		// latched sprite pixel parameters
		uint32_t ppu_Sprite_Pixel_Latch;
		uint32_t ppu_Sprite_Priority_Latch;

		bool ppu_Sprite_Semi_Transparent_Latch;
		bool ppu_Sprite_Mosaic_Latch;
		bool ppu_Sprite_Pixel_Occupied_Latch;

		// BG rendering
		uint64_t ppu_Current_Ref_X_2, ppu_Current_Ref_Y_2;
		uint64_t ppu_Current_Ref_X_3, ppu_Current_Ref_Y_3;

		uint64_t ppu_F_Rot_A_2, ppu_F_Rot_B_2, ppu_F_Rot_C_2, ppu_F_Rot_D_2;
		uint64_t ppu_F_Rot_A_3, ppu_F_Rot_B_3, ppu_F_Rot_C_3, ppu_F_Rot_D_3;

		uint32_t ppu_Base_LY_2, ppu_Base_LY_3;
		
		uint32_t ppu_Fetch_Count[4] = { };
		uint32_t ppu_Scroll_Cycle[4] = { };
		uint32_t ppu_Pixel_Color[4] = { };
		uint32_t ppu_Pixel_Color_2[4] = { };
		uint32_t ppu_Pixel_Color_1[4] = { };
		uint32_t ppu_Pixel_Color_M[4] = { };
		uint32_t ppu_Pixel_Color_R[4] = { };
		uint32_t ppu_Tile_Addr[4] = { };
		uint32_t ppu_Y_Flip_Ofst[4] = { };

		uint16_t ppu_BG_Start_Time[4] = { };

		uint8_t ppu_BG_Effect_Byte[4] = { };
		uint8_t ppu_BG_Effect_Byte_New[4] = { };

		bool ppu_BG_Rendering_Complete[4] = { };
		bool ppu_BG_Has_Pixel[4] = { };
		bool ppu_BG_Has_Pixel_2[4] = { };
		bool ppu_BG_Has_Pixel_1[4] = { };
		bool ppu_BG_Has_Pixel_M[4] = { };
		bool ppu_BG_Has_Pixel_R[4] = { };

		uint32_t ppu_BG_Pixel_F;
		uint32_t ppu_BG_Pixel_S;
		uint32_t ppu_Final_Pixel;
		uint32_t ppu_Blend_Pixel;

		uint16_t ppu_BG_Mosaic_X_Mod;

		bool ppu_Brighten_Final_Pixel;
		bool ppu_Blend_Final_Pixel;

		// Palette fetches for BG's are true unless they happen from mode 3 or 5, where the color is directly encoded in VRAM
		// When this occurs, the first BG pixel will always be from BG 2, and the second from backdrop
		bool ppu_Fetch_BG;

		bool ppu_Fetch_Target_1;
		bool ppu_Fetch_Target_2;

		bool ppu_Rendering_Complete;
		bool ppu_PAL_Rendering_Complete;

		uint16_t ppu_VRAM_Open_Bus;

		// Derived values, not stated, reloaded with savestate
		uint32_t BG_Scale_X[4] = { };
		uint32_t BG_Scale_Y[4] = { };
		uint32_t BG_Num_Tiles[4] = { };
		uint32_t ppu_BG_Char_Base[4] = { };
		uint32_t ppu_BG_Screen_Base[4] = { };

		uint16_t ppu_ROT_OBJ_X[128] = { };
		uint16_t ppu_ROT_OBJ_Y[128] = { };

		uint16_t ppu_MOS_OBJ_X[0x200] = { };

		uint16_t ppu_MOS_BG_Y[0x200] = { };

		uint8_t ppu_BG_Priority[4] = { };
		uint8_t ppu_BG_Screen_Size[4] = { };

		bool ppu_BG_Mosaic[4] = { };
		bool ppu_BG_Pal_Size[4] = { };
		bool ppu_BG_Overflow[4] = { };
		bool ppu_WIN0_BG_En[4] = { };
		bool ppu_WIN1_BG_En[4] = { };
		bool ppu_OBJ_BG_En[4] = { };
		bool ppu_OUT_BG_En[4] = { };

		uint32_t ppu_SFX_mode, ppu_SFX_BRT_Num;
		uint32_t ppu_SFX_Alpha_Num_1, ppu_SFX_Alpha_Num_2;

		uint16_t ppu_OBJ_Mosaic_X, ppu_OBJ_Mosaic_Y;
		uint16_t ppu_BG_Mosaic_X, ppu_BG_Mosaic_Y;

		uint8_t ppu_WIN0_Left, ppu_WIN1_Left, ppu_WIN0_Right, ppu_WIN1_Right;
		uint8_t ppu_WIN0_Top, ppu_WIN1_Top, ppu_WIN0_Bot, ppu_WIN1_Bot;

		bool ppu_WIN0_OBJ_En, ppu_WIN1_OBJ_En, ppu_OUT_OBJ_En, ppu_OBJ_OBJ_En;
		bool ppu_WIN0_Color_En, ppu_WIN1_Color_En, ppu_OUT_Color_En, ppu_OBJ_Color_En;

		bool ppu_SFX_OBJ_Target_1, ppu_SFX_OBJ_Target_2;

		// volatile variables used every cycle in rendering, not stated
		bool BG_Go[4] = { };
		bool BG_Go_Disp[4] = { };

		void (*ScanlineCallback)(void);
		int Callback_Scanline;

		void ppu_Reset();

		uint8_t ppu_Read_Reg_8(uint32_t addr)
		{
			uint8_t ret = 0;

			switch (addr)
			{
				case 0x00: ret = (uint8_t)(ppu_CTRL & 0xFF); break;
				case 0x01: ret = (uint8_t)((ppu_CTRL & 0xFF00) >> 8); break;
				case 0x02: ret = (uint8_t)(ppu_Green_Swap & 0xFF); break;
				case 0x03: ret = (uint8_t)((ppu_Green_Swap & 0xFF00) >> 8); break;
				case 0x04: ret = ppu_STAT; break;
				case 0x05: ret = ppu_LYC; break;
				case 0x06: ret = ppu_LY; break;
				case 0x07: ret = 0; break;

				case 0x08: ret = (uint8_t)(ppu_BG_CTRL[0] & 0xFF); break;
				case 0x09: ret = (uint8_t)((ppu_BG_CTRL[0] & 0xFF00) >> 8); break;
				case 0x0A: ret = (uint8_t)(ppu_BG_CTRL[1] & 0xFF); break;
				case 0x0B: ret = (uint8_t)((ppu_BG_CTRL[1] & 0xFF00) >> 8); break;
				case 0x0C: ret = (uint8_t)(ppu_BG_CTRL[2] & 0xFF); break;
				case 0x0D: ret = (uint8_t)((ppu_BG_CTRL[2] & 0xFF00) >> 8); break;
				case 0x0E: ret = (uint8_t)(ppu_BG_CTRL[3] & 0xFF); break;
				case 0x0F: ret = (uint8_t)((ppu_BG_CTRL[3] & 0xFF00) >> 8); break;

				case 0x48: ret = (uint8_t)(ppu_WIN_In & 0xFF); break;
				case 0x49: ret = (uint8_t)((ppu_WIN_In & 0xFF00) >> 8); break;
				case 0x4A: ret = (uint8_t)(ppu_WIN_Out & 0xFF); break;
				case 0x4B: ret = (uint8_t)((ppu_WIN_Out & 0xFF00) >> 8); break;

				case 0x50: ret = (uint8_t)(ppu_Special_FX & 0xFF); break;
				case 0x51: ret = (uint8_t)((ppu_Special_FX & 0xFF00) >> 8); break;
				case 0x52: ret = (uint8_t)(ppu_Alpha & 0xFF); break;
				case 0x53: ret = (uint8_t)((ppu_Alpha & 0xFF00) >> 8); break;

				default: ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (uint32_t)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		uint16_t ppu_Read_Reg_16(uint32_t addr)
		{
			uint16_t ret = 0;

			switch (addr)
			{
				case 0x00: ret = ppu_CTRL; break;
				case 0x02: ret = ppu_Green_Swap; break;
				case 0x04: ret = (uint16_t)((ppu_LYC << 8) | ppu_STAT); break;
				case 0x06: ret = ppu_LY; break;

				case 0x08: ret = ppu_BG_CTRL[0]; break;
				case 0x0A: ret = ppu_BG_CTRL[1]; break;
				case 0x0C: ret = ppu_BG_CTRL[2]; break;
				case 0x0E: ret = ppu_BG_CTRL[3]; break;

				case 0x48: ret = ppu_WIN_In; break;
				case 0x4A: ret = ppu_WIN_Out; break;

				case 0x50: ret = ppu_Special_FX; break;
				case 0x52: ret = ppu_Alpha; break;

				default: ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		uint32_t ppu_Read_Reg_32(uint32_t addr)
		{
			uint32_t ret = 0;

			switch (addr)
			{
				case 0x00: ret = (uint32_t)((ppu_Green_Swap << 16) | ppu_CTRL); break;
				case 0x04: ret = (uint32_t)((ppu_LY << 16) | (ppu_LYC << 8) | ppu_STAT); break;

				case 0x08: ret = (uint32_t)((ppu_BG_CTRL[1] << 16) | ppu_BG_CTRL[0]); break;
				case 0x0C: ret = (uint32_t)((ppu_BG_CTRL[3] << 16) | ppu_BG_CTRL[2]); break;

				case 0x48: ret = (uint32_t)((ppu_WIN_Out << 16) | ppu_WIN_In); break;

				case 0x50: ret = (uint32_t)((ppu_Alpha << 16) | ppu_Special_FX); break;

				default: ret = cpu_Last_Bus_Value; break;
			}

			return ret;
		}

		void ppu_Write_Reg_8(uint32_t addr, uint8_t value)
		{
			//Message_String = "wr 8 addr: " + to_string(addr) + " v: " + to_string(value) + " ly: " + to_string(ppu_LY) + " cyc: " + to_string(ppu_Cycle);

			//MessageCallback(Message_String.length());
			
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write((uint16_t)((ppu_CTRL & 0xFF00) | value)); break;
				case 0x01: ppu_CTRL_Write((uint16_t)((ppu_CTRL & 0x00FF) | (value << 8))); break;
				case 0x02: ppu_Green_Swap = (uint16_t)((ppu_Green_Swap & 0xFF00) | value); ppu_Do_Green_Swap = (ppu_Green_Swap & 1) == 1; break;
				case 0x03: ppu_Green_Swap = (uint16_t)((ppu_Green_Swap & 0x00FF) | (value << 8)); break;
				case 0x04: ppu_STAT_Write(value); break;
				case 0x05: ppu_Update_LYC(value); break;
				case 0x06: // No Effect on LY
				case 0x07: // No Effect on LY
				case 0x08: ppu_BG_CTRL[0] = (uint16_t)((ppu_BG_CTRL[0] & 0xDF00) | value); ppu_BG_CTRL_Write(0); break;
				case 0x09: ppu_BG_CTRL[0] = (uint16_t)((ppu_BG_CTRL[0] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(0); break;
				case 0x0A: ppu_BG_CTRL[1] = (uint16_t)((ppu_BG_CTRL[1] & 0xDF00) | value); ppu_BG_CTRL_Write(1); break;
				case 0x0B: ppu_BG_CTRL[1] = (uint16_t)((ppu_BG_CTRL[1] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = (uint16_t)((ppu_BG_CTRL[2] & 0xFF00) | value); ppu_BG_CTRL_Write(2); break;
				case 0x0D: ppu_BG_CTRL[2] = (uint16_t)((ppu_BG_CTRL[2] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(2); break;
				case 0x0E: ppu_BG_CTRL[3] = (uint16_t)((ppu_BG_CTRL[3] & 0xFF00) | value); ppu_BG_CTRL_Write(3); break;
				case 0x0F: ppu_BG_CTRL[3] = (uint16_t)((ppu_BG_CTRL[3] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X_Write((uint16_t)((ppu_BG_X[0] & 0xFF00) | value), 0); break;
				case 0x11: ppu_BG_X_Write((uint16_t)((ppu_BG_X[0] & 0x00FF) | (value << 8)), 0); break;
				case 0x12: ppu_BG_Y[0] = (uint16_t)((ppu_BG_Y[0] & 0xFF00) | value); break;
				case 0x13: ppu_BG_Y[0] = (uint16_t)((ppu_BG_Y[0] & 0x00FF) | (value << 8)); break;
				case 0x14: ppu_BG_X_Write((uint16_t)((ppu_BG_X[1] & 0xFF00) | value), 1); break;
				case 0x15: ppu_BG_X_Write((uint16_t)((ppu_BG_X[1] & 0x00FF) | (value << 8)), 1); break;
				case 0x16: ppu_BG_Y[1] = (uint16_t)((ppu_BG_Y[1] & 0xFF00) | value); break;
				case 0x17: ppu_BG_Y[1] = (uint16_t)((ppu_BG_Y[1] & 0x00FF) | (value << 8)); break;
				case 0x18: ppu_BG_X_Write((uint16_t)((ppu_BG_X[2] & 0xFF00) | value), 2); break;
				case 0x19: ppu_BG_X_Write((uint16_t)((ppu_BG_X[2] & 0x00FF) | (value << 8)), 2); break;
				case 0x1A: ppu_BG_Y[2] = (uint16_t)((ppu_BG_Y[2] & 0xFF00) | value); break;
				case 0x1B: ppu_BG_Y[2] = (uint16_t)((ppu_BG_Y[2] & 0x00FF) | (value << 8)); break;
				case 0x1C: ppu_BG_X_Write((uint16_t)((ppu_BG_X[3] & 0xFF00) | value), 3); break;
				case 0x1D: ppu_BG_X_Write((uint16_t)((ppu_BG_X[3] & 0x00FF) | (value << 8)), 3); break;
				case 0x1E: ppu_BG_Y[3] = (uint16_t)((ppu_BG_Y[3] & 0xFF00) | value); break;
				case 0x1F: ppu_BG_Y[3] = (uint16_t)((ppu_BG_Y[3] & 0x00FF) | (value << 8)); break;

				case 0x20: ppu_BG_Rot_A[2] = (uint16_t)((ppu_BG_Rot_A[2] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x21: ppu_BG_Rot_A[2] = (uint16_t)((ppu_BG_Rot_A[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x22: ppu_BG_Rot_B[2] = (uint16_t)((ppu_BG_Rot_B[2] & 0xFF00) | value); break;
				case 0x23: ppu_BG_Rot_B[2] = (uint16_t)((ppu_BG_Rot_B[2] & 0x00FF) | (value << 8)); break;
				case 0x24: ppu_BG_Rot_C[2] = (uint16_t)((ppu_BG_Rot_C[2] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x25: ppu_BG_Rot_C[2] = (uint16_t)((ppu_BG_Rot_C[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x26: ppu_BG_Rot_D[2] = (uint16_t)((ppu_BG_Rot_D[2] & 0xFF00) | value); break;
				case 0x27: ppu_BG_Rot_D[2] = (uint16_t)((ppu_BG_Rot_D[2] & 0x00FF) | (value << 8)); break;
				case 0x28: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFFFFFF00) | value); ppu_ROT_REF_X_Update(2); break;
				case 0x29: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_X_Update(2); break;
				case 0x2A: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_X_Update(2); break;
				case 0x2B: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_X_Update(2); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFFFFFF00) | value); ppu_ROT_REF_LY_Update(2); break;
				case 0x2D: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_LY_Update(2); break;
				case 0x2E: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(2); break;
				case 0x2F: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = (uint16_t)((ppu_BG_Rot_A[3] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x31: ppu_BG_Rot_A[3] = (uint16_t)((ppu_BG_Rot_A[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x32: ppu_BG_Rot_B[3] = (uint16_t)((ppu_BG_Rot_B[3] & 0xFF00) | value); break;
				case 0x33: ppu_BG_Rot_B[3] = (uint16_t)((ppu_BG_Rot_B[3] & 0x00FF) | (value << 8)); break;
				case 0x34: ppu_BG_Rot_C[3] = (uint16_t)((ppu_BG_Rot_C[3] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x35: ppu_BG_Rot_C[3] = (uint16_t)((ppu_BG_Rot_C[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x36: ppu_BG_Rot_D[3] = (uint16_t)((ppu_BG_Rot_D[3] & 0xFF00) | value); break;
				case 0x37: ppu_BG_Rot_D[3] = (uint16_t)((ppu_BG_Rot_D[3] & 0x00FF) | (value << 8)); break;
				case 0x38: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFFFFFF00) | value); ppu_ROT_REF_X_Update(3); break;
				case 0x39: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_X_Update(3); break;
				case 0x3A: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_X_Update(3); break;
				case 0x3B: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_X_Update(3); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFFFFFF00) | value); ppu_ROT_REF_LY_Update(3); break;
				case 0x3D: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_LY_Update(3); break;
				case 0x3E: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(3); break;
				case 0x3F: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_LY_Update(3); break;

				case 0x40: ppu_WIN_Hor_0 = (uint16_t)((ppu_WIN_Hor_0 & 0xFF00) | value); ppu_Calc_Win0(); break;
				case 0x41: ppu_WIN_Hor_0 = (uint16_t)((ppu_WIN_Hor_0 & 0x00FF) | (value << 8)); ppu_Calc_Win0(); break;
				case 0x42: ppu_WIN_Hor_1 = (uint16_t)((ppu_WIN_Hor_1 & 0xFF00) | value); ppu_Calc_Win1(); break;
				case 0x43: ppu_WIN_Hor_1 = (uint16_t)((ppu_WIN_Hor_1 & 0x00FF) | (value << 8)); ppu_Calc_Win1(); break;
				case 0x44: ppu_WIN_Vert_0 = (uint16_t)((ppu_WIN_Vert_0 & 0xFF00) | value); ppu_Calc_Win0(); break;
				case 0x45: ppu_WIN_Vert_0 = (uint16_t)((ppu_WIN_Vert_0 & 0x00FF) | (value << 8)); ppu_Calc_Win0(); break;
				case 0x46: ppu_WIN_Vert_1 = (uint16_t)((ppu_WIN_Vert_1 & 0xFF00) | value); ppu_Calc_Win1(); break;
				case 0x47: ppu_WIN_Vert_1 = (uint16_t)((ppu_WIN_Vert_1 & 0x00FF) | (value << 8)); ppu_Calc_Win1(); break;
				case 0x48: ppu_Update_Win_In((uint16_t)((ppu_WIN_In & 0xFF00) | value)); break;
				case 0x49: ppu_Update_Win_In((uint16_t)((ppu_WIN_In & 0x00FF) | (value << 8))); break;
				case 0x4A: ppu_Update_Win_Out((uint16_t)((ppu_WIN_Out & 0xFF00) | value)); break;
				case 0x4B: ppu_Update_Win_Out((uint16_t)((ppu_WIN_Out & 0x00FF) | (value << 8))); break;
				case 0x4C: ppu_Update_Mosaic((uint16_t)((ppu_Mosaic & 0xFF00) | value)); break;
				case 0x4D: ppu_Update_Mosaic((uint16_t)((ppu_Mosaic & 0x00FF) | (value << 8))); break;

				case 0x50: ppu_Update_Special_FX((uint16_t)((ppu_Special_FX & 0xFF00) | value)); break;
				case 0x51: ppu_Update_Special_FX((uint16_t)((ppu_Special_FX & 0x00FF) | (value << 8))); break;
				case 0x52: ppu_Update_Alpha((uint16_t)((ppu_Alpha & 0xFF00) | value)); break;
				case 0x53: ppu_Update_Alpha((uint16_t)((ppu_Alpha & 0x00FF) | (value << 8))); break;
				case 0x54: ppu_Update_Bright((uint16_t)((ppu_Bright & 0xFF00) | value)); break;
				case 0x55: ppu_Update_Bright((uint16_t)((ppu_Bright & 0x00FF) | (value << 8))); break;
			}
		}

		void ppu_Write_Reg_16(uint32_t addr, uint16_t value)
		{
			//Message_String = "wr 16 addr: " + to_string(addr) + " v: " + to_string(value) + " ly: " + to_string(ppu_LY) + " cyc: " + to_string(ppu_Cycle);

			//MessageCallback(Message_String.length());
			
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write(value); break;
				case 0x02: ppu_Green_Swap = value; ppu_Do_Green_Swap = (ppu_Green_Swap & 1) == 1; break;
				case 0x04: ppu_STAT_Write((uint8_t)value); ppu_Update_LYC((uint8_t)(value >> 8)); break;
				case 0x06: // No Effect on LY
				case 0x08: ppu_BG_CTRL[0] = (uint16_t)(value & 0xDFFF); ppu_BG_CTRL_Write(0); break;
				case 0x0A: ppu_BG_CTRL[1] = (uint16_t)(value & 0xDFFF); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = value; ppu_BG_CTRL_Write(2); break;
				case 0x0E: ppu_BG_CTRL[3] = value; ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X_Write(value, 0); break;
				case 0x12: ppu_BG_Y[0] = value; break;
				case 0x14: ppu_BG_X_Write(value, 1); break;
				case 0x16: ppu_BG_Y[1] = value; break;
				case 0x18: ppu_BG_X_Write(value, 2); break;
				case 0x1A: ppu_BG_Y[2] = value; break;
				case 0x1C: ppu_BG_X_Write(value, 3); break;
				case 0x1E: ppu_BG_Y[3] = value; break;

				case 0x20: ppu_BG_Rot_A[2] = value; ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x22: ppu_BG_Rot_B[2] = value; break;
				case 0x24: ppu_BG_Rot_C[2] = value; ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x26: ppu_BG_Rot_D[2] = value; break;
				case 0x28: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFFFF0000) | value); ppu_ROT_REF_X_Update(2); break;
				case 0x2A: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_X_Update(2); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFFFF0000) | value); ppu_ROT_REF_LY_Update(2); break;
				case 0x2E: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = value; ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x32: ppu_BG_Rot_B[3] = value; break;
				case 0x34: ppu_BG_Rot_C[3] = value; ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x36: ppu_BG_Rot_D[3] = value; break;
				case 0x38: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFFFF0000) | value); ppu_ROT_REF_X_Update(3); break;
				case 0x3A: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_X_Update(3); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFFFF0000) | value); ppu_ROT_REF_LY_Update(3); break;
				case 0x3E: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_LY_Update(3); break;

				case 0x40: ppu_WIN_Hor_0 = value; ppu_Calc_Win0(); break;
				case 0x42: ppu_WIN_Hor_1 = value; ppu_Calc_Win1(); break;
				case 0x44: ppu_WIN_Vert_0 = value; ppu_Calc_Win0(); break;
				case 0x46: ppu_WIN_Vert_1 = value; ppu_Calc_Win1(); break;
				case 0x48: ppu_Update_Win_In(value); break;
				case 0x4A: ppu_Update_Win_Out(value); break;
				case 0x4C: ppu_Update_Mosaic(value); break;

				case 0x50: ppu_Update_Special_FX(value); break;
				case 0x52: ppu_Update_Alpha(value); break;
				case 0x54: ppu_Update_Bright(value); break;
			}
		}

		void ppu_Write_Reg_32(uint32_t addr, uint32_t value)
		{
			//Message_String = "wr 32 addr: " + to_string(addr) + " v: " + to_string(value) + " ly: " + to_string(ppu_LY) + " cyc: " + to_string(ppu_Cycle);

			//MessageCallback(Message_String.length());
			
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write((uint16_t)(value & 0xFFFF));
						   ppu_Green_Swap = (uint16_t)((value >> 16) & 0xFFFF); ppu_Do_Green_Swap = (ppu_Green_Swap & 1) == 1; break;
				case 0x04: ppu_STAT_Write((uint8_t)value); ppu_Update_LYC((uint8_t)(value >> 8)); break;
						   /* no effect on LY*/
				case 0x08: ppu_BG_CTRL[0] = (uint16_t)(value & 0xDFFF); ppu_BG_CTRL_Write(0);
						   ppu_BG_CTRL[1] = (uint16_t)((value >> 16) & 0xDFFF); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = (uint16_t)(value & 0xFFFF); ppu_BG_CTRL_Write(2);
						   ppu_BG_CTRL[3] = (uint16_t)((value >> 16) & 0xFFFF); ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X_Write((uint16_t)(value & 0xFFFF), 0);
						   ppu_BG_Y[0] = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x14: ppu_BG_X_Write((uint16_t)(value & 0xFFFF), 1);
						   ppu_BG_Y[1] = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x18: ppu_BG_X_Write((uint16_t)(value & 0xFFFF), 2);
						   ppu_BG_Y[2] = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x1C: ppu_BG_X_Write((uint16_t)(value & 0xFFFF), 3);
						   ppu_BG_Y[3] = (uint16_t)((value >> 16) & 0xFFFF); break;

				case 0x20: ppu_BG_Rot_A[2] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_B[2] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x24: ppu_BG_Rot_C[2] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_D[2] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x28: ppu_BG_Ref_X[2] = (value & 0xFFFFFFF); ppu_ROT_REF_X_Update(2); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (value & 0xFFFFFFF); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_B[3] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x34: ppu_BG_Rot_C[3] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_D[3] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x38: ppu_BG_Ref_X[3] = (value & 0xFFFFFFF); ppu_ROT_REF_X_Update(3); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (value & 0xFFFFFFF); ppu_ROT_REF_LY_Update(3); break;

				case 0x40: ppu_WIN_Hor_0 = (uint16_t)(value & 0xFFFF);
						   ppu_WIN_Hor_1 = (uint16_t)((value >> 16) & 0xFFFF); ppu_Calc_Win0(); ppu_Calc_Win1(); break;
				case 0x44: ppu_WIN_Vert_0 = (uint16_t)(value & 0xFFFF);
						   ppu_WIN_Vert_1 = (uint16_t)((value >> 16) & 0xFFFF); ppu_Calc_Win0(); ppu_Calc_Win1(); break;
				case 0x48: ppu_Update_Win_In((uint16_t)(value & 0xFFFF));
						   ppu_Update_Win_Out((uint16_t)((value >> 16) & 0xFFFF)); break;
				case 0x4C: ppu_Update_Mosaic((uint16_t)(value & 0xFFFF));
						   /* no effect*/ break;

				case 0x50: ppu_Update_Special_FX((uint16_t)(value & 0xFFFF));
						   ppu_Update_Alpha((uint16_t)((value >> 16) & 0xFFFF)); break;
				case 0x54: ppu_Update_Bright((uint16_t)(value & 0xFFFF));
						   /* no effect*/ break;
			}
		}

		void ppu_BG_X_Write(uint16_t value, uint16_t num)
		{
			ppu_BG_X[num] = value;

			delays_to_process = true;
			ppu_Latch_Delays = true;

			ppu_BG_X_Latch_cd[num] = 3;
			ppu_BG_X_Latch_Delays[num] = true;
		}

		void ppu_STAT_Write(uint8_t value)
		{
			ppu_STAT &= 0xC7;
			ppu_STAT |= (uint8_t)(value & 0x38);
		}

		void ppu_Update_LYC(uint8_t value)
		{
			uint8_t old_LYC = ppu_LYC;

			ppu_LYC = value;

			// check if writing a matching LYC value will trigger an interrupt
			if ((old_LYC != ppu_LY) && (ppu_LY == ppu_LYC))
			{
				ppu_STAT |= 0x04;
				if (ppu_LYC_IRQ_cd == 0)
				{
					ppu_LYC_IRQ_cd = 4;
					ppu_Delays = true;
					delays_to_process = true;
				}
			}

			// does changing to non-matching value clear the flag? for now assume yes
			// also assume it doesn't disable any pending interrupts
			if ((old_LYC == ppu_LY) && (ppu_LY != ppu_LYC))
			{
				ppu_STAT &= 0xFB;
			}
		}

		void ppu_CTRL_Write(uint16_t value)
		{
			//delays_to_process = true;
			//ppu_Latch_Delays = true;
			//ppu_Ctrl_Latch_cd = 2;
			//ppu_Ctrl_Latch_Delay = true;

			ppu_BG_Mode = value & 7;
			ppu_Display_Frame = (value >> 4) & 1;

			ppu_HBL_Free = (value & 0x20) == 0x20;
			ppu_OBJ_Dim = (value & 0x40) == 0x40;

			ppu_WIN0_On = (value & 0x2000) == 0x2000;
			ppu_WIN1_On = (value & 0x4000) == 0x4000;
			ppu_OBJ_WIN = (value & 0x8000) == 0x8000;

			for (int i = 0; i < 4; i++)
			{
				ppu_BG_On[i] = (value & (0x100 << i)) == (0x100 << i);
			}

			// sprites require one scanline to turn on
			// latched value for display takes one cycle to update
			if ((value & 0x1000) == 0)
			{
				ppu_OBJ_On_Time = 0;
			}
			else if (ppu_OBJ_On_Time == 0)
			{
				if (ppu_Cycle < 40)
				{
					ppu_OBJ_On_Time = 2;
				}
				else
				{
					ppu_OBJ_On_Time = 3;
				}
			}

			delays_to_process = true;
			ppu_Latch_Delays = true;
			ppu_Sprite_Delay_Disp = true;
			ppu_Sprite_Disp_cd = 3;

			// forced blank timing is the same as BG enable
			if ((value & 0x80) == 0x80)
			{
				ppu_Forced_Blank = true;
				ppu_Forced_Blank_Time = 0;
			}
			else if (ppu_Forced_Blank_Time == 0)
			{
				if (ppu_Cycle < 40)
				{
					ppu_Forced_Blank_Time = 2;
				}
				else
				{
					ppu_Forced_Blank_Time = 3;
				}
			}

			if (ppu_HBL_Free)
			{
				ppu_Sprite_Eval_Time_OAM = 960;
				ppu_Sprite_Eval_Time_VRAM = 958;
			}
			else
			{
				ppu_Sprite_Eval_Time_OAM = 1230;
				ppu_Sprite_Eval_Time_VRAM = 1230;
			}

			ppu_Any_Window_On = ppu_WIN0_On || ppu_WIN1_On || ppu_OBJ_WIN;

			//Message_String = "ctrl " + to_string(value) + " old: " + to_string(ppu_CTRL) + " LY: " + to_string(ppu_LY) + " Cyc: " + to_string(ppu_Cycle);

			//if (MessageCallback) MessageCallback(Message_String.length());

			ppu_CTRL = value;

			// ensure BG controls match new mode
			ppu_BG_CTRL_Write(0);
			ppu_BG_CTRL_Write(1);
			ppu_BG_CTRL_Write(2);
			ppu_BG_CTRL_Write(3);

			if (ppu_Forced_Blank)
			{
				ppu_VRAM_Access = false;
				ppu_PALRAM_Access = false;
			}
		}

		void ppu_Calc_Win0()
		{
			ppu_WIN0_Right = (uint8_t)(ppu_WIN_Hor_0 & 0xFF);
			ppu_WIN0_Left = (uint8_t)((ppu_WIN_Hor_0 >> 8) & 0xFF);

			ppu_WIN0_Bot = (uint8_t)(ppu_WIN_Vert_0 & 0xFF);
			ppu_WIN0_Top = (uint8_t)((ppu_WIN_Vert_0 >> 8) & 0xFF);
		}

		void ppu_Calc_Win1()
		{
			ppu_WIN1_Right = (uint8_t)(ppu_WIN_Hor_1 & 0xFF);
			ppu_WIN1_Left = (uint8_t)((ppu_WIN_Hor_1 >> 8) & 0xFF);

			ppu_WIN1_Bot = (uint8_t)(ppu_WIN_Vert_1 & 0xFF);
			ppu_WIN1_Top = (uint8_t)((ppu_WIN_Vert_1 >> 8) & 0xFF);
		}

		void ppu_Update_Win_In(uint16_t value)
		{
			ppu_WIN_In = (uint16_t)(value & 0x3F3F);

			ppu_WIN0_BG_En[0] = (ppu_WIN_In & 0x1) == 0x1;
			ppu_WIN0_BG_En[1] = (ppu_WIN_In & 0x2) == 0x2;
			ppu_WIN0_BG_En[2] = (ppu_WIN_In & 0x4) == 0x4;
			ppu_WIN0_BG_En[3] = (ppu_WIN_In & 0x8) == 0x8;
			ppu_WIN0_OBJ_En = (ppu_WIN_In & 0x10) == 0x10;
			ppu_WIN0_Color_En = (ppu_WIN_In & 0x20) == 0x20;

			ppu_WIN1_BG_En[0] = (ppu_WIN_In & 0x100) == 0x100;
			ppu_WIN1_BG_En[1] = (ppu_WIN_In & 0x200) == 0x200;
			ppu_WIN1_BG_En[2] = (ppu_WIN_In & 0x400) == 0x400;
			ppu_WIN1_BG_En[3] = (ppu_WIN_In & 0x800) == 0x800;
			ppu_WIN1_OBJ_En = (ppu_WIN_In & 0x1000) == 0x1000;
			ppu_WIN1_Color_En = (ppu_WIN_In & 0x2000) == 0x2000;
		}

		void ppu_Update_Win_Out(uint16_t value)
		{
			ppu_WIN_Out = (uint16_t)(value & 0x3F3F);

			ppu_OUT_BG_En[0] = (ppu_WIN_Out & 0x1) == 0x1;
			ppu_OUT_BG_En[1] = (ppu_WIN_Out & 0x2) == 0x2;
			ppu_OUT_BG_En[2] = (ppu_WIN_Out & 0x4) == 0x4;
			ppu_OUT_BG_En[3] = (ppu_WIN_Out & 0x8) == 0x8;
			ppu_OUT_OBJ_En = (ppu_WIN_Out & 0x10) == 0x10;
			ppu_OUT_Color_En = (ppu_WIN_Out & 0x20) == 0x20;

			ppu_OBJ_BG_En[0] = (ppu_WIN_Out & 0x100) == 0x100;
			ppu_OBJ_BG_En[1] = (ppu_WIN_Out & 0x200) == 0x200;
			ppu_OBJ_BG_En[2] = (ppu_WIN_Out & 0x400) == 0x400;
			ppu_OBJ_BG_En[3] = (ppu_WIN_Out & 0x800) == 0x800;
			ppu_OBJ_OBJ_En = (ppu_WIN_Out & 0x1000) == 0x1000;
			ppu_OBJ_Color_En = (ppu_WIN_Out & 0x2000) == 0x2000;
		}

		void ppu_ROT_REF_X_Update(int layer)
		{
			if (ppu_BG_On[layer])
			{
				ppu_BG_Ref_X_Change[layer] = true;
			}
		}

		void ppu_ROT_REF_LY_Update(int layer)
		{
			if (ppu_BG_On[layer])
			{
				ppu_BG_Ref_LY_Change[layer] = true;
			}
		}

		void ppu_Update_Special_FX(uint16_t value)
		{
			ppu_Special_FX = (uint16_t)(value & 0x3FFF);

			ppu_SFX_mode = ((ppu_Special_FX >> 6) & 3);

			ppu_SFX_OBJ_Target_1 = ((ppu_Special_FX >> 4) & 0x1) != 0;

			ppu_SFX_OBJ_Target_2 = ((ppu_Special_FX >> 12) & 0x1) != 0;
		}

		void ppu_Update_Alpha(uint16_t value)
		{
			ppu_Alpha = (uint16_t)(value & 0x1F1F);

			if ((ppu_Alpha & 0x10) == 0x10)
			{
				ppu_SFX_Alpha_Num_1 = 16;
			}
			else
			{
				ppu_SFX_Alpha_Num_1 = ppu_Alpha & 0xF;
			}

			if ((ppu_Alpha & 0x1000) == 0x1000)
			{
				ppu_SFX_Alpha_Num_2 = 16;
			}
			else
			{
				ppu_SFX_Alpha_Num_2 = (ppu_Alpha >> 8) & 0xF;
			}
		}

		void ppu_Update_Bright(uint16_t value)
		{
			ppu_Bright = value;

			if ((ppu_Bright & 0x10) == 0x10)
			{
				ppu_SFX_BRT_Num = 16;
			}
			else
			{
				ppu_SFX_BRT_Num = ppu_Bright & 0xF;
			}
		}

		void ppu_Update_Mosaic(uint16_t value)
		{
			ppu_Mosaic = value;

			int j;

			uint16_t mosaic_x, mosaic_y;

			ppu_BG_Mosaic_X = (uint16_t)((ppu_Mosaic & 0xF) + 1);
			ppu_BG_Mosaic_Y = (uint16_t)(((ppu_Mosaic >> 4) & 0xF) + 1);
			ppu_OBJ_Mosaic_X = (uint16_t)(((ppu_Mosaic >> 8) & 0xF) + 1);
			ppu_OBJ_Mosaic_Y = (uint16_t)(((ppu_Mosaic >> 12) & 0xF) + 1);

			mosaic_y = 0;
			j = 0;

			for (int i = 0; i < 0x200; i++)
			{
				ppu_MOS_BG_Y[i] = mosaic_y;

				j++;

				if (j == ppu_BG_Mosaic_Y)
				{
					mosaic_y += ppu_BG_Mosaic_Y;
					j = 0;
				}
			}

			mosaic_x = 0;
			j = 0;

			for (int i = 0; i < 0x200; i++)
			{
				ppu_MOS_OBJ_X[i] = mosaic_x;

				j++;

				if (j == ppu_OBJ_Mosaic_X)
				{
					mosaic_x += ppu_OBJ_Mosaic_X;
					j = 0;
				}
			}
		}

		void ppu_BG_CTRL_Write(int lyr)
		{
			ppu_BG_Priority[lyr] = (uint8_t)(ppu_BG_CTRL[lyr] & 3);
			ppu_BG_Char_Base[lyr] = ((ppu_BG_CTRL[lyr] >> 2) & 3) * 16 * 1024;
			ppu_BG_Mosaic[lyr] = (ppu_BG_CTRL[lyr] & 0x40) == 0x40;
			ppu_BG_Pal_Size[lyr] = (ppu_BG_CTRL[lyr] & 0x80) == 0x80;
			ppu_BG_Screen_Base[lyr] = ((ppu_BG_CTRL[lyr] >> 8) & 0x1F) * 2 * 1024;
			ppu_BG_Overflow[lyr] = (ppu_BG_CTRL[lyr] & 0x2000) == 0x2000;
			ppu_BG_Screen_Size[lyr] = (uint8_t)((ppu_BG_CTRL[lyr] >> 14) & 3);

			if (ppu_BG_Mode == 0)
			{
				switch (ppu_BG_Screen_Size[lyr])
				{
				case 0:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 256;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 1:
					BG_Scale_X[lyr] = 512;
					BG_Scale_Y[lyr] = 256;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 2:
					BG_Scale_X[lyr] = 256;
					BG_Scale_Y[lyr] = 512;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 3:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 512;
					BG_Num_Tiles[lyr] = 32;
					break;
				}
			}
			else if ((lyr < 2) && (ppu_BG_Mode == 1))
			{
				switch (ppu_BG_Screen_Size[lyr])
				{
				case 0:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 256;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 1:
					BG_Scale_X[lyr] = 512;
					BG_Scale_Y[lyr] = 256;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 2:
					BG_Scale_X[lyr] = 256;
					BG_Scale_Y[lyr] = 512;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 3:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 512;
					BG_Num_Tiles[lyr] = 32;
					break;
				}
			}
			else
			{
				switch (ppu_BG_Screen_Size[lyr])
				{
				case 0:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 128;
					BG_Num_Tiles[lyr] = 16;
					break;

				case 1:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 256;
					BG_Num_Tiles[lyr] = 32;
					break;

				case 2:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 512;
					BG_Num_Tiles[lyr] = 64;
					break;

				case 3:
					BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 1024;
					BG_Num_Tiles[lyr] = 128;
					break;
				}
			}
		}

		void ppu_Render();

		inline void ppu_Set_VRAM_Access_True()
		{
			ppu_VRAM_Access = true;

			if (ppu_VRAM_In_Use)
			{
				if (cpu_Is_Paused)
				{
					dma_Access_Wait += 1;
				}
				else
				{
					cpu_Fetch_Wait += 1;
				}
			}
		}

		void ppu_Render_Sprites();

		void ppu_Sprite_VRAM_Cnt_Reset()
		{
			ppu_Fetch_Sprite_VRAM_Cnt = 0;

			ppu_Sprite_X_Scale = ppu_Sprite_X_Size >> 3;

			ppu_Sprite_Mode = (ppu_Sprite_Attr_0 >> 10) & 3;

			// GB tek says lower bit of tile number should be ignored in some cases, but it appears this is not the case?
			// more testing needed
			if ((ppu_Sprite_Attr_0 & 0x2000) == 0)
			{
				ppu_Sprite_VRAM_Mod = 0x3FF;
			}
			else
			{
				ppu_Sprite_VRAM_Mod = 0x3FF;
			}

			ppu_Sprite_Mosaic = (ppu_Sprite_Attr_0 & 0x1000) == 0x1000;

			// clip left side of sprite if needed
			// note if the sprite is completely clipped it is skipped altogether
			// so this case only comes up if there is definitely something to clip
			if (ppu_Sprite_X_Pos >= 240)
			{
				ppu_Fetch_Sprite_VRAM_Cnt = 512 - ppu_Sprite_X_Pos;

				if (!ppu_Rot_Scale) { ppu_Fetch_Sprite_VRAM_Cnt &= 0xFFFE; }
			}
		}

		void ppu_Do_Sprite_Calculation_Rot()
		{
			uint64_t A, B, C, D;

			uint64_t cur_x, cur_y;
			uint64_t sol_x, sol_y;

			A = ppu_Sprite_A_Latch;
			B = ppu_Sprite_B_Latch;
			C = ppu_Sprite_C_Latch;
			D = ppu_Sprite_D_Latch;

			if ((A & 0x8000) == 0x8000) { A |= 0xFFFFFFFFFFFF0000; }
			if ((B & 0x8000) == 0x8000) { B |= 0xFFFFFFFFFFFF0000; }
			if ((C & 0x8000) == 0x8000) { C |= 0xFFFFFFFFFFFF0000; }
			if ((D & 0x8000) == 0x8000) { D |= 0xFFFFFFFFFFFF0000; }

			if (((ppu_Sprite_Attr_0 >> 9) & 0x1) == 1)
			{
				for (int j = 0; j < 2 * ppu_Sprite_X_Size; j++)
				{
					cur_x = (uint64_t)(j - ppu_Sprite_X_Size);
					cur_y = (uint64_t)(ppu_Cur_Sprite_Y - ppu_Sprite_Y_Size);

					sol_x = A * cur_x + B * cur_y;
					sol_y = C * cur_x + D * cur_y;

					sol_x >>= 8;
					sol_y >>= 8;

					sol_x += (uint64_t)(ppu_Sprite_X_Size >> 1);
					sol_y += (uint64_t)(ppu_Sprite_Y_Size >> 1);

					ppu_ROT_OBJ_X[j] = (uint16_t)(sol_x);
					ppu_ROT_OBJ_Y[j] = (uint16_t)(sol_y);
				}
			}
			else
			{
				for (int j = 0; j < ppu_Sprite_X_Size; j++)
				{
					cur_x = (uint64_t)(j - (ppu_Sprite_X_Size >> 1));

					cur_y = (uint64_t)(ppu_Cur_Sprite_Y - (ppu_Sprite_Y_Size >> 1));

					sol_x = A * cur_x + B * cur_y;
					sol_y = C * cur_x + D * cur_y;

					sol_x >>= 8;
					sol_y >>= 8;

					sol_x += (uint64_t)(ppu_Sprite_X_Size >> 1);
					sol_y += (uint64_t)(ppu_Sprite_Y_Size >> 1);

					ppu_ROT_OBJ_X[j] = (uint16_t)(sol_x);
					ppu_ROT_OBJ_Y[j] = (uint16_t)(sol_y);
				}
			}
		}

		void ppu_Do_Sprite_Calculation()
		{
			bool h_flip, v_flip;

			uint32_t sol_x, sol_y;

			h_flip = ((ppu_Sprite_Attr_1 & 0x1000) == 0x1000);
			v_flip = ((ppu_Sprite_Attr_1 & 0x2000) == 0x2000);

			for (int j = 0; j < ppu_Sprite_X_Size; j++)
			{
				// horizontal flip
				if (h_flip)
				{
					sol_x = ppu_Sprite_X_Size - 1 - j;
				}
				else
				{
					sol_x = j;
				}

				// vertical flip
				if (v_flip)
				{
					sol_y = ppu_Sprite_Y_Size - 1 - ppu_Cur_Sprite_Y;
				}
				else
				{
					sol_y = ppu_Cur_Sprite_Y;
				}

				ppu_ROT_OBJ_X[j] = (uint16_t)sol_x;
				ppu_ROT_OBJ_Y[j] = (uint16_t)sol_y;
			}
		}

		void ppu_Convert_Rotation_to_ulong_AC(int layer)
		{
			if (layer == 2)
			{
				ppu_F_Rot_A_2 = ppu_BG_Rot_A[2];
				ppu_F_Rot_C_2 = ppu_BG_Rot_C[2];

				if ((ppu_F_Rot_A_2 & 0x8000) != 0) { ppu_F_Rot_A_2 |= 0xFFFFFFFFFFFF0000; }
				if ((ppu_F_Rot_C_2 & 0x8000) != 0) { ppu_F_Rot_C_2 |= 0xFFFFFFFFFFFF0000; }
			}
			else
			{
				ppu_F_Rot_A_3 = ppu_BG_Rot_A[3];
				ppu_F_Rot_C_3 = ppu_BG_Rot_C[3];

				if ((ppu_F_Rot_A_3 & 0x8000) != 0) { ppu_F_Rot_A_3 |= 0xFFFFFFFFFFFF0000; }
				if ((ppu_F_Rot_C_3 & 0x8000) != 0) { ppu_F_Rot_C_3 |= 0xFFFFFFFFFFFF0000; }
			}
		}

		uint8_t* ppu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ppu_In_VBlank, saver);
			saver = bool_saver(ppu_Delays, saver);
			saver = bool_saver(ppu_Latch_Delays, saver);
			saver = bool_saver(ppu_Sprite_Delay_Disp, saver);
			saver = bool_saver(ppu_Do_Green_Swap, saver);

			saver = bool_saver(ppu_VRAM_In_Use, saver);
			saver = bool_saver(ppu_VRAM_High_In_Use, saver);
			saver = bool_saver(ppu_PALRAM_In_Use, saver);

			saver = bool_saver(ppu_VRAM_Access, saver);
			saver = bool_saver(ppu_PALRAM_Access, saver);
			saver = bool_saver(ppu_OAM_Access, saver);
			saver = bool_saver(ppu_Continue_Fetch_OAM, saver);

			saver = bool_saver(ppu_HBL_Free, saver);
			saver = bool_saver(ppu_OBJ_Dim, saver);
			saver = bool_saver(ppu_Forced_Blank, saver);
			saver = bool_saver(ppu_Any_Window_On, saver);
			saver = bool_saver(ppu_OBJ_On, saver);
			saver = bool_saver(ppu_WIN0_On, saver);
			saver = bool_saver(ppu_WIN1_On, saver);
			saver = bool_saver(ppu_OBJ_WIN, saver);
			saver = bool_saver(ppu_WIN0_Active, saver);
			saver = bool_saver(ppu_WIN1_Active, saver);
			saver = bool_saver(ppu_OBJ_On_Disp, saver);
			saver = bool_saver(ppu_Ctrl_Latch_Delay, saver);

			saver = byte_saver(ppu_STAT, saver);
			saver = byte_saver(ppu_LY, saver);
			saver = byte_saver(ppu_LYC, saver);

			saver = short_saver(ppu_CTRL, saver);
			saver = short_saver(ppu_Green_Swap, saver);
			saver = short_saver(ppu_Cycle, saver);
			saver = short_saver(ppu_Display_Cycle, saver);
			saver = short_saver(ppu_Sprite_Eval_Time_OAM, saver);
			saver = short_saver(ppu_Sprite_Eval_Time_VRAM, saver);
			saver = short_saver(ppu_WIN_Hor_0, saver);
			saver = short_saver(ppu_WIN_Hor_1, saver);
			saver = short_saver(ppu_WIN_Vert_0, saver);
			saver = short_saver(ppu_WIN_Vert_1, saver);
			saver = short_saver(ppu_WIN_In, saver);
			saver = short_saver(ppu_WIN_Out, saver);
			saver = short_saver(ppu_Mosaic, saver);
			saver = short_saver(ppu_Special_FX, saver);
			saver = short_saver(ppu_Alpha, saver);
			saver = short_saver(ppu_Bright, saver);

			saver = int_saver(ppu_BG_Mode, saver);
			saver = int_saver(ppu_Display_Frame, saver);
			saver = int_saver(ppu_X_RS, saver);
			saver = int_saver(ppu_Y_RS, saver);
			saver = int_saver(ppu_Forced_Blank_Time, saver);
			saver = int_saver(ppu_OBJ_On_Time, saver);

			saver = int_saver(ppu_VBL_IRQ_cd, saver);
			saver = int_saver(ppu_HBL_IRQ_cd, saver);
			saver = int_saver(ppu_LYC_IRQ_cd, saver);
			saver = int_saver(ppu_Sprite_Disp_cd, saver);

			saver = int_saver(ppu_Ctrl_Latch_cd, saver);

			saver = bool_array_saver(ppu_BG_On, saver, 4);
			saver = bool_array_saver(ppu_BG_On_Disp, saver, 4);
			saver = bool_array_saver(ppu_BG_On_Latch, saver, 4);
			saver = bool_array_saver(ppu_BG_On_Latch_2, saver, 4);
			saver = bool_array_saver(ppu_BG_Ref_X_Change, saver, 4);
			saver = bool_array_saver(ppu_BG_Ref_LY_Change, saver, 4);
			saver = bool_array_saver(ppu_BG_X_Latch_Delays, saver, 4);
			saver = bool_array_saver(ppu_BG_Line_End, saver, 4);
			saver = bool_array_saver(ppu_BG_Line_End_BGS5, saver, 4);

			saver = short_array_saver(ppu_BG_CTRL, saver, 4);
			saver = short_array_saver(ppu_BG_X, saver, 4);
			saver = short_array_saver(ppu_BG_Y, saver, 4);
			saver = short_array_saver(ppu_BG_X_Latch, saver, 4);
			saver = short_array_saver(ppu_BG_Y_Latch, saver, 4);

			saver = short_array_saver(ppu_BG_Rot_A, saver, 4);
			saver = short_array_saver(ppu_BG_Rot_B, saver, 4);
			saver = short_array_saver(ppu_BG_Rot_C, saver, 4);
			saver = short_array_saver(ppu_BG_Rot_D, saver, 4);

			saver = short_array_saver(ppu_ROT_REF_LY, saver, 4);

			saver = int_array_saver(ppu_BG_Ref_X, saver, 4);
			saver = int_array_saver(ppu_BG_Ref_Y, saver, 4);

			saver = int_array_saver(ppu_BG_X_Latch_cd, saver, 4);

			// Sprite Evaluation
			saver = bool_saver(ppu_Rot_Scale, saver);
			saver = bool_saver(ppu_Rot_Scale_Temp, saver);
			saver = bool_saver(ppu_Fetch_OAM_0, saver);
			saver = bool_saver(ppu_Fetch_OAM_2, saver);
			saver = bool_saver(ppu_Fetch_OAM_A_D, saver);
			saver = bool_saver(ppu_Fetch_Sprite_VRAM, saver);
			saver = bool_saver(ppu_New_Sprite, saver);
			saver = bool_saver(ppu_Sprite_Eval_Finished, saver);
			saver = bool_saver(ppu_Sprite_Mosaic, saver);

			saver = byte_saver(ppu_Sprite_LY_Check, saver);

			saver = short_saver(ppu_Sprite_Attr_0, saver);
			saver = short_saver(ppu_Sprite_Attr_1, saver);
			saver = short_saver(ppu_Sprite_Attr_2, saver);
			saver = short_saver(ppu_Sprite_Attr_0_Temp, saver);
			saver = short_saver(ppu_Sprite_Attr_1_Temp, saver);

			saver = short_saver(ppu_Sprite_A_Latch, saver);
			saver = short_saver(ppu_Sprite_B_Latch, saver);
			saver = short_saver(ppu_Sprite_C_Latch, saver);
			saver = short_saver(ppu_Sprite_D_Latch, saver);

			saver = int_saver(ppu_Cur_Sprite_X, saver);
			saver = int_saver(ppu_Cur_Sprite_Y, saver);
			saver = int_saver(ppu_Cur_Sprite_Y_Temp, saver);

			saver = int_saver(ppu_Current_Sprite, saver);
			saver = int_saver(ppu_Process_Sprite, saver);
			saver = int_saver(ppu_Process_Sprite_Temp, saver);
			saver = int_saver(ppu_Sprite_ofst_eval, saver);
			saver = int_saver(ppu_Sprite_ofst_draw, saver);
			saver = int_saver(ppu_Sprite_X_Pos, saver);
			saver = int_saver(ppu_Sprite_Y_Pos, saver);
			saver = int_saver(ppu_Sprite_X_Pos_Temp, saver);
			saver = int_saver(ppu_Sprite_Y_Pos_Temp, saver);
			saver = int_saver(ppu_Sprite_X_Size, saver);
			saver = int_saver(ppu_Sprite_Y_Size, saver);
			saver = int_saver(ppu_Sprite_X_Size_Temp, saver);
			saver = int_saver(ppu_Sprite_Y_Size_Temp, saver);
			saver = int_saver(ppu_Sprite_Render_Cycle, saver);
			saver = int_saver(ppu_Fetch_OAM_A_D_Cnt, saver);
			saver = int_saver(ppu_Fetch_Sprite_VRAM_Cnt, saver);
			saver = int_saver(ppu_Sprite_VRAM_Mod, saver);
			saver = int_saver(ppu_Sprite_X_Scale, saver);
			saver = int_saver(ppu_Sprite_Size_X_Ofst, saver);
			saver = int_saver(ppu_Sprite_Size_Y_Ofst, saver);
			saver = int_saver(ppu_Sprite_Size_X_Ofst_Temp, saver);
			saver = int_saver(ppu_Sprite_Size_Y_Ofst_Temp, saver);
			saver = int_saver(ppu_Sprite_Mode, saver);
			saver = int_saver(ppu_Sprite_Next_Fetch, saver);
			saver = int_saver(ppu_Param_Pick, saver);
			saver = int_saver(ppu_Sprite_Mosaic_Y_Counter, saver);
			saver = int_saver(ppu_Sprite_Mosaic_Y_Compare, saver);

			saver = bool_array_saver(ppu_Sprite_Pixel_Occupied, saver, 240 * 2);
			saver = bool_array_saver(ppu_Sprite_Semi_Transparent, saver, 240 * 2);
			saver = bool_array_saver(ppu_Sprite_Object_Window, saver, 240 * 2);
			saver = bool_array_saver(ppu_Sprite_Is_Mosaic, saver, 240 * 2);

			saver = int_array_saver(ppu_Sprite_Pixels, saver, 240 * 2);
			saver = int_array_saver(ppu_Sprite_Priority, saver, 240 * 2);

			// sprite latches
			saver = int_saver(ppu_Sprite_Pixel_Latch, saver);
			saver = int_saver(ppu_Sprite_Priority_Latch, saver);

			saver = bool_saver(ppu_Sprite_Semi_Transparent_Latch, saver);
			saver = bool_saver(ppu_Sprite_Mosaic_Latch, saver);
			saver = bool_saver(ppu_Sprite_Pixel_Occupied_Latch, saver);

			// BG rendering
			saver = int_array_saver(ppu_Fetch_Count, saver, 4);
			saver = int_array_saver(ppu_Scroll_Cycle, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color_2, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color_1, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color_M, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color_R, saver, 4);
			saver = int_array_saver(ppu_Tile_Addr, saver, 4);
			saver = int_array_saver(ppu_Y_Flip_Ofst, saver, 4);

			saver = short_array_saver(ppu_BG_Start_Time, saver, 4);

			saver = byte_array_saver(ppu_BG_Effect_Byte, saver, 4);
			saver = byte_array_saver(ppu_BG_Effect_Byte_New, saver, 4);

			saver = bool_array_saver(ppu_BG_Rendering_Complete, saver, 4);
			saver = bool_array_saver(ppu_BG_Has_Pixel, saver, 4);
			saver = bool_array_saver(ppu_BG_Has_Pixel_2, saver, 4);
			saver = bool_array_saver(ppu_BG_Has_Pixel_1, saver, 4);
			saver = bool_array_saver(ppu_BG_Has_Pixel_M, saver, 4);
			saver = bool_array_saver(ppu_BG_Has_Pixel_R, saver, 4);

			saver = long_saver(ppu_Current_Ref_X_2, saver);
			saver = long_saver(ppu_Current_Ref_Y_2, saver);
			saver = long_saver(ppu_Current_Ref_X_3, saver);
			saver = long_saver(ppu_Current_Ref_Y_3, saver);

			saver = int_saver(ppu_Base_LY_2, saver);
			saver = int_saver(ppu_Base_LY_3, saver);

			saver = int_saver(ppu_BG_Pixel_F, saver);
			saver = int_saver(ppu_BG_Pixel_S, saver);
			saver = int_saver(ppu_Final_Pixel, saver);
			saver = int_saver(ppu_Blend_Pixel, saver);

			saver = short_saver(ppu_BG_Mosaic_X_Mod, saver);

			saver = bool_saver(ppu_Brighten_Final_Pixel, saver);
			saver = bool_saver(ppu_Blend_Final_Pixel, saver);

			saver = bool_saver(ppu_Fetch_BG, saver);

			saver = bool_saver(ppu_Fetch_Target_1, saver);
			saver = bool_saver(ppu_Fetch_Target_2, saver);

			saver = bool_saver(ppu_Rendering_Complete, saver);
			saver = bool_saver(ppu_PAL_Rendering_Complete, saver);

			saver = short_saver(ppu_VRAM_Open_Bus, saver);
			
			return saver;
		}

		uint8_t* ppu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ppu_In_VBlank, loader);
			loader = bool_loader(&ppu_Delays, loader);
			loader = bool_loader(&ppu_Latch_Delays, loader);
			loader = bool_loader(&ppu_Sprite_Delay_Disp, loader);
			loader = bool_loader(&ppu_Do_Green_Swap, loader);

			loader = bool_loader(&ppu_VRAM_In_Use, loader);
			loader = bool_loader(&ppu_VRAM_High_In_Use, loader);
			loader = bool_loader(&ppu_PALRAM_In_Use, loader);

			loader = bool_loader(&ppu_VRAM_Access, loader);
			loader = bool_loader(&ppu_PALRAM_Access, loader);
			loader = bool_loader(&ppu_OAM_Access, loader);
			loader = bool_loader(&ppu_Continue_Fetch_OAM, loader);

			loader = bool_loader(&ppu_HBL_Free, loader);
			loader = bool_loader(&ppu_OBJ_Dim, loader);
			loader = bool_loader(&ppu_Forced_Blank, loader);
			loader = bool_loader(&ppu_Any_Window_On, loader);
			loader = bool_loader(&ppu_OBJ_On, loader);
			loader = bool_loader(&ppu_WIN0_On, loader);
			loader = bool_loader(&ppu_WIN1_On, loader);
			loader = bool_loader(&ppu_OBJ_WIN, loader);
			loader = bool_loader(&ppu_WIN0_Active, loader);
			loader = bool_loader(&ppu_WIN1_Active, loader);
			loader = bool_loader(&ppu_OBJ_On_Disp, loader);
			loader = bool_loader(&ppu_Ctrl_Latch_Delay, loader);

			loader = byte_loader(&ppu_STAT, loader);
			loader = byte_loader(&ppu_LY, loader);
			loader = byte_loader(&ppu_LYC, loader);

			loader = short_loader(&ppu_CTRL, loader);
			loader = short_loader(&ppu_Green_Swap, loader);
			loader = short_loader(&ppu_Cycle, loader);
			loader = short_loader(&ppu_Display_Cycle, loader);
			loader = short_loader(&ppu_Sprite_Eval_Time_OAM, loader);
			loader = short_loader(&ppu_Sprite_Eval_Time_VRAM, loader);
			loader = short_loader(&ppu_WIN_Hor_0, loader);
			loader = short_loader(&ppu_WIN_Hor_1, loader);
			loader = short_loader(&ppu_WIN_Vert_0, loader);
			loader = short_loader(&ppu_WIN_Vert_1, loader);
			loader = short_loader(&ppu_WIN_In, loader);
			loader = short_loader(&ppu_WIN_Out, loader);
			loader = short_loader(&ppu_Mosaic, loader);
			loader = short_loader(&ppu_Special_FX, loader);
			loader = short_loader(&ppu_Alpha, loader);
			loader = short_loader(&ppu_Bright, loader);

			loader = int_loader(&ppu_BG_Mode, loader);
			loader = int_loader(&ppu_Display_Frame, loader);
			loader = int_loader(&ppu_X_RS, loader);
			loader = int_loader(&ppu_Y_RS, loader);
			loader = int_loader(&ppu_Forced_Blank_Time, loader);
			loader = int_loader(&ppu_OBJ_On_Time, loader);

			loader = int_loader(&ppu_VBL_IRQ_cd, loader);
			loader = int_loader(&ppu_HBL_IRQ_cd, loader);
			loader = int_loader(&ppu_LYC_IRQ_cd, loader);
			loader = int_loader(&ppu_Sprite_Disp_cd, loader);

			loader = int_loader(&ppu_Ctrl_Latch_cd, loader);

			loader = bool_array_loader(ppu_BG_On, loader, 4);
			loader = bool_array_loader(ppu_BG_On_Disp, loader, 4);
			loader = bool_array_loader(ppu_BG_On_Latch, loader, 4);
			loader = bool_array_loader(ppu_BG_On_Latch_2, loader, 4);
			loader = bool_array_loader(ppu_BG_Ref_X_Change, loader, 4);
			loader = bool_array_loader(ppu_BG_Ref_LY_Change, loader, 4);
			loader = bool_array_loader(ppu_BG_X_Latch_Delays, loader, 4);
			loader = bool_array_loader(ppu_BG_Line_End, loader, 4);
			loader = bool_array_loader(ppu_BG_Line_End_BGS5, loader, 4);

			loader = short_array_loader(ppu_BG_CTRL, loader, 4);
			loader = short_array_loader(ppu_BG_X, loader, 4);
			loader = short_array_loader(ppu_BG_Y, loader, 4);
			loader = short_array_loader(ppu_BG_X_Latch, loader, 4);
			loader = short_array_loader(ppu_BG_Y_Latch, loader, 4);

			loader = short_array_loader(ppu_BG_Rot_A, loader, 4);
			loader = short_array_loader(ppu_BG_Rot_B, loader, 4);
			loader = short_array_loader(ppu_BG_Rot_C, loader, 4);
			loader = short_array_loader(ppu_BG_Rot_D, loader, 4);

			loader = short_array_loader(ppu_ROT_REF_LY, loader, 4);

			loader = int_array_loader(ppu_BG_Ref_X, loader, 4);
			loader = int_array_loader(ppu_BG_Ref_Y, loader, 4);

			loader = int_array_loader(ppu_BG_X_Latch_cd, loader, 4);

			// Sprite Evaluation
			loader = bool_loader(&ppu_Rot_Scale, loader);
			loader = bool_loader(&ppu_Rot_Scale_Temp, loader);
			loader = bool_loader(&ppu_Fetch_OAM_0, loader);
			loader = bool_loader(&ppu_Fetch_OAM_2, loader);
			loader = bool_loader(&ppu_Fetch_OAM_A_D, loader);
			loader = bool_loader(&ppu_Fetch_Sprite_VRAM, loader);
			loader = bool_loader(&ppu_New_Sprite, loader);
			loader = bool_loader(&ppu_Sprite_Eval_Finished, loader);
			loader = bool_loader(&ppu_Sprite_Mosaic, loader);

			loader = byte_loader(&ppu_Sprite_LY_Check, loader);

			loader = short_loader(&ppu_Sprite_Attr_0, loader);
			loader = short_loader(&ppu_Sprite_Attr_1, loader);
			loader = short_loader(&ppu_Sprite_Attr_2, loader);
			loader = short_loader(&ppu_Sprite_Attr_0_Temp, loader);
			loader = short_loader(&ppu_Sprite_Attr_1_Temp, loader);

			loader = short_loader(&ppu_Sprite_A_Latch, loader);
			loader = short_loader(&ppu_Sprite_B_Latch, loader);
			loader = short_loader(&ppu_Sprite_C_Latch, loader);
			loader = short_loader(&ppu_Sprite_D_Latch, loader);

			loader = int_loader(&ppu_Cur_Sprite_X, loader);
			loader = int_loader(&ppu_Cur_Sprite_Y, loader);
			loader = int_loader(&ppu_Cur_Sprite_Y_Temp, loader);

			loader = int_loader(&ppu_Current_Sprite, loader);
			loader = int_loader(&ppu_Process_Sprite, loader);
			loader = int_loader(&ppu_Process_Sprite_Temp, loader);
			loader = int_loader(&ppu_Sprite_ofst_eval, loader);
			loader = int_loader(&ppu_Sprite_ofst_draw, loader);
			loader = int_loader(&ppu_Sprite_X_Pos, loader);
			loader = int_loader(&ppu_Sprite_Y_Pos, loader);
			loader = int_loader(&ppu_Sprite_X_Pos_Temp, loader);
			loader = int_loader(&ppu_Sprite_Y_Pos_Temp, loader);
			loader = int_loader(&ppu_Sprite_X_Size, loader);
			loader = int_loader(&ppu_Sprite_Y_Size, loader);
			loader = int_loader(&ppu_Sprite_X_Size_Temp, loader);
			loader = int_loader(&ppu_Sprite_Y_Size_Temp, loader);
			loader = int_loader(&ppu_Sprite_Render_Cycle, loader);
			loader = int_loader(&ppu_Fetch_OAM_A_D_Cnt, loader);
			loader = int_loader(&ppu_Fetch_Sprite_VRAM_Cnt, loader);
			loader = int_loader(&ppu_Sprite_VRAM_Mod, loader);
			loader = int_loader(&ppu_Sprite_X_Scale, loader);
			loader = int_loader(&ppu_Sprite_Size_X_Ofst, loader);
			loader = int_loader(&ppu_Sprite_Size_Y_Ofst, loader);
			loader = int_loader(&ppu_Sprite_Size_X_Ofst_Temp, loader);
			loader = int_loader(&ppu_Sprite_Size_Y_Ofst_Temp, loader);
			loader = int_loader(&ppu_Sprite_Mode, loader);
			loader = int_loader(&ppu_Sprite_Next_Fetch, loader);
			loader = int_loader(&ppu_Param_Pick, loader);
			loader = int_loader(&ppu_Sprite_Mosaic_Y_Counter, loader);
			loader = int_loader(&ppu_Sprite_Mosaic_Y_Compare, loader);

			loader = bool_array_loader(ppu_Sprite_Pixel_Occupied, loader, 240 * 2);
			loader = bool_array_loader(ppu_Sprite_Semi_Transparent, loader, 240 * 2);
			loader = bool_array_loader(ppu_Sprite_Object_Window, loader, 240 * 2);
			loader = bool_array_loader(ppu_Sprite_Is_Mosaic, loader, 240 * 2);

			loader = int_array_loader(ppu_Sprite_Pixels, loader, 240 * 2);
			loader = int_array_loader(ppu_Sprite_Priority, loader, 240 * 2);

			// sprite latches
			loader = int_loader(&ppu_Sprite_Pixel_Latch, loader);
			loader = int_loader(&ppu_Sprite_Priority_Latch, loader);

			loader = bool_loader(&ppu_Sprite_Semi_Transparent_Latch, loader);
			loader = bool_loader(&ppu_Sprite_Mosaic_Latch, loader);
			loader = bool_loader(&ppu_Sprite_Pixel_Occupied_Latch, loader);

			// BG rendering
			loader = int_array_loader(ppu_Fetch_Count, loader, 4);
			loader = int_array_loader(ppu_Scroll_Cycle, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color_2, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color_1, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color_M, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color_R, loader, 4);
			loader = int_array_loader(ppu_Tile_Addr, loader, 4);
			loader = int_array_loader(ppu_Y_Flip_Ofst, loader, 4);

			loader = short_array_loader(ppu_BG_Start_Time, loader, 4);

			loader = byte_array_loader(ppu_BG_Effect_Byte, loader, 4);
			loader = byte_array_loader(ppu_BG_Effect_Byte_New, loader, 4);

			loader = bool_array_loader(ppu_BG_Rendering_Complete, loader, 4);
			loader = bool_array_loader(ppu_BG_Has_Pixel, loader, 4);
			loader = bool_array_loader(ppu_BG_Has_Pixel_2, loader, 4);
			loader = bool_array_loader(ppu_BG_Has_Pixel_1, loader, 4);
			loader = bool_array_loader(ppu_BG_Has_Pixel_M, loader, 4);
			loader = bool_array_loader(ppu_BG_Has_Pixel_R, loader, 4);

			loader = long_loader(&ppu_Current_Ref_X_2, loader);
			loader = long_loader(&ppu_Current_Ref_Y_2, loader);
			loader = long_loader(&ppu_Current_Ref_X_3, loader);
			loader = long_loader(&ppu_Current_Ref_Y_3, loader);

			loader = int_loader(&ppu_Base_LY_2, loader);
			loader = int_loader(&ppu_Base_LY_3, loader);

			loader = int_loader(&ppu_BG_Pixel_F, loader);
			loader = int_loader(&ppu_BG_Pixel_S, loader);
			loader = int_loader(&ppu_Final_Pixel, loader);
			loader = int_loader(&ppu_Blend_Pixel, loader);

			loader = short_loader(&ppu_BG_Mosaic_X_Mod, loader);

			loader = bool_loader(&ppu_Brighten_Final_Pixel, loader);
			loader = bool_loader(&ppu_Blend_Final_Pixel, loader);

			loader = bool_loader(&ppu_Fetch_BG, loader);

			loader = bool_loader(&ppu_Fetch_Target_1, loader);
			loader = bool_loader(&ppu_Fetch_Target_2, loader);

			loader = bool_loader(&ppu_Rendering_Complete, loader);
			loader = bool_loader(&ppu_PAL_Rendering_Complete, loader);

			loader = short_loader(&ppu_VRAM_Open_Bus, loader);

			// update derived values
			ppu_Calc_Win0();
			ppu_Calc_Win1();
			ppu_Update_Win_In(ppu_WIN_In);
			ppu_Update_Win_Out(ppu_WIN_Out);
			ppu_BG_CTRL_Write(0);
			ppu_BG_CTRL_Write(1);
			ppu_BG_CTRL_Write(2);
			ppu_BG_CTRL_Write(3);
			ppu_Update_Special_FX(ppu_Special_FX);
			ppu_Update_Alpha(ppu_Alpha);
			ppu_Update_Bright(ppu_Bright);
			ppu_Update_Mosaic(ppu_Mosaic);

			ppu_Convert_Rotation_to_ulong_AC(2);
			ppu_Convert_Rotation_to_ulong_AC(3);

			return loader;
		}

	#pragma endregion

	#pragma region Audio

		bool snd_Duty_Cycles[32] = {false, false, false, false, false, false, false, true,
									true, false, false, false, false, false, false, true,
									true, false, false, false, false, true, true, true,
									false, true, true, true, true, true, true, false };

		uint32_t snd_Divisor[8] = {8, 16, 32, 48, 64, 80, 96, 112};

		uint32_t snd_Chan_Mult_Table[4] = {1, 2, 4, 4};

		uint8_t snd_Audio_Regs[0x30] = { };

		uint8_t snd_Wave_RAM[32] = { };

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

		// FIFOs
		uint32_t snd_FIFO_A_ptr, snd_FIFO_B_ptr;
		uint32_t snd_FIFO_A_Timer, snd_FIFO_B_Timer;
		uint32_t snd_FIFO_A_Output, snd_FIFO_B_Output;
		uint32_t snd_FIFO_A_Mult, snd_FIFO_B_Mult;

		uint8_t snd_FIFO_A[32] = { };
		uint8_t snd_FIFO_B[32] = { };

		uint8_t snd_FIFO_A_Data[4] = { };
		uint8_t snd_FIFO_B_Data[4] = { };

		uint8_t snd_FIFO_A_Sample, snd_FIFO_B_Sample;

		bool snd_FIFO_A_Tick, snd_FIFO_B_Tick;
		bool snd_FIFO_A_Enable_L, snd_FIFO_B_Enable_L, snd_FIFO_A_Enable_R, snd_FIFO_B_Enable_R;

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

		uint8_t snd_Read_Reg_8(uint32_t addr)
		{
			uint8_t ret = 0;

			if (addr < 0x8C)
			{
				ret = snd_Audio_Regs[addr - 0x60];
			}
			else if ((addr < 0xA0) && (addr >= 0x90))
			{
				int ofst = (int)(snd_Wave_Bank + addr - 0x90);

				ret = snd_Wave_RAM[ofst];
			}
			else
			{
				// FIFO not readable, other addresses are open bus
				ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus;
			}

			return ret;
		}

		uint16_t snd_Read_Reg_16(uint32_t addr)
		{
			uint16_t ret = 0;

			if (addr < 0x8C)
			{
				ret = (uint16_t)((snd_Audio_Regs[addr - 0x60 + 1] << 8) | snd_Audio_Regs[addr - 0x60]);
			}
			else if ((addr < 0xA0) && (addr >= 0x90))
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));

				ret = (uint16_t)((snd_Wave_RAM[ofst + 1] << 8) | snd_Wave_RAM[ofst]);
			}
			else
			{
				// FIFO not readable, other addresses are open bus
				ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); // open bus
			}

			return ret;
		}

		uint32_t snd_Read_Reg_32(uint32_t addr)
		{
			uint32_t ret = 0;

			if (addr < 0x8C)
			{
				ret = (uint32_t)((snd_Audio_Regs[addr - 0x60 + 3] << 24) | (snd_Audio_Regs[addr - 0x60 + 2] << 16) | (snd_Audio_Regs[addr - 0x60 + 1] << 8) | snd_Audio_Regs[addr - 0x60]);
			}
			else if ((addr < 0xA0) && (addr >= 0x90))
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));

				ret = (uint32_t)((snd_Wave_RAM[ofst + 3] << 24) | (snd_Wave_RAM[ofst + 2] << 16) | (snd_Wave_RAM[ofst + 1] << 8) | snd_Wave_RAM[ofst]);
			}
			else
			{
				// FIFO not readable, other addresses are open bus
				ret = cpu_Last_Bus_Value; // open bus
			}

			return ret;
		}

		void snd_Write_Reg_8(uint32_t addr, uint8_t value)
		{
			if (addr < 0x90)
			{
				snd_Update_Regs(addr, value);
			}
			else if (addr < 0xA0)
			{
				int ofst = (int)(snd_Wave_Bank + addr - 0x90);

				snd_Wave_RAM[ofst] = value;
			}
			else if (addr < 0xA4)
			{
				snd_FIFO_A_Data[(addr & 3)] = value;
				snd_Write_FIFO_Data(true);
			}
			else if (addr < 0xA8)
			{
				snd_FIFO_B_Data[(addr & 3)] = value;
				snd_Write_FIFO_Data(false);
			}
		}

		void snd_Write_Reg_16(uint32_t addr, uint16_t value)
		{
			if (addr < 0x90)
			{
				snd_Update_Regs(addr, (uint8_t)(value & 0xFF));
				snd_Update_Regs((uint32_t)(addr + 1), (uint8_t)((value >> 8) & 0xFF));
			}
			else if (addr < 0xA0)
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));
				snd_Wave_RAM[ofst] = (uint8_t)(value & 0xFF);
				snd_Wave_RAM[ofst + 1] = (uint8_t)((value >> 8) & 0xFF);
			}
			else if (addr < 0xA4)
			{
				if (addr == 0xA0)
				{
					snd_FIFO_A_Data[0] = (uint8_t)(value & 0xFF);
					snd_FIFO_A_Data[1] = (uint8_t)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(true);
				}
				else
				{
					snd_FIFO_A_Data[2] = (uint8_t)(value & 0xFF);
					snd_FIFO_A_Data[3] = (uint8_t)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(true);
				}
			}
			else if (addr < 0xA8)
			{
				if (addr == 0xA6)
				{
					snd_FIFO_B_Data[0] = (uint8_t)(value & 0xFF);
					snd_FIFO_B_Data[1] = (uint8_t)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(false);
				}
				else
				{
					snd_FIFO_B_Data[2] = (uint8_t)(value & 0xFF);
					snd_FIFO_B_Data[3] = (uint8_t)((value >> 8) & 0xFF);
					snd_Write_FIFO_Data(false);
				}
			}
		}

		void snd_Write_Reg_32(uint32_t addr, uint32_t value)
		{
			if (addr < 0x90)
			{
				snd_Update_Regs(addr, (uint8_t)(value & 0xFF));
				snd_Update_Regs((uint32_t)(addr + 1), (uint8_t)((value >> 8) & 0xFF));
				snd_Update_Regs((uint32_t)(addr + 2), (uint8_t)((value >> 16) & 0xFF));
				snd_Update_Regs((uint32_t)(addr + 3), (uint8_t)((value >> 24) & 0xFF));
			}
			else if (addr < 0xA0)
			{
				int ofst = (int)(snd_Wave_Bank + (addr - 0x90));
				snd_Wave_RAM[ofst] = (uint8_t)(value & 0xFF);
				snd_Wave_RAM[ofst + 1] = (uint8_t)((value >> 8) & 0xFF);
				snd_Wave_RAM[ofst + 2] = (uint8_t)((value >> 16) & 0xFF);
				snd_Wave_RAM[ofst + 3] = (uint8_t)((value >> 24) & 0xFF);
			}
			else if (addr < 0xA4)
			{
				snd_FIFO_A_Data[0] = (uint8_t)(value & 0xFF);
				snd_FIFO_A_Data[1] = (uint8_t)((value >> 8) & 0xFF);
				snd_FIFO_A_Data[2] = (uint8_t)((value >> 16) & 0xFF);
				snd_FIFO_A_Data[3] = (uint8_t)((value >> 24) & 0xFF);
				snd_Write_FIFO_Data(true);
			}
			else if (addr < 0xA8)
			{
				snd_FIFO_B_Data[0] = (uint8_t)(value & 0xFF);
				snd_FIFO_B_Data[1] = (uint8_t)((value >> 8) & 0xFF);
				snd_FIFO_B_Data[2] = (uint8_t)((value >> 16) & 0xFF);
				snd_FIFO_B_Data[3] = (uint8_t)((value >> 24) & 0xFF);
				snd_Write_FIFO_Data(false);
			}
		}

		void snd_Write_FIFO_Data(bool chan_A)
		{
			if (snd_CTRL_power)
			{
				for (int i = 0; i < 4; i++)
				{
					if (chan_A)
					{
						if (snd_FIFO_A_ptr < 32)
						{
							snd_FIFO_A[snd_FIFO_A_ptr] = snd_FIFO_A_Data[i];
							snd_FIFO_A_ptr += 1;
						}

						if (snd_FIFO_A_ptr == 32)
						{
							snd_FIFO_A_ptr = 0;
						}
					}
					else
					{
						if (snd_FIFO_B_ptr < 32)
						{
							snd_FIFO_B[snd_FIFO_B_ptr] = snd_FIFO_B_Data[i];
							snd_FIFO_B_ptr += 1;
						}

						if (snd_FIFO_B_ptr == 32)
						{
							snd_FIFO_B_ptr = 0;
						}
					}
				}
			}
		}

		void snd_Update_Regs(uint32_t addr, uint8_t value)
		{
			// while power is on, everything is writable
			//Message_String = to_string(addr) + " " + to_string(value) + " " + to_string(CycleCount);

			//if (MessageCallback) { MessageCallback(Message_String.length()); }

			if (snd_CTRL_power)
			{
				if (addr < 0x90)
				{
					switch (addr)
					{
						case 0x60:												// NR10 (sweep)
							snd_SQ1_swp_prd = (uint8_t)((value & 0x70) >> 4);
							snd_SQ1_negate = (value & 8) > 0;
							snd_SQ1_shift = (uint8_t)(value & 7);

							if (!snd_SQ1_negate && snd_SQ1_calc_done) { snd_SQ1_enable = false; }

							value &= 0x7F;
							break;

						case 0x61:
							// not writable
							value = 0;
							break;

						case 0x62:												// NR11 (sound length / wave pattern duty %)
							snd_SQ1_duty = (uint8_t)((value & 0xC0) >> 6);
							snd_SQ1_length = (uint16_t)(64 - (value & 0x3F));
							snd_SQ1_len_cntr = snd_SQ1_length;

							// lower bits not readable
							value &= 0xC0;
							break;

						case 0x63:												// NR12 (envelope)
							snd_SQ1_st_vol = (uint8_t)((value & 0xF0) >> 4);
							snd_SQ1_env_add = (value & 8) > 0;
							snd_SQ1_per = (uint8_t)(value & 7);

							// several glitchy effects happen when writing to snd_NRx2 during audio playing
							if (((snd_Audio_Regs[0x03] & 7) == 0) && !snd_SQ1_vol_done) { snd_SQ1_vol_state++; }
							else if ((snd_Audio_Regs[0x03] & 8) == 0) { snd_SQ1_vol_state += 2; }

							if (((snd_Audio_Regs[0x03] ^ value) & 8) > 0) { snd_SQ1_vol_state = (uint8_t)(0x10 - snd_SQ1_vol_state); }

							snd_SQ1_vol_state &= 0xF;

							if ((value & 0xF8) == 0) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
							break;

						case 0x64:												// NR13 (freq low)
							snd_SQ1_frq &= 0x700;
							snd_SQ1_frq |= value;

							// not readable
							value = 0;
							break;

						case 0x65:												// NR14 (freq hi)
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
									shadow_frq = shadow_frq >> snd_SQ1_shift;
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

							snd_SQ1_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x66:
						case 0x67:
							// not writable
							value = 0;
							break;

						case 0x68:												// NR21 (sound length / wave pattern duty %)		
							snd_SQ2_duty = (uint8_t)((value & 0xC0) >> 6);
							snd_SQ2_length = (uint16_t)(64 - (value & 0x3F));
							snd_SQ2_len_cntr = snd_SQ2_length;

							value &= 0xC0;
							break;

						case 0x69:												// NR22 (envelope)
							snd_SQ2_st_vol = (uint8_t)((value & 0xF0) >> 4);
							snd_SQ2_env_add = (value & 8) > 0;
							snd_SQ2_per = (uint8_t)(value & 7);

							// several glitchy effects happen when writing to snd_NRx2 during audio playing
							if (((snd_Audio_Regs[0x09] & 7) == 0) && !snd_SQ2_vol_done) { snd_SQ2_vol_state++; }
							else if ((snd_Audio_Regs[0x09] & 8) == 0) { snd_SQ2_vol_state += 2; }

							if (((snd_Audio_Regs[0x09] ^ value) & 8) > 0) { snd_SQ2_vol_state = (uint8_t)(0x10 - snd_SQ2_vol_state); }

							snd_SQ2_vol_state &= 0xF;
							if ((value & 0xF8) == 0) { snd_SQ2_enable = false; }

							break;

						case 0x6A:
						case 0x6B:
							// not writable
							value = 0;
							break;

						case 0x6C:												// NR23 (freq low)
							snd_SQ2_frq &= 0x700;
							snd_SQ2_frq |= value;

							value = 0;
							break;

						case 0x6D:												// NR24 (freq hi)
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

							snd_SQ2_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x6E:
						case 0x6F:
							// not writable
							value = 0;
							break;

						case 0x70:												// NR30 (on/off)
							snd_WAVE_DAC_pow = (value & 0x80) > 0;
							if (!snd_WAVE_DAC_pow) { snd_WAVE_enable = false; }

							// selected bank is played, other one can be written to
							snd_Wave_Bank = ((value & 0x40) == 0) ? 16 : 0;

							snd_Wave_Size = (value & 0x20) == 0x20;

							value &= 0xE0;
							break;

						case 0x71:
							value = 0;
							break;

						case 0x72:												// NR31 (length)
							snd_WAVE_length = (uint16_t)(256 - value);
							snd_WAVE_len_cntr = snd_WAVE_length;

							value = 0;
							break;

						case 0x73:												// NR32 (level output)
							snd_WAVE_vol_code = (uint8_t)((value & 0x60) >> 5);
							snd_Wave_Vol_Force = (value & 0x80) == 0x80;

							value &= 0xE0;
							break;

						case 0x74:												// NR33 (freq low)
							snd_WAVE_frq &= 0x700;
							snd_WAVE_frq |= value;

							value = 0;
							break;

						case 0x75:												// NR34 (freq hi)
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
								snd_WAVE_enable = true;

								if (snd_WAVE_len_cntr == 0)
								{
									snd_WAVE_len_cntr = 256;
									if (((value & 0x40) > 0) && ((snd_Sequencer_len & 1) == 0)) { snd_WAVE_len_cntr--; }
								}
								snd_WAVE_intl_cntr = (2048 - snd_WAVE_frq) * 2 + 6; // trigger delay for wave channel

								snd_WAVE_wave_cntr = 0;
								if (!snd_WAVE_DAC_pow) { snd_WAVE_enable = false; }

								snd_Wave_Bank_Playing = (snd_Wave_Bank == 0) ? 16 : 0;
							}

							snd_WAVE_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x76:
						case 0x77:
							// not writable
							value = 0;
							break;

						case 0x78:												// NR41 (length)
							snd_NOISE_length = (uint16_t)(64 - (value & 0x3F));
							snd_NOISE_len_cntr = snd_NOISE_length;

							value = 0;
							break;

						case 0x79:												// NR42 (envelope)
							snd_NOISE_st_vol = (uint8_t)((value & 0xF0) >> 4);
							snd_NOISE_env_add = (value & 8) > 0;
							snd_NOISE_per = (uint8_t)(value & 7);

							// several glitchy effects happen when writing to snd_NRx2 during audio playing
							if (((snd_Audio_Regs[0x19] & 7) == 0) && !snd_NOISE_vol_done) { snd_NOISE_vol_state++; }
							else if ((snd_Audio_Regs[0x19] & 8) == 0) { snd_NOISE_vol_state += 2; }

							if (((snd_Audio_Regs[0x19] ^ value) & 8) > 0) { snd_NOISE_vol_state = (uint8_t)(0x10 - snd_NOISE_vol_state); }

							snd_NOISE_vol_state &= 0xF;
							if ((value & 0xF8) == 0) { snd_NOISE_enable = false; }
							break;

						case 0x7A:
						case 0x7B:
							// not writable
							value = 0;
							break;

						case 0x7C:												// NR43 (shift)
							snd_NOISE_clk_shft = (uint8_t)((value & 0xF0) >> 4);
							snd_NOISE_wdth_md = (value & 8) > 0;
							snd_NOISE_div_code = (uint8_t)(value & 7);
							// Mickey's Dangerous Chase requires writes here to take effect immediately (for sound of taking damage)
							snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
							break;

						case 0x7D:												// NR44 (trigger)
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

							snd_NOISE_len_en = (value & 0x40) > 0;

							value &= 0x40;
							break;

						case 0x7E:
						case 0x7F:
							// not writable
							value = 0;
							break;

						case 0x80:												// NR50 (ctrl)
							snd_CTRL_vol_L = (uint8_t)((value & 0x70) >> 4);
							snd_CTRL_vol_R = (uint8_t)(value & 7);

							value &= 0x77;
							break;

						case 0x81:												// NR51 (ctrl)
							snd_CTRL_noise_L_en = (value & 0x80) > 0;
							snd_CTRL_wave_L_en = (value & 0x40) > 0;
							snd_CTRL_sq2_L_en = (value & 0x20) > 0;
							snd_CTRL_sq1_L_en = (value & 0x10) > 0;
							snd_CTRL_noise_R_en = (value & 8) > 0;
							snd_CTRL_wave_R_en = (value & 4) > 0;
							snd_CTRL_sq2_R_en = (value & 2) > 0;
							snd_CTRL_sq1_R_en = (value & 1) > 0;
							break;

						case 0x82:                                              // GB Low (ctrl)
							snd_Chan_Mult = snd_Chan_Mult_Table[value & 3];

							snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 6 : 3;
							snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 6 : 3;

							value &= 0xF;
							break;

						case 0x83:                                              // GB High (ctrl)
							snd_FIFO_A_Enable_R = (value & 1) == 1;
							snd_FIFO_A_Enable_L = (value & 2) == 2;

							snd_FIFO_B_Enable_R = (value & 0x10) == 0x10;
							snd_FIFO_B_Enable_L = (value & 0x20) == 0x20;

							snd_FIFO_A_Timer = (value & 0x04) >> 2;
							snd_FIFO_B_Timer = (value & 0x40) >> 6;

							if ((value & 0x08) == 0x08)
							{
								for (int i = 0; i < 32; i++)
								{
									snd_FIFO_A[i] = 0;
								}

								snd_FIFO_A_ptr = 0;
							}

							if ((value & 0x80) == 0x80)
							{
								for (int i = 0; i < 32; i++)
								{
									snd_FIFO_B[i] = 0;
								}

								snd_FIFO_B_ptr = 0;
							}

							value &= 0x77;
							break;

						case 0x84:												// NR52 (ctrl)						
							// NOTE: Make sure to do the power off first since it will call the write_reg function again
							if ((value & 0x80) == 0) { power_off(); }
							snd_CTRL_power = (value & 0x80) > 0;

							value &= 0x80;
							break;

						case 0x85:
							// not writable
							value = 0;
							break;

						case 0x86:
						case 0x87:
							// not writable
							value = 0;
							break;

						case 0x88:                                              // Bias Control
							snd_Bias_Low = value;

							snd_BIAS_Offset &= 0xFFFFFF00;
							snd_BIAS_Offset |= snd_Bias_Low;
							break;

						case 0x89:
							snd_Bias_High = value;

							snd_BIAS_Offset &= 0x000000FF;
							snd_BIAS_Offset |= ((snd_Bias_High & 0x3) << 8);

							if ((snd_BIAS_Offset & 0x200) == 0x200)
							{
								snd_BIAS_Offset |= 0xFFFFFC00;
							}

							switch (value & 0xC0)
							{
							case 0x00: snd_Sample_Rate = 0x1FF; break;
							case 0x40: snd_Sample_Rate = 0xFF; break;
							case 0x80: snd_Sample_Rate = 0x7F; break;
							case 0xC0: snd_Sample_Rate = 0x3F; break;
							}

							break;

						case 0x8A:
						case 0x8B:
							// not writable
							value = 0;
							break;
					}

					snd_Audio_Regs[addr - 0x60] = value;
					snd_Update_NR52();
				}
				else
				{

				}
			}
			// when power is off, only length counters and waveRAM are effected by writes
			// FIFO registers are also read / writable
			else
			{
				switch (addr)
				{
					case 0x82:                                              // GB Low (ctrl)
						snd_Chan_Mult = snd_Chan_Mult_Table[value & 3];

						snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 6 : 3;
						snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 6 : 3;

						value &= 0xF;
						break;

					case 0x83:                                              // GB High (ctrl)
						snd_FIFO_A_Enable_R = (value & 1) == 1;
						snd_FIFO_A_Enable_L = (value & 2) == 2;

						snd_FIFO_B_Enable_R = (value & 0x10) == 0x10;
						snd_FIFO_B_Enable_L = (value & 0x20) == 0x20;

						snd_FIFO_A_Timer = (value & 0x04) >> 2;
						snd_FIFO_B_Timer = (value & 0x40) >> 6;

						if ((value & 0x08) == 0x08)
						{
							for (int i = 0; i < 32; i++)
							{
								snd_FIFO_A[i] = 0;
							}

							snd_FIFO_A_ptr = 0;
						}

						if ((value & 0x80) == 0x80)
						{
							for (int i = 0; i < 32; i++)
							{
								snd_FIFO_B[i] = 0;
							}

							snd_FIFO_B_ptr = 0;
						}

						value &= 0x77;
						break;
					case 0x84:                                        // NR52 (ctrl)
						snd_CTRL_power = (value & 0x80) > 0;
						if (snd_CTRL_power)
						{
							snd_Sequencer_reset_cd = 4;
						}

						value &= 0x80;
						snd_Audio_Regs[addr - 0x60] = value;
						snd_Update_NR52();
						break;
					case 0x88:                                        // Bias Control
						snd_Bias_Low = value;

						snd_BIAS_Offset &= 0xFFFFFF00;
						snd_BIAS_Offset |= snd_Bias_Low;

						snd_Audio_Regs[addr - 0x60] = value;
						break;
					case 0x89:                                        // Bias Control
						snd_Bias_High = value;

						snd_BIAS_Offset &= 0x000000FF;
						snd_BIAS_Offset |= ((snd_Bias_High & 0x3) << 8);

						if ((snd_BIAS_Offset & 0x200) == 0x200)
						{
							snd_BIAS_Offset |= 0xFFFFFC00;
						}

						switch (value & 0xC0)
						{
						case 0x00: snd_Sample_Rate = 0x1FF; break;
						case 0x40: snd_Sample_Rate = 0xFF; break;
						case 0x80: snd_Sample_Rate = 0x7F; break;
						case 0xC0: snd_Sample_Rate = 0x3F; break;
						}

						snd_Audio_Regs[addr - 0x60] = value;
						break;
				}

				snd_Audio_Regs[addr - 0x60] = value;
			}
		}

		void power_off()
		{
			for (uint32_t i = 0x60; i < 0x82; i++)
			{
				snd_Write_Reg_8(i, 0);
			}

			for (int i = 0; i < 32; i++)
			{
				snd_FIFO_A[i] = 0;
				snd_FIFO_B[i] = 0;
			}

			snd_FIFO_A_ptr = snd_FIFO_B_ptr = 0;

			// duty and length are reset
			snd_SQ1_duty_cntr = snd_SQ2_duty_cntr = 0;

			// reset state variables
			snd_SQ1_enable = snd_SQ1_swp_enable = snd_SQ2_enable = snd_WAVE_enable = snd_NOISE_enable = false;

			snd_SQ1_len_en = snd_SQ2_len_en = snd_WAVE_len_en = snd_NOISE_len_en = false;

			snd_SQ1_output = snd_SQ2_output = snd_WAVE_output = snd_NOISE_output = 0;

			// on GBC, lengths are also reset
			snd_SQ1_length = snd_SQ2_length = snd_WAVE_length = snd_NOISE_length = 0;
			snd_SQ1_len_cntr = snd_SQ2_len_cntr = snd_WAVE_len_cntr = snd_NOISE_len_cntr = 0;

			snd_FIFO_A_Sample = snd_FIFO_B_Sample = 0;

			snd_FIFO_A_Output = snd_FIFO_B_Output = 0;

			snd_Update_NR52();
		}

		void snd_Reset()
		{
			for (int i = 0; i < 32; i++)
			{
				snd_Wave_RAM[i] = (uint8_t)(((i & 1) == 0) ? 0 : 0xFF);
				snd_FIFO_A[i] = 0;
				snd_FIFO_B[i] = 0;
			}

			for (int i = 0; i < 0x30; i++)
			{
				snd_Audio_Regs[i] = 0;
			}

			for (int i = 0; i < 0x30; i++)
			{
				snd_Write_Reg_8((uint32_t)(0x60 + i), 0);
			}

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

			snd_FIFO_A_ptr = snd_FIFO_B_ptr = 0;

			snd_FIFO_A_Sample = snd_FIFO_B_Sample = 0;

			snd_FIFO_A_Output = snd_FIFO_B_Output = 0;

			snd_BIAS_Offset = 0;

			snd_Sample_Rate = 0x1FF;

			snd_Update_NR52();
		}

		void snd_Update_NR52()
		{
			snd_Audio_Regs[0x24] = (uint8_t)(
				((snd_CTRL_power ? 1 : 0) << 7) |
				(snd_SQ1_enable ? 1 : 0) |
				((snd_SQ2_enable ? 1 : 0) << 1) |
				((snd_WAVE_enable ? 1 : 0) << 2) |
				((snd_NOISE_enable ? 1 : 0) << 3));
		}

		uint8_t* snd_SaveState(uint8_t* saver)
		{
			saver = byte_array_saver(snd_Audio_Regs, saver, 0x30);

			saver = byte_array_saver(snd_Wave_RAM, saver, 32);

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

			// FIFOs
			saver = int_saver(snd_FIFO_A_ptr, saver);
			saver = int_saver(snd_FIFO_B_ptr, saver);
			saver = int_saver(snd_FIFO_A_Timer, saver);
			saver = int_saver(snd_FIFO_B_Timer, saver);
			saver = int_saver(snd_FIFO_A_Output, saver);
			saver = int_saver(snd_FIFO_B_Output, saver);
			saver = int_saver(snd_FIFO_A_Mult, saver);
			saver = int_saver(snd_FIFO_B_Mult, saver);

			saver = byte_array_saver(snd_FIFO_A, saver, 32);
			saver = byte_array_saver(snd_FIFO_B, saver, 32);

			saver = byte_array_saver(snd_FIFO_A_Data, saver, 4);
			saver = byte_array_saver(snd_FIFO_B_Data, saver, 4);

			saver = byte_saver(snd_FIFO_A_Sample, saver); 
			saver = byte_saver(snd_FIFO_B_Sample, saver);

			saver = bool_saver(snd_FIFO_A_Tick, saver);
			saver = bool_saver(snd_FIFO_B_Tick, saver);
			saver = bool_saver(snd_FIFO_A_Enable_L, saver);
			saver = bool_saver(snd_FIFO_B_Enable_L, saver);
			saver = bool_saver(snd_FIFO_A_Enable_R, saver);
			saver = bool_saver(snd_FIFO_B_Enable_R, saver);

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

			return saver;
		}

		uint8_t* snd_LoadState(uint8_t* loader)
		{
			loader = byte_array_loader(snd_Audio_Regs, loader, 0x30);

			loader = byte_array_loader(snd_Wave_RAM, loader, 32);

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

			// FIFOs
			loader = int_loader(&snd_FIFO_A_ptr, loader);
			loader = int_loader(&snd_FIFO_B_ptr, loader);
			loader = int_loader(&snd_FIFO_A_Timer, loader);
			loader = int_loader(&snd_FIFO_B_Timer, loader);
			loader = int_loader(&snd_FIFO_A_Output, loader);
			loader = int_loader(&snd_FIFO_B_Output, loader);
			loader = int_loader(&snd_FIFO_A_Mult, loader);
			loader = int_loader(&snd_FIFO_B_Mult, loader);

			loader = byte_array_loader(snd_FIFO_A, loader, 32);
			loader = byte_array_loader(snd_FIFO_B, loader, 32);

			loader = byte_array_loader(snd_FIFO_A_Data, loader, 4);
			loader = byte_array_loader(snd_FIFO_B_Data, loader, 4);

			loader = byte_loader(&snd_FIFO_A_Sample, loader);
			loader = byte_loader(&snd_FIFO_B_Sample, loader);

			loader = bool_loader(&snd_FIFO_A_Tick, loader);
			loader = bool_loader(&snd_FIFO_B_Tick, loader);
			loader = bool_loader(&snd_FIFO_A_Enable_L, loader);
			loader = bool_loader(&snd_FIFO_B_Enable_L, loader);
			loader = bool_loader(&snd_FIFO_A_Enable_R, loader);
			loader = bool_loader(&snd_FIFO_B_Enable_R, loader);

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
			saver = bool_saver(double_speed, saver);
			saver = bool_saver(speed_switch, saver);
			saver = bool_saver(HDMA_transfer, saver);

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
			saver = int_saver(cpu_state_hold, saver);
			saver = int_saver(clear_counter, saver);
			saver = int_saver(IR_write, saver);
			saver = int_saver(RAM_Bank, saver);
			saver = int_saver(RAM_Bank_ret, saver);

			saver = long_saver(bus_access_time, saver);

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

			if (Cart_RAM_Length != 0)
			{
				saver = byte_array_saver(Cart_RAM, saver, Cart_RAM_Length);
			}

			saver = snd_SaveState(saver);
			saver = ppu_SaveState(saver);
			saver = dma_SaveState(saver);
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
			loader = bool_loader(&double_speed, loader);
			loader = bool_loader(&speed_switch, loader);
			loader = bool_loader(&HDMA_transfer, loader);

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
			loader = int_loader(&cpu_state_hold, loader);
			loader = int_loader(&clear_counter, loader);
			loader = int_loader(&IR_write, loader);
			loader = int_loader(&RAM_Bank, loader);
			loader = int_loader(&RAM_Bank_ret, loader);

			loader = long_loader(&bus_access_time, loader);
			
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

			if (Cart_RAM_Length != 0)
			{	
				loader = byte_array_loader(Cart_RAM, loader, Cart_RAM_Length);
			}

			loader = snd_LoadState(loader);
			loader = ppu_LoadState(loader);
			loader = dma_LoadState(loader);
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