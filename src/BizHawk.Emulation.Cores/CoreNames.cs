using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores
{
	/// <summary>
	/// holds the names of ported and Hawk cores as consts,
	/// to be used for <see cref="CoreAttribute">[Core]</see> and various front-end needs
	/// (though sadly not in Designer files)
	/// </summary>
	public static class CoreNames
	{
		public const string GBAHawk_Debug = "GBAHawk_Debug";
		public const string GBAHawk = "GBAHawk";
		public const string GBAHawkLink = "GBAHawkLink";
		public const string GBHawk = "GBHawk";
		public const string GBHawkLink = "GBHawkLink";
		public const string GBHawkLink4x = "GBHawkLink4x";
		public const string SubGBHawk = "SubGBHawk";
	}
}
