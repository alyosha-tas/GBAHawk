using System;
using System.IO;
using System.Linq;

using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Consoles.Nintendo.Gameboy;

namespace BizHawk.Client.Common
{
	public static class MovieConversionExtensions
	{
		public static ITasMovie ToTasMovie(this IMovie old)
		{
			string newFilename = ConvertFileNameToTasMovie(old.Filename);
			var tas = (ITasMovie)old.Session.Get(newFilename);
			tas.CopyLog(old.GetLogEntries());

			old.Truncate(0); // Trying to minimize ram usage

			tas.HeaderEntries.Clear();
			foreach (var (k, v) in old.HeaderEntries) tas.HeaderEntries[k] = v;

			// TODO: we have this version number string generated in multiple places
			tas.HeaderEntries[HeaderKeys.MovieVersion] = $"GBAHawk v1.0 Tasproj v{TasMovie.CurrentVersion}";

			tas.SyncSettingsJson = old.SyncSettingsJson;

			tas.Comments.Clear();
			foreach (var comment in old.Comments)
			{
				tas.Comments.Add(comment);
			}

			tas.Subtitles.Clear();
			foreach (var sub in old.Subtitles)
			{
				tas.Subtitles.Add(sub);
			}

			tas.StartsFromSavestate = old.StartsFromSavestate;
			tas.BinarySavestate = old.BinarySavestate;
			tas.SaveRam = old.SaveRam;

			return tas;
		}

		public static IMovie Togbmv(this IMovie old)
		{
			var gbmv = old.Session.Get(old.Filename.Replace(old.PreferredExtension, gbmvMovie.Extension));
			gbmv.CopyLog(old.GetLogEntries());

			gbmv.HeaderEntries.Clear();
			foreach (var (k, v) in old.HeaderEntries) gbmv.HeaderEntries[k] = v;

			// TODO: we have this version number string generated in multiple places
			gbmv.HeaderEntries[HeaderKeys.MovieVersion] = "GBAHawk v1.0";

			gbmv.SyncSettingsJson = old.SyncSettingsJson;

			gbmv.Comments.Clear();
			foreach (var comment in old.Comments)
			{
				gbmv.Comments.Add(comment);
			}

			gbmv.Subtitles.Clear();
			foreach (var sub in old.Subtitles)
			{
				gbmv.Subtitles.Add(sub);
			}

			gbmv.BinarySavestate = old.BinarySavestate;
			gbmv.SaveRam = old.SaveRam;

			return gbmv;
		}

		public static ITasMovie ConvertToSavestateAnchoredMovie(this ITasMovie old, int frame, byte[] savestate)
		{
			string newFilename = ConvertFileNameToTasMovie(old.Filename);

			var tas = (ITasMovie)old.Session.Get(newFilename);
			tas.BinarySavestate = savestate;
			tas.LagLog.Clear();

			var entries = old.GetLogEntries();

			tas.CopyLog(entries.Skip(frame));
			tas.CopyVerificationLog(old.VerificationLog);
			tas.CopyVerificationLog(entries.Take(frame));

			// States can't be easily moved over, because they contain the frame number.
			// TODO? I'm not sure how this would be done.
			old.TasStateManager.Clear();

			// Lag Log
			tas.LagLog.FromLagLog(old.LagLog);
			tas.LagLog.StartFromFrame(frame);

			tas.HeaderEntries.Clear();
			foreach (var (k, v) in old.HeaderEntries) tas.HeaderEntries[k] = v;

			tas.StartsFromSavestate = true;
			tas.SyncSettingsJson = old.SyncSettingsJson;

			tas.Comments.Clear();
			foreach (string comment in old.Comments)
			{
				tas.Comments.Add(comment);
			}

			tas.Subtitles.Clear();
			foreach (Subtitle sub in old.Subtitles)
			{
				tas.Subtitles.Add(sub);
			}

			foreach (TasMovieMarker marker in old.Markers)
			{
				if (marker.Frame > frame)
				{
					tas.Markers.Add(new TasMovieMarker(marker.Frame - frame, marker.Message));
				}
			}

			tas.TasStateManager.UpdateSettings(old.TasStateManager.Settings);

			tas.Save();
			return tas;
		}

		public static ITasMovie ConvertToSaveRamAnchoredMovie(this ITasMovie old, byte[] saveRam)
		{
			string newFilename = ConvertFileNameToTasMovie(old.Filename);

			var tas = (ITasMovie)old.Session.Get(newFilename);
			tas.SaveRam = saveRam;
			tas.TasStateManager.Clear();
			tas.LagLog.Clear();

			var entries = old.GetLogEntries();

			tas.CopyVerificationLog(old.VerificationLog);
			tas.CopyVerificationLog(entries);

			tas.HeaderEntries.Clear();
			foreach (var (k, v) in old.HeaderEntries) tas.HeaderEntries[k] = v;

			tas.StartsFromSaveRam = true;
			tas.SyncSettingsJson = old.SyncSettingsJson;

			tas.Comments.Clear();
			foreach (string comment in old.Comments)
			{
				tas.Comments.Add(comment);
			}

			tas.Subtitles.Clear();
			foreach (Subtitle sub in old.Subtitles)
			{
				tas.Subtitles.Add(sub);
			}

			tas.TasStateManager.UpdateSettings(old.TasStateManager.Settings);

			tas.Save();
			return tas;
		}

		// TODO: This doesn't really belong here, but not sure where to put it
		public static void PopulateWithDefaultHeaderValues(
			this IMovie movie,
			IEmulator emulator,
			IGameInfo game,
			FirmwareManager firmwareManager,
			string author)
		{
			movie.Author = author;
			movie.EmulatorVersion = VersionInfo.GetEmuVersion();
			movie.OriginalEmulatorVersion = VersionInfo.GetEmuVersion();
			movie.SystemID = emulator.SystemId;

			var settable = new SettingsAdapter(emulator);
			if (settable.HasSyncSettings)
			{
				movie.SyncSettingsJson = ConfigService.SaveWithType(settable.GetSyncSettings());
			}

			if (game.IsNullInstance())
			{
				movie.GameName = "NULL";
			}
			else
			{
				movie.GameName = game.FilesystemSafeName();
				movie.Hash = game.Hash;
				if (game.FirmwareHash != null)
				{
					movie.FirmwareHash = game.FirmwareHash;
				}
			}

			if (emulator.HasRegions())
			{
				var region = emulator.AsRegionable().Region;
				if (region == DisplayType.PAL)
				{
					movie.HeaderEntries.Add(HeaderKeys.Pal, "1");
				}
			}

			if (firmwareManager.RecentlyServed.Count != 0)
			{
				foreach (var firmware in firmwareManager.RecentlyServed)
				{
					var key = firmware.ID.MovieHeaderKey;
					if (!movie.HeaderEntries.ContainsKey(key))
					{
						movie.HeaderEntries.Add(key, firmware.Hash);
					}
				}
			}

			if (emulator is IGameboyCommon gb)
			{
				if (gb.IsCGBMode())
				{
					movie.HeaderEntries.Add(gb.IsCGBDMGMode() ? "IsCGBDMGMode" : "IsCGBMode", "1");
				}
			}

			if (emulator is ICycleTiming)
			{
				movie.HeaderEntries.Add(HeaderKeys.CycleCount, "0");
				movie.HeaderEntries.Add(HeaderKeys.ClockRate, "0");
			}

			movie.Core = ((CoreAttribute)Attribute
				.GetCustomAttribute(emulator.GetType(), typeof(CoreAttribute)))
				.CoreName;
		}

		internal static string ConvertFileNameToTasMovie(string oldFileName)
		{
			string newFileName = Path.ChangeExtension(oldFileName, $".{TasMovie.Extension}");
			int fileSuffix = 0;
			while (File.Exists(newFileName))
			{
				// Using this should hopefully be system agnostic
				var temp_path = Path.Combine(Path.GetDirectoryName(oldFileName), Path.GetFileNameWithoutExtension(oldFileName));
				newFileName = $"{temp_path} {++fileSuffix}.{TasMovie.Extension}";
			}

			return newFileName;
		}
	}
}
