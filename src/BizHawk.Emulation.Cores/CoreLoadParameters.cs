using System.Collections.Generic;
using BizHawk.Emulation.Common;
using System;

namespace BizHawk.Emulation.Cores
{
	[AttributeUsage(AttributeTargets.Constructor, AllowMultiple = true)]
	public sealed class CoreConstructorAttribute : Attribute
	{
		public string System { get; }
		public CoreConstructorAttribute(string system)
		{
			System = system;
		}
	}

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
	}
}
