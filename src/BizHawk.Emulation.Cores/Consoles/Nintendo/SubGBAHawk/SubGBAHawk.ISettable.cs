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
			public enum InitRTCState
			{
				Reset_Good_Batt,
				Reset_Bad_Batt,
				RTC_Set
			}

			[DisplayName("RTC Initial State")]
			[Description("Choose how the RTC starts up")]
			[DefaultValue(InitRTCState.Reset_Good_Batt)]
			public InitRTCState RTCInitialState { get; set; }

			[DisplayName("RTC Initial Time")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime
			{
				get => _RTCInitialTime;
				set => _RTCInitialTime = value;
			}

			[DisplayName("RTC Offset")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset
			{
				get => _RTCOffset;
				set => _RTCOffset = value;
			}

			[DisplayName("EEPROM Offset")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset
			{
				get => _EEPROM_Offset;
				set => _EEPROM_Offset = value;
			}

			[DisplayName("Use Existing SaveRAM")]
			[Description("When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(true)]
			public bool Use_SRAM { get; set; }

			[JsonIgnore]
			private DateTime _RTCInitialTime;
			[JsonIgnore]
			private short _RTCOffset;
			[JsonIgnore]
			private short _EEPROM_Offset;

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
