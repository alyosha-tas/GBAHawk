using System.Collections.Generic;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores
{
	public interface IRomAsset
	{
		byte[] RomData { get; }
		byte[] FileData { get; }
		string Extension { get; }
		public string RomPath { get; }
		/// <summary>
		/// GameInfo for this individual asset.  Doesn't make sense a lot of the time;
		/// only use this if your individual rom assets are full proper games when considered alone.
		/// Not guaranteed to be set in any other situation.
		/// </summary>
		GameInfo Game { get; }
	}

	public class CoreLoadParameters<TSettiing, TSync>
	{
		public CoreComm Comm { get; set; }
		public GameInfo Game { get; set; }
		/// <summary>
		/// Settings previously returned from the core.  May be null.
		/// </summary>
		public TSettiing Settings { get; set; }
		/// <summary>
		/// Sync Settings previously returned from the core.  May be null.
		/// </summary>
		public TSync SyncSettings { get; set; }
		/// <summary>
		/// All roms that should be loaded as part of this core load.
		/// Order may be significant.  Does not include firmwares or other general resources.
		/// </summary>
		public List<IRomAsset> Roms { get; set; } = new List<IRomAsset>();

		public bool DeterministicEmulationRequested { get; set; }
	}
}
