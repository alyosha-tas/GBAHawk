#nullable disable

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using BizHawk.Common;

namespace BizHawk.Emulation.Common
{
	public static class Database
	{
		private static readonly Dictionary<string, CompactGameInfo> DB = new Dictionary<string, CompactGameInfo>();

		private static string _bundledRoot = null;

		private static IList<string> _expected = null;

		private static string _userRoot = null;

		public static string RemoveHashType(string hash)
		{
			hash = hash.ToUpper();

			if (hash.StartsWith("SHA1:"))
			{
				hash = hash.Substring(5);
			}

			return hash;
		}

		private static void LoadDatabase_Escape(string line, bool inUser, bool silent)
		{
			if (!line.StartsWith("#include", StringComparison.InvariantCultureIgnoreCase)) return;

			var isUserInclude = line.StartsWith("#includeuser", StringComparison.InvariantCultureIgnoreCase);
			var searchUser = inUser || isUserInclude;
			line = line.Substring(isUserInclude ? 12 : 8).TrimStart();
			var filename = Path.Combine(searchUser ? _userRoot : _bundledRoot, line);
			if (File.Exists(filename))
			{
				if (!silent) Util.DebugWriteLine($"loading external game database {line} ({(searchUser ? "user" : "bundled")})");
				initializeWork(filename, inUser: searchUser, silent: silent);
			}
			else if (inUser)
			{
				Util.DebugWriteLine($"BENIGN: missing external game database {line} (user)");
			}
			else if (!isUserInclude)
			{
				throw new FileNotFoundException($"missing external game database {line} (bundled)");
			}
		}

		private static bool initialized = false;

		private static void initializeWork(string path, bool inUser, bool silent)
		{
			if (!inUser) _expected.Remove(Path.GetFileName(path));

			using var reader = new StreamReader(new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read));
			while (reader.EndOfStream == false)
			{
				var line = reader.ReadLine() ?? "";
				try
				{
					if (line.StartsWith(";"))
					{
						continue; // comment
					}

					if (line.StartsWith("#"))
					{
						LoadDatabase_Escape(line, inUser: inUser, silent: silent);
						continue;
					}

					if (line.Trim().Length == 0)
					{
						continue;
					}

					var items = line.Split('\t');

					var game = new CompactGameInfo
					{
						Hash = RemoveHashType(items[0].ToUpper()),
						Name = items[1],
						System = items[2],
						// blank space here
						Options = items[4]
					};
					if (game.Hash is SHA1Checksum.EmptyFile or MD5Checksum.EmptyFile)
					{
						Console.WriteLine($"WARNING: gamedb {path} contains entry for empty rom as \"{game.Name}\"!");
					}
					if (!silent && DB.TryGetValue(game.Hash, out var dupe))
					{
						Console.WriteLine("gamedb: Multiple hash entries {0}, duplicate detected on \"{1}\" and \"{2}\"", game.Hash, game.Name, dupe.Name);
					}

					DB[game.Hash] = game;
				}
				catch
				{
					Util.DebugWriteLine($"Error parsing database entry: {line}");
				}
			}
		}

		public static void InitializeDatabase(string path, bool silent)
		{
			if (initialized) throw new InvalidOperationException("Did not expect re-initialize of game Database");
			initialized = true;

			_bundledRoot = Path.GetDirectoryName(path);
			_userRoot = Environment.GetEnvironmentVariable("BIZHAWK_DATA_HOME");
			if (!string.IsNullOrEmpty(_userRoot) && Directory.Exists(_userRoot)) _userRoot = Path.Combine(_userRoot, "gamedb");
			else _userRoot = _bundledRoot;
			Console.WriteLine($"user root: {_userRoot}");

			_expected = new DirectoryInfo(_bundledRoot!).EnumerateFiles("*.txt").Select(static fi => fi.Name).ToList();

			initializeWork(path, inUser: false, silent: silent);
		}

		public static GameInfo CheckDatabase(string hash)
		{
			var hashNoType = RemoveHashType(hash);
			DB.TryGetValue(hashNoType, out var cgi);
			if (cgi == null)
			{
				Console.WriteLine($"DB: hash {hash} not in game database.");
				return null;
			}

			return new GameInfo(cgi);
		}

		public static GameInfo GetGameInfo(byte[] romData, string fileName)
		{
			var hashSHA1 = SHA1Checksum.ComputeDigestHex(romData);
			if (DB.TryGetValue(hashSHA1, out var cgi))
			{
				return new GameInfo(cgi);
			}

			// rom is not in database. make some best-guesses
			var game = new GameInfo
			{
				Hash = hashSHA1,
				NotInDatabase = true
			};

			Console.WriteLine($"Game was not in DB. SHA1: {hashSHA1}");

			var ext = Path.GetExtension(fileName)?.ToUpperInvariant();

			switch (ext)
			{
				case ".GB":
					game.System = VSystemID.Raw.GB;
					break;
				case ".GBC":
					game.System = VSystemID.Raw.GBC;
					break;
				case ".GBA":
					game.System = VSystemID.Raw.GBA;
					break;
				case ".NES":
					game.System = VSystemID.Raw.NES;
					break;
				case ".SFC":
				case ".SMC":
					game.System = VSystemID.Raw.SNES;
					break;
				case ".DEBUG":
					game.System = VSystemID.Raw.DEBUG;
					break;
			}

			game.Name = Path.GetFileNameWithoutExtension(fileName)?.Replace('_', ' ');

			return game;
		}
	}

	public class CompactGameInfo
	{
		public string Name { get; set; }
		public string System { get; set; }
		public string Options { get; set; }
		public string Hash { get; set; }
	}
}
