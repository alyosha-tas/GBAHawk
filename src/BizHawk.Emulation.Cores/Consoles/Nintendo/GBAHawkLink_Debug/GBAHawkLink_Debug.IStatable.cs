using System.IO;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	public partial class GBAHawkLink_Debug : IStatable
	{
		private readonly IStatable _lStates;
		private readonly IStatable _rStates;

		public void SaveStateBinary(BinaryWriter bw)
		{
			_lStates.SaveStateBinary(bw);
			_rStates.SaveStateBinary(bw);
			// other variables
			SyncState(new Serializer(bw));
		}

		public void LoadStateBinary(BinaryReader br)
		{
			_lStates.LoadStateBinary(br);
			_rStates.LoadStateBinary(br);
			// other variables
			SyncState(new Serializer(br));
		}

		private void SyncState(Serializer ser)
		{
			ser.Sync("Lag", ref Lag_Count);
			ser.Sync("Frame", ref _frame);
			ser.Sync("IsLag", ref Is_Lag);
			ser.Sync(nameof(_cableconnected), ref _cableconnected);
			ser.Sync(nameof(_cablediscosignal), ref _cablediscosignal);
			ser.Sync(nameof(do_r_next), ref do_r_next);
			_controllerDeck.SyncState(ser);

			if (ser.IsReader)
			{
				FillVideoBuffer();
			}
		}
	}
}
