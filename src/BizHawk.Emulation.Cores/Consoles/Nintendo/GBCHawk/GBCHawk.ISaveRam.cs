using System;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBCHawk
{
	public partial class GBCHawk : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			if (cart_RAM != null)
			{
				for (int i = 0; i < cart_RAM.Length; i++)
				{
					cart_RAM[i] = LibGBCHawk.GBC_getsram(GBC_Pntr, i, Current_sync_on_vbl);
				}

				return cart_RAM;
			}

			return (byte[])cart_RAM?.Clone();
		}

		public void StoreSaveRam(byte[] data)
		{
			Buffer.BlockCopy(data, 0, cart_RAM, 0, data.Length);
			Console.WriteLine("loading SRAM here");
			LibGBCHawk.GBC_load_SRAM(GBC_Pntr, cart_RAM, (uint)cart_RAM.Length);
		}

		public bool SaveRamModified => has_bat;
	}
}
