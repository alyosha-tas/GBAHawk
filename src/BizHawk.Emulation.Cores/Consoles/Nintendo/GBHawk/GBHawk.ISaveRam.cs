using System;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawk
{
	public partial class GBHawk : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			if (cart_RAM != null)
			{
				for (int i = 0; i < cart_RAM.Length; i++)
				{
					cart_RAM[i] = LibGBHawk.GB_getsram(GB_Pntr, i);
				}

				return cart_RAM;
			}

			return (byte[])cart_RAM?.Clone();
		}

		public void StoreSaveRam(byte[] data)
		{
			Buffer.BlockCopy(data, 0, cart_RAM, 0, data.Length);
			Console.WriteLine("loading SRAM here");
			LibGBHawk.GB_load_SRAM(GB_Pntr, cart_RAM, (uint)cart_RAM.Length);
		}

		public bool SaveRamModified => has_bat;
	}
}
