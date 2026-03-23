using System;

using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	public partial class GBHawkLink : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			// calculate total RAM present and set up buffer of that size
			uint size_total = 0;

			for (int i = 0; i < 4; i++)
			{
				if (Cart_RAM_Size[i] != 0)
				{
					size_total += Cart_RAM_Size[i];
				}
			}

			if (size_total != 0)
			{ 
				byte[] temp = new byte[size_total];

				uint index = 0;

				for (int i = 0; i < 4; i++)
				{
					if (Cart_RAM_Size[i] != 0)
					{
						for (int j = 0; j < cart_RAM[0].Length; j++)
						{
							cart_RAM[i][j] = LibGBHawkLink.GBLink_getsram(GBLink_Pntr, j, Current_sync_on_vbl[i], 0);

							temp[index] = cart_RAM[i][j];
							index++;

						}
					}
				}

				return temp;
			}

			return null;
		}

		public void StoreSaveRam(byte[] data)
		{
			int offset = 0;
			
			for (int i = 0; i < 4; i++)
			{
				if (Cart_RAM_Size[i] != 0)
				{
					Buffer.BlockCopy(data, offset, cart_RAM[i], 0, cart_RAM[i].Length);

					LibGBHawkLink.GBLink_load_SRAM(GBLink_Pntr, cart_RAM[i], (uint)cart_RAM[i].Length, (uint)i);

					offset += cart_RAM[i].Length;
				}

			}
		}

		public bool SaveRamModified => (has_bat[0] || has_bat[1] || has_bat[2] || has_bat[3]);
	}
}
