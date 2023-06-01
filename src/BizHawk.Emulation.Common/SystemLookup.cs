using System.Collections.Generic;
using System.Linq;

namespace BizHawk.Emulation.Common
{
	// TODO: This should build itself from the Cores assembly, we don't want to maintain this
	public class SystemLookup
	{
		private readonly List<SystemInfo> _systems = new List<SystemInfo>
		{
			new(VSystemID.Raw.GB, "Gameboy"),
			new(VSystemID.Raw.GBA, "Gameboy Advance")
		};

		public IEnumerable<SystemInfo> AllSystems => _systems;

		public class SystemInfo
		{
			public SystemInfo(string systemId, string fullName)
			{
				SystemId = systemId;
				FullName = fullName;
			}

			public string SystemId { get; }
			public string FullName { get; }
		}
	}

	public static class VSystemID
	{
		public static class Raw
		{
			public const string DEBUG = "DEBUG";
			public const string GB = "GB";
			public const string GBA = "GBA";
			public const string GBAL = "GBAL";
			public const string GBC = "GBC";
			public const string GBL = "GBL";
			public const string NULL = "NULL";
		}
	}
}
