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

			[DisplayName("Audio Selection")]
			[Description("Choose Audio Source. Both will produce Stereo sound.")]
			[DefaultValue(AudioSrc.Left)]
			public AudioSrc AudioSet { get; set; }

			public GBAHawkLink_Debug_Settings Clone() => (GBAHawkLink_Debug_Settings)MemberwiseClone();

			public GBAHawkLink_Debug_Settings() => SettingsUtil.SetDefaultValues(this);
		}

		public class GBAHawkLink_Debug_SyncSettings
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
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public short RTCOffset_L
			{
				get => _RTCOffset_L;
				set => _RTCOffset_L = value;
			}

			[DisplayName("RTC Offset R")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public short RTCOffset_R
			{
				get => _RTCOffset_R;
				set => _RTCOffset_R = value;
			}

			[DisplayName("Use Existing SaveRAM")]
			[Description("(Intended for development, for regular use leave as true.) When true, existing SaveRAM will be loaded at boot up.")]
			[DefaultValue(false)]
			public bool Use_SRAM { get; set; }

			[JsonIgnore]
			private DateTime _RTCInitialTime_L;
			[JsonIgnore]
			private DateTime _RTCInitialTime_R;
			[JsonIgnore]
			private short _RTCOffset_L;
			[JsonIgnore]
			private short _RTCOffset_R;

			public GBAHawkLink_Debug_SyncSettings Clone() => (GBAHawkLink_Debug_SyncSettings)MemberwiseClone();

			public GBAHawkLink_Debug_SyncSettings() => SettingsUtil.SetDefaultValues(this);

			public static bool NeedsReboot(GBAHawkLink_Debug_SyncSettings x, GBAHawkLink_Debug_SyncSettings y)
			{
				return !DeepEquality.DeepEquals(x, y);
			}
		}
	}
}
