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

		if (ppu_pntr->DMA_bus_control)
		{
			// some of gekkio's tests require these to be accessible during DMA
			if (addr < 0x8000)
			{
				if (ppu_pntr->DMA_addr < 0x80)
				{
					return ppu_pntr->DMA_byte;
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
				if (ppu_pntr->DMA_OAM_access)
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

			return ppu_pntr->DMA_byte;
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
				if ((GB_bios_register & 0x1) == 0)
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

		if (ppu_pntr->DMA_bus_control)
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
			else if (addr >= 0xFE00 && addr < 0xFEA0 && ppu_pntr->DMA_OAM_access)
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
		if (ppu_pntr->DMA_bus_control)
		{
			// some of gekkio's tests require these to be accessible during DMA
			if (addr < 0x8000)
			{
				if (ppu_pntr->DMA_addr < 0x80)
				{
					return ppu_pntr->DMA_byte;
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
				if (ppu_pntr->DMA_OAM_access)
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

			return ppu_pntr->DMA_byte;
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
				if ((GB_bios_register & 0x1) == 0)
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

	#pragma region DMA Memory Map
	void GB_System::Read_Memory_16_DMA(uint32_t addr, uint32_t chan)
	{
		uint16_t ret = 0;

		// DMA always force aligned
		addr &= 0xFFFFFFFE;

		if (addr >= 0x08000000)
		{
			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;

			if (addr < 0x0D000000)
			{
				addr -= 0x08000000;
				ret = ROM_16[addr >> 1];
				dma_Last_Bus_Value[chan] = (uint32_t)((ret << 16) | ret);
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					addr -= 0x08000000;
					ret = ROM_16[addr >> 1];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						ret = (uint16_t)((dma_Last_Bus_Value[chan] & 0xFFFE) | mapper_pntr->Mapper_EEPROM_Read());
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							ret = (uint16_t)((dma_Last_Bus_Value[chan] & 0xFFFE) | mapper_pntr->Mapper_EEPROM_Read());
						}
						else
						{
							addr -= 0x08000000;
							ret = ROM_16[addr >> 1];
						}
					}
				}

				dma_Last_Bus_Value[chan] = (uint32_t)((ret << 16) | ret);
			}
			else if (addr < 0x10000000)
			{
				ret = mapper_pntr->Read_Memory_16(addr - 0x0E000000);
				dma_Last_Bus_Value[chan] = (uint32_t)((ret << 16) | ret);
			}
		}
		else if (addr >= 0x04000000)
		{
			if (addr >= 0x07000000)
			{
				ret = OAM_16[(addr & 0x3FE) >> 1];
			}
			else if (addr >= 0x06000000)
			{
				if ((addr & 0x00010000) == 0x00010000)
				{
					// mirrors behave differently depending on mode
					if ((addr & 0x00008000) == 0x00008000)
					{
						if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
						{
							ret = VRAM_16[(addr & 0x17FFE) >> 1];
						}
						else
						{
							ret = 0;
						}
					}
					else
					{
						ret = VRAM_16[(addr & 0x17FFE) >> 1];
					}
				}
				else
				{
					ret = VRAM_16[(addr & 0xFFFF) >> 1];
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr >= 0x05000000)
			{
				ret = PALRAM_16[(addr & 0x3FE) >> 1];

				ppu_PALRAM_In_Use = false;
			}
			else
			{
				if (addr < 0x04000800)
				{
					ret = Read_Registers_16(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFC) == 0x04000800)
				{
					switch (addr & 3)
					{
					case 0: ret = (uint16_t)(Memory_CTRL & 0xFFFF); break;
					default: ret = (uint16_t)((Memory_CTRL >> 16) & 0xFFFF); break;
					}
				}
				else
				{
					switch (addr & 3)
					{
					case 0: ret = (uint16_t)(cpu_Last_Bus_Value & 0xFFFF); break;
					default: ret = (uint16_t)((cpu_Last_Bus_Value >> 16) & 0xFFFF); break;
					}
				}
			}

			dma_Last_Bus_Value[chan] = (uint32_t)((ret << 16) | ret);
		}
		else if (addr >= 0x03000000)
		{
			ret = IWRAM_16[(addr & 0x7FFE) >> 1];
			dma_Last_Bus_Value[chan] = (uint32_t)((ret << 16) | ret);
		}
		else if (addr >= 0x02000000)
		{
			ret = WRAM_16[(addr & 0x3FFFE) >> 1];
			dma_Last_Bus_Value[chan] = (uint32_t)((ret << 16) | ret);
		}

		// DMA cannot access BIOS, nothing here	
	}

	void GB_System::Read_Memory_32_DMA(uint32_t addr, uint32_t chan)
	{
		// DMA always force aligned
		addr &= 0xFFFFFFFC;

		if (addr >= 0x08000000)
		{
			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;

			if (addr < 0x0D000000)
			{
				addr -= 0x08000000;
				dma_Last_Bus_Value[chan] = ROM_32[addr >> 2];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					addr -= 0x08000000;
					dma_Last_Bus_Value[chan] = ROM_32[addr >> 2];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						dma_Last_Bus_Value[chan] = (uint32_t)((dma_Last_Bus_Value[chan] & 0xFFFFFFFE) | mapper_pntr->Mapper_EEPROM_Read());
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							dma_Last_Bus_Value[chan] = (uint32_t)((dma_Last_Bus_Value[chan] & 0xFFFFFFFE) | mapper_pntr->Mapper_EEPROM_Read());
						}
						else
						{
							addr -= 0x08000000;
							dma_Last_Bus_Value[chan] = ROM_32[addr >> 2];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				dma_Last_Bus_Value[chan] = mapper_pntr->Read_Memory_32(addr - 0x0E000000);
			}
		}
		else if (addr >= 0x04000000)
		{
			if (addr >= 0x07000000)
			{
				dma_Last_Bus_Value[chan] = OAM_32[(addr & 0x3FC) >> 2];
			}
			else if (addr >= 0x06000000)
			{
				if ((addr & 0x00010000) == 0x00010000)
				{
					// mirrors behave differently depending on mode
					if ((addr & 0x00008000) == 0x00008000)
					{
						if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
						{
							dma_Last_Bus_Value[chan] = VRAM_32[(addr & 0x17FFC) >> 2];
						}
						else
						{
							dma_Last_Bus_Value[chan] = 0;
						}
					}
					else
					{
						dma_Last_Bus_Value[chan] = VRAM_32[(addr & 0x17FFC) >> 2];
					}
				}
				else
				{
					dma_Last_Bus_Value[chan] = VRAM_32[(addr & 0xFFFC) >> 2];
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr >= 0x05000000)
			{
				dma_Last_Bus_Value[chan] = PALRAM_32[(addr & 0x3FC) >> 2];

				ppu_PALRAM_In_Use = false;
			}
			else
			{
				if (addr < 0x04000800)
				{
					dma_Last_Bus_Value[chan] = Read_Registers_32(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFC) == 0x04000800)
				{
					dma_Last_Bus_Value[chan] = Memory_CTRL;
				}
				else
				{
					dma_Last_Bus_Value[chan] = cpu_Last_Bus_Value;
				}
			}
		}
		else if (addr >= 0x03000000)
		{
			dma_Last_Bus_Value[chan] = IWRAM_32[(addr & 0x7FFC) >> 2];
		}
		else if (addr >= 0x02000000)
		{
			dma_Last_Bus_Value[chan] = WRAM_32[(addr & 0x3FFFC) >> 2];
		}

		// DMA cannot access the BIOS, so nothing here	
	}

	void GB_System::Write_Memory_16_DMA(uint32_t addr, uint16_t value, uint32_t chan)
	{
		// DMA always force aligned
		addr &= 0xFFFFFFFE;

		if (addr < 0x03000000)
		{
			if (addr >= 0x02000000)
			{
				WRAM_16[(addr & 0x3FFFE) >> 1] = value;
			}
		}
		else if (addr < 0x04000000)
		{
			IWRAM_16[(addr & 0x7FFE) >> 1] = value;
		}
		else if (addr < 0x05000000)
		{
			if (addr < 0x04000800)
			{
				Write_Registers_16(addr - 0x04000000, value);
			}
			else if ((addr & 0x0400FFFC) == 0x04000800)
			{
				switch (addr & 3)
				{
				case 0x00: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0xFFFF0000) | value)); break;
				default: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0x0000FFFF) | (value << 16))); break;
				}
			}
		}
		else if (addr < 0x06000000)
		{
			PALRAM_16[(addr & 0x3FE) >> 1] = value;

			ppu_PALRAM_In_Use = false;
		}
		else if (addr < 0x07000000)
		{
			if ((addr & 0x00010000) == 0x00010000)
			{
				// mirrors behave differently depending on mode
				if ((addr & 0x00008000) == 0x00008000)
				{
					if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
					{
						VRAM_16[(addr & 0x17FFE) >> 1] = value;
					}
				}
				else
				{
					VRAM_16[(addr & 0x17FFE) >> 1] = value;
				}
			}
			else
			{
				VRAM_16[(addr & 0xFFFE) >> 1] = value;
			}

			ppu_VRAM_High_In_Use = false;
			ppu_VRAM_In_Use = false;
		}
		else if (addr < 0x08000000)
		{
			OAM_16[(addr & 0x3FE) >> 1] = value;
		}
		else if (addr < 0x0D000000)
		{
			mapper_pntr->Write_ROM_16(addr, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if ((addr >= 0x0D000000) && (addr < 0x0E000000))
		{
			if (Is_EEPROM)
			{
				if (EEPROM_Wiring)
				{
					mapper_pntr->Mapper_EEPROM_Write((uint8_t)value);
				}
				else
				{
					if ((addr & 0xDFFFE00) == 0xDFFFE00)
					{
						mapper_pntr->Mapper_EEPROM_Write((uint8_t)value);
					}
				}
			}

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if ((addr >= 0x0E000000) && (addr < 0x10000000))
		{
			mapper_pntr->Write_Memory_16(addr - 0x0E000000, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
	}

	void GB_System::Write_Memory_32_DMA(uint32_t addr, uint32_t value, uint32_t chan)
	{
		// DMA always force aligned
		addr &= 0xFFFFFFFC;

		if (addr < 0x03000000)
		{
			if (addr >= 0x02000000)
			{
				WRAM_32[(addr & 0x3FFFC) >> 2] = value;
			}
		}
		else if (addr < 0x04000000)
		{
			IWRAM_32[(addr & 0x7FFC) >> 2] = value;
		}
		else if (addr < 0x05000000)
		{
			if (addr < 0x04000800)
			{
				Write_Registers_32(addr - 0x04000000, value);
			}
			else if ((addr & 0x0400FFFC) == 0x04000800)
			{
				Update_Memory_CTRL(value);
			}
		}
		else if (addr < 0x06000000)
		{
			PALRAM_32[(addr & 0x3FC) >> 2] = value;

			ppu_PALRAM_In_Use = false;
			PALRAM_32_Check = false;
		}
		else if (addr < 0x07000000)
		{
			if ((addr & 0x00010000) == 0x00010000)
			{
				// mirrors behave differently depending on mode
				if ((addr & 0x00008000) == 0x00008000)
				{
					if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
					{
						VRAM_32[(addr & 0x17FFC) >> 2] = value;
					}
				}
				else
				{
					VRAM_32[(addr & 0x17FFC) >> 2] = value;
				}
			}
			else
			{
				VRAM_32[(addr & 0xFFFC) >> 2] = value;
			}

			ppu_VRAM_High_In_Use = false;
			ppu_VRAM_In_Use = false;
			VRAM_32_Check = false;
		}
		else if (addr < 0x08000000)
		{
			OAM_32[(addr & 0x3FC) >> 2] = value;
		}
		else if (addr < 0x0D000000)
		{
			mapper_pntr->Write_ROM_32(addr, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if ((addr >= 0x0D000000) && (addr < 0x0E000000))
		{
			if (Is_EEPROM)
			{
				if (EEPROM_Wiring)
				{
					mapper_pntr->Mapper_EEPROM_Write((uint8_t)value);
				}
				else
				{
					if ((addr & 0xDFFFE00) == 0xDFFFE00)
					{
						mapper_pntr->Mapper_EEPROM_Write((uint8_t)value);
					}
				}
			}

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if ((addr >= 0x0E000000) && (addr < 0x10000000))
		{
			mapper_pntr->Write_Memory_32(addr - 0x0E000000, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
	}
	#pragma endregion
}