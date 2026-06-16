#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "GBA_System.h"
#include "Mappers.h"

/*
	$10000000-$FFFFFFFF    Unmapped
	$0E010000-$0FFFFFFF    Unmapped
	$0E000000-$0E00FFFF    SRAM
	$0C000000-$0DFFFFFF    ROM - Wait State 2
	$0A000000-$0BFFFFFF    ROM - Wait State 1
	$08000000-$09FFFFFF    ROM - Wait State 0
	$07000400-$07FFFFFF    Unmapped
	$07000000-$070003FF    OAM
	$06018000-$06FFFFFF    Unmapped
	$06000000-$06017FFF    VRAM
	$05000400-$05FFFFFF    Unmapped
	$05000000-$050003FF    Palette RAM
	$04XX0800-$04XX0800    Mem Control
	$04000000-$040007FF    I/O Regs
	$03008000-$03FFFFFF    Unmapped
	$03000000-$03007FFF    IWRAM
	$02040000-$02FFFFFF    Unmapped
	$02000000-$0203FFFF    WRAM
	$00004000-$01FFFFFF    Unmapped
	$00000000-$00003FFF    BIOS
*/

namespace GBAHawk
{
	void GBA_System::Update_Bus_Read_8(uint32_t addr, uint8_t value)
	{
		if (Update_Bus)
		{		
			if ((addr & 0xFF000000) == 0x03000000)
			{
				// Update IWRAM bus
				cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFF << (((int)addr & 3) * 8));
				cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 3) * 8);

				cpu_Last_Bus_Value = cpu_IWRAM_Last_Bus_Value;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];

				cpu_Last_Bus_Value = cpu_OAM_Last_Bus_Value;
			}
			else if (addr < 0x4000)
			{
				if (cpu_Regs[15] >= 0x4000)
				{
					cpu_Last_Bus_Value = Last_BIOS_Read;
				}
				else
				{
					// Update BIOS bus (entire word goes on the bus)
					Last_BIOS_Read = BIOS_32[(addr & 0x3FFC) >> 2];

					cpu_Last_Bus_Value = Last_BIOS_Read;
				}
			}
			else
			{
				cpu_Last_Bus_Value = (uint32_t)value | (value << 8) | (value << 16) | (value << 24);
			}
		}
	}

	void GBA_System::Update_Bus_Read_16(uint32_t addr, uint16_t value)
	{
		if (Update_Bus)
		{
			// Update IWRAM bus
			if ((addr & 0xFF000000) == 0x03000000)
			{
				cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
				cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);

				cpu_Last_Bus_Value = cpu_IWRAM_Last_Bus_Value;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];

				cpu_Last_Bus_Value = cpu_OAM_Last_Bus_Value;
			}
			else if (addr < 0x4000)
			{
				if (cpu_Regs[15] >= 0x4000)
				{
					cpu_Last_Bus_Value = Last_BIOS_Read;
				}
				else
				{
					// Update BIOS bus (entire word goes on the bus)
					Last_BIOS_Read = BIOS_32[(addr & 0x3FFC) >> 2];

					cpu_Last_Bus_Value = Last_BIOS_Read;
				}
			}
			else
			{
				cpu_Last_Bus_Value = (uint32_t)value | (value << 16);
			}
		}
	}

	void GBA_System::Update_Bus_Read_32(uint32_t addr, uint32_t value)
	{
		if (Update_Bus)
		{
			// Update IWRAM bus
			if ((addr & 0xFF000000) == 0x03000000)
			{
				cpu_IWRAM_Last_Bus_Value = value;
				cpu_Last_Bus_Value = cpu_IWRAM_Last_Bus_Value;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];
				cpu_Last_Bus_Value = cpu_OAM_Last_Bus_Value;
			}
			else if (addr < 0x4000)
			{
				if (cpu_Regs[15] >= 0x4000)
				{
					cpu_Last_Bus_Value = Last_BIOS_Read;
				}
				else
				{
					// Update BIOS bus (entire word goes on the bus)
					Last_BIOS_Read = BIOS_32[(addr & 0x3FFC) >> 2];

					cpu_Last_Bus_Value = Last_BIOS_Read;
				}
			}
			else
			{
				cpu_Last_Bus_Value = value;
			}
		}
	}

	void GBA_System::Update_Bus_Write_8(uint32_t addr, uint8_t value)
	{
		cpu_Last_Bus_Value &= (uint32_t)~(0xFF << (((int)addr & 3) * 8));
		cpu_Last_Bus_Value |= (uint32_t)value << (((int)addr & 3) * 8);

		// Update IWRAM bus
		if ((addr & 0xFF000000) == 0x03000000)
		{
			cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFF << (((int)addr & 3) * 8));
			cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 3) * 8);
		}

		// cannot write to BIOS or 8 bits to OAM
	}

	void GBA_System::Update_Bus_Write_16(uint32_t addr, uint16_t value)
	{
		cpu_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
		cpu_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
	
		if ((addr & 0xFF000000) == 0x03000000)
		{
			// Update IWRAM bus
			cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
			cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
		}
		else if ((addr & 0xFF000000) == 0x07000000)
		{
			// Update OAM bus (needs testing)
			cpu_OAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
			cpu_OAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
		}
	}

	void GBA_System::Update_Bus_Write_32(uint32_t addr, uint32_t value)
	{
		cpu_Last_Bus_Value = value;
		
		// Update IWRAM bus
		if ((addr & 0xFF000000) == 0x03000000)
		{
			cpu_IWRAM_Last_Bus_Value = value;
		}
		else if ((addr & 0xFF000000) == 0x07000000)
		{
			// Update OAM bus (needs testing)
			cpu_OAM_Last_Bus_Value = value;
		}
	}

	void GBA_System::Update_Bus_Read_16_DMA(uint32_t addr, uint16_t value, uint32_t chan)
	{
		if (Update_Bus)
		{
			// Update IWRAM bus
			if ((addr & 0xFF000000) == 0x03000000)
			{
				cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
				cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);

				dma_Last_Bus_Value[chan] = cpu_IWRAM_Last_Bus_Value;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];

				dma_Last_Bus_Value[chan] = cpu_OAM_Last_Bus_Value;
			}
			else
			{
				dma_Last_Bus_Value[chan] = (uint32_t)value | (value << 16);
			}

			// also update cpu bus
			cpu_Last_Bus_Value = dma_Last_Bus_Value[chan];
		}
	}

	void GBA_System::Update_Bus_Read_32_DMA(uint32_t addr, uint32_t value, uint32_t chan)
	{
		if (Update_Bus)
		{
			// Update IWRAM bus
			if ((addr & 0xFF000000) == 0x03000000)
			{
				cpu_IWRAM_Last_Bus_Value = value;
				dma_Last_Bus_Value[chan] = cpu_IWRAM_Last_Bus_Value;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];
				dma_Last_Bus_Value[chan] = cpu_OAM_Last_Bus_Value;
			}
			else
			{
				dma_Last_Bus_Value[chan] = value;
			}

			// also update cpu bus
			cpu_Last_Bus_Value = dma_Last_Bus_Value[chan];
		}
	}

	void GBA_System::Update_Bus_Write_16_DMA(uint32_t addr, uint16_t value, uint32_t chan)
	{
		cpu_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
		cpu_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);

		dma_Last_Bus_Value[chan] &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
		dma_Last_Bus_Value[chan] |= (uint32_t)value << (((int)addr & 2) * 8);

		if ((addr & 0xFF000000) == 0x03000000)
		{
			// Update IWRAM bus
			cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
			cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
		}
		else if ((addr & 0xFF000000) == 0x07000000)
		{
			// Update OAM bus (needs testing)
			cpu_OAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
			cpu_OAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
		}
	}

	void GBA_System::Update_Bus_Write_32_DMA(uint32_t addr, uint32_t value, uint32_t chan)
	{
		cpu_Last_Bus_Value = value;

		dma_Last_Bus_Value[chan] = value;

		// Update IWRAM bus
		if ((addr & 0xFF000000) == 0x03000000)
		{
			cpu_IWRAM_Last_Bus_Value = value;
		}
		else if ((addr & 0xFF000000) == 0x07000000)
		{
			// Update OAM bus (needs testing)
			cpu_OAM_Last_Bus_Value = value;
		}
	}
}