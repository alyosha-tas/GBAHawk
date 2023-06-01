using System.Collections.Generic;
using System.Linq;

using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink
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
					(addr) => LibGBAHawkLink.GBA_getwram(GBA_Pntr, (int)(addr & 0x3FFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"IWRAM",
					0x8000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawkLink.GBA_getiwram(GBA_Pntr, (int)(addr & 0x7FFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"VRAM",
					0x18000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawkLink.GBA_getvram(GBA_Pntr, (int)(addr & 0x1FFFF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"OAM",
					0x400,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBA_getoam(GBA_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"PALRAM",
					0x400,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBA_getpalram(GBA_Pntr, (int)(addr & 0x3FF)),
					(addr, value) => { },
					1),
				new MemoryDomainDelegate(
					"System Bus",
					0x10000000,
					MemoryDomain.Endian.Little,
					(addr) => LibGBAHawkLink.GBA_getsysbus(GBA_Pntr, (int)(addr & 0xFFFFFFF)),
					(addr, value) => { },
					1)
			};
			
			if (cart_RAMS[0] != null)
			{
				var CartRam0 = new MemoryDomainDelegate(
					"CartRAM 0",
					cart_RAMS[0].Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBA_getsram(GBA_Pntr, (int)(addr & (cart_RAMS[0].Length - 1)), 0),
					(addr, value) => cart_RAMS[0][addr] = value,
					1);
				domains.Add(CartRam0);
			}

			if (cart_RAMS[1] != null)
			{
				var CartRam1 = new MemoryDomainDelegate(
					"CartRAM 1",
					cart_RAMS[1].Length,
					MemoryDomain.Endian.Little,
					addr => LibGBAHawkLink.GBA_getsram(GBA_Pntr, (int)(addr & (cart_RAMS[1].Length - 1)), 1),
					(addr, value) => cart_RAMS[1][addr] = value,
					1);
				domains.Add(CartRam1);
			}

			SyncAllByteArrayDomains();

			MemoryDomains = new MemoryDomainList(_byteArrayDomains.Values.Concat(domains).ToList());
			(ServiceProvider as BasicServiceProvider).Register<IMemoryDomains>(MemoryDomains);

			_memoryDomainsInit = true;
		}

		private void SyncAllByteArrayDomains()
		{
			SyncByteArrayDomain("ROM 0", ROMS[0]);
			SyncByteArrayDomain("ROM 1", ROMS[1]);
		}

		private void SyncByteArrayDomain(string name, byte[] data)
		{
			if (_memoryDomainsInit)
			{
				var m = _byteArrayDomains[name];
				m.Data = data;
			}
			else
			{
				var m = new MemoryDomainByteArray(name, MemoryDomain.Endian.Little, data, true, 1);
				_byteArrayDomains.Add(name, m);
			}
		}
	}
}
