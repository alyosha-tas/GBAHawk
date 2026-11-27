using System.Collections.Generic;
using System.Linq;

using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBCHawk
{
	public partial class GBCHawk
	{
		private MemoryDomainList MemoryDomains;
		private readonly Dictionary<string, MemoryDomainByteArray> _byteArrayDomains = new Dictionary<string, MemoryDomainByteArray>();

		private void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain>
			{
				new MemoryDomainDelegate(
					"RAM",
					0x8000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBCHawk.GBC_getram(GBC_Pntr, (int)(addr & 0x7FFF), Current_sync_on_vbl),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"HRAM",
					0x80,
					MemoryDomain.Endian.Little,
					(addr) => LibGBCHawk.GBC_gethram(GBC_Pntr, (int)(addr & 0x7F), Current_sync_on_vbl),
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
					0x4000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBCHawk.GBC_getvram(GBC_Pntr, (int)(addr & 0x3FFF), Current_sync_on_vbl),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"OAM",
					0xA0,
					MemoryDomain.Endian.Little,
					addr => LibGBCHawk.GBC_getoam(GBC_Pntr, (int)(addr & 0xFF), Current_sync_on_vbl),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"Registers",
					0x400,
					MemoryDomain.Endian.Little,
					addr => LibGBCHawk.GBC_getregisters(GBC_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"System Bus",
					0x10000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBCHawk.GBC_getsysbus(GBC_Pntr, (int)(addr & 0xFFFF)),
					(addr, value) => { },
					1)
			};

			if (cart_RAM != null)
			{
				var CartRam = new MemoryDomainDelegate(
					"CartRAM",
					cart_RAM.Length,
					MemoryDomain.Endian.Little,
					addr => LibGBCHawk.GBC_getsram(GBC_Pntr, (int)(addr & (cart_RAM.Length - 1)), Current_sync_on_vbl),
					(addr, value) => cart_RAM[addr] = value,
					1);
				domains.Add(CartRam);
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}
	}
}
