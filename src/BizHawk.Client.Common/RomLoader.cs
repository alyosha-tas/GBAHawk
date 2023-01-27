using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;

using BizHawk.Common;
using BizHawk.Common.StringExtensions;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores;

namespace BizHawk.Client.Common
{
	public class RomLoader
	{
		private class RomAsset : IRomAsset
		{
			public byte[] RomData { get; set; }
			public byte[] FileData { get; set; }
			public string Extension { get; set; }
			public string RomPath { get; set; }
			public GameInfo Game { get; set; }
		}
		private class CoreInventoryParameters : ICoreInventoryParameters
		{
			private readonly RomLoader _parent;
			public CoreInventoryParameters(RomLoader parent)
			{
				_parent = parent;
			}
			public CoreComm Comm { get; set; }

			public GameInfo Game { get; set; }

			public List<IRomAsset> Roms { get; set; } = new List<IRomAsset>();

			public bool DeterministicEmulationRequested => _parent.Deterministic;

			public object FetchSettings(Type emulatorType, Type settingsType)
				=> _parent.GetCoreSettings(emulatorType, settingsType);

			public object FetchSyncSettings(Type emulatorType, Type syncSettingsType)
				=> _parent.GetCoreSyncSettings(emulatorType, syncSettingsType);
		}
		private readonly Config _config;

		public RomLoader(Config config)
		{
			_config = config;
		}

		public enum LoadErrorType
		{
			Unknown, MissingFirmware, Xml, DiscError
		}

		// helper methods for the settings events
		private TSetting GetCoreSettings<TCore, TSetting>()
			where TCore : IEmulator
		{
			return (TSetting)GetCoreSettings(typeof(TCore), typeof(TSetting));
		}

		private TSync GetCoreSyncSettings<TCore, TSync>()
			where TCore : IEmulator
		{
			return (TSync)GetCoreSyncSettings(typeof(TCore), typeof(TSync));
		}

		private object GetCoreSettings(Type t, Type settingsType)
		{
			var e = new SettingsLoadArgs(t, settingsType);
			if (OnLoadSettings == null)
				throw new InvalidOperationException("Frontend failed to provide a settings getter");
			OnLoadSettings(this, e);
			if (e.Settings != null && e.Settings.GetType() != settingsType)
				throw new InvalidOperationException($"Frontend did not provide the requested settings type: Expected {settingsType}, got {e.Settings.GetType()}");
			return e.Settings;
		}

		private object GetCoreSyncSettings(Type t, Type syncSettingsType)
		{
			var e = new SettingsLoadArgs(t, syncSettingsType);
			if (OnLoadSyncSettings == null)
				throw new InvalidOperationException("Frontend failed to provide a sync settings getter");
			OnLoadSyncSettings(this, e);
			if (e.Settings != null && e.Settings.GetType() != syncSettingsType)
				throw new InvalidOperationException($"Frontend did not provide the requested sync settings type: Expected {syncSettingsType}, got {e.Settings.GetType()}");
			return e.Settings;
		}

		// For not throwing errors but simply outputting information to the screen
		public Action<string> MessageCallback { get; set; }

		// TODO: reconsider the need for exposing these;
		public IEmulator LoadedEmulator { get; private set; }
		public GameInfo Game { get; private set; }
		public RomGame Rom { get; private set; }
		public string CanonicalFullPath { get; private set; }

		public bool Deterministic { get; set; }

		public class RomErrorArgs : EventArgs
		{
			// TODO: think about naming here, what to pass, a lot of potential good information about what went wrong could go here!
			public RomErrorArgs(string message, string systemId, LoadErrorType type)
			{
				Message = message;
				AttemptedCoreLoad = systemId;
				Type = type;
			}

			public RomErrorArgs(string message, string systemId, string path, bool? det, LoadErrorType type)
				: this(message, systemId, type)
			{
				Deterministic = det;
				RomPath = path;
			}

			public string Message { get; }
			public string AttemptedCoreLoad { get; }
			public string RomPath { get; }
			public bool? Deterministic { get; set; }
			public bool Retry { get; set; }
			public LoadErrorType Type { get; }
		}

		public class SettingsLoadArgs : EventArgs
		{
			public object Settings { get; set; }
			public Type Core { get; }
			public Type SettingsType { get; }
			public SettingsLoadArgs(Type t, Type s)
			{
				Core = t;
				SettingsType = s;
				Settings = null;
			}
		}

		public delegate void SettingsLoadEventHandler(object sender, SettingsLoadArgs e);
		public event SettingsLoadEventHandler OnLoadSettings;
		public event SettingsLoadEventHandler OnLoadSyncSettings;

		public delegate void LoadErrorEventHandler(object sender, RomErrorArgs e);
		public event LoadErrorEventHandler OnLoadError;

		public Func<HawkFile, int?> ChooseArchive { get; set; }

		public Func<RomGame, string> ChoosePlatform { get; set; }

		// in case we get sent back through the picker more than once, use the same choice the second time
		private int? _previousChoice;
		private int? HandleArchive(HawkFile file)
		{
			if (_previousChoice.HasValue)
			{
				return _previousChoice;
			}

			if (ChooseArchive != null)
			{
				_previousChoice = ChooseArchive(file);
				return _previousChoice;
			}

			return null;
		}

		// May want to phase out this method in favor of the overload with more parameters
		private void DoLoadErrorCallback(string message, string systemId, LoadErrorType type = LoadErrorType.Unknown)
		{
			OnLoadError?.Invoke(this, new RomErrorArgs(message, systemId, type));
		}

		private void DoLoadErrorCallback(string message, string systemId, string path, bool det, LoadErrorType type = LoadErrorType.Unknown)
		{
			OnLoadError?.Invoke(this, new RomErrorArgs(message, systemId, path, det, type));
		}

		private bool HandleArchiveBinding(HawkFile file)
		{
			// try binding normal rom extensions first
			if (!file.IsBound)
			{
				file.BindSoleItemOf(RomFileExtensions.AutoloadFromArchive);
			}
			// ...including unrecognised extensions that the user has set a platform for
			if (!file.IsBound)
			{
				var exts = _config.PreferredPlatformsForExtensions.Where(static kvp => !string.IsNullOrEmpty(kvp.Value))
					.Select(static kvp => kvp.Key)
					.ToList();
				if (exts.Count is not 0) file.BindSoleItemOf(exts);
			}

			// if we have an archive and need to bind something, then pop the dialog
			if (file.IsArchive && !file.IsBound)
			{
				int? result = HandleArchive(file);
				if (result.HasValue)
				{
					file.BindArchiveMember(result.Value);
				}
				else
				{
					return false;
				}
			}

			CanonicalFullPath = file.CanonicalFullPath;

			return true;
		}

		private IEmulator MakeCoreFromCoreInventory(CoreInventoryParameters cip, string forcedCoreName = null)
		{
			IReadOnlyCollection<CoreInventory.Core> cores;
			if (forcedCoreName != null)
			{
				var singleCore = CoreInventory.Instance.GetCores(cip.Game.System).SingleOrDefault(c => c.Name == forcedCoreName);
				cores = singleCore != null ? new[] { singleCore } : Array.Empty<CoreInventory.Core>();
			}
			else
			{
				_config.PreferredCores.TryGetValue(cip.Game.System, out var preferredCore);
				var dbForcedCoreName = cip.Game.ForcedCore;
				cores = CoreInventory.Instance.GetCores(cip.Game.System)
					.OrderBy(c =>
					{
						if (c.Name == preferredCore)
						{
							return (int)CorePriority.UserPreference;
						}

						if (string.Equals(c.Name, dbForcedCoreName, StringComparison.InvariantCultureIgnoreCase))
						{
							return (int)CorePriority.GameDbPreference;
						}

						return (int)c.Priority;
					})
					.ToList();
				if (cores.Count == 0) throw new InvalidOperationException("No core was found to try on the game");
			}
			var exceptions = new List<Exception>();
			foreach (var core in cores)
			{
				try
				{
					return core.Create(cip);
				}
				catch (Exception e)
				{
					if (_config.DontTryOtherCores || e is MissingFirmwareException || e.InnerException is MissingFirmwareException)
						throw;
					exceptions.Add(e);
				}
			}
			throw new AggregateException("No core could load the game", exceptions);
		}

		private void LoadOther(CoreComm nextComm, HawkFile file, string forcedCoreName, out IEmulator nextEmulator, out RomGame rom, out GameInfo game, out bool cancel)
		{
			cancel = false;
			rom = new RomGame(file);

			// hacky for now
			if (file.Extension == ".nsf")
			{
				rom.GameInfo.System = VSystemID.Raw.NES;
			}

			Util.DebugWriteLine(rom.GameInfo.System);

			if (string.IsNullOrEmpty(rom.GameInfo.System))
			{
				// Has the user picked a preference for this extension?
				if (_config.TryGetChosenSystemForFileExt(rom.Extension.ToLowerInvariant(), out var systemID))
				{
					rom.GameInfo.System = systemID;
				}
				else if (ChoosePlatform != null)
				{
					var result = ChoosePlatform(rom);
					if (!string.IsNullOrEmpty(result))
					{
						rom.GameInfo.System = result;
					}
					else
					{
						cancel = true;
					}
				}
			}

			game = rom.GameInfo;

			nextEmulator = null;
			if (game.System == null)
				return; // The user picked nothing in the Core picker

			switch (game.System)
			{
				case VSystemID.Raw.GB:
				case VSystemID.Raw.GBC:
					break;
			}
			var cip = new CoreInventoryParameters(this)
			{
				Comm = nextComm,
				Game = game,
				Roms =
				{
					new RomAsset
					{
						RomData = rom.RomData,
						FileData = rom.FileData,
						Extension = rom.Extension,
						RomPath = file.FullPathWithoutMember,
						Game = game
					}
				},
			};
			nextEmulator = MakeCoreFromCoreInventory(cip, forcedCoreName);
		}

		private bool LoadXML(string path, CoreComm nextComm, HawkFile file, string forcedCoreName, out IEmulator nextEmulator, out RomGame rom, out GameInfo game)
		{
			nextEmulator = null;
			rom = null;
			game = null;
			try
			{
				var xmlGame = XmlGame.Create(file); // if load fails, are we supposed to retry as a bsnes XML????????
				game = xmlGame.GI;

				var system = game.System;
				var cip = new CoreInventoryParameters(this)
				{
					Comm = nextComm,
					Game = game,
					Roms = xmlGame.Assets
						.Where(kvp => true)
						.Select(kvp => (IRomAsset)new RomAsset
						{
							RomData = kvp.Value,
							FileData = kvp.Value, // TODO: Hope no one needed anything special here
							Extension = Path.GetExtension(kvp.Key),
							Game = Database.GetGameInfo(kvp.Value, Path.GetFileName(kvp.Key))
						})
						.ToList(),
				};
				nextEmulator = MakeCoreFromCoreInventory(cip, forcedCoreName);
				return true;
			}
			catch (Exception ex)
			{
				DoLoadErrorCallback(ex.ToString(), VSystemID.Raw.GBL, LoadErrorType.Xml);
				return false;
			}
		}

		public bool LoadRom(string path, CoreComm nextComm, string forcedCoreName = null, int recursiveCount = 0)
		{
			if (path == null) return false;

			if (recursiveCount > 1) // hack to stop recursive calls from endlessly rerunning if we can't load it
			{
				DoLoadErrorCallback("Failed multiple attempts to load ROM.", "");
				return false;
			}

			bool allowArchives = true;
			using var file = new HawkFile(path, false, allowArchives);
			if (!file.Exists) return false; // if the provided file doesn't even exist, give up!

			CanonicalFullPath = file.CanonicalFullPath;

			IEmulator nextEmulator;
			RomGame rom = null;
			GameInfo game = null;

			try
			{
				var cancel = false;

				// do the archive binding we had to skip
				if (!HandleArchiveBinding(file))
				{
					return false;
				}

				// not libretro: do extension checking
				var ext = file.Extension;
				switch (ext)
				{
					case ".xml":
						if (!LoadXML(path, nextComm, file, forcedCoreName, out nextEmulator, out rom, out game))
							return false;
						break;
					default:
						LoadOther(nextComm, file, forcedCoreName, out nextEmulator, out rom, out game, out cancel); // must be called after LoadXML because of SNES hacks
						break;
				}

				if (nextEmulator == null)
				{
					if (!cancel)
					{
						DoLoadErrorCallback("No core could load the rom.", null);
					}

					return false;
				}
			}
			catch (Exception ex)
			{
				var system = game?.System;

				DispatchErrorMessage(ex, system: system, path: path);
				return false;
			}

			Rom = rom;
			LoadedEmulator = nextEmulator;
			Game = game;
			return true;
		}

		private void DispatchErrorMessage(Exception ex, string system, string path)
		{
			if (ex is AggregateException agg)
			{
				// all cores failed to load a game, so tell the user everything that went wrong and maybe they can fix it
				if (agg.InnerExceptions.Count > 1)
				{
					DoLoadErrorCallback("Multiple cores failed to load the rom:", system);
				}
				foreach (Exception e in agg.InnerExceptions)
				{
					DispatchErrorMessage(e, system: system, path: path);
				}

				return;
			}

			// all of the specific exceptions we're trying to catch here aren't expected to have inner exceptions,
			// so drill down in case we got a TargetInvocationException or something like that
			while (ex.InnerException != null)
				ex = ex.InnerException;

			if (ex is MissingFirmwareException)
			{
				DoLoadErrorCallback(ex.Message, system, path, Deterministic, LoadErrorType.MissingFirmware);
			}
			else if (ex is CGBNotSupportedException)
			{
				// failed to load SGB bios or game does not support SGB mode.
				DoLoadErrorCallback("Failed to load a GB rom in SGB mode.  You might try disabling SGB Mode.", system);
			}
			else if (ex is NoAvailableCoreException)
			{
				// handle exceptions thrown by the new detected systems that BizHawk does not have cores for
				DoLoadErrorCallback($"{ex.Message}\n\n{ex}", system);
			}
			else
			{
				DoLoadErrorCallback($"A core accepted the rom, but threw an exception while loading it:\n\n{ex}", system);
			}
		}

		/// <remarks>roms ONLY; when an archive is loaded with a single file whose extension is one of these, the user prompt is skipped</remarks>
		private static class RomFileExtensions
		{
			public static readonly IReadOnlyCollection<string> GB = new[] { "gb", "gbc", "sgb" };

			public static readonly IReadOnlyCollection<string> GBA = new[] { "gba" };

			public static readonly IReadOnlyCollection<string> NES = new[] { "nes", "fds", "unf" };

			public static readonly IReadOnlyCollection<string> AutoloadFromArchive = Array.Empty<string>()
				.Concat(GB)
				.Concat(GBA)
				.Concat(NES)
				.Select(static s => $".{s}") // this is what's expected at call-site
				.ToArray();
		}

		/// <remarks>TODO add and handle <see cref="FilesystemFilter.LuaScripts"/> (you can drag-and-drop scripts and there are already non-rom things in this list, so why not?)</remarks>
		private static readonly FilesystemFilterSet RomFSFilterSet = new FilesystemFilterSet(
			new FilesystemFilter("Music Files", Array.Empty<string>(), devBuildExtraExts: new[] { "psf", "minipsf", "sid", "nsf" }),
			new FilesystemFilter("Disc Images", new[] { "cue", "ccd", "mds", "m3u" }),
			new FilesystemFilter("NES", RomFileExtensions.NES.Concat(new[] { "nsf" }).ToList(), addArchiveExts: true),
			new FilesystemFilter("Gameboy", RomFileExtensions.GB, addArchiveExts: true),
			new FilesystemFilter("Gameboy Advance", RomFileExtensions.GBA, addArchiveExts: true),
			FilesystemFilter.Archives,
			FilesystemFilter.EmuHawkSaveStates
		);

		public static readonly IReadOnlyCollection<string> KnownRomExtensions = RomFSFilterSet.Filters
			.SelectMany(f => f.Extensions)
			.Distinct()
			.Except(FilesystemFilter.ArchiveExtensions.Concat(new[] { "State" }))
			.Select(s => $".{s.ToUpperInvariant()}") // this is what's expected at call-site
			.ToList();

		public static readonly string RomFilter = RomFSFilterSet.ToString("Everything");
	}
}
