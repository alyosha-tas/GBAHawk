#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"


using namespace std;

namespace NESHawk
{
	class Mapper_Game_Genie : public Mappers
	{
	public:

		bool Game_Genie_Mode;

		

		void Reset()
		{
			Game_Genie_Mode = true;

			Mapper->Reset();
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			if (Game_Genie_Mode)
			{
				if (addr < 0x4000)
				{
					return *Core_DB;
				}
				else
				{
					return Core_Game_Genie_ROM_Base[addr & 0xFFF];
				}
			}
			else

			{
				return Mapper->ReadPRG(addr);
			}		
		}

		uint8_t ReadExp(uint32_t addr)
		{
			if (Game_Genie_Mode)
			{
				return *Core_DB;
			}
			else

			{
				return Mapper->ReadExp(addr);
			}
		}

		uint8_t ReadWRAM(uint32_t addr)
		{
			if (Game_Genie_Mode)
			{
				return *Core_DB;
			}
			else

			{
				return Mapper->ReadWRAM(addr);
			}
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if (Game_Genie_Mode)
			{
				if (addr == 0)
				{
					if ((value & 1) == 1)
					{
						Game_Genie_Mode = false;
					}
				}
			}
			else

			{
				Mapper->WritePRG(addr, value);
			}
		}

		void WriteExp(uint32_t addr, uint8_t value)
		{
			if (Game_Genie_Mode)
			{

			}
			else

			{
				Mapper->WriteExp(addr, value);
			}
		}

		void WriteWRAM(uint32_t addr, uint8_t value)
		{
			if (Game_Genie_Mode)
			{

			}
			else

			{
				Mapper->WriteWRAM(addr, value);
			}
		}

		void Poke_Memory(uint32_t addr, uint8_t value) {}

		void AddressPPU(uint32_t addr)
		{
			if (!Game_Genie_Mode)
			{
				Mapper->AddressPPU(addr);
			}
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (Game_Genie_Mode)
			{
				uint8_t ret = 0;
				uint8_t temp = 0;
				
				if (addr < 0x2000)
				{
					if ((addr & 0x4) == 4)
					{
						temp = (addr & 0x20) >> 5;

						ret |= temp;
						ret |= (temp << 1);
						ret |= (temp << 2);
						ret |= (temp << 3);
						
						temp = (addr & 0x10);

						ret |= temp;
						ret |= (temp << 1);
						ret |= (temp << 2);
						ret |= (temp << 3);

						return ret;
					}
					else
					{
						temp = (addr & 0x80) >> 7;

						ret |= temp;
						ret |= (temp << 1);
						ret |= (temp << 2);
						ret |= (temp << 3);

						temp = (addr & 0x40) >> 2;

						ret |= temp;
						ret |= (temp << 1);
						ret |= (temp << 2);
						ret |= (temp << 3);

						return ret;
					}
				}

				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				return Core_CIRAM[block][ofs];
			}
			else

			{
				return Mapper->ReadPPU(addr);
			}
		}

		uint8_t PeekPPU(uint32_t addr)
		{
			if (Game_Genie_Mode)
			{
				return ReadPPU(addr);
			}
			else

			{
				return Mapper->PeekPPU(addr);
			}
		}

		uint8_t PeekCart(uint32_t addr)
		{
			if (Game_Genie_Mode)
			{
				if (addr >= 0x8000) { return ReadPRG(addr - 0x8000); }

				return *Core_DB;
			}
			else

			{
				return Mapper->PeekCart(addr);
			}
		}

		void WritePPU(uint32_t addr, uint8_t value)
		{
			if (Game_Genie_Mode)
			{
				if (addr >= 0x2000)
				{
					uint32_t ofs = addr & 0x3FF;
					uint32_t block = (addr >> 10) & 3;

					Core_CIRAM[block][ofs] = value;
				}
			}
			else
			{
				Mapper->WritePPU(addr, value);
			}
		}

		void ClockCPU()
		{
			if (!Game_Genie_Mode)
			{
				Mapper->ClockCPU();
			}
		}

		void ClockPPU()
		{
			if (!Game_Genie_Mode)
			{
				Mapper->ClockPPU();
			}
		}

		bool IrqSignal()
		{
			if (Game_Genie_Mode)
			{
				return false;
			}
			else

			{
				return Mapper->IrqSignal();
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Game_Genie_Mode, saver);

			Mapper->SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Game_Genie_Mode, loader);

			Mapper->LoadState(loader);

			return loader;
		}

		void Dispose()
		{
			Mapper->Dispose();
		}
	};
}
