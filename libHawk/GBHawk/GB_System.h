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
				if ((REG_FFFF& 0x10) == 0x10) { cpu_FlagI = true; }
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
					if (Is_GBC)
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
				cpu_Instr_Type = cpu_Instr_Type_List[op];
			}

			cpu_CB_Prefix = false;

			cpu_was_FlagI = cpu_FlagI;
		}

		inline void cpu_Halt_Enter();

		inline void cpu_Halt_Ex(uint8_t param);

		inline void cpu_Execute_One();

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

			//if (interrupt_src_reg.Bit(bit_check) && interrupt_enable_reg.Bit(bit_check)) { cpu_Int_Src = bit_check; cpu_Int_Clear = (byte)(1 << bit_check); }

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
			"                                                                                                                                                                                                                                                                                          ";
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
			reg_state.append(val_char_1, sprintf_s(val_char_1, 17, "%16lld", Cycle_Count));

			while (reg_state.length() < 282)
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
			reg_state.append(val_char_1, sprintf_s(val_char_1, 4, "%3u", ppu_LY));
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

			trace_string.append(":  ");

			uint16_t dis_pc = cpu_RegPCget();
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

		uint8_t cpu_Get_Disasm_Opcode(uint16_t addr);

		string cpu_Disassemble(uint16_t pc)
		{

			uint16_t diff = pc;

			uint8_t op = Peek_Memory(pc++);

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
			saver = bool_saver(Double_Speed, saver);
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
			loader = bool_loader(&Double_Speed, loader);
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