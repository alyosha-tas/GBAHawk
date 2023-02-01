using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace BizHawk.Emulation.Common
{
	/// <summary>
	/// You probably want <see cref="Raw"/>.
	/// It's laid out this way to match a local branch of mine where this is a struct. --yoshi
	/// </summary>
	public static class VSystemID
	{
		public static class Raw
		{
			public const string DEBUG = "DEBUG";
			public const string GB = "GB";
			public const string GBA = "GBA";
			public const string GBC = "GBC";
			public const string GBL = "GBL";
			public const string NULL = "NULL";
		}

		private static List<string>? _allSysIDs = null;

		private static List<string> AllSysIDs
			=> _allSysIDs ??= typeof(Raw).GetFields(BindingFlags.Public | BindingFlags.Static)
				.Select(x => (string) x.GetRawConstantValue())
				.OrderBy(s => s)
				.ToList();

		/// <returns><paramref name="sysID"/> iff it's in <see cref="Raw">the valid list</see>, else <see langword="null"/></returns>
		public static string? Validate(string sysID)
			=> AllSysIDs.BinarySearch(sysID) < 0 ? null : sysID;
	}
}
