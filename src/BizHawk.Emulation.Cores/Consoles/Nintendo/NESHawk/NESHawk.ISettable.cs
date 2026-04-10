using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.NESHawk
{
	public partial class NESHawk : ISettable<NESHawk.NESHawkSettings, NESHawk.NESHawkSyncSettings>
	{
		public NESHawkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public NESHawkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(NESHawkSettings o)
		{
			bool ret = NESHawkSettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(NESHawkSyncSettings o)
		{
			bool ret = NESHawkSyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal NESHawkSettings Settings { get; private set; }
		internal NESHawkSyncSettings SyncSettings { get; private set; }

		public class NESHawkSettings
		{
			[DisplayName("Display background")]
			[Description("When true, displays background, does not effect emulation")]
			[DefaultValue(true)]
			public bool DispBackground { get; set; }

			[DisplayName("Display sprites")]
			[Description("When true, displays sprites, does not effect emulation")]
			[DefaultValue(true)]
			public bool DispSprites { get; set; }

			public NESHawkSettings Clone()
			{
				return (NESHawkSettings)MemberwiseClone();
			}

			public NESHawkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(NESHawkSettings x, NESHawkSettings y)
			{
				return false;
			}
		}

		public class NESHawkSyncSettings
		{
			public enum ControllerType
			{
				Standard,
				Zapper,
				SNES,
				FourScore,
				Unplugged
			}

			[DisplayName("Left Controller Port")]
			[Description("Player 1")]
			[DefaultValue(ControllerType.Standard)]
			public ControllerType LeftController { get; set; }

			[DisplayName("Right Controller Port")]
			[Description("Player 2")]
			[DefaultValue(ControllerType.Standard)]
			public ControllerType RightController { get; set; }

			public enum MMC3IRQType
			{
				Old,
				New
			}

			[DisplayName("MMC3 IRQ Type")]
			[Description("Chip Dependent")]
			[DefaultValue(MMC3IRQType.New)]
			public MMC3IRQType MMC3_IRQ_Type { get; set; }

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

			public NESHawkSyncSettings Clone()
			{
				return (NESHawkSyncSettings)MemberwiseClone();
			}

			public NESHawkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(NESHawkSyncSettings x, NESHawkSyncSettings y)
			{
				return true;
			}
		}
	}
}
