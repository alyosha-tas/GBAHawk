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
