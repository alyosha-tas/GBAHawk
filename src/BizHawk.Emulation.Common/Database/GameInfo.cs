#nullable disable

using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Globalization;

namespace BizHawk.Emulation.Common
{
	public interface IGameInfo
	{
		string Name { get; }
		string System { get; }
		/// <value> SHA1, hex-encoded, unprefixed</value>
		string Hash { get; }
		bool NotInDatabase { get; }
		string FirmwareHash { get; }
	}

	public class GameInfo : IGameInfo
	{
		public string Name { get; set; }
		public string System { get; set; }
		public string Hash { get; set; }
		public bool NotInDatabase { get; set; } = true;
		public string FirmwareHash { get; set; }

		private Dictionary<string, string> Options { get; set; } = new Dictionary<string, string>();

		public GameInfo()
		{
		}

		public GameInfo Clone()
		{
			var ret = (GameInfo)MemberwiseClone();
			ret.Options = new Dictionary<string, string>(Options);
			return ret;
		}

		public static GameInfo NullInstance => new GameInfo
		{
			Name = "Null",
			System = VSystemID.Raw.NULL,
			Hash = "",
			NotInDatabase = false
		};

		internal GameInfo(CompactGameInfo cgi)
		{
			Name = cgi.Name;
			System = cgi.System;
			Hash = cgi.Hash;
			NotInDatabase = false;
			ParseOptionsDictionary(cgi.Options);
		}

		public bool this[string option] => Options.ContainsKey(option);

		public bool OptionPresent(string option)
		{
			return Options.ContainsKey(option);
		}

		public string OptionValue(string option)
		{
			return Options.TryGetValue(option, out var s) ? s : null;
		}

		public int GetIntValue(string option)
		{
			return int.Parse(Options[option]);
		}

		public string GetStringValue(string option)
		{
			return Options[option];
		}

		public int GetHexValue(string option)
		{
			return int.Parse(Options[option], NumberStyles.HexNumber);
		}

		/// <summary>
		/// /// Gets a boolean value from the database
		/// </summary>
		/// <param name="parameter">The option to look up</param>
		/// <param name="defaultVal">The value to return if the option is invalid or doesn't exist</param>
		/// <returns> The boolean value from the database if present, otherwise the given default value</returns>
		public bool GetBool(string parameter, bool defaultVal)
		{
			if (OptionValue(parameter) == "true")
			{
				return true;
			}

			if (OptionValue(parameter) == "false")
			{
				return false;
			}
			
			return defaultVal;
		}

		/// <summary>
		/// /// Gets an integer value from the database
		/// </summary>
		/// <param name="parameter">The option to look up</param>
		/// <param name="defaultVal">The value to return if the option is invalid or doesn't exist</param>
		/// <returns> The integer value from the database if present, otherwise the given default value</returns>
		public int GetInt(string parameter, int defaultVal)
		{
			if (Options.ContainsKey(parameter))
			{
				try
				{
					return int.Parse(OptionValue(parameter));
				}
				catch
				{
					return defaultVal;
				}
			}

			return defaultVal;
		}

		public IReadOnlyDictionary<string, string> GetOptions()
		{
			return new ReadOnlyDictionary<string, string>(Options);
		}

		private void ParseOptionsDictionary(string options)
		{
			if (string.IsNullOrEmpty(options))
			{
				return;
			}

			var opts = options.Split(';').Where(opt => string.IsNullOrEmpty(opt) == false).ToArray();

			foreach (var opt in opts)
			{
				var parts = opt.Split('=');
				var key = parts[0];
				var value = parts.Length > 1 ? parts[1] : "";
				Options[key] = value;
			}
		}
	}

	public static class GameInfoExtensions
	{
		public static bool IsNullInstance(this IGameInfo game)
		{
			return game == null || game.System == VSystemID.Raw.NULL;
		}
	}
}
