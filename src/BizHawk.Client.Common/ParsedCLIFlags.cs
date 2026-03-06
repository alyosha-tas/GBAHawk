#nullable enable

using System.Collections.Generic;

namespace BizHawk.Client.Common
{
	public readonly struct ParsedCLIFlags
	{
		public readonly string? cmdLoadSlot;

		public readonly string? cmdLoadState;

		public readonly string? cmdConfigFile;

		public readonly string? cmdMovie;

		public readonly string? cmdDumpType;

		public readonly HashSet<int>? _currAviWriterFrameList;

		public readonly int _autoDumpLength;

		public readonly bool printVersion;

		public readonly string? cmdDumpName;

		public readonly bool _autoCloseOnDump;

		public readonly string? luaScript;

		public readonly bool luaConsole;

		public readonly string? MMFFilename;

		public readonly bool? audiosync;

		public readonly string? cmdRom;

		public ParsedCLIFlags(string? cmdLoadSlot,
			string? cmdLoadState,
			string? cmdConfigFile,
			string? cmdMovie,
			string? cmdDumpType,
			HashSet<int>? currAviWriterFrameList,
			int autoDumpLength,
			bool printVersion,
			string? cmdDumpName,
			bool autoCloseOnDump,
			bool chromeless,
			string? luaScript,
			bool luaConsole,
			string? mmfFilename,
			bool? audiosync,
			string? cmdRom)
		{
			this.cmdLoadSlot = cmdLoadSlot;
			this.cmdLoadState = cmdLoadState;
			this.cmdConfigFile = cmdConfigFile;
			this.cmdMovie = cmdMovie;
			this.cmdDumpType = cmdDumpType;
			_currAviWriterFrameList = currAviWriterFrameList;
			_autoDumpLength = autoDumpLength;
			this.printVersion = printVersion;
			this.cmdDumpName = cmdDumpName;
			_autoCloseOnDump = autoCloseOnDump;
			this.luaScript = luaScript;
			this.luaConsole = luaConsole;
			MMFFilename = mmfFilename;
			this.audiosync = audiosync;
			this.cmdRom = cmdRom;
		}
	}
}
