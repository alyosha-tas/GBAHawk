﻿using System;
using System.ComponentModel;

using NLua;

// ReSharper disable UnusedMember.Global
// ReSharper disable UnusedAutoPropertyAccessor.Local
namespace BizHawk.Client.Common
{
	[Description("A library for interacting with the currently loaded emulator core")]
	public sealed class EmulationLuaLibrary : LuaLibraryBase
	{
		public Action FrameAdvanceCallback { get; set; }
		public Action YieldCallback { get; set; }

		public EmulationLuaLibrary(IPlatformLuaLibEnv luaLibsImpl, ApiContainer apiContainer, Action<string> logOutputCallback)
			: base(luaLibsImpl, apiContainer, logOutputCallback) {}

		public override string Name => "emu";

		[LuaMethodExample("emu.displayvsync( true );")]
		[LuaMethod("displayvsync", "Sets the display vsync property of the emulator")]
		public void DisplayVsync(bool enabled)
			=> APIs.Emulation.DisplayVsync(enabled);

		[LuaMethodExample("emu.frameadvance( );")]
		[LuaMethod("frameadvance", "Signals to the emulator to resume emulation. Necessary for any lua script while loop or else the emulator will freeze!")]
		public void FrameAdvance()
		{
			FrameAdvanceCallback();
		}

		[LuaMethodExample("local inemufra = emu.framecount( );")]
		[LuaMethod("framecount", "Returns the current frame count")]
		public int FrameCount()
			=> APIs.Emulation.FrameCount();

		[LuaMethodExample("local obemudis = emu.disassemble( 0x8000 );")]
		[LuaMethod("disassemble", "Returns the disassembly object (disasm string and length int) for the given PC address. Uses System Bus domain if no domain name provided")]
		public object Disassemble(uint pc, string name = "")
			=> APIs.Emulation.Disassemble(pc, name);

		[LuaMethodExample("local inemutot = emu.totalexecutedcycles( );")]
		[LuaMethod("totalexecutedcycles", "gets the total number of executed cpu cycles")]
		public long TotalExecutedycles()
			=> APIs.Emulation.TotalExecutedCycles();

		[LuaMethodExample("local stemuget = emu.getsystemid( );")]
		[LuaMethod("getsystemid", "Returns the ID string of the current core loaded. Note: No ROM loaded will return the string NULL")]
		public string GetSystemId()
			=> APIs.Emulation.GetSystemId();

		[LuaMethodExample("if ( emu.islagged( ) ) then\r\n\tconsole.log( \"Returns whether or not the current frame is a lag frame\" );\r\nend;")]
		[LuaMethod("islagged", "Returns whether or not the current frame is a lag frame")]
		public bool IsLagged()
			=> APIs.Emulation.IsLagged();

		[LuaMethodExample("emu.setislagged( true );")]
		[LuaMethod("setislagged", "Sets the lag flag for the current frame. If no value is provided, it will default to true")]
		public void SetIsLagged(bool value = true)
			=> APIs.Emulation.SetIsLagged(value);

		[LuaMethodExample("local inemulag = emu.lagcount( );")]
		[LuaMethod("lagcount", "Returns the current lag count")]
		public int LagCount()
			=> APIs.Emulation.LagCount();

		[LuaMethodExample("emu.setlagcount( 50 );")]
		[LuaMethod("setlagcount", "Sets the current lag count")]
		public void SetLagCount(int count)
			=> APIs.Emulation.SetLagCount(count);

		[LuaMethodExample("emu.limitframerate( true );")]
		[LuaMethod("limitframerate", "sets the limit framerate property of the emulator")]
		public void LimitFramerate(bool enabled)
			=> APIs.Emulation.LimitFramerate(enabled);

		[LuaMethodExample("emu.minimizeframeskip( true );")]
		[LuaMethod("minimizeframeskip", "Sets the autominimizeframeskip value of the emulator")]
		public void MinimizeFrameskip(bool enabled)
			=> APIs.Emulation.MinimizeFrameskip(enabled);

		[LuaMethodExample("emu.yield( );")]
		[LuaMethod("yield", "allows a script to run while emulation is paused and interact with the gui/main window in realtime ")]
		public void Yield()
		{
			YieldCallback();
		}

		[LuaMethodExample("local stemuget = emu.getdisplaytype();")]
		[LuaMethod("getdisplaytype", "returns the display type (PAL vs NTSC) that the emulator is currently running in")]
		public string GetDisplayType()
			=> APIs.Emulation.GetDisplayType();

		[LuaDeprecatedMethod]
		[LuaMethod("getluacore", "returns the name of the Lua core currently in use")]
		public string GetLuaBackend()
		{
			Log("Deprecated function emu.getluacore() used, replace the call with client.get_lua_engine().");
			return _luaLibsImpl.EngineName;
		}
	}
}
