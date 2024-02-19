using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBAHawk : ISettable<GBAHawk.GBASettings, GBAHawk.GBASyncSettings>
	{
		public GBASettings GetSettings()
		{
			return Settings.Clone();
		}

		public GBASyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBASettings o)
		{
			bool ret = GBASettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBASyncSettings o)
		{
			bool ret = GBASyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal GBASettings Settings { get; private set; }
		internal GBASyncSettings SyncSettings { get; private set; }

		public class GBASettings
		{
			public GBASettings Clone()
			{
				return (GBASettings)MemberwiseClone();
			}

			public GBASettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(GBASettings x, GBASettings y)
			{
				return false;
			}
		}

		public class GBASyncSettings
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

			public GBASyncSettings Clone()
			{
				return (GBASyncSettings)MemberwiseClone();
			}

			public GBASyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(GBASyncSettings x, GBASyncSettings y)
			{
				return true;
			}
		}
	}
}
