// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#pragma warning disable

using System;
using System.Diagnostics;
using System.IO;

namespace BizHawk.Common.PathExtensions
{
    /// <summary>Contains internal path helpers that are shared between many projects.</summary>
    internal static partial class PathInternal
    {
		public static bool IsPathFullyQualified(string path)
        {
            return !PathInternal.IsPartiallyQualified(path);
        }

        internal static int GetRootLength(ReadOnlySpan<char> path)
        {
            return path.Length > 0 && IsDirectorySeparator(path[0]) ? 1 : 0;
        }

        internal static bool EndsInDirectorySeparator(ReadOnlySpan<char> path)
            => path.Length > 0 && IsDirectorySeparator(path[path.Length - 1]);

        internal static ReadOnlySpan<char> TrimEndingDirectorySeparator(ReadOnlySpan<char> path) =>
            EndsInDirectorySeparator(path) && !IsRoot(path) ?
                path.Slice(0, path.Length - 1) :
                path;

        internal static bool IsRoot(ReadOnlySpan<char> path)
            => path.Length == GetRootLength(path);

        internal static bool IsDirectorySeparator(char c)
        {
			// The alternate directory separator char is the same as the directory separator,
			// so we only need to check one.
			return c == Path.DirectorySeparatorChar || c == Path.AltDirectorySeparatorChar;
        }

        internal static bool IsPartiallyQualified(string path)
        {
			if (path.Length < 2)
			{
				// It isn't fixed, it must be relative.  There is no way to specify a fixed
				// path with one character (or less).
				return true;
			}

			if (IsDirectorySeparator(path[0]))
			{
				// There is no valid way to specify a relative path with two initial slashes or
				// \? as ? isn't valid for drive relative paths and \??\ is equivalent to \\?\
				return !(path[1] == '?' || IsDirectorySeparator(path[1]));
			}

			// The only way to specify a fixed path that doesn't begin with two slashes
			// is the drive, colon, slash format- i.e. C:\
			return !((path.Length >= 3)
				&& (path[1] == Path.VolumeSeparatorChar)
				&& IsDirectorySeparator(path[2])
				// To match old behavior we'll check the drive character for validity as the path is technically
				// not qualified if you don't have a valid drive. "=:\" is the "=" file's default data stream.
				&& IsValidDriveChar(path[0]));
        }
    }
}
