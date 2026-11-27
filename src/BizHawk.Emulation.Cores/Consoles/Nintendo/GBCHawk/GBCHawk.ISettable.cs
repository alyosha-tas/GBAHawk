using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBCHawk
{
	public partial class GBCHawk : IEmulator, ISettable<GBCHawk.GBCHawkSettings, GBCHawk.GBCHawkSyncSettings>
	{
		public GBCHawkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public GBCHawkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBCHawkSettings o)
		{
			Settings = o;
			return PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBCHawkSyncSettings o)
		{
			bool ret = GBCHawkSyncSettings.NeedsReboot(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public GBCHawkSettings Settings = new GBCHawkSettings();
		public GBCHawkSyncSettings SyncSettings = new GBCHawkSyncSettings();

		public class GBCHawkSettings
		{
			public enum Cycle_Return
			{
				CPU,
				GBI
			}

			[DisplayName("Read Domains on VBlank")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool VBL_sync { get; set; }

			[DisplayName("TotalExecutedCycles Return Value")]
			[Description("CPU returns the actual CPU cycles executed, GBI returns the values needed for console verification")]
			[DefaultValue(Cycle_Return.CPU)]
			public Cycle_Return cycle_return_setting { get; set; }

			public GBCHawkSettings Clone()
			{
				return (GBCHawkSettings)MemberwiseClone();
			}

			public GBCHawkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}
		}

		public class GBCHawkSyncSettings
		{
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

			public GBCHawkSyncSettings Clone()
			{
				return (GBCHawkSyncSettings)MemberwiseClone();
			}

			public GBCHawkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool NeedsReboot(GBCHawkSyncSettings x, GBCHawkSyncSettings y)
			{
				return !DeepEquality.DeepEquals(x, y);
			}
		}
	}
}
