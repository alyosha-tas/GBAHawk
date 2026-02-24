#ifndef GBA_System_H
#define GBA_System_H

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

// Todo:

/*	Look into implementation of multiplication algorithm that implements carry flag
*
*	Fix details of ppu open bus implementation
*
*	Fix RTC tests
*
*	Redo SIO implementation
* 
*	Fix FIFO DMA shutdown bug
* 
*/

//Message_String = "Complete: " + to_string(ser_GBP_Transfer_Count) + " " + to_string(ser_Data_0 & 0xFF) + " " + to_string(CycleCount);

//MessageCallback(Message_String.length());

namespace GBAHawk
{
	class MemoryManager;
	class Mappers;

	class GBA_System
	{
	public:
		
		Mappers* mapper_pntr = nullptr;

		uint8_t* Cart_RAM = nullptr;
		uint32_t Cart_RAM_Length = 0;
		string Message_String = "";

		void (*MessageCallback)(int);

	# pragma region General System and Prefetch

		uint32_t video_buffer[240 * 160] = { };

		uint32_t GBP_TRansfer_List[18] = { 0x0000494E, 0x0000494E, 0xB6B1494E, 0xB6B1544E, 0xABB1544E, 0xABB14E45, 0xB1BA4E45, 0xB1BA4F44, 0xB0BB4F44,
										   0xB0BB8002, 0x10000010, 0x20000013, 0x30000003, 0x30000003, 0x30000003, 0x30000003, 0x30000003, 0x00000000 };

		void Frame_Advance();
		bool SubFrame_Advance(uint32_t reset_cycle);
		inline void Single_Step();

		uint8_t Read_Memory_8(uint32_t addr);
		uint16_t Read_Memory_16(uint32_t addr);
		uint32_t Read_Memory_32(uint32_t addr);

		void Write_Memory_8(uint32_t addr, uint8_t value);
		void Write_Memory_16(uint32_t addr, uint16_t value);
		void Write_Memory_32(uint32_t addr, uint32_t value);

		void Read_Memory_16_DMA(uint32_t addr, uint32_t chan);
		void Read_Memory_32_DMA(uint32_t addr, uint32_t chan);

		void Write_Memory_16_DMA(uint32_t addr, uint16_t value, uint32_t chan);
		void Write_Memory_32_DMA(uint32_t addr, uint32_t value, uint32_t chan);

		uint8_t Peek_Memory_8(uint32_t addr);

		// General Variables
		bool Is_Lag;
		bool VBlank_Rise;
		bool All_RAM_Disable, WRAM_Enable;

		bool INT_Master_On;
		bool Cart_RAM_Present;
		bool Is_EEPROM;
		bool EEPROM_Wiring; // when true, can access anywhere in 0xDxxxxxx range, otheriwse only 0xDFFFFE0

		bool delays_to_process;
		bool IRQ_Write_Delay;

		bool VRAM_32_Check, PALRAM_32_Check;
		bool VRAM_32_Delay, PALRAM_32_Delay;
		bool IRQ_Delays, Misc_Delays;
		bool FIFO_DMA_A_Delay, FIFO_DMA_B_Delay;
		bool DMA_Any_Start, DMA_Any_IRQ;
		bool Halt_Enter, Halt_Leave;
		bool GBP_Mode_Enabled;
		bool Rumble_State;

		uint8_t Post_Boot, Halt_CTRL;

		uint8_t ext_num = 0; // zero here means disconnected

		bool is_linked_system = false;

		bool Is_GBP = false;

		uint16_t INT_EN, INT_Flags, INT_Master, Wait_CTRL;
		uint16_t INT_Flags_Gather, INT_Flags_Use;
		uint16_t controller_state, controller_state_old;
		uint16_t PALRAM_32W_Value, VRAM_32W_Value;
		uint16_t FIFO_DMA_A_cd, FIFO_DMA_B_cd;
		uint16_t Halt_Enter_cd, Halt_Leave_cd;
		uint16_t Halt_Held_CPU_Instr;

		uint32_t PALRAM_32W_Addr, VRAM_32W_Addr;
		uint32_t Memory_CTRL, ROM_Length;
		uint32_t Last_BIOS_Read;
		uint32_t WRAM_Waits, SRAM_Waits;

		uint32_t ROM_Waits_0_N, ROM_Waits_1_N, ROM_Waits_2_N, ROM_Waits_0_S, ROM_Waits_1_S, ROM_Waits_2_S;

		bool DMA_Start_Delay[4] = { };
		bool DMA_IRQ_Delay[4] = { };

		uint8_t WRAM[0x40000] = { };
		uint8_t IWRAM[0x8000] = { };
		uint8_t PALRAM[0x400] = { };
		uint8_t VRAM[0x18000] = { };
		uint8_t OAM[0x400] = { };

		// not stated
		uint8_t BIOS[0x4000] = { };
		uint8_t ROM[0x6000000] = { };

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

		// Prefetcher
		bool pre_Cycle_Glitch;

		bool pre_Run, pre_Enable;

		bool pre_Force_Non_Seq;

		bool pre_Buffer_Was_Full;

		bool pre_Boundary_Reached;

		bool pre_Following;

		bool pre_Inactive;

		uint32_t pre_Read_Addr, pre_Check_Addr;
		uint32_t pre_Buffer_Cnt;

		uint32_t pre_Fetch_Cnt, pre_Fetch_Wait;

		GBA_System()
		{
			System_Reset();
		}

		void System_Reset() 
		{
			delays_to_process = false;

			IRQ_Write_Delay = false;

			IRQ_Delays = Misc_Delays = VRAM_32_Delay = PALRAM_32_Delay = false;

			FIFO_DMA_A_Delay = FIFO_DMA_B_Delay = false;

			DMA_Any_Start = DMA_Any_IRQ = false;

			for (int i = 0; i < 4; i++) { DMA_IRQ_Delay[i] = false; DMA_Start_Delay[i] = false; }

			Halt_Enter =  Halt_Leave = false;

			GBP_Mode_Enabled = false;

			Rumble_State = false;

			VRAM_32_Check = PALRAM_32_Check = false;

			controller_state = 0x3FF;

			Memory_CTRL = 0;

			Last_BIOS_Read = 0;

			WRAM_Waits = SRAM_Waits = 0;

			ROM_Waits_0_N = ROM_Waits_1_N = ROM_Waits_2_N = ROM_Waits_0_S = ROM_Waits_1_S = ROM_Waits_2_S = 0;

			INT_EN = INT_Flags = INT_Master = Wait_CTRL = 0;

			INT_Flags_Gather = INT_Flags_Use = 0;

			Post_Boot = Halt_CTRL = 0;

			PALRAM_32W_Value = VRAM_32W_Value = 0;
			
			FIFO_DMA_A_cd = FIFO_DMA_B_cd = 0;

			Halt_Enter_cd = Halt_Leave_cd = Halt_Held_CPU_Instr = 0;

			All_RAM_Disable = WRAM_Enable = false;

			INT_Master_On = false;

			snd_Reset();
			ppu_Reset();
			dma_Reset();
			pre_Reset();
			ser_Reset();
			tim_Reset();
			cpu_Reset();

			// default memory config hardware initialized
			Update_Memory_CTRL(0x0D000020);

			uint32_t startup_color = 0xFFF8F8F8;

			for (int i = 0; i < 240*160; i++)
			{
				video_buffer[i] = startup_color;
			}

			for (int i = 0; i < 0x8000; i++)
			{
				IWRAM[i] = 0;
			}

			for (int i = 0; i < 0x40000; i++)
			{
				WRAM[i] = 0;
			}
		}

		void Update_Memory_CTRL(uint32_t value)
		{
			All_RAM_Disable = (value & 1) == 1;

			WRAM_Enable = (value & 0x20) == 0x20;

			WRAM_Waits = (uint32_t)(((~value) >> 24) & 0xF);

			Memory_CTRL = value;
		}

		void do_controller_check(bool from_reg)
		{
			// only check interrupts on new button press or change in register
			bool do_check = false;

			if (from_reg)
			{
				do_check = true;
			}
			for (int i = 0; i < 10; i++)
			{
				if (((controller_state >> i) & 1) == 0)
				{
					if (((controller_state_old >> i) & 1) == 1)
					{
						do_check = true;
					}
				}
			}
			
			if (do_check)
			{
				if ((key_CTRL & 0x4000) == 0x4000)
				{
					if ((key_CTRL & 0x8000) == 0x8000)
					{
						if ((key_CTRL & ~controller_state & 0x3FF) == (key_CTRL & 0x3FF))
						{
							// doesn't trigger an interrupt if no keys are selected. (see joypad.gba test rom)
							if ((key_CTRL & 0x3FF) != 0)
							{
								Trigger_IRQ(12);
							}
						}
					}
					else
					{
						if ((key_CTRL & ~controller_state & 0x3FF) != 0)
						{
							Trigger_IRQ(12);
						}
					}
				}
			}		
		}

		// only on writes, it is possible to trigger an interrupt with and mode and no keys selected or pressed
		void do_controller_check_glitch()
		{
			if ((key_CTRL & 0xC3FF) == 0xC000)
			{
				if ((controller_state & 0x3FF) == 0x3FF)
				{
					Trigger_IRQ(12);
				}
			}
		}

		void Trigger_IRQ(uint16_t bit)
		{
			INT_Flags_Gather |= (uint16_t)(1 << bit);

			delays_to_process = true;
			IRQ_Write_Delay = true;
			IRQ_Delays = true;
		}

		void pre_Reg_Write(uint16_t value)
		{
			if (!pre_Enable && ((value & 0x4000) == 0x4000))
			{
				// set read address to current cpu address
				pre_Check_Addr = 0;
				pre_Buffer_Cnt = 0;
				pre_Fetch_Cnt = 0;
				pre_Inactive = true;
				pre_Run = true;
			}

			if (pre_Enable && ((value & 0x4000) != 0x4000))
			{
				pre_Force_Non_Seq = true;

				if (pre_Fetch_Cnt == 0)
				{
					// if in ARM mode finish the 32 bit access
					if ((pre_Buffer_Cnt & 1) == 0) { pre_Run = false; }
					else if (cpu_Thumb_Mode) { pre_Run = false; }

					if (pre_Buffer_Cnt == 0) { pre_Check_Addr = 0; }
				}
			}

			pre_Enable = (value & 0x4000) == 0x4000;
		}

		void pre_Reset()
		{
			VBlank_Rise = false;

			pre_Read_Addr = pre_Check_Addr = 0;
			pre_Buffer_Cnt = 0;

			pre_Fetch_Cnt = pre_Fetch_Wait = 0;

			pre_Cycle_Glitch;

			pre_Run = pre_Enable = false;

			pre_Force_Non_Seq = false;

			pre_Buffer_Was_Full = pre_Boundary_Reached = false;

			pre_Following = false;

			pre_Inactive = true;
		}

		void On_VBlank()
		{
			// things to do on vblank
		}

	#pragma endregion

	#pragma region HW Registers

		uint8_t Read_Registers_8(uint32_t addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_8(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_8(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_8(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_8(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_8(addr);
			}

			uint8_t ret = 0;

			switch (addr)
			{
				case 0x200: ret = (uint8_t)(INT_EN & 0xFF); break;
				case 0x201: ret = (uint8_t)((INT_EN & 0xFF00) >> 8); break;
				case 0x202: ret = (uint8_t)(INT_Flags & 0xFF); break;
				case 0x203: ret = (uint8_t)((INT_Flags & 0xFF00) >> 8); break;
				case 0x204: ret = (uint8_t)(Wait_CTRL & 0xFF); break;
				case 0x205: ret = (uint8_t)((Wait_CTRL & 0xFF00) >> 8); break;
				case 0x206: ret = 0; break;
				case 0x207: ret = 0; break;
				case 0x208: ret = (uint8_t)(INT_Master & 0xFF); break;
				case 0x209: ret = (uint8_t)((INT_Master & 0xFF00) >> 8); break;
				case 0x20A: ret = 0; break;
				case 0x20B: ret = 0; break;

				case 0x300: ret = Post_Boot; break;
				case 0x301: ret = Halt_CTRL; break;
				case 0x302: ret = 0; break;
				case 0x303: ret = 0; break;

				default: ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (uint32_t)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		uint16_t Read_Registers_16(uint32_t addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_16(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_16(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_16(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_16(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_16(addr);
			}

			uint16_t ret = 0;

			switch (addr)
			{
				case 0x200: ret = INT_EN; break;
				case 0x202: ret = INT_Flags; break;
				case 0x204: ret = Wait_CTRL; break;
				case 0x206: ret = 0; break;
				case 0x208: ret = INT_Master; break;
				case 0x20A: ret = 0; break;

				case 0x300: ret = (uint16_t)((Halt_CTRL << 8) | Post_Boot); break;
				case 0x302: ret = 0; break;

				default: ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		uint32_t Read_Registers_32(uint32_t addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_32(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_32(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_32(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_32(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_32(addr);
			}

			uint32_t ret = 0;

			switch (addr)
			{
				case 0x200: ret = (uint32_t)((INT_Flags << 16) | INT_EN); break;
				case 0x204: ret = (uint32_t)(0x00000000 | Wait_CTRL); break;

				case 0x208: ret = (uint32_t)(0x00000000 | INT_Master); break;

				case 0x300: ret = (uint32_t)(0x00000000 | (Halt_CTRL << 8) | Post_Boot); break;

				default: ret = cpu_Last_Bus_Value; break;
			}

			return ret;
		}

		void Write_Registers_8(uint32_t addr, uint8_t value)
		{
			//Message_String = "wr8 " + to_string(addr) + " " + to_string(value) + " " + to_string(ppu_LY) + " " + to_string(ppu_Cycle) + " " + to_string(CycleCount);

			//MessageCallback(Message_String.length());
			
			if (addr < 0x60)
			{
				ppu_Write_Reg_8(addr, value);
			}
			else if (addr < 0xB0)
			{
				snd_Write_Reg_8(addr, value);
			}
			else if (addr < 0x100)
			{
				dma_Write_Reg_8(addr, value);
			}
			else if (addr < 0x120)
			{
				tim_Write_Reg_8(addr, value);
			}
			else if (addr < 0x200)
			{
				ser_Write_Reg_8(addr, value);
			}
			else
			{
				switch (addr)
				{
					case 0x200: Update_INT_EN((uint16_t)((INT_EN & 0xFF00) | value)); break;
					case 0x201: Update_INT_EN((uint16_t)((INT_EN & 0x00FF) | (value << 8))); break;
					case 0x202: Update_INT_Flags((uint16_t)((INT_Flags & 0xFF00) | value)); break;
					case 0x203: Update_INT_Flags((uint16_t)((INT_Flags & 0x00FF) | (value << 8))); break;
					case 0x204: Update_Wait_CTRL((uint16_t)((Wait_CTRL & 0xFF00) | value)); break;
					case 0x205: Update_Wait_CTRL((uint16_t)((Wait_CTRL & 0x00FF) | (value << 8))); break;

					case 0x208: Update_INT_Master((uint16_t)((INT_Master & 0xFF00) | value)); break;
					case 0x209: Update_INT_Master((uint16_t)((INT_Master & 0x00FF) | (value << 8))); break;

					case 0x300: Update_Post_Boot(value); break;
					case 0x301: Update_Halt_CTRL(value); break;
				}
			}
		}

		void Write_Registers_16(uint32_t addr, uint16_t value)
		{
			//Message_String = "wr16 " + to_string(addr) + " " + to_string(value) + " " + to_string(ppu_LY) + " " + to_string(ppu_Cycle) + " " + to_string(CycleCount);

			//MessageCallback(Message_String.length());
			
			if (addr < 0x60)
			{
				ppu_Write_Reg_16(addr, value);
			}
			else if (addr < 0xB0)
			{
				snd_Write_Reg_16(addr, value);
			}
			else if (addr < 0x100)
			{
				dma_Write_Reg_16(addr, value);
			}
			else if (addr < 0x120)
			{
				tim_Write_Reg_16(addr, value);
			}
			else if (addr < 0x200)
			{
				ser_Write_Reg_16(addr, value);
			}
			else
			{
				switch (addr)
				{
					case 0x200: Update_INT_EN(value); break;
					case 0x202: Update_INT_Flags(value); break;
					case 0x204: Update_Wait_CTRL(value); break;

					case 0x208: Update_INT_Master(value); break;

					case 0x300: Update_Post_Boot((uint8_t)(value & 0xFF)); Update_Halt_CTRL((uint8_t)((value >> 8) & 0xFF)); break;
				}
			}
		}

		void Write_Registers_32(uint32_t addr, uint32_t value)
		{
			//Message_String = "wr32 " + to_string(addr) + " " + to_string(value) + " " + to_string(ppu_LY) + " " + to_string(ppu_Cycle) + " " + to_string(CycleCount);

			//MessageCallback(Message_String.length());
			
			if (addr < 0x60)
			{
				ppu_Write_Reg_32(addr, value);
			}
			else if (addr < 0xB0)
			{
				snd_Write_Reg_32(addr, value);
			}
			else if (addr < 0x100)
			{
				dma_Write_Reg_32(addr, value);
			}
			else if (addr < 0x120)
			{
				tim_Write_Reg_32(addr, value);
			}
			else if (addr < 0x200)
			{
				ser_Write_Reg_32(addr, value);
			}
			else
			{
				switch (addr)
				{
					case 0x200: Update_INT_EN((uint16_t)(value & 0xFFFF));
								Update_INT_Flags((uint16_t)((value >> 16) & 0xFFFF)); break;
					case 0x204: Update_Wait_CTRL((uint16_t)(value & 0xFFFF)); break;

					case 0x208: Update_INT_Master((uint16_t)(value & 0xFFFF)); break;

					case 0x300: Update_Post_Boot((uint8_t)(value & 0xFF)); Update_Halt_CTRL((uint8_t)((value >> 8) & 0xFF)); break;
				}
			}
		}

		uint8_t Get_Registers_Internal(uint32_t addr)
		{
			uint8_t ret = 0;
			
			if (addr < 0x60)
			{
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

					case 0x10: ret = (uint8_t)(ppu_BG_X[0] & 0xFF); break;
					case 0x11: ret = (uint8_t)((ppu_BG_X[0] >> 8) & 0xFF); break;
					case 0x12: ret = (uint8_t)(ppu_BG_Y[0] & 0xFF); break;
					case 0x13: ret = (uint8_t)((ppu_BG_Y[0] >> 8) & 0xFF); break;
					case 0x14: ret = (uint8_t)(ppu_BG_X[1] & 0xFF); break;
					case 0x15: ret = (uint8_t)((ppu_BG_X[1] >> 8) & 0xFF); break;
					case 0x16: ret = (uint8_t)(ppu_BG_Y[1] & 0xFF); break;
					case 0x17: ret = (uint8_t)((ppu_BG_Y[1] >> 8) & 0xFF); break;
					case 0x18: ret = (uint8_t)(ppu_BG_X[2] & 0xFF); break;
					case 0x19: ret = (uint8_t)((ppu_BG_X[2] >> 8) & 0xFF); break;
					case 0x1A: ret = (uint8_t)(ppu_BG_Y[2] & 0xFF); break;
					case 0x1B: ret = (uint8_t)((ppu_BG_Y[2] >> 8) & 0xFF); break;
					case 0x1C: ret = (uint8_t)(ppu_BG_X[3] & 0xFF); break;
					case 0x1D: ret = (uint8_t)((ppu_BG_X[3] >> 8) & 0xFF); break;
					case 0x1E: ret = (uint8_t)(ppu_BG_Y[3] & 0xFF); break;
					case 0x1F: ret = (uint8_t)((ppu_BG_Y[3] >> 8) & 0xFF); break;

					case 0x20: ret = (uint8_t)(ppu_BG_Rot_A[2] & 0xFF); break;
					case 0x21: ret = (uint8_t)((ppu_BG_Rot_A[2] >> 8) & 0xFF); break;
					case 0x22: ret = (uint8_t)(ppu_BG_Rot_B[2] & 0xFF); break;
					case 0x23: ret = (uint8_t)((ppu_BG_Rot_B[2] >> 8) & 0xFF); break;
					case 0x24: ret = (uint8_t)(ppu_BG_Rot_C[2] & 0xFF); break;
					case 0x25: ret = (uint8_t)((ppu_BG_Rot_C[2] >> 8) & 0xFF); break;
					case 0x26: ret = (uint8_t)(ppu_BG_Rot_D[2] & 0xFF); break;
					case 0x27: ret = (uint8_t)((ppu_BG_Rot_D[2] >> 8) & 0xFF); break;
					case 0x28: ret = (uint8_t)(ppu_BG_Ref_X[2] & 0xFF); break;
					case 0x29: ret = (uint8_t)((ppu_BG_Ref_X[2] >> 8) & 0xFF); break;
					case 0x2A: ret = (uint8_t)((ppu_BG_Ref_X[2] >> 16) & 0xFF); break;
					case 0x2B: ret = (uint8_t)((ppu_BG_Ref_X[2] >> 24) & 0xFF); break;
					case 0x2C: ret = (uint8_t)(ppu_BG_Ref_Y[2] & 0xFF); break;
					case 0x2D: ret = (uint8_t)((ppu_BG_Ref_Y[2] >> 8) & 0xFF); break;
					case 0x2E: ret = (uint8_t)((ppu_BG_Ref_Y[2] >> 16) & 0xFF); break;
					case 0x2F: ret = (uint8_t)((ppu_BG_Ref_Y[2] >> 24) & 0xFF); break;

					case 0x30: ret = (uint8_t)(ppu_BG_Rot_A[3] & 0xFF); break;
					case 0x31: ret = (uint8_t)((ppu_BG_Rot_A[3] >> 8) & 0xFF); break;
					case 0x32: ret = (uint8_t)(ppu_BG_Rot_B[3] & 0xFF); break;
					case 0x33: ret = (uint8_t)((ppu_BG_Rot_B[3] >> 8) & 0xFF); break;
					case 0x34: ret = (uint8_t)(ppu_BG_Rot_C[3] & 0xFF); break;
					case 0x35: ret = (uint8_t)((ppu_BG_Rot_C[3] >> 8) & 0xFF); break;
					case 0x36: ret = (uint8_t)(ppu_BG_Rot_D[3] & 0xFF); break;
					case 0x37: ret = (uint8_t)((ppu_BG_Rot_D[3] >> 8) & 0xFF); break;
					case 0x38: ret = (uint8_t)(ppu_BG_Ref_X[3] & 0xFF); break;
					case 0x39: ret = (uint8_t)((ppu_BG_Ref_X[3] >> 8) & 0xFF); break;
					case 0x3A: ret = (uint8_t)((ppu_BG_Ref_X[3] >> 16) & 0xFF); break;
					case 0x3B: ret = (uint8_t)((ppu_BG_Ref_X[3] >> 24) & 0xFF); break;
					case 0x3C: ret = (uint8_t)(ppu_BG_Ref_Y[3] & 0xFF); break;
					case 0x3D: ret = (uint8_t)((ppu_BG_Ref_Y[3] >> 8) & 0xFF); break;
					case 0x3E: ret = (uint8_t)((ppu_BG_Ref_Y[3] >> 16) & 0xFF); break;
					case 0x3F: ret = (uint8_t)((ppu_BG_Ref_Y[3] >> 24) & 0xFF); break;

					case 0x40: ret = (uint8_t)(ppu_WIN_Hor_0 & 0xFF); break;
					case 0x41: ret = (uint8_t)((ppu_WIN_Hor_0 & 0xFF00) >> 8); break;
					case 0x42: ret = (uint8_t)(ppu_WIN_Hor_1 & 0xFF); break;
					case 0x43: ret = (uint8_t)((ppu_WIN_Hor_1 & 0xFF00) >> 8); break;
					case 0x44: ret = (uint8_t)(ppu_WIN_Vert_0 & 0xFF); break;
					case 0x45: ret = (uint8_t)((ppu_WIN_Vert_0 & 0xFF00) >> 8); break;
					case 0x46: ret = (uint8_t)(ppu_WIN_Vert_1 & 0xFF); break;
					case 0x47: ret = (uint8_t)((ppu_WIN_Vert_1 & 0xFF00) >> 8); break;

					case 0x48: ret = (uint8_t)(ppu_WIN_In & 0xFF); break;
					case 0x49: ret = (uint8_t)((ppu_WIN_In & 0xFF00) >> 8); break;
					case 0x4A: ret = (uint8_t)(ppu_WIN_Out & 0xFF); break;
					case 0x4B: ret = (uint8_t)((ppu_WIN_Out & 0xFF00) >> 8); break;
					case 0x4C: ret = (uint8_t)(ppu_Mosaic & 0xFF); break;
					case 0x4D: ret = (uint8_t)((ppu_Mosaic & 0xFF00) >> 8); break;

					case 0x50: ret = (uint8_t)(ppu_Special_FX & 0xFF); break;
					case 0x51: ret = (uint8_t)((ppu_Special_FX & 0xFF00) >> 8); break;
					case 0x52: ret = (uint8_t)(ppu_Alpha & 0xFF); break;
					case 0x53: ret = (uint8_t)((ppu_Alpha & 0xFF00) >> 8); break;
					case 0x54: ret = (uint8_t)(ppu_Bright & 0xFF); break;
					case 0x55: ret = (uint8_t)((ppu_Bright & 0xFF00) >> 8); break;

					default: ret = 0;
				}
			}
			else if (addr < 0xB0)
			{
				if (addr < 0x8C)
				{
					ret = snd_Audio_Regs[addr - 0x60];
				}
				else if ((addr < 0xA0) && (addr >= 0x90))
				{
					int ofst = (int)(snd_Wave_Bank + addr - 0x90);

					ret = snd_Wave_RAM[ofst];
				}
				else if (addr < 0xA4)
				{
					ret = snd_FIFO_A_Data[(addr & 3)];
				}
				else if (addr < 0xA8)
				{
					ret = snd_FIFO_B_Data[(addr & 3)];
				}

				return ret;
			}
			else if (addr < 0x100)
			{
				switch (addr)
				{
					case 0xB0: ret = (uint8_t)(dma_SRC[0] & 0xFF); break;
					case 0xB1: ret = (uint8_t)((dma_SRC[0] >> 8) & 0xFF); break;
					case 0xB2: ret = (uint8_t)((dma_SRC[0] >> 16) & 0xFF); break;
					case 0xB3: ret = (uint8_t)((dma_SRC[0] >> 24) & 0xFF); break;
					case 0xB4: ret = (uint8_t)(dma_DST[0] & 0xFF); break;
					case 0xB5: ret = (uint8_t)((dma_DST[0] >> 8) & 0xFF); break;
					case 0xB6: ret = (uint8_t)((dma_DST[0] >> 16) & 0xFF); break;
					case 0xB7: ret = (uint8_t)((dma_DST[0] >> 24) & 0xFF); break;
					case 0xB8: ret = (uint8_t)(dma_CNT[0] & 0xFF); break;
					case 0xB9: ret = (uint8_t)((dma_CNT[0] >> 8) & 0xFF); break;
					case 0xBA: ret = (uint8_t)(dma_CTRL[0] & 0xFF); break;
					case 0xBB: ret = (uint8_t)((dma_CTRL[0] >> 8) & 0xFF); break;

					case 0xBC: ret = (uint8_t)(dma_SRC[1] & 0xFF); break;
					case 0xBD: ret = (uint8_t)((dma_SRC[1] >> 8) & 0xFF); break;
					case 0xBE: ret = (uint8_t)((dma_SRC[1] >> 16) & 0xFF); break;
					case 0xBF: ret = (uint8_t)((dma_SRC[1] >> 24) & 0xFF); break;
					case 0xC0: ret = (uint8_t)(dma_DST[1] & 0xFF); break;
					case 0xC1: ret = (uint8_t)((dma_DST[1] >> 8) & 0xFF); break;
					case 0xC2: ret = (uint8_t)((dma_DST[1] >> 16) & 0xFF); break;
					case 0xC3: ret = (uint8_t)((dma_DST[1] >> 24) & 0xFF); break;
					case 0xC4: ret = (uint8_t)(dma_CNT[1] & 0xFF); break;
					case 0xC5: ret = (uint8_t)((dma_CNT[1] >> 8) & 0xFF); break;
					case 0xC6: ret = (uint8_t)(dma_CTRL[1] & 0xFF); break;
					case 0xC7: ret = (uint8_t)((dma_CTRL[1] >> 8) & 0xFF); break;

					case 0xC8: ret = (uint8_t)(dma_SRC[2] & 0xFF); break;
					case 0xC9: ret = (uint8_t)((dma_SRC[2] >> 8) & 0xFF); break;
					case 0xCA: ret = (uint8_t)((dma_SRC[2] >> 16) & 0xFF); break;
					case 0xCB: ret = (uint8_t)((dma_SRC[2] >> 24) & 0xFF); break;
					case 0xCC: ret = (uint8_t)(dma_DST[2] & 0xFF); break;
					case 0xCD: ret = (uint8_t)((dma_DST[2] >> 8) & 0xFF); break;
					case 0xCE: ret = (uint8_t)((dma_DST[2] >> 16) & 0xFF); break;
					case 0xCF: ret = (uint8_t)((dma_DST[2] >> 24) & 0xFF); break;
					case 0xD0: ret = (uint8_t)(dma_CNT[2] & 0xFF); break;
					case 0xD1: ret = (uint8_t)((dma_CNT[2] >> 8) & 0xFF); break;
					case 0xD2: ret = (uint8_t)(dma_CTRL[2] & 0xFF); break;
					case 0xD3: ret = (uint8_t)((dma_CTRL[2] >> 8) & 0xFF); break;

					case 0xD4: ret = (uint8_t)(dma_SRC[3] & 0xFF); break;
					case 0xD5: ret = (uint8_t)((dma_SRC[3] >> 8) & 0xFF); break;
					case 0xD6: ret = (uint8_t)((dma_SRC[3] >> 16) & 0xFF); break;
					case 0xD7: ret = (uint8_t)((dma_SRC[3] >> 24) & 0xFF); break;
					case 0xD8: ret = (uint8_t)(dma_DST[3] & 0xFF); break;
					case 0xD9: ret = (uint8_t)((dma_DST[3] >> 8) & 0xFF); break;
					case 0xDA: ret = (uint8_t)((dma_DST[3] >> 16) & 0xFF); break;
					case 0xDB: ret = (uint8_t)((dma_DST[3] >> 24) & 0xFF); break;
					case 0xDC: ret = (uint8_t)(dma_CNT[3] & 0xFF); break;
					case 0xDD: ret = (uint8_t)((dma_CNT[3] >> 8) & 0xFF); break;
					case 0xDE: ret = (uint8_t)(dma_CTRL[3] & 0xFF); break;
					case 0xDF: ret = (uint8_t)((dma_CTRL[3] >> 8) & 0xFF); break;

					default: ret = 0;
				}
			}
			else if (addr < 0x120)
			{
				if (addr < 0x110)
				{
					switch (addr)
					{
						case 0x100: ret = (uint8_t)(tim_Reload[0] & 0xFF); break;
						case 0x101: ret = (uint8_t)((tim_Reload[0] & 0xFF00) >> 8); break;
						case 0x102: ret = (uint8_t)(tim_Control[0] & 0xFF); break;
						case 0x103: ret = (uint8_t)((tim_Control[0] & 0xFF00) >> 8); break;

						case 0x104: ret = (uint8_t)(tim_Reload[1] & 0xFF); break;
						case 0x105: ret = (uint8_t)((tim_Reload[1] & 0xFF00) >> 8); break;
						case 0x106: ret = (uint8_t)(tim_Control[1] & 0xFF); break;
						case 0x107: ret = (uint8_t)((tim_Control[1] & 0xFF00) >> 8); break;

						case 0x108: ret = (uint8_t)(tim_Reload[2] & 0xFF); break;
						case 0x109: ret = (uint8_t)((tim_Reload[2] & 0xFF00) >> 8); break;
						case 0x10A: ret = (uint8_t)(tim_Control[2] & 0xFF); break;
						case 0x10B: ret = (uint8_t)((tim_Control[2] & 0xFF00) >> 8); break;

						case 0x10C: ret = (uint8_t)(tim_Reload[3] & 0xFF); break;
						case 0x10D: ret = (uint8_t)((tim_Reload[3] & 0xFF00) >> 8); break;
						case 0x10E: ret = (uint8_t)(tim_Control[3] & 0xFF); break;
						case 0x10F: ret = (uint8_t)((tim_Control[3] & 0xFF00) >> 8); break;
					}
				}
				else
				{
					return 0;
				}
			}
			else if (addr < 0x200)
			{
				switch (addr)
				{
					case 0x120: ret = (uint8_t)(ser_Data_0 & 0xFF); break;
					case 0x121: ret = (uint8_t)((ser_Data_0 & 0xFF00) >> 8); break;
					case 0x122: ret = (uint8_t)(ser_Data_1 & 0xFF); break;
					case 0x123: ret = (uint8_t)((ser_Data_1 & 0xFF00) >> 8); break;
					case 0x124: ret = (uint8_t)(ser_Data_2 & 0xFF); break;
					case 0x125: ret = (uint8_t)((ser_Data_2 & 0xFF00) >> 8); break;
					case 0x126: ret = (uint8_t)(ser_Data_3 & 0xFF); break;
					case 0x127: ret = (uint8_t)((ser_Data_3 & 0xFF00) >> 8); break;
					case 0x128: ret = (uint8_t)(ser_CTRL & 0xFF); break;
					case 0x129: ret = (uint8_t)((ser_CTRL & 0xFF00) >> 8); break;
					case 0x12A: ret = (uint8_t)(ser_Data_M & 0xFF); break;
					case 0x12B: ret = (uint8_t)((ser_Data_M & 0xFF00) >> 8); break;

					case 0x130: ret = (uint8_t)(controller_state & 0xFF); break;
					case 0x131: ret = (uint8_t)((controller_state & 0xFF00) >> 8); break;
					case 0x132: ret = (uint8_t)(key_CTRL & 0xFF); break;
					case 0x133: ret = (uint8_t)((key_CTRL & 0xFF00) >> 8); break;

					case 0x134: ret = (uint8_t)(ser_Mode & 0xFF); break;
					case 0x135: ret = (uint8_t)((ser_Mode & 0xFF00) >> 8); break;
					case 0x136: ret = 0; break;
					case 0x137: ret = 0; break;

					case 0x140: ret = (uint8_t)(ser_CTRL_J & 0xFF); break;
					case 0x141: ret = (uint8_t)((ser_CTRL_J & 0xFF00) >> 8); break;
					case 0x142: ret = 0; break;
					case 0x143: ret = 0; break;

					case 0x150: ret = (uint8_t)(ser_RECV_J & 0xFF); break;
					case 0x151: ret = (uint8_t)((ser_RECV_J & 0xFF00) >> 8); break;
					case 0x152: ret = (uint8_t)((ser_RECV_J & 0xFF0000) >> 16); break;
					case 0x153: ret = (uint8_t)((ser_RECV_J & 0xFF000000) >> 24); break;
					case 0x154: ret = (uint8_t)(ser_TRANS_J & 0xFF); break;
					case 0x155: ret = (uint8_t)((ser_TRANS_J & 0xFF00) >> 8); break;
					case 0x156: ret = (uint8_t)((ser_TRANS_J & 0xFF0000) >> 16); break;
					case 0x157: ret = (uint8_t)((ser_TRANS_J & 0xFF000000) >> 24); break;
					case 0x158: ret = (uint8_t)(ser_STAT_J & 0xFF); break;
					case 0x159: ret = (uint8_t)((ser_STAT_J & 0xFF00) >> 8); break;
					case 0x15A: ret = 0; break;
					case 0x15B: ret = 0; break;

					default: ret = 0;
				}
			}
			else
			{
				switch (addr)
				{
					case 0x200: ret = (uint8_t)(INT_EN & 0xFF); break;
					case 0x201: ret = (uint8_t)((INT_EN & 0xFF00) >> 8); break;
					case 0x202: ret = (uint8_t)(INT_Flags & 0xFF); break;
					case 0x203: ret = (uint8_t)((INT_Flags & 0xFF00) >> 8); break;
					case 0x204: ret = (uint8_t)(Wait_CTRL & 0xFF); break;
					case 0x205: ret = (uint8_t)((Wait_CTRL & 0xFF00) >> 8); break;
					case 0x206: ret = 0; break;
					case 0x207: ret = 0; break;
					case 0x208: ret = (uint8_t)(INT_Master & 0xFF); break;
					case 0x209: ret = (uint8_t)((INT_Master & 0xFF00) >> 8); break;
					case 0x20A: ret = 0; break;
					case 0x20B: ret = 0; break;

					case 0x300: ret = Post_Boot; break;
					case 0x301: ret = Halt_CTRL; break;
					case 0x302: ret = 0; break;
					case 0x303: ret = 0; break;

					default: ret = 0;
				}
			}

			return ret;
		}

		void Update_INT_EN(uint16_t value)
		{
			// changes to IRQ that happen due to writes should take place in 3 cycles
			delays_to_process = true;
			IRQ_Write_Delay = true;
			IRQ_Delays = true;

			INT_EN = value;
		}

		void Update_INT_Flags(uint16_t value)
		{
			// writing one acknowledges interrupt at that bit
			for (int i = 0; i < 14; i++)
			{
				if ((value & (1 << i)) == (1 << i))
				{
					INT_Flags &= (uint16_t)(~(1 << i));

					// if a flag is set on the same cycle a write occurs, it is cleared
					INT_Flags_Gather &= (uint16_t)(~(1 << i));
				}
			}

			// changes to IRQ that happen due to writes should take place in 3 cycles
			delays_to_process = true;
			IRQ_Write_Delay = true;
			IRQ_Delays = true;
		}

		void Update_INT_Master(uint16_t value)
		{
			INT_Master_On = (value & 1) == 1;

			// changes to IRQ that happen due to writes should take place in 3 cycles
			delays_to_process = true;
			IRQ_Write_Delay = true;
			IRQ_Delays = true;

			INT_Master = value;
		}

		void Update_Wait_CTRL(uint16_t value)
		{
			switch (value & 3)
			{
				case 0: SRAM_Waits = 4; break;
				case 1: SRAM_Waits = 3; break;
				case 2: SRAM_Waits = 2; break;
				case 3: SRAM_Waits = 8; break;
			}

			switch ((value >> 2) & 3)
			{
				case 0: ROM_Waits_0_N = 4; break;
				case 1: ROM_Waits_0_N = 3; break;
				case 2: ROM_Waits_0_N = 2; break;
				case 3: ROM_Waits_0_N = 8; break;
			}

			switch ((value >> 4) & 1)
			{
				case 0: ROM_Waits_0_S = 2; break;
				case 1: ROM_Waits_0_S = 1; break;
			}

			switch ((value >> 5) & 3)
			{
				case 0: ROM_Waits_1_N = 4; break;
				case 1: ROM_Waits_1_N = 3; break;
				case 2: ROM_Waits_1_N = 2; break;
				case 3: ROM_Waits_1_N = 8; break;
			}

			switch ((value >> 7) & 1)
			{
				case 0: ROM_Waits_1_S = 4; break;
				case 1: ROM_Waits_1_S = 1; break;
			}

			switch ((value >> 8) & 3)
			{
				case 0: ROM_Waits_2_N = 4; break;
				case 1: ROM_Waits_2_N = 3; break;
				case 2: ROM_Waits_2_N = 2; break;
				case 3: ROM_Waits_2_N = 8; break;
			}

			switch ((value >> 10) & 1)
			{
				case 0: ROM_Waits_2_S = 8; break;
				case 1: ROM_Waits_2_S = 1; break;
			}

			pre_Reg_Write(value);

			Wait_CTRL = (uint16_t)(value & 0x5FFF);
		
			//Message_String = to_string(value) + " " + to_string(CycleCount);

			//MessageCallback(Message_String.length());			
		}


		void Update_Post_Boot(uint8_t value)
		{
			if (Post_Boot == 0)
			{
				Post_Boot = value;
			}
		}

		void Update_Halt_CTRL(uint8_t value)
		{
			// appears to only be addressable from BIOS, see haltcnt.gba, timing of first test indicates no halting, which is written from outside BIOS
			if (cpu_Regs[15] <= 0x3FFF)
			{
				if ((value & 0x80) == 0)
				{
					Halt_Enter = true;
					Halt_Enter_cd = 2;
					IRQ_Delays = true;
					delays_to_process = true;
				}
				else
				{
					if (!is_linked_system && !Is_GBP)
					{
						stopped = true;
						// use this to end the frame
						VBlank_Rise = true;
					}
				}
			}
		}


		uint8_t Peek_Registers_8(uint8_t addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_8(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_8(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_8(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_8(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_8(addr);
			}

			uint8_t ret = 0;

			switch (addr)
			{
				case 0x200: ret = (uint8_t)(INT_EN & 0xFF); break;
				case 0x201: ret = (uint8_t)((INT_EN & 0xFF00) >> 8); break;
				case 0x202: ret = (uint8_t)(INT_Flags & 0xFF); break;
				case 0x203: ret = (uint8_t)((INT_Flags & 0xFF00) >> 8); break;
				case 0x204: ret = (uint8_t)(Wait_CTRL & 0xFF); break;
				case 0x205: ret = (uint8_t)((Wait_CTRL & 0xFF00) >> 8); break;

				case 0x208: ret = (uint8_t)(INT_Master & 0xFF); break;
				case 0x209: ret = (uint8_t)((INT_Master & 0xFF00) >> 8); break;

				case 0x300: ret = Post_Boot; break;
				case 0x301: ret = Halt_CTRL; break;

				default: ret = 0xFF; break;
			}

			return ret;
		}
	# pragma endregion

	#pragma region ARM7_TDMI
		#pragma region Variables
		// General Execution
		bool cpu_Thumb_Mode;
		bool cpu_ARM_Cond_Passed;
		bool cpu_Seq_Access;
		bool cpu_IRQ_Input;
		bool cpu_IRQ_Input_Use;
		bool cpu_Next_IRQ_Input;
		bool cpu_Next_IRQ_Input_2;
		bool cpu_Next_IRQ_Input_3;
		bool cpu_Is_Paused;
		bool cpu_No_IRQ_Clock;
		bool cpu_Restore_IRQ_Clock;
		bool cpu_Take_Branch;
		bool cpu_LS_Is_Load;
		bool cpu_LS_First_Access;
		bool cpu_Invalidate_Pipeline;
		bool cpu_Overwrite_Base_Reg;
		bool cpu_Multi_Before;
		bool cpu_Multi_Inc;
		bool cpu_Multi_S_Bit;
		bool cpu_ALU_S_Bit;
		bool cpu_Multi_Swap;
		bool cpu_Sign_Extend_Load;
		bool cpu_Dest_Is_R15;
		bool cpu_Swap_Store;
		bool cpu_Swap_Lock;
		bool cpu_Clear_Pipeline;
		bool cpu_Special_Inc;
		bool cpu_FlagI_Old;
		bool cpu_LDM_Glitch_Mode;
		bool cpu_LDM_Glitch_Store;

		bool stopped;

		bool cpu_Trigger_Unhalt;
		bool cpu_Trigger_Unhalt_2;
		bool cpu_Trigger_Unhalt_3;
		bool cpu_Trigger_Unhalt_4;

		// ARM Related Variables
		uint16_t cpu_Exec_ARM;

		// Thumb related Variables
		uint16_t cpu_Exec_TMB;

		uint16_t cpu_Instr_TMB_0, cpu_Instr_TMB_1, cpu_Instr_TMB_2;
		uint16_t cpu_Instr_Type;
		uint16_t cpu_Exception_Type;
		uint16_t cpu_Next_Load_Store_Type;

		// register contents are saved and copied out during mode switches
		uint32_t cpu_user_R8, cpu_user_R9, cpu_user_R10, cpu_user_R11, cpu_user_R12, cpu_user_R13, cpu_user_R14;
		uint32_t cpu_spr_R13, cpu_spr_R14, cpu_spr_S;
		uint32_t cpu_abort_R13, cpu_abort_R14, cpu_abort_S;
		uint32_t cpu_undf_R13, cpu_undf_R14, cpu_undf_S;
		uint32_t cpu_intr_R13, cpu_intr_R14, cpu_intr_S;
		uint32_t cpu_fiq_R8, cpu_fiq_R9, cpu_fiq_R10, cpu_fiq_R11, cpu_fiq_R12, cpu_fiq_R13, cpu_fiq_R14, cpu_fiq_S;

		uint32_t cpu_Instr_ARM_0, cpu_Instr_ARM_1, cpu_Instr_ARM_2;
		uint32_t cpu_Temp_Reg;
		uint32_t cpu_Temp_Addr;
		uint32_t cpu_Temp_Data;
		uint32_t cpu_Temp_Mode;
		uint32_t cpu_Bit_To_Check;
		uint32_t cpu_Write_Back_Addr;
		uint32_t cpu_Addr_Offset;
		uint32_t cpu_Last_Bus_Value;
		uint32_t cpu_Last_Bus_Value_Old;

		uint32_t cpu_ALU_Temp_Val, cpu_ALU_Temp_S_Val, cpu_ALU_Shift_Carry;

		int cpu_Fetch_Cnt, cpu_Fetch_Wait;

		int cpu_Multi_List_Ptr, cpu_Multi_List_Size, cpu_Temp_Reg_Ptr, cpu_Base_Reg, cpu_Base_Reg_2;

		int cpu_ALU_Reg_Dest, cpu_ALU_Reg_Src;

		int cpu_Mul_Cycles, cpu_Mul_Cycles_Cnt;

		int cpu_Shift_Imm;

		uint64_t cpu_ALU_Long_Result;
		uint64_t CycleCount;
		uint64_t Clock_Update_Cycle;
		uint64_t FrameCycle;

		int64_t cpu_ALU_Signed_Long_Result;

		uint32_t cpu_Regs_To_Access[16] = { };
		uint32_t cpu_Regs[18] = { };

		inline bool cpu_FlagNget() { return (cpu_Regs[16] & 0x80000000) != 0; }
		inline void cpu_FlagNset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x80000000) | (value ? 0x80000000 : 0x00000000)); }

		inline bool cpu_FlagZget() { return (cpu_Regs[16] & 0x40000000) != 0; }
		inline void cpu_FlagZset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x40000000) | (value ? 0x40000000 : 0x00000000)); }

		inline bool cpu_FlagCget() { return (cpu_Regs[16] & 0x20000000) != 0; }
		inline void cpu_FlagCset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x20000000) | (value ? 0x20000000 : 0x00000000)); }

		inline bool cpu_FlagVget() { return (cpu_Regs[16] & 0x10000000) != 0; }
		inline void cpu_FlagVset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x10000000) | (value ? 0x10000000 : 0x00000000)); }

		inline bool cpu_FlagIget() { return (cpu_Regs[16] & 0x00000080) != 0; }
		inline void cpu_FlagIset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x00000080) | (value ? 0x00000080 : 0x00000000)); }

		inline bool cpu_FlagFget() { return (cpu_Regs[16] & 0x00000040) != 0; }
		inline void cpu_FlagFset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x00000040) | (value ? 0x00000040 : 0x00000000)); }

		inline bool cpu_FlagTget() { return (cpu_Regs[16] & 0x00000020) != 0; }
		inline void cpu_FlagTset(bool value) { cpu_Regs[16] = (uint32_t)((cpu_Regs[16] & ~0x00000020) | (value ? 0x00000020 : 0x00000000)); }

		void ResetRegisters()
		{
			cpu_user_R8 = cpu_user_R9 = cpu_user_R10 = cpu_user_R11 = cpu_user_R12 = cpu_user_R13 = cpu_user_R14 = 0;
			cpu_spr_R13 = cpu_spr_R14 = cpu_spr_S = 0;
			cpu_abort_R13 = cpu_abort_R14 = cpu_abort_S = 0;
			cpu_undf_R13 = cpu_undf_R14 = cpu_undf_S = 0;
			cpu_intr_R13 = cpu_intr_R14 = cpu_intr_S = 0;
			cpu_fiq_R8 = cpu_fiq_R9 = cpu_fiq_R10 = cpu_fiq_R11 = cpu_fiq_R12 = cpu_fiq_R13 = cpu_fiq_R14 = cpu_fiq_S = 0;

			for (int i = 0; i < 18; i++)
			{
				cpu_Regs[i] = 0;
			}

			// The processor starts in ARM, supervisor mode, with interrupts disabled
			cpu_Regs[16] = 0x13;

			// upper bit of spsr always set
			cpu_Regs[17] = 0x10;

			cpu_FlagIset(true);
			cpu_FlagFset(true);
		}

		// NOTE: system and user have same regs
		void cpu_Swap_Regs(uint32_t New_State, bool C_to_S, bool S_to_C)
		{
			uint32_t cpu_s_to_c_reg = cpu_Regs[17];
			
			// user and system
			if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
			{
				cpu_user_R13 = cpu_Regs[13];
				cpu_user_R14 = cpu_Regs[14];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					// nothing to do
				}
				else if (New_State == 0x11)		// FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// FIQ
			if ((cpu_Regs[16] & 0x1F) == 0x11)
			{
				cpu_fiq_R8 = cpu_Regs[8];
				cpu_fiq_R9 = cpu_Regs[9];
				cpu_fiq_R10 = cpu_Regs[10];
				cpu_fiq_R11 = cpu_Regs[11];
				cpu_fiq_R12 = cpu_Regs[12];
				cpu_fiq_R13 = cpu_Regs[13];
				cpu_fiq_R14 = cpu_Regs[14];
				cpu_fiq_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					// nothing to do
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// IRQ
			if ((cpu_Regs[16] & 0x1F) == 0x12)
			{
				cpu_intr_R13 = cpu_Regs[13];
				cpu_intr_R14 = cpu_Regs[14];
				cpu_intr_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))		// user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)		// FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					// nothing to do
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// Supervisor
			if ((cpu_Regs[16] & 0x1F) == 0x13)
			{
				cpu_spr_R13 = cpu_Regs[13];
				cpu_spr_R14 = cpu_Regs[14];
				cpu_spr_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					// nothing to so
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// Abort
			if ((cpu_Regs[16] & 0x1F) == 0x17)
			{
				cpu_abort_R13 = cpu_Regs[13];
				cpu_abort_R14 = cpu_Regs[14];
				cpu_abort_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					// nothing to so
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// Undefined Instruction
			if ((cpu_Regs[16] & 0x1F) == 0x1B)
			{
				cpu_undf_R13 = cpu_Regs[13];
				cpu_undf_R14 = cpu_Regs[14];
				cpu_undf_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					// nothing to so
				}
			}

			if (C_to_S) { cpu_Regs[17] = cpu_Regs[16]; }

			if (S_to_C) { cpu_Regs[16] = cpu_s_to_c_reg; }

			cpu_Regs[16] &= 0xFFFFFFE0;
			cpu_Regs[16] |= New_State;
		}
		#pragma endregion

		#pragma region Constant Declarations
		// Instruction types
		const static uint16_t cpu_Internal_And_Prefetch_ARM = 0;
		const static uint16_t cpu_Internal_And_Prefetch_2_ARM = 1;
		const static uint16_t cpu_Internal_And_Prefetch_3_ARM = 2;
		const static uint16_t cpu_Internal_And_Branch_1_ARM = 3;
		const static uint16_t cpu_Internal_And_Branch_2_ARM = 4;
		const static uint16_t cpu_Internal_And_Branch_3_ARM = 5;
		const static uint16_t cpu_Prefetch_Only_1_ARM = 6;
		const static uint16_t cpu_Prefetch_Only_2_ARM = 7;
		const static uint16_t cpu_Prefetch_And_Load_Store_ARM = 8;
		const static uint16_t cpu_Load_Store_Word_ARM = 9;
		const static uint16_t cpu_Load_Store_Half_ARM = 10;
		const static uint16_t cpu_Load_Store_Byte_ARM = 11;
		const static uint16_t cpu_Multi_Load_Store_ARM = 12;
		const static uint16_t cpu_Multiply_ARM = 13;
		const static uint16_t cpu_Prefetch_Swap_ARM = 14;
		const static uint16_t cpu_Swap_ARM = 15;
		const static uint16_t cpu_Prefetch_And_Branch_Ex_ARM = 16;

		const static uint16_t cpu_Internal_And_Prefetch_TMB = 20;
		const static uint16_t cpu_Internal_And_Prefetch_2_TMB = 21;
		const static uint16_t cpu_Internal_And_Branch_1_TMB = 22;
		const static uint16_t cpu_Prefetch_Only_1_TMB = 23;
		const static uint16_t cpu_Prefetch_Only_2_TMB = 24;
		const static uint16_t cpu_Prefetch_And_Load_Store_TMB = 25;
		const static uint16_t cpu_Load_Store_Word_TMB = 26;
		const static uint16_t cpu_Load_Store_Half_TMB = 27;
		const static uint16_t cpu_Load_Store_Byte_TMB = 28;
		const static uint16_t cpu_Multi_Load_Store_TMB = 29;
		const static uint16_t cpu_Multiply_TMB = 30;
		const static uint16_t cpu_Prefetch_And_Branch_Ex_TMB = 31;

		// SWI and undefined opcode cycles, same for both Thumb and ARM
		const static uint16_t cpu_Prefetch_And_SWI_Undef = 38;
		const static uint16_t cpu_Prefetch_IRQ = 39;
		const static uint16_t cpu_Internal_Reset_1 = 40;
		const static uint16_t cpu_Internal_Reset_2 = 41;

		// These operations are all internal cycles, they can take place whilea DMA is occuring
		const static uint16_t cpu_Internal_And_Branch_4_ARM = 42;
		const static uint16_t cpu_Internal_Can_Save_ARM = 43;
		const static uint16_t cpu_Internal_Can_Save_TMB = 44;
		const static uint16_t cpu_Internal_Halted = 45;
		const static uint16_t cpu_Multiply_Cycles = 46;
		const static uint16_t cpu_Pause_For_DMA = 47;
		
		// Instruction Operations ARM
		const static uint16_t cpu_ARM_AND = 10;
		const static uint16_t cpu_ARM_EOR = 11;
		const static uint16_t cpu_ARM_SUB = 12;
		const static uint16_t cpu_ARM_RSB = 13;
		const static uint16_t cpu_ARM_ADD = 14;
		const static uint16_t cpu_ARM_ADC = 15;
		const static uint16_t cpu_ARM_SBC = 16;
		const static uint16_t cpu_ARM_RSC = 17;
		const static uint16_t cpu_ARM_TST = 18;
		const static uint16_t cpu_ARM_TEQ = 19;
		const static uint16_t cpu_ARM_CMP = 20;
		const static uint16_t cpu_ARM_CMN = 21;
		const static uint16_t cpu_ARM_ORR = 22;
		const static uint16_t cpu_ARM_MOV = 23;
		const static uint16_t cpu_ARM_BIC = 24;
		const static uint16_t cpu_ARM_MVN = 25;
		const static uint16_t cpu_ARM_MSR = 26;
		const static uint16_t cpu_ARM_MSR_Glitchy = 27;
		const static uint16_t cpu_ARM_MRS = 28;
		const static uint16_t cpu_ARM_Bx = 29;
		const static uint16_t cpu_ARM_MUL = 30;
		const static uint16_t cpu_ARM_MUL_UL = 31;
		const static uint16_t cpu_ARM_MUL_SL = 32;
		const static uint16_t cpu_ARM_Swap = 33;
		const static uint16_t cpu_ARM_Imm_LS = 34;
		const static uint16_t cpu_ARM_Reg_LS = 35;
		const static uint16_t cpu_ARM_Multi_1 = 36;
		const static uint16_t cpu_ARM_Branch = 37;
		const static uint16_t cpu_ARM_Cond_Check_Only = 38;

		const static uint16_t cpu_ARM_AND_LDM = 110;
		const static uint16_t cpu_ARM_EOR_LDM = 111;
		const static uint16_t cpu_ARM_SUB_LDM = 112;
		const static uint16_t cpu_ARM_RSB_LDM = 113;
		const static uint16_t cpu_ARM_ADD_LDM = 114;
		const static uint16_t cpu_ARM_ADC_LDM = 115;
		const static uint16_t cpu_ARM_SBC_LDM = 116;
		const static uint16_t cpu_ARM_RSC_LDM = 117;
		const static uint16_t cpu_ARM_TST_LDM = 118;
		const static uint16_t cpu_ARM_TEQ_LDM = 119;
		const static uint16_t cpu_ARM_CMP_LDM = 120;
		const static uint16_t cpu_ARM_CMN_LDM = 121;
		const static uint16_t cpu_ARM_ORR_LDM = 122;
		const static uint16_t cpu_ARM_MOV_LDM = 123;
		const static uint16_t cpu_ARM_BIC_LDM = 124;
		const static uint16_t cpu_ARM_MVN_LDM = 125;
		const static uint16_t cpu_ARM_MSR_LDM = 126;
		const static uint16_t cpu_ARM_MSR_LDM_Glitchy = 127;
		const static uint16_t cpu_ARM_MRS_LDM = 128;
		const static uint16_t cpu_ARM_Bx_LDM = 129;
		const static uint16_t cpu_ARM_MUL_LDM = 130;
		const static uint16_t cpu_ARM_MUL_UL_LDM = 131;
		const static uint16_t cpu_ARM_MUL_SL_LDM = 132;
		const static uint16_t cpu_ARM_Swap_LDM = 133;
		const static uint16_t cpu_ARM_Imm_LS_LDM = 134;
		const static uint16_t cpu_ARM_Reg_LS_LDM = 135;
		const static uint16_t cpu_ARM_Multi_1_LDM = 136;
		const static uint16_t cpu_ARM_Branch_LDM = 137;
		const static uint16_t cpu_ARM_Cond_Check_Only_LDM = 138;

		// Instruction Operations Thumb
		const static uint16_t cpu_Thumb_Shift = 5;
		const static uint16_t cpu_Thumb_Add_Sub_Reg = 6;
		const static uint16_t cpu_Thumb_AND = 10;
		const static uint16_t cpu_Thumb_EOR = 11;
		const static uint16_t cpu_Thumb_LSL = 12;
		const static uint16_t cpu_Thumb_LSR = 13;
		const static uint16_t cpu_Thumb_ASR = 14;
		const static uint16_t cpu_Thumb_ADC = 15;
		const static uint16_t cpu_Thumb_SBC = 16;
		const static uint16_t cpu_Thumb_ROR = 17;
		const static uint16_t cpu_Thumb_TST = 18;
		const static uint16_t cpu_Thumb_NEG = 19;
		const static uint16_t cpu_Thumb_CMP = 20;
		const static uint16_t cpu_Thumb_CMN = 21;
		const static uint16_t cpu_Thumb_ORR = 22;
		const static uint16_t cpu_Thumb_MUL = 23;
		const static uint16_t cpu_Thumb_BIC = 24;
		const static uint16_t cpu_Thumb_MVN = 25;

		const static uint16_t cpu_Thumb_High_Add = 30;
		const static uint16_t cpu_Thumb_High_Cmp = 31;
		const static uint16_t cpu_Thumb_High_Bx = 32;
		const static uint16_t cpu_Thumb_High_MOV = 33;
		const static uint16_t cpu_Thumb_ALU_Imm = 34;
		const static uint16_t cpu_Thumb_PC_Rel_LS = 35;
		const static uint16_t cpu_Thumb_Rel_LS = 36;
		const static uint16_t cpu_Thumb_Imm_LS = 37;
		const static uint16_t cpu_Thumb_Half_LS = 38;
		const static uint16_t cpu_Thumb_SP_REL_LS = 39;
		const static uint16_t cpu_Thumb_Add_SP_PC = 40;
		const static uint16_t cpu_Thumb_Add_Sub_Stack = 41;
		const static uint16_t cpu_Thumb_Push_Pop = 50;
		const static uint16_t cpu_Thumb_Multi_1 = 60;
		const static uint16_t cpu_Thumb_Branch = 100;
		const static uint16_t cpu_Thumb_Branch_Cond = 101;
		const static uint16_t cpu_Thumb_Branch_Link_1 = 102;
		const static uint16_t cpu_Thumb_Branch_Link_2 = 103;

		// Exception Types
		const static uint16_t cpu_IRQ_Exc = 0;
		const static uint16_t cpu_SWI_Exc = 1;
		const static uint16_t cpu_Undef_Exc = 2;
		const static uint16_t cpu_Reset_Exc = 3;

		// ALU related
		const static uint64_t cpu_Carry_Compare = 0x100000000;
		const static uint64_t cpu_Long_Neg_Compare = 0x8000000000000000;

		const static uint32_t cpu_Neg_Compare = 0x80000000;
		const static uint32_t cpu_Cast_Int = 0xFFFFFFFF;

		// Masks
		const static uint32_t cpu_Unalloc_Mask = 0x0FFFFF00;
		const static uint32_t cpu_User_Mask = 0xF0000000;
		const static uint32_t cpu_Priv_Mask = 0x000000DF;
		const static uint32_t cpu_State_Mask = 0x00000020;
		#pragma endregion

		#pragma region ARM7_TDMI functions

		void cpu_Reset();

		void cpu_Decode_ARM();

		void cpu_Decode_TMB();

		void cpu_Calculate_Mul_Cycles()
		{
			uint32_t temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

			if (cpu_Thumb_Mode)
			{
				temp_calc = cpu_Regs[cpu_ALU_Reg_Dest];
			}

			// all F's seems to be a special case
			if ((temp_calc & 0xFF000000) == 0xFF000000)
			{
				cpu_Mul_Cycles = 3;

				if ((temp_calc & 0x00FF0000) == 0x00FF0000)
				{
					cpu_Mul_Cycles -= 1;

					if ((temp_calc & 0x0000FF00) == 0x0000FF00)
					{
						cpu_Mul_Cycles -= 1;
					}
				}
			}
			else if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 2;
			}
			else
			{
				cpu_Mul_Cycles = 1;
			}

			if (!cpu_Thumb_Mode)
			{
				if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
				{
					cpu_Mul_Cycles += 1;
				}
			}
		}

		// seems to be based on the number of non-zero upper bits
		void cpu_Calculate_Mul_Cycles_UL()
		{
			uint32_t temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

			if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 5;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else
			{
				cpu_Mul_Cycles = 2;
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		void cpu_Calculate_Mul_Cycles_SL()
		{
			uint32_t temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

			// all F's seems to be a special case
			if ((temp_calc & 0xFF000000) == 0xFF000000)
			{
				cpu_Mul_Cycles = 4;

				if ((temp_calc & 0x00FF0000) == 0x00FF0000)
				{
					cpu_Mul_Cycles -= 1;

					if ((temp_calc & 0x0000FF00) == 0x0000FF00)
					{
						cpu_Mul_Cycles -= 1;
					}
				}
			}
			else if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 5;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else
			{
				cpu_Mul_Cycles = 2;
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		void cpu_Calculate_Mul_Cycles_LDM()
		{
			uint32_t temp_calc = 0;

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
			}
			else
			{
				temp_calc = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF);
			}

			// all F's seems to be a special case
			if ((temp_calc & 0xFF000000) == 0xFF000000)
			{
				cpu_Mul_Cycles = 3;

				if ((temp_calc & 0x00FF0000) == 0x00FF0000)
				{
					cpu_Mul_Cycles -= 1;

					if ((temp_calc & 0x0000FF00) == 0x0000FF00)
					{
						cpu_Mul_Cycles -= 1;
					}
				}
			}
			else if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 2;
			}
			else
			{
				cpu_Mul_Cycles = 1;
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		// seems to be based on the number of non-zero upper bits
		void cpu_Calculate_Mul_Cycles_UL_LDM()
		{
			uint32_t temp_calc = 0;
			
			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
			}
			else
			{
				temp_calc = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF);
			}

			if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 5;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else
			{
				cpu_Mul_Cycles = 2;
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		void cpu_Calculate_Mul_Cycles_SL_LDM()
		{
			uint32_t temp_calc = 0;

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				temp_calc = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
			}
			else
			{
				temp_calc = cpu_LDM_Glitch_Get_Reg((cpu_Instr_ARM_2 >> 8) & 0xF);
			}

			// all F's seems to be a special case
			if ((temp_calc & 0xFF000000) == 0xFF000000)
			{
				cpu_Mul_Cycles = 4;

				if ((temp_calc & 0x00FF0000) == 0x00FF0000)
				{
					cpu_Mul_Cycles -= 1;

					if ((temp_calc & 0x0000FF00) == 0x0000FF00)
					{
						cpu_Mul_Cycles -= 1;
					}
				}
			}
			else if ((temp_calc & 0xFF000000) != 0)
			{
				cpu_Mul_Cycles = 5;
			}
			else if ((temp_calc & 0x00FF0000) != 0)
			{
				cpu_Mul_Cycles = 4;
			}
			else if ((temp_calc & 0x0000FF00) != 0)
			{
				cpu_Mul_Cycles = 3;
			}
			else
			{
				cpu_Mul_Cycles = 2;
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		void cpu_Execute_Internal_Only_ARM();

		void cpu_Execute_Internal_Only_TMB();

		bool cpu_ARM_Condition_Check()
		{
			switch (cpu_Instr_ARM_2 & 0xF0000000)
			{
			case 0x00000000: return cpu_FlagZget();
			case 0x10000000: return !cpu_FlagZget();
			case 0x20000000: return cpu_FlagCget();
			case 0x30000000: return !cpu_FlagCget();
			case 0x40000000: return cpu_FlagNget();
			case 0x50000000: return !cpu_FlagNget();
			case 0x60000000: return cpu_FlagVget();
			case 0x70000000: return !cpu_FlagVget();
			case 0x80000000: return (cpu_FlagCget() && !cpu_FlagZget());
			case 0x90000000: return (!cpu_FlagCget() || cpu_FlagZget());
			case 0xA0000000: return (cpu_FlagNget() == cpu_FlagVget());
			case 0xB0000000: return (cpu_FlagNget() != cpu_FlagVget());
			case 0xC0000000: return (!cpu_FlagZget() && (cpu_FlagNget() == cpu_FlagVget()));
			case 0xD0000000: return (cpu_FlagZget() || (cpu_FlagNget() != cpu_FlagVget()));
			case 0xE0000000: return true;
			case 0xF0000000: return false; // Architecturally invalid, but is just false
			}

			return true;
		}

		bool cpu_Calc_V_Flag_Add(uint32_t val1, uint32_t val2, uint32_t val3)
		{
			if ((val1 & cpu_Neg_Compare) == cpu_Neg_Compare)
			{
				if ((val2 & cpu_Neg_Compare) == cpu_Neg_Compare)
				{
					if ((val3 & cpu_Neg_Compare) == 0)
					{
						return true;
					}

					return false;
				}

				return false;
			}
			else
			{
				if ((val2 & cpu_Neg_Compare) == 0)
				{
					if ((val3 & cpu_Neg_Compare) == cpu_Neg_Compare)
					{
						return true;
					}

					return false;
				}

				return false;
			}
		}

		bool cpu_Calc_V_Flag_Sub(uint32_t val1, uint32_t val2, uint32_t val3)
		{
			if ((val1 & cpu_Neg_Compare) == cpu_Neg_Compare)
			{
				if ((val2 & cpu_Neg_Compare) == 0)
				{
					if ((val3 & cpu_Neg_Compare) == 0)
					{
						return true;
					}

					return false;
				}

				return false;
			}
			else
			{
				if ((val2 & cpu_Neg_Compare) == cpu_Neg_Compare)
				{
					if ((val3 & cpu_Neg_Compare) == cpu_Neg_Compare)
					{
						return true;
					}

					return false;
				}

				return false;
			}
		}

		bool cpu_TMB_Condition_Check()
		{
			switch (cpu_Instr_TMB_2 & 0xF00)
			{
			case 0x000: return cpu_FlagZget();
			case 0x100: return !cpu_FlagZget();
			case 0x200: return cpu_FlagCget();
			case 0x300: return !cpu_FlagCget();
			case 0x400: return cpu_FlagNget();
			case 0x500: return !cpu_FlagNget();
			case 0x600: return cpu_FlagVget();
			case 0x700: return !cpu_FlagVget();
			case 0x800: return (cpu_FlagCget() && !cpu_FlagZget());
			case 0x900: return (!cpu_FlagCget() || cpu_FlagZget());
			case 0xA00: return (cpu_FlagNget() == cpu_FlagVget());
			case 0xB00: return (cpu_FlagNget() != cpu_FlagVget());
			case 0xC00: return (!cpu_FlagZget() && (cpu_FlagNget() == cpu_FlagVget()));
			case 0xD00: return (cpu_FlagZget() || (cpu_FlagNget() != cpu_FlagVget()));
			case 0xE00: return true;
			case 0xF00: return false; // Architecturally invalid, but is just false
			}

			return true;
		}

		// LDM^ glitch details:
		// If the next register access happens immediately following the completion of the LDM^/STM^ instruction,
		// and this access is to one of the banked registers,
		// then the returned value will be the banked register and user mode register OR'd together
		void cpu_LDM_Glitch_Decode_ARM();

		uint32_t cpu_LDM_Glitch_Get_Reg(uint32_t reg_num)
		{
			uint32_t ret = 0;

			if (reg_num < 8)
			{
				ret = cpu_Regs[reg_num];
			}
			else
			{
				if (reg_num == 13)
				{
					ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R13);
				}
				else if (reg_num == 14)
				{
					ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R14);
				}
				else if ((cpu_Regs[16] & 0x1F) == 0x11)
				{
					if (reg_num == 8)
					{
						ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R8);
					}
					else if (reg_num == 9)
					{
						ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R9);
					}
					else if (reg_num == 10)
					{
						ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R10);
					}
					else if (reg_num == 11)
					{
						ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R11);
					}
					else if (reg_num == 12)
					{
						ret = (uint32_t)(cpu_Regs[reg_num] | cpu_user_R12);
					}
				}
				else
				{
					ret = cpu_Regs[reg_num];
				}
			}

			return ret;
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

	#pragma region Wait States
		// NOTE: For 32 bit accesses, PALRAM and VRAM take 2 accesses, but could be interrupted by rendering. So the 32 bit wait state processors need to know about
		// the destination and value in this case.
		// For the CPU, the value will be in cpu_Regs[cpu_Temp_Reg_Ptr], for DMA it is in dma_TFR_Word. 
		// For the CPU, whether it is a write or not is in cpu_LS_Is_Load, for DMA it is in dma_Read_Cycle	

		uint32_t Wait_State_Access_8(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;

								ppu_VRAM_High_In_Use = true;
							}
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;

									ppu_VRAM_In_Use = true;
								}
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;

									ppu_VRAM_High_In_Use = true;
								}
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;

							ppu_VRAM_In_Use = true;
						}
					}
				}
				else
				{
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;

						ppu_PALRAM_In_Use = true;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += WRAM_Waits; //WRAM
			}

			return wait_ret;
		}

		uint32_t Wait_State_Access_16(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0			
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;

								ppu_VRAM_High_In_Use = true;
							}
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;

									ppu_VRAM_In_Use = true;
								}
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;

									ppu_VRAM_High_In_Use = true;
								}
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;

							ppu_VRAM_In_Use = true;
						}
					}
				}
				else
				{
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;

						ppu_PALRAM_In_Use = true;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += WRAM_Waits; //WRAM
			}

			return wait_ret;
		}

		uint32_t Wait_State_Access_32(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;
							}

							// set to true since we also need to check the next cycle
							ppu_VRAM_High_In_Use = true;
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_In_Use = true;
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_High_In_Use = true;
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;
						}

						// set to true since we also need to check the next cycle
						ppu_VRAM_In_Use = true;
					}

					if (!cpu_LS_Is_Load)
					{
						VRAM_32_Check = true;
						VRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						VRAM_32W_Addr = addr;
						VRAM_32W_Value = (uint16_t)cpu_Regs[cpu_Temp_Reg_Ptr];
					}
				}
				else
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_PALRAM_In_Use = true;

					if (!cpu_LS_Is_Load)
					{
						PALRAM_32_Check = true;
						PALRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						PALRAM_32W_Addr = addr;
						PALRAM_32W_Value = (uint16_t)cpu_Regs[cpu_Temp_Reg_Ptr];
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
			}

			return wait_ret;
		}

		uint32_t Wait_State_Access_32_DMA(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x10000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
					}
					else if (addr < 0x0E000000)
					{
						if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
					}
					else
					{
						wait_ret += SRAM_Waits; // SRAM
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;
							}

							// set to true since we also need to check the next cycle
							ppu_VRAM_High_In_Use = true;
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_In_Use = true;
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_High_In_Use = true;
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;
						}

						// set to true since we also need to check the next cycle
						ppu_VRAM_In_Use = true;
					}

					if (!dma_Read_Cycle[dma_Chan_Exec])
					{
						VRAM_32_Check = true;
						VRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						VRAM_32W_Addr = addr;
						VRAM_32W_Value = (uint16_t)dma_TFR_Word;
					}
				}
				else
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_PALRAM_In_Use = true;

					if (!dma_Read_Cycle[dma_Chan_Exec])
					{
						PALRAM_32_Check = true;
						PALRAM_32_Delay = true;

						Misc_Delays = true;
						delays_to_process = true;

						PALRAM_32W_Addr = addr;
						PALRAM_32W_Value = (uint16_t)dma_TFR_Word;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
			}

			return wait_ret;
		}

		uint32_t Wait_State_Access_16_Instr(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
				{
					if (addr == pre_Check_Addr)
					{
						if ((pre_Check_Addr != pre_Read_Addr) && (pre_Buffer_Cnt > 0))
						{
							// we have this address, can immediately read it
							pre_Check_Addr += 2;
							pre_Buffer_Cnt -= 1;

							if (pre_Buffer_Cnt == 0)
							{
								if (pre_Buffer_Was_Full) { pre_Check_Addr = 0; pre_Force_Non_Seq = true; }

								if (!pre_Enable) { pre_Check_Addr = 0; }
							}
						}
						else if ((addr & 0x1FFFE) == 0)
						{
							// we encountered a boundary, always use non-sequential timing						
							if (addr < 0x0A000000)
							{
								wait_ret += ROM_Waits_0_N;
							}
							else if (addr < 0x0C000000)
							{
								wait_ret += ROM_Waits_1_N;
							}
							else
							{
								wait_ret += ROM_Waits_2_N;
							}
							
							if (pre_Cycle_Glitch)
							{
								// lose 1 cycle if prefetcher is holding the bus
								wait_ret += 1;
							}

							pre_Read_Addr += 2;
							pre_Check_Addr += 2;
							pre_Fetch_Cnt = 0;

							if (!pre_Enable) { pre_Check_Addr = 0; pre_Run = false; }

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Inactive = true;
						}
						else
						{
							// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
							if (!Seq_Access && (pre_Fetch_Cnt == 1) && !pre_Following)
							{
								// this happens in a branch to the current prefetcher fetch address
								if (addr < 0x0A000000)
								{
									wait_ret += ROM_Waits_0_N;
								}
								else if (addr < 0x0C000000)
								{
									wait_ret += ROM_Waits_1_N;
								}
								else
								{
									wait_ret += ROM_Waits_2_N;
								}
							}
							else
							{
								// plus 1 since the prefetcher already used this cycle, so don't double count
								wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;
							}

							pre_Read_Addr += 2;
							pre_Check_Addr += 2;
							pre_Fetch_Cnt = 0;

							if (!pre_Enable) { pre_Check_Addr = 0; pre_Run = false; }

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Inactive = true;
						}
					}
					else
					{
						// the address is not related to the current ones available to the prefetcher
						Seq_Access &= !pre_Force_Non_Seq;

						if (addr < 0x0A000000)
						{
							if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_0_N; } // ROM 0, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_0_S : ROM_Waits_0_N; } // ROM 0
						}
						else if (addr < 0x0C000000)
						{
							if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_1_N; } // ROM 1, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_1_S : ROM_Waits_1_N; } // ROM 1
						}
						else
						{
							if ((addr & 0x1FFFE) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
						}

						pre_Force_Non_Seq = false;

						if (pre_Cycle_Glitch)
						{
							// lose 1 cycle if prefetcher is holding the bus
							wait_ret += 1;
						}

						// abandon the prefetcher current fetch and reset the address
						pre_Buffer_Cnt = 0;
						pre_Fetch_Cnt = 0;
						pre_Run = pre_Enable;
						pre_Buffer_Was_Full = false;
						pre_Following = false;

						pre_Inactive = true;

						if (pre_Enable) { pre_Check_Addr = pre_Read_Addr = addr + 2; }
						else { pre_Check_Addr = 0; }
					}
				}
				else if (addr < 0x10000000)
				{
					wait_ret += SRAM_Waits; // SRAM

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;

								ppu_VRAM_High_In_Use = true;
							}
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;

									ppu_VRAM_In_Use = true;
								}
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;

									ppu_VRAM_High_In_Use = true;
								}
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;

							ppu_VRAM_In_Use = true;
						}
					}
				}
				else
				{
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;

						ppu_PALRAM_In_Use = true;
					}
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += WRAM_Waits; //WRAM
			}

			return wait_ret;
		}

		uint32_t Wait_State_Access_32_Instr(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;
			uint32_t addr_check = (addr & 0xFFFFFFFC);

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
				{
					if (addr_check == pre_Check_Addr)
					{
						if ((pre_Check_Addr != pre_Read_Addr) && (pre_Buffer_Cnt > 0))
						{
							// we have this address, can immediately read it
							pre_Check_Addr += 2;
							pre_Buffer_Cnt -= 1;

							// check if we have the next address as well
							if ((pre_Check_Addr != pre_Read_Addr) && (pre_Buffer_Cnt > 0))
							{
								// the prefetcher can return 32 bits in 1 cycle if it has it available
								pre_Check_Addr += 2;
								pre_Buffer_Cnt -= 1;

								if (pre_Buffer_Cnt == 0)
								{
									if (pre_Buffer_Was_Full) { pre_Check_Addr = 0; pre_Force_Non_Seq = true; }

									if (!pre_Enable) { pre_Check_Addr = 0; }
								}
							}
							else
							{
								// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
								if (pre_Buffer_Was_Full && (pre_Buffer_Cnt == 0))
								{
									// console crashes when prefetcher is stopped mid instruction fetch
									wait_ret = (int)0x7FFFFFF0;

									//abandon the prefetcher current fetch and reset
									pre_Fetch_Cnt = 0;
									pre_Check_Addr = 0;
								}
								else
								{
									// plus 1 since the prefetcher already used this cycle, so don't double count
									wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;

									// it is as if the cpu takes over a regular access, so reset the pre-fetcher
									pre_Inactive = true;

									pre_Read_Addr += 2;
									pre_Check_Addr += 2;
									pre_Fetch_Cnt = 0;
									pre_Buffer_Cnt = 0;
									pre_Run = pre_Enable;

									if (!pre_Enable) { pre_Check_Addr = 0; }
								}
							}
						}
						else if ((addr & 0x1FFFC) == 0)
						{
							// we encountered a boundary, always use non-sequential timing						
							if (addr < 0x0A000000)
							{
								wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1;
							}
							else if (addr < 0x0C000000)
							{
								wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1;
							}
							else
							{
								wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1;
							}

							if (pre_Cycle_Glitch)
							{
								// lose 1 cycle if prefetcher is holding the bus
								wait_ret += 1;
							}

							pre_Read_Addr += 4;
							pre_Check_Addr += 4;
							pre_Fetch_Cnt = 0;

							if (!pre_Enable) { pre_Check_Addr = 0; pre_Run = false; }

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Inactive = true;
						}
						else
						{
							// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
							if (!Seq_Access && (pre_Fetch_Cnt == 1) && !pre_Following)
							{
								// this happens in a branch to the current prefetcher fetch address
								if (addr < 0x0A000000)
								{
									wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1;
								}
								else if (addr < 0x0C000000)
								{
									wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1;
								}
								else
								{
									wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1;
								}
							}
							else
							{
								// plus 1 since the prefetcher already used this cycle, so don't double count
								wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;

								// then add the second access
								if (addr < 0x0A000000)
								{
									wait_ret += ROM_Waits_0_S + 1; // ROM 0					
								}
								else if (addr < 0x0C000000)
								{
									wait_ret += ROM_Waits_1_S + 1; // ROM 1
								}
								else
								{
									wait_ret += ROM_Waits_2_S + 1; // ROM 2
								}
							}

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Inactive = true;

							pre_Read_Addr += 4;
							pre_Check_Addr += 4;
							pre_Fetch_Cnt = 0;
							pre_Run = pre_Enable;

							if (!pre_Enable) { pre_Check_Addr = 0; }
						}
					}
					else
					{
						// the address is not related to the current ones available to the prefetcher
						Seq_Access &= !pre_Force_Non_Seq;

						if (addr < 0x0A000000)
						{
							if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)				
						}
						else if (addr < 0x0C000000)
						{
							if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
						}
						else
						{
							if ((addr & 0x1FFFC) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
						}

						pre_Force_Non_Seq = false;

						if (pre_Cycle_Glitch)
						{
							// lose 1 cycle if prefetcher is holding the bus
							wait_ret += 1;
						}

						// abandon the prefetcher current fetch and reset the address
						pre_Buffer_Cnt = 0;
						pre_Fetch_Cnt = 0;
						pre_Run = pre_Enable;
						pre_Buffer_Was_Full = false;
						pre_Following = false;

						pre_Inactive = true;

						if (pre_Enable) { pre_Check_Addr = pre_Read_Addr = addr + 4; }
						else { pre_Check_Addr = 0; }
					}
				}
				else if (addr < 0x10000000)
				{
					wait_ret += SRAM_Waits; // SRAM

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;
					}

					// abandon the prefetcher current fetch and reset
					pre_Fetch_Cnt = 0;
					pre_Check_Addr = 0;
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if ((addr & 0x00010000) == 0x00010000)
					{
						if ((addr & 0x00004000) == 0x00004000)
						{
							if (ppu_VRAM_High_Access)
							{
								wait_ret += 1;
							}

							// set to true since we also need to check the next cycle
							ppu_VRAM_High_In_Use = true;
						}
						else
						{
							if ((ppu_BG_Mode >= 3) && (ppu_BG_Mode <= 5))
							{
								if (ppu_VRAM_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_In_Use = true;
							}
							else
							{
								if (ppu_VRAM_High_Access)
								{
									wait_ret += 1;
								}

								// set to true since we also need to check the next cycle
								ppu_VRAM_High_In_Use = true;
							}
						}
					}
					else
					{
						if (ppu_VRAM_Access)
						{
							wait_ret += 1;
						}

						// set to true since we also need to check the next cycle
						ppu_VRAM_In_Use = true;
					}
				}
				else
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					// check both edges of the access
					if (ppu_PALRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_PALRAM_In_Use = true;
				}
			}
			else if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
			}

			return wait_ret;
		}

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

		// GBP external commands
		uint64_t ser_GBP_Next_Start_Time;

		uint32_t ser_GBP_Div_Count;
		uint32_t ser_GBP_Transfer_Count;

		uint32_t ser_RECV_J, ser_TRANS_J;
		uint32_t ser_div_cnt, ser_Reload;

		uint16_t ser_Data_0, ser_Data_1, ser_Data_2, ser_Data_3, ser_Data_M;
		uint16_t ser_CTRL, ser_CTRL_J, ser_STAT_J, ser_Mode;
		uint16_t key_CTRL;

		uint8_t ser_SC, ser_SD, ser_SI, ser_SO;

		uint8_t ser_Mode_State, ser_Ctrl_Mode_State;

		uint8_t ser_Ext_Current_Console;

		uint8_t ser_Bit_Count, ser_Bit_Total, ser_Bit_Total_Send;

		bool ser_Ext_Update, ser_Ext_Tick;

		bool ser_Internal_Clock, ser_Start;

		uint8_t ser_Read_Reg_8(uint32_t addr)
		{
			uint8_t ret = 0;

			switch (addr)
			{
				case 0x120: ret = (uint8_t)(ser_Data_0 & 0xFF); break;
				case 0x121: ret = (uint8_t)((ser_Data_0 & 0xFF00) >> 8); break;
				case 0x122: ret = (uint8_t)(ser_Data_1 & 0xFF); break;
				case 0x123: ret = (uint8_t)((ser_Data_1 & 0xFF00) >> 8); break;
				case 0x124: ret = (uint8_t)(ser_Data_2 & 0xFF); break;
				case 0x125: ret = (uint8_t)((ser_Data_2 & 0xFF00) >> 8); break;
				case 0x126: ret = (uint8_t)(ser_Data_3 & 0xFF); break;
				case 0x127: ret = (uint8_t)((ser_Data_3 & 0xFF00) >> 8); break;
				case 0x128: ret = (uint8_t)(ser_CTRL & 0xFF); break;
				case 0x129: ret = (uint8_t)((ser_CTRL & 0xFF00) >> 8); break;
				case 0x12A: ret = (uint8_t)(ser_Data_M & 0xFF); break;
				case 0x12B: ret = (uint8_t)((ser_Data_M & 0xFF00) >> 8); break;

				case 0x130: ret = (uint8_t)(controller_state & 0xFF); Is_Lag = false; break;
				case 0x131: ret = (uint8_t)((controller_state & 0xFF00) >> 8); Is_Lag = false; break;
				case 0x132: ret = (uint8_t)(key_CTRL & 0xFF); break;
				case 0x133: ret = (uint8_t)((key_CTRL & 0xFF00) >> 8); break;

				case 0x134: ret = (uint8_t)(ser_Mode & 0xFF); break;
				case 0x135: ret = (uint8_t)((ser_Mode & 0xFF00) >> 8); break;
				case 0x136: ret = 0; break;
				case 0x137: ret = 0; break;

				case 0x140: ret = (uint8_t)(ser_CTRL_J & 0xFF); break;
				case 0x141: ret = (uint8_t)((ser_CTRL_J & 0xFF00) >> 8); break;
				case 0x142: ret = 0; break;
				case 0x143: ret = 0; break;

				case 0x150: ret = (uint8_t)(ser_RECV_J & 0xFF); break;
				case 0x151: ret = (uint8_t)((ser_RECV_J & 0xFF00) >> 8); break;
				case 0x152: ret = (uint8_t)((ser_RECV_J & 0xFF0000) >> 16); break;
				case 0x153: ret = (uint8_t)((ser_RECV_J & 0xFF000000) >> 24); break;
				case 0x154: ret = (uint8_t)(ser_TRANS_J & 0xFF); break;
				case 0x155: ret = (uint8_t)((ser_TRANS_J & 0xFF00) >> 8); break;
				case 0x156: ret = (uint8_t)((ser_TRANS_J & 0xFF0000) >> 16); break;
				case 0x157: ret = (uint8_t)((ser_TRANS_J & 0xFF000000) >> 24); break;
				case 0x158: ret = (uint8_t)(ser_STAT_J & 0xFF); break;
				case 0x159: ret = (uint8_t)((ser_STAT_J & 0xFF00) >> 8); break;
				case 0x15A: ret = 0; break;
				case 0x15B: ret = 0; break;

				default: ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (uint32_t)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		uint16_t ser_Read_Reg_16(uint32_t addr)
		{
			uint16_t ret = 0;

			switch (addr)
			{
				case 0x120: ret = ser_Data_0; break;
				case 0x122: ret = ser_Data_1; break;
				case 0x124: ret = ser_Data_2; break;
				case 0x126: ret = ser_Data_3; break;
				case 0x128: ret = ser_CTRL; break;
				case 0x12A: ret = ser_Data_M; break;

				case 0x130: ret = controller_state; Is_Lag = false; break;
				case 0x132: ret = key_CTRL; break;

				case 0x134: ret = ser_Mode; break;
				case 0x136: ret = 0; break;

				case 0x140: ret = ser_CTRL_J; break;
				case 0x142: ret = 0; break;

				case 0x150: ret = (uint16_t)(ser_RECV_J & 0xFFFF); break;
				case 0x152: ret = (uint16_t)((ser_RECV_J & 0xFFFF0000) >> 16); break;
				case 0x154: ret = (uint16_t)(ser_TRANS_J & 0xFFFF); break;
				case 0x156: ret = (uint16_t)((ser_TRANS_J & 0xFFFF0000) >> 16); break;
				case 0x158: ret = ser_STAT_J; break;
				case 0x15A: ret = 0; break;


				default: ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		uint32_t ser_Read_Reg_32(uint32_t addr)
		{
			uint32_t ret = 0;

			switch (addr)
			{
				case 0x120: ret = (uint32_t)((ser_Data_1 << 16) | ser_Data_0); break;
				case 0x124: ret = (uint32_t)((ser_Data_3 << 16) | ser_Data_2); break;
				case 0x128: ret = (uint32_t)((ser_Data_M << 16) | ser_CTRL); break;

				case 0x130: ret = (uint32_t)((key_CTRL << 16) | controller_state); Is_Lag = false; break;

				case 0x134: ret = (uint32_t)((0x00000000) | ser_Mode); break;

				case 0x140: ret = (uint32_t)((0x00000000) | ser_CTRL_J); break;

				case 0x150: ret = ser_RECV_J; break;
				case 0x154: ret = ser_TRANS_J; break;
				case 0x158: ret = (uint32_t)((0x00000000) | ser_STAT_J); break;

				default: ret = cpu_Last_Bus_Value; break;
			}

			return ret;
		}

		void ser_Write_Reg_8(uint32_t addr, uint8_t value)
		{
			bool writable = ser_Check_Write(addr);

			if (writable)
			{
				switch (addr)
				{
					case 0x120: ser_Data_0 = (uint16_t)((ser_Data_0 & 0xFF00) | value); break;
					case 0x121: ser_Data_0 = (uint16_t)((ser_Data_0 & 0x00FF) | (value << 8)); break;
					case 0x122: ser_Data_1 = (uint16_t)((ser_Data_1 & 0xFF00) | value); break;
					case 0x123: ser_Data_1 = (uint16_t)((ser_Data_1 & 0x00FF) | (value << 8)); break;
					case 0x124: ser_Data_2 = (uint16_t)((ser_Data_2 & 0xFF00) | value); break;
					case 0x125: ser_Data_2 = (uint16_t)((ser_Data_2 & 0x00FF) | (value << 8)); break;
					case 0x126: ser_Data_3 = (uint16_t)((ser_Data_3 & 0xFF00) | value); break;
					case 0x127: ser_Data_3 = (uint16_t)((ser_Data_3 & 0x00FF) | (value << 8)); break;
					case 0x128: ser_CTRL_Update((uint16_t)((ser_CTRL & 0xFF00) | value)); break;
					case 0x129: ser_CTRL_Update((uint16_t)((ser_CTRL & 0x00FF) | (value << 8))); break;
					case 0x12A: ser_Data_M = (uint16_t)((ser_Data_M & 0xFF00) | value); break;
					case 0x12B: ser_Data_M = (uint16_t)((ser_Data_M & 0x00FF) | (value << 8)); break;

					case 0x130: // no effect
					case 0x131: // no effect
					case 0x132: key_CTRL = (uint16_t)((key_CTRL & 0xFF00) | value); do_controller_check(true); do_controller_check_glitch(); break;
						// note no check here, does not seem to trigger onhardware, see joypad.gba
					case 0x133: key_CTRL = (uint16_t)((key_CTRL & 0x00FF) | (value << 8)); /*do_controller_check(); do_controller_check_glitch(); */ break;

					case 0x134: ser_Mode_Update((uint16_t)((ser_Mode & 0xFF00) | value)); break;
					case 0x135: ser_Mode_Update((uint16_t)((ser_Mode & 0x00FF) | (value << 8))); break;

					case 0x140: ser_JoyCnt_Update((uint16_t)((ser_CTRL_J & 0xFF00) | value)); break;
					case 0x141: ser_JoyCnt_Update((uint16_t)((ser_CTRL_J & 0x00FF) | (value << 8))); break;

					case 0x150: ser_RECV_J = (uint32_t)((ser_RECV_J & 0xFFFFFF00) | value); break;
					case 0x151: ser_RECV_J = (uint32_t)((ser_RECV_J & 0xFFFF00FF) | (value << 8)); break;
					case 0x152: ser_RECV_J = (uint32_t)((ser_RECV_J & 0xFF00FFFF) | (value << 16)); break;
					case 0x153: ser_RECV_J = (uint32_t)((ser_RECV_J & 0x00FFFFFF) | (value << 24)); break;
					case 0x154: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0xFFFFFF00) | value); break;
					case 0x155: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0xFFFF00FF) | (value << 8)); break;
					case 0x156: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0xFF00FFFF) | (value << 16)); break;
					case 0x157: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0x00FFFFFF) | (value << 24)); break;
					case 0x158: ser_STAT_J = (uint16_t)((ser_STAT_J & 0xFF00) | value); break;
					case 0x159: ser_STAT_J = (uint16_t)((ser_STAT_J & 0x00FF) | (value << 8)); break;
				}
			}
		}

		void ser_Write_Reg_16(uint32_t addr, uint16_t value)
		{
			bool writable = ser_Check_Write(addr);

			if (writable)
			{
				switch (addr)
				{
					case 0x120: ser_Data_0 = value; break;
					case 0x122: ser_Data_1 = value; break;
					case 0x124: ser_Data_2 = value; break;
					case 0x126: ser_Data_3 = value; break;
					case 0x128: ser_CTRL_Update(value); break;
					case 0x12A: ser_Data_M = value; break;

					case 0x130: // no effect
					case 0x132: key_CTRL = value; do_controller_check(true); do_controller_check_glitch(); break;

					case 0x134: ser_Mode_Update(value); break;

					case 0x140: ser_JoyCnt_Update(value); break;

					case 0x150: ser_RECV_J = (uint32_t)((ser_RECV_J & 0xFFFF0000) | value); break;
					case 0x152: ser_RECV_J = (uint32_t)((ser_RECV_J & 0x0000FFFF) | (value << 16)); break;
					case 0x154: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0xFFFF0000) | value); break;
					case 0x156: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0x0000FFFF) | (value << 16)); break;
					case 0x158: ser_STAT_J = value; break;
				}
			}
		}

		void ser_Write_Reg_32(uint32_t addr, uint32_t value)
		{
			bool writable = ser_Check_Write(addr);

			if (writable)
			{
				switch (addr)
				{
					case 0x120: ser_Data_0 = (uint16_t)(value & 0xFFFF);
						ser_Data_1 = (uint16_t)((value >> 16) & 0xFFFF); break;
					case 0x124: ser_Data_2 = (uint16_t)(value & 0xFFFF);
						ser_Data_3 = (uint16_t)((value >> 16) & 0xFFFF); break;
					case 0x128: ser_CTRL_Update((uint16_t)(value & 0xFFFF));
						// need seperate case here
						if (ser_Check_Write(0x12A)) { ser_Data_M = (uint16_t)((value >> 16) & 0xFFFF); }
						break;

					case 0x130: key_CTRL = (uint16_t)((value >> 16) & 0xFFFF); do_controller_check(true); do_controller_check_glitch(); break;

					case 0x134: ser_Mode_Update((uint16_t)(value & 0xFFFF)); break;

					case 0x140: ser_JoyCnt_Update((uint16_t)(value & 0xFFFF)); break;

					case 0x150: ser_RECV_J = value; break;
					case 0x154: ser_TRANS_J = value; break;
					case 0x158: ser_STAT_J = (uint16_t)(value & 0xFFFF); break;
				}
			}
		}

		bool ser_Check_Write(uint16_t addr)
		{
			addr &= 0xFFFE;

			switch (addr)
			{
				case 0x120: 
					if ((ser_Mode_State < 2) && (ser_Ctrl_Mode_State < 2) && (ser_Bit_Total == 32)) { return true; }
					return false;

				case 0x122:
					if ((ser_Mode_State < 2) && (ser_Ctrl_Mode_State < 2) && (ser_Bit_Total == 32)) { return true; }
					return false;

				case 0x124: return false;

				case 0x126: return false;

				case 0x128: return true;

				case 0x12A:
					if ((ser_Mode_State < 2) && (ser_Ctrl_Mode_State == 3)) { return false; }
					
					return true;

				case 0x150: return false;

				case 0x152: return false;

				case 0x154: return false;

				case 0x156: return false;

				case 0x158: return false;
			}
					
			return true;
		}

		void ser_CTRL_Update(uint16_t value)
		{
			ser_Ctrl_Mode_State = (uint8_t)((value & 0x3000) >> 12);

			ser_Ext_Update = true;

			if (ser_Mode_State == 3)
			{

			}
			else if (ser_Mode_State == 2)
			{

			}
			else
			{
				if (ser_Ctrl_Mode_State == 3)
				{
					// uart

				}
				else if (ser_Ctrl_Mode_State == 2)
				{
					// multiplayer
					ser_CTRL = (uint16_t)((value & 0x7F83) | (ser_CTRL & 0x70));

					// status bit will be set extrenally
					ser_CTRL &= 0xFFF7;

					if (ext_num == 1)
					{
						ser_CTRL &= 0xFFFB;

						// actiavte the port
						if (!ser_Start && ((value & 0x80) == 0x80))
						{
							ser_Bit_Count = 0;

							ser_Bit_Total_Send = 36;

							if ((value & 3) == 0)
							{
								ser_Reload = 0x378;
							}
							else if ((value & 3) == 1)
							{
								ser_Reload = 0xE9;
							}
							else if ((value & 3) == 2)
							{
								ser_Reload = 0xA0;
							}
							else if ((value & 3) == 3)
							{
								ser_Reload = 0x57;
							}

							ser_div_cnt = 6;

							ser_Internal_Clock = true;

							ser_Start = true;

							ser_Ext_Current_Console = 1;

							ser_CTRL |= 0x80;
						}

						if ((value & 0x80) != 0x80) { ser_Start = false; }
					}
					else
					{
						ser_CTRL |= 4;

						if ((value & 0x80) != 0x80) { ser_Start = false; }
					}
				}
				else
				{
					// normal

					ser_Bit_Total = (uint8_t)((value & 0x1000) == 0x1000 ? 32 : 8);

					// actiavte the port
					if (!ser_Start && ((value & 0x80) == 0x80))
					{
						ser_Bit_Count = 0;

						ser_Bit_Total_Send = ser_Bit_Total;

						ser_Reload = (uint8_t)((value & 0x2) == 0x2 ? 0x8 : 0x40);

						ser_div_cnt = 6;

						ser_Internal_Clock = (value & 0x1) == 0x1;

						ser_Start = true;
					}

					if ((value & 0x80) != 0x80) { ser_Start = false; }

					ser_CTRL = value;

					if (ext_num == 0)
					{
						ser_CTRL |= 4; // open, no connection
					}
					else
					{
						ser_CTRL |= (uint8_t)(ser_SI << 2);
					}

					// GBP features
					if (!ser_Internal_Clock && GBP_Mode_Enabled && ser_Start && (ser_Bit_Total == 32))
					{
						// start on scanline 16
						if (ser_GBP_Transfer_Count == 0)
						{
							if (ppu_LY < 16)
							{
								ser_GBP_Next_Start_Time = CycleCount + 16 * 1232 - FrameCycle;
							}
							else
							{
								ser_GBP_Next_Start_Time = CycleCount + 16 * 1232 + 228 * 1232 - FrameCycle;
							}
						}
						else
						{
							ser_GBP_Next_Start_Time = CycleCount + 32;
						}

						ser_GBP_Div_Count = 0;
					}

					//Message_String = "ser: " + to_string(ser_CTRL) + " " + to_string(ser_Start) + " " + to_string(CycleCount);

					//MessageCallback(Message_String.length());
				}
			}

			ser_CTRL &= 0x7FFF;

		}

		void ser_Mode_Update(uint16_t value)
		{
			ser_Mode = value;

			ser_Mode_State = (uint8_t)((value & 0xC000) >> 14);

			ser_Ext_Update = true;
		}

		void ser_JoyCnt_Update(uint16_t value)
		{
			ser_CTRL_J = value & 0x40;
		}

		void ser_Reset()
		{
			ser_RECV_J = ser_TRANS_J = 0;

			ser_Data_0 = ser_Data_1 = ser_Data_2 = ser_Data_3 = ser_Data_M = 0;

			ser_CTRL = 4; // assuming no connection

			ser_SC = ser_SD = ser_SI = ser_SO = 0;

			ser_Mode_State = ser_Ctrl_Mode_State = 0;

			ser_CTRL_J = ser_STAT_J = ser_Mode = 0;

			key_CTRL = 0;

			ser_div_cnt = 0;

			ser_Reload = 0xF;

			ser_Bit_Count = 0;

			ser_Bit_Total = ser_Bit_Total_Send = 8;

			ser_Ext_Current_Console = 0;

			ser_Internal_Clock = ser_Start = false;

			ser_Ext_Update = ser_Ext_Tick = false;

			ser_GBP_Next_Start_Time = 0;

			ser_GBP_Div_Count = 0;

			ser_GBP_Transfer_Count = 0;
		}

		uint8_t* ser_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ser_Internal_Clock, saver);
			saver = bool_saver(ser_Start, saver);
			saver = bool_saver(ser_Ext_Update, saver);
			saver = bool_saver(ser_Ext_Tick, saver);

			saver = byte_saver(ser_Bit_Count, saver);
			saver = byte_saver(ser_Bit_Total, saver);
			saver = byte_saver(ser_Bit_Total_Send, saver);

			saver = byte_saver(ser_SC, saver);
			saver = byte_saver(ser_SD, saver);
			saver = byte_saver(ser_SI, saver);
			saver = byte_saver(ser_SO, saver);
			saver = byte_saver(ser_Mode_State, saver);
			saver = byte_saver(ser_Ctrl_Mode_State, saver);
			saver = byte_saver(ser_Ext_Current_Console, saver);

			saver = short_saver(ser_Data_0, saver);
			saver = short_saver(ser_Data_1, saver);
			saver = short_saver(ser_Data_2, saver);
			saver = short_saver(ser_Data_3, saver);
			saver = short_saver(ser_Data_M, saver);
			saver = short_saver(ser_CTRL, saver);
			saver = short_saver(ser_CTRL_J, saver);
			saver = short_saver(ser_STAT_J, saver);
			saver = short_saver(ser_Mode, saver);
			saver = short_saver(key_CTRL, saver);

			saver = int_saver(ser_RECV_J, saver);
			saver = int_saver(ser_TRANS_J, saver);
			saver = int_saver(ser_div_cnt, saver);
			saver = int_saver(ser_Reload, saver);

			saver = int_saver(ser_GBP_Div_Count, saver);
			saver = int_saver(ser_GBP_Transfer_Count, saver);

			saver = long_saver(ser_GBP_Next_Start_Time, saver);

			return saver;
		}

		uint8_t* ser_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ser_Internal_Clock, loader);
			loader = bool_loader(&ser_Start, loader);
			loader = bool_loader(&ser_Ext_Update, loader);
			loader = bool_loader(&ser_Ext_Tick, loader);

			loader = byte_loader(&ser_Bit_Count, loader);
			loader = byte_loader(&ser_Bit_Total, loader);
			loader = byte_loader(&ser_Bit_Total_Send, loader);

			loader = byte_loader(&ser_SC, loader);
			loader = byte_loader(&ser_SD, loader);
			loader = byte_loader(&ser_SI, loader);
			loader = byte_loader(&ser_SO, loader);
			loader = byte_loader(&ser_Mode_State, loader);
			loader = byte_loader(&ser_Ctrl_Mode_State, loader);
			loader = byte_loader(&ser_Ext_Current_Console, loader);

			loader = short_loader(&ser_Data_0, loader);
			loader = short_loader(&ser_Data_1, loader);
			loader = short_loader(&ser_Data_2, loader);
			loader = short_loader(&ser_Data_3, loader);
			loader = short_loader(&ser_Data_M, loader);
			loader = short_loader(&ser_CTRL, loader);
			loader = short_loader(&ser_CTRL_J, loader);
			loader = short_loader(&ser_STAT_J, loader);
			loader = short_loader(&ser_Mode, loader);
			loader = short_loader(&key_CTRL, loader);

			loader = int_loader(&ser_RECV_J, loader);
			loader = int_loader(&ser_TRANS_J, loader);
			loader = int_loader(&ser_div_cnt, loader);
			loader = int_loader(&ser_Reload, loader);

			loader = int_loader(&ser_GBP_Div_Count, loader);
			loader = int_loader(&ser_GBP_Transfer_Count, loader);

			loader = long_loader(&ser_GBP_Next_Start_Time, loader);

			return loader;
		}

	#pragma endregion

	#pragma region Timer
		
		bool tim_All_Off;

		uint16_t tim_SubCnt;

		uint16_t tim_Old_Reload;

		uint32_t tim_Just_Reloaded;

		bool tim_Go[4] = { };
		bool tim_Tick_By_Prev[4] = { };
		bool tim_Prev_Tick[5] = { };
		bool tim_Disable[4] = { };
		bool tim_Old_IRQ[4] = { };
		bool tim_Glitch_Tick[4] = { };
		bool tim_Enable_Not_Update[4] = { };

		uint16_t tim_Timer[4] = { };
		uint16_t tim_Reload[4] = { };
		uint16_t tim_Control[4] = { };
		uint16_t tim_PreSc[4] = { };
		uint16_t tim_PreSc_En[4] = { };
		uint16_t tim_ST_Time[4] = { };

		uint16_t PreScales[4] = {0, 0x3F, 0xFF, 0x3FF};

		uint8_t tim_Read_Reg_8(uint32_t addr)
		{
			uint8_t ret = 0;

			switch (addr)
			{
				case 0x100: ret = (uint8_t)(tim_Timer[0] & 0xFF); break;
				case 0x101: ret = (uint8_t)((tim_Timer[0] & 0xFF00) >> 8); break;
				case 0x102: ret = (uint8_t)(tim_Control[0] & 0xFF); break;
				case 0x103: ret = (uint8_t)((tim_Control[0] & 0xFF00) >> 8); break;

				case 0x104: ret = (uint8_t)(tim_Timer[1] & 0xFF); break;
				case 0x105: ret = (uint8_t)((tim_Timer[1] & 0xFF00) >> 8); break;
				case 0x106: ret = (uint8_t)(tim_Control[1] & 0xFF); break;
				case 0x107: ret = (uint8_t)((tim_Control[1] & 0xFF00) >> 8); break;

				case 0x108: ret = (uint8_t)(tim_Timer[2] & 0xFF); break;
				case 0x109: ret = (uint8_t)((tim_Timer[2] & 0xFF00) >> 8); break;
				case 0x10A: ret = (uint8_t)(tim_Control[2] & 0xFF); break;
				case 0x10B: ret = (uint8_t)((tim_Control[2] & 0xFF00) >> 8); break;

				case 0x10C: ret = (uint8_t)(tim_Timer[3] & 0xFF); break;
				case 0x10D: ret = (uint8_t)((tim_Timer[3] & 0xFF00) >> 8); break;
				case 0x10E: ret = (uint8_t)(tim_Control[3] & 0xFF); break;
				case 0x10F: ret = (uint8_t)((tim_Control[3] & 0xFF00) >> 8); break;

				default: ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (uint32_t)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		uint16_t tim_Read_Reg_16(uint32_t addr)
		{
			uint16_t ret = 0;

			switch (addr)
			{
				case 0x100: ret = tim_Timer[0]; break;
				case 0x102: ret = tim_Control[0]; break;

				case 0x104: ret = tim_Timer[1]; break;
				case 0x106: ret = tim_Control[1]; break;

				case 0x108: ret = tim_Timer[2]; break;
				case 0x10A: ret = tim_Control[2]; break;

				case 0x10C: ret = tim_Timer[3]; break;
				case 0x10E: ret = tim_Control[3]; break;

				default: ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		uint32_t tim_Read_Reg_32(uint32_t addr)
		{
			uint32_t ret = 0;

			switch (addr)
			{
				case 0x100: ret = (uint32_t)((tim_Control[0] << 16) | tim_Timer[0]); break;

				case 0x104: ret = (uint32_t)((tim_Control[1] << 16) | tim_Timer[1]); break;

				case 0x108: ret = (uint32_t)((tim_Control[2] << 16) | tim_Timer[2]); break;

				case 0x10C: ret = (uint32_t)((tim_Control[3] << 16) | tim_Timer[3]); break;

				default: ret = cpu_Last_Bus_Value; break;
			}

			return ret;
		}

		void tim_Write_Reg_8(uint32_t addr, uint8_t value)
		{
			switch (addr)
			{
				case 0x100: tim_rld_upd((uint16_t)((tim_Reload[0] & 0xFF00) | value), 0); break;
				case 0x101: tim_rld_upd((uint16_t)((tim_Reload[0] & 0x00FF) | (value << 8)), 0); break;
				case 0x102: tim_upd((uint16_t)((tim_Control[0] & 0xFF00) | value), 0); break;
				case 0x103: tim_Control[0] = (uint16_t)((tim_Control[0] & 0x00FF) | (value << 8)); break;

				case 0x104: tim_rld_upd((uint16_t)((tim_Reload[1] & 0xFF00) | value), 1); break;
				case 0x105: tim_rld_upd((uint16_t)((tim_Reload[1] & 0x00FF) | (value << 8)), 1); break;
				case 0x106: tim_upd((uint16_t)((tim_Control[1] & 0xFF00) | value), 1); break;
				case 0x107: tim_Control[1] = (uint16_t)((tim_Control[1] & 0x00FF) | (value << 8)); break;

				case 0x108: tim_rld_upd((uint16_t)((tim_Reload[2] & 0xFF00) | value), 2); break;
				case 0x109: tim_rld_upd((uint16_t)((tim_Reload[2] & 0x00FF) | (value << 8)), 2); break;
				case 0x10A: tim_upd((uint16_t)((tim_Control[2] & 0xFF00) | value), 2); break;
				case 0x10B: tim_Control[2] = (uint16_t)((tim_Control[2] & 0x00FF) | (value << 8)); break;

				case 0x10C: tim_rld_upd((uint16_t)((tim_Reload[3] & 0xFF00) | value), 3); break;
				case 0x10D: tim_rld_upd((uint16_t)((tim_Reload[3] & 0x00FF) | (value << 8)), 3); break;
				case 0x10E: tim_upd((uint16_t)((tim_Control[3] & 0xFF00) | value), 3); break;
				case 0x10F: tim_Control[3] = (uint16_t)((tim_Control[3] & 0x00FF) | (value << 8)); break;
			}
		}

		void tim_Write_Reg_16(uint32_t addr, uint16_t value)
		{
			switch (addr)
			{
				case 0x100: tim_rld_upd(value, 0); break;
				case 0x102: tim_upd(value, 0); break;

				case 0x104: tim_rld_upd(value, 1); break;
				case 0x106: tim_upd(value, 1); break;

				case 0x108: tim_rld_upd(value, 2); break;
				case 0x10A: tim_upd(value, 2); break;

				case 0x10C: tim_rld_upd(value, 3); break;
				case 0x10E: tim_upd(value, 3); break;
			}
		}

		// Note that in case of 32 bit write, new reload value is used when enabling the timer
		void tim_Write_Reg_32(uint32_t addr, uint32_t value)
		{
			switch (addr)
			{
				case 0x100: tim_rld_upd((uint16_t)(value & 0xFFFF), 0);
					tim_upd((uint16_t)((value >> 16) & 0xFFFF), 0); break;

				case 0x104: tim_rld_upd((uint16_t)(value & 0xFFFF), 1);
					tim_upd((uint16_t)((value >> 16) & 0xFFFF), 1); break;

				case 0x108: tim_rld_upd((uint16_t)(value & 0xFFFF), 2);
					tim_upd((uint16_t)((value >> 16) & 0xFFFF), 2); break;

				case 0x10C: tim_rld_upd((uint16_t)(value & 0xFFFF), 3);
					tim_upd((uint16_t)((value >> 16) & 0xFFFF), 3); break;
			}
		}

		void tim_rld_upd(uint16_t value, uint32_t nbr)
		{
			tim_Old_Reload = tim_Reload[nbr];

			tim_Reload[nbr] = value;

			tim_Just_Reloaded = nbr;
		}

		void tim_upd(uint16_t value, uint32_t nbr)
		{
			if (((tim_Control[nbr] & 0x80) == 0) && ((value & 0x80) != 0))
			{
				// if enabling when internal timer value is 0xFFFF, an extra tick cycle occurs before resetting
				// so it may trigger an interrupt
				tim_Glitch_Tick[nbr] = false;

				if (tim_Timer[nbr] == 0xFFFF)
				{
					// TODO: check cases of ticking by previous timer
					if ((nbr == 0) || ((value & 0x4) == 0))
					{
						tim_Glitch_Tick[nbr] = true;
					}
				}

				tim_ST_Time[nbr] = 3;

				tim_PreSc_En[nbr] = PreScales[value & 3];

				if (nbr != 0) { tim_Tick_By_Prev[nbr] = ((value & 0x4) == 0x4); }

				tim_All_Off = false;

				tim_Enable_Not_Update[nbr] = true;
			}
			else if (((tim_Control[nbr] & 0x80) != 0) && ((value & 0x80) != 0))
			{		
				// some settings can be updated even while the timer is running (which ones?)
				// what happens if these changes happen very close together? (The cpu could change them within 2 clocks, but takes 3 to start channel)
				// for now use the new value
				tim_PreSc_En[nbr] = PreScales[value & 3];

				// TODO: check exact changeover timing
				if (nbr != 0) { tim_Tick_By_Prev[nbr] = ((value & 0x4) == 0x4); }

				if (tim_ST_Time[nbr] == 0)
				{
					tim_ST_Time[nbr] = 2;
				}

				tim_Enable_Not_Update[nbr] = false;
			}

			if ((value & 0x80) == 0)
			{
				// timer ticks for one additional cycle when disabled if currently running
				if (tim_Go[nbr] || (tim_ST_Time[nbr] == 1))
				{
					tim_Disable[nbr] = true;

					tim_Old_IRQ[nbr] = (tim_Control[nbr] & 0x40) == 0x40;
				}
				else if (tim_ST_Time[nbr] > 1)
				{
					tim_ST_Time[nbr] = 0;

					tim_All_Off = true;

					for (int k = 0; k < 4; k++)
					{
						tim_All_Off &= !tim_Go[k];
						tim_All_Off &= (tim_ST_Time[k] == 0);
					}

					tim_Old_IRQ[nbr] = false;
				}
			}

			//Message_String = "rld " + to_string(nbr) + " v " + to_string(value) + " t " + to_string(tim_Timer[nbr]) + " sub " + to_string(tim_SubCnt) + " " + to_string(CycleCount);

			//MessageCallback(Message_String.length());

			// bits 3-5 always 0
			// bit 2 always 0 for timer 0
			if (nbr == 0)
			{
				value &= 0xFFC3;
			}
			else
			{
				value &= 0xFFC7;
			}


			tim_Control[nbr] = value;
		}

		void tim_Reset()
		{
			for (int i = 0; i < 4; i++)
			{
				tim_Timer[i] = 0;
				tim_Reload[i] = 0;
				tim_Control[i] = 0;
				tim_PreSc[i] = 0;
				tim_PreSc_En[i] = 0;
				tim_ST_Time[i] = 0;

				tim_Go[i] = false;
				tim_Tick_By_Prev[i] = false;
				tim_Prev_Tick[i] = false;
				tim_Disable[i] = false;
				tim_Old_IRQ[i] = false;
				tim_Glitch_Tick[i] = false;
				tim_Enable_Not_Update[i] = false;
			}

			tim_Just_Reloaded = 5;

			tim_SubCnt = 0xFFFF;

			tim_Old_Reload = 0;

			tim_All_Off = true;
		}

		uint8_t* tim_SaveState(uint8_t* saver)
		{
			saver = bool_saver(tim_All_Off, saver);

			saver = short_saver(tim_SubCnt, saver);
			saver = short_saver(tim_Old_Reload, saver);

			saver = int_saver(tim_Just_Reloaded, saver);

			saver = bool_array_saver(tim_Go, saver, 4);
			saver = bool_array_saver(tim_Tick_By_Prev, saver, 4);
			saver = bool_array_saver(tim_Prev_Tick, saver, 5);
			saver = bool_array_saver(tim_Disable, saver, 4);
			saver = bool_array_saver(tim_Old_IRQ, saver, 4);
			saver = bool_array_saver(tim_Glitch_Tick, saver, 4);
			saver = bool_array_saver(tim_Enable_Not_Update, saver, 4);

			saver = short_array_saver(tim_Timer, saver, 4);
			saver = short_array_saver(tim_Reload, saver, 4);
			saver = short_array_saver(tim_Control, saver, 4);
			saver = short_array_saver(tim_PreSc, saver, 4);
			saver = short_array_saver(tim_PreSc_En, saver, 4);
			saver = short_array_saver(tim_ST_Time, saver, 4);

			return saver;
		}

		uint8_t* tim_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&tim_All_Off, loader);

			loader = short_loader(&tim_SubCnt, loader);
			loader = short_loader(&tim_Old_Reload, loader);

			loader = int_loader(&tim_Just_Reloaded, loader);

			loader = bool_array_loader(tim_Go, loader, 4);
			loader = bool_array_loader(tim_Tick_By_Prev, loader, 4);
			loader = bool_array_loader(tim_Prev_Tick, loader, 5);
			loader = bool_array_loader(tim_Disable, loader, 4);
			loader = bool_array_loader(tim_Old_IRQ, loader, 4);
			loader = bool_array_loader(tim_Glitch_Tick, loader, 4);
			loader = bool_array_loader(tim_Enable_Not_Update, loader, 4);

			loader = short_array_loader(tim_Timer, loader, 4);
			loader = short_array_loader(tim_Reload, loader, 4);
			loader = short_array_loader(tim_Control, loader, 4);
			loader = short_array_loader(tim_PreSc, loader, 4);
			loader = short_array_loader(tim_PreSc_En, loader, 4);
			loader = short_array_loader(tim_ST_Time, loader, 4);

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

			// GBA tek says lower bit of tile number should be ignored in some cases, but it appears this is not the case?
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

		// GBA specific registers
		uint32_t snd_Wave_Bank, snd_Wave_Bank_Playing;
		uint32_t snd_Chan_Mult;
		uint8_t snd_CTRL_GBA_Low, snd_CTRL_GBA_High;
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

						case 0x82:                                              // GBA Low (ctrl)
							snd_Chan_Mult = snd_Chan_Mult_Table[value & 3];

							snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 6 : 3;
							snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 6 : 3;

							value &= 0xF;
							break;

						case 0x83:                                              // GBA High (ctrl)
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
					case 0x82:                                              // GBA Low (ctrl)
						snd_Chan_Mult = snd_Chan_Mult_Table[value & 3];

						snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 6 : 3;
						snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 6 : 3;

						value &= 0xF;
						break;

					case 0x83:                                              // GBA High (ctrl)
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

			snd_CTRL_GBA_Low = snd_CTRL_GBA_High = 0;
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

			// GBA specific registers
			saver = int_saver(snd_Wave_Bank, saver);
			saver = int_saver(snd_Wave_Bank_Playing, saver);
			saver = int_saver(snd_Chan_Mult, saver);
			saver = byte_saver(snd_CTRL_GBA_Low, saver);
			saver = byte_saver(snd_CTRL_GBA_High, saver);
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

			// GBA specific registers
			loader = int_loader(&snd_Wave_Bank, loader);
			loader = int_loader(&snd_Wave_Bank_Playing, loader);
			loader = int_loader(&snd_Chan_Mult, loader);
			loader = byte_loader(&snd_CTRL_GBA_Low, loader);
			loader = byte_loader(&snd_CTRL_GBA_High, loader);
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
			saver = bool_saver(All_RAM_Disable, saver);
			saver = bool_saver(WRAM_Enable, saver);
			saver = bool_saver(INT_Master_On, saver);
			saver = bool_saver(Cart_RAM_Present, saver);
			saver = bool_saver(Is_EEPROM, saver);
			saver = bool_saver(EEPROM_Wiring, saver);

			saver = bool_saver(delays_to_process, saver);
			saver = bool_saver(IRQ_Write_Delay, saver);

			saver = bool_saver(VRAM_32_Check, saver);
			saver = bool_saver(PALRAM_32_Check, saver);
			saver = bool_saver(VRAM_32_Delay, saver);
			saver = bool_saver(PALRAM_32_Delay, saver);
			saver = bool_saver(IRQ_Delays, saver);
			saver = bool_saver(Misc_Delays, saver);
			saver = bool_saver(FIFO_DMA_A_Delay, saver);
			saver = bool_saver(FIFO_DMA_B_Delay, saver);
			saver = bool_saver(DMA_Any_Start, saver);
			saver = bool_saver(DMA_Any_IRQ, saver);
			saver = bool_saver(Halt_Enter, saver);
			saver = bool_saver(Halt_Leave, saver);
			saver = bool_saver(GBP_Mode_Enabled, saver);
			saver = bool_saver(Rumble_State, saver);

			saver = byte_saver(Post_Boot, saver);
			saver = byte_saver(Halt_CTRL, saver);
			saver = byte_saver(ext_num, saver);

			saver = short_saver(INT_EN, saver);
			saver = short_saver(INT_Flags, saver);
			saver = short_saver(INT_Master, saver);
			saver = short_saver(Wait_CTRL, saver);
			saver = short_saver(INT_Flags_Gather, saver);
			saver = short_saver(INT_Flags_Use, saver);

			saver = short_saver(controller_state, saver);
			saver = short_saver(controller_state_old, saver);
			saver = short_saver(PALRAM_32W_Value, saver);
			saver = short_saver(VRAM_32W_Value, saver);
			saver = short_saver(FIFO_DMA_A_cd, saver);
			saver = short_saver(FIFO_DMA_B_cd, saver);
			saver = short_saver(Halt_Enter_cd, saver);
			saver = short_saver(Halt_Leave_cd, saver);
			saver = short_saver(Halt_Held_CPU_Instr, saver);

			saver = int_saver(PALRAM_32W_Addr, saver);
			saver = int_saver(VRAM_32W_Addr, saver);
			saver = int_saver(Memory_CTRL, saver);
			saver = int_saver(ROM_Length, saver);

			saver = int_saver(Last_BIOS_Read, saver);

			saver = int_saver(WRAM_Waits, saver);
			saver = int_saver(SRAM_Waits, saver);

			saver = int_saver(ROM_Waits_0_N, saver);
			saver = int_saver(ROM_Waits_1_N, saver);
			saver = int_saver(ROM_Waits_2_N, saver);
			saver = int_saver(ROM_Waits_0_S, saver);
			saver = int_saver(ROM_Waits_1_S, saver);
			saver = int_saver(ROM_Waits_2_S, saver);

			saver = bool_array_saver(DMA_Start_Delay, saver, 4);
			saver = bool_array_saver(DMA_IRQ_Delay, saver, 4);
			
			saver = byte_array_saver(WRAM, saver, 0x40000);
			saver = byte_array_saver(IWRAM, saver, 0x8000);
			saver = byte_array_saver(PALRAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x18000);
			saver = byte_array_saver(OAM, saver, 0x400);

			// Prefetcher
			saver = bool_saver(pre_Cycle_Glitch, saver);
			saver = bool_saver(pre_Run, saver);
			saver = bool_saver(pre_Enable, saver);
			saver = bool_saver(pre_Force_Non_Seq, saver);
			saver = bool_saver(pre_Buffer_Was_Full, saver);
			saver = bool_saver(pre_Boundary_Reached, saver);
			saver = bool_saver(pre_Following, saver);
			saver = bool_saver(pre_Inactive, saver);

			saver = int_saver(pre_Read_Addr, saver);
			saver = int_saver(pre_Check_Addr, saver);
			saver = int_saver(pre_Buffer_Cnt, saver);

			saver = int_saver(pre_Fetch_Cnt, saver);
			saver = int_saver(pre_Fetch_Wait, saver);

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
			loader = bool_loader(&All_RAM_Disable, loader);
			loader = bool_loader(&WRAM_Enable, loader);
			loader = bool_loader(&INT_Master_On, loader);
			loader = bool_loader(&Cart_RAM_Present, loader);
			loader = bool_saver(&Is_EEPROM, loader);
			loader = bool_saver(&EEPROM_Wiring, loader);

			loader = bool_loader(&delays_to_process, loader);
			loader = bool_loader(&IRQ_Write_Delay, loader);

			loader = bool_loader(&VRAM_32_Check, loader);
			loader = bool_loader(&PALRAM_32_Check, loader);
			loader = bool_loader(&VRAM_32_Delay, loader);
			loader = bool_loader(&PALRAM_32_Delay, loader);
			loader = bool_loader(&IRQ_Delays, loader);
			loader = bool_loader(&Misc_Delays, loader);
			loader = bool_loader(&FIFO_DMA_A_Delay, loader);
			loader = bool_loader(&FIFO_DMA_B_Delay, loader);
			loader = bool_loader(&DMA_Any_Start, loader);
			loader = bool_loader(&DMA_Any_IRQ, loader);
			loader = bool_loader(&Halt_Enter, loader);
			loader = bool_loader(&Halt_Leave, loader);
			loader = bool_loader(&GBP_Mode_Enabled, loader);
			loader = bool_loader(&Rumble_State, loader);

			loader = byte_loader(&Post_Boot, loader);
			loader = byte_loader(&Halt_CTRL, loader);
			loader = byte_loader(&ext_num, loader);

			loader = short_loader(&INT_EN, loader);
			loader = short_loader(&INT_Flags, loader);
			loader = short_loader(&INT_Master, loader);
			loader = short_loader(&Wait_CTRL, loader);
			loader = short_loader(&INT_Flags_Gather, loader);
			loader = short_loader(&INT_Flags_Use, loader);

			loader = short_loader(&controller_state, loader);
			loader = short_loader(&controller_state_old, loader);
			loader = short_loader(&PALRAM_32W_Value, loader);
			loader = short_loader(&VRAM_32W_Value, loader);
			loader = short_loader(&FIFO_DMA_A_cd, loader);
			loader = short_loader(&FIFO_DMA_B_cd, loader);
			loader = short_loader(&Halt_Enter_cd, loader);
			loader = short_loader(&Halt_Leave_cd, loader);
			loader = short_loader(&Halt_Held_CPU_Instr, loader);

			loader = int_loader(&PALRAM_32W_Addr, loader);
			loader = int_loader(&VRAM_32W_Addr, loader);
			loader = int_loader(&Memory_CTRL, loader);
			loader = int_loader(&ROM_Length, loader);

			loader = int_loader(&Last_BIOS_Read, loader);

			loader = int_loader(&WRAM_Waits, loader);
			loader = int_loader(&SRAM_Waits, loader);

			loader = int_loader(&ROM_Waits_0_N, loader);
			loader = int_loader(&ROM_Waits_1_N, loader);
			loader = int_loader(&ROM_Waits_2_N, loader);
			loader = int_loader(&ROM_Waits_0_S, loader);
			loader = int_loader(&ROM_Waits_1_S, loader);
			loader = int_loader(&ROM_Waits_2_S, loader);

			loader = bool_array_loader(DMA_Start_Delay, loader, 4);
			loader = bool_array_loader(DMA_IRQ_Delay, loader, 4);
			
			loader = byte_array_loader(WRAM, loader, 0x40000);
			loader = byte_array_loader(IWRAM, loader, 0x8000);
			loader = byte_array_loader(PALRAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x18000);
			loader = byte_array_loader(OAM, loader, 0x400);

			// Prefetcher
			loader = bool_loader(&pre_Cycle_Glitch, loader);
			loader = bool_loader(&pre_Run, loader);
			loader = bool_loader(&pre_Enable, loader);
			loader = bool_loader(&pre_Force_Non_Seq, loader);
			loader = bool_loader(&pre_Buffer_Was_Full, loader);
			loader = bool_loader(&pre_Boundary_Reached, loader);
			loader = bool_loader(&pre_Following, loader);
			loader = bool_loader(&pre_Inactive, loader);

			loader = int_loader(&pre_Read_Addr, loader);
			loader = int_loader(&pre_Check_Addr, loader);
			loader = int_loader(&pre_Buffer_Cnt, loader);

			loader = int_loader(&pre_Fetch_Cnt, loader);
			loader = int_loader(&pre_Fetch_Wait, loader);

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

			if (RumbleCallback) { RumbleCallback(Rumble_State); }

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