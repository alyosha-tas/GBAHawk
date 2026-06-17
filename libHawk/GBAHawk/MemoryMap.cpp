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
		else if (addr >= 0x03000000)
		{
			ret = IWRAM[addr & 0x7FFF];
		}
		else if (addr >= 0x02000000)
		{
			ret = WRAM[addr & 0x3FFFF];
		}
		else if (addr < 0x4000)
		{
			// BIOS is protected against reading from memory beyond the BIOS range
			if (cpu_Regs[15] >= 0x4000)
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
		else if (addr >= 0x03000000)
		{
			// Forced Align
			ret = IWRAM_16[(addr & 0x7FFE) >> 1];
		}
		else if (addr >= 0x02000000)
		{
			// Forced Align
			ret = WRAM_16[(addr & 0x3FFFE) >> 1];
		}
		else if (addr < 0x4000)
		{
			// BIOS is protected against reading from memory beyond the BIOS range
			if (cpu_Regs[15] >= 0x4000)
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
		else if (addr >= 0x03000000)
		{
			// Forced Align
			ret = IWRAM_32[(addr & 0x7FFC) >> 2];
		}
		else if (addr >= 0x02000000)
		{
			// Forced Align
			ret = WRAM_32[(addr & 0x3FFFC) >> 2];
		}
		else if (addr < 0x4000)
		{
			// BIOS is protected against reading from memory beyond the BIOS range
			if (cpu_Regs[15] >= 0x4000)
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

		Update_Bus_Read_32(addr, ret);

		return ret;
	}

	void GBA_System::Write_Memory_8(uint32_t addr, uint8_t value)
	{
		if (addr < 0x03000000)
		{
			if (addr >= 0x02000000)
			{
				WRAM[addr & 0x3FFFF] = value;
			}
		}
		else if (addr < 0x04000000)
		{
			IWRAM[addr & 0x7FFF] = value;
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
			PALRAM[addr & 0x3FF] = value;
			PALRAM[(addr + 1) & 0x3FF] = value;

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
							// 8 bit writes stored as halfword (needs more research)
							VRAM[addr & 0x17FFF] = value;
							VRAM[(addr + 1) & 0x17FFF] = value;
						}
					}
				}
				else
				{
					if (ppu_BG_Mode >= 3)
					{
						if ((addr & 0x17FFF) < 0x14000)
						{
							// 8 bit writes stored as halfword (needs more research)
							VRAM[addr & 0x17FFF] = value;
							VRAM[(addr + 1) & 0x17FFF] = value;
						}
					}
				}
			}
			else
			{
				// 8 bit writes stored as halfword (needs more research)
				VRAM[addr & 0xFFFF] = value;
				VRAM[(addr + 1) & 0xFFFF] = value;
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
		if (addr < 0x03000000)
		{
			if (addr >= 0x02000000)
			{
				// Forced Align
				WRAM_16[(addr & 0x3FFFE) >> 1] = value;
			}
		}
		else if (addr < 0x04000000)
		{
			// Forced Align
			IWRAM_16[(addr & 0x7FFE) >> 1] = value;
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
		if (addr < 0x03000000)
		{
			if (addr >= 0x02000000)
			{
				// Forced Align
				WRAM_32[(addr & 0x3FFFC) >> 2] = value;
			}
		}
		else if (addr < 0x04000000)
		{
			// Forced Align
			IWRAM_32[(addr & 0x7FFC) >> 2] = value;
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
			}
		}
		else if (addr >= 0x03000000)
		{
			ret = IWRAM_16[(addr & 0x7FFE) >> 1];
		}
		else if (addr >= 0x02000000)
		{
			ret = WRAM_16[(addr & 0x3FFFE) >> 1];
		}
		else
		{
			// DMA cannot access BIOS, or other invalid addresses in this range, so nothing here	
			ret = (uint16_t)((dma_Last_Bus_Value[chan] >> ((addr & 2) * 8)) & 0xFFFF);
		}

		// DMA always updates the bus
		Update_Bus = true;

		Update_Bus_Read_16_DMA(addr, ret, chan);
	}

	void GBA_System::Read_Memory_32_DMA(uint32_t addr, uint32_t chan)
	{
		uint32_t ret = 0;
		
		// DMA always force aligned
		addr &= 0xFFFFFFFC;

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
			}
		}
		else if (addr >= 0x03000000)
		{
			ret = IWRAM_32[(addr & 0x7FFC) >> 2];
		}
		else if (addr >= 0x02000000)
		{
			ret = WRAM_32[(addr & 0x3FFFC) >> 2];
		}
		else
		{
			// DMA cannot access the BIOS, so nothing here	
			ret = dma_Last_Bus_Value[chan];
		}

		// DMA always updates the bus
		Update_Bus = true;

		Update_Bus_Read_32_DMA(addr, ret, chan);
	}

	void GBA_System::Write_Memory_16_DMA(uint32_t addr, uint16_t value, uint32_t chan)
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

		Update_Bus_Write_32_DMA(addr, value, chan);
	}
	#pragma endregion
}