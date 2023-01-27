using System.Collections.Generic;
using System.Linq;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.NES
{
	public partial class NES : ISettable<NES.NESSettings, NES.NESSyncSettings>
	{
		public NESSettings GetSettings() => Settings.Clone();

		public NESSyncSettings GetSyncSettings() => SyncSettings.Clone();

		public PutSettingsDirtyBits PutSettings(NESSettings o)
		{
			Settings = o;
			if (Settings.ClipLeftAndRight)
			{
				videoProvider.left = 0;
				videoProvider.right = 255;
			}
			else
			{
				videoProvider.left = 0;
				videoProvider.right = 255;
			}

			SetPalette(Settings.Palette);

			apu.m_vol = Settings.APU_vol;

			return PutSettingsDirtyBits.None;
		}

		public PutSettingsDirtyBits PutSyncSettings(NESSyncSettings o)
		{
			bool ret = NESSyncSettings.NeedsReboot(SyncSettings, o);
			SyncSettings = o;
			return ret ? PutSettingsDirtyBits.RebootCore : PutSettingsDirtyBits.None;
		}

		internal NESSettings Settings = new NESSettings();
		internal NESSyncSettings SyncSettings = new NESSyncSettings();

		public class NESSyncSettings
		{
			public Dictionary<string, string> BoardProperties = new Dictionary<string, string>();

			public enum Region
			{
				Default,
				NES,
				FAM
			}

			public Region RegionOverride = Region.NES;

			public NESControlSettings Controls = new NESControlSettings();

			public List<byte> InitialWRamStatePattern = null;

			public NESSyncSettings Clone()
			{
				var ret = (NESSyncSettings)MemberwiseClone();
				ret.BoardProperties = new Dictionary<string, string>(BoardProperties);
				ret.Controls = Controls.Clone();
				return ret;
			}

			public static bool NeedsReboot(NESSyncSettings x, NESSyncSettings y)
			{
				return !(Util.DictionaryEqual(x.BoardProperties, y.BoardProperties)
					&& x.RegionOverride == y.RegionOverride
					&& !NESControlSettings.NeedsReboot(x.Controls, y.Controls)
					&& ((x.InitialWRamStatePattern ?? new List<byte>()).SequenceEqual(y.InitialWRamStatePattern ?? new List<byte>())));
			}
		}

		public class NESSettings
		{
			public bool ClipLeftAndRight = false;
			public bool DispBackground = true;
			public bool DispSprites = true;
			public int BackgroundColor = 0;

			public int NTSC_TopLine = 0;
			public int NTSC_BottomLine = 239;
			public int PAL_TopLine = 0;
			public int PAL_BottomLine = 239;

			public byte[,] Palette;

			public int APU_vol = 1;

			public NESSettings Clone()
			{
				var ret = (NESSettings)MemberwiseClone();
				ret.Palette = (byte[,])ret.Palette.Clone();
				return ret;
			}

			public NESSettings()
			{
				Palette = (byte[,])Palettes.QuickNESPalette.Clone();
			}

			[Newtonsoft.Json.JsonConstructor]
			public NESSettings(byte[,] Palette)
			{
				if (Palette == null)
					// only needed for SVN purposes
					// edit: what does this mean?
					this.Palette = (byte[,])Palettes.QuickNESPalette.Clone();
				else
					this.Palette = Palette;
			}
		}
	}
}
