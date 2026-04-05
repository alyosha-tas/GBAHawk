using System;

namespace BizHawk.Emulation.Common
{
	/// <summary>
	/// This service provides the ability to output sound from the client,
	/// If available the client will provide sound output
	/// If unavailable the client will fallback to a default sound implementation
	/// that generates empty samples (silence)
	/// </summary>
	public interface ISoundProvider : IEmulatorService
	{
		/// <summary>
		/// Provides samples in sync mode
		/// If the core is not in sync mode, this should throw an InvalidOperationException
		/// </summary>
		/// <exception cref="InvalidOperationException"></exception>
		void GetSamplesSync(out short[] samples, out int nsamp);

		/// <summary>
		/// Discards stuff, is there anything more to say here?
		/// </summary>
		void DiscardSamples();
	}
}
