using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	public partial class SubGBAHawk : ISettable<SubGBAHawk.SubGBASettings, SubGBAHawk.SubGBASyncSettings>
	{
		public SubGBASettings GetSettings()
		{
			return Settings.Clone();
		}

		public SubGBASyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(SubGBASettings o)
		{
			bool ret = SubGBASettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(SubGBASyncSettings o)
		{
			bool ret = SubGBASyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal SubGBASettings Settings { get; private set; }
		internal SubGBASyncSettings SyncSettings { get; private set; }

		public class SubGBASettings
		{
			public SubGBASettings Clone()
			{
				return (SubGBASettings)MemberwiseClone();
			}

			public SubGBASettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(SubGBASettings x, SubGBASettings y)
			{
				return false;
			}
		}

		public class SubGBASyncSettings
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
			[Description("When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(false)]
			public bool Use_SRAM { get; set; }

			[JsonIgnore]
			private int _RTCInitialTime;
			[JsonIgnore]
			private int _RTCOffset;
			[JsonIgnore]
			public ushort _DivInitialTime = 8;


			public SubGBASyncSettings Clone()
			{
				return (SubGBASyncSettings)MemberwiseClone();
			}

			public SubGBASyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(SubGBASyncSettings x, SubGBASyncSettings y)
			{
				return true;
			}
		}
	}
}
