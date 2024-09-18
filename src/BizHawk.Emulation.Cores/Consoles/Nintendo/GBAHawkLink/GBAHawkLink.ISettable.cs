using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;
using static BizHawk.Emulation.Cores.Nintendo.GBHawkLink.GBHawkLink;
using static BizHawk.Emulation.Cores.Nintendo.GBHawkLink.GBHawkLink.GBLinkSettings;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink : ISettable<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings>
	{
		public GBALinkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public GBALinkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBALinkSettings o)
		{
			bool ret = GBALinkSettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBALinkSyncSettings o)
		{
			bool ret = GBALinkSyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal GBALinkSettings Settings { get; private set; }
		internal GBALinkSyncSettings SyncSettings { get; private set; }

		public class GBALinkSettings
		{
			public enum TraceSrc
			{
				Left,
				Right
			}

			[DisplayName("Tracer Selection")]
			[Description("Choose Trace Logger Source.")]
			[DefaultValue(TraceSrc.Left)]
			public TraceSrc TraceSet { get; set; }

			public enum AudioSrc
			{
				Left,
				Right,
				Both
			}

			public enum VideoSrc
			{
				Left,
				Right,
				Both
			}

			[DisplayName("Audio Selection")]
			[Description("Choose Audio Source. Both will produce Stereo sound.")]
			[DefaultValue(AudioSrc.Left)]
			public AudioSrc AudioSet { get; set; }

			[DisplayName("Video Selection")]
			[Description("Choose Video Source.")]
			[DefaultValue(VideoSrc.Both)]
			public VideoSrc VideoSet { get; set; }

			public GBALinkSettings Clone()
			{
				return (GBALinkSettings)MemberwiseClone();
			}

			public GBALinkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(GBALinkSettings x, GBALinkSettings y)
			{
				return false;
			}
		}

		public class GBALinkSyncSettings
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
			public InitRTCState RTCInitialState_L { get; set; }

			[DisplayName("RTC Initial State")]
			[Description("Choose how the RTC starts up")]
			[DefaultValue(InitRTCState.Reset_Good_Batt)]
			public InitRTCState RTCInitialState_R { get; set; }

			[DisplayName("RTC Initial Time L")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime_L
			{
				get => _RTCInitialTime_L;
				set => _RTCInitialTime_L = value;
			}

			[DisplayName("RTC Initial Time R")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime_R
			{
				get => _RTCInitialTime_R;
				set => _RTCInitialTime_R = value;
			}

			[DisplayName("RTC Offset L")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset_L
			{
				get => _RTCOffset_L;
				set => _RTCOffset_L = value;
			}

			[DisplayName("RTC Offset R")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset_R
			{
				get => _RTCOffset_R;
				set => _RTCOffset_R = value;
			}

			[DisplayName("EEPROM Offset L")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset_L
			{
				get => _EEPROM_Offset_L;
				set => _EEPROM_Offset_L = value;
			}

			[DisplayName("EEPROM Offset R")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset_R
			{
				get => _EEPROM_Offset_R;
				set => _EEPROM_Offset_R = value;
			}

			[DisplayName("Flash Write Offset L")]
			[Description("Set offset in Flash write timing (-512 to 511)")]
			[DefaultValue(0)]
			public short FlashWriteOffset_L
			{
				get => _Flash_Write_Offset_L;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset_L = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset_L = -512;
					}
					else
					{
						_Flash_Write_Offset_L = value;
					}
				}
			}

			[DisplayName("Flash Write Offset R")]
			[Description("Set offset in Flash write timing (-512 to 511)")]
			[DefaultValue(0)]
			public short FlashWriteOffset_R
			{
				get => _Flash_Write_Offset_R;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset_R = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset_R = -512;
					}
					else
					{
						_Flash_Write_Offset_R = value;
					}
				}
			}

			[DisplayName("Flash Sector Erase Offset L")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset_L
			{
				get => _Flash_Sector_Erase_Offset_L;
				set => _Flash_Sector_Erase_Offset_L = value;
			}

			[DisplayName("Flash Sector Erase Offset R")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset_R
			{
				get => _Flash_Sector_Erase_Offset_R;
				set => _Flash_Sector_Erase_Offset_R = value;
			}

			[DisplayName("Flash Chip Erase Offset L")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset_L
			{
				get => _Flash_Chip_Erase_Offset_L;
				set => _Flash_Chip_Erase_Offset_L = value;
			}

			[DisplayName("Flash Chip Erase Offset R")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset_R
			{
				get => _Flash_Chip_Erase_Offset_R;
				set => _Flash_Chip_Erase_Offset_R = value;
			}

			[DisplayName("Use Existing SaveRAM")]
			[Description("(Intended for development, for regular use leave as true.) When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(true)]
			public bool Use_SRAM { get; set; }

			[DisplayName("Use Gamerboy Player L")]
			[Description("Gameboy Player will be used and detected by supported games on left console.")]
			[DefaultValue(false)]
			public bool Use_GBP_L { get; set; }

			[DisplayName("Use Gamerboy Player R")]
			[Description("Gameboy Player will be used and detected by supported games on right console.")]
			[DefaultValue(false)]
			public bool Use_GBP_R { get; set; }

			[JsonIgnore]
			private DateTime _RTCInitialTime_L;
			[JsonIgnore]
			private DateTime _RTCInitialTime_R;
			[JsonIgnore]
			private short _RTCOffset_L;
			[JsonIgnore]
			private short _RTCOffset_R;
			[JsonIgnore]
			private short _EEPROM_Offset_L;
			[JsonIgnore]
			private short _EEPROM_Offset_R;
			[JsonIgnore]
			private short _Flash_Write_Offset_L;
			[JsonIgnore]
			private short _Flash_Write_Offset_R;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset_L;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset_R;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset_L;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset_R;

			public GBALinkSyncSettings Clone()
			{
				return (GBALinkSyncSettings)MemberwiseClone();
			}

			public GBALinkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(GBALinkSyncSettings x, GBALinkSyncSettings y)
			{
				return true;
			}
		}
	}
}
