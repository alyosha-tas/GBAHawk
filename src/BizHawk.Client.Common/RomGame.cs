using System;
using System.Globalization;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.Common
{
	public class RomGame
	{
		public byte[] RomData { get; }
		public byte[] FileData { get; }
		public GameInfo GameInfo { get; }
		public string Extension { get; }

		private const int BankSize = 1024;

		public RomGame(HawkFile file)
			: this(file, null)
		{
		}

		/// <exception cref="Exception"><paramref name="file"/> does not exist</exception>
		public RomGame(HawkFile file, string patch)
		{
			if (!file.Exists)
			{
				throw new Exception("The file needs to exist, yo.");
			}

			Extension = file.Extension.ToUpperInvariant();

			var stream = file.GetStream();
			int fileLength = (int)stream.Length;

			// read the entire file into FileData.
			FileData = new byte[fileLength];
			stream.Position = 0;
			stream.Read(FileData, 0, fileLength);

			string SHA1_check = SHA1Checksum.ComputePrefixedHex(FileData);

			RomData = FileData;

			if (file.Extension == ".z64" || file.Extension == ".n64" || file.Extension == ".v64")
			{
				// Use a simple magic number to detect N64 rom format, then byteswap the ROM to ensure a consistent endianness/order
				RomData = RomData[0] switch
				{
					0x37 => EndiannessUtils.ByteSwap16(RomData), // V64 format (byte swapped)
					0x40 => EndiannessUtils.ByteSwap32(RomData), // N64 format (word swapped)
					_ => RomData // Z64 format (no swap), or something unexpected; in either case do nothing
				};
			}

			// note: this will be taking several hashes, of a potentially large amount of data.. yikes!
			GameInfo = Database.GetGameInfo(RomData, file.Name);

			CheckForPatchOptions();

			if (patch != null)
			{
				using var patchFile = new HawkFile(patch);
				patchFile.BindFirstOf(".ips");
				if (patchFile.IsBound)
				{
					RomData = IPS.Patch(RomData, patchFile.GetStream());
				}
			}
		}

		private void CheckForPatchOptions()
		{
			try
			{
				if (GameInfo["PatchBytes"])
				{
					var args = GameInfo.OptionValue("PatchBytes");
					foreach (var val in args.Split(','))
					{
						var split = val.Split(':');
						int offset = int.Parse(split[0], NumberStyles.HexNumber);
						byte value = byte.Parse(split[1], NumberStyles.HexNumber);
						RomData[offset] = value;
					}
				}
			}
			catch (Exception)
			{
				// No need for errors in patching to propagate.
			}
		}
	}
}
