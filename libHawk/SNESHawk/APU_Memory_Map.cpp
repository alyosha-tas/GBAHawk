#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "APU_System.h"

/*

*/
namespace SNESHawk
{
#pragma region APU Memory Map

	uint8_t APU::ReadMemory(uint32_t addr)
	{
		uint8_t ret;

		if (addr < 0xF0)
		{
			ret = RAM[addr];
		}
		else if (addr < 0x0800)
		{
			ret = ReadReg(addr);
		}
		else if (addr < 0xFFC0)
		{
			ret = RAM[addr];
		}
		else
		{
			if (IPL_Enable)
			{
				ret = IPL_ROM[addr - 0xFFC0];
			}
			else
			{
				ret = RAM[addr];
			}
		}

		return ret;
	}

	void APU::WriteMemory(uint32_t addr, uint8_t value)
	{
		if (addr < 0xF0)
		{
			// easy optimization, since rom reads are so common, move this up (reordering the rest of these else ifs is not easy)
			RAM[addr] = value;
		}
		else if (addr < 0x0800)
		{
			WriteReg(addr, value);
		}
		else
		{
			RAM[addr] = value;
		}
	}

	uint8_t APU::ReadReg(uint32_t addr)
	{
		switch (addr)
		{
			case 0xF0: return Undoc_Reg;
			case 0xF1: return Ctrl_Reg;
			case 0xF2: return DSP_Reg_Addr;
			case 0xF3: return DSP_Reg_Data;
			case 0xF4: return Port_0_Reg;
			case 0xF5: return Port_1_Reg;
			case 0xF6: return Port_2_Reg;
			case 0xF7: return Port_3_Reg;
			case 0xF8: return RAM_F8;
			case 0xF9: return RAM_F9;
			case 0xFA: return Timer_0_Reg;
			case 0xFB: return Timer_1_Reg;
			case 0xFC: return Timer_2_Reg;
			case 0xFD: return Counter_0_Reg;
			case 0xFE: return Counter_1_Reg;
			case 0xFF: return Counter_2_Reg;
		}
	}

	void APU::WriteReg(uint32_t addr, uint8_t value)
	{
		switch (addr)
		{
			case 0xF0:
				Undoc_Reg = value;
				break;
			case 0xF1:
				Ctrl_Reg = value;
				break;
			case 0xF2:
				DSP_Reg_Addr = value;
				break;
			case 0xF3:
				DSP_Reg_Data = value;
				break;
			case 0xF4:
				Port_0_Reg = value;
				break;
			case 0xF5:
				Port_1_Reg = value;
				break;
			case 0xF6:
				Port_2_Reg = value;
				break;
			case 0xF7:
				Port_3_Reg = value;
				break;
			case 0xF8:
				RAM_F8 = value;
				break;
			case 0xF9:
				RAM_F9 = value;
				break;
			case 0xFA:
				Timer_0_Reg = value;
				break;
			case 0xFB:
				Timer_1_Reg = value;
				break;
			case 0xFC:
				Timer_2_Reg = value;
				break;
			case 0xFD:
				Counter_0_Reg = value;
				break;
			case 0xFE:
				Counter_1_Reg = value;
				break;
			case 0xFF:
				Counter_2_Reg = value;
				break;
		}
	}

	uint8_t APU::PeekMemory(uint32_t addr)
	{
		uint8_t ret;

		if (addr < 0xF0)
		{
			ret = RAM[addr];
		}
		else if (addr < 0x0800)
		{
			ret = PeekReg(addr);
		}
		else if (addr < 0xFFC0)
		{
			ret = RAM[addr];
		}
		else
		{
			if (IPL_Enable)
			{
				ret = IPL_ROM[addr - 0xFFC0];
			}
			else
			{
				ret = RAM[addr];
			}
		}

		return ret;
	}

	uint8_t APU::PeekReg(uint32_t addr)
	{
		switch (addr)
		{
			case 0xF0: return Undoc_Reg;
			case 0xF1: return Ctrl_Reg;
			case 0xF2: return DSP_Reg_Addr;
			case 0xF3: return DSP_Reg_Data;
			case 0xF4: return Port_0_Reg;
			case 0xF5: return Port_1_Reg;
			case 0xF6: return Port_2_Reg;
			case 0xF7: return Port_3_Reg;
			case 0xF8: return RAM_F8;
			case 0xF9: return RAM_F9;
			case 0xFA: return Timer_0_Reg;
			case 0xFB: return Timer_1_Reg;
			case 0xFC: return Timer_2_Reg;
			case 0xFD: return Counter_0_Reg;
			case 0xFE: return Counter_1_Reg;
			case 0xFF: return Counter_2_Reg;
		}
	}

	uint16_t APU::Peek_Memory_16(uint32_t addr)
	{
		uint16_t ret = 0;

		ret = PeekMemory(addr);

		ret |= (uint16_t)(PeekMemory(++addr) << 8);

		return ret;
	}

	uint16_t APU::Peek_Memory_8_Branch(uint32_t addr)
	{
		int16_t ret = 0;

		ret = (int16_t)PeekMemory(addr);

		if ((ret & 0x80) == 0x80)
		{
			ret |= 0xFF00;
		}

		return (uint16_t)((int16_t)addr + ret + 1);
	}

#pragma endregion
}