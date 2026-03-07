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


			[DisplayName("Emulate Mapper Bus Conflicts")]
			[Description("Effects CNROM, AxROM, UxROM")]
			[DefaultValue(true)]
			public bool Mapper_Bus_Conflicts { get; set; }

			[DisplayName("Activate APU Test Registers")]
			[Description("See NESDev documentation")]
			[DefaultValue(false)]
			public bool Use_APU_Test_Regs { get; set; }

			[DisplayName("Set CPU Zero Flag at Reset")]
			[Description("Revision / console dependent")]
			[DefaultValue(true)]
			public bool CPU_Zero_Reset { get; set; }

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
