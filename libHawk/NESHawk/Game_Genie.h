#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	// TODO: MMC6 WRAM enable needs to account for second enable bit
	// TODO: Do bus conflicts for EXP area ($4020-$5FFF)
	// TODO: MMC5 bus conflicts
	
	class Mapper_Game_Genie : public Mappers
	{
	public:

		bool Game_Genie_Mode;

		bool Cheat_1_enable, Cheat_2_enable, Cheat_3_enable;
		bool Cheat_1_compare, Cheat_2_compare, Cheat_3_compare;

		uint8_t Cheat_1_value, Cheat_2_value, Cheat_3_value;
		uint8_t Cheat_1_compare_value, Cheat_2_compare_value, Cheat_3_compare_value;

		uint32_t Cheat_1_addr, Cheat_2_addr, Cheat_3_addr;


		void Reset()
		{
			Game_Genie_Mode = true;

			Cheat_1_enable = Cheat_2_enable = Cheat_3_enable = false;
			Cheat_1_compare = Cheat_2_compare = Cheat_3_compare = false;

			Cheat_1_value = Cheat_2_value = Cheat_3_value = 0;
			Cheat_1_compare_value = Cheat_2_compare_value = Cheat_3_compare_value = 0;

			Cheat_1_addr = Cheat_2_addr = Cheat_3_addr = 0x8000;

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
				uint8_t ret = Mapper->ReadPRG(addr);

				if (Cheat_1_enable && (addr == (Cheat_1_addr - 0x8000)))
				{
					if (Cheat_1_compare)
					{
						if (ret == Cheat_1_compare_value)
						{
							ret = Cheat_1_value;
						}
					}
					else
					{
						ret = Cheat_1_value;
					}

					// bus conflict with underlying WRAM or exp
					if ((addr & 0x6000) == 0x6000)
					{
						if ((Mapper->Core_Cart_RAM_Length[0] > 0) && Mapper->WRAM_Enable)
						{
							ret = ReadWRAM(addr - 0x6000);
						}
					}
				}
				else if (Cheat_2_enable && (addr == (Cheat_2_addr - 0x8000)))
				{
					if (Cheat_2_compare)
					{
						if (ret == Cheat_2_compare_value)
						{
							ret = Cheat_2_value;
						}
					}
					else
					{
						ret = Cheat_2_value;
					}

					// bus conflict with underlying WRAM or exp
					if ((addr & 0x6000) == 0x6000)
					{
						if ((Mapper->Core_Cart_RAM_Length[0] > 0) && Mapper->WRAM_Enable)
						{
							ret = ReadWRAM(addr - 0x6000);
						}
					}
				}
				else if (Cheat_3_enable && (addr == (Cheat_3_addr - 0x8000)))
				{
					if (Cheat_3_compare)
					{
						if (ret == Cheat_3_compare_value)
						{
							ret = Cheat_3_value;
						}
					}
					else
					{
						ret = Cheat_3_value;
					}

					// bus conflict with underlying WRAM or exp
					if ((addr & 0x6000) == 0x6000)
					{
						if ((Mapper->Core_Cart_RAM_Length[0] > 0) && Mapper->WRAM_Enable)
						{
							ret = ReadWRAM(addr - 0x6000);
						}
					}
				}
				
				return ret;
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
				if (addr == 0x0)
				{
					if ((value & 1) == 1)
					{
						Game_Genie_Mode = false;
					}

					Cheat_1_compare = ((value >> 1) & 1) == 1;
					Cheat_2_compare = ((value >> 2) & 1) == 1; 
					Cheat_3_compare = ((value >> 3) & 1) == 1;

					Cheat_1_enable = ((value >> 4) & 1) == 0;
					Cheat_2_enable = ((value >> 5) & 1) == 0;
					Cheat_3_enable = ((value >> 6) & 1) == 0;			
				}
				else if (addr == 0x1) { Cheat_1_addr |= ((value & 0x7F) << 8); }
				else if (addr == 0x2) { Cheat_1_addr |= value; }
				else if (addr == 0x3) { Cheat_1_compare_value = value; }
				else if (addr == 0x4) { Cheat_1_value = value; }
				else if (addr == 0x5) { Cheat_2_addr |= ((value & 0x7F) << 8); }
				else if (addr == 0x6) { Cheat_2_addr |= value; }
				else if (addr == 0x7) { Cheat_2_compare_value = value; }
				else if (addr == 0x8) { Cheat_2_value = value; }
				else if (addr == 0x9) { Cheat_3_addr |= ((value & 0x7F) << 8); }
				else if (addr == 0xA) { Cheat_3_addr |= value; }
				else if (addr == 0xB) { Cheat_3_compare_value = value; }
				else if (addr == 0xC) { Cheat_3_value = value; }
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

			saver = bool_saver(Cheat_1_enable, saver);
			saver = bool_saver(Cheat_2_enable, saver);
			saver = bool_saver(Cheat_3_enable, saver);

			saver = bool_saver(Cheat_1_compare, saver);
			saver = bool_saver(Cheat_2_compare, saver);
			saver = bool_saver(Cheat_3_compare, saver);

			saver = byte_saver(Cheat_1_value, saver);
			saver = byte_saver(Cheat_2_value, saver);
			saver = byte_saver(Cheat_3_value, saver);

			saver = byte_saver(Cheat_1_compare_value, saver);
			saver = byte_saver(Cheat_2_compare_value, saver);
			saver = byte_saver(Cheat_3_compare_value, saver);

			saver = int_saver(Cheat_1_addr, saver);
			saver = int_saver(Cheat_2_addr, saver);
			saver = int_saver(Cheat_3_addr, saver);

			Mapper->SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Game_Genie_Mode, loader);

			loader = bool_loader(&Cheat_1_enable, loader);
			loader = bool_loader(&Cheat_2_enable, loader);
			loader = bool_loader(&Cheat_3_enable, loader);

			loader = bool_loader(&Cheat_1_compare, loader);
			loader = bool_loader(&Cheat_2_compare, loader);
			loader = bool_loader(&Cheat_3_compare, loader);

			loader = byte_loader(&Cheat_1_value, loader);
			loader = byte_loader(&Cheat_2_value, loader);
			loader = byte_loader(&Cheat_3_value, loader);

			loader = byte_loader(&Cheat_1_compare_value, loader);
			loader = byte_loader(&Cheat_2_compare_value, loader);
			loader = byte_loader(&Cheat_3_compare_value, loader);

			loader = int_loader(&Cheat_1_addr, loader);
			loader = int_loader(&Cheat_2_addr, loader);
			loader = int_loader(&Cheat_3_addr, loader);

			Mapper->LoadState(loader);

			return loader;
		}

		void Dispose()
		{
			Mapper->Dispose();
		}
	};
}
