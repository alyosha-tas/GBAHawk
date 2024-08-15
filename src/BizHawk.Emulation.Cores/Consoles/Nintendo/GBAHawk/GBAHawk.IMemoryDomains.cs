﻿using System.Collections.Generic;
using System.Linq;

using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBAHawk
	{
		private MemoryDomainList MemoryDomains;
		private readonly Dictionary<string, MemoryDomainByteArray> _byteArrayDomains = new Dictionary<string, MemoryDomainByteArray>();
		private bool _memoryDomainsInit = false;

		private void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain>
			{
				new MemoryDomainDelegate(
					"WRAM",
					0x40000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawk.GBA_getwram(GBA_Pntr, (int)(addr & 0x3FFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"IWRAM",
					0x8000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawk.GBA_getiwram(GBA_Pntr, (int)(addr & 0x7FFF)),
					(addr, value) => { },
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
					0x18000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawk.GBA_getvram(GBA_Pntr, (int)(addr & 0x1FFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"OAM",
					0x400,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawk.GBA_getoam(GBA_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"PALRAM",
					0x400,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawk.GBA_getpalram(GBA_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"System Bus",
					0x10000000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawk.GBA_getsysbus(GBA_Pntr, (int)(addr & 0xFFFFFFF)),
					(addr, value) => { },
					1)
			};
			
			if (cart_RAM != null)
			{
				var CartRam = new MemoryDomainDelegate(
					"CartRAM",
					cart_RAM.Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawk.GBA_getsram(GBA_Pntr, (int)(addr & (cart_RAM.Length - 1))),
					(addr, value) => cart_RAM[addr] = value,
					1);
				domains.Add(CartRam);
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);

			_memoryDomainsInit = true;
		}
	}
}
