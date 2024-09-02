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

			[DisplayName("Flash Write Offset")]
			[Description("Set offset in Flash write timing (-128 to 127)")]
			[DefaultValue(0)]
			public short FlashWriteOffset
			{
				get => _Flash_Write_Offset;
				set
				{
					if (value > 127)
					{
						_Flash_Write_Offset = 127;
					}
					else if (value < -128)
					{
						_Flash_Write_Offset = -128;
					}
					else
					{
						_Flash_Write_Offset = value;
					}
				}
			}

			[DisplayName("Flash Sector Erase Offset")]
			[Description("Set offset in Flash Sector Erase timing (-32768 to 32767)")]
			[DefaultValue(0)]
			public short FlashSectorEraseOffset
			{
				get => _Flash_Sector_Erase_Offset;
				set => _Flash_Sector_Erase_Offset = value;
			}

			[DisplayName("Flash Chip Erase Offset")]
			[Description("Set offset in Flash Chip Erase timing (-32768 to 32767)")]
			[DefaultValue(0)]
			public short FlashChipEraseOffset
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
			private short _Flash_Sector_Erase_Offset;
			[JsonIgnore]
			private short _Flash_Chip_Erase_Offset;

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
