using System;

using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.N64.Common;

namespace BizHawk.Emulation.Cores.Nintendo.N64Hawk
{
	public partial class N64Hawk : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			if (cart_RAM != null)
			{
				for (int i = 0; i < cart_RAM.Length; i++)
				{
					cart_RAM[i] = LibN64Hawk.N64_getsram(N64_Pntr, i);
				}

				return cart_RAM;
			}
			
			return (byte[])cart_RAM?.Clone();
		}

		public void StoreSaveRam(byte[] data)
		{
			Buffer.BlockCopy(data, 0, cart_RAM, 0, data.Length);
			Console.WriteLine("loading SRAM here");
			LibN64Hawk.N64_load_SRAM(N64_Pntr, cart_RAM, (uint)cart_RAM.Length);
		}

		public bool SaveRamModified => has_bat;
	}
}
