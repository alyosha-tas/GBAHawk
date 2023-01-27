using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using BizHawk.Common.PathExtensions;
using BizHawk.Emulation.Common;

using Newtonsoft.Json;

namespace BizHawk.Client.Common
{
	[JsonObject]
	public class PathEntryCollection : IEnumerable<PathEntry>
	{
		private static readonly string COMBINED_SYSIDS_GB = string.Join("_", VSystemID.Raw.GB, VSystemID.Raw.GBC);

		public static readonly string GLOBAL = string.Join("_", "Global", VSystemID.Raw.NULL);

		private static readonly Dictionary<string, string> _displayNameLookup = new()
		{
			[GLOBAL] = "Global",
			[VSystemID.Raw.NES] = "NES",
			[VSystemID.Raw.GBA] = "GBA",
			[COMBINED_SYSIDS_GB] = "Gameboy",
			[VSystemID.Raw.GBL] = "Gameboy Link",
		};

		private static PathEntry BaseEntryFor(string sysID, string path)
			=> new(sysID, "Base", path);

		private static IEnumerable<PathEntry> CommonEntriesFor(string sysID, string basePath, bool omitSaveRAM = false)
		{
			yield return BaseEntryFor(sysID, basePath);
			yield return ROMEntryFor(sysID);
			yield return SavestatesEntryFor(sysID);
			if (!omitSaveRAM) yield return SaveRAMEntryFor(sysID);
			yield return ScreenshotsEntryFor(sysID);
		}

		public static string GetDisplayNameFor(string sysID)
		{
			if (_displayNameLookup.TryGetValue(sysID, out var dispName)) return dispName;
			var newDispName = $"{sysID} (INTERIM)";
			_displayNameLookup[sysID] = newDispName;
			return newDispName;
		}

		public static bool InGroup(string sysID, string group)
			=> sysID == group || group.Split('_').Contains(sysID);

		private static PathEntry PalettesEntryFor(string sysID)
			=> new(sysID, "Palettes", Path.Combine(".", "Palettes"));

		private static PathEntry ROMEntryFor(string sysID, string path = ".")
			=> new(sysID, "ROM", path);

		private static PathEntry SaveRAMEntryFor(string sysID)
			=> new(sysID, "Save RAM", Path.Combine(".", "SaveRAM"));

		private static PathEntry SavestatesEntryFor(string sysID)
			=> new(sysID, "Savestates", Path.Combine(".", "State"));

		private static PathEntry ScreenshotsEntryFor(string sysID)
			=> new(sysID, "Screenshots", Path.Combine(".", "Screenshots"));

		public List<PathEntry> Paths { get; }

		[JsonConstructor]
		public PathEntryCollection(List<PathEntry> paths)
		{
			Paths = paths;
		}

		public PathEntryCollection() : this(new List<PathEntry>(Defaults.Value)) {}

		public bool UseRecentForRoms { get; set; }
		public string LastRomPath { get; set; } = ".";

		public IEnumerator<PathEntry> GetEnumerator() => Paths.GetEnumerator();
		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

		public PathEntry this[string system, string type] =>
			Paths.FirstOrDefault(p => p.IsSystem(system) && p.Type == type)
			?? TryGetDebugPath(system, type);

		private PathEntry TryGetDebugPath(string system, string type)
		{
			if (Paths.Any(p => p.IsSystem(system)))
			{
				// we have the system, but not the type.  don't attempt to add an unknown type
				return null;
			}

			// we don't have anything for the system in question.  add a set of stock paths
			Paths.AddRange(CommonEntriesFor(system, basePath: Path.Combine(".", $"{system.RemoveInvalidFileSystemChars()}_INTERIM")));

			return this[system, type];
		}

		public void ResolveWithDefaults()
		{
			// Add missing entries
			foreach (var defaultPath in Defaults.Value)
			{
				var path = Paths.FirstOrDefault(p => p.System == defaultPath.System && p.Type == defaultPath.Type);
				if (path == null)
				{
					Paths.Add(defaultPath);
				}
			}

			var entriesToRemove = new List<PathEntry>();

			// Remove entries that no longer exist in defaults
			foreach (PathEntry pathEntry in Paths)
			{
				var path = Defaults.Value.FirstOrDefault(p => p.System == pathEntry.System && p.Type == pathEntry.Type);
				if (path == null)
				{
					entriesToRemove.Add(pathEntry);
				}
			}

			foreach (PathEntry entry in entriesToRemove)
			{
				Paths.Remove(entry);
			}
		}

		[JsonIgnore]
		public string FirmwaresPathFragment => this[GLOBAL, "Firmware"].Path;

		[JsonIgnore]
		internal string TempFilesFragment => this[GLOBAL, "Temp Files"].Path;

		public static Lazy<IReadOnlyList<PathEntry>> Defaults = new(() => new[]
		{
			new[] {
				BaseEntryFor(GLOBAL, "."),
				ROMEntryFor(GLOBAL),
				new(GLOBAL, "Firmware", Path.Combine(".", "Firmware")),
				new(GLOBAL, "Movies", Path.Combine(".", "Movies")),
				new(GLOBAL, "Movie backups", Path.Combine(".", "Movies", "backup")),
				new(GLOBAL, "A/V Dumps", "."),
				new(GLOBAL, "Tools", Path.Combine(".", "Tools")),
				new(GLOBAL, "Lua", Path.Combine(".", "Lua")),
				new(GLOBAL, "Watch (.wch)", Path.Combine(".", ".")),
				new(GLOBAL, "Debug Logs", Path.Combine(".", "")),
				new(GLOBAL, "Macros", Path.Combine(".", "Movies", "Macros")),
				new(GLOBAL, "TAStudio states", Path.Combine(".", "Movies", "TAStudio states")),
				new(GLOBAL, "Multi-Disk Bundles", Path.Combine(".", "")),
				new(GLOBAL, "External Tools", Path.Combine(".", "ExternalTools")),
				new(GLOBAL, "Temp Files", ""),
			},

			CommonEntriesFor(VSystemID.Raw.GBL, basePath: Path.Combine(".", "Gameboy Link")),
			new[] {
				PalettesEntryFor(VSystemID.Raw.GBL),
			},

			CommonEntriesFor(COMBINED_SYSIDS_GB, basePath: Path.Combine(".", "Gameboy")),
			new[] {
				PalettesEntryFor(COMBINED_SYSIDS_GB),
			},

			CommonEntriesFor(VSystemID.Raw.GBA, basePath: Path.Combine(".", "GBA")),

			CommonEntriesFor(VSystemID.Raw.NES, basePath: Path.Combine(".", "NES")),
			new[] {
				PalettesEntryFor(VSystemID.Raw.NES),
			},

		}.SelectMany(a => a).ToArray());
	}
}
