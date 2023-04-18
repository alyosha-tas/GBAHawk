#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

using BizHawk.Common;
using BizHawk.Common.StringExtensions;

namespace BizHawk.Client.GBAHawk
{
	public static class ReflectionCache
	{
		private const string EMBED_PREFIX = "BizHawk.Client.GBAHawk.";

		private static Type[]? _types = null;

		private static readonly Assembly Asm = typeof(BizHawk.Client.GBAHawk.ReflectionCache).Assembly;

		public static readonly Version AsmVersion = Asm.GetName().Version!;

		public static Type[] Types => _types ??= Asm.GetTypesWithoutLoadErrors().ToArray();

		public static IEnumerable<string> EmbeddedResourceList(string extraPrefix)
		{
			var fullPrefix = EMBED_PREFIX + extraPrefix;
			return Asm.GetManifestResourceNames().Where(s => s.StartsWith(fullPrefix)).Select(s => s.RemovePrefix(fullPrefix));
		}

		public static IEnumerable<string> EmbeddedResourceList()
			=> EmbeddedResourceList(string.Empty);

		/// <exception cref="ArgumentException">not found</exception>
		public static Stream EmbeddedResourceStream(string embedPath)
		{
			var fullPath = EMBED_PREFIX + embedPath;
			return Asm.GetManifestResourceStream(fullPath) ?? throw new ArgumentException($"resource at {fullPath} not found", nameof(embedPath));
		}
	}
}
