using BizHawk.Client.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.GBAHawk
{
	public interface IMainFormForConfig : IDialogParent
	{
		/// <remarks>only referenced from <see cref="GenericCoreConfig"/></remarks>
		IEmulator Emulator { get; }

		void PutCoreSettings(object o);

		void PutCoreSyncSettings(object o);
	}
}
