#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "GB_System.h"
#include "Mappers.h"
#include "PPUs.h"

/*
	$FFFF          Interrupt Enable Flag
	$FF80-$FFFE    Zero Page - 127 bytes
	$FF00-$FF7F    Hardware I/O Registers
	$FEA0-$FEFF    Unusable Memory
	$FE00-$FE9F    OAM - Object Attribute Memory
	$E000-$FDFF    Echo RAM - Reserved, Do Not Use
	$D000-$DFFF    Internal RAM - Bank 1-7 (switchable - CGB only)
	$C000-$CFFF    Internal RAM - Bank 0 (fixed)
	$A000-$BFFF    Cartridge RAM (If Available)
	$9C00-$9FFF    BG Map Data 2
	$9800-$9BFF    BG Map Data 1
	$8000-$97FF    Character RAM
	$4000-$7FFF    Cartridge ROM - Switchable Banks 1-xx
	$0150-$3FFF    Cartridge ROM - Bank 0 (fixed)
	$0100-$014F    Cartridge Header Area
	$0000-$00FF    Restart and Interrupt Vectors
*/

namespace GBHawk
{
	#pragma region Memory Map

	uint8_t GB_System::Read_Memory(uint16_t addr)
	{
		addr_access = addr;

		if (dma_Bus_Control)
		{
			// some of gekkio's tests require these to be accessible during DMA
			if (addr < 0x8000)
			{
				if (dma_Addr < 0x80)
				{
					return dma_Byte;
				}

				bus_value = mapper_pntr->ReadMemoryLow(addr);
				bus_access_time = Cycle_Count;
				return bus_value;
			}

			if (addr >= 0xA000 && addr < 0xC000)
			{
				// on GBC only, cart is accessible during DMA
				bus_value = mapper_pntr->ReadMemoryHigh(addr);
				bus_access_time = Cycle_Count;
				return bus_value;
			}

			if (addr >= 0xE000 && addr < 0xF000)
			{
				return RAM[addr - 0xE000];
			}

			if (addr >= 0xF000 && addr < 0xFE00)
			{
				//if (RAM_read[(RAM_Bank * 0x1000) + (addr - 0xF000)] == 0) { Console.WriteLine("RAM: " + addr + " " + cpu.TotalExecutedCycles); }
				return RAM[(RAM_Bank * 0x1000) + (addr - 0xF000)];
			}

			if (addr >= 0xFE00 && addr < 0xFEA0)
			{
				if (dma_OAM_Access)
				{
					return OAM[addr - 0xFE00];
				}
				else
				{
					return 0xFF;
				}
			}

			if (addr >= 0xFF00 && addr < 0xFF80) // The game GOAL! Requires Hardware Regs to be accessible
			{
				return Read_Registers(addr);
			}

			if (addr >= 0xFF80)
			{
				if (addr != 0xFFFF)
				{
					//if (ZP_RAM_read[addr - 0xFF80] == 0) { Console.WriteLine("ZP: " + (addr - 0xFF80) + " " + cpu.TotalExecutedCycles); }
					return ZP_RAM[addr - 0xFF80];
				}
				else
				{
					return Read_Registers(addr);
				}
			}

			return dma_Byte;
		}

		if (addr < 0x8000)
		{
			if (addr >= 0x900)
			{
				bus_value = mapper_pntr->ReadMemoryLow(addr);
				bus_access_time = Cycle_Count;
				return bus_value;
			}

			if (addr < 0x100)
			{
				// return Either BIOS ROM or Game ROM
				if ((GB_bios_register & 0x1) == 0)
				{
					return BIOS[addr]; // Return BIOS
				}

				bus_value = mapper_pntr->ReadMemoryLow(addr);
				bus_access_time = Cycle_Count;
				return bus_value;
			}

			if (addr >= 0x200)
			{
				// return Either BIOS ROM or Game ROM
				if (((GB_bios_register & 0x1) == 0) && Is_GBC)
				{
					return BIOS[addr]; // Return BIOS
				}

				bus_value = mapper_pntr->ReadMemoryLow(addr);
				bus_access_time = Cycle_Count;
				return bus_value;
			}

			bus_value = mapper_pntr->ReadMemoryLow(addr);
			bus_access_time = Cycle_Count;
			return bus_value;
		}

		if (addr < 0xA000)
		{
			if (ppu_pntr->VRAM_access_read)
			{
				return VRAM[VRAM_Bank * 0x2000 + (addr - 0x8000)];
			}

			if (!HDMA_Transfer)
			{
				if (ppu_pntr->pixel_counter == 160)
				{
					/*
					Console.WriteLine("VRAM Glitch " + cpu.TotalExecutedCycles + " " + ppu_pntr->bus_address + " " +
						VRAM[ppu_pntr->bus_address] + " " + ppu_pntr->read_case_prev + " " + (ppu_pntr->internal_cycle & 1) + " " +
						(VRAM_Bank * 0x2000 + (addr - 0x8000)) + " " + VRAM[VRAM_Bank * 0x2000 + (addr - 0x8000)]);					
					*/

					// TODO: This is a complicated case because the PPU is accessing 2 areas of VRAM at the same time.
					if ((ppu_pntr->read_case_prev == 0) || (ppu_pntr->read_case_prev == 4))
					{
						if ((VRAM_Bank * 0x2000 + (addr - 0x8000)) < 0x3800)
						{
							return VRAM[VRAM_Bank * 0x2000 + (addr - 0x8000)];
						}
						return VRAM[ppu_pntr->bus_address];
					}

					// TODO: What is returned when the ppu isn't accessing VRAM?
					//if ((ppu_pntr->read_case_prev == 3) || (ppu_pntr->read_case_prev == 7))
					//{
					//	return VRAM[VRAM_Bank * 0x2000 + (addr - 0x8000)];
					//}
					return VRAM[ppu_pntr->bus_address];
				}
				return 0xFF;
			}
			else
			{
				return 0xFF;
			}
		}

		if (addr < 0xC000)
		{
			bus_value = mapper_pntr->ReadMemoryHigh(addr);
			bus_access_time = Cycle_Count;
			return bus_value;
		}

		if (addr < 0xFE00)
		{
			addr = (uint16_t)(RAM_Bank * (addr & 0x1000) + (addr & 0xFFF));
			//if (RAM_read[addr] == 0) { Console.WriteLine("RAM: " + addr + " " + cpu.TotalExecutedCycles); }
			return RAM[addr];
		}

		if (addr < 0xFF00)
		{
			if (addr < 0xFEA0)
			{
				if (ppu_pntr->OAM_access_read)
				{
					return OAM[addr - 0xFE00];
				}

				return 0xFF;
			}

			// unmapped memory, return depends on console and rendering
			if (Is_GBC_GBA)
			{
				// in GBA mode, it returns a reflection of the address somehow
				if (ppu_pntr->OAM_access_read)
				{
					return (uint8_t)((addr & 0xF0) | ((addr & 0xF0) >> 4));
				}

				return 0xFF;
			}
			else
			{
				// otherwise the return value is revision dependent. Assume CGB-E is same as GBA mode consoles for now
				if (ppu_pntr->OAM_access_read)
				{
					return (uint8_t)((addr & 0xF0) | ((addr & 0xF0) >> 4));
				}

				return 0xFF;
			}
		}

		if (addr < 0xFF80)
		{
			return Read_Registers(addr);
		}

		if (addr < 0xFFFF)
		{
			//if (ZP_RAM_read[addr - 0xFF80] == 0) { Console.WriteLine("ZP: " + (addr - 0xFF80) + " " + cpu.TotalExecutedCycles); }
			return ZP_RAM[addr - 0xFF80];
		}

		return Read_Registers(addr);
	}

	void GB_System::Write_Memory(uint16_t addr, uint8_t value)
	{
		addr_access = addr;

		if (dma_Bus_Control)
		{
			// some of gekkio's tests require this to be accessible during DMA
			if (addr >= 0xA000 && addr < 0xC000)
			{
				// on GBC only, cart is accessible during DMA
				bus_value = value;
				bus_access_time = Cycle_Count;
				mapper_pntr->WriteMemory(addr, value);
			}

			if (addr >= 0xE000 && addr < 0xF000)
			{
				//RAM_read[addr - 0xE000] = 1;
				RAM[addr - 0xE000] = value;
			}
			else if (addr >= 0xF000 && addr < 0xFE00)
			{
				//RAM_read[RAM_Bank * 0x1000 + (addr - 0xF000)] = 1;
				RAM[RAM_Bank * 0x1000 + (addr - 0xF000)] = value;
			}
			else if (addr >= 0xFE00 && addr < 0xFEA0 && dma_OAM_Access)
			{
				OAM[addr - 0xFE00] = value;
			}
			else if (addr >= 0xFF00 && addr < 0xFF80) // The game GOAL! Requires Hardware Regs to be accessible
			{
				Write_Registers(addr, value);
			}
			else if (addr >= 0xFF80)
			{
				if (addr != 0xFFFF)
				{
					//ZP_RAM_read[addr - 0xFF80] = 1;
					ZP_RAM[addr - 0xFF80] = value;
				}
				else
				{
					Write_Registers(addr, value);
				}
			}

			return;
		}

		// Writes are more likely from the top down
		if (addr >= 0xFF00)
		{
			if (addr < 0xFF80)
			{
				Write_Registers(addr, value);
			}
			else if (addr < 0xFFFF)
			{
				//ZP_RAM_read[addr - 0xFF80] = 1;
				ZP_RAM[addr - 0xFF80] = value;
			}
			else
			{
				Write_Registers(addr, value);
			}
		}
		else if (addr >= 0xFE00)
		{
			if (addr < 0xFEA0)
			{
				if (ppu_pntr->OAM_access_write) { OAM[addr - 0xFE00] = value; }
			}
			// unmapped memory writes depend on console		
			else
			{
				if (Is_GBC_GBA)
				{
					// in GBA mode, writes have no effect as far as tested, might need more thorough tests
				}
				else
				{
					// otherwise it's revision dependent. Assume CGB-E is same as GBA mode consoles for now
				}
			}
		}
		else if (addr >= 0xC000)
		{
			addr = (uint16_t)(RAM_Bank * (addr & 0x1000) + (addr & 0xFFF));
			//RAM_read[addr] = 1;
			RAM[addr] = value;
		}
		else if (addr >= 0xA000)
		{
			bus_value = value;
			bus_access_time = Cycle_Count;
			mapper_pntr->WriteMemory(addr, value);
		}
		else if (addr >= 0x8000)
		{
			if (ppu_pntr->VRAM_access_write)
			{
				VRAM[(VRAM_Bank * 0x2000) + (addr - 0x8000)] = value;
			}
		}
		else
		{
			if (addr >= 0x900)
			{
				bus_value = value;
				bus_access_time = Cycle_Count;
				mapper_pntr->WriteMemory(addr, value);
			}
			else
			{
				if (addr < 0x100)
				{
					if ((GB_bios_register & 0x1) == 0)
					{
						// No Writing to BIOS
					}
					else
					{
						bus_value = value;
						bus_access_time = Cycle_Count;
						mapper_pntr->WriteMemory(addr, value);
					}
				}
				else if (addr >= 0x200)
				{
					if (((GB_bios_register & 0x1) == 0) && Is_GBC)
					{
						// No Writing to BIOS
					}
					else
					{
						bus_value = value;
						bus_access_time = Cycle_Count;
						mapper_pntr->WriteMemory(addr, value);
					}
				}
				else
				{
					bus_value = value;
					bus_access_time = Cycle_Count;
					mapper_pntr->WriteMemory(addr, value);
				}
			}
		}
	}

	uint8_t GB_System::Peek_Memory(uint16_t addr)
	{
		if (dma_Bus_Control)
		{
			// some of gekkio's tests require these to be accessible during DMA
			if (addr < 0x8000)
			{
				if (dma_Addr < 0x80)
				{
					return dma_Byte;
				}

				return mapper_pntr->PeekMemoryLow(addr);
			}

			if ((addr >= 0xA000) && (addr < 0xC000))
			{
				// on GBC only, cart is accessible during DMA
				return mapper_pntr->PeekMemoryHigh(addr);
			}

			if (addr >= 0xE000 && addr < 0xF000)
			{
				return RAM[addr - 0xE000];
			}

			if (addr >= 0xF000 && addr < 0xFE00)
			{
				return RAM[(RAM_Bank * 0x1000) + (addr - 0xF000)];
			}

			if (addr >= 0xFE00 && addr < 0xFEA0)
			{
				if (dma_OAM_Access)
				{
					return OAM[addr - 0xFE00];
				}
				else
				{
					return 0xFF;
				}
			}

			if (addr >= 0xFF00 && addr < 0xFF80) // The game GOAL! Requires Hardware Regs to be accessible
			{
				return Read_Registers(addr);
			}

			if (addr >= 0xFF80)
			{
				if (addr != 0xFFFF)
				{
					return ZP_RAM[addr - 0xFF80];
				}
				else
				{
					return Read_Registers(addr);
				}
			}

			return dma_Byte;
		}

		if (addr < 0x8000)
		{
			if (addr >= 0x900)
			{
				return mapper_pntr->PeekMemoryLow(addr);
			}

			if (addr < 0x100)
			{
				// return Either BIOS ROM or Game ROM
				if ((GB_bios_register & 0x1) == 0)
				{
					return BIOS[addr]; // Return BIOS
				}

				return mapper_pntr->PeekMemoryLow(addr);
			}

			if (addr >= 0x200)
			{
				// return Either BIOS ROM or Game ROM
				if (((GB_bios_register & 0x1) == 0) && Is_GBC)
				{
					return BIOS[addr]; // Return BIOS
				}

				return mapper_pntr->PeekMemoryLow(addr);
			}

			return mapper_pntr->PeekMemoryLow(addr);
		}

		if (addr < 0xA000)
		{
			if (ppu_pntr->VRAM_access_read)
			{
				return VRAM[(VRAM_Bank * 0x2000) + (addr - 0x8000)];
			}

			return 0xFF;
		}

		if (addr < 0xC000)
		{
			return mapper_pntr->PeekMemoryHigh(addr);
		}

		if (addr < 0xFE00)
		{
			addr = (uint16_t)(RAM_Bank * (addr & 0x1000) + (addr & 0xFFF));
			return RAM[addr];
		}

		if (addr < 0xFF00)
		{
			if (addr < 0xFEA0)
			{
				if (ppu_pntr->OAM_access_read)
				{
					return OAM[addr - 0xFE00];
				}

				return 0xFF;
			}

			// unmapped memory, return depends on console and rendering
			if (Is_GBC_GBA)
			{
				// in GBA mode, it returns a reflection of the address somehow
				if (ppu_pntr->OAM_access_read)
				{
					return (uint8_t)((addr & 0xF0) | ((addr & 0xF0) >> 4));
				}

				return 0xFF;
			}
			else
			{
				// otherwise the return value is revision dependent. Assume CGB-E is same as GBA mode consoles for now
				if (ppu_pntr->OAM_access_read)
				{
					return (uint8_t)((addr & 0xF0) | ((addr & 0xF0) >> 4));
				}

				return 0xFF;
			}
		}

		if (addr < 0xFF80)
		{
			return Read_Registers(addr);
		}

		if (addr < 0xFFFF)
		{
			return ZP_RAM[addr - 0xFF80];
		}

		return Read_Registers(addr);
	}

	int8_t GB_System::Peek_Memory_Signed(uint16_t addr)
	{
		return (int8_t)Peek_Memory(addr);
	}

	uint16_t GB_System::Peek_Memory_16(uint16_t addr)
	{
		return (uint16_t)(Peek_Memory(addr) | (Peek_Memory((uint16_t)(addr + 1)) << 8));
	}

	#pragma endregion
}