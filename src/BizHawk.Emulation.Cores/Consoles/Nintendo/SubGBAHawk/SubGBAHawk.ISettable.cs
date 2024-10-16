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

			public enum FlashChipType64
			{
				Atmel,
				Panasonic,
				Macronix,
				SST
			}

			public enum FlashChipType128
			{
				Macronix,
				Sanyo
			}

			[DisplayName("Flash Chip Type (64K)")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType64.Panasonic)]
			public FlashChipType64 Flash_Type_64 { get; set; }

			[DisplayName("Flash Chip Type (128K)")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType128.Sanyo)]
			public FlashChipType128 Flash_Type_128 { get; set; }

			[DisplayName("Flash Write Offset (Not used for Atmel)")]
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

			[DisplayName("Flash Sector Erase (or Atmel erase/write) Offset")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset
			{
				get => _Flash_Sector_Erase_Offset;
				set => _Flash_Sector_Erase_Offset = value;
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
