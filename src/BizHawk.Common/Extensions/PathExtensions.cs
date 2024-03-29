﻿using System;
using System.Text;
using System.IO;
using System.Reflection;
using System.Linq;

using BizHawk.Common.StringExtensions;

namespace BizHawk.Common.PathExtensions
{
	public static partial class PathExtensions
	{
		/// <returns><see langword="true"/> iff <paramref name="childPath"/> indicates a child of <paramref name="parentPath"/>, with <see langword="false"/> being returned if either path is <see langword="null"/></returns>
		/// <remarks>algorithm for Windows taken from https://stackoverflow.com/a/7710620/7467292</remarks>
		public static bool IsSubfolderOf(this string? childPath, string? parentPath)
		{
			if (childPath == null || parentPath == null) return false;
			if (childPath == parentPath || childPath.StartsWith($"{parentPath}{Path.DirectorySeparatorChar}")) return true;

			var parentUri = new Uri(parentPath.RemoveSuffix(Path.DirectorySeparatorChar));
			for (var childUri = new DirectoryInfo(childPath); childUri != null; childUri = childUri.Parent)
			{
				if (new Uri(childUri.FullName) == parentUri) return true;
			}
			return false;
		}

		/// <returns><see langword="true"/> iff absolute (OS-dependent)</returns>
		/// <seealso cref="IsRelative"/>
		public static bool IsAbsolute(this string path)
		{
			return PathInternal.IsPathFullyQualified(path);
		}

		/// <returns><see langword="false"/> iff absolute (OS-dependent)</returns>
		/// <remarks>that means it may return <see langword="true"/> for invalid paths</remarks>
		/// <seealso cref="IsAbsolute"/>
		public static bool IsRelative(this string path) => !path.IsAbsolute();

		/// <exception cref="ArgumentException">running on Windows host, and unmanaged call failed</exception>
		/// <exception cref="FileNotFoundException">running on Windows host, and either path is not a regular file or directory</exception>
		/// <remarks>
		/// always returns a relative path, even if it means going up first<br/>
		/// algorithm for Windows taken from https://stackoverflow.com/a/485516/7467292<br/>
		/// the parameter names seem backwards, but those are the names used in the Win32 API we're calling
		/// </remarks>
		public static string? GetRelativePath(string? fromPath, string? toPath)
		{
			if (fromPath == null || toPath == null) return null;

			//TODO merge this with the Windows implementation in MakeRelativeTo
			static FileAttributes GetPathAttribute(string path1)
			{
				var di = new DirectoryInfo(path1.Split('|').First());
				if (di.Exists)
				{
					return FileAttributes.Directory;
				}

				var fi = new FileInfo(path1.Split('|').First());
				if (fi.Exists)
				{
					return FileAttributes.Normal;
				}

				throw new FileNotFoundException();
			}
			var path = new StringBuilder(260 /* = MAX_PATH */);
			return Win32Imports.PathRelativePathTo(path, fromPath, GetPathAttribute(fromPath), toPath, GetPathAttribute(toPath))
				? path.ToString()
				: throw new ArgumentException("Paths must have a common prefix");
		}

		/// <returns>absolute path (OS-dependent) equivalent to <paramref name="path"/></returns>
		/// <remarks>
		/// unless <paramref name="cwd"/> is given, uses <see cref="CWDHacks.Get">CWDHacks.Get</see>/<see cref="Environment.CurrentDirectory">Environment.CurrentDirectory</see>,
		/// so take care when calling this after startup
		/// </remarks>
		public static string MakeAbsolute(this string path, string? cwd = null)
		{
			if (path.IsAbsolute())
				return path;
			else
			{
				// FileInfo for normalisation ("C:\a\b\..\c" => "C:\a\c")
				var mycwd = cwd ?? (CWDHacks.Get());
				var finalpath = $"{mycwd}/{path}";
				var fi = new FileInfo(finalpath);
				return fi.FullName;
			}
		}

		/// <returns>the absolute path equivalent to <paramref name="path"/> which contains <c>%exe%</c> (expanded) as a prefix</returns>
		/// <remarks>
		/// returned string omits trailing slash<br/>
		/// note that the returned string is an absolute path and not a relative path; but TODO it was intended to be relative
		/// </remarks>
		public static string MakeProgramRelativePath(this string path) => Path.Combine(PathUtils.ExeDirectoryPath, path);

		/// <returns>the relative path which is equivalent to <paramref name="absolutePath"/> when the CWD is <paramref name="basePath"/>, or <see langword="null"/> if either path is <see langword="null"/></returns>
		/// <remarks>
		/// only returns a relative path if <paramref name="absolutePath"/> is a child of <paramref name="basePath"/> (uses <see cref="IsSubfolderOf"/>), otherwise returns <paramref name="absolutePath"/><br/>
		/// returned string omits trailing slash
		/// </remarks>
		public static string? MakeRelativeTo(this string? absolutePath, string? basePath)
		{
			if (absolutePath == null || basePath == null) return null;
			if (!absolutePath.IsSubfolderOf(basePath)) return absolutePath;
			return absolutePath.Replace(basePath, ".").RemoveSuffix(Path.DirectorySeparatorChar);
		}

		/// <returns><see langword="false"/> iff <paramref name="path"/> is blank, or is <c>"."</c> (relative path to CWD), regardless of trailing slash</returns>
		public static bool PathIsSet(this string path) => !string.IsNullOrWhiteSpace(path) && path != "." && path != "./" && path != ".\\";

		public static string RemoveInvalidFileSystemChars(this string name) => string.Concat(name.Split(Path.GetInvalidFileNameChars()));
	}

	public static class PathUtils
	{
		/// <returns>absolute path of the dll dir (sibling of EmuHawk.exe)</returns>
		/// <remarks>returned string omits trailing slash</remarks>
		public static readonly string DllDirectoryPath;

		/// <returns>absolute path of the parent dir of DiscoHawk.exe/EmuHawk.exe, commonly referred to as <c>%exe%</c> though none of our code adds it to the environment</returns>
		/// <remarks>returned string omits trailing slash</remarks>
		public static readonly string ExeDirectoryPath;

		static PathUtils()
		{
			var dirPath = Path.GetDirectoryName(Assembly.GetEntryAssembly()?.Location);
			ExeDirectoryPath = string.IsNullOrEmpty(dirPath) ? throw new Exception("failed to get location of executable, very bad things must have happened") : dirPath.RemoveSuffix('\\');
			DllDirectoryPath = Path.Combine(false && ExeDirectoryPath == string.Empty ? "/" : ExeDirectoryPath, "dll");
			// yes, this is a lot of extra code to make sure BizHawk can run in `/` on Unix, but I've made up for it by caching these for the program lifecycle --yoshi
		}
	}
}
