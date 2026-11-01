#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_206 : public Mappers
	{
	public:

		uint32_t CHR_Bank;
		uint32_t CHR_Mask;
		uint32_t PRG_Bank;
		uint32_t PRG_Mask;
		uint32_t Reg_Addr;

		uint8_t M206_Regs[8] = { };
		uint8_t M206_CHR_Regs_1K[8] = { };

		void Reset()
		{
			Reg_Addr = 0;

			M206_Regs[0] = 0;
			M206_Regs[1] = 0;
			M206_Regs[2] = 0;
			M206_Regs[3] = 0;
			M206_Regs[4] = 0;
			M206_Regs[5] = 0;
			M206_Regs[6] = 0;
			M206_Regs[7] = 0;

			CHR_Bank = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 10) - 1;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			PRG_Bank = 0;

			Remap_ROM();
		}

		void Remap_ROM()
		{
			Core_ROM[0] = Core_ROM_Base + (M206_Regs[6] & PRG_Mask & 0x3F) * 0x2000;
			Core_ROM[1] = Core_ROM_Base + (M206_Regs[7] & PRG_Mask & 0x3F) * 0x2000;
			Core_ROM[2] = Core_ROM_Base + *Core_ROM_Length - 0x4000;
			Core_ROM[3] = Core_ROM_Base + *Core_ROM_Length - 0x2000;

			M206_CHR_Regs_1K[0] = M206_Regs[0];
			M206_CHR_Regs_1K[1] = M206_Regs[0] + 1;
			M206_CHR_Regs_1K[2] = M206_Regs[1];
			M206_CHR_Regs_1K[3] = M206_Regs[1] + 1;

			M206_CHR_Regs_1K[4] = M206_Regs[2];
			M206_CHR_Regs_1K[5] = M206_Regs[3];
			M206_CHR_Regs_1K[6] = M206_Regs[4];
			M206_CHR_Regs_1K[7] = M206_Regs[5];
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			switch (addr & 0x6001)
			{
				case 0x0000: //$8000
					Reg_Addr = (value & 7);
					break;
				case 0x0001: //$8001
					switch (Reg_Addr)
					{
						case 0: M206_Regs[Reg_Addr] = value & 0x3E; break;
						case 1: M206_Regs[Reg_Addr] = value & 0x3E; break;
						case 2: M206_Regs[Reg_Addr] = value & 0x3F; break;
						case 3: M206_Regs[Reg_Addr] = value & 0x3F; break;
						case 4: M206_Regs[Reg_Addr] = value & 0x3F; break;
						case 5: M206_Regs[Reg_Addr] = value & 0x3F; break;
						case 6: M206_Regs[Reg_Addr] = value & 0x0F; break;
						case 7: M206_Regs[Reg_Addr] = value & 0x0F; break;
					}

					break;
			}

			Remap_ROM();
		}

		uint32_t Get_CHRBank_1K(uint32_t addr)
		{
			uint32_t bank_1k = addr >> 10;
			bank_1k = M206_CHR_Regs_1K[bank_1k];
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

				// mapper conflict when writing while executing from $8000-$9FFF
				if ((*Core_PC >= 0x8000) && (*Core_PC < 0xA000))
				{
					WritePRG(addr, value);
				}
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Mask, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Mask, saver);
			saver = int_saver(Reg_Addr, saver);

			saver = byte_array_saver(M206_Regs, saver, 8);
			saver = byte_array_saver(M206_CHR_Regs_1K, saver, 8);

			// common to all mappers
			saver = Base_Saver(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Mask, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);
			loader = int_loader(&Reg_Addr, loader);

			loader = byte_array_loader(M206_Regs, loader, 8);
			loader = byte_array_loader(M206_CHR_Regs_1K, loader, 8);

			// common to all mappers
			loader = Base_Loader(loader);

			Remap_ROM();

			return loader;
		}
	};
}