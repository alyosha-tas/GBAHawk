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
					0x800,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getram(NES_Pntr, (int)(addr & 0x7FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"ROM",
					ROM_Length,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getrom(NES_Pntr, (int)(addr & (ROM_Length - 1))),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"OAM",
					0x400,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getoam(NES_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"PALRAM",
					0x20,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getpalram(NES_Pntr, (int)(addr & 0x1F)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"PPU Bus",
					0x4000,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_get_ppu_bus(NES_Pntr, (uint)(addr & 0x3FFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"System Bus",
					0x10000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getsysbus(NES_Pntr, (int)(addr & 0xFFFF)),
					(addr, value) => { },
					1)
			};

			if (CHR_ROM_Length != 0)
			{
				var CartVRom = new MemoryDomainDelegate(
					"CHR ROM",
					CHR_ROM_Length,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getchrrom(NES_Pntr, (int)(addr & (CHR_ROM_Length - 1))),
					(addr, value) => { },
					1);
				domains.Add(CartVRom);
			}
			else if (vram_32)
			{
				var CartVRam = new MemoryDomainDelegate(
					"CHR RAM",
					0x8000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getvram(NES_Pntr, (int)(addr & 0x7FFF)),
					(addr, value) => { },
					1);
				domains.Add(CartVRam);
			}
			else
			{
				var CartVRam = new MemoryDomainDelegate(
					"CHR RAM",
					0x2000,
					MemoryDomain.Endian.Little,
					(addr) => LibSNESHawk.SNES_getvram(NES_Pntr, (int)(addr & 0x1FFF)),
					(addr, value) => { },
					1);
				domains.Add(CartVRam);
			}

			if (cart_RAM != null)
			{
				var CartRam = new MemoryDomainDelegate(
					"CartRAM",
					cart_RAM.Length,
					MemoryDomain.Endian.Little,
					addr => LibSNESHawk.SNES_getsram(NES_Pntr, (int)(addr & (cart_RAM.Length - 1))),
					(addr, value) => cart_RAM[addr] = value,
					1);
				domains.Add(CartRam);
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}
	}
}
