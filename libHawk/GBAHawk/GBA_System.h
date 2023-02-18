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

		void (*MessageCallback)(void);

	# pragma region General System and Prefetch

		uint32_t video_buffer[240* 160] = { };
		uint32_t Frame_Buffer[240 * 160] = { };

		void Frame_Advance();

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
		bool IRQ_Write_Delay, IRQ_Write_Delay_2, IRQ_Write_Delay_3;

		uint8_t Post_Boot, Halt_CTRL;

		uint16_t INT_EN, INT_Flags, INT_Master, Wait_CTRL;

		uint16_t controller_state;

		uint32_t Memory_CTRL, ROM_Length;

		uint32_t Last_BIOS_Read;

		uint32_t WRAM_Waits, SRAM_Waits;

		uint32_t ROM_Waits_0_N, ROM_Waits_1_N, ROM_Waits_2_N, ROM_Waits_0_S, ROM_Waits_1_S, ROM_Waits_2_S;

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

		// Prefetcher
		bool pre_Cycle_Glitch;

		bool pre_Enable, pre_Seq_Access;

		uint32_t pre_Read_Addr, pre_Check_Addr;
		uint32_t pre_Buffer_Cnt;

		uint32_t pre_Fetch_Cnt, pre_Fetch_Wait, pre_Fetch_Cnt_Inc;

		GBA_System()
		{
			System_Reset();
		}

		void System_Reset() 
		{
			delays_to_process = false;

			IRQ_Write_Delay = IRQ_Write_Delay_2 = IRQ_Write_Delay_3 = false;

			controller_state = 0x3FF;

			Memory_CTRL = 0;

			Last_BIOS_Read = 0;

			WRAM_Waits = SRAM_Waits = 0;

			ROM_Waits_0_N = ROM_Waits_1_N = ROM_Waits_2_N = ROM_Waits_0_S = ROM_Waits_1_S = ROM_Waits_2_S = 0;

			INT_EN = INT_Flags = INT_Master = Wait_CTRL = 0;

			Post_Boot = Halt_CTRL = 0;

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

			uint32_t startup_color = 0xFFFFFFFF;

			for (int i = 0; i < 240*160; i++)
			{
				video_buffer[i] = startup_color;
				Frame_Buffer[i] = startup_color;
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

		void do_controller_check()
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
							INT_Flags |= 0x1000;

							key_Delay = true;
							key_Delay_cd = 2;
							delays_to_process = true;
						}
					}
				}
				else
				{
					if ((key_CTRL & ~controller_state & 0x3FF) != 0)
					{
						// doesn't trigger an interrupt if all keys are selected. (see megaman and bass)
						if ((key_CTRL & 0x3FF) != 0x3FF)
						{
							INT_Flags |= 0x1000;

							key_Delay = true;
							key_Delay_cd = 2;
							delays_to_process = true;
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
					INT_Flags |= 0x1000;

					key_Delay = true;
					key_Delay_cd = 2;
					delays_to_process = true;
				}
			}
		}

		void pre_Reg_Write(uint16_t value)
		{
			if (!pre_Enable && ((value & 0x4000) == 0x4000))
			{
				// set read address to current cpu address
				pre_Check_Addr = pre_Read_Addr = cpu_Regs[15];
				pre_Buffer_Cnt = 0;
			}

			pre_Enable = (value & 0x4000) == 0x4000;

			if (!pre_Enable)
			{
				if (pre_Fetch_Cnt == 0)
				{
					pre_Seq_Access = false;
					pre_Read_Addr = 0;
				}
			}
		}

		void pre_Reset()
		{
			VBlank_Rise = false;

			pre_Read_Addr = pre_Check_Addr = 0;
			pre_Buffer_Cnt = 0;

			pre_Fetch_Cnt = pre_Fetch_Wait = 0;
			pre_Fetch_Cnt_Inc = 1;

			pre_Cycle_Glitch = false;

			pre_Enable = pre_Seq_Access = false;
		}

		void On_VBlank()
		{
			// send the image on VBlank
			std::memcpy(Frame_Buffer, video_buffer, sizeof(uint32_t) * 240 * 160);
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

		void Update_INT_EN(uint16_t value)
		{
			if (INT_Master_On)
			{
				if ((value & INT_Flags & 0x3FFF) != 0)
				{
					cpu_Next_IRQ_Input_3 = true;
				}
				else
				{
					cpu_Next_IRQ_Input_3 = false;
				}
			}

			// changes to IRQ that happen due to writes should take place next cycle
			delays_to_process = true;
			IRQ_Write_Delay_3 = true;

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
				}
			}

			// if button is pressed interrupt will immediately fire again
			do_controller_check();

			// check if any remaining interrupts are still valid
			if (INT_Master_On)
			{
				if ((INT_EN & INT_Flags & 0x3FFF) != 0)
				{
					cpu_Next_IRQ_Input_3 = true;
				}
				else
				{
					cpu_Next_IRQ_Input_3 = false;
				}
			}

			// changes to IRQ that happen due to writes should take place next cycle
			delays_to_process = true;
			IRQ_Write_Delay_3 = true;
		}

		void Update_INT_Master(uint16_t value)
		{
			INT_Master_On = (value & 1) == 1;

			if (INT_Master_On)
			{
				if ((INT_EN & INT_Flags & 0x3FFF) != 0)
				{
					cpu_Next_IRQ_Input_3 = true;
				}
			}
			else
			{
				cpu_Next_IRQ_Input_3 = false;
			}

			// changes to IRQ that happen due to writes should take place next cycle
			delays_to_process = true;
			IRQ_Write_Delay_3 = true;

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

			Wait_CTRL = value;
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
					cpu_Halted = true;
					cpu_Just_Halted = true;
					if (TraceCallback) TraceCallback(4);
					cpu_HS_Ofst_ARM0 = 0x8;
					cpu_HS_Ofst_TMB0 = 0x8;

					cpu_HS_Ofst_ARM1 = 0x8;
					cpu_HS_Ofst_TMB1 = 0x8;

					cpu_HS_Ofst_ARM2 = 0x8;
					cpu_HS_Ofst_TMB2 = 0x8;
				}
				else
				{
					/* stop mode */
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
		bool cpu_Take_Branch;
		bool cpu_LS_Is_Load;
		bool cpu_LS_First_Access;
		bool cpu_Internal_Save_Access;
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

		bool cpu_Halted;
		bool stopped;
		bool jammed;

		bool cpu_Trigger_Unhalt;
		bool cpu_Just_Halted;

		// ARM Related Variables
		uint16_t cpu_Exec_ARM;

		// Thumb related Variables
		uint16_t cpu_Exec_TMB;

		uint16_t cpu_Instr_TMB_0, cpu_Instr_TMB_1, cpu_Instr_TMB_2;
		uint16_t cpu_HS_Ofst_TMB0, cpu_HS_Ofst_TMB1, cpu_HS_Ofst_TMB2;
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
		uint32_t cpu_HS_Ofst_ARM0, cpu_HS_Ofst_ARM1, cpu_HS_Ofst_ARM2;
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

			cpu_FlagIset(true);
			cpu_FlagFset(true);
		}

		// NOTTE: system and user have same regs
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
		const static uint16_t cpu_Internal_Halted = 46;
		const static uint16_t cpu_Multiply_Cycles = 47;
		const static uint16_t cpu_Pause_For_DMA = 48;
		
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
		const static uint16_t cpu_ARM_MRS = 27;
		const static uint16_t cpu_ARM_Bx = 28;
		const static uint16_t cpu_ARM_MUL = 29;
		const static uint16_t cpu_ARM_MUL_UL = 30;
		const static uint16_t cpu_ARM_MUL_SL = 31;
		const static uint16_t cpu_ARM_Swap = 32;
		const static uint16_t cpu_ARM_Imm_LS = 70;
		const static uint16_t cpu_ARM_Reg_LS = 80;
		const static uint16_t cpu_ARM_Multi_1 = 90;
		const static uint16_t cpu_ARM_Branch = 100;
		const static uint16_t cpu_ARM_Cond_Check_Only = 110;

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

		void cpu_Reset()
		{
			for (int i = 0; i < 16; i++) { cpu_Regs_To_Access[i] = 0; }

			cpu_Fetch_Cnt = cpu_Fetch_Wait = 0;

			cpu_Multi_List_Ptr = cpu_Multi_List_Size = cpu_Temp_Reg_Ptr = cpu_Base_Reg = cpu_Base_Reg_2 = 0;

			cpu_ALU_Reg_Dest = cpu_ALU_Reg_Src = 0;

			cpu_Mul_Cycles = cpu_Mul_Cycles_Cnt = 0;

			cpu_HS_Ofst_ARM0 = cpu_HS_Ofst_ARM1 = cpu_HS_Ofst_ARM2 = 0;

			cpu_Temp_Reg = cpu_Temp_Addr = cpu_Temp_Data = cpu_Temp_Mode = cpu_Bit_To_Check = 0;

			cpu_Write_Back_Addr = cpu_Addr_Offset = cpu_Last_Bus_Value = cpu_Last_Bus_Value_Old = 0;

			cpu_ALU_Temp_Val = cpu_ALU_Temp_S_Val = cpu_ALU_Shift_Carry = 0;

			cpu_HS_Ofst_TMB0 = cpu_HS_Ofst_TMB1 = cpu_HS_Ofst_TMB2 = 0;

			cpu_Thumb_Mode = cpu_ARM_Cond_Passed = false; // Reset is exitted in ARM mode

			cpu_Instr_Type = cpu_Internal_Reset_1; // 2 internal cycles pass after rest before instruction fetching begins

			cpu_Exception_Type = cpu_Next_Load_Store_Type = 0;

			ResetRegisters();

			CycleCount = 0;

			cpu_Seq_Access = cpu_IRQ_Input = cpu_IRQ_Input_Use = cpu_Is_Paused = cpu_Take_Branch = false;

			cpu_Next_IRQ_Input = cpu_Next_IRQ_Input_2 = cpu_Next_IRQ_Input_3 = false;

			cpu_LS_Is_Load = cpu_LS_First_Access = cpu_Internal_Save_Access = cpu_Invalidate_Pipeline = false;

			cpu_Overwrite_Base_Reg = cpu_Multi_Before = cpu_Multi_Inc = cpu_Multi_S_Bit = cpu_Multi_Swap = false;

			cpu_ALU_S_Bit = cpu_Sign_Extend_Load = cpu_Dest_Is_R15 = false;

			cpu_Swap_Store = cpu_Swap_Lock = cpu_Clear_Pipeline = cpu_Special_Inc = false;

			cpu_FlagI_Old = false;

			stopped = jammed = cpu_Halted = false;

			cpu_Trigger_Unhalt = cpu_Just_Halted = false;
		}

		void cpu_Decode_ARM()
		{
			switch (((cpu_Instr_ARM_2 >> 25) & 7) + cpu_HS_Ofst_ARM2)
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
							cpu_Instr_Type = cpu_Multiply_ARM;
							cpu_Exec_ARM = cpu_ARM_MUL;
							if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
							{
								cpu_Mul_Cycles = 2;
							}
							else
							{
								cpu_Calculate_Mul_Cycles();
							}
							break;

						case 0x1:
							// Undefined Opcode Exception
							cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
							cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
							cpu_Exception_Type = cpu_Undef_Exc;
							break;

						case 0x2:
							// Multiply Long - Unsigned
							cpu_Instr_Type = cpu_Multiply_ARM;
							cpu_Exec_ARM = cpu_ARM_MUL_UL;
							cpu_Calculate_Mul_Cycles_UL();
							break;
						case 0x3:
							// Multiply Long - Signed
							cpu_Instr_Type = cpu_Multiply_ARM;
							cpu_Exec_ARM = cpu_ARM_MUL_SL;
							cpu_Calculate_Mul_Cycles_SL();
							break;

						case 0x4:
						case 0x5:
							// Swap
							cpu_Instr_Type = cpu_Prefetch_Swap_ARM;
							cpu_Next_Load_Store_Type = cpu_Swap_ARM;
							cpu_Exec_ARM = cpu_ARM_Swap;
							cpu_Swap_Store = false;
							break;

						case 0x6:
						case 0x7:
							// Undefined Opcode Exception
							cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
							cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
							cpu_Exception_Type = cpu_Undef_Exc;
							break;
						}
					}
					else
					{
						// halfword or byte transfers
						cpu_Exec_ARM = cpu_ARM_Imm_LS;
						cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

						switch ((cpu_Instr_ARM_2 >> 5) & 0x3)
						{
							// 0 case is not a load store instruction
						case 0x1:
							// Unsigned halfword
							cpu_Next_Load_Store_Type = cpu_Load_Store_Half_ARM;
							cpu_Sign_Extend_Load = false;
							break;

						case 0x2:
							// Signed Byte
							cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
							cpu_Sign_Extend_Load = true;
							break;
						case 0x3:
							// Signed halfword
							cpu_Next_Load_Store_Type = cpu_Load_Store_Half_ARM;
							cpu_Sign_Extend_Load = true;
							break;
						}

						if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
						{
							cpu_Addr_Offset = ((cpu_Instr_ARM_2 >> 4) & 0xF0) | (cpu_Instr_ARM_2 & 0xF);
						}
						else
						{
							cpu_Addr_Offset = cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						}
					}
				}
				else
				{
					// ALU ops
					cpu_ALU_S_Bit = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_ALU_Reg_Dest = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

					cpu_ALU_Reg_Src = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);
					cpu_Temp_Reg = cpu_Regs[cpu_ALU_Reg_Src];

					// slightly different code path for R15 as destination, since it's closer to a branch
					cpu_Dest_Is_R15 = (cpu_ALU_Reg_Dest == 15);

					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
					case 0x0: cpu_Exec_ARM = cpu_ARM_AND; cpu_Clear_Pipeline = true; break;
					case 0x1: cpu_Exec_ARM = cpu_ARM_EOR; cpu_Clear_Pipeline = true; break;
					case 0x2: cpu_Exec_ARM = cpu_ARM_SUB; cpu_Clear_Pipeline = true; break;
					case 0x3: cpu_Exec_ARM = cpu_ARM_RSB; cpu_Clear_Pipeline = true; break;
					case 0x4: cpu_Exec_ARM = cpu_ARM_ADD; cpu_Clear_Pipeline = true; break;
					case 0x5: cpu_Exec_ARM = cpu_ARM_ADC; cpu_Clear_Pipeline = true; break;
					case 0x6: cpu_Exec_ARM = cpu_ARM_SBC; cpu_Clear_Pipeline = true; break;
					case 0x7: cpu_Exec_ARM = cpu_ARM_RSC; cpu_Clear_Pipeline = true; break;
					case 0x8: cpu_Exec_ARM = cpu_ARM_TST; cpu_Clear_Pipeline = false; break;
					case 0x9: cpu_Exec_ARM = cpu_ARM_TEQ; cpu_Clear_Pipeline = false; break;
					case 0xA: cpu_Exec_ARM = cpu_ARM_CMP; cpu_Clear_Pipeline = false; break;
					case 0xB: cpu_Exec_ARM = cpu_ARM_CMN; cpu_Clear_Pipeline = false; break;
					case 0xC: cpu_Exec_ARM = cpu_ARM_ORR; cpu_Clear_Pipeline = true; break;
					case 0xD: cpu_Exec_ARM = cpu_ARM_MOV; cpu_Clear_Pipeline = true; break;
					case 0xE: cpu_Exec_ARM = cpu_ARM_BIC; cpu_Clear_Pipeline = true; break;
					case 0xF: cpu_Exec_ARM = cpu_ARM_MVN; cpu_Clear_Pipeline = true; break;
					}

					// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
					cpu_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

					bool is_RRX = false;

					cpu_ALU_Long_Result = cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					if ((cpu_Instr_ARM_2 & 0x10) != 0x0)
					{
						// if the pc is the shifted value or operand, and its a register shift, it is the incremented value that is used
						if ((cpu_Instr_ARM_2 & 0xF) == 15)
						{
							cpu_ALU_Long_Result += 4; cpu_ALU_Long_Result &= cpu_Cast_Int;
						}

						if (cpu_ALU_Reg_Src == 15)
						{
							cpu_Temp_Reg += 4;
						}

						// register shifts take an extra cycle
						if (cpu_Instr_Type == cpu_Internal_And_Prefetch_ARM) { cpu_Instr_Type = cpu_Internal_And_Prefetch_2_ARM; }
						if (cpu_Instr_Type == cpu_Internal_And_Branch_2_ARM) { cpu_Instr_Type = cpu_Internal_And_Branch_3_ARM; }
					}

					cpu_ALU_Shift_Carry = (uint32_t)(cpu_FlagCget() ? 1 : 0);

					switch ((cpu_Instr_ARM_2 >> 5) & 3)
					{
					case 0:         // LSL
						// calculate immedaite value
						if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
						{
							// immediate shift
							cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);
						}
						else
						{
							// register shift
							cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
						}

						cpu_ALU_Long_Result = cpu_ALU_Long_Result << cpu_Shift_Imm;

						if (cpu_Shift_Imm != 0)
						{
							cpu_ALU_Shift_Carry = (uint32_t)((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare ? 1 : 0);
						}
						break;

					case 1:         // LSR
						// calculate immedaite value
						if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
						{
							// immediate shift
							cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

							if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
						}
						else
						{
							// register shift
							cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
						}

						if (cpu_Shift_Imm != 0)
						{
							cpu_ALU_Shift_Carry = (uint32_t)((cpu_ALU_Long_Result >> (cpu_Shift_Imm - 1)) & 1);
							cpu_ALU_Long_Result = cpu_ALU_Long_Result >> cpu_Shift_Imm;
						}
						break;

					case 2:         // ASR
						// calculate immedaite value
						if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
						{
							// immediate shift
							cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

							if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }
						}
						else
						{
							// register shift
							cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
						}

						cpu_ALU_Temp_S_Val = (uint32_t)(cpu_ALU_Long_Result & cpu_Neg_Compare);

						for (int i = 1; i <= cpu_Shift_Imm; i++)
						{
							cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
							cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
							cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
						}
						break;

					case 3:         // RRX
						// calculate immedaite value
						if ((cpu_Instr_ARM_2 & 0x10) == 0x0)
						{
							// immediate shift
							cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

							if (cpu_Shift_Imm == 0) { is_RRX = true; }
						}
						else
						{
							// register shift
							cpu_Shift_Imm = (uint32_t)(cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF] & 0xFF);
						}

						if (is_RRX)
						{
							cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
							cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
							cpu_ALU_Long_Result |= cpu_FlagCget() ? 0x80000000 : 0;
						}
						else
						{
							for (int i = 1; i <= cpu_Shift_Imm; i++)
							{
								cpu_ALU_Shift_Carry = (uint32_t)(cpu_ALU_Long_Result & 1);
								cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
								cpu_ALU_Long_Result |= (cpu_ALU_Shift_Carry << 31);
							}
						}
						break;
					}

					cpu_ALU_Temp_Val = (uint32_t)cpu_ALU_Long_Result;

					// overwrite certain instructions
					if (!cpu_ALU_S_Bit)
					{
						switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
						{
						case 0x8:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_ARM;
							cpu_Exec_ARM = cpu_ARM_MRS;
							break;
						case 0x9:
							if ((cpu_Instr_ARM_2 & 0XFFFF0) == 0xFFF10)
							{
								// Branch and exchange
								cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_ARM;
								cpu_Exec_ARM = cpu_ARM_Bx;
							}
							else
							{
								cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
								cpu_Exec_ARM = cpu_ARM_MSR;
							}
							break;
						case 0xA:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_ARM;
							cpu_Exec_ARM = cpu_ARM_MRS;
							break;
						case 0xB:
							cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
							cpu_Exec_ARM = cpu_ARM_MSR;
							break;
						}
					}
				}
				break;

			case 1:
				// ALU ops
				cpu_ALU_S_Bit = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

				cpu_ALU_Reg_Dest = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

				cpu_ALU_Reg_Src = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);
				cpu_Temp_Reg = cpu_Regs[cpu_ALU_Reg_Src];

				// slightly different code path for R15 as destination, since it's closer to a branch
				cpu_Dest_Is_R15 = (cpu_ALU_Reg_Dest == 15);

				switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
				{
				case 0x0: cpu_Exec_ARM = cpu_ARM_AND; cpu_Clear_Pipeline = true; break;
				case 0x1: cpu_Exec_ARM = cpu_ARM_EOR; cpu_Clear_Pipeline = true; break;
				case 0x2: cpu_Exec_ARM = cpu_ARM_SUB; cpu_Clear_Pipeline = true; break;
				case 0x3: cpu_Exec_ARM = cpu_ARM_RSB; cpu_Clear_Pipeline = true; break;
				case 0x4: cpu_Exec_ARM = cpu_ARM_ADD; cpu_Clear_Pipeline = true; break;
				case 0x5: cpu_Exec_ARM = cpu_ARM_ADC; cpu_Clear_Pipeline = true; break;
				case 0x6: cpu_Exec_ARM = cpu_ARM_SBC; cpu_Clear_Pipeline = true; break;
				case 0x7: cpu_Exec_ARM = cpu_ARM_RSC; cpu_Clear_Pipeline = true; break;
				case 0x8: cpu_Exec_ARM = cpu_ARM_TST; cpu_Clear_Pipeline = false; break;
				case 0x9: cpu_Exec_ARM = cpu_ARM_TEQ; cpu_Clear_Pipeline = false; break;
				case 0xA: cpu_Exec_ARM = cpu_ARM_CMP; cpu_Clear_Pipeline = false; break;
				case 0xB: cpu_Exec_ARM = cpu_ARM_CMN; cpu_Clear_Pipeline = false; break;
				case 0xC: cpu_Exec_ARM = cpu_ARM_ORR; cpu_Clear_Pipeline = true; break;
				case 0xD: cpu_Exec_ARM = cpu_ARM_MOV; cpu_Clear_Pipeline = true; break;
				case 0xE: cpu_Exec_ARM = cpu_ARM_BIC; cpu_Clear_Pipeline = true; break;
				case 0xF: cpu_Exec_ARM = cpu_ARM_MVN; cpu_Clear_Pipeline = true; break;
				}

				// even TST / TEQ / CMP / CMN take the branch path, but don't reset the pipeline
				cpu_Instr_Type = cpu_Dest_Is_R15 ? cpu_Internal_And_Branch_2_ARM : cpu_Internal_And_Prefetch_ARM;

				// calculate immedaite value
				cpu_ALU_Temp_Val = cpu_Instr_ARM_2 & 0xFF;

				cpu_ALU_Shift_Carry = (uint32_t)(cpu_FlagCget() ? 1 : 0);

				// Note: the shift val is multiplied by 2 (so only shift by 7 below)
				for (int i = 1; i <= ((cpu_Instr_ARM_2 >> 7) & 0x1E); i++)
				{
					cpu_ALU_Shift_Carry = cpu_ALU_Temp_Val & 1;
					cpu_ALU_Temp_Val = (cpu_ALU_Temp_Val >> 1) | (cpu_ALU_Shift_Carry << 31);
				}

				// overwrite certain instructions
				if (!cpu_ALU_S_Bit)
				{
					switch ((cpu_Instr_ARM_2 >> 21) & 0xF)
					{
					case 0x8:
						// Undefined Opcode Exception
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
						cpu_Exception_Type = cpu_Undef_Exc;
						break;
					case 0x9:
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR;
						break;
					case 0xA:
						// Undefined Opcode Exception
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
						cpu_Exception_Type = cpu_Undef_Exc;
						break;
					case 0xB:
						cpu_Instr_Type = cpu_Internal_And_Prefetch_3_ARM;
						cpu_Exec_ARM = cpu_ARM_MSR;
						break;
					}
				}
				break;

			case 2:
				// load / store immediate offset
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

				if ((cpu_Instr_ARM_2 & 0x400000) == 0x400000)
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
				}
				else
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_ARM;
				}

				cpu_Exec_ARM = cpu_ARM_Imm_LS;
				cpu_Addr_Offset = cpu_Instr_ARM_2 & 0xFFF;
				break;

			case 3:
				if ((cpu_Instr_ARM_2 & 0x10) == 0)
				{
					// load / store register offset
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;

					if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
					{
						cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_ARM;
					}
					else
					{
						cpu_Next_Load_Store_Type = cpu_Load_Store_Word_ARM;
					}

					cpu_Exec_ARM = cpu_ARM_Reg_LS;
				}
				else
				{
					// Undefined Opcode Exception
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				break;

			case 4:
				// block transfer
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_ARM;
				cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_ARM;
				cpu_Exec_ARM = cpu_ARM_Multi_1;
				break;

			case 5:
				// branch
				cpu_Instr_Type = cpu_Internal_And_Branch_1_ARM;
				cpu_Exec_ARM = cpu_ARM_Branch;
				break;

			case 6:
				// Coprocessor Instruction (treat as Undefined Opcode Exception)
				cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
				cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
				cpu_Exception_Type = cpu_Undef_Exc;
				break;

			case 7:
				if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
				{
					// software interrupt
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_SWI_Exc;
				}
				else
				{
					// Coprocessor Instruction (treat as Undefined Opcode Exception)
					cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
					cpu_Exec_ARM = cpu_ARM_Cond_Check_Only;
					cpu_Exception_Type = cpu_Undef_Exc;
				}
				break;

			default:
				cpu_Instr_Type = cpu_Internal_Halted;
				break;
			}

			if ((TraceCallback) && (cpu_HS_Ofst_ARM2 == 0)) TraceCallback(0);

			cpu_HS_Ofst_ARM2 = cpu_HS_Ofst_ARM1;
			cpu_HS_Ofst_ARM1 = cpu_HS_Ofst_ARM0;
		}

		void cpu_Decode_TMB()
		{
			switch (((cpu_Instr_TMB_2 >> 13) & 7) + cpu_HS_Ofst_TMB2)
			{
			case 0:
				// shift / add / sub
				if ((cpu_Instr_TMB_2 & 0x1800) == 0x1800)
				{
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
					cpu_Exec_TMB = cpu_Thumb_Add_Sub_Reg;
				}
				else
				{
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
					cpu_Exec_TMB = cpu_Thumb_Shift;
				}
				break;

			case 1:
				// data ops (immedaite)
				cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
				cpu_Exec_TMB = cpu_Thumb_ALU_Imm;
				break;

			case 2:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0x0)
				{
					if ((cpu_Instr_TMB_2 & 0x800) == 0x0)
					{
						if ((cpu_Instr_TMB_2 & 0x400) == 0x0)
						{
							// ALU Ops
							// shifts in this path take an extra cycle, as the shift length is register defined
							cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

							cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

							switch ((cpu_Instr_TMB_2 >> 6) & 0xF)
							{
							case 0x0: cpu_Exec_TMB = cpu_Thumb_AND; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x1: cpu_Exec_TMB = cpu_Thumb_EOR; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x2: cpu_Exec_TMB = cpu_Thumb_LSL; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x3: cpu_Exec_TMB = cpu_Thumb_LSR; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x4: cpu_Exec_TMB = cpu_Thumb_ASR; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x5: cpu_Exec_TMB = cpu_Thumb_ADC; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x6: cpu_Exec_TMB = cpu_Thumb_SBC; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x7: cpu_Exec_TMB = cpu_Thumb_ROR; cpu_Instr_Type = cpu_Internal_And_Prefetch_2_TMB; break;
							case 0x8: cpu_Exec_TMB = cpu_Thumb_TST; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0x9: cpu_Exec_TMB = cpu_Thumb_NEG; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xA: cpu_Exec_TMB = cpu_Thumb_CMP; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xB: cpu_Exec_TMB = cpu_Thumb_CMN; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xC: cpu_Exec_TMB = cpu_Thumb_ORR; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xD: cpu_Exec_TMB = cpu_Thumb_MUL; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xE: cpu_Exec_TMB = cpu_Thumb_BIC; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							case 0xF: cpu_Exec_TMB = cpu_Thumb_MVN; cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB; break;
							}

							// special case for multiply
							if (((cpu_Instr_TMB_2 >> 6) & 0xF) == 0xD)
							{
								cpu_Instr_Type = cpu_Multiply_TMB;
								cpu_Calculate_Mul_Cycles();
							}
						}
						else
						{
							// High Regs / Branch and exchange
							switch ((cpu_Instr_TMB_2 >> 8) & 3)
							{
							case 0:
								cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
								cpu_Exec_TMB = cpu_Thumb_High_Add;
								break;

							case 1:
								cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
								cpu_Exec_TMB = cpu_Thumb_High_Cmp;
								break;

							case 2:
								if ((cpu_Instr_TMB_2 & 0xC0) == 0x0)
								{
									// copy only available in ARMv6 and above
									throw new std::invalid_argument("undefined instruction (CPY) 01000110 " + to_string(cpu_Regs[15]));
								}
								if (((cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8)) == 0xF)
								{
									cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
								}
								else
								{
									cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
								}

								cpu_Exec_TMB = cpu_Thumb_High_MOV;
								break;

							case 3:
								if ((cpu_Instr_TMB_2 & 0x80) == 0)
								{
									cpu_Instr_Type = cpu_Prefetch_And_Branch_Ex_TMB;
									cpu_Exec_TMB = cpu_Thumb_High_Bx;
								}
								else
								{
									// This version only available in ARM V5 and above
									throw new std::invalid_argument("undefined instruction (BLX) 010001111 " + to_string(cpu_Regs[15]));
								}
								break;
							}
						}
					}
					else
					{
						// PC relative load
						cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
						cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;
						cpu_Exec_TMB = cpu_Thumb_PC_Rel_LS;
					}
				}
				else
				{
					// Load / store Relative offset
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;

					switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
					{
					case 0: cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB; break;
					case 1: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; break;
					case 2: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; break;
					case 3: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; break;
					case 4: cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB; break;
					case 5: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; break;
					case 6: cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB; break;
					case 7: cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB; break;
					}

					cpu_Exec_TMB = cpu_Thumb_Rel_LS;
				}
				break;

			case 3:
				// Load / store Immediate offset
				cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;

				if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Byte_TMB;
				}
				else
				{
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;
				}

				cpu_Exec_TMB = cpu_Thumb_Imm_LS;
				break;

			case 4:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Load / store half word
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
					cpu_Next_Load_Store_Type = cpu_Load_Store_Half_TMB;
					cpu_Exec_TMB = cpu_Thumb_Half_LS;
				}
				else
				{
					// SP relative load store
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
					cpu_Next_Load_Store_Type = cpu_Load_Store_Word_TMB;
					cpu_Exec_TMB = cpu_Thumb_SP_REL_LS;
				}
				break;

			case 5:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Load Address
					cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
					cpu_Exec_TMB = cpu_Thumb_Add_SP_PC;
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0xF00) == 0x0)
					{
						// Add offset to stack
						cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
						cpu_Exec_TMB = cpu_Thumb_Add_Sub_Stack;
					}
					else
					{
						if ((cpu_Instr_TMB_2 & 0x600) == 0x400)
						{
							// Push / Pop
							cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
							cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_TMB;
							cpu_Exec_TMB = cpu_Thumb_Push_Pop;
						}
						else
						{
							// Undefined Opcode Exception
							cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
							cpu_Exception_Type = cpu_Undef_Exc;
						}
					}
				}
				break;

			case 6:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					// Multiple Load/Store
					cpu_Instr_Type = cpu_Prefetch_And_Load_Store_TMB;
					cpu_Next_Load_Store_Type = cpu_Multi_Load_Store_TMB;
					cpu_Exec_TMB = cpu_Thumb_Multi_1;
				}
				else
				{
					if ((cpu_Instr_TMB_2 & 0xF00) == 0xF00)
					{
						// Software Interrupt
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exception_Type = cpu_SWI_Exc;
					}
					else if ((cpu_Instr_TMB_2 & 0xE00) == 0xE00)
					{
						// Undefined instruction
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exception_Type = cpu_Undef_Exc;
					}
					else
					{
						// Conditional Branch
						cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
						cpu_Exec_TMB = cpu_Thumb_Branch_Cond;
					}
				}

				break;

			case 7:
				if ((cpu_Instr_TMB_2 & 0x1000) == 0)
				{
					if ((cpu_Instr_TMB_2 & 0x800) == 0)
					{
						// Unconditional branch
						cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
						cpu_Exec_TMB = cpu_Thumb_Branch;
					}
					else
					{
						// Undefined Opcode Exception
						cpu_Instr_Type = cpu_Prefetch_And_SWI_Undef;
						cpu_Exception_Type = cpu_Undef_Exc;
					}
				}
				else
				{
					// Branch with link
					if ((cpu_Instr_TMB_2 & 0x800) == 0)
					{
						// A standard data operation assigning the upper part of the branch
						cpu_Instr_Type = cpu_Internal_And_Prefetch_TMB;
						cpu_Exec_TMB = cpu_Thumb_Branch_Link_1;
					}
					else
					{
						// Actual branch operation (can it occur without the first one?)
						cpu_Instr_Type = cpu_Internal_And_Branch_1_TMB;
						cpu_Exec_TMB = cpu_Thumb_Branch_Link_2;
					}
				}
				break;

			default:
				cpu_Instr_Type = cpu_Internal_Halted;
				break;
			}

			if ((TraceCallback) && (cpu_HS_Ofst_TMB2 == 0)) TraceCallback(0);

			cpu_HS_Ofst_TMB2 = cpu_HS_Ofst_TMB1;
			cpu_HS_Ofst_TMB1 = cpu_HS_Ofst_TMB0;
		}

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

			if ((temp_calc & 0x80000000) == 0)
			{
				// seems to be based on the number of non-zero upper bits if sign bit is zero
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
			}
			else
			{
				// seems to be based on the number of non-zero lower bits if sign bit is not zero
				if ((temp_calc & 0x00FFFFFF) == 0)
				{
					cpu_Mul_Cycles = 4;
				}
				else if ((temp_calc & 0x0000FFFF) == 0)
				{
					cpu_Mul_Cycles = 3;
				}
				else if ((temp_calc & 0x000000FF) == 0)
				{
					cpu_Mul_Cycles = 2;
				}
				else
				{
					cpu_Mul_Cycles = 2;
				}
			}

			if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
			{
				cpu_Mul_Cycles += 1;
			}
		}

		void cpu_Execute_Internal_Only_ARM()
		{
			// Do Tracer Stuff here
			int ofst = 0;

			// local variables not stated (evaluated each time)
			bool Use_Reg_15 = false;
			uint32_t byte_mask = 0;
			uint32_t total_mask = 0;
			int64_t s1 = 0;
			int64_t s2 = 0;
			int64_t sf1 = 0;
			int64_t sf2 = 0;

			// Condition code check
			if (cpu_ARM_Condition_Check())
			{
				cpu_ARM_Cond_Passed = true;

				// Do Tracer stuff here
				switch (cpu_Exec_ARM)
				{
				case cpu_ARM_AND:
					cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_EOR:
					cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_SUB:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_RSB:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= cpu_Temp_Reg;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_ADD:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_ADC:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result += (uint64_t)(cpu_FlagCget() ? 1 : 0);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_SBC:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_RSC:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;
					cpu_ALU_Long_Result -= cpu_Temp_Reg;
					cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_ALU_Temp_Val, cpu_Temp_Reg, (uint32_t)cpu_ALU_Long_Result));
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_TST:
					cpu_ALU_Long_Result = cpu_Temp_Reg & cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					break;

				case cpu_ARM_TEQ:
					cpu_ALU_Long_Result = cpu_Temp_Reg ^ cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}
					break;

				case cpu_ARM_CMP:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}
					break;

				case cpu_ARM_CMN:
					cpu_ALU_Long_Result = cpu_Temp_Reg;
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

						cpu_ALU_Long_Result &= cpu_Cast_Int;

						cpu_FlagZset(cpu_ALU_Long_Result == 0);
						cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Temp_Reg, cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					}
					break;

				case cpu_ARM_ORR:
					cpu_ALU_Long_Result = cpu_Temp_Reg | cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MOV:
					cpu_ALU_Long_Result = cpu_ALU_Temp_Val;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_BIC:
					cpu_ALU_Long_Result = cpu_Temp_Reg & (~cpu_ALU_Temp_Val);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MVN:
					cpu_ALU_Long_Result = ~cpu_ALU_Temp_Val;

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if (cpu_ALU_S_Bit && !cpu_Dest_Is_R15)
					{
						cpu_FlagCset(cpu_ALU_Shift_Carry == 1);
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MSR:
					if ((cpu_Instr_ARM_2 & 0x10000) == 0x10000) { byte_mask |= 0x000000FF; }
					if ((cpu_Instr_ARM_2 & 0x20000) == 0x20000) { byte_mask |= 0x0000FF00; }
					if ((cpu_Instr_ARM_2 & 0x40000) == 0x40000) { byte_mask |= 0x00FF0000; }
					if ((cpu_Instr_ARM_2 & 0x80000) == 0x80000) { byte_mask |= 0xFF000000; }

					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						// user (unpriviliged)
						if ((cpu_Regs[16] & 0x1F) == 0x10)
						{
							total_mask = byte_mask & cpu_User_Mask;
						}
						else
						{
							if ((cpu_ALU_Temp_Val & cpu_State_Mask) != 0)
							{
								// architecturally unpredictable, but on hardwarae has no ill effects (ex. feline.gba transparency when seen by lab rat.)
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
							else
							{
								total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask);
							}
						}

						cpu_Swap_Regs(((cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask)) & 0x1F, false, false);
						cpu_Regs[16] = (cpu_Regs[16] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);

						cpu_Thumb_Mode = cpu_FlagTget();
					}
					else
					{
						// user (unpriviliged)
						if ((cpu_Regs[16] & 0x1F) == 0x10)
						{
							// unpredictable
						}
						else
						{
							total_mask = byte_mask & (cpu_User_Mask | cpu_Priv_Mask | cpu_State_Mask);
							cpu_Regs[17] = (cpu_Regs[17] & ~total_mask) | (cpu_ALU_Temp_Val & total_mask);
						}
					}
					break;

				case cpu_ARM_MRS:
					if ((cpu_Instr_ARM_2 & 0x400000) == 0x0)
					{
						cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[16];
					}
					else
					{
						cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = cpu_Regs[17];
					}
					break;

				case cpu_ARM_Bx:
					// Branch and exchange mode (possibly)
					cpu_Base_Reg = (uint32_t)(cpu_Instr_ARM_2 & 0xF);

					cpu_Temp_Reg = (cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFE);
					break;

				case cpu_ARM_MUL:
					cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
					cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						cpu_ALU_Long_Result += cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
					}

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case cpu_ARM_MUL_UL:
					cpu_ALU_Long_Result = cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
					cpu_ALU_Long_Result *= cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{

						uint64_t a1 = cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
						uint64_t a2 = cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF];
						a2 = a2 << 32;
						cpu_ALU_Long_Result += (a1 + a2);
					}

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);

						// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1
						// then it does not change (probably multiplier takes a short cut for these cases)
						// and changes differently if it is -1
						sf1 = (int32_t)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
						sf2 = (int32_t)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];
						if (sf2 == -1)
						{
							// maybe its a carry from lower 16 bits)???
							if ((sf1 & 0xFFFF) >= 2) { cpu_FlagCset(true); }
							else { cpu_FlagCset(false); }
						}
						else if ((sf2 != 0) && (sf2 != 1))
						{
							cpu_FlagCset(((sf1 & 0x80000000) == 0x80000000));
						}
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)(cpu_ALU_Long_Result >> 32);
					break;

				case cpu_ARM_MUL_SL:
					s1 = (int32_t)cpu_Regs[cpu_Instr_ARM_2 & 0xF];
					s2 = (int32_t)cpu_Regs[(cpu_Instr_ARM_2 >> 8) & 0xF];

					cpu_ALU_Signed_Long_Result = s1 * s2;

					cpu_ALU_Long_Result = (uint64_t)cpu_ALU_Signed_Long_Result;

					if ((cpu_Instr_ARM_2 & 0x00200000) == 0x00200000)
					{
						uint64_t a1 = cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF];
						uint64_t a2 = cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF];
						a2 = a2 << 32;
						cpu_ALU_Long_Result += (a1 + a2);
					}

					if ((cpu_Instr_ARM_2 & 0x00100000) == 0x00100000)
					{
						cpu_FlagNset((cpu_ALU_Long_Result & cpu_Long_Neg_Compare) == cpu_Long_Neg_Compare);
						cpu_FlagZset(cpu_ALU_Long_Result == 0);

						// it appears the carry flag depends on the signs of the operands, but if the first operand is 0,1,-1
						// then it does not change (probably multiplier takes a short cut for these cases)
						if ((s2 != 0) && (s2 != 1) && (s2 != -1))
						{
							cpu_FlagCset(!((s1 & 0x80000000) == (s2 & 0x80000000)));
						}
					}

					cpu_Regs[(cpu_Instr_ARM_2 >> 12) & 0xF] = (uint32_t)cpu_ALU_Long_Result;
					cpu_Regs[(cpu_Instr_ARM_2 >> 16) & 0xF] = (uint32_t)(cpu_ALU_Long_Result >> 32);
					break;

				case cpu_ARM_Swap:
					cpu_Base_Reg_2 = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);
					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);
					cpu_Temp_Reg_Ptr = (uint32_t)(cpu_Instr_ARM_2 & 0xF);
					break;

				case cpu_ARM_Imm_LS:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg_Ptr = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// increment first
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Addr_Offset);
						}
						else
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] - cpu_Addr_Offset);
						}
					}
					else
					{
						// increment last
						cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];
					}

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;

						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr + cpu_Addr_Offset);
						}
						else
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - cpu_Addr_Offset);
						}
					}
					else if ((cpu_Instr_ARM_2 & 0x200000) == 0x200000)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}

					break;

				case cpu_ARM_Reg_LS:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Reg_Ptr = (uint32_t)((cpu_Instr_ARM_2 >> 12) & 0xF);

					cpu_Temp_Data = cpu_Regs[cpu_Instr_ARM_2 & 0xF];

					cpu_Shift_Imm = (uint32_t)((cpu_Instr_ARM_2 >> 7) & 0x1F);

					switch ((cpu_Instr_ARM_2 >> 5) & 3)
					{
					case 0:         // LSL
						cpu_Temp_Data = cpu_Temp_Data << cpu_Shift_Imm;
						break;

					case 1:			// LSR
						if (cpu_Shift_Imm == 0) { cpu_Temp_Data = 0; }
						else { cpu_Temp_Data = cpu_Temp_Data >> cpu_Shift_Imm; }
						break;

					case 2:         // ASR
						if (cpu_Shift_Imm == 0) { cpu_Shift_Imm = 32; }

						cpu_ALU_Temp_S_Val = cpu_Temp_Data & cpu_Neg_Compare;

						for (int i = 1; i <= cpu_Shift_Imm; i++)
						{
							cpu_Temp_Data = (cpu_Temp_Data >> 1);
							cpu_Temp_Data |= cpu_ALU_Temp_S_Val;
						}
						break;

					case 3:         // RRX
						if (cpu_Shift_Imm == 0)
						{
							cpu_Temp_Data = (cpu_Temp_Data >> 1);
							cpu_Temp_Data |= cpu_FlagCget() ? 0x80000000 : 0;
						}
						else
						{
							for (int i = 1; i <= cpu_Shift_Imm; i++)
							{
								cpu_ALU_Temp_S_Val = (uint32_t)(cpu_Temp_Data & 1);
								cpu_Temp_Data = (cpu_Temp_Data >> 1);
								cpu_Temp_Data |= (cpu_ALU_Temp_S_Val << 31);
							}
						}
						break;
					}

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						// increment first
						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Temp_Data);
						}
						else
						{
							cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] - cpu_Temp_Data);
						}
					}
					else
					{
						// increment last
						cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];
					}

					if ((cpu_Instr_ARM_2 & 0x1000000) == 0)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;

						if ((cpu_Instr_ARM_2 & 0x800000) == 0x800000)
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr + cpu_Temp_Data);
						}
						else
						{
							cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - cpu_Temp_Data);
						}
					}
					else if ((cpu_Instr_ARM_2 & 0x200000) == 0x200000)
					{
						// write back
						cpu_Overwrite_Base_Reg = true;
						cpu_Write_Back_Addr = cpu_Temp_Addr;
					}

					// don't write back a loaded register
					if (cpu_Overwrite_Base_Reg && cpu_LS_Is_Load)
					{
						if (cpu_Base_Reg == cpu_Temp_Reg_Ptr) { cpu_Overwrite_Base_Reg = false; }
					}
					break;

				case cpu_ARM_Multi_1:
					cpu_LS_Is_Load = (cpu_Instr_ARM_2 & 0x100000) == 0x100000;

					cpu_Base_Reg = (uint32_t)((cpu_Instr_ARM_2 >> 16) & 0xF);

					cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

					cpu_LS_First_Access = true;

					cpu_Overwrite_Base_Reg = (cpu_Instr_ARM_2 & 0x200000) == 0x200000;

					cpu_Multi_Before = (cpu_Instr_ARM_2 & 0x1000000) == 0x1000000;

					cpu_Multi_Inc = (cpu_Instr_ARM_2 & 0x800000) == 0x800000;

					// build list of registers to access
					cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

					// need some special logic here, if the S bit is set, we swap out registers to user mode for the accesses
					// then swap back afterwards, but only if reg15 is not accessed in a load
					cpu_Multi_S_Bit = (cpu_Instr_ARM_2 & 0x400000) == 0x400000;
					cpu_Multi_Swap = false;

					for (int i = 0; i < 16; i++)
					{
						if (((cpu_Instr_ARM_2 >> i) & 1) == 1)
						{
							cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
							if ((i == 15) && cpu_LS_Is_Load) { Use_Reg_15 = true; }

							cpu_Multi_List_Size += 1;

							// The documentation gives this case as unpredictable for now copy Thumb logic
							if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
						}
					}

					// No registers selected loads / stores R15 instead
					if (cpu_Multi_List_Size == 0)
					{
						cpu_Multi_List_Size = 1;
						cpu_Regs_To_Access[0] = 15;
						cpu_Special_Inc = true;
						Use_Reg_15 = true; // ?
					}

					// when decrementing, start address is at the bottom, (similar to a POP instruction)
					if (!cpu_Multi_Inc)
					{
						cpu_Temp_Addr -= (uint32_t)((cpu_Multi_List_Size - 1) * 4);

						// in either case, write back is the same
						cpu_Write_Back_Addr = (uint32_t)(cpu_Temp_Addr - 4);

						if (cpu_Multi_Before) { cpu_Temp_Addr -= 4; }

						if (cpu_Special_Inc)
						{
							cpu_Write_Back_Addr -= 0x3C;
							cpu_Temp_Addr -= 0x3C;
							cpu_Special_Inc = false;
						}
					}
					else
					{
						if (cpu_Multi_Before) { cpu_Temp_Addr += 4; }
					}

					// swap out registers for user mode ones
					if (cpu_Multi_S_Bit && !Use_Reg_15)
					{
						cpu_Multi_Swap = true;
						cpu_Temp_Mode = cpu_Regs[16] & 0x1F;

						cpu_Swap_Regs(0x10, false, false);
					}
					break;

				case cpu_ARM_Branch:
					ofst = (uint32_t)((cpu_Instr_ARM_2 & 0xFFFFFF) << 2);

					// offset is signed
					if ((ofst & 0x2000000) == 0x2000000) { ofst = (uint32_t)(ofst | 0xFC000000); }

					cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

					// Link if link bit set
					if ((cpu_Instr_ARM_2 & 0x1000000) == 0x1000000)
					{
						cpu_Regs[14] = (uint32_t)(cpu_Regs[15] - 4);
					}

					cpu_Take_Branch = true;
					break;
				}
			}
			else
			{
				cpu_ARM_Cond_Passed = false;
			}
		}

		void cpu_Execute_Internal_Only_TMB()
		{
			int ofst = 0;

			// Do Tracer stuff here
			switch (cpu_Exec_TMB)
			{
			case cpu_Thumb_Shift:
				cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

				cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

				cpu_ALU_Temp_Val = (uint32_t)((cpu_Instr_TMB_2 >> 6) & 0x1F);

				switch ((cpu_Instr_TMB_2 >> 11) & 0x3)
				{
				case 0:
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
					cpu_ALU_Long_Result = cpu_ALU_Long_Result << (uint32_t)cpu_ALU_Temp_Val;

					if (cpu_ALU_Temp_Val != 0) { cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare); }
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case 1:
					if (cpu_ALU_Temp_Val != 0)
					{
						cpu_FlagCset(((cpu_Regs[cpu_ALU_Reg_Src] >> (uint32_t)(cpu_ALU_Temp_Val - 1)) & 1) == 1);
						cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src] >> (uint32_t)cpu_ALU_Temp_Val;
					}
					else
					{
						cpu_FlagCset((cpu_Regs[cpu_ALU_Reg_Src] & cpu_Neg_Compare) == cpu_Neg_Compare);
						cpu_ALU_Long_Result = 0;
					}

					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case 2:
					cpu_ALU_Temp_S_Val = cpu_Regs[cpu_ALU_Reg_Src] & cpu_Neg_Compare;

					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];

					if (cpu_ALU_Temp_Val == 0) { cpu_ALU_Temp_Val = 32; }

					for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
					{
						cpu_FlagCset((cpu_ALU_Long_Result & 1) == 1);
						cpu_ALU_Long_Result = cpu_ALU_Long_Result >> 1;
						cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
					}

					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;
				}
				break;

			case cpu_Thumb_Add_Sub_Reg:
				cpu_ALU_Reg_Dest = cpu_Instr_TMB_2 & 0x7;

				cpu_ALU_Reg_Src = (cpu_Instr_TMB_2 >> 3) & 0x7;

				if ((cpu_Instr_TMB_2 & 0x400) == 0x400)
				{
					cpu_ALU_Temp_Val = (uint32_t)((cpu_Instr_TMB_2 >> 6) & 0x7);
				}
				else
				{
					cpu_ALU_Temp_Val = cpu_Regs[(cpu_Instr_TMB_2 >> 6) & 0x7];
				}

				if ((cpu_Instr_TMB_2 & 0x200) == 0x200)
				{
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Src], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				}
				else
				{
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Src];
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Src], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				}
				break;

			case cpu_Thumb_AND:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_EOR:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] ^ cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_LSL:
				cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
				cpu_ALU_Long_Result = cpu_ALU_Long_Result << (uint32_t)cpu_ALU_Temp_Val;

				if (cpu_ALU_Temp_Val != 0) { cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare); }
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_LSR:
				cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

				// NOTE: This is necessary due to C# only using lower 5 bits of shift count on integer sized values.
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

				if (cpu_ALU_Temp_Val != 0)
				{
					cpu_FlagCset(((cpu_ALU_Long_Result >> (uint32_t)(cpu_ALU_Temp_Val - 1)) & 1) == 1);
				}

				cpu_ALU_Long_Result = cpu_ALU_Long_Result >> (uint32_t)cpu_ALU_Temp_Val;

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_ASR:
				cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;
				cpu_ALU_Temp_S_Val = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Neg_Compare;

				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

				for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
				{
					cpu_FlagCset((cpu_ALU_Long_Result & 1) == 1);
					cpu_ALU_Long_Result = (cpu_ALU_Long_Result >> 1);
					cpu_ALU_Long_Result |= cpu_ALU_Temp_S_Val;
				}

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_ADC:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
				cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];
				cpu_ALU_Long_Result += (uint64_t)(cpu_FlagCget() ? 1 : 0);

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_SBC:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
				cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];
				cpu_ALU_Long_Result -= (uint64_t)(cpu_FlagCget() ? 0 : 1);

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_ROR:
				cpu_ALU_Temp_Val = cpu_Regs[cpu_ALU_Reg_Src] & 0xFF;

				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];

				for (int i = 1; i <= cpu_ALU_Temp_Val; i++)
				{
					cpu_ALU_Temp_S_Val = (uint32_t)(cpu_ALU_Long_Result & 1);
					cpu_ALU_Long_Result = cpu_ALU_Long_Result >> 1;
					cpu_ALU_Long_Result |= (cpu_ALU_Temp_S_Val << 31);
				}

				if (cpu_ALU_Temp_Val != 0) { cpu_FlagCset(cpu_ALU_Temp_S_Val == 1); }

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_TST:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);
				break;

			case cpu_Thumb_NEG:
				cpu_ALU_Long_Result = 0;
				cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Sub(0, cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_CMP:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
				cpu_ALU_Long_Result -= cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);
				break;

			case cpu_Thumb_CMN:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
				cpu_ALU_Long_Result += cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_Regs[cpu_ALU_Reg_Src], (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);
				break;

			case cpu_Thumb_ORR:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] | cpu_Regs[cpu_ALU_Reg_Src];

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_MUL:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] * cpu_Regs[cpu_ALU_Reg_Src];

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_BIC:
				cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest] & (~cpu_Regs[cpu_ALU_Reg_Src]);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_MVN:
				cpu_ALU_Long_Result = (~cpu_Regs[cpu_ALU_Reg_Src]);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);

				cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
				break;

			case cpu_Thumb_High_Add:
				// Add but no flags change
				// unpredictable if both registers are low registers?
				cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

				cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 3) & 0xF);

				if (cpu_Base_Reg == 15)
				{
					// this becomes a branch
					cpu_Take_Branch = true;

					cpu_Temp_Reg = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);
				}
				else
				{
					cpu_Take_Branch = false;
					cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);
				}
				break;

			case cpu_Thumb_High_Cmp:
				// Sub but no flags change
				// unpredictable if first register is 15?
				cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

				cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 3) & 0xF);

				cpu_ALU_Long_Result = cpu_Regs[cpu_Base_Reg];
				cpu_ALU_Long_Result -= cpu_Regs[cpu_Base_Reg_2];

				cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

				cpu_ALU_Long_Result &= cpu_Cast_Int;

				cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_Base_Reg], cpu_Regs[cpu_Base_Reg_2], (uint32_t)cpu_ALU_Long_Result));
				cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
				cpu_FlagZset(cpu_ALU_Long_Result == 0);
				break;

			case cpu_Thumb_High_Bx:
				// Branch and exchange mode (possibly)
				cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 0xF);

				cpu_Temp_Reg = (cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFE);
				break;

			case cpu_Thumb_High_MOV:
				cpu_Temp_Reg = cpu_Regs[((cpu_Instr_TMB_2 >> 3) & 0xF)];

				cpu_Base_Reg = (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8);

				// don't change the read address before branching if destination is a branch
				if (cpu_Base_Reg != 0xF)
				{
					cpu_Regs[cpu_Base_Reg] = cpu_Temp_Reg;
				}
				else
				{
					cpu_Take_Branch = true;
				}
				break;

			case cpu_Thumb_ALU_Imm:
				cpu_ALU_Reg_Dest = (cpu_Instr_TMB_2 >> 8) & 7;

				cpu_ALU_Temp_Val = (uint32_t)(cpu_Instr_TMB_2 & 0xFF);

				switch ((cpu_Instr_TMB_2 >> 11) & 3)
				{
				case 0:			// MOV
					cpu_Regs[cpu_ALU_Reg_Dest] = cpu_ALU_Temp_Val;
					cpu_FlagNset(false);
					cpu_FlagZset(cpu_ALU_Temp_Val == 0);
					break;

				case 1:         // CMP
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);
					break;

				case 2:         // ADD
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result += cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) == cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Add(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;

				case 3:         // SUB
					cpu_ALU_Long_Result = cpu_Regs[cpu_ALU_Reg_Dest];
					cpu_ALU_Long_Result -= cpu_ALU_Temp_Val;

					cpu_FlagCset((cpu_ALU_Long_Result & cpu_Carry_Compare) != cpu_Carry_Compare);

					cpu_ALU_Long_Result &= cpu_Cast_Int;

					cpu_FlagVset(cpu_Calc_V_Flag_Sub(cpu_Regs[cpu_ALU_Reg_Dest], cpu_ALU_Temp_Val, (uint32_t)cpu_ALU_Long_Result));
					cpu_FlagNset((cpu_ALU_Long_Result & cpu_Neg_Compare) == cpu_Neg_Compare);
					cpu_FlagZset(cpu_ALU_Long_Result == 0);

					cpu_Regs[cpu_ALU_Reg_Dest] = (uint32_t)cpu_ALU_Long_Result;
					break;
				}
				break;

			case cpu_Thumb_PC_Rel_LS:
				cpu_LS_Is_Load = true;

				cpu_Base_Reg = 15;

				cpu_Temp_Addr = (uint32_t)((cpu_Regs[cpu_Base_Reg] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));

				cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
				break;

			case cpu_Thumb_Rel_LS:
				switch ((cpu_Instr_TMB_2 & 0xE00) >> 9)
				{
				case 0: cpu_LS_Is_Load = false; break;
				case 1: cpu_LS_Is_Load = false; break;
				case 2: cpu_LS_Is_Load = false; break;
				case 3: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
				case 4: cpu_LS_Is_Load = true; break;
				case 5: cpu_LS_Is_Load = true; break;
				case 6: cpu_LS_Is_Load = true; break;
				case 7: cpu_LS_Is_Load = true; cpu_Sign_Extend_Load = true; break;
				}

				cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);
				cpu_Base_Reg_2 = ((cpu_Instr_TMB_2 >> 6) & 7);

				cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + cpu_Regs[cpu_Base_Reg_2]);

				cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
				break;

			case cpu_Thumb_Imm_LS:
				cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

				cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

				if ((cpu_Instr_TMB_2 & 0x1000) == 0x1000)
				{
					cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 6) & 0x1F));
				}
				else
				{
					cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 4) & 0x7C));
				}


				cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
				break;

			case cpu_Thumb_Half_LS:
				cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

				cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 3) & 7);

				cpu_Temp_Addr = (uint32_t)(cpu_Regs[cpu_Base_Reg] + ((cpu_Instr_TMB_2 >> 5) & 0x3E));

				cpu_Temp_Reg_Ptr = cpu_Instr_TMB_2 & 7;
				break;

			case cpu_Thumb_SP_REL_LS:
				cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

				cpu_Temp_Addr = (uint32_t)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));

				cpu_Temp_Reg_Ptr = (cpu_Instr_TMB_2 >> 8) & 7;
				break;

			case cpu_Thumb_Add_SP_PC:
				cpu_Base_Reg = ((cpu_Instr_TMB_2 >> 8) & 7);

				if ((cpu_Instr_TMB_2 & 0x800) == 0x800)
				{
					cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Regs[13] + ((cpu_Instr_TMB_2 & 0xFF) << 2));
				}
				else
				{
					cpu_Regs[cpu_Base_Reg] = (uint32_t)((cpu_Regs[15] & 0xFFFFFFFC) + ((cpu_Instr_TMB_2 & 0xFF) << 2));
				}
				break;

			case cpu_Thumb_Add_Sub_Stack:
				if ((cpu_Instr_TMB_2 & 0x80) == 0x0)
				{
					cpu_Regs[13] += (uint32_t)((cpu_Instr_TMB_2 & 0x7F) << 2);
				}
				else
				{
					cpu_Regs[13] -= (uint32_t)((cpu_Instr_TMB_2 & 0x7F) << 2);
				}
				break;

			case cpu_Thumb_Push_Pop:
				cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

				// base reg is always 13 for push / pop
				cpu_Base_Reg = 13;

				cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

				cpu_LS_First_Access = true;

				// always overwrite base reg
				cpu_Overwrite_Base_Reg = true;

				// Increment timing depends on it being a push or a pop
				cpu_Multi_Before = !cpu_LS_Is_Load;

				// Increment / Decrement depends on it being a push or a pop
				cpu_Multi_Inc = cpu_LS_Is_Load;

				// No analog to ARM S Bit here
				cpu_Multi_S_Bit = false;
				cpu_Multi_Swap = false;

				// build list of registers to access
				cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

				for (int i = 0; i < 8; i++)
				{
					if (((cpu_Instr_TMB_2 >> i) & 1) == 1)
					{
						cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
						cpu_Multi_List_Size += 1;

						if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
					}
				}

				// additionally, may impact reg 14 (Link), or 15 (PC)
				if ((cpu_Instr_TMB_2 & 0x100) == 0x100)
				{
					if (cpu_LS_Is_Load)
					{
						cpu_Regs_To_Access[cpu_Multi_List_Size] = 15;
					}
					else
					{
						cpu_Regs_To_Access[cpu_Multi_List_Size] = 14;
					}

					cpu_Multi_List_Size += 1;
				}

				// For Pop, start address is at the bottom
				if (!cpu_Multi_Inc)
				{
					cpu_Temp_Addr -= (uint32_t)((cpu_Multi_List_Size - 1) * 4);
					cpu_Temp_Addr -= 4;
					cpu_Write_Back_Addr = cpu_Temp_Addr;
				}

				// No registers selected is unpredictable
				if (cpu_Multi_List_Size == 0)
				{
					throw new std::invalid_argument("no registers selected in Multi Load/Store at " + to_string(cpu_Regs[15]));
				}
				break;

			case cpu_Thumb_Multi_1:
				cpu_LS_Is_Load = (cpu_Instr_TMB_2 & 0x800) == 0x800;

				cpu_Base_Reg = (cpu_Instr_TMB_2 >> 8) & 7;

				cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

				cpu_LS_First_Access = true;

				// always overwrite base reg
				cpu_Overwrite_Base_Reg = true;

				// always increment after
				cpu_Multi_Before = false;

				// always increment
				cpu_Multi_Inc = true;

				// No analog to ARM S Bit here
				cpu_Multi_S_Bit = false;
				cpu_Multi_Swap = false;

				// build list of registers to access
				cpu_Multi_List_Size = cpu_Multi_List_Ptr = 0;

				for (int i = 0; i < 8; i++)
				{
					if (((cpu_Instr_TMB_2 >> i) & 1) == 1)
					{
						cpu_Regs_To_Access[cpu_Multi_List_Size] = i;
						cpu_Multi_List_Size += 1;

						if (cpu_LS_Is_Load && (i == cpu_Base_Reg)) { cpu_Overwrite_Base_Reg = false; }
					}
				}

				// No registers selected loads R15 instead
				if (cpu_Multi_List_Size == 0)
				{
					cpu_Multi_List_Size = 1;
					cpu_Regs_To_Access[0] = 15;
					cpu_Special_Inc = true;
				}
				break;

			case cpu_Thumb_Branch:
				ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 1);

				// offset is signed
				if ((ofst & 0x800) == 0x800) { ofst = (uint32_t)(ofst | 0xFFFFF000); }

				cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

				cpu_Take_Branch = true;
				break;

			case cpu_Thumb_Branch_Cond:
				ofst = (uint32_t)((cpu_Instr_TMB_2 & 0xFF) << 1);

				// offset is signed
				if ((ofst & 0x100) == 0x100) { ofst = (uint32_t)(ofst | 0xFFFFFE00); }

				cpu_Temp_Reg = (uint32_t)(cpu_Regs[15] + ofst);

				cpu_Take_Branch = cpu_TMB_Condition_Check();
				break;

			case cpu_Thumb_Branch_Link_1:
				ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 12);

				// offset is signed
				if ((ofst & 0x400000) == 0x400000) { ofst = (uint32_t)(ofst | 0xFF800000); }

				cpu_Regs[14] = (uint32_t)(cpu_Regs[15] + ofst);
				break;

			case cpu_Thumb_Branch_Link_2:
				ofst = (uint32_t)((cpu_Instr_TMB_2 & 0x7FF) << 1);

				cpu_Temp_Reg = (uint32_t)(cpu_Regs[14] + ofst);

				// NOTE: OR with 1, probably reuses the same cpu circuitry that triggers switch to Thumb mode when writing to R[15] directly?
				cpu_Regs[14] = (uint32_t)((cpu_Regs[15] - 2) | 1);

				cpu_Take_Branch = true;
				break;
			}
		}

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

		#pragma endregion

		#pragma region Disassemble

		// disassemblies will also return strings of the same length
		const char* TraceHeader = "ARM7TDMI: PC, machine code, mnemonic, operands, registers, Cy, flags (ZNHCIFE)";
		const char* SWI_event =  "                                 ====SWI====                                 ";
		const char* UDF_event =  "                                 ====UDF====                                 ";
		const char* IRQ_event =  "                                 ====IRQ====                                 ";
		const char* HALT_event = "                                 ====HALT====                                ";
		const char* No_Reg = 
			"                                                                                                                                                                                                                                                                    ";
		const char* Reg_template = 
			"R0:XXXXXXXX R1:XXXXXXXX R2:XXXXXXXX R3:XXXXXXXX R4:XXXXXXXX R5:XXXXXXXX R6:XXXXXXXX R7:XXXXXXXX R8:XXXXXXXX R9:XXXXXXXX R10:XXXXXXXX R11:XXXXXXXX R12:XXXXXXXX R13:XXXXXXXX R14:XXXXXXXX R15:XXXXXXXX CPSR:XXXXXXXX SPSR:XXXXXXXX Cy:XXXXXXXXXXXXXXXX LY:XXX NZCVIFE";
		const char* Disasm_template = "PCPCPCPCPC:  AABBCCDD  (fail)  Di Di Di Di Di Di Di Di                       ";

		char replacer[40] = {};
		char* val_char_1 = nullptr;
		char* val_char_2 = nullptr;
		uint32_t temp_reg;

		void (*TraceCallback)(int);

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
							sprintf_s(val_char_2, 40, "MUL R%02d, R%02d * R%02d", ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 8) & 0xF), (cpu_Instr_ARM_2 & 0xF));
							return std::string(val_char_2);

						case 0x1:
							// Undefined Opcode Exception
							return "Undefined";

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

						case 0x6:
						case 0x7:
							// Undefined Opcode Exception
							return "Undefined";
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

							sprintf_s(val_char_2, 40, "R%02d)", ((cpu_Instr_ARM_2 >> 12) & 0xF));
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
							if ((cpu_Instr_ARM_2 & 0XFFFF0) == 0xFFF10)
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
							sprintf_s(val_char_2, 40, "MSR SPSR, mask:%02d, ", ((cpu_Instr_ARM_2 >> 16) & 0xF));
							ret.append(std::string(val_char_2));
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
					case 0x8: return "Undefined";
					case 0x9:
						sprintf_s(val_char_2, 40, "MSR CPSR, mask:%02d, (%2X >> %2X)",
								((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1E));
						return std::string(val_char_2);
					case 0xA: return "Undefined";
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
						switch ((cpu_Instr_ARM_2 >> 5) & 3)
						{
						case 0:         // LSL
							sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d << %2X)",
									((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F));
						case 1:         // LSR
							sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d >> %2X)",
									((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F));
						case 2:         // ASR
							sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d ASR %2X)",
									((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F));
						case 3:         // RRX
							sprintf_s(val_char_2, 40, "LD R%02d, (R%02d, R%02d RRX %2X)",
									((cpu_Instr_ARM_2 >> 12) & 0xF), ((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F));
						}
					}
					else
					{
						switch ((cpu_Instr_ARM_2 >> 5) & 3)
						{
						case 0:         // LSL
							sprintf_s(val_char_2, 40, "ST (R%02d, R%02d << %2X), R%02d",
									((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF));
						case 1:         // LSR
							sprintf_s(val_char_2, 40, "ST (R%02d, R%02d >> %2X), R%02d",
									((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF));
						case 2:         // ASR
							sprintf_s(val_char_2, 40, "ST (R%02d, R%02d ASR %2X), R%02d",
									((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF));
						case 3:         // RRX
							sprintf_s(val_char_2, 40, "ST (R%02d, R%02d RRX %2X), R%02d",
									((cpu_Instr_ARM_2 >> 16) & 0xF), (cpu_Instr_ARM_2 & 0xF), ((cpu_Instr_ARM_2 >> 7) & 0x1F), ((cpu_Instr_ARM_2 >> 12) & 0xF));
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
						sprintf_s(val_char_2, 40, "LSL IMM R%02d = R%02d << %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F));

					case 1:
						sprintf_s(val_char_2, 40, "LSR IMM R%02d = R%02d >> %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F));

					case 2:
						sprintf_s(val_char_2, 40, "ROR IMM R%02d = R%02d >> %2X", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 0x1F));
					}
				}

				return std::string(val_char_2);

			case 1:
				// data ops (immedaite)
				switch ((cpu_Instr_TMB_2 >> 11) & 3)
				{
					case 0:         // MOV
						printf_s(val_char_2, 40, "MOV IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF));

					case 1:         // CMP
						sprintf_s(val_char_2, 40, "CMP IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF));

					case 2:         // ADD
						sprintf_s(val_char_2, 40, "ADD IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF));

					case 3:         // SUB
						sprintf_s(val_char_2, 40, "SUB IMM R%02d, %2X", ((cpu_Instr_TMB_2 >> 8) & 7), (cpu_Instr_TMB_2 & 0xFF));
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
								case 0x0: sprintf_s(val_char_2, 40, "AND R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x1: sprintf_s(val_char_2, 40, "EOR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x2: sprintf_s(val_char_2, 40, "LSL R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x3: sprintf_s(val_char_2, 40, "LSR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x4: sprintf_s(val_char_2, 40, "ASR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x5: sprintf_s(val_char_2, 40, "ADC R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x6: sprintf_s(val_char_2, 40, "SBC R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x7: sprintf_s(val_char_2, 40, "ROR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x8: sprintf_s(val_char_2, 40, "TST R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0x9: sprintf_s(val_char_2, 40, "NEG R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0xA: sprintf_s(val_char_2, 40, "CMP R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0xB: sprintf_s(val_char_2, 40, "CMN R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0xC: sprintf_s(val_char_2, 40, "ORR R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0xD: sprintf_s(val_char_2, 40, "MUL R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0xE: sprintf_s(val_char_2, 40, "BIC R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
								case 0xF: sprintf_s(val_char_2, 40, "MVN R%02d, R%02d", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7));
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
										return "CPY?";
									}
									else
									{
										sprintf_s(val_char_2, 40, "MOV R%02d, R%02d", (cpu_Instr_TMB_2 & 0x7) + ((cpu_Instr_TMB_2 >> 4) & 0x8), ((cpu_Instr_TMB_2 >> 3) & 0xF));
										return std::string(val_char_2);
									}

								case 3:
									if ((cpu_Instr_TMB_2 & 0x80) == 0)
									{
										sprintf_s(val_char_2, 40, "Bx (R%02d)", ((cpu_Instr_TMB_2 >> 3) & 0xF));
										return std::string(val_char_2);
									}
									else
									{
										// This version only available in ARM V5 and above
										return "";
									}
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
						case 0: sprintf_s(val_char_2, 40, "ST (R%02d + R%02d), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7), (cpu_Instr_TMB_2 & 7));
						case 1: sprintf_s(val_char_2, 40, "STH (R%02d + R%02d), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7), (cpu_Instr_TMB_2 & 7));
						case 2: sprintf_s(val_char_2, 40, "STB (R%02d + R%02d), R%02d", ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7), (cpu_Instr_TMB_2 & 7));
						case 3: sprintf_s(val_char_2, 40, "LDSB R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
						case 4: sprintf_s(val_char_2, 40, "LD R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
						case 5: sprintf_s(val_char_2, 40, "LDH R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
						case 6: sprintf_s(val_char_2, 40, "LDB R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
						case 7: sprintf_s(val_char_2, 40, "LDSH R%02d, (R%02d + R%02d)", (cpu_Instr_TMB_2 & 7), ((cpu_Instr_TMB_2 >> 3) & 7), ((cpu_Instr_TMB_2 >> 6) & 7));
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
			saver = bool_saver(cpu_Take_Branch, saver);
			saver = bool_saver(cpu_LS_Is_Load, saver);
			saver = bool_saver(cpu_LS_First_Access, saver);
			saver = bool_saver(cpu_Internal_Save_Access, saver);
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

			saver = bool_saver(cpu_Halted, saver);
			saver = bool_saver(stopped, saver);
			saver = bool_saver(jammed, saver);
			saver = bool_saver(cpu_Trigger_Unhalt, saver);
			saver = bool_saver(cpu_Just_Halted, saver);

			saver = short_saver(cpu_Exec_ARM, saver);
			saver = short_saver(cpu_Exec_TMB, saver);
			saver = short_saver(cpu_Instr_TMB_0, saver);
			saver = short_saver(cpu_Instr_TMB_1, saver);
			saver = short_saver(cpu_Instr_TMB_2, saver);
			saver = short_saver(cpu_HS_Ofst_TMB0, saver);
			saver = short_saver(cpu_HS_Ofst_TMB1, saver);
			saver = short_saver(cpu_HS_Ofst_TMB2, saver);
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
			saver = int_saver(cpu_HS_Ofst_ARM0, saver);
			saver = int_saver(cpu_HS_Ofst_ARM1, saver);
			saver = int_saver(cpu_HS_Ofst_ARM2, saver);
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
			loader = bool_loader(&cpu_Take_Branch, loader);
			loader = bool_loader(&cpu_LS_Is_Load, loader);
			loader = bool_loader(&cpu_LS_First_Access, loader);
			loader = bool_loader(&cpu_Internal_Save_Access, loader);
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

			loader = bool_loader(&cpu_Halted, loader);
			loader = bool_loader(&stopped, loader);
			loader = bool_loader(&jammed, loader);
			loader = bool_loader(&cpu_Trigger_Unhalt, loader);
			loader = bool_loader(&cpu_Just_Halted, loader);

			loader = short_loader(&cpu_Exec_ARM, loader);
			loader = short_loader(&cpu_Exec_TMB, loader);
			loader = short_loader(&cpu_Instr_TMB_0, loader);
			loader = short_loader(&cpu_Instr_TMB_1, loader);
			loader = short_loader(&cpu_Instr_TMB_2, loader);
			loader = short_loader(&cpu_HS_Ofst_TMB0, loader);
			loader = short_loader(&cpu_HS_Ofst_TMB1, loader);
			loader = short_loader(&cpu_HS_Ofst_TMB2, loader);
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
			loader = int_loader(&cpu_HS_Ofst_ARM0, loader);
			loader = int_loader(&cpu_HS_Ofst_ARM1, loader);
			loader = int_loader(&cpu_HS_Ofst_ARM2, loader);
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
		// For the CPU, the value will be in cpu_Temp_Reg_Ptr, for DMA it is in dma_TFR_Word. 
		// For the CPU, whether it is a write or not is in cpu_LS_Is_Load, for DMA it is in dma_Read_Cycle	

		uint32_t Wait_State_Access_8(uint32_t addr, bool Seq_Access)
		{
			uint32_t wait_ret = 1;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
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
					else
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;

						// additionally, the prefetch value is not added to the buffer
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}

					//abandon the prefetcher current fetch
					pre_Fetch_Cnt = 0;
					pre_Seq_Access = false;
					pre_Fetch_Cnt_Inc = 0;

					// if the fetch was in ARM mode, discard the whole thing if only part was fetched
					if (!cpu_Thumb_Mode && ((pre_Buffer_Cnt & 1) != 0))
					{
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}
				}
				else if ((Cart_RAM_Present) && (addr < 0x10000000))
				{
					wait_ret += SRAM_Waits; // SRAM
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;

						ppu_OAM_In_Use = true;
					}
				}
				else if (addr >= 0x06000000)
				{
					if (ppu_VRAM_Access)
					{
						wait_ret += 1;

						ppu_VRAM_In_Use = true;
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
				if (addr < 0x0E000000)
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
					else
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
						else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;

						// additionally, the prefetch value is not added to the buffer
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}

					//abandon the prefetcher current fetch
					pre_Fetch_Cnt = 0;
					pre_Seq_Access = false;
					pre_Fetch_Cnt_Inc = 0;

					// if the fetch was in ARM mode, discard the whole thing if only part was fetched
					if (!cpu_Thumb_Mode && ((pre_Buffer_Cnt & 1) != 0))
					{
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}
				}
				else if ((Cart_RAM_Present) && (addr < 0x10000000))
				{
					wait_ret += SRAM_Waits; // SRAM
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;

						ppu_OAM_In_Use = true;
					}
				}
				else if (addr >= 0x06000000)
				{
					if (ppu_VRAM_Access)
					{
						wait_ret += 1;

						ppu_VRAM_In_Use = true;
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
				if (addr < 0x0E000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
					}
					else
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;

						// additionally, the prefetch value is not added to the buffer
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}

					//abandon the prefetcher current fetch
					pre_Fetch_Cnt = 0;
					pre_Seq_Access = false;
					pre_Fetch_Cnt_Inc = 0;

					// if the fetch was in ARM mode, discard the whole thing if only part was fetched
					if (!cpu_Thumb_Mode && ((pre_Buffer_Cnt & 1) != 0))
					{
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}
				}
				else if ((Cart_RAM_Present) && (addr < 0x10000000))
				{
					wait_ret += SRAM_Waits; // SRAM
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;

						ppu_OAM_In_Use = true;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_VRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_VRAM_In_Use = true;
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
				if (addr < 0x0E000000)
				{
					if (addr < 0x0A000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)
					}
					else if (addr < 0x0C000000)
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
					}
					else
					{
						if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
						else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
					}

					if (pre_Cycle_Glitch)
					{
						// lose 1 cycle if prefetcher is holding the bus
						wait_ret += 1;

						// additionally, the prefetch value is not added to the buffer
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}

					//abandon the prefetcher current fetch
					pre_Fetch_Cnt = 0;
					pre_Seq_Access = false;
					pre_Fetch_Cnt_Inc = 0;

					// if the fetch was in ARM mode, discard the whole thing if only part was fetched
					if (!cpu_Thumb_Mode && ((pre_Buffer_Cnt & 1) != 0))
					{
						pre_Buffer_Cnt -= 1;
						pre_Read_Addr -= 2;
					}
				}
				else if ((Cart_RAM_Present) && (addr < 0x10000000))
				{
					wait_ret += SRAM_Waits; // SRAM
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;

						ppu_OAM_In_Use = true;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_VRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_VRAM_In_Use = true;
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
			pre_Seq_Access = true;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
				{
					if (addr == pre_Check_Addr)
					{
						if (pre_Check_Addr != pre_Read_Addr)
						{
							// we have this address, can immediately read it
							pre_Check_Addr += 2;
							pre_Buffer_Cnt -= 1;
						}
						else
						{
							// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
							// plus 1 since the prefetcher already used this cycle, so don't double count
							wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;

							pre_Read_Addr += 2;
							pre_Check_Addr += 2;
							pre_Fetch_Cnt = 0;

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Fetch_Cnt_Inc = 0;
						}
					}
					else
					{
						// the address is not related to the current ones available to the prefetcher
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
						else
						{
							if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N; } // ROM 2, Forced Non-Sequential
							else { wait_ret += Seq_Access ? ROM_Waits_2_S : ROM_Waits_2_N; } // ROM 2
						}

						//abandon the prefetcher current fetch and reset the address
						pre_Buffer_Cnt = 0;
						pre_Fetch_Cnt = 0;
						pre_Fetch_Cnt_Inc = 0;

						if (pre_Enable) { pre_Check_Addr = pre_Read_Addr = addr + 2; }
					}
				}
				else if ((Cart_RAM_Present) && (addr < 0x10000000))
				{
					wait_ret += SRAM_Waits; // SRAM
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;

						ppu_OAM_In_Use = true;
					}
				}
				else if (addr >= 0x06000000)
				{
					if (ppu_VRAM_Access)
					{
						wait_ret += 1;

						ppu_VRAM_In_Use = true;
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
			pre_Seq_Access = true;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0E000000)
				{
					if (addr == pre_Check_Addr)
					{
						if (pre_Check_Addr != pre_Read_Addr)
						{
							// we have this address, can immediately read it
							pre_Check_Addr += 2;
							pre_Buffer_Cnt -= 1;

							// check if we have the next address as well
							if (pre_Check_Addr != pre_Read_Addr)
							{
								// the prefetcher can retun 32 bits in 1 cycle if it has it available
								pre_Check_Addr += 2;
								pre_Buffer_Cnt -= 1;
							}
							else
							{
								// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
								// plus 1 since the prefetcher already used this cycle, so don't double count
								wait_ret = pre_Fetch_Wait - pre_Fetch_Cnt + 1;

								// it is as if the cpu takes over a regular access, so reset the pre-fetcher
								pre_Fetch_Cnt_Inc = 0;

								pre_Read_Addr += 2;
								pre_Check_Addr += 2;
								pre_Fetch_Cnt = 0;
								pre_Buffer_Cnt = 0;
							}
						}
						else
						{
							// we are in the middle of a prefetch access, it takes however many cycles remain to fetch it
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

							// it is as if the cpu takes over a regular access, so reset the pre-fetcher
							pre_Fetch_Cnt_Inc = 0;

							pre_Read_Addr += 4;
							pre_Check_Addr += 4;
							pre_Fetch_Cnt = 0;
						}
					}
					else
					{
						// the address is not related to the current ones available to the prefetcher
						if (addr < 0x0A000000)
						{
							if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_0_S * 2 + 1 : ROM_Waits_0_N + ROM_Waits_0_S + 1; } // ROM 0 (2 accesses)		
						}
						else if (addr < 0x0C000000)
						{
							if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_1_S * 2 + 1 : ROM_Waits_1_N + ROM_Waits_1_S + 1; } // ROM 1 (2 accesses)
						}
						else
						{
							if ((addr & 0x1FFFF) == 0) { wait_ret += ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2, Forced Non-Sequential (2 accesses)
							else { wait_ret += Seq_Access ? ROM_Waits_2_S * 2 + 1 : ROM_Waits_2_N + ROM_Waits_2_S + 1; } // ROM 2 (2 accesses)
						}

						//abandon the prefetcher current fetch and reset the address
						pre_Buffer_Cnt = 0;
						pre_Fetch_Cnt = 0;
						pre_Fetch_Cnt_Inc = 0;

						if (pre_Enable) { pre_Check_Addr = pre_Read_Addr = addr + 4; }
					}
				}
				else if ((Cart_RAM_Present) && (addr < 0x10000000))
				{
					wait_ret += SRAM_Waits; // SRAM
				}
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					if (ppu_OAM_Access)
					{
						wait_ret += 1;

						ppu_OAM_In_Use = true;
					}
				}
				else if (addr >= 0x06000000)
				{
					wait_ret += 1; // PALRAM and VRAM take 2 cycles on 32 bit accesses

					if (ppu_VRAM_Access)
					{
						wait_ret += 1;
					}

					// set to true since we also need to check the next cycle
					ppu_VRAM_In_Use = true;
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
		bool dma_Read_Cycle;
		bool dma_Pausable;
		bool dma_All_Off;
		bool dma_Shutdown;
		bool dma_Release_Bus;
		bool dma_Delay;
		bool dma_Video_DMA_Start;

		uint16_t dma_TFR_HWord;
		uint16_t dma_Held_CPU_Instr;

		uint32_t dma_TFR_Word;

		uint32_t dma_Access_Cnt, dma_Access_Wait, dma_Chan_Exec;
		uint32_t dma_ST_Time_Adjust;

		bool dma_Go[4] = { }; // Tell Condition checkers when the channel is on
		bool dma_Start_VBL[4] = { };
		bool dma_Start_HBL[4] = { };
		bool dma_Start_Snd_Vid[4] = { };
		bool dma_Run[4] = { }; // Actually run the DMA channel
		bool dma_Access_32[4] = { };
		bool dma_Use_ROM_Addr_SRC[4] = { };
		bool dma_Use_ROM_Addr_DST[4] = { };
		bool dma_ROM_Being_Used[4] = { };
		bool dma_External_Source[4] = { };

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
		uint32_t dma_IRQ_cd[4] = { };

		uint64_t dma_Run_En_Time[4] = { };
		
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
				dma_Run_En_Time[chan] = 0xFFFFFFFFFFFFFFFF;

				dma_Start_Snd_Vid[chan] = false;
				if ((value & 0x3000) == 0x0000) { dma_Run_En_Time[chan] = CycleCount + 3; }
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
						}
					}
				}

				//Console.WriteLine(chan + " " + value);

				dma_Go[chan] = true;

				dma_All_Off = false;

				dma_External_Source[chan] = false;
			}

			if ((value & 0x8000) == 0)
			{
				// if the channel isnt currently running, turn it off
				dma_Run[chan] = false;
				dma_Go[chan] = false;
				dma_Run_En_Time[chan] = 0xFFFFFFFFFFFFFFFF;

				dma_All_Off = true;

				for (int i = 0; i < 4; i++) { dma_All_Off &= !dma_Go[i]; }
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

				dma_Run_En_Time[i] = 0xFFFFFFFFFFFFFFFF;

				dma_ST_Time[i] = 0;
				dma_ROM_Addr[i] = 0;
				dma_IRQ_cd[i] = 0;

				dma_SRC[i] = 0;
				dma_DST[i] = 0;
				dma_SRC_intl[i] = 0;
				dma_DST_intl[i] = 0;
				dma_SRC_INC[i] = 0;
				dma_DST_INC[i] = 0;

				dma_Last_Bus_Value[i] = 0;

				dma_CNT[i] = 0;
				dma_CTRL[i] = 0;

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
				dma_External_Source[i] = false;
			}

			dma_Access_Cnt = dma_Access_Wait = 0;

			dma_Chan_Exec = 4;

			dma_ST_Time_Adjust = 0;

			dma_TFR_Word = 0;

			dma_TFR_HWord = dma_Held_CPU_Instr = 0;

			dma_Seq_Access = false;
			dma_Read_Cycle = true;
			dma_Pausable = true;
			dma_All_Off = true;
			dma_Shutdown = dma_Release_Bus = false;
			dma_Delay = false;
			dma_Video_DMA_Start = false;
		}

		uint8_t* dma_SaveState(uint8_t* saver)
		{
			saver = bool_saver(dma_Seq_Access, saver);
			saver = bool_saver(dma_Read_Cycle, saver);
			saver = bool_saver(dma_Pausable, saver);
			saver = bool_saver(dma_All_Off, saver);
			saver = bool_saver(dma_Shutdown, saver);
			saver = bool_saver(dma_Release_Bus, saver);
			saver = bool_saver(dma_Delay, saver);
			saver = bool_saver(dma_Video_DMA_Start, saver);

			saver = short_saver(dma_TFR_HWord, saver);
			saver = short_saver(dma_Held_CPU_Instr, saver);

			saver = int_saver(dma_TFR_Word, saver);

			saver = int_saver(dma_Access_Cnt, saver);
			saver = int_saver(dma_Access_Wait, saver);
			saver = int_saver(dma_Chan_Exec, saver);
			saver = int_saver(dma_ST_Time_Adjust, saver);

			saver = bool_array_saver(dma_Go, saver, 4);
			saver = bool_array_saver(dma_Start_VBL, saver, 4);
			saver = bool_array_saver(dma_Start_HBL, saver, 4);
			saver = bool_array_saver(dma_Start_Snd_Vid, saver, 4);
			saver = bool_array_saver(dma_Run, saver, 4);
			saver = bool_array_saver(dma_Access_32, saver, 4);
			saver = bool_array_saver(dma_Use_ROM_Addr_SRC, saver, 4);
			saver = bool_array_saver(dma_Use_ROM_Addr_DST, saver, 4);
			saver = bool_array_saver(dma_ROM_Being_Used, saver, 4);
			saver = bool_array_saver(dma_External_Source, saver, 4);

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
			saver = int_array_saver(dma_IRQ_cd, saver, 4);

			saver = long_array_saver(dma_Run_En_Time, saver, 4);

			return saver;
		}

		uint8_t* dma_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&dma_Seq_Access, loader);
			loader = bool_loader(&dma_Read_Cycle, loader);
			loader = bool_loader(&dma_Pausable, loader);
			loader = bool_loader(&dma_All_Off, loader);
			loader = bool_loader(&dma_Shutdown, loader);
			loader = bool_loader(&dma_Release_Bus, loader);
			loader = bool_loader(&dma_Delay, loader);
			loader = bool_loader(&dma_Video_DMA_Start, loader);

			loader = short_loader(&dma_TFR_HWord, loader);
			loader = short_loader(&dma_Held_CPU_Instr, loader);

			loader = int_loader(&dma_TFR_Word, loader);

			loader = int_loader(&dma_Access_Cnt, loader);
			loader = int_loader(&dma_Access_Wait, loader);
			loader = int_loader(&dma_Chan_Exec, loader);
			loader = int_loader(&dma_ST_Time_Adjust, loader);

			loader = bool_array_loader(dma_Go, loader, 4);
			loader = bool_array_loader(dma_Start_VBL, loader, 4);
			loader = bool_array_loader(dma_Start_HBL, loader, 4);
			loader = bool_array_loader(dma_Start_Snd_Vid, loader, 4);
			loader = bool_array_loader(dma_Run, loader, 4);
			loader = bool_array_loader(dma_Access_32, loader, 4);
			loader = bool_array_loader(dma_Use_ROM_Addr_SRC, loader, 4);
			loader = bool_array_loader(dma_Use_ROM_Addr_DST, loader, 4);
			loader = bool_array_loader(dma_ROM_Being_Used, loader, 4);
			loader = bool_array_loader(dma_External_Source, loader, 4);

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
			loader = int_array_loader(dma_IRQ_cd, loader, 4);

			loader = long_array_loader(dma_Run_En_Time, loader, 4);

			return loader;
		}

	#pragma endregion

	#pragma region Serial port

		bool ser_Internal_Clock, ser_Start;
		bool ser_Delay, key_Delay;

		uint8_t ser_div_cnt, ser_Mask;
		uint8_t ser_Bit_Count, ser_Bit_Total;
		
		uint16_t ser_Data_0, ser_Data_1, ser_Data_2, ser_Data_3, ser_Data_M;
		uint16_t ser_CTRL, ser_CTRL_J, ser_STAT_J, ser_Mode;
		uint16_t key_CTRL;

		uint32_t ser_RECV_J, ser_TRANS_J;
		uint32_t ser_Delay_cd, key_Delay_cd;

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
				case 0x132: key_CTRL = (uint16_t)((key_CTRL & 0xFF00) | value); do_controller_check(); do_controller_check_glitch(); break;
				// note no check here, does not seem to trigger onhardware, see joypad.gba
				case 0x133: key_CTRL = (uint16_t)((key_CTRL & 0x00FF) | (value << 8)); /*do_controller_check(); do_controller_check_glitch(); */ break;

				case 0x134: ser_Mode = (uint16_t)((ser_Mode & 0xFF00) | value); break;
				case 0x135: ser_Mode = (uint16_t)((ser_Mode & 0x00FF) | (value << 8)); break;

				case 0x140: ser_CTRL_J = (uint16_t)((ser_CTRL_J & 0xFF00) | value); break;
				case 0x141: ser_CTRL_J = (uint16_t)((ser_CTRL_J & 0x00FF) | (value << 8)); break;

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

		void ser_Write_Reg_16(uint32_t addr, uint16_t value)
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
				case 0x132: key_CTRL = value; do_controller_check(); do_controller_check_glitch(); break;

				case 0x134: ser_Mode = value; break;

				case 0x140: ser_CTRL_J = value; break;

				case 0x150: ser_RECV_J = (uint32_t)((ser_RECV_J & 0xFFFF0000) | value); break;
				case 0x152: ser_RECV_J = (uint32_t)((ser_RECV_J & 0x0000FFFF) | (value << 16)); break;
				case 0x154: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0xFFFF0000) | value); break;
				case 0x156: ser_TRANS_J = (uint32_t)((ser_TRANS_J & 0x0000FFFF) | (value << 16)); break;
				case 0x158: ser_STAT_J = value; break;
			}
		}

		void ser_Write_Reg_32(uint32_t addr, uint32_t value)
		{
			switch (addr)
			{
				case 0x120: ser_Data_0 = (uint16_t)(value & 0xFFFF);
					ser_Data_1 = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x124: ser_Data_2 = (uint16_t)(value & 0xFFFF);
					ser_Data_3 = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x128: ser_CTRL_Update((uint16_t)(value & 0xFFFF));
					ser_Data_M = (uint16_t)((value >> 16) & 0xFFFF); break;

				case 0x130: key_CTRL = (uint16_t)((value >> 16) & 0xFFFF); do_controller_check(); do_controller_check_glitch(); break;

				case 0x134: ser_Mode = (uint16_t)(value & 0xFFFF); break;

				case 0x140: ser_CTRL_J = (uint16_t)(value & 0xFFFF); break;

				case 0x150: ser_RECV_J = value; break;
				case 0x154: ser_TRANS_J = value; break;
				case 0x158: ser_STAT_J = (uint16_t)(value & 0xFFFF); break;
			}
		}

		void ser_CTRL_Update(uint16_t value)
		{
			// actiavte the port
			if (!ser_Start && ((value & 0x80) == 0x80))
			{
				ser_Bit_Count = 0;

				ser_Bit_Total = (uint8_t)((value & 0x1000) == 0x1000 ? 32 : 8);

				ser_Mask = (uint8_t)((value & 0x2) == 0x2 ? 0x7 : 0xF);

				ser_Internal_Clock = (value & 0x1) == 0x1;

				ser_Start = true;
			}

			if ((value & 0x80) != 0x80) { ser_Start = false; }

			ser_CTRL = value;
		}

		void ser_Reset()
		{
			ser_RECV_J = ser_TRANS_J = 0;

			ser_Delay_cd = key_Delay_cd = 0;

			ser_Data_0 = ser_Data_1 = ser_Data_2 = ser_Data_3 = ser_Data_M = 0;

			ser_CTRL = ser_CTRL_J = ser_STAT_J = ser_Mode = 0;

			key_CTRL = 0;

			ser_div_cnt = 0;

			ser_Mask = 0xF;

			ser_Bit_Count = ser_Bit_Total = 0;

			ser_Internal_Clock = ser_Start = false;

			ser_Delay = key_Delay = false;
		}

		uint8_t* ser_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ser_Internal_Clock, saver);
			saver = bool_saver(ser_Start, saver);
			saver = bool_saver(ser_Delay, saver);
			saver = bool_saver(key_Delay, saver);

			saver = byte_saver(ser_div_cnt, saver);
			saver = byte_saver(ser_Mask, saver);
			saver = byte_saver(ser_Bit_Count, saver);
			saver = byte_saver(ser_Bit_Total, saver);

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

			return saver;
		}

		uint8_t* ser_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ser_Internal_Clock, loader);
			loader = bool_loader(&ser_Start, loader);
			loader = bool_loader(&ser_Delay, loader);
			loader = bool_loader(&key_Delay, loader);

			loader = byte_loader(&ser_div_cnt, loader);
			loader = byte_loader(&ser_Mask, loader);
			loader = byte_loader(&ser_Bit_Count, loader);
			loader = byte_loader(&ser_Bit_Total, loader);

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

		uint16_t tim_Timer[4] = { };
		uint16_t tim_Reload[4] = { };
		uint16_t tim_Control[4] = { };
		uint16_t tim_PreSc[4] = { };
		uint16_t tim_ST_Time[4] = { };
		uint16_t tim_IRQ_CD[4] = { };
		uint16_t tim_IRQ_Time[4] = { };

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
				tim_Timer[nbr] = tim_Reload[nbr];

				tim_ST_Time[nbr] = 3;

				tim_PreSc[nbr] = PreScales[value & 3];

				if (nbr != 0) { tim_Tick_By_Prev[nbr] = ((value & 0x4) == 0x4); }

				tim_IRQ_Time[nbr] = 3;

				tim_All_Off = false;

				// if enabling at 0xFFFF and no prescale, it is as if the timer is delayed an extra tick
				// but the IRQ is still triggered immediately
				if ((tim_Timer[nbr] == 0xFFFF) && ((value & 3) == 0) && !tim_Tick_By_Prev[nbr])
				{
					tim_ST_Time[nbr] = 4;
					tim_IRQ_Time[nbr] = 2;
				}
			}

			if ((value & 0x80) == 0)
			{
				// timer ticks for one additional cycle when disabled
				if (tim_Go[nbr])
				{
					tim_Disable[nbr] = true;
				}
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
				tim_ST_Time[i] = 0;
				tim_IRQ_CD[i] = 0;
				tim_IRQ_Time[i] = 0;

				tim_Go[i] = false;
				tim_Tick_By_Prev[i] = false;
				tim_Prev_Tick[i] = false;
				tim_Disable[i] = false;
			}

			tim_Just_Reloaded = 5;

			tim_SubCnt = 0;

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

			saver = short_array_saver(tim_Timer, saver, 4);
			saver = short_array_saver(tim_Reload, saver, 4);
			saver = short_array_saver(tim_Control, saver, 4);
			saver = short_array_saver(tim_PreSc, saver, 4);
			saver = short_array_saver(tim_ST_Time, saver, 4);
			saver = short_array_saver(tim_IRQ_CD, saver, 4);
			saver = short_array_saver(tim_IRQ_Time, saver, 4);

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

			loader = short_array_loader(tim_Timer, loader, 4);
			loader = short_array_loader(tim_Reload, loader, 4);
			loader = short_array_loader(tim_Control, loader, 4);
			loader = short_array_loader(tim_PreSc, loader, 4);
			loader = short_array_loader(tim_ST_Time, loader, 4);
			loader = short_array_loader(tim_IRQ_CD, loader, 4);
			loader = short_array_loader(tim_IRQ_Time, loader, 4);

			return loader;
		}

	#pragma endregion

	#pragma region PPU

		uint32_t ppu_OBj_Sizes_X[16] = {8, 16, 8, 8,
										16, 32, 8, 8,
										32, 32, 16, 8,
										64, 64, 32, 8 };

		uint32_t ppu_OBj_Sizes_Y[16] = {8, 8, 16, 8,
										16, 8, 32, 8,
										32, 16, 32, 8,
										64, 32, 64, 8 };

		bool ppu_In_VBlank;
		bool ppu_Delays;

		bool ppu_VRAM_In_Use, ppu_PALRAM_In_Use, ppu_OAM_In_Use;

		bool ppu_VRAM_Access;
		bool ppu_PALRAM_Access;
		bool ppu_OAM_Access;

		bool ppu_HBL_Free, ppu_OBJ_Dim, ppu_Forced_Blank, ppu_Any_Window_On;
		bool ppu_OBJ_On, ppu_WIN0_On, ppu_WIN1_On, ppu_OBJ_WIN;

		uint8_t ppu_STAT, ppu_LY, ppu_LYC;

		uint16_t ppu_CTRL, ppu_Green_Swap, ppu_Cycle, ppu_Display_Cycle, ppu_Sprite_Eval_Time;
		uint16_t ppu_WIN_Hor_0, ppu_WIN_Hor_1, ppu_WIN_Vert_0, ppu_WIN_Vert_1;
		uint16_t ppu_WIN_In, ppu_WIN_Out, ppu_Mosaic, ppu_Special_FX, ppu_Alpha, ppu_Bright;

		uint32_t ppu_BG_Mode, ppu_Display_Frame;
		
		uint32_t ppu_X_RS, ppu_Y_RS;

		uint32_t ppu_VBL_IRQ_cd, ppu_HBL_IRQ_cd, ppu_LYC_IRQ_cd;

		uint32_t ppu_LYC_Vid_Check_cd;

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
		uint32_t ppu_BG_Ref_X_Latch[4] = { };
		uint32_t ppu_BG_Ref_Y_Latch[4] = { };

		uint32_t ppu_BG_On_Update_Time[4] = { };

		bool ppu_BG_On[4] = { };
		bool ppu_BG_On_New[4] = { };

		// Sprite Evaluation
		bool ppu_New_Sprite, ppu_Sprite_Eval_Finished;

		uint16_t ppu_Sprite_Attr_0, ppu_Sprite_Attr_1, ppu_Sprite_Attr_2;

		uint32_t ppu_Current_Sprite;
		uint32_t ppu_Sprite_ofst_eval;
		uint32_t ppu_Sprite_ofst_draw;
		uint32_t ppu_Sprite_proc_time;
		uint32_t ppu_Sprite_X_Pos, ppu_Sprite_Y_Pos;
		uint32_t ppu_Sprite_X_Size, ppu_Sprite_Y_Size;

		bool ppu_Sprite_Pixel_Occupied[240 * 2] = { };
		bool ppu_Sprite_Semi_Transparent[240 * 2] = { };
		bool ppu_Sprite_Object_Window[240 * 2] = { };

		uint32_t ppu_Sprite_Pixels[240 * 2] = { };
		uint32_t ppu_Sprite_Priority[240 * 2] = { };

		// BG rendering
		uint32_t ppu_Fetch_Count[4] = { };
		uint32_t ppu_Scroll_Cycle[4] = { };
		uint32_t ppu_Pixel_Color[4] = { };
		uint32_t ppu_Pixel_Color_2[4] = { };
		uint32_t ppu_Pixel_Color_1[4] = { };
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
		bool ppu_BG_Has_Pixel_R[4] = { };

		uint32_t ppu_BG_Pixel_F;
		uint32_t ppu_BG_Pixel_S;
		uint32_t ppu_Final_Pixel;
		uint32_t ppu_Blend_Pixel;

		bool ppu_Brighten_Final_Pixel;
		bool ppu_Blend_Final_Pixel;

		// Palette fetches for BG's are true unless they happen from mode 3 or 5, where the color is directly encoded in VRAM
		// When this occurs, the first BG pixel will always be from BG 2, and the second from backdrop
		bool ppu_Fetch_BG;

		bool ppu_Fetch_Target_1;
		bool ppu_Fetch_Target_2;

		bool ppu_Rendering_Complete;
		bool ppu_PAL_Rendering_Complete;

		// Derived values, not stated, reloaded with savestate
		uint32_t BG_Scale_X[4] = { };
		uint32_t BG_Scale_Y[4] = { };
		uint32_t BG_Num_Tiles[4] = { };
		uint32_t ppu_BG_Char_Base[4] = { };
		uint32_t ppu_BG_Screen_Base[4] = { };

		uint16_t ppu_ROT_OBJ_X[128 * 128 * 128] = { };
		uint16_t ppu_ROT_OBJ_Y[128 * 128 * 128] = { };

		uint16_t ppu_MOS_OBJ_X[0x200] = { };
		uint16_t ppu_MOS_OBJ_Y[0x100] = { };

		uint16_t ppu_MOS_BG_X[0x200] = { };
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

		double ppu_F_Ref_X_2, ppu_F_Ref_X_3;
		double ppu_F_Ref_Y_2, ppu_F_Ref_Y_3;

		double ppu_F_Rot_A_2, ppu_F_Rot_B_2, ppu_F_Rot_C_2, ppu_F_Rot_D_2;
		double ppu_F_Rot_A_3, ppu_F_Rot_B_3, ppu_F_Rot_C_3, ppu_F_Rot_D_3;

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
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write((uint16_t)((ppu_CTRL & 0xFF00) | value)); break;
				case 0x01: ppu_CTRL_Write((uint16_t)((ppu_CTRL & 0x00FF) | (value << 8))); break;
				case 0x02: ppu_Green_Swap = (uint16_t)((ppu_Green_Swap & 0xFF00) | value); break;
				case 0x03: ppu_Green_Swap = (uint16_t)((ppu_Green_Swap & 0x00FF) | (value << 8)); break;
				case 0x04: ppu_STAT_Write(value); break;
				case 0x05: ppu_LYC = value; break;
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

				case 0x10: ppu_BG_X[0] = (uint16_t)((ppu_BG_X[0] & 0xFF00) | value); break;
				case 0x11: ppu_BG_X[0] = (uint16_t)((ppu_BG_X[0] & 0x00FF) | (value << 8)); break;
				case 0x12: ppu_BG_Y[0] = (uint16_t)((ppu_BG_Y[0] & 0xFF00) | value); break;
				case 0x13: ppu_BG_Y[0] = (uint16_t)((ppu_BG_Y[0] & 0x00FF) | (value << 8)); break;
				case 0x14: ppu_BG_X[1] = (uint16_t)((ppu_BG_X[1] & 0xFF00) | value); break;
				case 0x15: ppu_BG_X[1] = (uint16_t)((ppu_BG_X[1] & 0x00FF) | (value << 8)); break;
				case 0x16: ppu_BG_Y[1] = (uint16_t)((ppu_BG_Y[1] & 0xFF00) | value); break;
				case 0x17: ppu_BG_Y[1] = (uint16_t)((ppu_BG_Y[1] & 0x00FF) | (value << 8)); break;
				case 0x18: ppu_BG_X[2] = (uint16_t)((ppu_BG_X[2] & 0xFF00) | value); break;
				case 0x19: ppu_BG_X[2] = (uint16_t)((ppu_BG_X[2] & 0x00FF) | (value << 8)); break;
				case 0x1A: ppu_BG_Y[2] = (uint16_t)((ppu_BG_Y[2] & 0xFF00) | value); break;
				case 0x1B: ppu_BG_Y[2] = (uint16_t)((ppu_BG_Y[2] & 0x00FF) | (value << 8)); break;
				case 0x1C: ppu_BG_X[3] = (uint16_t)((ppu_BG_X[3] & 0xFF00) | value); break;
				case 0x1D: ppu_BG_X[3] = (uint16_t)((ppu_BG_X[3] & 0x00FF) | (value << 8)); break;
				case 0x1E: ppu_BG_Y[3] = (uint16_t)((ppu_BG_Y[3] & 0xFF00) | value); break;
				case 0x1F: ppu_BG_Y[3] = (uint16_t)((ppu_BG_Y[3] & 0x00FF) | (value << 8)); break;

				case 0x20: ppu_BG_Rot_A[2] = (uint16_t)((ppu_BG_Rot_A[2] & 0xFF00) | value); ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x21: ppu_BG_Rot_A[2] = (uint16_t)((ppu_BG_Rot_A[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x22: ppu_BG_Rot_B[2] = (uint16_t)((ppu_BG_Rot_B[2] & 0xFF00) | value); ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x23: ppu_BG_Rot_B[2] = (uint16_t)((ppu_BG_Rot_B[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x24: ppu_BG_Rot_C[2] = (uint16_t)((ppu_BG_Rot_C[2] & 0xFF00) | value); ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x25: ppu_BG_Rot_C[2] = (uint16_t)((ppu_BG_Rot_C[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x26: ppu_BG_Rot_D[2] = (uint16_t)((ppu_BG_Rot_D[2] & 0xFF00) | value); ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x27: ppu_BG_Rot_D[2] = (uint16_t)((ppu_BG_Rot_D[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x28: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFFFFFF00) | value); break;
				case 0x29: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFFFF00FF) | (value << 8)); break;
				case 0x2A: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFF00FFFF) | (value << 16)); break;
				case 0x2B: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0x00FFFFFF) | (value << 24)); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFFFFFF00) | value); ppu_ROT_REF_LY_Update(2); break;
				case 0x2D: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_LY_Update(2); break;
				case 0x2E: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(2); break;
				case 0x2F: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0x00FFFFFF) | (value << 24)); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = (uint16_t)((ppu_BG_Rot_A[3] & 0xFF00) | value); ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x31: ppu_BG_Rot_A[3] = (uint16_t)((ppu_BG_Rot_A[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x32: ppu_BG_Rot_B[3] = (uint16_t)((ppu_BG_Rot_B[3] & 0xFF00) | value); ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x33: ppu_BG_Rot_B[3] = (uint16_t)((ppu_BG_Rot_B[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x34: ppu_BG_Rot_C[3] = (uint16_t)((ppu_BG_Rot_C[3] & 0xFF00) | value); ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x35: ppu_BG_Rot_C[3] = (uint16_t)((ppu_BG_Rot_C[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x36: ppu_BG_Rot_D[3] = (uint16_t)((ppu_BG_Rot_D[3] & 0xFF00) | value); ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x37: ppu_BG_Rot_D[3] = (uint16_t)((ppu_BG_Rot_D[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x38: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFFFFFF00) | value); break;
				case 0x39: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFFFF00FF) | (value << 8)); break;
				case 0x3A: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFF00FFFF) | (value << 16)); break;
				case 0x3B: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0x00FFFFFF) | (value << 24)); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFFFFFF00) | value); ppu_ROT_REF_LY_Update(3); break;
				case 0x3D: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_LY_Update(3); break;
				case 0x3E: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(3); break;
				case 0x3F: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0x00FFFFFF) | (value << 24)); ppu_ROT_REF_LY_Update(3); break;

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
				case 0x4A: ppu_Update_Win_Out((uint16_t)((ppu_WIN_In & 0xFF00) | value)); break;
				case 0x4B: ppu_Update_Win_Out((uint16_t)((ppu_WIN_In & 0x00FF) | (value << 8))); break;
				case 0x4C: ppu_Update_Mosaic((uint16_t)((ppu_Mosaic & 0xFF00) | value)); break;
				case 0x4D: ppu_Update_Mosaic((uint16_t)((ppu_Mosaic & 0x00FF) | (value << 8))); break;

				case 0x50: ppu_Update_Special_FX((uint16_t)((ppu_WIN_In & 0xFF00) | value)); break;
				case 0x51: ppu_Update_Special_FX((uint16_t)((ppu_WIN_In & 0x00FF) | (value << 8))); break;
				case 0x52: ppu_Update_Alpha((uint16_t)((ppu_WIN_In & 0xFF00) | value)); break;
				case 0x53: ppu_Update_Alpha((uint16_t)((ppu_WIN_In & 0x00FF) | (value << 8))); break;
				case 0x54: ppu_Update_Bright((uint16_t)((ppu_Mosaic & 0xFF00) | value)); break;
				case 0x55: ppu_Update_Bright((uint16_t)((ppu_Mosaic & 0x00FF) | (value << 8))); break;
			}
		}

		void ppu_Write_Reg_16(uint32_t addr, uint16_t value)
		{
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write(value); break;
				case 0x02: ppu_Green_Swap = value; break;
				case 0x04: ppu_STAT_Write((uint8_t)value); ppu_LYC = (uint8_t)(value >> 8); break;
				case 0x06: // No Effect on LY
				case 0x08: ppu_BG_CTRL[0] = (uint16_t)(value & 0xDFFF); ppu_BG_CTRL_Write(0); break;
				case 0x0A: ppu_BG_CTRL[1] = (uint16_t)(value & 0xDFFF); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = value; ppu_BG_CTRL_Write(2); break;
				case 0x0E: ppu_BG_CTRL[3] = value; ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X[0] = value; break;
				case 0x12: ppu_BG_Y[0] = value; break;
				case 0x14: ppu_BG_X[1] = value; break;
				case 0x16: ppu_BG_Y[1] = value; break;
				case 0x18: ppu_BG_X[2] = value; break;
				case 0x1A: ppu_BG_Y[2] = value; break;
				case 0x1C: ppu_BG_X[3] = value; break;
				case 0x1E: ppu_BG_Y[3] = value; break;

				case 0x20: ppu_BG_Rot_A[2] = value; ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x22: ppu_BG_Rot_B[2] = value; ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x24: ppu_BG_Rot_C[2] = value; ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x26: ppu_BG_Rot_D[2] = value; ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x28: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0xFFFF0000) | value); break;
				case 0x2A: ppu_BG_Ref_X[2] = (uint32_t)((ppu_BG_Ref_X[2] & 0x0000FFFF) | (value << 16)); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0xFFFF0000) | value); ppu_ROT_REF_LY_Update(2); break;
				case 0x2E: ppu_BG_Ref_Y[2] = (uint32_t)((ppu_BG_Ref_Y[2] & 0x0000FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = value; ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x32: ppu_BG_Rot_B[3] = value; ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x34: ppu_BG_Rot_C[3] = value; ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x36: ppu_BG_Rot_D[3] = value; ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x38: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0xFFFF0000) | value); break;
				case 0x3A: ppu_BG_Ref_X[3] = (uint32_t)((ppu_BG_Ref_X[3] & 0x0000FFFF) | (value << 16)); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0xFFFF0000) | value); ppu_ROT_REF_LY_Update(3); break;
				case 0x3E: ppu_BG_Ref_Y[3] = (uint32_t)((ppu_BG_Ref_Y[3] & 0x0000FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(3); break;

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
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write((uint16_t)(value & 0xFFFF));
						   ppu_Green_Swap = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x04: ppu_STAT_Write((uint8_t)value); ppu_LYC = (uint8_t)(value >> 8); break;
						   /* no effect on LY*/
				case 0x08: ppu_BG_CTRL[0] = (uint16_t)(value & 0xDFFF); ppu_BG_CTRL_Write(0);
						   ppu_BG_CTRL[1] = (uint16_t)((value >> 16) & 0xDFFF); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = (uint16_t)(value & 0xFFFF); ppu_BG_CTRL_Write(2);
						   ppu_BG_CTRL[3] = (uint16_t)((value >> 16) & 0xFFFF); ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X[0] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Y[0] = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x14: ppu_BG_X[1] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Y[1] = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x18: ppu_BG_X[2] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Y[2] = (uint16_t)((value >> 16) & 0xFFFF); break;
				case 0x1C: ppu_BG_X[3] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Y[3] = (uint16_t)((value >> 16) & 0xFFFF); break;

				case 0x20: ppu_BG_Rot_A[2] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_B[2] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_float_AB(2); break;
				case 0x24: ppu_BG_Rot_C[2] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_D[2] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_float_CD(2); break;
				case 0x28: ppu_BG_Ref_X[2] = value; break;
				case 0x2C: ppu_BG_Ref_Y[2] = value; ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_B[3] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_float_AB(3); break;
				case 0x34: ppu_BG_Rot_C[3] = (uint16_t)(value & 0xFFFF);
						   ppu_BG_Rot_D[3] = (uint16_t)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_float_CD(3); break;
				case 0x38: ppu_BG_Ref_X[3] = value; break;
				case 0x3C: ppu_BG_Ref_Y[3] = value; ppu_ROT_REF_LY_Update(3); break;

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

		void ppu_STAT_Write(uint8_t value)
		{
			ppu_STAT &= 0xC7;
			ppu_STAT |= (uint8_t)(value & 0x38);
		}

		void ppu_CTRL_Write(uint16_t value)
		{
			ppu_BG_Mode = value & 7;
			ppu_Display_Frame = (value >> 4) & 1;

			ppu_HBL_Free = (value & 0x20) == 0x20;
			ppu_OBJ_Dim = (value & 0x40) == 0x40;
			ppu_Forced_Blank = (value & 0x80) == 0x80;

			ppu_OBJ_On = (value & 0x1000) == 0x1000;
			ppu_WIN0_On = (value & 0x2000) == 0x2000;
			ppu_WIN1_On = (value & 0x4000) == 0x4000;
			ppu_OBJ_WIN = (value & 0x8000) == 0x8000;

			// BG's don't turn on immediately, but do turn off immediately
			for (int i = 0; i < 4; i++)
			{
				if ((value & (0x100 << i)) == 0)
				{
					ppu_BG_On[i] = false;
					ppu_BG_On_Update_Time[i] = 0;
				}
				else if (ppu_BG_On_Update_Time[i] == 0)
				{
					// if the change happens before around cycle 40, then the delay is only 2 scanlines not 3
					// see spyro season of ice, talking to the first fairy
					if (ppu_Cycle < 40)
					{
						ppu_BG_On_Update_Time[i] = 2;
					}
					else
					{
						ppu_BG_On_Update_Time[i] = 3;
					}
				}
			}

			if (ppu_HBL_Free) { ppu_Sprite_Eval_Time = 960; }
			else { ppu_Sprite_Eval_Time = 1216; }

			ppu_Any_Window_On = ppu_WIN0_On || ppu_WIN1_On || ppu_OBJ_WIN;

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
				ppu_OAM_Access = false;
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

		void ppu_ROT_REF_LY_Update(int layer)
		{
			if (ppu_BG_On[layer])
			{
				if (ppu_Cycle < 40)
				{
					ppu_ROT_REF_LY[layer] = ppu_LY;
				}
				else
				{
					ppu_ROT_REF_LY[layer] = (uint16_t)(ppu_LY + 1);
				}
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

			mosaic_x = 0;
			j = 0;

			for (int i = 0; i < 0x200; i++)
			{
				ppu_MOS_BG_X[i] = mosaic_x;

				j++;

				if (j == ppu_BG_Mosaic_X)
				{
					mosaic_x += ppu_BG_Mosaic_X;
					j = 0;
				}
			}

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

			mosaic_y = 0;
			j = 0;

			for (int i = 0; i < 0x100; i++)
			{
				ppu_MOS_OBJ_Y[i] = mosaic_y;

				j++;

				if (j == ppu_OBJ_Mosaic_Y)
				{
					mosaic_y += ppu_OBJ_Mosaic_Y;
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

		inline void ppu_Render()
		{
			double cur_x, cur_y;
			double sol_x, sol_y;

			uint32_t R, G, B;
			uint32_t R2, G2, B2;

			uint32_t spr_pixel;

			int cur_layer_priority;
			int second_layer_priority;

			int temp_addr;
			int temp_color;

			int Screen_Offset;

			int VRAM_ofst_X, VRAM_ofst_Y;

			int cur_BG_layer;
			int second_BG_layer;

			bool Is_Outside;
			bool OBJ_Has_Pixel;
			bool OBJ_Go;
			bool Color_FX_Go;

			BG_Go[0] = ppu_BG_On[0];
			BG_Go[1] = ppu_BG_On[1];
			BG_Go[2] = ppu_BG_On[2];
			BG_Go[3] = ppu_BG_On[3];

			if (!ppu_PAL_Rendering_Complete)
			{
				// move pixel data up up the pipeline
				if (((ppu_Cycle & 3) == 0) && (ppu_Cycle >= 36))
				{
					switch (ppu_BG_Mode)
					{
					case 0:
						for (int c0 = 0; c0 < 4; c0++)
						{
							if (ppu_BG_On[c0])
							{
								temp_color = 0;

								ppu_Pixel_Color_R[c0] = ppu_Pixel_Color_1[c0];
								ppu_BG_Has_Pixel_R[c0] = ppu_BG_Has_Pixel_1[c0];

								if (ppu_BG_Pal_Size[c0])
								{
									if ((ppu_Scroll_Cycle[c0] & 7) == 0)
									{
										if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
										{
											temp_color = ppu_Pixel_Color[c0] & 0xFF;
										}
										else
										{
											temp_color = (ppu_Pixel_Color[c0] >> 8) & 0xFF;
										}
									}
									else
									{
										if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
										{
											temp_color = (ppu_Pixel_Color[c0] >> 8) & 0xFF;
										}
										else
										{
											temp_color = ppu_Pixel_Color[c0] & 0xFF;
										}
									}

									temp_color <<= 1;

									ppu_Pixel_Color_1[c0] = temp_color;
									ppu_BG_Has_Pixel_1[c0] = temp_color != 0;
								}
								else
								{
									if ((ppu_Scroll_Cycle[c0] & 0xF) == 0)
									{
										if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
										{
											temp_color = ((ppu_Pixel_Color[c0] >> 8) & 0xF);
										}
										else
										{
											temp_color = ((ppu_Pixel_Color[c0] >> 4) & 0xF);
										}
									}
									else if ((ppu_Scroll_Cycle[c0] & 0xF) == 4)
									{
										if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
										{
											temp_color = ((ppu_Pixel_Color[c0] >> 12) & 0xF);
										}
										else
										{
											temp_color = (ppu_Pixel_Color[c0] & 0xF);
										}
									}
									else if ((ppu_Scroll_Cycle[c0] & 0xF) == 8)
									{
										if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
										{
											temp_color = (ppu_Pixel_Color[c0] & 0xF);
										}
										else
										{
											temp_color = ((ppu_Pixel_Color[c0] >> 12) & 0xF);
										}
									}
									else if ((ppu_Scroll_Cycle[c0] & 0xF) == 12)
									{
										if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
										{
											temp_color = ((ppu_Pixel_Color[c0] >> 4) & 0xF);
										}
										else
										{
											temp_color = ((ppu_Pixel_Color[c0] >> 8) & 0xF);
										}
									}

									temp_color |= (ppu_BG_Effect_Byte[c0] & 0xF0);

									temp_color <<= 1;

									ppu_Pixel_Color_1[c0] = temp_color;
									ppu_BG_Has_Pixel_1[c0] = ((temp_color & 0x1F) != 0);
								}
							}
						}
						break;

					case 1:
						for (int c1 = 0; c1 < 2; c1++)
						{
							if (ppu_BG_On[c1])
							{
								temp_color = 0;

								ppu_Pixel_Color_R[c1] = ppu_Pixel_Color_1[c1];
								ppu_BG_Has_Pixel_R[c1] = ppu_BG_Has_Pixel_1[c1];

								if (ppu_BG_Pal_Size[c1])
								{
									if ((ppu_Scroll_Cycle[c1] & 7) == 0)
									{
										if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
										{
											temp_color = ppu_Pixel_Color[c1] & 0xFF;
										}
										else
										{
											temp_color = (ppu_Pixel_Color[c1] >> 8) & 0xFF;
										}
									}
									else
									{
										if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
										{
											temp_color = (ppu_Pixel_Color[c1] >> 8) & 0xFF;
										}
										else
										{
											temp_color = ppu_Pixel_Color[c1] & 0xFF;
										}
									}

									temp_color <<= 1;

									ppu_Pixel_Color_1[c1] = temp_color;
									ppu_BG_Has_Pixel_1[c1] = temp_color != 0;
								}
								else
								{
									if ((ppu_Scroll_Cycle[c1] & 0xF) == 0)
									{
										if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
										{
											temp_color = ((ppu_Pixel_Color[c1] >> 8) & 0xF);
										}
										else
										{
											temp_color = ((ppu_Pixel_Color[c1] >> 4) & 0xF);
										}
									}
									else if ((ppu_Scroll_Cycle[c1] & 0xF) == 4)
									{
										if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
										{
											temp_color = ((ppu_Pixel_Color[c1] >> 12) & 0xF);
										}
										else
										{
											temp_color = (ppu_Pixel_Color[c1] & 0xF);
										}
									}
									else if ((ppu_Scroll_Cycle[c1] & 0xF) == 8)
									{
										if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
										{
											temp_color = (ppu_Pixel_Color[c1] & 0xF);
										}
										else
										{
											temp_color = ((ppu_Pixel_Color[c1] >> 12) & 0xF);
										}
									}
									else if ((ppu_Scroll_Cycle[c1] & 0xF) == 12)
									{
										if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
										{
											temp_color = ((ppu_Pixel_Color[c1] >> 4) & 0xF);
										}
										else
										{
											temp_color = ((ppu_Pixel_Color[c1] >> 8) & 0xF);
										}
									}

									temp_color |= (ppu_BG_Effect_Byte[c1] & 0xF0);

									temp_color <<= 1;

									ppu_Pixel_Color_1[c1] = temp_color;
									ppu_BG_Has_Pixel_1[c1] = ((temp_color & 0x1F) != 0);
								}
							}
						}

						ppu_Pixel_Color_R[2] = ppu_Pixel_Color_1[2];
						ppu_Pixel_Color_1[2] = ppu_Pixel_Color_2[2];
						ppu_Pixel_Color_2[2] = ppu_Pixel_Color[2];

						ppu_BG_Has_Pixel_R[2] = ppu_BG_Has_Pixel_1[2];
						ppu_BG_Has_Pixel_1[2] = ppu_BG_Has_Pixel_2[2];
						ppu_BG_Has_Pixel_2[2] = ppu_BG_Has_Pixel[2];
						break;

					case 2:
						ppu_Pixel_Color_R[2] = ppu_Pixel_Color_1[2];
						ppu_Pixel_Color_1[2] = ppu_Pixel_Color_2[2];
						ppu_Pixel_Color_2[2] = ppu_Pixel_Color[2];

						ppu_Pixel_Color_R[3] = ppu_Pixel_Color_1[3];
						ppu_Pixel_Color_1[3] = ppu_Pixel_Color_2[3];
						ppu_Pixel_Color_2[3] = ppu_Pixel_Color[3];

						ppu_BG_Has_Pixel_R[2] = ppu_BG_Has_Pixel_1[2];
						ppu_BG_Has_Pixel_1[2] = ppu_BG_Has_Pixel_2[2];
						ppu_BG_Has_Pixel_2[2] = ppu_BG_Has_Pixel[2];

						ppu_BG_Has_Pixel_R[3] = ppu_BG_Has_Pixel_1[3];
						ppu_BG_Has_Pixel_1[3] = ppu_BG_Has_Pixel_2[3];
						ppu_BG_Has_Pixel_2[3] = ppu_BG_Has_Pixel[3];
						break;

					case 3:
					case 4:
					case 5:
						ppu_Pixel_Color_R[2] = ppu_Pixel_Color_1[2];
						ppu_Pixel_Color_1[2] = ppu_Pixel_Color_2[2];
						ppu_Pixel_Color_2[2] = ppu_Pixel_Color[2];

						ppu_BG_Has_Pixel_R[2] = ppu_BG_Has_Pixel_1[2];
						ppu_BG_Has_Pixel_1[2] = ppu_BG_Has_Pixel_2[2];
						ppu_BG_Has_Pixel_2[2] = ppu_BG_Has_Pixel[2];
						break;
					}
				}

				// render
				if (ppu_Cycle >= 44)
				{
					// determine what BG pixels will actually be rendered
					if ((ppu_Cycle & 3) == 0)
					{
						cur_layer_priority = 4;
						second_layer_priority = 4;

						cur_BG_layer = 5;  // convenient to set to 5 for special effects
						second_BG_layer = 5;

						Is_Outside = true;
						OBJ_Go = false;
						Color_FX_Go = false;

						ppu_Fetch_BG = true;

						ppu_Brighten_Final_Pixel = false;
						ppu_Blend_Final_Pixel = false;

						OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_draw + ppu_Display_Cycle] && ppu_OBJ_On;

						BG_Go_Disp[0] = ppu_BG_On[0];
						BG_Go_Disp[1] = ppu_BG_On[1];
						BG_Go_Disp[2] = ppu_BG_On[2];
						BG_Go_Disp[3] = ppu_BG_On[3];

						ppu_BG_Pixel_F = 0;
						ppu_BG_Pixel_S = 0;

						ppu_Fetch_Target_1 = false;
						ppu_Fetch_Target_2 = false;

						// Check enabled pixels
						if (ppu_Any_Window_On)
						{
							if (ppu_WIN0_On && (((ppu_Display_Cycle - ppu_WIN0_Left) & 0xFF) < ((ppu_WIN0_Right - ppu_WIN0_Left) & 0xFF)) &&
								(((ppu_LY - ppu_WIN0_Top) & 0xFF) < ((ppu_WIN0_Bot - ppu_WIN0_Top) & 0xFF)))
							{
								Is_Outside = false;

								for (int w0 = 0; w0 < 4; w0++) { BG_Go_Disp[w0] &= ppu_WIN0_BG_En[w0]; }

								OBJ_Go = ppu_WIN0_OBJ_En && OBJ_Has_Pixel;
								Color_FX_Go = ppu_WIN0_Color_En;
							}
							else if (ppu_WIN1_On && (((ppu_Display_Cycle - ppu_WIN1_Left) & 0xFF) < ((ppu_WIN1_Right - ppu_WIN1_Left) & 0xFF)) &&
								(((ppu_LY - ppu_WIN1_Top) & 0xFF) < ((ppu_WIN1_Bot - ppu_WIN1_Top) & 0xFF)))
							{
								Is_Outside = false;

								for (int w1 = 0; w1 < 4; w1++) { BG_Go_Disp[w1] &= ppu_WIN1_BG_En[w1]; }

								OBJ_Go = ppu_WIN1_OBJ_En && OBJ_Has_Pixel;
								Color_FX_Go = ppu_WIN1_Color_En;
							}
							else if (ppu_OBJ_WIN && ppu_Sprite_Object_Window[ppu_Sprite_ofst_draw + ppu_Display_Cycle])
							{
								Is_Outside = false;

								for (int ob = 0; ob < 4; ob++) { BG_Go_Disp[ob] &= ppu_OBJ_BG_En[ob]; }

								OBJ_Go = ppu_OBJ_OBJ_En && OBJ_Has_Pixel;
								Color_FX_Go = ppu_OBJ_Color_En;
							}

							if (Is_Outside)
							{
								for (int outs = 0; outs < 4; outs++) { BG_Go_Disp[outs] &= ppu_OUT_BG_En[outs]; }

								OBJ_Go = ppu_OUT_OBJ_En && OBJ_Has_Pixel;
								Color_FX_Go = ppu_OUT_Color_En;
							}
						}
						else
						{
							OBJ_Go = OBJ_Has_Pixel;
							Color_FX_Go = true;
						}

						switch (ppu_BG_Mode)
						{
						case 0:
							for (int c0 = 0; c0 < 4; c0++)
							{
								if (BG_Go_Disp[c0])
								{
									if (ppu_BG_Has_Pixel_R[c0])
									{
										if (ppu_BG_Priority[c0] < cur_layer_priority)
										{
											ppu_BG_Pixel_S = ppu_BG_Pixel_F;
											second_BG_layer = cur_BG_layer;
											second_layer_priority = cur_layer_priority;

											ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[c0];
											cur_BG_layer = c0;
											cur_layer_priority = ppu_BG_Priority[c0];
										}
										else if (ppu_BG_Priority[c0] < second_layer_priority)
										{
											ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[c0];
											second_BG_layer = c0;
											second_layer_priority = ppu_BG_Priority[c0];
										}
									}
								}
							}
							break;

						case 1:
							for (int c1 = 0; c1 < 2; c1++)
							{
								if (BG_Go_Disp[c1])
								{
									if (ppu_BG_Has_Pixel_R[c1])
									{
										if (ppu_BG_Priority[c1] < cur_layer_priority)
										{
											ppu_BG_Pixel_S = ppu_BG_Pixel_F;
											second_BG_layer = cur_BG_layer;
											second_layer_priority = cur_layer_priority;

											ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[c1];
											cur_BG_layer = c1;
											cur_layer_priority = ppu_BG_Priority[c1];
										}
										else if (ppu_BG_Priority[c1] < second_layer_priority)
										{
											ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[c1];
											second_BG_layer = c1;
											second_layer_priority = ppu_BG_Priority[c1];
										}
									}
								}
							}

							if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
							{
								if (ppu_BG_Priority[2] < cur_layer_priority)
								{
									ppu_BG_Pixel_S = ppu_BG_Pixel_F;
									second_BG_layer = cur_BG_layer;
									second_layer_priority = cur_layer_priority;

									ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
									cur_BG_layer = 2;
									cur_layer_priority = ppu_BG_Priority[2];
								}
								else if (ppu_BG_Priority[2] < second_layer_priority)
								{
									ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[2];
									second_BG_layer = 2;
									second_layer_priority = ppu_BG_Priority[2];
								}
							}
							break;

						case 2:
							for (int c2 = 2; c2 < 4; c2++)
							{
								if (BG_Go_Disp[c2] && ppu_BG_Has_Pixel_R[c2])
								{
									if (ppu_BG_Priority[c2] < cur_layer_priority)
									{
										ppu_BG_Pixel_S = ppu_BG_Pixel_F;
										second_BG_layer = cur_BG_layer;
										second_layer_priority = cur_layer_priority;

										ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[c2];
										cur_BG_layer = c2;
										cur_layer_priority = ppu_BG_Priority[c2];
									}
									else if (ppu_BG_Priority[c2] < second_layer_priority)
									{
										ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[c2];
										second_BG_layer = c2;
										second_layer_priority = ppu_BG_Priority[c2];
									}
								}
							}
							break;

						case 3:
							// Direct Bitmap only, BG2
							if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
							{
								// no transparency possible
								cur_BG_layer = 2;
								cur_layer_priority = ppu_BG_Priority[2];

								ppu_Fetch_BG = false;

								ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
							}
							break;

						case 4:
							// bitmaps, only BG2
							if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
							{
								// no transparency possible
								cur_BG_layer = 2;
								cur_layer_priority = ppu_BG_Priority[2];
								ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
							}
							break;

						case 5:
							// bitmaps, only BG2
							if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
							{
								// no transparency possible
								cur_BG_layer = 2;
								cur_layer_priority = ppu_BG_Priority[2];

								ppu_Fetch_BG = false;

								ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
							}
							break;

						case 6:
							// invalid
							break;

						case 7:
							// invalid
							break;
						}

						// determine final pixel color, based on sprites and special effects
						if (OBJ_Go)
						{
							spr_pixel = ppu_Sprite_Pixels[ppu_Sprite_ofst_draw + ppu_Display_Cycle];

							//Console.WriteLine(ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle] + " " + cur_layer_priority + " " + cur_BG_layer + " " + ppu_LY);
							// sprite pixel available, check ordering
							if (ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle] <= cur_layer_priority)
							{
								// sprite pixel has higher priority than BG pixel
								if (ppu_Sprite_Semi_Transparent[ppu_Sprite_ofst_draw + ppu_Display_Cycle])
								{
									// semi transparent pixels with highest priority always enable alpha blending if possible, even if otherwise disabled.
									// alpha blend if possible
									if ((ppu_Special_FX & (1 << (cur_BG_layer + 8))) != 0)
									{
										// Alpha blending Sprite - BG
										ppu_Final_Pixel = spr_pixel;
										ppu_Blend_Pixel = ppu_BG_Pixel_F;

										ppu_Fetch_Target_1 = true;
										ppu_Fetch_Target_2 = ppu_Fetch_BG;

										ppu_Blend_Final_Pixel = true;
									}
									else
									{
										ppu_Final_Pixel = spr_pixel;

										ppu_Fetch_Target_1 = true;

										if ((ppu_SFX_mode >= 2) && ppu_SFX_OBJ_Target_1 && Color_FX_Go)
										{
											ppu_Brighten_Final_Pixel = true;
										}
									}
								}
								else if (Color_FX_Go)
								{
									// Alpha blending as normal if enabled
									if (ppu_SFX_mode != 1)
									{
										ppu_Final_Pixel = spr_pixel;

										ppu_Fetch_Target_1 = true;

										if ((ppu_SFX_mode != 0) && ppu_SFX_OBJ_Target_1)
										{
											ppu_Brighten_Final_Pixel = true;
										}
									}
									else
									{
										// alpha blend if possible
										if (((ppu_Special_FX & (1 << (cur_BG_layer + 8))) != 0) && ppu_SFX_OBJ_Target_1)
										{
											// Alpha blending Sprite - BG
											ppu_Final_Pixel = spr_pixel;
											ppu_Blend_Pixel = ppu_BG_Pixel_F;

											ppu_Fetch_Target_1 = true;
											ppu_Fetch_Target_2 = ppu_Fetch_BG;

											ppu_Blend_Final_Pixel = true;
										}
										else
										{
											ppu_Final_Pixel = spr_pixel;

											ppu_Fetch_Target_1 = true;
										}
									}
								}
								else
								{
									ppu_Final_Pixel = spr_pixel;

									ppu_Fetch_Target_1 = true;
								}
							}
							else
							{
								if (Color_FX_Go)
								{
									// BG pixel has higher priority than sprite pixel
									if (ppu_SFX_mode != 1)
									{
										ppu_Final_Pixel = ppu_BG_Pixel_F;

										ppu_Fetch_Target_1 = ppu_Fetch_BG;

										if ((ppu_SFX_mode != 0) && ((ppu_Special_FX & (1 << cur_BG_layer)) != 0))
										{
											ppu_Brighten_Final_Pixel = true;
										}
									}
									else
									{
										// alpha blend if possible
										// check if another bg layer has a higher priority pixel than the sprite
										if ((ppu_Special_FX & (1 << cur_BG_layer)) != 0)
										{
											if ((second_layer_priority < ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle]) && ((ppu_Special_FX & (1 << second_BG_layer)) != 0))
											{
												// Alpha blending BG - BG
												ppu_Final_Pixel = ppu_BG_Pixel_F;
												ppu_Blend_Pixel = ppu_BG_Pixel_S;

												ppu_Fetch_Target_1 = ppu_Fetch_BG;
												ppu_Fetch_Target_2 = true;

												ppu_Blend_Final_Pixel = true;
											}
											else if ((ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle] <= second_layer_priority) && ppu_SFX_OBJ_Target_2)
											{
												// Alpha blending BG - Sprite
												ppu_Final_Pixel = ppu_BG_Pixel_F;
												ppu_Blend_Pixel = spr_pixel;

												ppu_Fetch_Target_1 = ppu_Fetch_BG;
												ppu_Fetch_Target_2 = true;

												ppu_Blend_Final_Pixel = true;
											}
											else
											{
												ppu_Final_Pixel = ppu_BG_Pixel_F;

												ppu_Fetch_Target_1 = ppu_Fetch_BG;
											}
										}
										else
										{
											ppu_Final_Pixel = ppu_BG_Pixel_F;

											ppu_Fetch_Target_1 = ppu_Fetch_BG;
										}
									}
								}
								else
								{
									ppu_Final_Pixel = ppu_BG_Pixel_F;

									ppu_Fetch_Target_1 = ppu_Fetch_BG;
								}
							}
						}
						else
						{
							// only BG pixels available, check brightness and 1st target for special effects
							if (Color_FX_Go)
							{
								if (ppu_SFX_mode != 1)
								{
									ppu_Final_Pixel = ppu_BG_Pixel_F;

									ppu_Fetch_Target_1 = ppu_Fetch_BG;

									if ((ppu_SFX_mode >= 2) && ((ppu_Special_FX & (1 << cur_BG_layer)) != 0))
									{
										ppu_Brighten_Final_Pixel = true;
									}
								}
								else
								{
									// alpha blend if possible
									// check if the top two layers are targets for blending
									if (((ppu_Special_FX & (1 << cur_BG_layer)) != 0) && ((ppu_Special_FX & (1 << (second_BG_layer + 8))) != 0))
									{
										// Alpha blending BG - BG
										ppu_Final_Pixel = ppu_BG_Pixel_F;
										ppu_Blend_Pixel = ppu_BG_Pixel_S;

										ppu_Fetch_Target_1 = ppu_Fetch_BG;
										ppu_Fetch_Target_2 = true;

										ppu_Blend_Final_Pixel = true;
									}
									else
									{
										ppu_Final_Pixel = ppu_BG_Pixel_F;

										ppu_Fetch_Target_1 = ppu_Fetch_BG;
									}
								}
							}
							else
							{
								ppu_Final_Pixel = ppu_BG_Pixel_F;

								ppu_Fetch_Target_1 = ppu_Fetch_BG;
							}
						}
					}
					else if ((ppu_Cycle & 3) == 1)
					{
						if (ppu_Fetch_Target_1)
						{
							ppu_PALRAM_Access = true;

							ppu_Final_Pixel = (uint32_t)(PALRAM_16[ppu_Final_Pixel >> 1]);

							if (ppu_PALRAM_In_Use)
							{
								if (ppu_PALRAM_Access)
								{
									cpu_Fetch_Wait += 1;
								}
							}
						}

						ppu_Final_Pixel = (uint32_t)(0xFF000000 |
							((ppu_Final_Pixel & 0x1F) << 19) |
							((ppu_Final_Pixel & 0x3E0) << 6) |
							((ppu_Final_Pixel & 0x7C00) >> 7));
					}
					else if ((ppu_Cycle & 3) == 3)
					{
						if (ppu_Fetch_Target_2)
						{
							ppu_PALRAM_Access = true;

							ppu_Blend_Pixel = (uint32_t)(PALRAM_16[ppu_Blend_Pixel >> 1]);

							if (ppu_PALRAM_In_Use)
							{
								if (ppu_PALRAM_Access)
								{
									cpu_Fetch_Wait += 1;
								}
							}
						}

						if (ppu_Brighten_Final_Pixel)
						{
							R = (ppu_Final_Pixel >> 3) & 0x1F;
							G = (ppu_Final_Pixel >> 11) & 0x1F;
							B = (ppu_Final_Pixel >> 19) & 0x1F;

							if (ppu_SFX_mode == 2)
							{
								R = (uint32_t)(R + (((31 - R) * ppu_SFX_BRT_Num) >> 4));
								G = (uint32_t)(G + (((31 - G) * ppu_SFX_BRT_Num) >> 4));
								B = (uint32_t)(B + (((31 - B) * ppu_SFX_BRT_Num) >> 4));
							}
							else
							{
								R = (uint32_t)(R - ((R * ppu_SFX_BRT_Num) >> 4));
								G = (uint32_t)(G - ((G * ppu_SFX_BRT_Num) >> 4));
								B = (uint32_t)(B - ((B * ppu_SFX_BRT_Num) >> 4));
							}

							ppu_Final_Pixel = (uint32_t)(0xFF000000 |
								(R << 3) |
								(G << 11) |
								(B << 19));
						}
						else if (ppu_Blend_Final_Pixel)
						{
							ppu_Blend_Pixel = (uint32_t)(0xFF000000 |
								((ppu_Blend_Pixel & 0x1F) << 19) |
								((ppu_Blend_Pixel & 0x3E0) << 6) |
								((ppu_Blend_Pixel & 0x7C00) >> 7));

							R = (ppu_Final_Pixel >> 3) & 0x1F;
							G = (ppu_Final_Pixel >> 11) & 0x1F;
							B = (ppu_Final_Pixel >> 19) & 0x1F;

							R2 = (ppu_Blend_Pixel >> 3) & 0x1F;
							G2 = (ppu_Blend_Pixel >> 11) & 0x1F;
							B2 = (ppu_Blend_Pixel >> 19) & 0x1F;

							R = (uint32_t)(((R * ppu_SFX_Alpha_Num_1) >> 4) + ((R2 * ppu_SFX_Alpha_Num_2) >> 4));
							G = (uint32_t)(((G * ppu_SFX_Alpha_Num_1) >> 4) + ((G2 * ppu_SFX_Alpha_Num_2) >> 4));
							B = (uint32_t)(((B * ppu_SFX_Alpha_Num_1) >> 4) + ((B2 * ppu_SFX_Alpha_Num_2) >> 4));

							if (R > 31) { R = 31; }
							if (G > 31) { G = 31; }
							if (B > 31) { B = 31; }

							ppu_Final_Pixel = (uint32_t)(0xFF000000 |
								(R << 3) |
								(G << 11) |
								(B << 19));
						}

						// push pixel to display
						video_buffer[ppu_Display_Cycle + ppu_LY * 240] = ppu_Final_Pixel;

						ppu_Display_Cycle += 1;

						if (ppu_Display_Cycle == 240)
						{
							ppu_PAL_Rendering_Complete = true;

							ppu_Rendering_Complete = true;

							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
						}
					}
				}
			}

			switch (ppu_BG_Mode)
			{
			case 0:
				for (int c0 = 0; c0 < 4; c0++)
				{
					if (!ppu_BG_Rendering_Complete[c0])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[c0]))
						{
							if ((ppu_Scroll_Cycle[c0] & 31) == c0)
							{
								// calculate scrolling
								if (ppu_BG_Mosaic[c0])
								{
									ppu_X_RS = (int)((ppu_MOS_BG_X[ppu_Fetch_Count[c0] << 3] + ppu_BG_X_Latch[c0]) & 0x1FF);
									ppu_Y_RS = (int)((ppu_MOS_BG_Y[ppu_LY] + ppu_BG_Y_Latch[c0]) & 0x1FF);
								}
								else
								{
									ppu_X_RS = (int)(((ppu_Fetch_Count[c0] << 3) + ppu_BG_X_Latch[c0]) & 0x1FF);
									ppu_Y_RS = (int)((ppu_LY + ppu_BG_Y_Latch[c0]) & 0x1FF);
								}

								// always wrap around, this means pixels will always be in a valid range
								ppu_X_RS &= (BG_Scale_X[c0] - 1);
								ppu_Y_RS &= (BG_Scale_Y[c0] - 1);

								ppu_Y_Flip_Ofst[c0] = ppu_Y_RS & 7;

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								VRAM_ofst_X = ppu_X_RS >> 3;
								VRAM_ofst_Y = ppu_Y_RS >> 3;

								Screen_Offset = 0;

								if (VRAM_ofst_X > 31)
								{
									if (VRAM_ofst_Y > 31)
									{
										Screen_Offset = 6 * 1024;
									}
									else
									{
										Screen_Offset = 2 * 1024;
									}
								}
								else if (VRAM_ofst_Y > 31)
								{
									if (ppu_BG_Screen_Size[c0] == 2)
									{
										Screen_Offset = 2 * 1024;
									}
									else
									{
										Screen_Offset = 4 * 1024;
									}
								}

								VRAM_ofst_X &= 31;
								VRAM_ofst_Y &= 31;

								ppu_Tile_Addr[c0] = ppu_BG_Screen_Base[c0] + Screen_Offset + VRAM_ofst_Y * BG_Num_Tiles[c0] * 2 + VRAM_ofst_X * 2;

								ppu_BG_Effect_Byte_New[c0] = VRAM[ppu_Tile_Addr[c0] + 1];

								ppu_Tile_Addr[c0] = VRAM_16[ppu_Tile_Addr[c0] >> 1] & 0x3FF;
							}
							else if (((ppu_Scroll_Cycle[c0] & 31) == (c0 + 4)) || ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 20)))
							{
								// this access will always occur
								ppu_Set_VRAM_Access_True();

								// this update happens here so that rendering isn't effected further up
								ppu_BG_Effect_Byte[c0] = ppu_BG_Effect_Byte_New[c0];

								if (ppu_BG_Pal_Size[c0])
								{
									temp_addr = ppu_Tile_Addr[c0];

									temp_addr = temp_addr * 64 + ppu_BG_Char_Base[c0];

									if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 4))
										{
											temp_addr += 0;
										}
										else
										{
											temp_addr += 4;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 4))
										{
											temp_addr += 6;
										}
										else
										{
											temp_addr += 2;
										}
									}

									if ((ppu_BG_Effect_Byte[c0] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[c0] * 8;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[c0]) * 8;
									}

									ppu_Pixel_Color[c0] = VRAM_16[temp_addr >> 1];
								}
								else
								{
									temp_addr = ppu_Tile_Addr[c0];

									temp_addr = temp_addr * 32 + ppu_BG_Char_Base[c0];

									if ((ppu_BG_Effect_Byte[c0] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[c0] * 4;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[c0]) * 4;
									}

									if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 4))
										{
											temp_addr += 0;
										}
										else
										{
											temp_addr += 2;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 4))
										{
											temp_addr += 2;
										}
										else
										{
											temp_addr += 0;
										}
									}

									ppu_Pixel_Color[c0] = VRAM_16[temp_addr >> 1];
								}
							}
							else if (((ppu_Scroll_Cycle[c0] & 31) == (c0 + 12)) || ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 28)))
							{
								// this access will only occur in 256color mode
								if (ppu_BG_Pal_Size[c0])
								{
									ppu_Set_VRAM_Access_True();

									temp_addr = ppu_Tile_Addr[c0];

									temp_addr = temp_addr * 64 + ppu_BG_Char_Base[c0];

									if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 12))
										{
											temp_addr += 2;
										}
										else
										{
											temp_addr += 6;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 12))
										{
											temp_addr += 4;
										}
										else
										{
											temp_addr += 0;
										}
									}

									if ((ppu_BG_Effect_Byte[c0] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[c0] * 8;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[c0]) * 8;
									}

									ppu_Pixel_Color[c0] = VRAM_16[temp_addr >> 1];
								}

								if ((ppu_Scroll_Cycle[c0] & 31) == (c0 + 28))
								{
									ppu_Fetch_Count[c0] += 1;

									if (ppu_Fetch_Count[c0] == 31)
									{
										ppu_BG_Rendering_Complete[c0] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}

							ppu_Scroll_Cycle[c0] += 1;
						}
					}
				}
				break;

			case 1:
				for (int c1 = 0; c1 < 2; c1++)
				{
					if (!ppu_BG_Rendering_Complete[c1])
					{
						if (ppu_Cycle >= ppu_BG_Start_Time[c1])
						{
							if ((ppu_Scroll_Cycle[c1] & 31) == c1)
							{
								// calculate scrolling
								if (ppu_BG_Mosaic[c1])
								{
									ppu_X_RS = (int)((ppu_MOS_BG_X[ppu_Fetch_Count[c1] << 3] + ppu_BG_X_Latch[c1]) & 0x1FF);
									ppu_Y_RS = (int)((ppu_MOS_BG_Y[ppu_LY] + ppu_BG_Y_Latch[c1]) & 0x1FF);
								}
								else
								{
									ppu_X_RS = (int)(((ppu_Fetch_Count[c1] << 3) + ppu_BG_X_Latch[c1]) & 0x1FF);
									ppu_Y_RS = (int)((ppu_LY + ppu_BG_Y_Latch[c1]) & 0x1FF);
								}

								// always wrap around, so pixel is always in range
								ppu_X_RS &= (BG_Scale_X[c1] - 1);
								ppu_Y_RS &= (BG_Scale_Y[c1] - 1);

								ppu_Y_Flip_Ofst[c1] = ppu_Y_RS & 7;

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								VRAM_ofst_X = ppu_X_RS >> 3;
								VRAM_ofst_Y = ppu_Y_RS >> 3;

								Screen_Offset = 0;

								if (VRAM_ofst_X > 31)
								{
									if (VRAM_ofst_Y > 31)
									{
										Screen_Offset = 6 * 1024;
									}
									else
									{
										Screen_Offset = 2 * 1024;
									}
								}
								else if (VRAM_ofst_Y > 31)
								{
									if (ppu_BG_Screen_Size[c1] == 2)
									{
										Screen_Offset = 2 * 1024;
									}
									else
									{
										Screen_Offset = 4 * 1024;
									}
								}

								VRAM_ofst_X &= 31;
								VRAM_ofst_Y &= 31;

								ppu_Tile_Addr[c1] = ppu_BG_Screen_Base[c1] + Screen_Offset + VRAM_ofst_Y * BG_Num_Tiles[c1] * 2 + VRAM_ofst_X * 2;

								ppu_BG_Effect_Byte_New[c1] = VRAM[ppu_Tile_Addr[c1] + 1];

								ppu_Tile_Addr[c1] = VRAM_16[ppu_Tile_Addr[c1] >> 1] & 0x3FF;
							}
							else if (((ppu_Scroll_Cycle[c1] & 31) == (c1 + 4)) || ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 20)))
							{
								// this access will always occur
								ppu_Set_VRAM_Access_True();

								// this update happens here so that rendering isn't effected further up
								ppu_BG_Effect_Byte[c1] = ppu_BG_Effect_Byte_New[c1];

								if (ppu_BG_Pal_Size[c1])
								{
									temp_addr = ppu_Tile_Addr[c1];

									temp_addr = temp_addr * 64 + ppu_BG_Char_Base[c1];

									if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 4))
										{
											temp_addr += 0;
										}
										else
										{
											temp_addr += 4;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 4))
										{
											temp_addr += 6;
										}
										else
										{
											temp_addr += 2;
										}
									}

									if ((ppu_BG_Effect_Byte[c1] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[c1] * 8;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[c1]) * 8;
									}

									ppu_Pixel_Color[c1] = VRAM_16[temp_addr >> 1];;
								}
								else
								{
									temp_addr = ppu_Tile_Addr[c1];

									temp_addr = temp_addr * 32 + ppu_BG_Char_Base[c1];

									if ((ppu_BG_Effect_Byte[c1] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[c1] * 4;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[c1]) * 4;
									}

									if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 4))
										{
											temp_addr += 0;
										}
										else
										{
											temp_addr += 2;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 4))
										{
											temp_addr += 2;
										}
										else
										{
											temp_addr += 0;
										}
									}

									ppu_Pixel_Color[c1] = VRAM_16[temp_addr >> 1];
								}
							}
							else if (((ppu_Scroll_Cycle[c1] & 31) == (c1 + 12)) || ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 28)))
							{
								// this access will only occur in 256color mode
								if (ppu_BG_Pal_Size[c1])
								{
									ppu_Set_VRAM_Access_True();

									temp_addr = ppu_Tile_Addr[c1];

									temp_addr = temp_addr * 64 + ppu_BG_Char_Base[c1];

									if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 12))
										{
											temp_addr += 2;
										}
										else
										{
											temp_addr += 6;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 12))
										{
											temp_addr += 4;
										}
										else
										{
											temp_addr += 0;
										}
									}

									if ((ppu_BG_Effect_Byte[c1] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[c1] * 8;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[c1]) * 8;
									}

									ppu_Pixel_Color[c1] = VRAM_16[temp_addr >> 1];
								}

								if ((ppu_Scroll_Cycle[c1] & 31) == (c1 + 28))
								{
									ppu_Fetch_Count[c1] += 1;

									if (ppu_Fetch_Count[c1] == 31)
									{
										ppu_BG_Rendering_Complete[c1] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}

							ppu_Scroll_Cycle[c1] += 1;
						}
					}
				}

				if (!ppu_BG_Rendering_Complete[2])
				{
					if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
					{
						if ((ppu_Cycle & 3) == 2)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// calculate rotation and scaling
								if (ppu_BG_Mosaic[2])
								{
									cur_y = -ppu_MOS_BG_Y[ppu_LY - ppu_ROT_REF_LY[2]];
									cur_x = ppu_MOS_BG_X[ppu_Fetch_Count[2]];
								}
								else
								{
									cur_y = -(ppu_LY - ppu_ROT_REF_LY[2]);
									cur_x = ppu_Fetch_Count[2];
								}

								sol_x = ppu_F_Rot_A_2 * cur_x - ppu_F_Rot_B_2 * cur_y;
								sol_y = -ppu_F_Rot_C_2 * cur_x + ppu_F_Rot_D_2 * cur_y;

								ppu_X_RS = (uint16_t)floor(sol_x + ppu_F_Ref_X_2);
								ppu_Y_RS = (uint16_t)floor(-(sol_y - ppu_F_Ref_Y_2));

								// adjust if wraparound is enabled
								if (ppu_BG_Overflow[2])
								{
									ppu_X_RS &= (BG_Scale_X[2] - 1);
									ppu_Y_RS &= (BG_Scale_Y[2] - 1);
								}

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								// determine if pixel is in valid range, and pick out color if so
								if ((ppu_X_RS >= 0) && (ppu_Y_RS >= 0) && (ppu_X_RS < BG_Scale_X[2]) && (ppu_Y_RS < BG_Scale_Y[2]))
								{
									VRAM_ofst_X = ppu_X_RS >> 3;
									VRAM_ofst_Y = ppu_Y_RS >> 3;

									ppu_Tile_Addr[2] = ppu_BG_Screen_Base[2] + VRAM_ofst_Y * BG_Num_Tiles[2] + VRAM_ofst_X;

									ppu_Tile_Addr[2] = VRAM[ppu_Tile_Addr[2]];

									ppu_BG_Has_Pixel[2] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								if (ppu_Fetch_Count[2] == 244)
								{
									ppu_BG_Rendering_Complete[2] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
						else if ((ppu_Cycle & 3) == 3)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								if (ppu_BG_Has_Pixel[2])
								{
									ppu_Tile_Addr[2] = ppu_Tile_Addr[2] * 64 + ppu_BG_Char_Base[2];

									ppu_Tile_Addr[2] = ppu_Tile_Addr[2] + (ppu_X_RS & 7) + (ppu_Y_RS & 7) * 8;

									ppu_Pixel_Color[2] = (VRAM[ppu_Tile_Addr[2]] << 1);

									if (ppu_Pixel_Color[2] != 0)
									{
										ppu_BG_Has_Pixel[2] = true;
									}
									else
									{
										ppu_BG_Has_Pixel[2] = false;
									}
								}

								ppu_Fetch_Count[2] += 1;
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								ppu_Fetch_Count[2] += 1;
							}
						}
					}
				}
				break;

			case 2:
				if (!ppu_BG_Rendering_Complete[2])
				{
					if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
					{
						if ((ppu_Cycle & 3) == 2)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// calculate rotation and scaling
								if (ppu_BG_Mosaic[2])
								{
									cur_y = -ppu_MOS_BG_Y[ppu_LY - ppu_ROT_REF_LY[2]];
									cur_x = ppu_MOS_BG_X[ppu_Fetch_Count[2]];
								}
								else
								{
									cur_y = -(ppu_LY - ppu_ROT_REF_LY[2]);
									cur_x = ppu_Fetch_Count[2];
								}

								sol_x = ppu_F_Rot_A_2 * cur_x - ppu_F_Rot_B_2 * cur_y;
								sol_y = -ppu_F_Rot_C_2 * cur_x + ppu_F_Rot_D_2 * cur_y;

								ppu_X_RS = (uint16_t)floor(sol_x + ppu_F_Ref_X_2);
								ppu_Y_RS = (uint16_t)floor(-(sol_y - ppu_F_Ref_Y_2));

								// adjust if wraparound is enabled
								if (ppu_BG_Overflow[2])
								{
									ppu_X_RS &= (BG_Scale_X[2] - 1);
									ppu_Y_RS &= (BG_Scale_Y[2] - 1);
								}

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								// determine if pixel is in valid range, and pick out color if so
								if ((ppu_X_RS >= 0) && (ppu_Y_RS >= 0) && (ppu_X_RS < BG_Scale_X[2]) && (ppu_Y_RS < BG_Scale_Y[2]))
								{
									VRAM_ofst_X = ppu_X_RS >> 3;
									VRAM_ofst_Y = ppu_Y_RS >> 3;

									ppu_Tile_Addr[2] = ppu_BG_Screen_Base[2] + VRAM_ofst_Y * BG_Num_Tiles[2] + VRAM_ofst_X;

									ppu_Tile_Addr[2] = VRAM[ppu_Tile_Addr[2]];

									ppu_BG_Has_Pixel[2] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								if (ppu_Fetch_Count[2] == 244)
								{
									ppu_BG_Rendering_Complete[2] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
						else if ((ppu_Cycle & 3) == 3)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								if (ppu_BG_Has_Pixel[2])
								{
									ppu_Tile_Addr[2] = ppu_Tile_Addr[2] * 64 + ppu_BG_Char_Base[2];

									ppu_Tile_Addr[2] = ppu_Tile_Addr[2] + (ppu_X_RS & 7) + (ppu_Y_RS & 7) * 8;

									ppu_Pixel_Color[2] = (VRAM[ppu_Tile_Addr[2]] << 1);

									if (ppu_Pixel_Color[2] != 0)
									{
										ppu_BG_Has_Pixel[2] = true;
									}
									else
									{
										ppu_BG_Has_Pixel[2] = false;
									}
								}

								ppu_Fetch_Count[2] += 1;
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								ppu_Fetch_Count[2] += 1;
							}
						}
					}
				}

				if (!ppu_BG_Rendering_Complete[3])
				{
					if ((ppu_Cycle >= ppu_BG_Start_Time[3]))
					{
						if ((ppu_Cycle & 3) == 0)
						{
							if (ppu_Fetch_Count[3] < 240)
							{
								// calculate rotation and scaling
								if (ppu_BG_Mosaic[3])
								{
									cur_y = -ppu_MOS_BG_Y[ppu_LY - ppu_ROT_REF_LY[3]];
									cur_x = ppu_MOS_BG_X[ppu_Fetch_Count[3]];
								}
								else
								{
									cur_y = -(ppu_LY - ppu_ROT_REF_LY[3]);
									cur_x = ppu_Fetch_Count[3];
								}

								sol_x = ppu_F_Rot_A_3 * cur_x - ppu_F_Rot_B_3 * cur_y;
								sol_y = -ppu_F_Rot_C_3 * cur_x + ppu_F_Rot_D_3 * cur_y;

								ppu_X_RS = (uint16_t)floor(sol_x + ppu_F_Ref_X_3);
								ppu_Y_RS = (uint16_t)floor(-(sol_y - ppu_F_Ref_Y_3));

								// adjust if wraparound is enabled
								if (ppu_BG_Overflow[3])
								{
									ppu_X_RS &= (BG_Scale_X[3] - 1);
									ppu_Y_RS &= (BG_Scale_Y[3] - 1);
								}

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								// determine if pixel is in valid range, and pick out color if so
								if ((ppu_X_RS >= 0) && (ppu_Y_RS >= 0) && (ppu_X_RS < BG_Scale_X[3]) && (ppu_Y_RS < BG_Scale_Y[3]))
								{
									VRAM_ofst_X = ppu_X_RS >> 3;
									VRAM_ofst_Y = ppu_Y_RS >> 3;

									ppu_Tile_Addr[3] = ppu_BG_Screen_Base[3] + VRAM_ofst_Y * BG_Num_Tiles[3] + VRAM_ofst_X;

									ppu_Tile_Addr[3] = VRAM[ppu_Tile_Addr[3]];

									ppu_BG_Has_Pixel[3] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[3] = false;
								}
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[3] = false;

								if (ppu_Fetch_Count[3] == 244)
								{
									ppu_BG_Rendering_Complete[3] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
						else if ((ppu_Cycle & 3) == 1)
						{
							if (ppu_Fetch_Count[3] < 240)
							{
								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								if (ppu_BG_Has_Pixel[3])
								{
									ppu_Tile_Addr[3] = ppu_Tile_Addr[3] * 64 + ppu_BG_Char_Base[3];

									ppu_Tile_Addr[3] = ppu_Tile_Addr[3] + (ppu_X_RS & 7) + (ppu_Y_RS & 7) * 8;

									ppu_Pixel_Color[3] = (VRAM[ppu_Tile_Addr[3]] << 1);

									if (ppu_Pixel_Color[3] != 0)
									{
										ppu_BG_Has_Pixel[3] = true;
									}
									else
									{
										ppu_BG_Has_Pixel[3] = false;
									}
								}

								ppu_Fetch_Count[3] += 1;
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[3] = false;

								ppu_Fetch_Count[3] += 1;
							}
						}
					}
				}
				break;

			case 3:
				// Direct Bitmap only, BG2
				if (!ppu_BG_Rendering_Complete[2])
				{
					if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
					{
						if ((ppu_Cycle & 3) == 3)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// calculate rotation and scaling
								if (ppu_BG_Mosaic[2])
								{
									cur_y = -ppu_MOS_BG_Y[ppu_LY - ppu_ROT_REF_LY[2]];
									cur_x = ppu_MOS_BG_X[ppu_Fetch_Count[2]];
								}
								else
								{
									cur_y = -(ppu_LY - ppu_ROT_REF_LY[2]);
									cur_x = ppu_Fetch_Count[2];
								}

								sol_x = ppu_F_Rot_A_2 * cur_x - ppu_F_Rot_B_2 * cur_y;
								sol_y = -ppu_F_Rot_C_2 * cur_x + ppu_F_Rot_D_2 * cur_y;

								ppu_X_RS = (uint16_t)floor(sol_x + ppu_F_Ref_X_2);
								ppu_Y_RS = (uint16_t)floor(-(sol_y - ppu_F_Ref_Y_2));

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								if ((ppu_X_RS < 240) && (ppu_Y_RS < 160) && (ppu_X_RS >= 0) && (ppu_Y_RS >= 0))
								{
									// pixel color comes direct from VRAM
									int m3_ofst = ppu_X_RS + ppu_Y_RS * 240;

									ppu_Pixel_Color[2] = VRAM_16[m3_ofst];

									ppu_BG_Has_Pixel[2] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								ppu_Fetch_Count[2] += 1;
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								ppu_Fetch_Count[2] += 1;
								if (ppu_Fetch_Count[2] == 243)
								{
									ppu_BG_Rendering_Complete[2] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
					}
				}
				break;

			case 4:
				// bitmaps, only BG2
				if (!ppu_BG_Rendering_Complete[2])
				{
					if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
					{
						if ((ppu_Cycle & 3) == 3)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// calculate rotation and scaling
								if (ppu_BG_Mosaic[2])
								{
									cur_y = -ppu_MOS_BG_Y[ppu_LY - ppu_ROT_REF_LY[2]];
									cur_x = ppu_MOS_BG_X[ppu_Fetch_Count[2]];
								}
								else
								{
									cur_y = -(ppu_LY - ppu_ROT_REF_LY[2]);
									cur_x = ppu_Fetch_Count[2];
								}

								sol_x = ppu_F_Rot_A_2 * cur_x - ppu_F_Rot_B_2 * cur_y;
								sol_y = -ppu_F_Rot_C_2 * cur_x + ppu_F_Rot_D_2 * cur_y;

								ppu_X_RS = (uint16_t)floor(sol_x + ppu_F_Ref_X_2);
								ppu_Y_RS = (uint16_t)floor(-(sol_y - ppu_F_Ref_Y_2));

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								if ((ppu_X_RS < 240) && (ppu_Y_RS < 160) && (ppu_X_RS >= 0) && (ppu_Y_RS >= 0))
								{
									ppu_Pixel_Color[2] = VRAM[ppu_Display_Frame * 0xA000 + ppu_Y_RS * 240 + ppu_X_RS];

									ppu_Pixel_Color[2] <<= 1;

									if (ppu_Pixel_Color[2] != 0)
									{
										ppu_BG_Has_Pixel[2] = true;
									}
									else
									{
										ppu_BG_Has_Pixel[2] = false;
									}
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								ppu_Fetch_Count[2] += 1;
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								ppu_Fetch_Count[2] += 1;
								if (ppu_Fetch_Count[2] == 243)
								{
									ppu_BG_Rendering_Complete[2] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
					}
				}
				break;

			case 5:
				// bitmaps, only BG2
				if (!ppu_BG_Rendering_Complete[2])
				{
					if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
					{
						if ((ppu_Cycle & 3) == 3)
						{
							if (ppu_Fetch_Count[2] < 240)
							{
								// calculate rotation and scaling
								if (ppu_BG_Mosaic[2])
								{
									cur_y = -ppu_MOS_BG_Y[ppu_LY - ppu_ROT_REF_LY[2]];
									cur_x = ppu_MOS_BG_X[ppu_Fetch_Count[2]];
								}
								else
								{
									cur_y = -(ppu_LY - ppu_ROT_REF_LY[2]);
									cur_x = ppu_Fetch_Count[2];
								}

								sol_x = ppu_F_Rot_A_2 * cur_x - ppu_F_Rot_B_2 * cur_y;
								sol_y = -ppu_F_Rot_C_2 * cur_x + ppu_F_Rot_D_2 * cur_y;

								ppu_X_RS = (uint16_t)floor(sol_x + ppu_F_Ref_X_2);
								ppu_Y_RS = (uint16_t)floor(-(sol_y - ppu_F_Ref_Y_2));

								// mark for VRAM access even if it is out of bounds
								ppu_Set_VRAM_Access_True();

								// display split into 2 frames, outside of 160 x 128, display backdrop
								if ((ppu_X_RS < 160) && (ppu_Y_RS < 128) && (ppu_X_RS >= 0) && (ppu_Y_RS >= 0))
								{
									// pixel color comes direct from VRAM
									int m5_ofst = ppu_X_RS + ppu_Y_RS * 160;

									// Note 0x5000 not 0xA000 since shifting right 1
									ppu_Pixel_Color[2] = VRAM_16[ppu_Display_Frame * 0x5000 + m5_ofst];

									ppu_BG_Has_Pixel[2] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								ppu_Fetch_Count[2] += 1;
							}
							else
							{
								// mark for VRAM access even though we don't need this data
								ppu_Set_VRAM_Access_True();

								ppu_BG_Has_Pixel[2] = false;

								ppu_Fetch_Count[2] += 1;
								if (ppu_Fetch_Count[2] == 243)
								{
									ppu_BG_Rendering_Complete[2] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
					}
				}
				break;

			case 6:
				// invalid
				break;

			case 7:
				// invalid
				break;
			}
		}

		inline void ppu_Set_VRAM_Access_True()
		{
			ppu_VRAM_Access = true;

			if (ppu_VRAM_In_Use)
			{
				if (ppu_VRAM_Access)
				{
					cpu_Fetch_Wait += 1;
				}
			}
		}

		void ppu_Render_Sprites()
		{
			uint32_t cur_x_pos = 0;
			uint32_t cur_y_pos = 0;
			int spr_tile = 0;
			int spr_tile_row = 0;
			int x_scale = 0;
			int y_scale = 0;
			int tile_x_offset = 0;
			int tile_y_offset = 0;

			uint32_t pix_color = 0;

			bool double_size = false;
			bool rot_scale = false;
			bool mosaic = false;

			int size_x_offset = 0;
			int size_y_offset = 0;

			int spr_mode = 0;

			int actual_x_index = 0;
			int actual_y_index = 0;

			int base_offset = 0;

			int rel_x_offset = 0;
			int rel_y_offset = 0;

			int spr_mod = 0;

			uint32_t pal_scale = 0;


			// account for processing time of rotated sprites, also the first 6 cycles are not used (probably pipeline reset)
			// when does reading from VRAM / OAM actually happen?
			if (ppu_Sprite_proc_time > 0)
			{
				ppu_Sprite_proc_time -= 1;
				if (ppu_Sprite_proc_time == 0)
				{
					if (ppu_New_Sprite)
					{
						// advance sprite variables if necessary
						ppu_New_Sprite = false;

						ppu_Sprite_Attr_0 = OAM_16[ppu_Current_Sprite * 4];
						ppu_Sprite_Attr_1 = OAM_16[ppu_Current_Sprite * 4 + 1];
						ppu_Sprite_Attr_2 = OAM_16[ppu_Current_Sprite * 4 + 2];

						ppu_Sprite_X_Pos = ppu_Sprite_Attr_1 & 0x1FF;
						ppu_Sprite_Y_Pos = ppu_Sprite_Attr_0 & 0xFF;

						ppu_Sprite_X_Size = ppu_OBj_Sizes_X[((ppu_Sprite_Attr_1 >> 14) & 3) * 4 + ((ppu_Sprite_Attr_0 >> 14) & 3)];
						ppu_Sprite_Y_Size = ppu_OBj_Sizes_Y[((ppu_Sprite_Attr_1 >> 14) & 3) * 4 + ((ppu_Sprite_Attr_0 >> 14) & 3)];

						// check if the sprite is in range (in the Y direction)

						// check scaling, only in rotation and scaling mode
						double_size = (ppu_Sprite_Attr_0 & 0x200) == 0x200;
						rot_scale = (ppu_Sprite_Attr_0 & 0x100) == 0x100;

						if (double_size && rot_scale)
						{
							size_y_offset = ppu_Sprite_Y_Size;
							size_x_offset = ppu_Sprite_X_Size;
						}

						// NOTE: double_size means disable in the context of not being in rot_scale mode
						if (double_size && !rot_scale)
						{
							// sprite not displayed
							ppu_Sprite_proc_time = 1;
							ppu_Current_Sprite += 1;
							ppu_New_Sprite = true;

							if (ppu_Current_Sprite == 128)
							{
								ppu_Sprite_Eval_Finished = true;
							}
						}
						else if (ppu_LY != 227)
						{
							// account for screen wrapping
							// if the object would appear at the top of the screen, that is the only part that is drawn
							if (ppu_Sprite_Y_Pos + ppu_Sprite_Y_Size + size_y_offset > 0xFF)
							{
								if (((ppu_LY + 1) >= ((ppu_Sprite_Y_Pos + ppu_Sprite_Y_Size + size_y_offset) & 0xFF)))
								{
									// sprite vertically out of range
									ppu_Sprite_proc_time = rot_scale ? 10 : 1;
									ppu_Current_Sprite += 1;
									ppu_New_Sprite = true;

									if (ppu_Current_Sprite == 128)
									{
										ppu_Sprite_Eval_Finished = true;
									}
								}
							}
							else
							{
								if (((ppu_LY + 1) < ppu_Sprite_Y_Pos) || ((ppu_LY + 1) >= ((ppu_Sprite_Y_Pos + ppu_Sprite_Y_Size + size_y_offset) & 0xFF)))
								{
									// sprite vertically out of range
									ppu_Sprite_proc_time = rot_scale ? 10 : 1;
									ppu_Current_Sprite += 1;
									ppu_New_Sprite = true;

									if (ppu_Current_Sprite == 128)
									{
										ppu_Sprite_Eval_Finished = true;
									}
								}
							}

							cur_y_pos = (uint32_t)((ppu_LY + 1 - ppu_Sprite_Y_Pos) & 0xFF);
						}
						else
						{
							// account for screen wrapping
							// if the object would appear at the top of the screen, that is the only part that is drawn
							if (ppu_Sprite_Y_Pos + ppu_Sprite_Y_Size + size_y_offset > 0xFF)
							{
								if (0 >= ((ppu_Sprite_Y_Pos + ppu_Sprite_Y_Size + size_y_offset) & 0xFF))
								{
									// sprite vertically out of range
									ppu_Sprite_proc_time = rot_scale ? 10 : 1;
									ppu_Current_Sprite += 1;
									ppu_New_Sprite = true;

									if (ppu_Current_Sprite == 128)
									{
										ppu_Sprite_Eval_Finished = true;
									}
								}
							}
							else
							{
								if ((0 < ppu_Sprite_Y_Pos) || (0 >= ((ppu_Sprite_Y_Pos + ppu_Sprite_Y_Size + size_y_offset) & 0xFF)))
								{
									// sprite vertically out of range
									ppu_Sprite_proc_time = rot_scale ? 10 : 1;
									ppu_Current_Sprite += 1;
									ppu_New_Sprite = true;

									if (ppu_Current_Sprite == 128)
									{
										ppu_Sprite_Eval_Finished = true;
									}
								}
							}

							cur_y_pos = (uint32_t)((0 - ppu_Sprite_Y_Pos) & 0xFF);
						}

						// if the sprite is in range, add it's data to the sprite buffer
						if (!ppu_New_Sprite)
						{
							x_scale = ppu_Sprite_X_Size >> 3;
							y_scale = ppu_Sprite_Y_Size >> 3;

							if ((ppu_Sprite_Attr_0 & 0x100) == 0)
							{
								ppu_Sprite_proc_time = ppu_Sprite_X_Size;
							}
							else
							{
								ppu_Sprite_proc_time = 10 + ((ppu_Sprite_X_Size + size_x_offset) << 1);
							}

							base_offset = ppu_Current_Sprite * 16384;

							rel_y_offset = (int)cur_y_pos;

							spr_mode = (ppu_Sprite_Attr_0 >> 10) & 3;

							// GBA tek says lower bit of tile number should be ignored in some cases, but it ppears this is not the case?
							// more testing needed
							if ((ppu_Sprite_Attr_0 & 0x2000) == 0)
							{
								spr_mod = 0x3FF;
							}
							else
							{
								spr_mod = 0x3FF;
							}

							mosaic = (ppu_Sprite_Attr_0 & 0x1000) == 0x1000;

							for (int i = 0; i < ppu_Sprite_X_Size + size_x_offset; i++)
							{
								cur_x_pos = (uint32_t)((ppu_Sprite_X_Pos + i) & 0x1FF);

								if (mosaic)
								{
									if (ppu_MOS_OBJ_X[cur_x_pos] < ppu_Sprite_X_Pos)
									{
										// lower pixels of sprite not aligned with mosaic grid, nothing to display (in x direction)
										rel_x_offset = 0;
										cur_x_pos = 255;
									}
									else
									{
										// calculate the pixel that is on a grid point, the grid is relative to the screen, not the sprite
										rel_x_offset = (ppu_MOS_OBJ_X[cur_x_pos] - ppu_Sprite_X_Pos) & 0x1FF;
									}

									if (ppu_MOS_OBJ_Y[ppu_LY] < ppu_Sprite_Y_Pos)
									{
										// lower pixels of sprite not aligned with mosaic grid, nothing to display (in y direction)
										rel_y_offset = 0;
										cur_x_pos = 255;
									}
									else
									{
										// calculate the pixel that is on a grid point, the grid is relative to the screen, not the sprite
										rel_y_offset = (ppu_MOS_OBJ_Y[ppu_LY] - ppu_Sprite_Y_Pos) & 0xFF;
									}
								}
								else
								{
									rel_x_offset = i;
								}

								// if sprite is in range horizontally
								if (cur_x_pos < 240)
								{
									// if the sprite's position is not occupied by a higher priority sprite, or it is a sprite window sprite, process it
									if (!ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_eval + cur_x_pos] || (spr_mode == 2) || 
										(((ppu_Sprite_Attr_2 >> 10) & 3) < ppu_Sprite_Priority[ppu_Sprite_ofst_eval + cur_x_pos]))
									{
										spr_tile = ppu_Sprite_Attr_2 & spr_mod;

										// look up the actual pixel to be used in the sprite rotation tables
										actual_x_index = ppu_ROT_OBJ_X[base_offset + rel_x_offset + rel_y_offset * 128];
										actual_y_index = ppu_ROT_OBJ_Y[base_offset + rel_x_offset + rel_y_offset * 128];

										if ((actual_x_index < ppu_Sprite_X_Size) && (actual_y_index < ppu_Sprite_Y_Size))
										{
											// pick out the tile to use
											if ((ppu_Sprite_Attr_0 & 0x2000) == 0)
											{
												if (ppu_OBJ_Dim)
												{
													spr_tile += (actual_x_index >> 3) + (x_scale) * (int)(actual_y_index >> 3);
												}
												else
												{
													// large x values wrap around
													spr_tile += (0x20) * (int)(actual_y_index >> 3);

													spr_tile_row = (int)(spr_tile & 0xFFFFFFE0);

													spr_tile += (actual_x_index >> 3);

													spr_tile &= 0x1F;
													spr_tile |= spr_tile_row;
												}

												spr_tile <<= 5;

												// pick out the correct pixel from the tile
												tile_x_offset = actual_x_index & 7;
												tile_y_offset = (int)(actual_y_index & 7);

												spr_tile += (tile_x_offset >> 1) + tile_y_offset * 4;

												spr_tile &= 0x7FFF;

												pix_color = (uint32_t)VRAM[0x10000 + spr_tile];

												if ((tile_x_offset & 1) == 0)
												{
													pix_color &= 0xF;
												}
												else
												{
													pix_color = (pix_color >> 4) & 0xF;
												}

												pix_color *= 2;
												pix_color += (uint32_t)(32 * (ppu_Sprite_Attr_2 >> 12));

												pal_scale = 0x1E;
											}
											else
											{
												spr_tile <<= 5;

												if (ppu_OBJ_Dim)
												{
													spr_tile += ((actual_x_index >> 3) + (x_scale) * (int)(actual_y_index >> 3)) << 6;
												}
												else
												{
													// large x values wrap around
													spr_tile += ((0x20) * (int)(actual_y_index >> 3) << 5);

													spr_tile_row = (int)(spr_tile & 0xFFFFFC00);

													spr_tile += ((actual_x_index >> 3) << 6);

													spr_tile &= 0x3FF;
													spr_tile |= spr_tile_row;
												}

												// pick out the correct pixel from the tile
												tile_x_offset = (actual_x_index & 7);
												tile_y_offset = (int)(actual_y_index & 7);

												spr_tile += tile_x_offset + tile_y_offset * 8;

												spr_tile &= 0x7FFF;

												pix_color = (uint32_t)(VRAM[0x10000 + spr_tile] << 1);

												pal_scale = 0x1FE;
											}

											// only allow upper half of vram sprite tiles to be used in modes 3-5
											if ((ppu_BG_Mode >= 3) && (spr_tile < 0x4000))
											{
												pix_color = 0;
											}

											if ((pix_color & pal_scale) != 0)
											{
												if (spr_mode < 2)
												{
													ppu_Sprite_Pixels[ppu_Sprite_ofst_eval + cur_x_pos] = pix_color + 0x200;

													ppu_Sprite_Priority[ppu_Sprite_ofst_eval + cur_x_pos] = (ppu_Sprite_Attr_2 >> 10) & 3;

													ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_eval + cur_x_pos] = true;

													ppu_Sprite_Semi_Transparent[ppu_Sprite_ofst_eval + cur_x_pos] = spr_mode == 1;
												}
												else if (spr_mode == 2)
												{
													ppu_Sprite_Object_Window[ppu_Sprite_ofst_eval + cur_x_pos] = true;
												}
											}
										}
									}
								}
							}
						}
					}
					else
					{
						// finished processing the sprite, go to the next one
						ppu_Sprite_proc_time = 1;
						ppu_Current_Sprite += 1;
						ppu_New_Sprite = true;

						if (ppu_Current_Sprite == 128)
						{
							ppu_Sprite_Eval_Finished = true;
						}
					}
				}
			}
		}

		// all sprites are centered located started in the upper left corner of a 128 x 128 pixel region
		void ppu_Calculate_Sprites_Pixels(uint32_t addr, bool do_all)
		{
			int set_changed;

			if (do_all)
			{
				for (int i = 0; i < 128; i++)
				{
					ppu_Do_Sprite_Calculation(i);
				}
			}
			else
			{
				// if the effected address is a sprite attribute, only update that one sprite
				// if it is a rotation parameter, update all sprites using that parameter
				if (((addr & 0xF) == 0x6) || ((addr & 0xF) == 0xE))
				{
					set_changed = (int)((addr >> 4) & 0x1F);

					for (int i = 0; i < 128; i++)
					{
						// update the sprite rotation / scaling if it is enabled
						if ((OAM[i * 8 + 1] & 0x1) == 1)
						{
							if (((OAM[i * 8 + 3] >> 1) & 0x1F) == set_changed)
							{
								ppu_Do_Sprite_Calculation(i);
							}
						}
					}
				}
				else
				{
					// update only one sprite
					ppu_Do_Sprite_Calculation((int)((addr >> 3) & 0x7F));
				}
			}
		}

		void ppu_Do_Sprite_Calculation(int i)
		{
			bool h_flip, v_flip;

			uint32_t base_ofst = 0;

			uint32_t A, B, C, D;

			int32_t i_A, i_B, i_C, i_D;

			double f_A, f_B, f_C, f_D;

			double fract_part = 0.5;

			double cur_x, cur_y;
			double sol_x, sol_y;

			int param_pick;

			base_ofst = i * 16384;

			ppu_Sprite_Attr_0 = OAM_16[i * 4];
			ppu_Sprite_Attr_1 = OAM_16[i * 4 + 1];

			ppu_Sprite_X_Size = ppu_OBj_Sizes_X[((ppu_Sprite_Attr_1 >> 14) & 3) * 4 + ((ppu_Sprite_Attr_0 >> 14) & 3)];
			ppu_Sprite_Y_Size = ppu_OBj_Sizes_Y[((ppu_Sprite_Attr_1 >> 14) & 3) * 4 + ((ppu_Sprite_Attr_0 >> 14) & 3)];

			double spr_size_x = ppu_Sprite_X_Size;
			double spr_size_y = ppu_Sprite_Y_Size;

			double spr_half_x = spr_size_x * 0.5;
			double spr_half_y = spr_size_y * 0.5;

			if ((OAM[i * 8 + 1] & 0x1) == 1)
			{
				fract_part = 0.5;

				// rotation and scaling enabled
				// pick out parameters
				param_pick = (ppu_Sprite_Attr_1 >> 9) & 0x1F;

				A = OAM_16[(0x06 + 0x20 * param_pick) >> 1];
				B = OAM_16[(0x0E + 0x20 * param_pick) >> 1];
				C = OAM_16[(0x16 + 0x20 * param_pick) >> 1];
				D = OAM_16[(0x1E + 0x20 * param_pick) >> 1];

				i_A = (int32_t)((A >> 8) & 0x7F);
				i_B = (int32_t)((B >> 8) & 0x7F);
				i_C = (int32_t)((C >> 8) & 0x7F);
				i_D = (int32_t)((D >> 8) & 0x7F);

				if ((A & 0x8000) == 0x8000) { i_A |= (int32_t)0xFFFFFF80; }
				if ((B & 0x8000) == 0x8000) { i_B |= (int32_t)0xFFFFFF80; }
				if ((C & 0x8000) == 0x8000) { i_C |= (int32_t)0xFFFFFF80; }
				if ((D & 0x8000) == 0x8000) { i_D |= (int32_t)0xFFFFFF80; }

				// convert to floats
				f_A = i_A;
				f_B = i_B;
				f_C = i_C;
				f_D = i_D;

				for (int j = 7; j >= 0; j--)
				{
					if ((A & (1 << j)) == (1 << j)) { f_A += fract_part; }
					if ((B & (1 << j)) == (1 << j)) { f_B += fract_part; }
					if ((C & (1 << j)) == (1 << j)) { f_C += fract_part; }
					if ((D & (1 << j)) == (1 << j)) { f_D += fract_part; }

					fract_part *= 0.5;
				}

				if (((ppu_Sprite_Attr_0 >> 9) & 0x1) == 1)
				{
					for (int j = 0; j < 2 * ppu_Sprite_X_Size; j++)
					{
						cur_x = j - spr_size_x;

						for (int k = 0; k < 2 * ppu_Sprite_Y_Size; k++)
						{
							cur_y = -k + spr_size_y;

							sol_x = f_A * cur_x - f_B * cur_y;
							sol_y = -f_C * cur_x + f_D * cur_y;

							sol_x += spr_half_x;
							sol_y -= spr_half_y;

							sol_y = -sol_y;

							sol_x = floor(sol_x);
							sol_y = floor(sol_y);

							ppu_ROT_OBJ_X[base_ofst + j + k * 128] = (uint16_t)(sol_x);
							ppu_ROT_OBJ_Y[base_ofst + j + k * 128] = (uint16_t)(sol_y);
						}
					}
				}
				else
				{
					for (int j = 0; j < ppu_Sprite_X_Size; j++)
					{
						cur_x = j - spr_half_x;

						for (int k = 0; k < ppu_Sprite_Y_Size; k++)
						{
							cur_y = -k + spr_half_y;

							sol_x = f_A * cur_x - f_B * cur_y;
							sol_y = -f_C * cur_x + f_D * cur_y;

							sol_x += spr_half_x;
							sol_y -= spr_half_y;

							sol_y = -sol_y;

							sol_x = floor(sol_x);
							sol_y = floor(sol_y);

							ppu_ROT_OBJ_X[base_ofst + j + k * 128] = (uint16_t)(sol_x);
							ppu_ROT_OBJ_Y[base_ofst + j + k * 128] = (uint16_t)(sol_y);
						}
					}
				}
			}
			else if ((OAM[i * 8 + 1] & 0x2) == 0)
			{
				h_flip = ((ppu_Sprite_Attr_1 & 0x1000) == 0x1000);
				v_flip = ((ppu_Sprite_Attr_1 & 0x2000) == 0x2000);

				for (int j = 0; j < ppu_Sprite_X_Size; j++)
				{
					for (int k = 0; k < ppu_Sprite_Y_Size; k++)
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
							sol_y = ppu_Sprite_Y_Size - 1 - k;
						}
						else
						{
							sol_y = k;
						}

						ppu_ROT_OBJ_X[base_ofst + j + k * 128] = (uint16_t)sol_x;
						ppu_ROT_OBJ_Y[base_ofst + j + k * 128] = (uint16_t)sol_y;
					}
				}
			}
		}

		void ppu_Convert_Rotation_to_float_AB(int layer)
		{
			uint16_t A, B;

			int32_t i_A, i_B;

			double f_A, f_B;

			double fract_part = 0.5;

			A = ppu_BG_Rot_A[layer];
			B = ppu_BG_Rot_B[layer];

			i_A = (int32_t)((A >> 8) & 0x7F);
			i_B = (int32_t)((B >> 8) & 0x7F);

			if ((A & 0x8000) == 0x8000) { i_A |= (int32_t)0xFFFFFF80; }
			if ((B & 0x8000) == 0x8000) { i_B |= (int32_t)0xFFFFFF80; }

			// convert to floats
			f_A = i_A;
			f_B = i_B;

			for (int j = 7; j >= 0; j--)
			{
				if ((A & (1 << j)) == (1 << j)) { f_A += fract_part; }
				if ((B & (1 << j)) == (1 << j)) { f_B += fract_part; }

				fract_part *= 0.5;
			}

			if (layer == 2)
			{
				ppu_F_Rot_A_2 = f_A;
				ppu_F_Rot_B_2 = f_B;
			}
			else
			{
				ppu_F_Rot_A_3 = f_A;
				ppu_F_Rot_B_3 = f_B;
			}
		}

		void ppu_Convert_Rotation_to_float_CD(int layer)
		{
			uint16_t C, D;

			int32_t i_C, i_D;

			double f_C, f_D;

			double fract_part = 0.5;

			C = ppu_BG_Rot_C[layer];
			D = ppu_BG_Rot_D[layer];

			i_C = (int32_t)((C >> 8) & 0x7F);
			i_D = (int32_t)((D >> 8) & 0x7F);

			if ((C & 0x8000) == 0x8000) { i_C |= (int32_t)0xFFFFFF80; }
			if ((D & 0x8000) == 0x8000) { i_D |= (int32_t)0xFFFFFF80; }

			// convert to floats
			f_C = i_C;
			f_D = i_D;

			for (int j = 7; j >= 0; j--)
			{
				if ((C & (1 << j)) == (1 << j)) { f_C += fract_part; }
				if ((D & (1 << j)) == (1 << j)) { f_D += fract_part; }

				fract_part *= 0.5;
			}

			if (layer == 2)
			{
				ppu_F_Rot_C_2 = f_C;
				ppu_F_Rot_D_2 = f_D;
			}
			else
			{
				ppu_F_Rot_C_3 = f_C;
				ppu_F_Rot_D_3 = f_D;
			}
		}

		void ppu_Convert_Offset_to_float(int layer)
		{
			double fract_part = 0.5;

			uint32_t Ref_X = ppu_BG_Ref_X_Latch[layer];
			uint32_t Ref_Y = ppu_BG_Ref_Y_Latch[layer];

			int32_t i_ref_x = (int)((Ref_X >> 8) & 0x7FFFF);
			int32_t i_ref_y = (int)((Ref_Y >> 8) & 0x7FFFF);

			if ((Ref_X & 0x8000000) == 0x8000000) { i_ref_x |= (int32_t)0xFFF80000; }
			if ((Ref_Y & 0x8000000) == 0x8000000) { i_ref_y |= (int32_t)0xFFF80000; }

			if (layer == 2)
			{
				ppu_F_Ref_X_2 = i_ref_x;
				ppu_F_Ref_Y_2 = i_ref_y;

				for (int j = 7; j >= 0; j--)
				{
					if ((Ref_X & (1 << j)) == (1 << j)) { ppu_F_Ref_X_2 += fract_part; }
					if ((Ref_Y & (1 << j)) == (1 << j)) { ppu_F_Ref_Y_2 += fract_part; }

					fract_part *= 0.5;
				}
			}
			else
			{
				ppu_F_Ref_X_3 = i_ref_x;
				ppu_F_Ref_Y_3 = i_ref_y;

				for (int j = 7; j >= 0; j--)
				{
					if ((Ref_X & (1 << j)) == (1 << j)) { ppu_F_Ref_X_3 += fract_part; }
					if ((Ref_Y & (1 << j)) == (1 << j)) { ppu_F_Ref_Y_3 += fract_part; }

					fract_part *= 0.5;
				}
			}
		}

		void ppu_Reset()
		{
			ppu_X_RS = ppu_Y_RS = 0;

			ppu_BG_Ref_X[2] = ppu_BG_Ref_X[3] = 0;
			ppu_BG_Ref_X_Latch[2] = ppu_BG_Ref_X_Latch[3] = 0;

			ppu_BG_Ref_Y[2] = ppu_BG_Ref_Y[3] = 0;
			ppu_BG_Ref_Y_Latch[2] = ppu_BG_Ref_Y_Latch[3] = 0;

			ppu_CTRL = ppu_Green_Swap = 0;

			ppu_VBL_IRQ_cd = ppu_HBL_IRQ_cd = ppu_LYC_IRQ_cd = 0;

			ppu_LYC_Vid_Check_cd = 0;

			for (int i = 0; i < 4; i++)
			{
				ppu_BG_CTRL[i] = 0;
				ppu_BG_X[0] = 0;
				ppu_BG_Y[0] = 0;

				ppu_BG_On_Update_Time[i] = 0;
			}

			ppu_BG_Rot_A[2] = ppu_BG_Rot_B[2] = ppu_BG_Rot_C[2] = ppu_BG_Rot_D[2] = 0;

			ppu_BG_Rot_A[3] = ppu_BG_Rot_B[3] = ppu_BG_Rot_C[3] = ppu_BG_Rot_D[3] = 0;

			ppu_WIN_Hor_0 = ppu_WIN_Hor_1 = ppu_WIN_Vert_0 = ppu_WIN_Vert_1 = 0;

			ppu_WIN_In = ppu_WIN_Out = ppu_Mosaic = ppu_Special_FX = ppu_Alpha = ppu_Bright = 0;

			ppu_STAT = 0;

			ppu_LYC = 0xFF;

			// based on music4.gba, initial state would either be Ly = 225 or 161.
			// based on console verification testing, it seems 225 is correct.
			ppu_LY = 225;

			// 2 gives the correct value in music4.gba
			ppu_Cycle = 2;
				
			ppu_Display_Cycle = 0;

			ppu_In_VBlank = true;

			ppu_Delays = false;

			// reset sprite evaluation variables
			ppu_Current_Sprite = 0;
			ppu_New_Sprite = true;
			ppu_Sprite_Eval_Finished = false;

			for (int i = 0; i < 240 * 2; i++)
			{
				ppu_Sprite_Pixels[i] = 0;
				ppu_Sprite_Priority[i] = 0;

				ppu_Sprite_Pixel_Occupied[i] = false;
				ppu_Sprite_Semi_Transparent[i] = false;
				ppu_Sprite_Object_Window[i] = false;
			}

			ppu_Sprite_Attr_0 = ppu_Sprite_Attr_1 = ppu_Sprite_Attr_2 = 0;

			ppu_Current_Sprite = ppu_Sprite_ofst_eval = 0;
			ppu_Sprite_ofst_draw = 240;

			ppu_Sprite_proc_time = 6;

			ppu_Sprite_X_Pos = ppu_Sprite_Y_Pos = ppu_Sprite_X_Size = ppu_Sprite_Y_Size = 0;

			ppu_VRAM_Access = false;
			ppu_PALRAM_Access = false;
			ppu_OAM_Access = false;

			ppu_VRAM_In_Use = ppu_PALRAM_In_Use = ppu_OAM_In_Use = false;

			// BG rendering
			for (int i = 0; i < 4; i++)
			{
				ppu_Fetch_Count[i] = 0;
				ppu_Scroll_Cycle[i] = 0;
				ppu_Pixel_Color[i] = 0;
				ppu_Pixel_Color_2[i] = 0;
				ppu_Pixel_Color_1[i] = 0;
				ppu_Pixel_Color_R[i] = 0;
				ppu_BG_Start_Time[i] = 0;
				ppu_BG_Effect_Byte[i] = 0;
				ppu_BG_Effect_Byte_New[i] = 0;
				ppu_Tile_Addr[i] = 0;
				ppu_Y_Flip_Ofst[i] = 0;

				ppu_BG_Rendering_Complete[i] = false;
				ppu_BG_Has_Pixel[i] = false;
				ppu_BG_Has_Pixel_2[i] = false;
				ppu_BG_Has_Pixel_1[i] = false;
				ppu_BG_Has_Pixel_R[i] = false;

			}

			ppu_BG_Pixel_F = 0;
			ppu_BG_Pixel_S = 0;
			ppu_Final_Pixel = 0;
			ppu_Blend_Pixel = 0;

			ppu_Brighten_Final_Pixel = false;
			ppu_Blend_Final_Pixel = false;

			ppu_Fetch_BG = true;

			ppu_Fetch_Target_1 = false;
			ppu_Fetch_Target_2 = false;

			ppu_Rendering_Complete = false;
			ppu_PAL_Rendering_Complete = false;

			// PPU power up
			ppu_CTRL_Write(0);

			// update derived values
			ppu_Calc_Win0();
			ppu_Calc_Win1();
			ppu_Update_Win_In(0);
			ppu_Update_Win_Out(0);
			ppu_BG_CTRL_Write(0);
			ppu_BG_CTRL_Write(1);
			ppu_BG_CTRL_Write(2);
			ppu_BG_CTRL_Write(3);
			ppu_Update_Special_FX(0);
			ppu_Update_Alpha(0);
			ppu_Update_Bright(0);
			ppu_Update_Mosaic(0);

			ppu_Convert_Offset_to_float(2);
			ppu_Convert_Offset_to_float(3);
			ppu_Convert_Rotation_to_float_AB(2);
			ppu_Convert_Rotation_to_float_CD(2);
			ppu_Convert_Rotation_to_float_AB(3);
			ppu_Convert_Rotation_to_float_CD(3);
		}

		uint8_t* ppu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ppu_In_VBlank, saver);
			saver = bool_saver(ppu_Delays, saver);

			saver = bool_saver(ppu_VRAM_In_Use, saver);
			saver = bool_saver(ppu_PALRAM_In_Use, saver);
			saver = bool_saver(ppu_OAM_In_Use, saver);

			saver = bool_saver(ppu_VRAM_Access, saver);
			saver = bool_saver(ppu_PALRAM_Access, saver);
			saver = bool_saver(ppu_OAM_Access, saver);

			saver = bool_saver(ppu_HBL_Free, saver);
			saver = bool_saver(ppu_OBJ_Dim, saver);
			saver = bool_saver(ppu_Forced_Blank, saver);
			saver = bool_saver(ppu_Any_Window_On, saver);
			saver = bool_saver(ppu_OBJ_On, saver);
			saver = bool_saver(ppu_WIN0_On, saver);
			saver = bool_saver(ppu_WIN1_On, saver);
			saver = bool_saver(ppu_OBJ_WIN, saver);

			saver = byte_saver(ppu_STAT, saver);
			saver = byte_saver(ppu_LY, saver);
			saver = byte_saver(ppu_LYC, saver);

			saver = short_saver(ppu_CTRL, saver);
			saver = short_saver(ppu_Green_Swap, saver);
			saver = short_saver(ppu_Cycle, saver);
			saver = short_saver(ppu_Display_Cycle, saver);
			saver = short_saver(ppu_Sprite_Eval_Time, saver);
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

			saver = int_saver(ppu_VBL_IRQ_cd, saver);
			saver = int_saver(ppu_HBL_IRQ_cd, saver);
			saver = int_saver(ppu_LYC_IRQ_cd, saver);

			saver = int_saver(ppu_LYC_Vid_Check_cd, saver);

			saver = bool_array_saver(ppu_BG_On, saver, 4);
			saver = bool_array_saver(ppu_BG_On_New, saver, 4);

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
			saver = int_array_saver(ppu_BG_Ref_X_Latch, saver, 4);
			saver = int_array_saver(ppu_BG_Ref_Y_Latch, saver, 4);

			saver = int_array_saver(ppu_BG_On_Update_Time, saver, 4);

			// Sprite Evaluation
			saver = bool_saver(ppu_New_Sprite, saver);
			saver = bool_saver(ppu_Sprite_Eval_Finished, saver);

			saver = short_saver(ppu_Sprite_Attr_0, saver);
			saver = short_saver(ppu_Sprite_Attr_1, saver);
			saver = short_saver(ppu_Sprite_Attr_2, saver);

			saver = int_saver(ppu_Current_Sprite, saver);
			saver = int_saver(ppu_Sprite_ofst_eval, saver);
			saver = int_saver(ppu_Sprite_ofst_draw, saver);
			saver = int_saver(ppu_Sprite_proc_time, saver);
			saver = int_saver(ppu_Sprite_X_Pos, saver);
			saver = int_saver(ppu_Sprite_Y_Pos, saver);
			saver = int_saver(ppu_Sprite_X_Size, saver);
			saver = int_saver(ppu_Sprite_Y_Size, saver);

			saver = bool_array_saver(ppu_Sprite_Pixel_Occupied, saver, 240 * 2);
			saver = bool_array_saver(ppu_Sprite_Semi_Transparent, saver, 240 * 2);
			saver = bool_array_saver(ppu_Sprite_Object_Window, saver, 240 * 2);

			saver = int_array_saver(ppu_Sprite_Pixels, saver, 240 * 2);
			saver = int_array_saver(ppu_Sprite_Priority, saver, 240 * 2);

			// BG rendering
			saver = int_array_saver(ppu_Fetch_Count, saver, 4);
			saver = int_array_saver(ppu_Scroll_Cycle, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color_2, saver, 4);
			saver = int_array_saver(ppu_Pixel_Color_1, saver, 4);
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
			saver = bool_array_saver(ppu_BG_Has_Pixel_R, saver, 4);

			saver = int_saver(ppu_BG_Pixel_F, saver);
			saver = int_saver(ppu_BG_Pixel_S, saver);
			saver = int_saver(ppu_Final_Pixel, saver);
			saver = int_saver(ppu_Blend_Pixel, saver);

			saver = bool_saver(ppu_Brighten_Final_Pixel, saver);
			saver = bool_saver(ppu_Blend_Final_Pixel, saver);

			saver = bool_saver(ppu_Fetch_BG, saver);

			saver = bool_saver(ppu_Fetch_Target_1, saver);
			saver = bool_saver(ppu_Fetch_Target_2, saver);

			saver = bool_saver(ppu_Rendering_Complete, saver);
			saver = bool_saver(ppu_PAL_Rendering_Complete, saver);
			
			return saver;
		}

		uint8_t* ppu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ppu_In_VBlank, loader);
			loader = bool_loader(&ppu_Delays, loader);

			loader = bool_loader(&ppu_VRAM_In_Use, loader);
			loader = bool_loader(&ppu_PALRAM_In_Use, loader);
			loader = bool_loader(&ppu_OAM_In_Use, loader);

			loader = bool_loader(&ppu_VRAM_Access, loader);
			loader = bool_loader(&ppu_PALRAM_Access, loader);
			loader = bool_loader(&ppu_OAM_Access, loader);

			loader = bool_loader(&ppu_HBL_Free, loader);
			loader = bool_loader(&ppu_OBJ_Dim, loader);
			loader = bool_loader(&ppu_Forced_Blank, loader);
			loader = bool_loader(&ppu_Any_Window_On, loader);
			loader = bool_loader(&ppu_OBJ_On, loader);
			loader = bool_loader(&ppu_WIN0_On, loader);
			loader = bool_loader(&ppu_WIN1_On, loader);
			loader = bool_loader(&ppu_OBJ_WIN, loader);

			loader = byte_loader(&ppu_STAT, loader);
			loader = byte_loader(&ppu_LY, loader);
			loader = byte_loader(&ppu_LYC, loader);

			loader = short_loader(&ppu_CTRL, loader);
			loader = short_loader(&ppu_Green_Swap, loader);
			loader = short_loader(&ppu_Cycle, loader);
			loader = short_loader(&ppu_Display_Cycle, loader);
			loader = short_loader(&ppu_Sprite_Eval_Time, loader);
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

			loader = int_loader(&ppu_VBL_IRQ_cd, loader);
			loader = int_loader(&ppu_HBL_IRQ_cd, loader);
			loader = int_loader(&ppu_LYC_IRQ_cd, loader);

			loader = int_loader(&ppu_LYC_Vid_Check_cd, loader);

			loader = bool_array_loader(ppu_BG_On, loader, 4);
			loader = bool_array_loader(ppu_BG_On_New, loader, 4);

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
			loader = int_array_loader(ppu_BG_Ref_X_Latch, loader, 4);
			loader = int_array_loader(ppu_BG_Ref_Y_Latch, loader, 4);

			loader = int_array_saver(ppu_BG_On_Update_Time, loader, 4);

			// Sprite Evaluation

			loader = bool_loader(&ppu_New_Sprite, loader);
			loader = bool_loader(&ppu_Sprite_Eval_Finished, loader);

			loader = short_loader(&ppu_Sprite_Attr_0, loader);
			loader = short_loader(&ppu_Sprite_Attr_1, loader);
			loader = short_loader(&ppu_Sprite_Attr_2, loader);

			loader = int_loader(&ppu_Current_Sprite, loader);
			loader = int_loader(&ppu_Sprite_ofst_eval, loader);
			loader = int_loader(&ppu_Sprite_ofst_draw, loader);
			loader = int_loader(&ppu_Sprite_proc_time, loader);
			loader = int_loader(&ppu_Sprite_X_Pos, loader);
			loader = int_loader(&ppu_Sprite_Y_Pos, loader);
			loader = int_loader(&ppu_Sprite_X_Size, loader);
			loader = int_loader(&ppu_Sprite_Y_Size, loader);

			loader = bool_array_loader(ppu_Sprite_Pixel_Occupied, loader, 240 * 2);
			loader = bool_array_loader(ppu_Sprite_Semi_Transparent, loader, 240 * 2);
			loader = bool_array_loader(ppu_Sprite_Object_Window, loader, 240 * 2);

			loader = int_array_loader(ppu_Sprite_Pixels, loader, 240 * 2);
			loader = int_array_loader(ppu_Sprite_Priority, loader, 240 * 2);

			// BG rendering
			loader = int_array_loader(ppu_Fetch_Count, loader, 4);
			loader = int_array_loader(ppu_Scroll_Cycle, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color_2, loader, 4);
			loader = int_array_loader(ppu_Pixel_Color_1, loader, 4);
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
			loader = bool_array_loader(ppu_BG_Has_Pixel_R, loader, 4);

			loader = int_loader(&ppu_BG_Pixel_F, loader);
			loader = int_loader(&ppu_BG_Pixel_S, loader);
			loader = int_loader(&ppu_Final_Pixel, loader);
			loader = int_loader(&ppu_Blend_Pixel, loader);

			loader = bool_loader(&ppu_Brighten_Final_Pixel, loader);
			loader = bool_loader(&ppu_Blend_Final_Pixel, loader);

			loader = bool_loader(&ppu_Fetch_BG, loader);

			loader = bool_loader(&ppu_Fetch_Target_1, loader);
			loader = bool_loader(&ppu_Fetch_Target_2, loader);

			loader = bool_loader(&ppu_Rendering_Complete, loader);
			loader = bool_loader(&ppu_PAL_Rendering_Complete, loader);

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

			ppu_Convert_Offset_to_float(2);
			ppu_Convert_Offset_to_float(3);
			ppu_Convert_Rotation_to_float_AB(2);
			ppu_Convert_Rotation_to_float_CD(2);
			ppu_Convert_Rotation_to_float_AB(3);
			ppu_Convert_Rotation_to_float_CD(3);

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
			for (int i = 0; i < 4; i++)
			{
				if (chan_A)
				{
					snd_FIFO_A[snd_FIFO_A_ptr] = snd_FIFO_A_Data[i];
					if (snd_FIFO_A_ptr < 31) { snd_FIFO_A_ptr += 1; }
				}
				else
				{
					snd_FIFO_B[snd_FIFO_B_ptr] = snd_FIFO_B_Data[i];
					if (snd_FIFO_B_ptr < 31) { snd_FIFO_B_ptr += 1; }
				}
			}
		}

		void snd_Update_Regs(uint32_t addr, uint8_t value)
		{
			// while power is on, everything is writable
			//Console.WriteLine("write: " + (addr & 0xFF) + " " + value + " " + CycleCount);
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

							snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 2 : 1;
							snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 2 : 1;

							value &= 0xF;
							break;

						case 0x83:                                              // GBA High (ctrl)
							snd_FIFO_A_Enable_R = (value & 1) == 1;
							snd_FIFO_A_Enable_L = (value & 2) == 2;

							snd_FIFO_B_Enable_R = (value & 0x10) == 0x10;
							snd_FIFO_B_Enable_L = (value & 0x20) == 0x20;

							snd_FIFO_A_Timer = (value & 0x04) >> 3;
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

						snd_FIFO_A_Mult = ((value & 0x04) == 0x04) ? 2 : 1;
						snd_FIFO_B_Mult = ((value & 0x08) == 0x08) ? 2 : 1;

						value &= 0xF;
						break;

					case 0x83:                                              // GBA High (ctrl)
						snd_FIFO_A_Enable_R = (value & 1) == 1;
						snd_FIFO_A_Enable_L = (value & 2) == 2;

						snd_FIFO_B_Enable_R = (value & 0x10) == 0x10;
						snd_FIFO_B_Enable_L = (value & 0x20) == 0x20;

						snd_FIFO_A_Timer = (value & 0x04) >> 3;
						snd_FIFO_B_Timer = (value & 0x40) >> 6;

						if ((value & 0x08) == 0x08)
						{
							for (int i = 0; i < 32; i++)
							{
								snd_FIFO_A[i] = 0;
							}

							snd_FIFO_A_ptr = 0;
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
			for (uint32_t i = 0x60; i < 0x84; i++)
			{
				snd_Write_Reg_8(i, 0);
			}

			for (int i = 0; i < 32; i++)
			{
				snd_FIFO_A[i] = 0;
				snd_FIFO_B[i] = 0;
			}

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

			for (int i = 0; i < 0x16; i++)
			{
				snd_Write_Reg_8((uint32_t)(0xFF10 + i), 0);
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
			saver = bool_saver(IRQ_Write_Delay_2, saver);
			saver = bool_saver(IRQ_Write_Delay_3, saver);

			saver = byte_saver(Post_Boot, saver);
			saver = byte_saver(Halt_CTRL, saver);

			saver = short_saver(INT_EN, saver);
			saver = short_saver(INT_Flags, saver);
			saver = short_saver(INT_Master, saver);
			saver = short_saver(Wait_CTRL, saver);
			saver = short_saver(controller_state, saver);

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


			
			saver = byte_array_saver(WRAM, saver, 0x40000);
			saver = byte_array_saver(IWRAM, saver, 0x8000);
			saver = byte_array_saver(PALRAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x18000);
			saver = byte_array_saver(OAM, saver, 0x400);

			// Prefetcher
			saver = bool_saver(pre_Cycle_Glitch, saver);

			saver = bool_saver(pre_Enable, saver);
			saver = bool_saver(pre_Seq_Access, saver);

			saver = int_saver(pre_Read_Addr, saver);
			saver = int_saver(pre_Check_Addr, saver);
			saver = int_saver(pre_Buffer_Cnt, saver);

			saver = int_saver(pre_Fetch_Cnt, saver);
			saver = int_saver(pre_Fetch_Wait, saver);
			saver = int_saver(pre_Fetch_Cnt_Inc, saver);

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
			loader = bool_loader(&IRQ_Write_Delay_2, loader);
			loader = bool_loader(&IRQ_Write_Delay_3, loader);

			loader = byte_loader(&Post_Boot, loader);
			loader = byte_loader(&Halt_CTRL, loader);

			loader = short_loader(&INT_EN, loader);
			loader = short_loader(&INT_Flags, loader);
			loader = short_loader(&INT_Master, loader);
			loader = short_loader(&Wait_CTRL, loader);
			loader = short_loader(&controller_state, loader);

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
			
			loader = byte_array_loader(WRAM, loader, 0x40000);
			loader = byte_array_loader(IWRAM, loader, 0x8000);
			loader = byte_array_loader(PALRAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x18000);
			loader = byte_array_loader(OAM, loader, 0x400);

			// Prefetcher
			loader = bool_loader(&pre_Cycle_Glitch, loader);

			loader = bool_loader(&pre_Enable, loader);
			loader = bool_loader(&pre_Seq_Access, loader);

			loader = int_loader(&pre_Read_Addr, loader);
			loader = int_loader(&pre_Check_Addr, loader);
			loader = int_loader(&pre_Buffer_Cnt, loader);

			loader = int_loader(&pre_Fetch_Cnt, loader);
			loader = int_loader(&pre_Fetch_Wait, loader);
			loader = int_loader(&pre_Fetch_Cnt_Inc, loader);

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
