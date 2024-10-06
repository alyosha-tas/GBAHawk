#nullable enable

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.Common
{
	[Description("A library for interacting with the currently loaded emulator core")]
	public sealed class EmulationApi : IEmulationApi
	{
		[RequiredService]
		private IEmulator? Emulator { get; set; }

		[OptionalService]
		private IDisassemblable? DisassemblableCore { get; set; }

		[OptionalService]
		private IInputPollable? InputPollableCore { get; set; }

		[OptionalService]
		private IMemoryDomains? MemoryDomains { get; set; }

		[OptionalService]
		private IRegionable? RegionableCore { get; set; }

		private readonly Config _config;

		private readonly IGameInfo? _game;

		private readonly Action<string> LogCallback;

		/// <summary>Using this property to get a reference to the global <see cref="Config"/> instance is a terrible, horrible, no good, very bad idea. That's why it's not in the <see cref="IEmulationApi">interface</see>.</summary>
		public Config ForbiddenConfigReference
		{
			get
			{
				ForbiddenConfigReferenceUsed = true;
				return _config;
			}
		}

		public bool ForbiddenConfigReferenceUsed { get; private set; }

		public EmulationApi(Action<string> logCallback, Config config, IGameInfo? game)
		{
			_config = config;
			_game = game;
			LogCallback = logCallback;
		}

		public void DisplayVsync(bool enabled) => _config.VSync = enabled;

		public int FrameCount()
			=> Emulator!.Frame;

		public object? Disassemble(uint pc, string? name = null)
		{
			try
			{
				if (DisassemblableCore != null)
				{
					return new {
						disasm = DisassemblableCore.Disassemble(
							string.IsNullOrEmpty(name) ? MemoryDomains!.SystemBus : MemoryDomains![name!]!,
							pc,
							out var l
						),
						length = l
					};
				}
			}
			catch (NotImplementedException) {}
			LogCallback($"Error: {Emulator.Attributes().CoreName} does not yet implement {nameof(IDisassemblable.Disassemble)}()");
			return null;
		}

		public long TotalExecutedCycles()
		{
			if (Emulator != null)
			{
				return (long)Emulator.TotalExecutedCycles;
			}

			return default;		
		}

		public string GetSystemId()
			=> _game!.System;

		public bool IsLagged()
		{
			if (InputPollableCore != null) return InputPollableCore.IsLagFrame;
			LogCallback($"Can not get lag information, {Emulator.Attributes().CoreName} does not implement {nameof(IInputPollable)}");
			return false;
		}

		public void SetIsLagged(bool value = true)
		{
			if (InputPollableCore != null) InputPollableCore.IsLagFrame = value;
			else LogCallback($"Can not set lag information, {Emulator.Attributes().CoreName} does not implement {nameof(IInputPollable)}");
		}

		public int LagCount()
		{
			if (InputPollableCore != null) return InputPollableCore.LagCount;
			LogCallback($"Can not get lag information, {Emulator.Attributes().CoreName} does not implement {nameof(IInputPollable)}");
			return default;
		}

		public void SetLagCount(int count)
		{
			if (InputPollableCore != null) InputPollableCore.LagCount = count;
			else LogCallback($"Can not set lag information, {Emulator.Attributes().CoreName} does not implement {nameof(IInputPollable)}");
		}

		public void LimitFramerate(bool enabled) => _config.ClockThrottle = enabled;

		public void MinimizeFrameskip(bool enabled) => _config.AutoMinimizeSkipping = enabled;

		public string GetDisplayType() => (RegionableCore?.Region)?.ToString() ?? "";

		public IGameInfo? GetGameInfo()
			=> _game;

		public IReadOnlyDictionary<string, string?> GetGameOptions()
			=> _game == null
				? new Dictionary<string, string?>()
				: ((GameInfo) _game).GetOptions().ToDictionary(static kvp => kvp.Key, static kvp => (string?) kvp.Value);

		public object? GetSettings() => Emulator switch
		{
			_ => null
		};

		public PutSettingsDirtyBits PutSettings(object settings) => Emulator switch
		{
			_ => PutSettingsDirtyBits.None
		};
	}
}
