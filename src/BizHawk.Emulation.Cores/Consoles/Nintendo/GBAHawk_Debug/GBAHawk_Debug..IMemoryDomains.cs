using System.Collections.Generic;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		private IMemoryDomains MemoryDomains;

		public void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain>
			{
				new MemoryDomainDelegate(
					"WRAM",
					WRAM.Length,
					MemoryDomain.Endian.Little,
					addr => PeekWRAM(addr),
					(addr, value) => WRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"IWRAM",
					IWRAM.Length,
					MemoryDomain.Endian.Little,
					addr => PeekIWRAM(addr),
					(addr, value) => IWRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"ROM",
					ROM.Length,
					MemoryDomain.Endian.Little,
					addr => ROM[addr],
					(addr, value) => ROM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"VRAM",
					VRAM.Length,
					MemoryDomain.Endian.Little,
					addr => PeekVRAM(addr),
					(addr, value) => VRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"OAM",
					OAM.Length,
					MemoryDomain.Endian.Little,
					addr => PeekOAM(addr),
					(addr, value) => OAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"PALRAM",
					PALRAM.Length,
					MemoryDomain.Endian.Little,
					addr => PeekPALRAM(addr),
					(addr, value) => PALRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"Registers",
					0x800,
					MemoryDomain.Endian.Little,
					addr => PeekSystemBus(addr + 0x04000000),
					(addr, value) => PokeSystemBus(addr + 0x04000000, value),
					1),
				new MemoryDomainDelegate(
					"System Bus",
					0x10000000,
					MemoryDomain.Endian.Little,
					addr => PeekSystemBus(addr),
					(addr, value) => PokeSystemBus(addr, value),
					1),
			};

			if (cart_RAM != null)
			{
				var CartRam = new MemoryDomainDelegate(
					"CartRAM",
					cart_RAM.Length,
					MemoryDomain.Endian.Little,
					addr => PeekCART(addr),
					(addr, value) => cart_RAM[addr] = value,
					1);
				domains.Add(CartRam);
			}

			MemoryDomains = new MemoryDomainList(domains);
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}

		private byte PeekWRAM(long addr)
		{
			uint addr2 = (uint)(addr & 0x3FFFF);

			return WRAM[addr2];
		}

		private byte PeekIWRAM(long addr)
		{
			uint addr2 = (uint)(addr & 0x7FFF);

			return IWRAM[addr2];
		}

		private byte PeekVRAM(long addr)
		{
			uint addr2 = (uint)(addr & 0x1FFFF);

			if (addr2 >= 0x100000)
			{
				addr2 &= 0x17FFF;
			}

			return VRAM[addr2];
		}

		private byte PeekPALRAM(long addr)
		{
			uint addr2 = (uint)(addr & 0x3FF);

			return PALRAM[addr2];
		}

		private byte PeekOAM(long addr)
		{
			uint addr2 = (uint)(addr & 0x3FF);

			return OAM[addr2];
		}

		private byte PeekCART(long addr)
		{
			if (cart_RAM != null)
			{
				if (addr < cart_RAM.Length)
				{
					return cart_RAM[addr];
				}

				return 0xFF;
			}

			return 0xFF;
		}

		private byte PeekSystemBus(long addr)
		{
			uint addr2 = (uint)(addr & 0xFFFFFFF);
			return Peek_Memory_8(addr2);
		}

		private void PokeSystemBus(long addr, byte value)
		{
			uint addr2 = (uint)(addr & 0xFFFFFFF);
			Write_Memory_8(addr2, value);
		}
	}
}
