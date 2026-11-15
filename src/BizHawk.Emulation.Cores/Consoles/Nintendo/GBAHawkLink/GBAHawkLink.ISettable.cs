using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

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
				A,
				B,
				C,
				D
			}

			[DisplayName("Tracer Selection")]
			[Description("Choose Trace Logger Source.")]
			[DefaultValue(TraceSrc.A)]
			public TraceSrc TraceSet { get; set; }

			public enum VideoSrc
			{
				Enable,
				Disable
			}

			[DisplayName("Enable Audio Console A")]
			[Description("Output audio if console is active.")]
			[DefaultValue(true)]
			public bool A_AudioSet { get; set; }

			[DisplayName("Enable Audio Console B")]
			[Description("Output audio if console is active.")]
			[DefaultValue(false)]
			public bool B_AudioSet { get; set; }

			[DisplayName("Enable Audio Console C")]
			[Description("Output audio if console is active.")]
			[DefaultValue(false)]
			public bool C_AudioSet { get; set; }

			[DisplayName("Enable Audio Console D")]
			[Description("Output audio if console is active.")]
			[DefaultValue(false)]
			public bool D_AudioSet { get; set; }

			[DisplayName("Enable Video Consola A")]
			[Description("Output video if console is active.")]
			[DefaultValue(VideoSrc.Enable)]
			public VideoSrc A_VideoSet { get; set; }

			[DisplayName("Enable Video Consola B")]
			[Description("Output video if console is active.")]
			[DefaultValue(VideoSrc.Enable)]
			public VideoSrc B_VideoSet { get; set; }

			[DisplayName("Enable Video Consola C")]
			[Description("Output video if console is active.")]
			[DefaultValue(VideoSrc.Enable)]
			public VideoSrc C_VideoSet { get; set; }

			[DisplayName("Enable Video Consola D")]
			[Description("Output video if console is active.")]
			[DefaultValue(VideoSrc.Enable)]
			public VideoSrc D_VideoSet { get; set; }

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
			[DisplayName("One Screen Mode")]
			[Description("Only display one console, based on setting selection.)")]
			[DefaultValue(false)]
			public bool OneScreenMode { get; set; }

			public enum InitRTCState
			{
				Reset_Good_Batt,
				Reset_Bad_Batt,
				RTC_Set
			}

			[DisplayName("RTC Initial State Console A")]
			[Description("Choose how the RTC starts up")]
			[DefaultValue(InitRTCState.Reset_Good_Batt)]
			public InitRTCState RTCInitialState_A { get; set; }

			[DisplayName("RTC Initial State Console B")]
			[Description("Choose how the RTC starts up")]
			[DefaultValue(InitRTCState.Reset_Good_Batt)]
			public InitRTCState RTCInitialState_B { get; set; }

			[DisplayName("RTC Initial State Console C")]
			[Description("Choose how the RTC starts up")]
			[DefaultValue(InitRTCState.Reset_Good_Batt)]
			public InitRTCState RTCInitialState_C { get; set; }

			[DisplayName("RTC Initial State Console D")]
			[Description("Choose how the RTC starts up")]
			[DefaultValue(InitRTCState.Reset_Good_Batt)]
			public InitRTCState RTCInitialState_D { get; set; }

			[DisplayName("RTC Initial Time Console A")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime_A
			{
				get => _RTCInitialTime_A;
				set => _RTCInitialTime_A = value;
			}

			[DisplayName("RTC Initial Time Console B")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime_B
			{
				get => _RTCInitialTime_B;
				set => _RTCInitialTime_B = value;
			}

			[DisplayName("RTC Initial Time Console C")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime_C
			{
				get => _RTCInitialTime_C;
				set => _RTCInitialTime_C = value;
			}

			[DisplayName("RTC Initial Time Console D")]
			[Description("Set the initial RTC Date and Time.")]
			[DefaultValue(typeof(DateTime), "1/1/2000 12:00:00 PM")]
			public DateTime RTCInitialTime_D
			{
				get => _RTCInitialTime_D;
				set => _RTCInitialTime_D = value;
			}

			[DisplayName("RTC Offset Console A")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset_A
			{
				get => _RTCOffset_A;
				set => _RTCOffset_A = value;
			}

			[DisplayName("RTC Offset Console B")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset_B
			{
				get => _RTCOffset_B;
				set => _RTCOffset_B = value;
			}

			[DisplayName("RTC Offset Console C")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset_C
			{
				get => _RTCOffset_C;
				set => _RTCOffset_C = value;
			}

			[DisplayName("RTC Offset Console D")]
			[Description("Set error in RTC clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short RTCOffset_D
			{
				get => _RTCOffset_D;
				set => _RTCOffset_D = value;
			}

			[DisplayName("EEPROM Offset Console A")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset_A
			{
				get => _EEPROM_Offset_A;
				set => _EEPROM_Offset_A = value;
			}

			[DisplayName("EEPROM Offset Console B")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset_B
			{
				get => _EEPROM_Offset_B;
				set => _EEPROM_Offset_B = value;
			}

			[DisplayName("EEPROM Offset Console C")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset_C
			{
				get => _EEPROM_Offset_C;
				set => _EEPROM_Offset_C = value;
			}

			[DisplayName("EEPROM Offset Console D")]
			[Description("Set error in EEPROM clocking (-32768 to 32767)")]
			[DefaultValue(0)]
			public short EEPROMOffset_D
			{
				get => _EEPROM_Offset_D;
				set => _EEPROM_Offset_D = value;
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

			[DisplayName("Flash Chip Type (64K) Console A")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType64.Panasonic)]
			public FlashChipType64 Flash_Type_64_A { get; set; }

			[DisplayName("Flash Chip Type (64K) Console B")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType64.Panasonic)]
			public FlashChipType64 Flash_Type_64_B { get; set; }

			[DisplayName("Flash Chip Type (64K) Console C")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType64.Panasonic)]
			public FlashChipType64 Flash_Type_64_C { get; set; }

			[DisplayName("Flash Chip Type (64K) Console D")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType64.Panasonic)]
			public FlashChipType64 Flash_Type_64_D { get; set; }

			[DisplayName("Flash Chip Type (128K) Console A")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType128.Sanyo)]
			public FlashChipType128 Flash_Type_128_A { get; set; }

			[DisplayName("Flash Chip Type (128K) Console B")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType128.Sanyo)]
			public FlashChipType128 Flash_Type_128_B { get; set; }

			[DisplayName("Flash Chip Type (128K) Console C")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType128.Sanyo)]
			public FlashChipType128 Flash_Type_128_C { get; set; }

			[DisplayName("Flash Chip Type (128K) Console D")]
			[Description("NOTE: Flash timings not fully characterized.")]
			[DefaultValue(FlashChipType128.Sanyo)]
			public FlashChipType128 Flash_Type_128_D { get; set; }

			[DisplayName("Flash Write Offset Console A (Not used for Atmel)")]
			[Description("Set offset in Flash write timing (-512 to 511). Value less than or equal to -325 results in instant writes.")]
			[DefaultValue(0)]
			public short FlashWriteOffset_A
			{
				get => _Flash_Write_Offset_A;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset_A = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset_A = -512;
					}
					else
					{
						_Flash_Write_Offset_A = value;
					}
				}
			}

			[DisplayName("Flash Write Offset Console B (Not used for Atmel)")]
			[Description("Set offset in Flash write timing (-512 to 511). Value less than or equal to -325 results in instant writes.")]
			[DefaultValue(0)]
			public short FlashWriteOffset_B
			{
				get => _Flash_Write_Offset_B;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset_B = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset_B = -512;
					}
					else
					{
						_Flash_Write_Offset_B = value;
					}
				}
			}

			[DisplayName("Flash Write Offset Console C (Not used for Atmel)")]
			[Description("Set offset in Flash write timing (-512 to 511). Value less than or equal to -325 results in instant writes.")]
			[DefaultValue(0)]
			public short FlashWriteOffset_C
			{
				get => _Flash_Write_Offset_C;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset_C = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset_C = -512;
					}
					else
					{
						_Flash_Write_Offset_C = value;
					}
				}
			}

			[DisplayName("Flash Write Offset Console D (Not used for Atmel)")]
			[Description("Set offset in Flash write timing (-512 to 511). Value less than or equal to -325 results in instant writes.")]
			[DefaultValue(0)]
			public short FlashWriteOffset_D
			{
				get => _Flash_Write_Offset_D;
				set
				{
					if (value > 511)
					{
						_Flash_Write_Offset_D = 511;
					}
					else if (value < -512)
					{
						_Flash_Write_Offset_D = -512;
					}
					else
					{
						_Flash_Write_Offset_D = value;
					}
				}
			}

			[DisplayName("Flash Sector Erase (or Atmel erase/write) Offset Console A")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset_A
			{
				get => _Flash_Sector_Erase_Offset_A;
				set => _Flash_Sector_Erase_Offset_A = value;
			}

			[DisplayName("Flash Sector Erase (or Atmel erase/write) Offset Console B")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset_B
			{
				get => _Flash_Sector_Erase_Offset_B;
				set => _Flash_Sector_Erase_Offset_B = value;
			}

			[DisplayName("Flash Sector Erase (or Atmel erase/write) Offset Console C")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset_C
			{
				get => _Flash_Sector_Erase_Offset_C;
				set => _Flash_Sector_Erase_Offset_C = value;
			}

			[DisplayName("Flash Sector Erase (or Atmel erase/write) Offset Console D")]
			[Description("Set offset in Flash Sector Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashSectorEraseOffset_D
			{
				get => _Flash_Sector_Erase_Offset_D;
				set => _Flash_Sector_Erase_Offset_D = value;
			}

			[DisplayName("Flash Chip Erase Offset Console A")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset_A
			{
				get => _Flash_Chip_Erase_Offset_A;
				set => _Flash_Chip_Erase_Offset_A = value;
			}

			[DisplayName("Flash Chip Erase Offset Console B")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset_B
			{
				get => _Flash_Chip_Erase_Offset_B;
				set => _Flash_Chip_Erase_Offset_B = value;
			}

			[DisplayName("Flash Chip Erase Offset Console C")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset_C
			{
				get => _Flash_Chip_Erase_Offset_C;
				set => _Flash_Chip_Erase_Offset_C = value;
			}

			[DisplayName("Flash Chip Erase Offset Console D")]
			[Description("Set offset in Flash Chip Erase timing (Int range)")]
			[DefaultValue(0)]
			public int FlashChipEraseOffset_D
			{
				get => _Flash_Chip_Erase_Offset_D;
				set => _Flash_Chip_Erase_Offset_D = value;
			}

			[DisplayName("Use Gamerboy Player Console A")]
			[Description("Gameboy Player will be used and detected by supported games on left console.")]
			[DefaultValue(false)]
			public bool Use_GBP_A { get; set; }

			[DisplayName("Use Gamerboy Player Console B")]
			[Description("Gameboy Player will be used and detected by supported games on left console.")]
			[DefaultValue(false)]
			public bool Use_GBP_B { get; set; }

			[DisplayName("Use Gamerboy Player Console C")]
			[Description("Gameboy Player will be used and detected by supported games on left console.")]
			[DefaultValue(false)]
			public bool Use_GBP_C { get; set; }

			[DisplayName("Use Gamerboy Player Console D")]
			[Description("Gameboy Player will be used and detected by supported games on left console.")]
			[DefaultValue(false)]
			public bool Use_GBP_D { get; set; }

			[DisplayName("Use Discrete Solar Levels Console A")]
			[Description("When true, light levels are selected via hotkeys at user settable levels")]
			[DefaultValue(false)]
			public bool Use_Discrete_Solar_A { get; set; }

			[DisplayName("Light Level 1 Console A")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel1_A
			{
				get => _Light_Level_1_A;
				set
				{
					if (value > 100) { _Light_Level_1_A = 100; }
					else if (value < 0) { _Light_Level_1_A = 0; }
					else { _Light_Level_1_A = value; }
				}
			}

			[DisplayName("Light Level 2 Console A")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel2_A
			{
				get => _Light_Level_2_A;
				set
				{
					if (value > 100) { _Light_Level_2_A = 100; }
					else if (value < 0) { _Light_Level_2_A = 0; }
					else { _Light_Level_2_A = value; }
				}
			}

			[DisplayName("Light Level 3 Console A")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel3_A
			{
				get => _Light_Level_3_A;
				set
				{
					if (value > 100) { _Light_Level_3_A = 100; }
					else if (value < 0) { _Light_Level_3_A = 0; }
					else { _Light_Level_3_A = value; }
				}
			}

			[DisplayName("Light Level 4 Console A")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel4_A
			{
				get => _Light_Level_4_A;
				set
				{
					if (value > 100) { _Light_Level_4_A = 100; }
					else if (value < 0) { _Light_Level_4_A = 0; }
					else { _Light_Level_4_A = value; }
				}
			}

			[DisplayName("Light Level 5 Console A")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel5_A
			{
				get => _Light_Level_5_A;
				set
				{
					if (value > 100) { _Light_Level_5_A = 100; }
					else if (value < 0) { _Light_Level_5_A = 0; }
					else { _Light_Level_5_A = value; }
				}
			}

			[DisplayName("Use Discrete Solar Levels Console B")]
			[Description("When true, light levels are selected via hotkeys at user settable levels")]
			[DefaultValue(false)]
			public bool Use_Discrete_Solar_B { get; set; }

			[DisplayName("Light Level 1 Console B")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel1_B
			{
				get => _Light_Level_1_B;
				set
				{
					if (value > 100) { _Light_Level_1_B = 100; }
					else if (value < 0) { _Light_Level_1_B = 0; }
					else { _Light_Level_1_B = value; }
				}
			}

			[DisplayName("Light Level 2 Console B")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel2_B
			{
				get => _Light_Level_2_B;
				set
				{
					if (value > 100) { _Light_Level_2_B = 100; }
					else if (value < 0) { _Light_Level_2_B = 0; }
					else { _Light_Level_2_B = value; }
				}
			}

			[DisplayName("Light Level 3 Console B")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel3_B
			{
				get => _Light_Level_3_B;
				set
				{
					if (value > 100) { _Light_Level_3_B = 100; }
					else if (value < 0) { _Light_Level_3_B = 0; }
					else { _Light_Level_3_B = value; }
				}
			}

			[DisplayName("Light Level 4 Console B")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel4_B
			{
				get => _Light_Level_4_B;
				set
				{
					if (value > 100) { _Light_Level_4_B = 100; }
					else if (value < 0) { _Light_Level_4_B = 0; }
					else { _Light_Level_4_B = value; }
				}
			}

			[DisplayName("Light Level 5 Console B")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel5_B
			{
				get => _Light_Level_5_B;
				set
				{
					if (value > 100) { _Light_Level_5_B = 100; }
					else if (value < 0) { _Light_Level_5_B = 0; }
					else { _Light_Level_5_B = value; }
				}
			}

			[DisplayName("Use Discrete Solar Levels Console C")]
			[Description("When true, light levels are selected via hotkeys at user settable levels")]
			[DefaultValue(false)]
			public bool Use_Discrete_Solar_C { get; set; }

			[DisplayName("Light Level 1 Console C")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel1_C
			{
				get => _Light_Level_1_C;
				set
				{
					if (value > 100) { _Light_Level_1_C = 100; }
					else if (value < 0) { _Light_Level_1_C = 0; }
					else { _Light_Level_1_C = value; }
				}
			}

			[DisplayName("Light Level 2 Console C")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel2_C
			{
				get => _Light_Level_2_C;
				set
				{
					if (value > 100) { _Light_Level_2_C = 100; }
					else if (value < 0) { _Light_Level_2_C = 0; }
					else { _Light_Level_2_C = value; }
				}
			}

			[DisplayName("Light Level 3 Console C")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel3_C
			{
				get => _Light_Level_3_C;
				set
				{
					if (value > 100) { _Light_Level_3_C = 100; }
					else if (value < 0) { _Light_Level_3_C = 0; }
					else { _Light_Level_3_C = value; }
				}
			}

			[DisplayName("Light Level 4 Console C")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel4_C
			{
				get => _Light_Level_4_C;
				set
				{
					if (value > 100) { _Light_Level_4_C = 100; }
					else if (value < 0) { _Light_Level_4_C = 0; }
					else { _Light_Level_4_C = value; }
				}
			}

			[DisplayName("Light Level 5 Console C")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel5_C
			{
				get => _Light_Level_5_C;
				set
				{
					if (value > 100) { _Light_Level_5_C = 100; }
					else if (value < 0) { _Light_Level_5_C = 0; }
					else { _Light_Level_5_C = value; }
				}
			}

			[DisplayName("Use Discrete Solar Levels Console D")]
			[Description("When true, light levels are selected via hotkeys at user settable levels")]
			[DefaultValue(false)]
			public bool Use_Discrete_Solar_D { get; set; }

			[DisplayName("Light Level 1 Console D")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel1_D
			{
				get => _Light_Level_1_D;
				set
				{
					if (value > 100) { _Light_Level_1_D = 100; }
					else if (value < 0) { _Light_Level_1_D = 0; }
					else { _Light_Level_1_D = value; }
				}
			}

			[DisplayName("Light Level 2 Console D")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel2_D
			{
				get => _Light_Level_2_D;
				set
				{
					if (value > 100) { _Light_Level_2_D = 100; }
					else if (value < 0) { _Light_Level_2_D = 0; }
					else { _Light_Level_2_D = value; }
				}
			}

			[DisplayName("Light Level 3 Console D")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel3_D
			{
				get => _Light_Level_3_D;
				set
				{
					if (value > 100) { _Light_Level_3_D = 100; }
					else if (value < 0) { _Light_Level_3_D = 0; }
					else { _Light_Level_3_D = value; }
				}
			}

			[DisplayName("Light Level 4 Console D")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel4_D
			{
				get => _Light_Level_4_D;
				set
				{
					if (value > 100) { _Light_Level_4_D = 100; }
					else if (value < 0) { _Light_Level_4_D = 0; }
					else { _Light_Level_4_D = value; }
				}
			}

			[DisplayName("Light Level 5 Console D")]
			[Description("User settable light level % (0-100)")]
			[DefaultValue(0)]
			public short LightLevel5_D
			{
				get => _Light_Level_5_D;
				set
				{
					if (value > 100) { _Light_Level_5_D = 100; }
					else if (value < 0) { _Light_Level_5_D = 0; }
					else { _Light_Level_5_D = value; }
				}
			}

			[JsonIgnore]
			private DateTime _RTCInitialTime_A;
			[JsonIgnore]
			private DateTime _RTCInitialTime_B;
			[JsonIgnore]
			private DateTime _RTCInitialTime_C;
			[JsonIgnore]
			private DateTime _RTCInitialTime_D;
			[JsonIgnore]
			private short _RTCOffset_A;
			[JsonIgnore]
			private short _RTCOffset_B;
			[JsonIgnore]
			private short _RTCOffset_C;
			[JsonIgnore]
			private short _RTCOffset_D;
			[JsonIgnore]
			private short _EEPROM_Offset_A;
			[JsonIgnore]
			private short _EEPROM_Offset_B;
			[JsonIgnore]
			private short _EEPROM_Offset_C;
			[JsonIgnore]
			private short _EEPROM_Offset_D;
			[JsonIgnore]
			private short _Flash_Write_Offset_A;
			[JsonIgnore]
			private short _Flash_Write_Offset_B;
			[JsonIgnore]
			private short _Flash_Write_Offset_C;
			[JsonIgnore]
			private short _Flash_Write_Offset_D;
			[JsonIgnore]
			private short _Light_Level_1_A;
			[JsonIgnore]
			private short _Light_Level_2_A;
			[JsonIgnore]
			private short _Light_Level_3_A;
			[JsonIgnore]
			private short _Light_Level_4_A;
			[JsonIgnore]
			private short _Light_Level_5_A;
			[JsonIgnore]
			private short _Light_Level_1_B;
			[JsonIgnore]
			private short _Light_Level_2_B;
			[JsonIgnore]
			private short _Light_Level_3_B;
			[JsonIgnore]
			private short _Light_Level_4_B;
			[JsonIgnore]
			private short _Light_Level_5_B;
			[JsonIgnore]
			private short _Light_Level_1_C;
			[JsonIgnore]
			private short _Light_Level_2_C;
			[JsonIgnore]
			private short _Light_Level_3_C;
			[JsonIgnore]
			private short _Light_Level_4_C;
			[JsonIgnore]
			private short _Light_Level_5_C;
			[JsonIgnore]
			private short _Light_Level_1_D;
			[JsonIgnore]
			private short _Light_Level_2_D;
			[JsonIgnore]
			private short _Light_Level_3_D;
			[JsonIgnore]
			private short _Light_Level_4_D;
			[JsonIgnore]
			private short _Light_Level_5_D;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset_A;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset_B;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset_C;
			[JsonIgnore]
			private int _Flash_Sector_Erase_Offset_D;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset_A;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset_B;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset_C;
			[JsonIgnore]
			private int _Flash_Chip_Erase_Offset_D;

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
