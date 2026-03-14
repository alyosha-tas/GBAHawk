using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawk
{
	public partial class GBHawk : IEmulator, ISettable<GBHawk.GBHawkSettings, GBHawk.GBHawkSyncSettings>
	{
		public GBHawkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public GBHawkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBHawkSettings o)
		{
			Settings = o;
			return PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBHawkSyncSettings o)
		{
			bool ret = GBHawkSyncSettings.NeedsReboot(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public GBHawkSettings Settings = new GBHawkSettings();
		public GBHawkSyncSettings SyncSettings = new GBHawkSyncSettings();

		public class GBHawkSettings
		{
			public enum PaletteType
			{
				BW,
				Gr
			}

			public enum Cycle_Return
			{
				CPU,
				GBI
			}

			[DisplayName("Color Mode")]
			[Description("Pick Between Green scale and Grey scale colors")]
			[DefaultValue(PaletteType.BW)]
			public PaletteType Palette { get; set; }

			[DisplayName("Read Domains on VBlank")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool VBL_sync { get; set; }

			[DisplayName("TotalExecutedCycles Return Value")]
			[Description("CPU returns the actual CPU cycles executed, GBI returns the values needed for console verification")]
			[DefaultValue(Cycle_Return.CPU)]
			public Cycle_Return cycle_return_setting { get; set; }

			public GBHawkSettings Clone()
			{
				return (GBHawkSettings)MemberwiseClone();
			}

			public GBHawkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}
		}

		public class GBHawkSyncSettings
		{
			public enum ConsoleModeType
			{
				GB,
				GBC
			}

			[DisplayName("Console Mode")]
			[Description("Deprecated, only left for compatibility")]
			[DefaultValue(ConsoleModeType.GBC)]
			public ConsoleModeType ConsoleMode { get; set; }

			[DisplayName("CGB in GBA")]
			[Description("Emulate GBA hardware running a CGB game, instead of CGB hardware.")]
			[DefaultValue(false)]
			public bool GBACGB { get; set; }

			[DisplayName("RTC Initial Time")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int RTCInitialTime
			{
				get => _RTCInitialTime;
				set => _RTCInitialTime = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Offset")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int RTCOffset
			{
				get => _RTCOffset;
				set => _RTCOffset = Math.Max(-127, Math.Min(127, value));
			}

			[JsonIgnore]
			private int _RTCInitialTime;
			[JsonIgnore]
			private int _RTCOffset;
			[JsonIgnore]
			public ushort _DivInitialTime = 8;

			public GBHawkSyncSettings Clone()
			{
				return (GBHawkSyncSettings)MemberwiseClone();
			}

			public GBHawkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool NeedsReboot(GBHawkSyncSettings x, GBHawkSyncSettings y)
			{
				return !DeepEquality.DeepEquals(x, y);
			}
		}
	}
}
