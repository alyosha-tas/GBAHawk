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
	#pragma region Memory Map

	uint8_t GBA_System::Read_Memory_8(uint32_t addr)
	{
		uint8_t ret = 0;

		Update_Bus = true;

		if (addr >= 0x08000000)
		{
			if (addr < 0x0D000000)
			{
				ret = ROM[(addr - 0x08000000)];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					ret = ROM[(addr - 0x08000000)];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						ret = (uint8_t)((cpu_Last_Bus_Value & 0xFE) | mapper_pntr->Mapper_EEPROM_Read());
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							ret = (uint8_t)((cpu_Last_Bus_Value & 0xFE) | mapper_pntr->Mapper_EEPROM_Read());
						}
						else
						{
							ret = ROM[(addr - 0x08000000)];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				ret = mapper_pntr->Read_Memory_8(addr - 0x0E000000);
			}
			else
			{
				ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (uint32_t)(addr & 3))) & 0xFF); // open bus
				Update_Bus = false;
			}

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if (addr >= 0x04000000)
		{
			if (addr >= 0x07000000)
			{
				ret = OAM[addr & 0x3FF];
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
							ret = VRAM[addr & 0x17FFF];
						}
						else
						{
							ret = 0;
						}
					}
					else
					{
						ret = VRAM[addr & 0x17FFF];
					}
				}
				else
				{
					ret = VRAM[addr & 0xFFFF];
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr >= 0x05000000)
			{
				ret = PALRAM[addr & 0x3FF];

				ppu_PALRAM_In_Use = false;
			}
			else
			{
				if (addr < 0x04000800)
				{
					ret = Read_Registers_8(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFC) == 0x04000800)
				{
					switch (addr & 3)
					{
						case 0: ret = (uint8_t)(Memory_CTRL & 0xFF); break;
						case 1: ret = (uint8_t)((Memory_CTRL >> 8) & 0xFF); break;
						case 2: ret = (uint8_t)((Memory_CTRL >> 16) & 0xFF); break;
						default: ret = (uint8_t)((Memory_CTRL >> 24) & 0xFF); break;
					}
				}
				else
				{
					ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus
					Update_Bus = false;
				}
			}
		}
		else
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr >= 0x03000000)
			{
				ret = IWRAM[addr & 0x7FFF];
				Update_IWRAM_Bus = true;
			}
			else if (addr >= 0x02000000)
			{
				if (WRAM_Enable)
				{
					ret = WRAM[addr & 0x3FFFF];
				}
				else
				{
					ret = IWRAM[addr & 0x7FFF];
					Update_IWRAM_Bus = true;
				}
			}
			else if (addr < 0x4000)
			{
				if (RAM_BIOS_Swap)
				{
					In_BIOS_Region = ((cpu_Regs[15] >= 0x2000000) && (cpu_Regs[15] <= 0x2003FFF));
				}
				else
				{
					In_BIOS_Region = (cpu_Regs[15] <= 0x3FFF);
				}
				
				// BIOS is protected against reading from memory beyond the BIOS range
				if (!In_BIOS_Region)
				{
					ret = (uint8_t)((Last_BIOS_Read >> (8 * (int)(addr & 3))) & 0xFF);
				}
				else
				{
					ret = BIOS[addr];
				}
			}
			else
			{
				ret = (uint8_t)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus
				Update_Bus = false;
			}
		}

		Update_Bus_Read_8(addr, ret);

		return ret;
	}

	uint16_t GBA_System::Read_Memory_16(uint32_t addr)
	{
		uint16_t ret = 0;

		Update_Bus = true;

		if (addr >= 0x08000000)
		{
			if (addr < 0x0D000000)
			{
				// Forced Align
				ret = ROM_16[(addr - 0x08000000) >> 1];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					// Forced Align
					ret = ROM_16[(addr - 0x08000000) >> 1];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						ret = (uint16_t)((cpu_Last_Bus_Value & 0xFFFE) | mapper_pntr->Mapper_EEPROM_Read());
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							ret = (uint16_t)((cpu_Last_Bus_Value & 0xFFFE) | mapper_pntr->Mapper_EEPROM_Read());
						}
						else
						{
							// Forced Align
							ret = ROM_16[(addr - 0x08000000) >> 1];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				ret = mapper_pntr->Read_Memory_16(addr - 0x0E000000);
			}
			else
			{
				ret = (uint16_t)((cpu_Last_Bus_Value >> (8 * (int)(addr & 2))) & 0xFFFF); // open bus
				Update_Bus = false;
			}

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if (addr >= 0x04000000)
		{
			// Forced Align
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
					ret = VRAM_16[(addr & 0xFFFE) >> 1];
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
					// Forced Align
					addr &= 0xFFFFFFFE;

					ret = Read_Registers_16(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFC) == 0x04000800)
				{
					// Forced Align
					addr &= 0xFFFFFFFE;

					ret = (uint16_t)((Memory_CTRL >> ((addr & 2) * 8)) & 0xFFFF);
				}
				else
				{
					ret = (uint16_t)((cpu_Last_Bus_Value >> (8 * (int)(addr & 2))) & 0xFFFF); // open bus
					Update_Bus = false;
				}
			}
		}
		else
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr >= 0x03000000)
			{
				// Forced Align
				ret = IWRAM_16[(addr & 0x7FFE) >> 1];
				Update_IWRAM_Bus = true;
			}
			else if (addr >= 0x02000000)
			{
				if (WRAM_Enable)
				{
					// Forced Align
					ret = WRAM_16[(addr & 0x3FFFE) >> 1];
				}
				else
				{
					// Forced Align
					ret = IWRAM_16[(addr & 0x7FFE) >> 1];
					Update_IWRAM_Bus = true;
				}
			}
			else if (addr < 0x4000)
			{
				if (RAM_BIOS_Swap)
				{
					In_BIOS_Region = ((cpu_Regs[15] >= 0x2000000) && (cpu_Regs[15] <= 0x2003FFF));
				}
				else
				{
					In_BIOS_Region = (cpu_Regs[15] <= 0x3FFF);
				}
				
				// BIOS is protected against reading from memory beyond the BIOS range
				if (!In_BIOS_Region)
				{
					if ((addr & 2) == 0)
					{
						ret = (uint16_t)(Last_BIOS_Read & 0xFFFF);
					}
					else
					{
						ret = (uint16_t)((Last_BIOS_Read >> 16) & 0xFFFF);
					}
				}
				else
				{
					// Forced Align
					ret = BIOS_16[addr >> 1];
				}
			}
			else
			{
				ret = (uint16_t)((cpu_Last_Bus_Value >> (8 * (int)(addr & 2))) & 0xFFFF); // open bus
				Update_Bus = false;
			}
		}

		Update_Bus_Read_16(addr, ret);

		return ret;
	}

	uint32_t GBA_System::Read_Memory_32(uint32_t addr)
	{
		uint32_t ret = 0;

		Update_Bus = true;

		if (addr >= 0x08000000)
		{		
			if (addr < 0x0D000000)
			{
				// Forced Align
				ret = ROM_32[(addr - 0x08000000) >> 2];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					// Forced Align
					ret = ROM_32[(addr - 0x08000000) >> 2];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						ret = (uint32_t)((cpu_Last_Bus_Value & 0xFFFFFFFE) | mapper_pntr->Mapper_EEPROM_Read());
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							ret = (uint32_t)((cpu_Last_Bus_Value & 0xFFFFFFFE) | mapper_pntr->Mapper_EEPROM_Read());
						}
						else
						{
							// Forced Align
							ret = ROM_32[(addr - 0x08000000) >> 2];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				ret = mapper_pntr->Read_Memory_32(addr - 0x0E000000);
			}
			else
			{
				ret = cpu_Last_Bus_Value; // open bus	
				Update_Bus = false;
			}

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if (addr >= 0x04000000)
		{
			// Forced Align
			if (addr >= 0x07000000)
			{
				ret = OAM_32[(addr & 0x3FC) >> 2];
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
							ret = VRAM_32[(addr & 0x17FFC) >> 2];
						}
						else
						{
							ret = 0;
						}
					}
					else
					{
						ret = VRAM_32[(addr & 0x17FFC) >> 2];
					}
				}
				else
				{
					ret = VRAM_32[(addr & 0xFFFC) >> 2];
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr >= 0x05000000)
			{
				ret = PALRAM_32[(addr & 0x3FC) >> 2];

				ppu_PALRAM_In_Use = false;
			}
			else
			{
				if (addr < 0x04000800)
				{
					// Forced Align
					addr &= 0xFFFFFFFC;

					ret = Read_Registers_32(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFC) == 0x04000800)
				{
					// Forced Align
					addr &= 0xFFFFFFFC;

					ret = Memory_CTRL;
				}
				else
				{
					ret = cpu_Last_Bus_Value; // open bus
					Update_Bus = false;
				}
			}
		}
		else
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr >= 0x03000000)
			{
				// Forced Align
				ret = IWRAM_32[(addr & 0x7FFC) >> 2];
				Update_IWRAM_Bus = true;
			}
			else if (addr >= 0x02000000)
			{
				if (WRAM_Enable)
				{
					// Forced Align
					ret = WRAM_32[(addr & 0x3FFFC) >> 2];
				}
				else
				{
					// Forced Align
					ret = IWRAM_32[(addr & 0x7FFC) >> 2];
					Update_IWRAM_Bus = true;
				}
			}
			else if (addr < 0x4000)
			{
				if (RAM_BIOS_Swap)
				{
					In_BIOS_Region = ((cpu_Regs[15] >= 0x2000000) && (cpu_Regs[15] <= 0x2003FFF));
				}
				else
				{
					In_BIOS_Region = (cpu_Regs[15] <= 0x3FFF);
				}
				
				// BIOS is protected against reading from memory beyond the BIOS range
				if (!In_BIOS_Region)
				{
					ret = Last_BIOS_Read;
				}
				else
				{
					// Forced Align
					ret = BIOS_32[addr >> 2];
				}
			}
			else
			{
				ret = cpu_Last_Bus_Value; // open bus
				Update_Bus = false;
			}
		}

		Update_Bus_Read_32(addr, ret);

		return ret;
	}

	void GBA_System::Write_Memory_8(uint32_t addr, uint8_t value)
	{
		if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					if (WRAM_Enable)
					{
						WRAM[addr & 0x3FFFF] = value;
					}
					else
					{
						IWRAM[addr & 0x7FFF] = value;
						Update_IWRAM_Bus = true;
					}
				}
			}
			else if (addr < 0x04000000)
			{
				IWRAM[addr & 0x7FFF] = value;
				Update_IWRAM_Bus = true;
			}
		}
		else if (addr < 0x05000000)
		{
			if (addr < 0x04000800)
			{
				Write_Registers_8(addr - 0x04000000, value);
			}
			else if ((addr & 0x0400FFFC) == 0x04000800)
			{
				switch (addr & 3)
				{
					case 0x00: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0xFFFFFF00) | value)); break;
					case 0x01: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0xFFFF00FF) | (value << 8))); break;
					case 0x02: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0xFF00FFFF) | (value << 16))); break;
					default: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0x00FFFFFF) | (value << 24))); break;
				}
			}
		}
		else if (addr < 0x06000000)
		{
			// 8 bit writes to PALRAM stored as halfword
			PALRAM[addr & 0x3FE] = value;
			PALRAM[(addr & 0x3FE) + 1] = value;

			ppu_PALRAM_In_Use = false;
		}
		else if (addr < 0x07000000)
		{
			if ((addr & 0x00010000) == 0x00010000)
			{
				// 8 bit writes ignored depending on mode
				// Seems like it has to do with what region is also used by sprites
				// bitmap modes (3-5) allow writes up to 0x14000, as this is reserved for BGs
				// other modes do not allow writes above 0x10000, as all of this is reserved for sprites
				// This effects the Quake demo

				// mirrors behave differently depending on mode
				if ((addr & 0x00008000) == 0x00008000)
				{
					if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
					{
						if ((addr & 0x17FFF) < 0x14000)
						{
							// 8 bit writes stored as halfword
							VRAM[addr & 0x17FFE] = value;
							VRAM[(addr & 0x17FFE) + 1] = value;
						}
					}
				}
				else
				{
					if (ppu_BG_Mode >= 3)
					{
						if ((addr & 0x17FFF) < 0x14000)
						{
							// 8 bit writes stored as halfword
							VRAM[addr & 0x17FFE] = value;
							VRAM[(addr & 0x17FFE) + 1] = value;
						}
					}
				}
			}
			else
			{
				// 8 bit writes stored as halfword
				VRAM[addr & 0xFFFE] = value;
				VRAM[(addr & 0xFFFE) + 1] = value;
			}

			ppu_VRAM_High_In_Use = false;
			ppu_VRAM_In_Use = false;
		}
		else if (addr < 0x08000000)
		{
			// 8 bit writes to OAM ignored
			// OAM[addr & 0x3FF] = value;
		}
		else if (addr < 0x0D000000)
		{
			mapper_pntr->Write_ROM_8(addr, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if (addr < 0x0E000000)
		{
			if (Is_EEPROM)
			{
				if (EEPROM_Wiring)
				{
					mapper_pntr->Mapper_EEPROM_Write(value);
				}
				else
				{
					if ((addr & 0xDFFFE00) == 0xDFFFE00)
					{
						mapper_pntr->Mapper_EEPROM_Write(value);
					}
				}
			}
			else
			{
				mapper_pntr->Write_ROM_8(addr, value);
			}

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if ((addr >= 0x0E000000) && (addr < 0x10000000))
		{
			mapper_pntr->Write_Memory_8(addr - 0x0E000000, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}

		Update_Bus_Write_8(addr, value);
	}

	void GBA_System::Write_Memory_16(uint32_t addr, uint16_t value)
	{
		if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }

			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					if (WRAM_Enable)
					{
						// Forced Align
						WRAM_16[(addr & 0x3FFFE) >> 1] = value;
					}
					else
					{
						// Forced Align
						IWRAM_16[(addr & 0x7FFE) >> 1] = value;
						Update_IWRAM_Bus = true;
					}
				}
			}
			else if (addr < 0x04000000)
			{
				// Forced Align
				IWRAM_16[(addr & 0x7FFE) >> 1] = value;
				Update_IWRAM_Bus = true;
			}
		}
		else if (addr < 0x05000000)
		{
			// Forced Align
			addr &= 0xFFFFFFFE;

			if (addr < 0x04000800)
			{
				Write_Registers_16(addr - 0x04000000, value);
			}
			else if ((addr & 0x0400FFFC) == 0x04000800)
			{
				switch (addr & 2)
				{
					case 0x00: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0xFFFF0000) | value)); break;
					default: Update_Memory_CTRL((uint32_t)((Memory_CTRL & 0x0000FFFF) | (value << 16))); break;
				}
			}
		}
		else if (addr < 0x06000000)
		{
			// Forced Align
			PALRAM_16[(addr & 0x3FE) >> 1] = value;

			ppu_PALRAM_In_Use = false;
		}
		else if (addr < 0x07000000)
		{
			// Forced Align
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
			// Forced Align
			OAM_16[(addr & 0x3FE) >> 1] = value;
		}
		else if (addr < 0x0D000000)
		{
			mapper_pntr->Write_ROM_16(addr, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if (addr < 0x0E000000)
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
			else
			{
				mapper_pntr->Write_ROM_16(addr, value);
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

		Update_Bus_Write_16(addr, value);
	}

	void GBA_System::Write_Memory_32(uint32_t addr, uint32_t value)
	{
		if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					if (WRAM_Enable)
					{
						// Forced Align
						WRAM_32[(addr & 0x3FFFC) >> 2] = value;
					}
					else
					{
						// Forced Align
						IWRAM_32[(addr & 0x7FFC) >> 2] = value;
						Update_IWRAM_Bus = true;
					}
				}
			}
			else if (addr < 0x04000000)
			{
				// Forced Align
				IWRAM_32[(addr & 0x7FFC) >> 2] = value;
				Update_IWRAM_Bus = true;
			}
		}
		else if (addr < 0x05000000)
		{
			// Forced Align
			addr &= 0xFFFFFFFC;

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
			// Forced Align
			PALRAM_32[(addr & 0x3FC) >> 2] = value;

			ppu_PALRAM_In_Use = false;
			PALRAM_32_Check = false;
		}
		else if (addr < 0x07000000)
		{
			// Forced Align
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
			// Forced Align
			OAM_32[(addr & 0x3FC) >> 2] = value;
		}
		else if (addr < 0x0D000000)
		{
			mapper_pntr->Write_ROM_32(addr, value);

			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;
		}
		else if (addr < 0x0E000000)
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
			else
			{
				mapper_pntr->Write_ROM_32(addr, value);
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

		Update_Bus_Write_32(addr, value);
	}

	uint8_t GBA_System::Peek_Memory_8(uint32_t addr)
	{
		if (addr >= 0x08000000)
		{
			if (addr < 0x0D000000)
			{
				return ROM[addr - 0x08000000];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					return ROM[addr - 0x08000000];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						return (uint8_t)mapper_pntr->Mapper_EEPROM_Read();
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							return (uint8_t)mapper_pntr->Mapper_EEPROM_Read();
						}
						else
						{
							return ROM[addr - 0x08000000];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				return mapper_pntr->Peek_Memory(addr - 0x0E000000);
			}

			return (uint8_t)(cpu_Last_Bus_Value & 0xFF); // open bus
		}
		else if (addr >= 0x05000000)
		{
			if (addr >= 0x07000000)
			{
				return OAM[addr & 0x3FF];
			}
			else if (addr >= 0x06000000)
			{
				if ((addr & 0x00010000) == 0x00010000)
				{
					return VRAM[addr & 0x17FFF];
				}
				else
				{
					return VRAM[addr & 0xFFFF];
				}
			}
			else
			{
				return PALRAM[addr & 0x3FF];
			}
		}
		else if (addr >= 0x04000000)
		{
			if (addr < 0x04000800)
			{
				return Peek_Registers_8(addr - 0x04000000);
			}
			else if ((addr & 0x0400FFFC) == 0x04000800)
			{
				switch (addr & 3)
				{
					case 0: return (uint8_t)(Memory_CTRL & 0xFF);
					case 1: return (uint8_t)((Memory_CTRL >> 8) & 0xFF);
					case 2: return (uint8_t)((Memory_CTRL >> 16) & 0xFF);
					default: return (uint8_t)((Memory_CTRL >> 24) & 0xFF);
				}
			}

			return (uint8_t)(cpu_Last_Bus_Value & 0xFF); // open bus
		}
		else if (addr >= 0x03000000)
		{
			return IWRAM[addr & 0x7FFF];
		}
		else if (addr >= 0x02000000)
		{
			return WRAM[addr & 0x3FFFF];
		}

		if (addr < 0x4000)
		{
			return BIOS[addr];
		}

		return (uint8_t)(cpu_Last_Bus_Value & 0xFF); // open bus
	}

	#pragma endregion

	#pragma region DMA Memory Map
	void GBA_System::Read_Memory_16_DMA(uint32_t addr, uint32_t chan)
	{
		uint16_t ret = 0;

		// DMA always force aligned
		addr &= 0xFFFFFFFE;

		Update_Bus = true;

		if (addr >= 0x08000000)
		{
			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;

			if (addr < 0x0D000000)
			{
				ret = ROM_16[(addr - 0x08000000) >> 1];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					ret = ROM_16[(addr - 0x08000000) >> 1];
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
							ret = ROM_16[(addr - 0x08000000) >> 1];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				ret = mapper_pntr->Read_Memory_16(addr - 0x0E000000);
			}
			else
			{
				ret = (uint16_t)((dma_Last_Bus_Value[chan] >> ((addr & 2) * 8)) & 0xFFFF);

				Update_Bus = false;
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
					ret = (uint16_t)((Memory_CTRL >> ((addr & 2) * 8)) & 0xFFFF);
				}
				else
				{
					ret = (uint16_t)((cpu_Last_Bus_Value >> ((addr & 2) * 8)) & 0xFFFF);
				}

				// all of these cases update the DMA bus, this overrides the setting from Read_Registers
				Update_Bus = true;
			}
		}
		else
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr >= 0x03000000)
			{
				ret = IWRAM_16[(addr & 0x7FFE) >> 1];
				Update_IWRAM_Bus = true;
			}
			else if (addr >= 0x02000000)
			{
				if (WRAM_Enable)
				{
					ret = WRAM_16[(addr & 0x3FFFE) >> 1];
				}
				else
				{
					ret = IWRAM_16[(addr & 0x7FFE) >> 1];
					Update_IWRAM_Bus = true;
				}
			}
			else
			{
				// DMA cannot access BIOS, or other invalid addresses in this range, so nothing here	
				ret = (uint16_t)((dma_Last_Bus_Value[chan] >> ((addr & 2) * 8)) & 0xFFFF);

				Update_Bus = false;
			}
		}

		Update_Bus_Read_16_DMA(addr, ret, chan);
	}

	void GBA_System::Read_Memory_32_DMA(uint32_t addr, uint32_t chan)
	{
		uint32_t ret = 0;
		
		// DMA always force aligned
		addr &= 0xFFFFFFFC;

		Update_Bus = true;

		if (addr >= 0x08000000)
		{
			// ROM access complete, re-enable prefetcher
			pre_Inactive = false;

			if (addr < 0x0D000000)
			{
				ret = ROM_32[(addr - 0x08000000) >> 2];
			}
			else if (addr < 0x0E000000)
			{
				if (!Is_EEPROM)
				{
					ret = ROM_32[(addr - 0x08000000) >> 2];
				}
				else
				{
					if (EEPROM_Wiring)
					{
						ret = (uint32_t)((dma_Last_Bus_Value[chan] & 0xFFFFFFFE) | mapper_pntr->Mapper_EEPROM_Read());
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							ret = (uint32_t)((dma_Last_Bus_Value[chan] & 0xFFFFFFFE) | mapper_pntr->Mapper_EEPROM_Read());
						}
						else
						{
							ret = ROM_32[(addr - 0x08000000) >> 2];
						}
					}
				}
			}
			else if (addr < 0x10000000)
			{
				ret = mapper_pntr->Read_Memory_32(addr - 0x0E000000);
			}
			else
			{
				ret = dma_Last_Bus_Value[chan];

				Update_Bus = false;
			}
		}
		else if (addr >= 0x04000000)
		{
			if (addr >= 0x07000000)
			{
				ret = OAM_32[(addr & 0x3FC) >> 2];
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
							ret = VRAM_32[(addr & 0x17FFC) >> 2];
						}
						else
						{
							ret = 0;
						}
					}
					else
					{
						ret = VRAM_32[(addr & 0x17FFC) >> 2];
					}
				}
				else
				{
					ret = VRAM_32[(addr & 0xFFFC) >> 2];
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr >= 0x05000000)
			{
				ret = PALRAM_32[(addr & 0x3FC) >> 2];

				ppu_PALRAM_In_Use = false;
			}
			else
			{
				if (addr < 0x04000800)
				{
					ret = Read_Registers_32(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFC) == 0x04000800)
				{
					ret = Memory_CTRL;
				}
				else
				{
					ret = cpu_Last_Bus_Value;
				}

				Update_Bus = true;
			}
		}
		else
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr >= 0x03000000)
			{
				ret = IWRAM_32[(addr & 0x7FFC) >> 2];
				Update_IWRAM_Bus = true;
			}
			else if (addr >= 0x02000000)
			{
				if (WRAM_Enable)
				{
					ret = WRAM_32[(addr & 0x3FFFC) >> 2];
				}
				else
				{
					ret = IWRAM_32[(addr & 0x7FFC) >> 2];
					Update_IWRAM_Bus = true;
				}
			}
			else
			{
				// DMA cannot access the BIOS, so nothing here	
				ret = dma_Last_Bus_Value[chan];

				Update_Bus = false;
			}
		}

		Update_Bus_Read_32_DMA(addr, ret, chan);
	}

	void GBA_System::Write_Memory_16_DMA(uint32_t addr, uint16_t value, uint32_t chan)
	{
		// DMA always force aligned
		addr &= 0xFFFFFFFE;

		if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					if (WRAM_Enable)
					{
						WRAM_16[(addr & 0x3FFFE) >> 1] = value;
					}
					else
					{
						IWRAM_16[(addr & 0x7FFE) >> 1] = value;
						Update_IWRAM_Bus = true;
					}
				}
			}
			else if (addr < 0x04000000)
			{
				IWRAM_16[(addr & 0x7FFE) >> 1] = value;
				Update_IWRAM_Bus = true;
			}
		}
		else if (addr < 0x05000000)
		{
			if (addr < 0x04000800)
			{
				Write_Registers_16(addr - 0x04000000, value);
			}
			else if ((addr & 0x0400FFFC) == 0x04000800)
			{
				switch (addr & 2)
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
			else
			{
				mapper_pntr->Write_ROM_16(addr, value);
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

		Update_Bus_Write_16_DMA(addr, value, chan);
	}

	void GBA_System::Write_Memory_32_DMA(uint32_t addr, uint32_t value, uint32_t chan)
	{
		// DMA always force aligned
		addr &= 0xFFFFFFFC;

		if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					if (WRAM_Enable)
					{
						WRAM_32[(addr & 0x3FFFC) >> 2] = value;
					}
					else
					{
						IWRAM_32[(addr & 0x7FFC) >> 2] = value;
						Update_IWRAM_Bus = true;
					}
				}
			}
			else if (addr < 0x04000000)
			{
				IWRAM_32[(addr & 0x7FFC) >> 2] = value;
				Update_IWRAM_Bus = true;
			}
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
			else
			{
				mapper_pntr->Write_ROM_32(addr, value);
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

		Update_Bus_Write_32_DMA(addr, value, chan);
	}
	#pragma endregion

	#pragma region Bus Updates
	void GBA_System::Update_Bus_Read_8(uint32_t addr, uint8_t value)
	{
		if (Update_Bus)
		{
			if (Update_IWRAM_Bus)
			{
				// Update IWRAM bus
				cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFF << (((int)addr & 3) * 8));
				cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 3) * 8);

				cpu_Last_Bus_Value = cpu_IWRAM_Last_Bus_Value;
				Update_IWRAM_Bus = false;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];

				cpu_Last_Bus_Value = cpu_OAM_Last_Bus_Value;
			}
			else if (addr < 0x4000)
			{
				if (!In_BIOS_Region)
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
			if (Update_IWRAM_Bus)
			{
				cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
				cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);

				cpu_Last_Bus_Value = cpu_IWRAM_Last_Bus_Value;
				Update_IWRAM_Bus = false;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];

				cpu_Last_Bus_Value = cpu_OAM_Last_Bus_Value;
			}
			else if (addr < 0x4000)
			{
				if (!In_BIOS_Region)
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
			if (Update_IWRAM_Bus)
			{
				cpu_IWRAM_Last_Bus_Value = value;
				cpu_Last_Bus_Value = cpu_IWRAM_Last_Bus_Value;
				Update_IWRAM_Bus = false;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];
				cpu_Last_Bus_Value = cpu_OAM_Last_Bus_Value;
			}
			else if (addr < 0x4000)
			{
				if (!In_BIOS_Region)
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
		if (Update_IWRAM_Bus)
		{
			cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFF << (((int)addr & 3) * 8));
			cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 3) * 8);
			Update_IWRAM_Bus = false;
		}

		// cannot write to BIOS or 8 bits to OAM
	}

	void GBA_System::Update_Bus_Write_16(uint32_t addr, uint16_t value)
	{
		cpu_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
		cpu_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);

		if (Update_IWRAM_Bus)
		{
			// Update IWRAM bus
			cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
			cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
			Update_IWRAM_Bus = false;
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
		if (Update_IWRAM_Bus)
		{
			cpu_IWRAM_Last_Bus_Value = value;
			Update_IWRAM_Bus = false;
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
			if (Update_IWRAM_Bus)
			{
				cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
				cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
				Update_IWRAM_Bus = false;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = OAM_32[(addr & 0x3FC) >> 2];
			}

			// DMA bus always gets updated in this way
			dma_Last_Bus_Value[chan] = (uint32_t)value | (value << 16);

			// also update cpu bus
			cpu_Last_Bus_Value = dma_Last_Bus_Value[chan];
		}
		else
		{
			// always update cpu bus
			cpu_Last_Bus_Value = (uint32_t)value | (value << 16);
		}
	}

	void GBA_System::Update_Bus_Read_32_DMA(uint32_t addr, uint32_t value, uint32_t chan)
	{
		if (Update_Bus)
		{
			// Update IWRAM bus
			if (Update_IWRAM_Bus)
			{
				cpu_IWRAM_Last_Bus_Value = value;
				Update_IWRAM_Bus = false;
			}
			else if ((addr & 0xFF000000) == 0x07000000)
			{
				// Update OAM bus (entire word goes on the bus)
				cpu_OAM_Last_Bus_Value = value;
			}

			dma_Last_Bus_Value[chan] = value;

			// also update cpu bus
			cpu_Last_Bus_Value = dma_Last_Bus_Value[chan];
		}
		else
		{
			// always update cpu bus
			cpu_Last_Bus_Value = value;
		}
	}

	void GBA_System::Update_Bus_Write_16_DMA(uint32_t addr, uint16_t value, uint32_t chan)
	{
		cpu_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
		cpu_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);

		dma_Last_Bus_Value[chan] &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
		dma_Last_Bus_Value[chan] |= (uint32_t)value << (((int)addr & 2) * 8);

		if (Update_IWRAM_Bus)
		{
			// Update IWRAM bus
			cpu_IWRAM_Last_Bus_Value &= (uint32_t)~(0xFFFF << (((int)addr & 2) * 8));
			cpu_IWRAM_Last_Bus_Value |= (uint32_t)value << (((int)addr & 2) * 8);
			Update_IWRAM_Bus = false;
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
		if (Update_IWRAM_Bus)
		{
			cpu_IWRAM_Last_Bus_Value = value;
			Update_IWRAM_Bus = false;
		}
		else if ((addr & 0xFF000000) == 0x07000000)
		{
			// Update OAM bus (needs testing)
			cpu_OAM_Last_Bus_Value = value;
		}
	}
	#pragma endregion

	#pragma region Wait States
	// NOTE: For 32 bit accesses, PALRAM and VRAM take 2 accesses, but could be interrupted by rendering. So the 32 bit wait state processors need to know about
	// the destination and value in this case.
	// For the CPU, the value will be in cpu_Regs[cpu_Temp_Reg_Ptr], for DMA it is in dma_TFR_Word. 
	// For the CPU, whether it is a write or not is in cpu_LS_Is_Load, for DMA it is in dma_Read_Cycle	

	uint32_t GBA_System::Wait_State_Access_8(uint32_t addr, bool Seq_Access)
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
		else if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }
			
			if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				if (WRAM_Enable)
				{
					wait_ret += WRAM_Waits; //WRAM
				}
			}
		}

		return wait_ret;
	}

	uint32_t GBA_System::Wait_State_Access_16(uint32_t addr, bool Seq_Access)
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
		else if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }

			if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				if (WRAM_Enable)
				{
					wait_ret += WRAM_Waits; //WRAM
				}
			}
		}

		return wait_ret;
	}

	uint32_t GBA_System::Wait_State_Access_32(uint32_t addr, bool Seq_Access)
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
		else if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }

			if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				if (WRAM_Enable)
				{
					wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
				}
			}
		}

		return wait_ret;
	}

	uint32_t GBA_System::Wait_State_Access_32_DMA(uint32_t addr, bool Seq_Access)
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
		else if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }

			if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				if (WRAM_Enable)
				{
					wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
				}
			}
		}

		return wait_ret;
	}

	uint32_t GBA_System::Wait_State_Access_16_Instr(uint32_t addr, bool Seq_Access)
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
		else if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }

			if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				if (WRAM_Enable)
				{
					wait_ret += WRAM_Waits; //WRAM
				}
			}
		}

		return wait_ret;
	}

	uint32_t GBA_System::Wait_State_Access_32_Instr(uint32_t addr, bool Seq_Access)
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
		else if (addr < 0x04000000)
		{
			if (RAM_BIOS_Swap) { addr ^= 0x02000000; }

			if ((addr < 0x03000000) && (addr >= 0x02000000))
			{
				if (WRAM_Enable)
				{
					wait_ret += (WRAM_Waits * 2 + 1); // WRAM (2 accesses)
				}
			}
		}

		return wait_ret;
	}

	#pragma endregion
}