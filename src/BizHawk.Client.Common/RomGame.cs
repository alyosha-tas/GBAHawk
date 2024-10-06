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

		/// <exception cref="Exception"><paramref name="file"/> does not exist</exception>
		public RomGame(HawkFile file)
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

			// note: this will be taking several hashes, of a potentially large amount of data.. yikes!
			GameInfo = Database.GetGameInfo(RomData, file.Name);
		}
	}
}
