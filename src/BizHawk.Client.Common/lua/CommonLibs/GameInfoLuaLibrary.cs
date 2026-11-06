using System;

using BizHawk.Emulation.Common;

using NLua;

// ReSharper disable UnusedMember.Global
// ReSharper disable UnusedAutoPropertyAccessor.Local
namespace BizHawk.Client.Common
{
	public sealed class GameInfoLuaLibrary : LuaLibraryBase
	{
		public GameInfoLuaLibrary(IPlatformLuaLibEnv luaLibsImpl, ApiContainer apiContainer, Action<string> logOutputCallback)
			: base(luaLibsImpl, apiContainer, logOutputCallback) {}

		public override string Name => "gameinfo";

		[LuaMethodExample("local stgamget = gameinfo.getromname( );")]
		[LuaMethod("getromname", "returns the name of the currently loaded rom, if a rom is loaded")]
		public string GetRomName()
			=> APIs.Emulation.GetGameInfo()?.Name ?? string.Empty;

		[LuaMethodExample("local stgamget = gameinfo.getromhash( );")]
		[LuaMethod("getromhash", "returns the hash of the currently loaded rom, if a rom is loaded")]
		public string GetRomHash()
			=> APIs.Emulation.GetGameInfo()?.Hash ?? string.Empty;

		[LuaMethodExample("if ( gameinfo.indatabase( ) ) then\r\n\tconsole.log( \"returns whether or not the currently loaded rom is in the game database\" );\r\nend;")]
		[LuaMethod("indatabase", "returns whether or not the currently loaded rom is in the game database")]
		public bool InDatabase()
			=> APIs.Emulation.GetGameInfo()?.NotInDatabase is false;

		[LuaMethodExample("local nlgamget = gameinfo.getoptions( );")]
		[LuaMethod("getoptions", "returns the game options for the currently loaded rom. Options vary per platform")]
		public LuaTable GetOptions()
			=> _th.DictToTable(APIs.Emulation.GetGameOptions());
	}
}
