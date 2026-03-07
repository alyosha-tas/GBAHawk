using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.N64Hawk
{
	public partial class N64Hawk : ISettable<N64Hawk.N64HawkSettings, N64Hawk.N64HawkSyncSettings>
	{
		public N64HawkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public N64HawkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(N64HawkSettings o)
		{
			bool ret = N64HawkSettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(N64HawkSyncSettings o)
		{
			bool ret = N64HawkSyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal N64HawkSettings Settings { get; private set; }
		internal N64HawkSyncSettings SyncSettings { get; private set; }

		public class N64HawkSettings
		{
			[DisplayName("Display background")]
			[Description("When true, displays background, does not effect emulation")]
			[DefaultValue(true)]
			public bool DispBackground { get; set; }

			[DisplayName("Display sprites")]
			[Description("When true, displays sprites, does not effect emulation")]
			[DefaultValue(true)]
			public bool DispSprites { get; set; }

			public N64HawkSettings Clone()
			{
				return (N64HawkSettings)MemberwiseClone();
			}

			public N64HawkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(N64HawkSettings x, N64HawkSettings y)
			{
				return false;
			}
		}

		public class N64HawkSyncSettings
		{
			public enum ControllerType
			{
				N64,
				Unplugged
			}

			[DisplayName("Left Controller Port")]
			[Description("Player 1")]
			[DefaultValue(ControllerType.N64)]
			public ControllerType LeftController { get; set; }

			[DisplayName("Right Controller Port")]
			[Description("Player 2")]
			[DefaultValue(ControllerType.N64)]
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

			public N64HawkSyncSettings Clone()
			{
				return (N64HawkSyncSettings)MemberwiseClone();
			}

			public N64HawkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(N64HawkSyncSettings x, N64HawkSyncSettings y)
			{
				return true;
			}
		}
	}
}
