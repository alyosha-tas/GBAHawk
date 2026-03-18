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

		private void SetupMemoryDomains()
		{
			var domains = new List<MemoryDomain> { };

			string[] con_list = {"A", "B", "C", "D" };

			for (int i = 0; i <Num_ROMS; i++)
			{
				var domain= new MemoryDomainDelegate(
								con_list[i] + " RAM",
								0x8000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getram(GBLink_Pntr, (int)(addr & 0x7FFF), Current_sync_on_vbl[i], (uint)i),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			for (int i = 0; i < Num_ROMS; i++)
			{
				var domain = new MemoryDomainDelegate(
								con_list[i] + " HRAM",
								0x80,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_gethram(GBLink_Pntr, (int)(addr & 0x7F), Current_sync_on_vbl[i], (uint)i),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			for (int i = 0; i < Num_ROMS; i++)
			{
				var domain = new MemoryDomainDelegate(
								con_list[i] + " ROM",
								ROM[i].Length,
								MemoryDomain.Endian.Little,
								addr => ROM[i][addr],
								(addr, value) => ROM[i][addr] = value,
								1);
				domains.Add(domain);
			}

			for (int i = 0; i < Num_ROMS; i++)
			{
				var domain = new MemoryDomainDelegate(
								con_list[i] + " VRAM",
								0x4000,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getvram(GBLink_Pntr, (int)(addr & 0x3FFF), Current_sync_on_vbl[i], (uint)i),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			for (int i = 0; i < Num_ROMS; i++)
			{
				var domain = new MemoryDomainDelegate(
								con_list[i] + " OAM",
								0xA0,
								MemoryDomain.Endian.Little,
								(addr) => LibGBHawkLink.GBLink_getoam(GBLink_Pntr, (int)(addr & 0xFF), Current_sync_on_vbl[i], (uint)i),
								(addr, value) => { },
								1);
				domains.Add(domain);
			}

			for (int i = 0; i < Num_ROMS; i++)
			{
				if (cart_RAM[i] != null)
				{
					var CartRam = new MemoryDomainDelegate(
						con_list[i] + " CartRAM",
						cart_RAM[i].Length,
						MemoryDomain.Endian.Little,
						addr => LibGBHawkLink.GBLink_getsram(GBLink_Pntr, (int)(addr & (cart_RAM[i].Length - 1)), Current_sync_on_vbl[i], (uint)i),
						(addr, value) => cart_RAM[i][addr] = value,
						1);
					domains.Add(CartRam);
				}
			}

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);
		}
	}
}
