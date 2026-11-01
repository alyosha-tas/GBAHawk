#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class VRC2 : public Mappers
	{
	public:

		bool Mirror_Mode;

		uint8_t SP_Latch;
		
		uint32_t CHR_Bank;
		uint32_t CHR_Mask;
		uint32_t PRG_Bank;
		uint32_t PRG_Mask;
		uint32_t Reg_Addr;

		uint16_t VRC2_Regs[10] = { };
		uint16_t VRC2_CHR_Regs_1K[8] = { };

		void Reset()
		{
			Mirror_Mode = false;

			SP_Latch = 0;

			Reg_Addr = 0;

			VRC2_Regs[0] = 0;
			VRC2_Regs[1] = 0;
			VRC2_Regs[2] = 0;
			VRC2_Regs[3] = 0;
			VRC2_Regs[4] = 0;
			VRC2_Regs[5] = 0;
			VRC2_Regs[6] = 0;
			VRC2_Regs[7] = 0;

			CHR_Bank = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 10) - 1;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			PRG_Bank = 0;

			Remap_ROM();
		}

		void Remap_ROM()
		{
			Core_ROM[0] = Core_ROM_Base + (VRC2_Regs[8] & PRG_Mask & 0x1F) * 0x2000;
			Core_ROM[1] = Core_ROM_Base + (VRC2_Regs[9] & PRG_Mask & 0x1F) * 0x2000;
			Core_ROM[2] = Core_ROM_Base + *Core_ROM_Length - 0x4000;
			Core_ROM[3] = Core_ROM_Base + *Core_ROM_Length - 0x2000;

			VRC2_CHR_Regs_1K[0] = VRC2_Regs[0];
			VRC2_CHR_Regs_1K[1] = VRC2_Regs[1];
			VRC2_CHR_Regs_1K[2] = VRC2_Regs[2];
			VRC2_CHR_Regs_1K[3] = VRC2_Regs[3];
			VRC2_CHR_Regs_1K[4] = VRC2_Regs[4];
			VRC2_CHR_Regs_1K[5] = VRC2_Regs[5];
			VRC2_CHR_Regs_1K[6] = VRC2_Regs[6];
			VRC2_CHR_Regs_1K[7] = VRC2_Regs[7];

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
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			// 0 is normal layout, 1 is reg layout with 1 and 2 swapped
			if (VRC_Register_Layout == 1)
			{
				uint32_t swap = (addr & 2) >> 1;
				swap |= (addr & 1) << 1;

				addr &= 0xFFFFFFFC;
				addr |= swap;
			}

			switch (addr & 0x7003)
			{
				case 0x0000: //$8000
				case 0x0001: //$8001
				case 0x0002: //$8002
				case 0x0003: //$8003
					VRC2_Regs[8] = value & 0x1F;
					break;

				case 0x1000: //$9000
				case 0x1001: //$9001
				case 0x1002: //$9002
				case 0x1003: //$9003
					Mirror_Mode = ((value & 0x1) == 1);
					break;

				case 0x2000: //$A000
				case 0x2001: //$A001
				case 0x2002: //$A002
				case 0x2003: //$A003
					VRC2_Regs[9] = value & 0x1F;
					break;

				case 0x3000: //$B000
					VRC2_Regs[0] &= 0x1F0;
					VRC2_Regs[0] |= (value & 0xF);
					break;
				case 0x3001: //$B001
					VRC2_Regs[0] &= 0xF;
					VRC2_Regs[0] |= ((value & 0x1F) << 4);
					break;

				case 0x3002: //$B002
					VRC2_Regs[1] &= 0x1F0;
					VRC2_Regs[1] |= (value & 0xF);
					break;
				case 0x3003: //$B003
					VRC2_Regs[1] &= 0xF;
					VRC2_Regs[1] |= ((value & 0x1F) << 4);
					break;

				case 0x4000: //$C000
					VRC2_Regs[2] &= 0x1F0;
					VRC2_Regs[2] |= (value & 0xF);
					break;
				case 0x4001: //$C001
					VRC2_Regs[2] &= 0xF;
					VRC2_Regs[2] |= ((value & 0x1F) << 4);
					break;

				case 0x4002: //$C002
					VRC2_Regs[3] &= 0x1F0;
					VRC2_Regs[3] |= (value & 0xF);
					break;
				case 0x4003: //$C003
					VRC2_Regs[3] &= 0xF;
					VRC2_Regs[3] |= ((value & 0x1F) << 4);
					break;

				case 0x5000: //$D000
					VRC2_Regs[4] &= 0x1F0;
					VRC2_Regs[4] |= (value & 0xF);
					break;
				case 0x5001: //$D001
					VRC2_Regs[4] &= 0xF;
					VRC2_Regs[4] |= ((value & 0x1F) << 4);
					break;

				case 0x5002: //$D002
					VRC2_Regs[5] &= 0x1F0;
					VRC2_Regs[5] |= (value & 0xF);
					break;
				case 0x5003: //$D003
					VRC2_Regs[5] &= 0xF;
					VRC2_Regs[5] |= ((value & 0x1F) << 4);
					break;

				case 0x6000: //$E000
					VRC2_Regs[6] &= 0x1F0;
					VRC2_Regs[6] |= (value & 0xF);
					break;
				case 0x6001: //$E001
					VRC2_Regs[6] &= 0xF;
					VRC2_Regs[6] |= ((value & 0x1F) << 4);
					break;

				case 0x6002: //$E002
					VRC2_Regs[7] &= 0x1F0;
					VRC2_Regs[7] |= (value & 0xF);
					break;
				case 0x6003: //$E003
					VRC2_Regs[7] &= 0xF;
					VRC2_Regs[7] |= ((value & 0x1F) << 4);
					break;

			}

			Remap_ROM();
		}

		uint32_t Get_CHRBank_1K(uint32_t addr)
		{
			uint32_t bank_1k = addr >> 10;
			bank_1k = VRC2_CHR_Regs_1K[bank_1k];
			return bank_1k;
		}

		uint32_t MapCHR(uint32_t addr)
		{
			uint32_t bank_1k = Get_CHRBank_1K(addr);
			// allow NPOT chr sizes
			bank_1k %= CHR_Mask + 1;
			addr = (bank_1k << 10) | (addr & 0x3FF);
			return addr;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				if (*Core_CHR_ROM_Length > 0)
				{
					addr = MapCHR(addr);

					return Core_CHR_ROM[addr];
				}
				else
				{
					return VRAM[addr];
				}
			}

			uint32_t ofs = addr & 0x3FF;
			uint32_t block = (addr >> 10) & 3;

			return Core_CIRAM[block][ofs];
		}

		void WritePPU(uint32_t addr, uint8_t value)
		{
			if (addr >= 0x2000)
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				Core_CIRAM[block][ofs] = value;
			}
			else
			{
				if (*Core_CHR_ROM_Length == 0)
				{
					VRAM[addr] = value;
				}
			}
		}

		uint8_t ReadWRAM(uint32_t addr)
		{
			if (addr < 0x1000)
			{
				// check if games rely on other behavior of the latch bit (could be always 0)
				return (*Core_DB & 0x7E | SP_Latch);
			}
			else
			{
				return *Core_DB;
			}
		}

		void WriteWRAM(uint32_t addr, uint8_t value)
		{
			if (addr < 0x1000)
			{
				SP_Latch = value & 1;
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Mirror_Mode, saver);

			saver = byte_saver(SP_Latch, saver);
			
			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Mask, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Mask, saver);
			saver = int_saver(Reg_Addr, saver);
			saver = int_saver(VRC_Register_Layout, saver);

			saver = short_array_saver(VRC2_Regs, saver, 10);
			saver = short_array_saver(VRC2_CHR_Regs_1K, saver, 8);

			// common to all mappers
			saver = Base_Saver(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Mirror_Mode, loader);

			loader = byte_loader(&SP_Latch, loader);
			
			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Mask, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);
			loader = int_loader(&Reg_Addr, loader);
			loader = int_loader(&VRC_Register_Layout, loader);

			loader = short_array_loader(VRC2_Regs, loader, 10);
			loader = short_array_loader(VRC2_CHR_Regs_1K, loader, 8);

			// common to all mappers
			loader = Base_Loader(loader);

			Remap_ROM();

			return loader;
		}
	};
}