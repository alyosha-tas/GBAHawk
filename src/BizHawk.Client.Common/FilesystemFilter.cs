#nullable enable

using System.Collections.Generic;
using System.Linq;

using BizHawk.Common;

namespace BizHawk.Client.Common
{
	public sealed class FilesystemFilter
	{
		private string? _ser = null;

		public readonly string Description;

		public readonly IReadOnlyCollection<string> Extensions;

		public FilesystemFilter(string description, IReadOnlyCollection<string> extensions, bool addArchiveExts = false)
		{
			Description = description;
			Extensions = addArchiveExts ? extensions.Concat(ArchiveExtensions).ToList() : extensions;
		}

		public FilesystemFilter(
			string description,
			IReadOnlyCollection<string> extensions,
			IReadOnlyCollection<string> devBuildExtraExts,
			bool addArchiveExts = false,
			bool devBuildAddArchiveExts = false)
		{
			Description = description;

			Extensions = addArchiveExts || devBuildAddArchiveExts
				? extensions.Concat(devBuildExtraExts).Concat(ArchiveExtensions).ToList()
				: extensions.Concat(devBuildExtraExts).ToList();
		}

		/// <summary>delegated to <see cref="SerializeEntry"/></summary>
		/// <remarks>return value is a valid <c>Filter</c> for <c>Save-</c>/<c>OpenFileDialog</c></remarks>
		public override string ToString() => _ser ??= SerializeEntry(Description, Extensions);

		public const string AllFilesEntry = "All Files|*";

		public static readonly IReadOnlyCollection<string> ArchiveExtensions = new[] { "zip", "rar", "7z", "gz" };

		public static readonly FilesystemFilter Archives = new FilesystemFilter("Archives", ArchiveExtensions);

		public static readonly FilesystemFilter BizHawkMovies = new FilesystemFilter("Movie Files", new[] { MovieService.StandardMovieExtension });

		public static readonly FilesystemFilter EmuHawkSaveStates = new FilesystemFilter("Save States", new[] { "State" });

		public static readonly FilesystemFilter LuaScripts = new FilesystemFilter("Lua Scripts", new[] { "lua" });

		public static readonly FilesystemFilter Palettes = new FilesystemFilter("Palette Files", new[] { "pal" });

		public static readonly FilesystemFilter PNGs = new FilesystemFilter("PNG Files", new[] { "png" });

		public static readonly FilesystemFilter TAStudioProjects = new FilesystemFilter("TAS Project Files", new[] { MovieService.TasMovieExtension });

		public static readonly FilesystemFilter TextFiles = new FilesystemFilter("Text Files", new[] { "txt" });

		/// <remarks>return value is a valid <c>Filter</c> for <c>Save-</c>/<c>OpenFileDialog</c></remarks>
		public static string SerializeEntry(string desc, IReadOnlyCollection<string> exts)
		{
			var joinedLower = string.Join(";", exts.Select(static ext => $"*.{ext}"));
			return $"{desc} ({joinedLower})|{joinedLower}";
		}
	}
}
