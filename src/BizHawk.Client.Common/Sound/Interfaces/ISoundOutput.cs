﻿using System;

namespace BizHawk.Client.Common
{
	public interface ISoundOutput : IDisposable
	{
		void StartSound();
		void StopSound();
		void ApplyVolumeSettings(double volume);
		int MaxSamplesDeficit { get; }
		int CalculateSamplesNeeded();
		void WriteSamples(short[] samples, int sampleOffset, int sampleCount);
	}
}
