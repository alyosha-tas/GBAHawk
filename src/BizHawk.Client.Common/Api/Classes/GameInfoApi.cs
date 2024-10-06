﻿#nullable enable

using System;
using System.Collections.Generic;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.Common
{
	[Obsolete("use IEmulationApi")]
	public sealed class GameInfoApi : IGameInfoApi
	{
		private readonly IGameInfo? _game;

		public GameInfoApi(IGameInfo? game)
			=> _game = game;

		public IGameInfo? GetGameInfo()
			=> _game;

		public IReadOnlyDictionary<string, string?> GetOptions()
		{
			var options = new Dictionary<string, string?>();
			if (_game == null) return options;
			foreach (var (k, v) in ((GameInfo) _game).GetOptions()) options[k] = v;
			return options;
		}
	}
}
