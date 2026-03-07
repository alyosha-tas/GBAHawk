using System;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class GBHawkOld : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			return (byte[])cart_RAM?.Clone();
		}

		public void StoreSaveRam(byte[] data)
		{
			Buffer.BlockCopy(data, 0, cart_RAM, 0, data.Length);
			Console.WriteLine("loading SRAM here");
		}

		public bool SaveRamModified => has_bat;
	}
}
