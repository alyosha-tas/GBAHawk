using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SNESHawk
{
	public partial class SNESHawk : ISettable<SNESHawk.SNESHawkSettings, SNESHawk.SNESHawkSyncSettings>
	{
		public SNESHawkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public SNESHawkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(SNESHawkSettings o)
		{
			bool ret = SNESHawkSettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(SNESHawkSyncSettings o)
		{
			bool ret = SNESHawkSyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal SNESHawkSettings Settings { get; private set; }
		internal SNESHawkSyncSettings SyncSettings { get; private set; }

		public class SNESHawkSettings
		{
			[DisplayName("Display background")]
			[Description("When true, displays background, does not effect emulation")]
			[DefaultValue(true)]
			public bool DispBackground { get; set; }

			[DisplayName("Display sprites")]
			[Description("When true, displays sprites, does not effect emulation")]
			[DefaultValue(true)]
			public bool DispSprites { get; set; }

			public SNESHawkSettings Clone()
			{
				return (SNESHawkSettings)MemberwiseClone();
			}

			public SNESHawkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(SNESHawkSettings x, SNESHawkSettings y)
			{
				return false;
			}
		}

		public class SNESHawkSyncSettings
		{
			public enum ControllerType
			{
				SNES,
				Unplugged
			}

			[DisplayName("Left Controller Port")]
			[Description("Player 1")]
			[DefaultValue(ControllerType.SNES)]
			public ControllerType LeftController { get; set; }

			[DisplayName("Right Controller Port")]
			[Description("Player 2")]
			[DefaultValue(ControllerType.SNES)]
			public ControllerType RightController { get; set; }


			[DisplayName("APU Frequency")]
			[Description("Audio Crystal Clock")]
			[DefaultValue(32045)]
			public uint APU_Freq { get; set; }

			[DisplayName("PPU Initial Horizontal Position")]
			[Description("PPU dot where CPU starts execution")]
			[DefaultValue(506)]
			public uint PPU_H_Pos { get; set; }

			[DisplayName("PPU Initial Vertical Position")]
			[Description("PPU scanline where CPU starts execution")]
			[DefaultValue(0)]
			public uint PPU_V_Pos { get; set; }

			[DisplayName("DRAM Refresh Position")]
			[Description("PPU dot where RAM refresh occurs")]
			[DefaultValue(538)]
			public uint DRAM_Refresh_Cycle { get; set; }

			public SNESHawkSyncSettings Clone()
			{
				return (SNESHawkSyncSettings)MemberwiseClone();
			}

			public SNESHawkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(SNESHawkSyncSettings x, SNESHawkSyncSettings y)
			{
				return true;
			}
		}
	}
}
