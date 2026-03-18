using System;
using System.ComponentModel;

using Newtonsoft.Json;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	public partial class GBHawkLink : ISettable<GBHawkLink.GBLinkSettings, GBHawkLink.GBLinkSyncSettings>
	{
		public GBLinkSettings GetSettings()
		{
			return Settings.Clone();
		}

		public GBLinkSyncSettings GetSyncSettings()
		{
			return SyncSettings.Clone();
		}

		public PutSettingsDirtyBits PutSettings(GBLinkSettings o)
		{
			bool ret = GBLinkSettings.RebootNeeded(Settings, o);
			Settings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(GBLinkSyncSettings o)
		{
			bool ret = GBLinkSyncSettings.RebootNeeded(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal GBLinkSettings Settings { get; private set; }
		internal GBLinkSyncSettings SyncSettings { get; private set; }

		public class GBLinkSettings
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

			public enum PaletteType
			{
				BW,
				Gr
			}

			[DisplayName("Color Mode Console A")]
			[Description("Pick Between Green scale and Grey scale colors")]
			[DefaultValue(PaletteType.BW)]
			public PaletteType A_Palette { get; set; }

			[DisplayName("Color Mode Console B")]
			[Description("Pick Between Green scale and Grey scale colors")]
			[DefaultValue(PaletteType.BW)]
			public PaletteType B_Palette { get; set; }

			[DisplayName("Color Mode Console C")]
			[Description("Pick Between Green scale and Grey scale colors")]
			[DefaultValue(PaletteType.BW)]
			public PaletteType C_Palette { get; set; }

			[DisplayName("Color Mode Console D")]
			[Description("Pick Between Green scale and Grey scale colors")]
			[DefaultValue(PaletteType.BW)]
			public PaletteType D_Palette { get; set; }

			public enum Cycle_Return
			{
				CPU,
				GBI
			}

			[DisplayName("Read Domains on VBlank Console A")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool A_VBL_sync { get; set; }

			[DisplayName("Read Domains on VBlank Console B")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool B_VBL_sync { get; set; }

			[DisplayName("Read Domains on VBlank Console C")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool C_VBL_sync { get; set; }

			[DisplayName("Read Domains on VBlank Console D")]
			[Description("When true, memory domains are only updated on VBlank. More consistent for LUA. NOTE: Does not work for system bus, does not apply to writes.")]
			[DefaultValue(false)]
			public bool D_VBL_sync { get; set; }

			[DisplayName("TotalExecutedCycles Return Value")]
			[Description("CPU returns the actual CPU cycles executed, GBI returns the values needed for console verification")]
			[DefaultValue(Cycle_Return.CPU)]
			public Cycle_Return cycle_return_setting { get; set; }

			public GBLinkSettings Clone()
			{
				return (GBLinkSettings)MemberwiseClone();
			}

			public GBLinkSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(GBLinkSettings x, GBLinkSettings y)
			{
				return false;
			}
		}

		public class GBLinkSyncSettings
		{
			public enum ConsoleModeType
			{
				GB,
				GBC
			}

			[DisplayName("Console Mode A")]
			[Description("Physical device used.")]
			[DefaultValue(ConsoleModeType.GBC)]
			public ConsoleModeType A_ConsoleMode { get; set; }

			[DisplayName("Console Mode D")]
			[Description("Physical device used.")]
			[DefaultValue(ConsoleModeType.GBC)]
			public ConsoleModeType B_ConsoleMode { get; set; }

			[DisplayName("Console Mode C")]
			[Description("Physical device used.")]
			[DefaultValue(ConsoleModeType.GBC)]
			public ConsoleModeType C_ConsoleMode { get; set; }

			[DisplayName("Console Mode D")]
			[Description("Physical device used.")]
			[DefaultValue(ConsoleModeType.GBC)]
			public ConsoleModeType D_ConsoleMode { get; set; }

			[DisplayName("CGB in GBA, Console A")]
			[Description("Emulate GBA hardware running a CGB game, instead of CGB hardware.")]
			[DefaultValue(false)]
			public bool A_GBACGB { get; set; }

			[DisplayName("CGB in GBA, Console B")]
			[Description("Emulate GBA hardware running a CGB game, instead of CGB hardware.")]
			[DefaultValue(false)]
			public bool B_GBACGB { get; set; }

			[DisplayName("CGB in GBA, Console C")]
			[Description("Emulate GBA hardware running a CGB game, instead of CGB hardware.")]
			[DefaultValue(false)]
			public bool C_GBACGB { get; set; }

			[DisplayName("CGB in GBA, Console D")]
			[Description("Emulate GBA hardware running a CGB game, instead of CGB hardware.")]
			[DefaultValue(false)]
			public bool D_GBACGB { get; set; }

			[DisplayName("RTC Initial Time Console A")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int A_RTCInitialTime
			{
				get => A_RTCIT;
				set => A_RTCIT = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Initial Time Console B")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int B_RTCInitialTime
			{
				get => B_RTCIT;
				set => B_RTCIT = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Initial Time Console C")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int C_RTCInitialTime
			{
				get => C_RTCIT;
				set => C_RTCIT = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Initial Time Console D")]
			[Description("Set the initial RTC time in terms of elapsed seconds.")]
			[DefaultValue(0)]
			public int D_RTCInitialTime
			{
				get => D_RTCIT;
				set => D_RTCIT = Math.Max(0, Math.Min(1024 * 24 * 60 * 60, value));
			}

			[DisplayName("RTC Offset Console A")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int A_RTCOffset
			{
				get => A_RTCOfst;
				set => A_RTCOfst = Math.Max(-127, Math.Min(127, value));
			}

			[DisplayName("RTC Offset Console B")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int B_RTCOffset
			{
				get => B_RTCOfst;
				set => B_RTCOfst = Math.Max(-127, Math.Min(127, value));
			}

			[DisplayName("RTC Offset Console C")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int C_RTCOffset
			{
				get => C_RTCOfst;
				set => C_RTCOfst = Math.Max(-127, Math.Min(127, value));
			}

			[DisplayName("RTC Offset Console D")]
			[Description("Set error in RTC clocking (-127 to 127)")]
			[DefaultValue(0)]
			public int D_RTCOffset
			{
				get => D_RTCOfst;
				set => D_RTCOfst = Math.Max(-127, Math.Min(127, value));
			}

			[JsonIgnore]
			private int A_RTCIT;
			[JsonIgnore]
			private int B_RTCIT;
			[JsonIgnore]
			private int C_RTCIT;
			[JsonIgnore]
			private int D_RTCIT;

			[JsonIgnore]
			private int A_RTCOfst;
			[JsonIgnore]
			private int B_RTCOfst;
			[JsonIgnore]
			private int C_RTCOfst;
			[JsonIgnore]
			private int D_RTCOfst;

			[JsonIgnore]
			public ushort A_DivIT = 8;
			[JsonIgnore]
			public ushort B_DivIT = 8;
			[JsonIgnore]
			public ushort C_DivIT = 8;
			[JsonIgnore]
			public ushort D_DivIT = 8;

			[DisplayName("One Screen Mode")]
			[Description("Only display one console, based on setting selection.)")]
			[DefaultValue(false)]
			public bool OneScreenMode { get; set; }

			public GBLinkSyncSettings Clone()
			{
				return (GBLinkSyncSettings)MemberwiseClone();
			}

			public GBLinkSyncSettings()
			{
				SettingsUtil.SetDefaultValues(this);
			}

			public static bool RebootNeeded(GBLinkSyncSettings x, GBLinkSyncSettings y)
			{
				return true;
			}
		}
	}
}
