using System.Collections.Generic;
using System.Linq;

using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.SNES.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SNESHawk
{
	public partial class SNESHawk
	{
		private MemoryDomainList MemoryDomains;
		private readonly Dictionary<string, MemoryDomainByteArray> _byteArrayDomains = new Dictionary<string, MemoryDomainByteArray>();

		private void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain>
			{
				new MemoryDomainDelegate(
					"RAM",
					0x20000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getram(SNES_Pntr, (int)(addr & 0x1FFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"APU RAM",
					0x10000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_get_apu_ram(SNES_Pntr, (int)(addr & 0xFFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"VRAM",
					0x10000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getvram(SNES_Pntr, (int)(addr & 0xFFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"ROM",
					ROM_Length,
					MemoryDomain.Endian.Little,
					addr => GamePack[addr & (ROM_Length - 1)],
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"OAM",
					544,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getoam(SNES_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"PALRAM",
					0x200,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getpalram(SNES_Pntr, (int)(addr & 0x1FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"PPU Bus",
					0x4000,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_get_ppu_bus(SNES_Pntr, (uint)(addr & 0x3FFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"System Bus",
					0x1000000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getsysbus(SNES_Pntr, (int)(addr & 0xFFFFFF)),
					(addr, value) => { },
					1)
			};

			if (cart_RAM != null)
			{
				var CartRam = new MemoryDomainDelegate(
					"CartRAM",
					cart_RAM.Length,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getsram(SNES_Pntr, (int)(addr & (cart_RAM.Length - 1))),
					(addr, value) => cart_RAM[addr] = value,
					1);
				domains.Add(CartRam);
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}
	}
}
