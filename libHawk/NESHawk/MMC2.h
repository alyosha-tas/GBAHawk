#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_MMC2 : public Mappers
	{
	public:

		uint8_t MMC2_Latch_0;
		uint8_t MMC2_Latch_1;

		uint32_t CHR_Bank;
		uint32_t CHR_Bank_2;
		uint32_t CHR_Bank_3;
		uint32_t CHR_Bank_4;
		uint32_t CHR_Mask;
		uint32_t PRG_Bank;
		uint32_t PRG_Mask;
		uint32_t Mirror_Mode;
		
		void Reset()
		{
			CHR_Mask = (*Core_CHR_ROM_Length >> 12) - 1;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			PRG_Bank = 0;

			CHR_Bank = 0;
			CHR_Bank_2 = 0;
			CHR_Bank_3 = 0;
			CHR_Bank_4 = 0;

			MMC2_Latch_0 = 0xFD;
			MMC2_Latch_1 = 0xFD;

			Mirror_Mode = 0;

			Remap_ROM();
		}

		void Remap_ROM()
		{
			if (Mirror_Mode == 0)
			{
				Core_CIRAM[0] = Core_CIRAM_Base;
				Core_CIRAM[1] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[2] = Core_CIRAM_Base;
				Core_CIRAM[3] = Core_CIRAM_Base + 0x400;
			}
			else
			{
				Core_CIRAM[0] = Core_CIRAM_Base;
				Core_CIRAM[1] = Core_CIRAM_Base;
				Core_CIRAM[2] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[3] = Core_CIRAM_Base + 0x400;
			}

			uint32_t temp_bank = 0;

			// last 3 banks are fixed
			Core_ROM[0] = Core_ROM_Base + PRG_Bank * 0x2000;
			Core_ROM[1] = Core_ROM_Base + *Core_CHR_ROM_Length - 0x6000;
			Core_ROM[2] = Core_ROM_Base + *Core_CHR_ROM_Length - 0x4000;
			Core_ROM[3] = Core_ROM_Base + *Core_CHR_ROM_Length - 0x2000;
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if (addr < 0x2000)
			{

			}
			else if (addr < 0x3000)
			{
				PRG_Bank = value & PRG_Mask & 0xF;
			}
			else if (addr < 0x4000)
			{
				CHR_Bank = value & CHR_Mask & 0x1F;
			}
			else if (addr < 0x5000)
			{
				CHR_Bank_2 = value & CHR_Mask & 0x1F;
			}
			else if (addr < 0x6000)
			{
				CHR_Bank_3 = value & CHR_Mask & 0x1F;
			}
			else if (addr < 0x7000)
			{
				CHR_Bank_4 = value & CHR_Mask & 0x1F;
			}
			else
			{
				Mirror_Mode = value & 1;
			}

			Remap_ROM();
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x1000)
			{
				if (MMC2_Latch_0 == 0xFD)
				{
					addr += CHR_Bank * 0x1000;
				}
				else
				{
					addr += CHR_Bank_2 * 0x1000;
				}

				if ((addr & 0xFFF) == 0xFD8)
				{
					MMC2_Latch_0 = 0xFD;
				}

				if ((addr & 0xFFF) == 0xFE8)
				{
					MMC2_Latch_0 = 0xFE;
				}

				return Core_CHR_ROM[addr];
			}
			else if (addr < 0x2000)
			{
				addr &= 0xFFF;
				
				if (MMC2_Latch_1 == 0xFD)
				{
					addr += CHR_Bank_3 * 0x1000;
				}
				else
				{
					addr += CHR_Bank_4 * 0x1000;
				}

				if ((addr & 0xFF8) == 0xFD8)
				{
					MMC2_Latch_1 = 0xFD;
				}

				if ((addr & 0xFF8) == 0xFE8)
				{
					MMC2_Latch_1 = 0xFE;
				}

				return Core_CHR_ROM[addr];
			}
			else
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				return Core_CIRAM[block][ofs];
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = byte_saver(MMC2_Latch_0, saver);
			saver = byte_saver(MMC2_Latch_1, saver);
			
			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Bank_2, saver);
			saver = int_saver(CHR_Bank_3, saver);
			saver = int_saver(CHR_Bank_4, saver);
			saver = int_saver(CHR_Mask, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Mask, saver);
			saver = int_saver(Mirror_Mode, saver);

			// common to all mappers
			saver = bool_saver(Mirroring, saver);
			saver = bool_saver(Bus_Conflicts, saver);
			saver = bool_saver(Old_IRQ_Type, saver);
			saver = bool_saver(Alt_Mirroring, saver);

			saver = int_saver(Size_Mask, saver);

			saver = byte_array_saver(Ex_RAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x8000);
			saver = byte_array_saver(EXT_CIRAM, saver, 0x2000);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = byte_loader(&MMC2_Latch_0, loader);
			loader = byte_loader(&MMC2_Latch_1, loader);
			
			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Bank_2, loader);
			loader = int_loader(&CHR_Bank_3, loader);
			loader = int_loader(&CHR_Bank_4, loader);
			loader = int_loader(&CHR_Mask, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);
			loader = int_loader(&Mirror_Mode, loader);

			// common to all mappers
			loader = bool_loader(&Mirroring, loader);
			loader = bool_loader(&Bus_Conflicts, loader);
			loader = bool_loader(&Old_IRQ_Type, loader);
			loader = bool_loader(&Alt_Mirroring, loader);

			loader = int_loader(&Size_Mask, loader);

			loader = byte_array_loader(Ex_RAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x8000);
			loader = byte_array_loader(EXT_CIRAM, loader, 0x2000);

			Remap_ROM();

			return loader;
		}
	};
}