using System;
using System.Collections.Generic;
using System.Linq;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	public partial class GBHawkLink
	{
		private MemoryDomainList MemoryDomains;
		private readonly Dictionary<string, MemoryDomainByteArray> _byteArrayDomains = new Dictionary<string, MemoryDomainByteArray>();
		MemoryDomainDelegate domain;
		private void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain> { };

			domain = new MemoryDomainDelegate(
								"A RAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getram(GBLink_Pntr, (int)(addr & 0x7FFF), Current_sync_on_vbl[0], 0),
								(addr, value) => { },
								1);
			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B RAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getram(GBLink_Pntr, (int)(addr & 0x7FFF), Current_sync_on_vbl[1], 1),
								(addr, value) => { },
								1);
			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C RAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getram(GBLink_Pntr, (int)(addr & 0x7FFF), Current_sync_on_vbl[2], 2),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D RAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getram(GBLink_Pntr, (int)(addr & 0x7FFF), Current_sync_on_vbl[3], 3),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A HRAM",
								0x80,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_gethram(GBLink_Pntr, (int)(addr & 0x7F), Current_sync_on_vbl[0], 0),
								(addr, value) => { },
								1);
			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B HRAM",
								0x80,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_gethram(GBLink_Pntr, (int)(addr & 0x7F), Current_sync_on_vbl[1], 1),
								(addr, value) => { },
								1);
			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C HRAM",
								0x80,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_gethram(GBLink_Pntr, (int)(addr & 0x7F), Current_sync_on_vbl[2], 2),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D HRAM",
								0x80,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_gethram(GBLink_Pntr, (int)(addr & 0x7F), Current_sync_on_vbl[3], 3),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
				"A ROM",
				ROM[0].Length,
				MemoryDomain.Endian.Little,
				addr => ROM[0][addr],
				(addr, value) => { },
				1);
			domains.Add(domain);

			domain = new MemoryDomainDelegate(
				"B ROM",
				ROM[1].Length,
				MemoryDomain.Endian.Little,
				addr => ROM[1][addr],
				(addr, value) => { },
				1);
			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
					"C ROM",
					ROM[2].Length,
					MemoryDomain.Endian.Little,
					addr => ROM[2][addr],
					(addr, value) => { },
					1);
				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
					"D ROM",
					ROM[3].Length,
					MemoryDomain.Endian.Little,
					addr => ROM[3][addr],
					(addr, value) => { },
					1);
				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A VRAM",
								0x4000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getvram(GBLink_Pntr, (int)(addr & 0x3FFF), Current_sync_on_vbl[0], 0),
								(addr, value) => { },
								1);
			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B VRAM",
								0x4000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getvram(GBLink_Pntr, (int)(addr & 0x3FFF), Current_sync_on_vbl[1], 1),
								(addr, value) => { },
								1);
			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C VRAM",
								0x4000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getvram(GBLink_Pntr, (int)(addr & 0x3FFF), Current_sync_on_vbl[2], 2),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D VRAM",
								0x4000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getvram(GBLink_Pntr, (int)(addr & 0x3FFF), Current_sync_on_vbl[3], 3),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			domain = new MemoryDomainDelegate(
								"A OAM",
								0xA0,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getoam(GBLink_Pntr, (int)(addr & 0xFF), Current_sync_on_vbl[0], 0),
								(addr, value) => { },
								1);
			domains.Add(domain);

			domain = new MemoryDomainDelegate(
								"B OAM",
								0xA0,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getoam(GBLink_Pntr, (int)(addr & 0xFF), Current_sync_on_vbl[1], 1),
								(addr, value) => { },
								1);
			domains.Add(domain);

			if (Num_ROMS > 2)
			{
				domain = new MemoryDomainDelegate(
								"C OAM",
								0xA0,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getoam(GBLink_Pntr, (int)(addr & 0xFF), Current_sync_on_vbl[2], 2),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			if (Num_ROMS > 3)
			{
				domain = new MemoryDomainDelegate(
								"D OAM",
								0xA0,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getoam(GBLink_Pntr, (int)(addr & 0xFF), Current_sync_on_vbl[3], 3),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			if (cart_RAM[0] != null)
			{
				domain = new MemoryDomainDelegate(
								"A CartRAM",
								cart_RAM[0].Length,
								MemoryDomain.Endian.Little,
								addr => LibGBHawkLink.GBLink_getsram(GBLink_Pntr, (int)(addr & (cart_RAM[0].Length - 1)), Current_sync_on_vbl[0], 0),
								(addr, value) => cart_RAM[0][addr] = value,
								1);
				domains.Add(domain);
			}

			if (cart_RAM[1] != null)
			{
				domain = new MemoryDomainDelegate(
								"B CartRAM",
								cart_RAM[1].Length,
								MemoryDomain.Endian.Little,
								addr => LibGBHawkLink.GBLink_getsram(GBLink_Pntr, (int)(addr & (cart_RAM[1].Length - 1)), Current_sync_on_vbl[1], 1),
								(addr, value) => cart_RAM[1][addr] = value,
								1);
				domains.Add(domain);
			}

			if (Num_ROMS > 2)
			{
				if (cart_RAM[2] != null)
				{
					domain = new MemoryDomainDelegate(
									"C CartRAM",
									cart_RAM[2].Length,
									MemoryDomain.Endian.Little,
									addr => LibGBHawkLink.GBLink_getsram(GBLink_Pntr, (int)(addr & (cart_RAM[2].Length - 1)), Current_sync_on_vbl[2], 2),
									(addr, value) => cart_RAM[2][addr] = value,
									1);
					domains.Add(domain);
				}
			}

			if (Num_ROMS > 3)
			{
				if (cart_RAM[3] != null)
				{
					domain = new MemoryDomainDelegate(
									"D CartRAM",
									cart_RAM[3].Length,
									MemoryDomain.Endian.Little,
									addr => LibGBHawkLink.GBLink_getsram(GBLink_Pntr, (int)(addr & (cart_RAM[3].Length - 1)), Current_sync_on_vbl[3], 3),
									(addr, value) => cart_RAM[3][addr] = value,
									1);
					domains.Add(domain);
				}
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}
	}
}
