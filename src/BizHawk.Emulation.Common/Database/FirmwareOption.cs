namespace BizHawk.Emulation.Common
{
	public readonly struct FirmwareOption
	{
		public readonly string Hash;

		public readonly FirmwareID ID;

		public readonly long Size;

		public FirmwareOption(FirmwareID id, string hash, long size)
		{
			Hash = hash;
			ID = id;
			Size = size;
		}
	}
}
