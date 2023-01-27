using System.Collections.Generic;
using System.Linq;

namespace BizHawk.Emulation.Common
{
	// TODO: This should build itself from the Cores assembly, we don't want to maintain this
	public class SystemLookup
	{
		private readonly List<SystemInfo> _systems = new List<SystemInfo>
		{
			new(VSystemID.Raw.NES, "NES"),
			new(VSystemID.Raw.GB, "Gameboy"),
			new(VSystemID.Raw.GBA, "Gameboy Advance")
		};

		public SystemInfo this[string systemId]
			=> _systems.FirstOrDefault(s => s.SystemId == systemId)
			?? new SystemInfo("Unknown", "Unknown");

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
}
