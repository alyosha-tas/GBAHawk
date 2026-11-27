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
		public const string NESHawk2 = "NESHawk2";
		public const string SubNESHawk2 = "SubNESHawk2";
		public const string GBAHawk = "GBAHawk";
		public const string GBAHawkLink = "GBAHawkLink";
		public const string GBHawk = "GBHawk";
		public const string GBCHawk = "GBCHawk";
		public const string GBHawkLink = "GBHawkLink";
		public const string GBCHawkLink = "GBCHawkLink";
		public const string SubGBHawk = "SubGBHawk";
		public const string SubGBCHawk = "SubGBCHawk";
		public const string SubGBAHawk = "SubGBAHawk";
		public const string SNESHawk = "SNESHawk";
	}
}
