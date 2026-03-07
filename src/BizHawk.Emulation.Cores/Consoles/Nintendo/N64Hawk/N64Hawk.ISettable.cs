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
			[DisplayName("Enable RAM Expansion Pack")]
			[Description("Adds 4MB RAM to the Console")]
			[DefaultValue(true)]
			public bool Enable_RAM_Pack { get; set; }

			public enum ControllerType
			{
				N64,
				Unplugged
			}

			[DisplayName("Player 1 Controller Port")]
			[Description("Player 1")]
			[DefaultValue(ControllerType.N64)]
			public ControllerType P1Controller { get; set; }

			[DisplayName("Player 2 Controller Port")]
			[Description("Player 2")]
			[DefaultValue(ControllerType.N64)]
			public ControllerType P2Controller { get; set; }

			[DisplayName("Player 3 Controller Port")]
			[Description("Player 3")]
			[DefaultValue(ControllerType.N64)]
			public ControllerType P3Controller { get; set; }

			[DisplayName("Player 4 Controller Port")]
			[Description("Player 4")]
			[DefaultValue(ControllerType.N64)]
			public ControllerType P4Controller { get; set; }

			public enum AddOnType
			{
				CntrPak,
				RumblePak,
				GBExp,
				Empty
			}

			[DisplayName("Player 1 Controller Add on")]
			[Description("Player 1")]
			[DefaultValue(AddOnType.Empty)]
			public AddOnType P1AddOn { get; set; }

			[DisplayName("Player 2 Controller Add on")]
			[Description("Player 2 Add on")]
			[DefaultValue(AddOnType.Empty)]
			public AddOnType P2AddOn { get; set; }

			[DisplayName("Player 3 Controller Add on")]
			[Description("Player 3 Add on")]
			[DefaultValue(AddOnType.Empty)]
			public AddOnType P3AddOn { get; set; }

			[DisplayName("Player 4 Controller Add on")]
			[Description("Player 4 Add on")]
			[DefaultValue(AddOnType.Empty)]
			public AddOnType P4AddOn { get; set; }

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
