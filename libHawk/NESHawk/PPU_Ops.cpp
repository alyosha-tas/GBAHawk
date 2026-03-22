#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "NES_System.h"
#include "Mappers.h"

// Note:

/*

*/

namespace NESHawk
{
	uint8_t NES_System::ppubus_read(uint32_t addr)
	{
		// clock the address bus change
		mapper_pntr->AddressPPU(addr);
		
		return mapper_pntr->ReadPPU(addr);
	}

	void NES_System::ppubus_clock(uint32_t addr)
	{
		// clock the address bus change
		mapper_pntr->AddressPPU(addr);
	}

	// debug tools peek into the ppu through this
	uint8_t NES_System::ppubus_peek(uint32_t addr)
	{
		return mapper_pntr->PeekPPU(addr);
	}

	uint8_t NES_System::read_2007()
	{
		int addr = ppu_VRAM_Address & 0x3FFF;
		int bus_case = 0;
		// ordinarily we return the buffered values
		uint8_t ret = VRAMBuffer;

		// in any case, we read from the ppu bus
		VRAMBuffer = mapper_pntr->ReadPPU(addr);

		// but reads from the palette are implemented in the PPU and return immediately
		if ((addr & 0x3F00) == 0x3F00)
		{
			// TODO apply greyscale shit?
			ret = PALRAM[addr & 0x1F];

			if (ppu_Color_Disable)
			{
				ret &= 0x30;
			}
			
			ret += (ppu_Open_Bus & 0xC0);
			bus_case = 1;
		}

		ppu_Increment_2007(ppu_Is_Rendering() && PPUON(), ppu_Vram_Incr32);

		// Clock the new VRAM address if not rendering (because Reg_v is on the bus)
		if (!ppu_Is_Rendering() || !PPUON())
		{
			ppu_VRAM_Address = ppu_Reg_v;
			mapper_pntr->AddressPPU(ppu_Reg_v);
		}

		// update open bus here
		ppu_Open_Bus = ret;
		if (bus_case == 0)
		{
			PpuOpenBusDecay(DecayType_All);
		}
		else
		{
			PpuOpenBusDecay(DecayType_Low);
		}

		return ret;
	}

	void NES_System::write_2007(uint8_t value)
	{
		int addr = ppu_VRAM_Address & 0x3FFF;
		if (ppuphase == PPU_PHASE_BG)
		{
			if (show_bg_new)
			{
				addr = ppu_Get_NT_Read();
			}
		}

		if ((addr & 0x3F00) == 0x3F00)
		{
			// handle palette writes, note greyscale does not effect written value
			addr &= 0x1F;
			uint8_t color = (uint8_t)(value & 0x3F);

			// this little hack will help you debug things while the screen is black
			// color = (uint8_t)(addr & 0x3F);

			PALRAM[addr] = color;
			if ((addr & 3) == 0)
			{
				PALRAM[addr ^ 0x10] = color;
			}
		}
		else
		{
			addr &= 0x3FFF;

			mapper_pntr->WritePPU(addr, value);
		}

		ppu_Increment_2007(ppu_Is_Rendering() && PPUON(), ppu_Vram_Incr32);

		// Clock the new VRAM address if not rendering (because Reg_v is on the bus)
		if (!ppu_Is_Rendering() || !PPUON())
		{
			ppu_VRAM_Address = ppu_Reg_v;
			mapper_pntr->AddressPPU(ppu_VRAM_Address);
		}
	}

	void NES_System::write_2007_Glitch(uint8_t value)
	{
		int addr = ppu_VRAM_Address & 0x3FFF;
		if (ppuphase == PPU_PHASE_BG)
		{
			if (show_bg_new)
			{
				addr = ppu_Get_NT_Read();
			}
		}

		if (glitch_2007_iter == 0)
		{
			if ((addr & 0x3F00) == 0x3F00)
			{
				// handle palette. this is being done nestopia style, because i found some documentation for it (appendix 1)
				addr &= 0x1F;
				uint8_t color = (uint8_t)(value & 0x3F); //are these bits really unwired? can they be read back somehow?

				// this little hack will help you debug things while the screen is black
				// color = (uint8_t)(addr & 0x3F);

				PALRAM[addr] = color;
				if ((addr & 3) == 0)
				{
					PALRAM[addr ^ 0x10] = color;
				}
			}
			else
			{
				addr &= 0x3FFF;

				glitch_2007_addr = addr & 0xFF;

				mapper_pntr->WritePPU(addr, glitch_2007_addr);
			}
		}
		else
		{
			if ((addr & 0x3F00) == 0x3F00)
			{
				// handle palette. this is being done nestopia style, because i found some documentation for it (appendix 1)
				addr &= 0x1F;
				uint8_t color = (uint8_t)(value & 0x3F); //are these bits really unwired? can they be read back somehow?

				// this little hack will help you debug things while the screen is black
				// color = (uint8_t)(addr & 0x3F);

				PALRAM[addr] = color;
				if ((addr & 3) == 0)
				{
					PALRAM[addr ^ 0x10] = color;
				}
			}
			else
			{
				addr &= 0x3FFF;

				addr &= 0xFF00;

				addr |= value;

				mapper_pntr->WritePPU(addr, value);
			}

		}

		ppu_Increment_2007(ppu_Is_Rendering() && PPUON(), ppu_Vram_Incr32);

		// Clock the new VRAM address if not rendering (because Reg_v is on the bus)
		if (!ppu_Is_Rendering() || !PPUON())
		{
			ppu_VRAM_Address = ppu_Reg_v;
			mapper_pntr->AddressPPU(ppu_VRAM_Address);
		}
	}

	void NES_System::ppu_Run()
	{
		//run one ppu cycle at a time so we can interact with the ppu and clockPPU at high granularity			
		if (install_2006 > 0)
		{
			install_2006--;
			if (install_2006 == 0)
			{
				if (!race_2006)
				{
					ppu_Install_Latches();

					// Clock the new VRAM address if not rendering (because Reg_v is on the bus)
					if (!ppu_Is_Rendering() || !PPUON())
					{
						ppu_VRAM_Address = ppu_Reg_v;
						mapper_pntr->AddressPPU(ppu_VRAM_Address);
					}
				}
				else
				{
					if (status_cycle == 256)
					{
						ppu_Reg_v &= ppu_Reg_t;
					}
					else
					{
						ppu_Reg_v &= ppu_Reg_t;
						ppu_Reg_v &= 0x41F;
						ppu_Reg_v |= (ppu_Reg_t & 0xFBE0);
					}
				}				
			}
		}

		race_2006 = false;

		if (install_2001 > 0)
		{
			install_2001--;
			if (install_2001 == 0)
			{
				bool ppu_was_on_temp = PPUON();
				
				show_bg_new = ppu_Show_BG;
				show_obj_new = ppu_Show_OBJ;

				// Clock the new VRAM address if not rendering (because Reg_v is on the bus)
				if (ppu_was_on_temp && ppu_Is_Rendering() && !PPUON())
				{
					ppu_VRAM_Address = ppu_Reg_v;
					mapper_pntr->AddressPPU(ppu_Reg_v);
				}

				// OAM corruption can occur at this point, keep track of the secondary OAM address here	
				if (ppu_Is_Rendering() && !PPUON() && ppu_was_on_temp)
				{
					ppu_Can_Corrupt = true;					

					ppu_OAM_Corrupt_Addr = soam_index & 7;
				}

				if (ppu_Is_Rendering() && PPUON() && !ppu_was_on_temp && ppu_Can_Corrupt)
				{
					for (int i = 0; i < 8; i++)
					{
						OAM[ppu_OAM_Corrupt_Addr * 8 + i] = OAM[i];
					}

					soam[ppu_OAM_Corrupt_Addr * 8] = soam[0];

					ppu_Can_Corrupt = false;
				}			
			}
		}

		if (status_cycle >= 258 && status_cycle <= 321 && ppu_Is_Rendering() && PPUON())
		{
			reg_2003 = 0;
		}

		// Here we execute a CPU instruction if enough PPU cycles have passed
		// also do other things that happen at instruction level granularity
		cpu_stepcounter++;
		if (cpu_stepcounter == 3)
		{
			cpu_step++;
			if (cpu_step == 5) cpu_step = 0;
			cpu_stepcounter = 0;

			// Check for NMIs
			if (NMI_PendingInstructions > 0)
			{
				NMI_PendingInstructions--;
				if (NMI_PendingInstructions <= 0)
				{
					NMI = true;
				}
			}

			// this is where the CPU instruction is called
			RunCpuOne();

			// decay the ppu bus, approximating real behaviour
			PpuOpenBusDecay(DecayType_None);
		}

		if (ppu_HasClockPPU)
		{
			mapper_pntr->ClockPPU();
		}
		Total_PPU_Clock_Cycles += 1;
	}
}