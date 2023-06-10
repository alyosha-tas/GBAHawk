using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;
using static BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug.GBAHawkLink_Debug;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	public partial class GBAHawkLink_Debug : IEmulator, IStatable, ISettable<GBAHawkLink_Debug.GBAHawkLink_Debug_Settings, GBAHawkLink_Debug.GBAHawkLink_Debug_SyncSettings>
	{
		public GBAHawkLink_Debug_Settings GetSettings() => linkSettings.Clone();

		public GBAHawkLink_Debug_SyncSettings GetSyncSettings() => linkSyncSettings.Clone();

		public PutSettingsDirtyBits PutSettings(GBAHawkLink_Debug_Settings o)
		{
			linkSettings = o;
			return PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBAHawkLink_Debug_SyncSettings o)
		{
			bool ret = GBAHawkLink_Debug_SyncSettings.NeedsReboot(linkSyncSettings, o);
			linkSyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		private GBAHawkLink_Debug_Settings linkSettings = new GBAHawkLink_Debug_Settings();
		public GBAHawkLink_Debug_SyncSettings linkSyncSettings = new GBAHawkLink_Debug_SyncSettings();

		public class GBAHawkLink_Debug_Settings
		{
			public enum AudioSrc
			{
				Left,
				Right,
				Both
			}

			[DisplayName("Audio Selection")]
			[Description("Choose Audio Source. Both will produce Stereo sound.")]
			[DefaultValue(AudioSrc.Left)]
			public AudioSrc AudioSet { get; set; }

			public GBAHawkLink_Debug_Settings Clone() => (GBAHawkLink_Debug_Settings)MemberwiseClone();

			public GBAHawkLink_Debug_Settings() => SettingsUtil.SetDefaultValues(this);
		}

		public class GBAHawkLink_Debug_SyncSettings
		{
			[DisplayName("Console Mode L")]
			[Description("Pick which console to run, 'Auto' chooses from ROM extension, 'GB' and 'GBC' chooses the respective system")]
			[DefaultValue(GBHawk.GBHawk.GBSyncSettings.ConsoleModeType.Auto)]
			public GBHawk.GBHawk.GBSyncSettings.ConsoleModeType ConsoleMode_L { get; set; }

			[DisplayName("Console Mode R")]
			[Description("Pick which console to run, 'Auto' chooses from ROM extension, 'GB' and 'GBC' chooses the respective system")]
			[DefaultValue(GBHawk.GBHawk.GBSyncSettings.ConsoleModeType.Auto)]
			public GBHawk.GBHawk.GBSyncSettings.ConsoleModeType ConsoleMode_R { get; set; }

			[DisplayName("CGB in GBA")]
			[Description("Emulate GBA hardware running a CGB game, instead of CGB hardware.  Relevant only for titles that detect the presense of a GBA, such as Shantae.")]
			[DefaultValue(false)]
			public bool GBACGB { get; set; }

			[DisplayName("RTC Initial Time L")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int RTCInitialTime_L
			{
				get => _RTCInitialTime_L;
				set => _RTCInitialTime_L = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Initial Time R")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int RTCInitialTime_R
			{
				get => _RTCInitialTime_R;
				set => _RTCInitialTime_R = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Offset L")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int RTCOffset_L
			{
				get => _RTCOffset_L;
				set => _RTCOffset_L = Math.Max(-127, Math.Min(127, value));
			}

			[DisplayName("RTC Offset R")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int RTCOffset_R
			{
				get => _RTCOffset_R;
				set => _RTCOffset_R = Math.Max(-127, Math.Min(127, value));
			}

			[DisplayName("Use Existing SaveRAM")]
			[Description("(Intended for development, for regular use leave as true.) When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(true)]
			public bool Use_SRAM { get; set; }

			[JsonIgnore]
			private int _RTCInitialTime_L;
			[JsonIgnore]
			private int _RTCInitialTime_R;
			[JsonIgnore]
			private int _RTCOffset_L;
			[JsonIgnore]
			private int _RTCOffset_R;
			[JsonIgnore]
			public ushort _DivInitialTime_L = 8;
			[JsonIgnore]
			public ushort _DivInitialTime_R = 8;

			public GBAHawkLink_Debug_SyncSettings Clone() => (GBAHawkLink_Debug_SyncSettings)MemberwiseClone();

			public GBAHawkLink_Debug_SyncSettings() => SettingsUtil.SetDefaultValues(this);

			public static bool NeedsReboot(GBAHawkLink_Debug_SyncSettings x, GBAHawkLink_Debug_SyncSettings y)
			{
				return !DeepEquality.DeepEquals(x, y);
			}
		}
	}
}
