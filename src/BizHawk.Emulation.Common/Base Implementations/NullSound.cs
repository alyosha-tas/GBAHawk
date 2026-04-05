using System;

namespace BizHawk.Emulation.Common
{
	/// <summary>
	/// A default and empty implementation of ISoundProvider
	/// that simply outputs "silence"
	/// </summary>
	/// <seealso cref="ISoundProvider" />
	public class NullSound : ISoundProvider
	{
		private readonly long _spfNumerator;
		private readonly long _spfDenominator;
		private long _remainder;
		private short[] _buff = Array.Empty<short>();

		private NullSound()
		{

		}

		/// <summary>
		/// Initializes a new instance of the <see cref="NullSound"/> class
		/// that provides an exact number of audio samples per call when in sync mode
		/// </summary>
		public NullSound(int spf)
			: this()
		{
			_spfNumerator = spf;
			_spfDenominator = 1;
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="NullSound"/> class
		/// that exactly matches a given frame rate when in sync mode
		/// </summary>
		public NullSound(long fpsNum, long fpsDen)
		{
			_spfNumerator = fpsDen * 44100;
			_spfDenominator = fpsNum;
		}

		public void GetSamplesSync(out short[] samples, out int nsamp)
		{
			int s = (int)((_spfNumerator + _remainder) / _spfDenominator);
			_remainder = (_spfNumerator + _remainder) % _spfDenominator;

			if (_buff.Length < s * 2)
			{
				_buff = new short[s * 2];
			}

			samples = _buff;
			nsamp = s;
		}

		public void DiscardSamples()
		{
		}
	}
}
