using System.Collections.Generic;

using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	public partial class GBAHawkLink_Debug
	{
		private IMemoryDomains MemoryDomains;

		public void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain>
			{
				new MemoryDomainDelegate(
					"Main RAM L",
					L.WRAM.Length,
					MemoryDomain.Endian.Little,
					addr => L.WRAM[addr],
					(addr, value) => L.WRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"Main RAM R",
					R.WRAM.Length,
					MemoryDomain.Endian.Little,
					addr => R.WRAM[addr],
					(addr, value) => R.WRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"Zero Page RAM L",
					L.IWRAM.Length,
					MemoryDomain.Endian.Little,
					addr => L.IWRAM[addr],
					(addr, value) => L.IWRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"Zero Page RAM R",
					R.IWRAM.Length,
					MemoryDomain.Endian.Little,
					addr => R.IWRAM[addr],
					(addr, value) => R.IWRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"System Bus L",
					0X10000,
					MemoryDomain.Endian.Little,
					PeekSystemBusL,
					PokeSystemBusL,
					1),
				new MemoryDomainDelegate(
					"System Bus R",
					0X10000,
					MemoryDomain.Endian.Little,
					PeekSystemBusR,
					PokeSystemBusR,
					1),
				new MemoryDomainDelegate(
					"ROM L",
					L.ROM.Length,
					MemoryDomain.Endian.Little,
					addr => L.ROM[addr],
					(addr, value) => L.ROM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"ROM R",
					R.ROM.Length,
					MemoryDomain.Endian.Little,
					addr => R.ROM[addr],
					(addr, value) => R.ROM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"VRAM L",
					L.VRAM.Length,
					MemoryDomain.Endian.Little,
					addr => L.VRAM[addr],
					(addr, value) => L.VRAM[addr] = value,
					1),
				new MemoryDomainDelegate(
					"VRAM R",
					R.VRAM.Length,
					MemoryDomain.Endian.Little,
					addr => R.VRAM[addr],
					(addr, value) => R.VRAM[addr] = value,
					1)
			};

			if (L.cart_RAM != null)
			{
				var cartRamL = new MemoryDomainByteArray("Cart RAM L", MemoryDomain.Endian.Little, L.cart_RAM, true, 1);
				domains.Add(cartRamL);
			}

			if (R.cart_RAM != null)
			{
				var cartRamR = new MemoryDomainByteArray("Cart RAM R", MemoryDomain.Endian.Little, R.cart_RAM, true, 1);
				domains.Add(cartRamR);
			}

			MemoryDomains = new MemoryDomainList(domains);
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}

		private byte PeekSystemBusL(long addr)
		{
			uint addr2 = (uint)(addr & 0xFFFFFFFF);
			return L.Peek_Memory_8(addr2);
		}

		private byte PeekSystemBusR(long addr)
		{
			uint addr2 = (uint)(addr & 0xFFFFFFFF);
			return R.Peek_Memory_8(addr2);
		}

		private void PokeSystemBusL(long addr, byte value)
		{
			uint addr2 = (uint)(addr & 0xFFFFFFFF);
			L.Write_Memory_8(addr2, value);
		}

		private void PokeSystemBusR(long addr, byte value)
		{
			uint addr2 = (uint)(addr & 0xFFFFFFFF);
			R.Write_Memory_8(addr2, value);
		}
	}
}
