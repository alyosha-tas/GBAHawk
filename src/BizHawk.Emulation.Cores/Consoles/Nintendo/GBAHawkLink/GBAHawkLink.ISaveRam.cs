using System;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			// calculate total RAM present and set up buffer of that size
			uint size_total = 0;

			for (int i = 0; i < 4; i++)
			{
				if (Cart_RAM_Sizes[i] != 0)
				{
					size_total += Cart_RAM_Sizes[i];
				}
			}

			if (size_total != 0)
			{
				byte[] temp = new byte[size_total];

				uint index = 0;

				for (int i = 0; i < 4; i++)
				{
					if (Cart_RAM_Sizes[i] != 0)
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = LibGBAHawkLink.GBALink_getsram(GBA_Pntr, j, i);

							temp[index] = cart_RAMS[i][j];
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
				if (Cart_RAM_Sizes[i] != 0)
				{
					Buffer.BlockCopy(data, offset, cart_RAMS[i], 0, cart_RAMS[i].Length);

					LibGBAHawkLink.GBALink_load_SRAM(GBA_Pntr, cart_RAMS[i], (uint)cart_RAMS[i].Length, (uint)i);

					offset += cart_RAMS[i].Length;
				}

			}
		}

		public bool SaveRamModified => (has_bats[0] || has_bats[1] || has_bats[2] || has_bats[3]);
	}
}
