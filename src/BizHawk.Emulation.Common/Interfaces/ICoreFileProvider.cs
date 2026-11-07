namespace BizHawk.Emulation.Common
{
	/// <summary>
	/// Defines the means by which firmware, bios and other necessary files are provided to a core that needs them
	/// </summary>
	public interface ICoreFileProvider
	{
		/// <summary>
		/// produces a path that contains emulation related DLL and exe files
		/// </summary>
		string DllPath();

		/// <param name="msg">warning message to show on failure</param>
		/// <returns><see langword="null"/> iff failed</returns>
		byte[]? GetFirmware(FirmwareID id, string? msg = null);

		/// <param name="msg">exception message to show on failure</param>
		/// <exception cref="MissingFirmwareException">if not found</exception>
		byte[] GetFirmwareOrThrow(FirmwareID id, string? msg = null);

		/// <param name="msg">exception message to show on failure</param>
		/// <exception cref="MissingFirmwareException">if not found</exception>
		/// <remarks>only used in PCEHawk</remarks>
		(byte[] FW, GameInfo Game) GetFirmwareWithGameInfoOrThrow(FirmwareID id, string? msg = null);
	}
}
