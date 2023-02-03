using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug : IEmulator, ISettable<GBAHawk_Debug.GBAHawk_Debug_Settings, GBAHawk_Debug.GBAHawk_Debug_SyncSettings>
	{
		public GBAHawk_Debug_Settings GetSettings()
		{
			return _settings.Clone();
		}

		public GBAHawk_Debug_SyncSettings GetSyncSettings()
		{
			return _syncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBAHawk_Debug_Settings o)
		{
			_settings = o;
			return PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBAHawk_Debug_SyncSettings o)
		{
			bool ret = GBAHawk_Debug_SyncSettings.NeedsReboot(_syncSettings, o);
			_syncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public GBAHawk_Debug_Settings _settings = new GBAHawk_Debug_Settings();
		public GBAHawk_Debug_SyncSettings _syncSettings = new GBAHawk_Debug_SyncSettings();

		public class GBAHawk_Debug_Settings
		{
			[DisplayName("Read Domains on VBlank")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool VBL_sync { get; set; }

			public GBAHawk_Debug_Settings Clone()
			{
				return (GBAHawk_Debug_Settings)MemberwiseClone();
			}

			public GBAHawk_Debug_Settings()
			{
				SettingsUtil.SetDefaultValues(this);
			}
		}

		public class GBAHawk_Debug_SyncSettings
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
			[DefaultValue(false)]
			public bool Use_SRAM { get; set; }

			[JsonIgnore]
			private int _RTCInitialTime;
			[JsonIgnore]
			private int _RTCOffset;
			[JsonIgnore]
			public ushort _DivInitialTime = 8;

			public GBAHawk_Debug_SyncSettings Clone()
			{
				return (GBAHawk_Debug_SyncSettings)MemberwiseClone();
			}

			public GBAHawk_Debug_SyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool NeedsReboot(GBAHawk_Debug_SyncSettings x, GBAHawk_Debug_SyncSettings y)
			{
				return !DeepEquality.DeepEquals(x, y);
			}
		}
	}
}
