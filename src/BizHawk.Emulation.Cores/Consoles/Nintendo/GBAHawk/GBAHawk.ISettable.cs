using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk
{
	public partial class GBAHawk : IEmulator, ISettable<GBAHawk.GBAHawkSettings, GBAHawk.GBAHawkSyncSettings>
	{
		public GBAHawkSettings GetSettings()
		{
			return _settings.Clone();
		}

		public GBAHawkSyncSettings GetSyncSettings()
		{
			return _syncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBAHawkSettings o)
		{
			_settings = o;
			return PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBAHawkSyncSettings o)
		{
			bool ret = GBAHawkSyncSettings.NeedsReboot(_syncSettings, o);
			_syncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public GBAHawkSettings _settings = new GBAHawkSettings();
		public GBAHawkSyncSettings _syncSettings = new GBAHawkSyncSettings();

		public class GBAHawkSettings
		{
			[DisplayName("Read Domains on VBlank")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool VBL_sync { get; set; }

			public GBAHawkSettings Clone()
			{
				return (GBAHawkSettings)MemberwiseClone();
			}

			public GBAHawkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}
		}

		public class GBAHawkSyncSettings
		{
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

			[DisplayName("Use Existing SaveRAM")]
			[Description("(Intended for development, for regular use leave as true.) When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(true)]
			public bool Use_SRAM { get; set; }

			[JsonIgnore]
			private int _RTCInitialTime;
			[JsonIgnore]
			private int _RTCOffset;
			[JsonIgnore]
			public ushort _DivInitialTime = 8;

			public GBAHawkSyncSettings Clone()
			{
				return (GBAHawkSyncSettings)MemberwiseClone();
			}

			public GBAHawkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool NeedsReboot(GBAHawkSyncSettings x, GBAHawkSyncSettings y)
			{
				return !DeepEquality.DeepEquals(x, y);
			}
		}
	}
}
