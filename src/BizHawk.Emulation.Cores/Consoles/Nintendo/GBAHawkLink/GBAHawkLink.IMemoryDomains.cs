using System.Collections.Generic;
using System.Linq;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink
	{
		private MemoryDomainList MemoryDomains;
		private readonly Dictionary<string, MemoryDomainByteArray> _byteArrayDomains = new Dictionary<string, MemoryDomainByteArray>();
		MemoryDomainDelegate domain;
		private void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain> { };

			domain = new MemoryDomainDelegate(
								"A WRAM",
								0x40000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getwram(GBA_Pntr, (int)(addr & 0x3FFFF), 0),
								(addr, value) => { },
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B WRAM",
								0x40000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getwram(GBA_Pntr, (int)(addr & 0x3FFFF), 1),
								(addr, value) => { },
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C WRAM",
								0x40000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getwram(GBA_Pntr, (int)(addr & 0x3FFFF), 2),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D WRAM",
								0x40000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getwram(GBA_Pntr, (int)(addr & 0x3FFFF), 3),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A IWRAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getiwram(GBA_Pntr, (int)(addr & 0x7FFF), 0),
								(addr, value) => { },
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B IWRAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getiwram(GBA_Pntr, (int)(addr & 0x7FFF), 1),
								(addr, value) => { },
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C IWRAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getiwram(GBA_Pntr, (int)(addr & 0x7FFF), 2),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D IWRAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getiwram(GBA_Pntr, (int)(addr & 0x7FFF), 3),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"ROM A",
								ROMS[0].Length,
								MemoryDomain.Endian.Little,
								addr => ROMS[0][addr],
								(addr, value) => ROMS[0][addr] = value,
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"ROM B",
								ROMS[1].Length,
								MemoryDomain.Endian.Little,
								addr => ROMS[1][addr],
								(addr, value) => ROMS[1][addr] = value,
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"ROM C",
								ROMS[2].Length,
								MemoryDomain.Endian.Little,
								addr => ROMS[2][addr],
								(addr, value) => ROMS[2][addr] = value,
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"ROM D",
								ROMS[3].Length,
								MemoryDomain.Endian.Little,
								addr => ROMS[3][addr],
								(addr, value) => ROMS[3][addr] = value,
								1);

				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A VRAM",
								0x18000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getvram(GBA_Pntr, (int)(addr & 0x1FFFF), 0),
								(addr, value) => { },
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B VRAM",
								0x18000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getvram(GBA_Pntr, (int)(addr & 0x1FFFF), 1),
								(addr, value) => { },
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C VRAM",
								0x18000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getvram(GBA_Pntr, (int)(addr & 0x1FFFF), 2),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D VRAM",
								0x18000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getvram(GBA_Pntr, (int)(addr & 0x1FFFF), 3),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A OAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getoam(GBA_Pntr, (int)(addr & 0x3FF), 0),
								(addr, value) => { },
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B OAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getoam(GBA_Pntr, (int)(addr & 0x3FF), 1),
								(addr, value) => { },
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C OAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getoam(GBA_Pntr, (int)(addr & 0x3FF), 2),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D OAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getoam(GBA_Pntr, (int)(addr & 0x3FF), 3),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A PALRAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getpalram(GBA_Pntr, (int)(addr & 0x3FF), 0),
								(addr, value) => { },
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B PALRAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getpalram(GBA_Pntr, (int)(addr & 0x3FF), 1),
								(addr, value) => { },
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C PALRAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getpalram(GBA_Pntr, (int)(addr & 0x3FF), 2),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D PALRAM",
								0x400,
								MemoryDomain.Endian.Little,
								addr => LibGBAHawkLink.GBALink_getpalram(GBA_Pntr, (int)(addr & 0x3FF), 3),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A System Bus",
								0x10000000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getsysbus(GBA_Pntr, (int)(addr & 0xFFFFFFF), 0),
								(addr, value) => { },
								1);

			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B System Bus",
								0x10000000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getsysbus(GBA_Pntr, (int)(addr & 0xFFFFFFF), 1),
								(addr, value) => { },
								1);

			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C System Bus",
								0x10000000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getsysbus(GBA_Pntr, (int)(addr & 0xFFFFFFF), 2),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D System Bus",
								0x10000000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBAHawkLink.GBALink_getsysbus(GBA_Pntr, (int)(addr & 0xFFFFFFF), 3),
								(addr, value) => { },
								1);

				domains.Add(domain);
			}

			if (cart_RAMS[0] != null)
			{
				domain = new MemoryDomainDelegate(
					"CartRAM A",
					cart_RAMS[0].Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBALink_getsram(GBA_Pntr, (int)(addr & (cart_RAMS[0].Length - 1)), 0),
					(addr, value) => cart_RAMS[0][addr] = value,
					1);
				domains.Add(domain);
			}

			if (cart_RAMS[1] != null)
			{
				domain = new MemoryDomainDelegate(
					"CartRAM B",
					cart_RAMS[1].Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBALink_getsram(GBA_Pntr, (int)(addr & (cart_RAMS[1].Length - 1)), 1),
					(addr, value) => cart_RAMS[1][addr] = value,
					1);
				domains.Add(domain);
			}

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
					"CartRAM C",
					cart_RAMS[2].Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBALink_getsram(GBA_Pntr, (int)(addr & (cart_RAMS[2].Length - 1)), 2),
					(addr, value) => cart_RAMS[2][addr] = value,
					1);
				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
					"CartRAM D",
					cart_RAMS[3].Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBALink_getsram(GBA_Pntr, (int)(addr & (cart_RAMS[3].Length - 1)), 3),
					(addr, value) => cart_RAMS[3][addr] = value,
					1);
				domains.Add(domain);
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}
	}
}
