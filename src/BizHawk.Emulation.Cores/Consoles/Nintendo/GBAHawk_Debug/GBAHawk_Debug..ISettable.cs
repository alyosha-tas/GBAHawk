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

			[DisplayName("Flash Write Offset")]
			[Description("Set offset in Flash write timing (-512 to 511)")]
			[DefaultValue(0)]
			public short FlashWriteOffset
			{
				get => _Flash_Write_Offset;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset = -512;
					}
					else
					{
						_Flash_Write_Offset = value;
					}
				}
			}

			[DisplayName("Flash Sector Erase Offset")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset
			{
				get => _Flash_Sector_Erase_Offset;
				set =>_Flash_Sector_Erase_Offset = value;
			}

			[DisplayName("Flash Chip Erase Offset")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset
			{
				get => _Flash_Chip_Erase_Offset;
				set => _Flash_Chip_Erase_Offset = value;
			}

			[DisplayName("Use Existing SaveRAM")]
			[Description("When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(true)]
			public bool Use_SRAM { get; set; }

			[DisplayName("Use Gamerboy Player")]
			[Description("Gameboy Player will be used and detected by supported games.")]
			[DefaultValue(false)]
			public bool Use_GBP { get; set; }

			[JsonIgnore]
			private DateTime _RTCInitialTime;
			[JsonIgnore]
			private short _RTCOffset;
			[JsonIgnore]
			private short _EEPROM_Offset;
			[JsonIgnore]
			private short _Flash_Write_Offset;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset;

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
