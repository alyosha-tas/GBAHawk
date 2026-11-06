namespace BizHawk.Emulation.Common
{
	public readonly struct FirmwareFile
	{
		public readonly string Description;

		public readonly string Hash;

		public readonly string Info;

		public readonly string RecommendedName;

		public readonly long Size;

		public FirmwareFile(
			string hash,
			long size,
			string recommendedName,
			string desc,
			string additionalInfo = "")
		{
			Description = desc;
			Hash = hash;
			Info = additionalInfo;
			RecommendedName = recommendedName;
			Size = size;
		}
	}
}
