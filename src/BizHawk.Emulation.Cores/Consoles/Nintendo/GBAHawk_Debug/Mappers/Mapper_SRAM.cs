using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	// Cart with ordinary SRAM
	public class MapperSRAM : MapperBase
	{
		public override void Reset()
		{
			// nothing to initialize
		}

		public override byte ReadMemory8(uint addr)
		{
			return Core.cart_RAM[addr & 0x7FFF];
		}

		public override ushort ReadMemory16(uint addr)
		{
			// 8 bit bus only
			ushort ret = Core.cart_RAM[addr & 0x7FFF];
			ret = (ushort)(ret | (ret << 8));
			return ret;
		}

		public override uint ReadMemory32(uint addr)
		{
			// 8 bit bus only
			uint ret = Core.cart_RAM[addr & 0x7FFF];
			ret = (uint)(ret | (ret << 8) | (ret << 16) | (ret << 24));
			return ret;
		}

		public override byte PeekMemory(uint addr)
		{
			return ReadMemory8(addr);
		}

		public override void WriteMemory8(uint addr, byte value)
		{
			Core.cart_RAM[addr & 0x7FFF] = value;
		}

		public override void WriteMemory16(uint addr, ushort value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 1) == 0)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)(value & 0xFF);
			}
			else
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 8) & 0xFF);
			}			
		}

		public override void WriteMemory32(uint addr, uint value)
		{
			// stores the correct byte in the correct position, but only 1
			if ((addr & 3) == 0)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)(value & 0xFF);
			}
			else if ((addr & 3) == 1)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 8) & 0xFF);
			}
			else if ((addr & 3) == 2)
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 16) & 0xFF);
			}
			else
			{
				Core.cart_RAM[addr & 0x7FFF] = (byte)((value >> 24) & 0xFF);
			}
		}

		public override void PokeMemory(uint addr, byte value)
		{
			WriteMemory8(addr, value);
		}
	}
}
