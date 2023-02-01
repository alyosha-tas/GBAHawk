using System;
using System.IO;
using System.Collections.Generic;

namespace BizHawk.Emulation.Cores
{
	/// <summary>
	/// Each of these should ideally represent a single file type.
	/// However for now they just may resemble a console, and a core would know how to parse some set of those after making its own determination.
	/// If formats are very similar but with small differences, and that determination can be made, then it will be in the ExtraInfo in the FileIDResult
	/// </summary>
	public enum FileIDType
	{
		None, 
		Multiple, //don't think this makes sense. shouldn't the multiple options be returned?

		Disc, //an unknown disc

		GB, GBC, GBA,
	}

	public class FileIDResult
	{
		public FileIDResult()
		{
		}

		public FileIDResult(FileIDType type, int confidence)
		{
			FileIDType = type;
			Confidence = confidence;
		}

		public FileIDResult(FileIDType type)
		{
			FileIDType = type;
		}

		/// <summary>
		/// a percentage between 0 and 100 assessing the confidence of this result
		/// </summary>
		public int Confidence;

		public FileIDType FileIDType;

		/// <summary>
		/// extra information which could be easily gotten during the file ID (region, suspected homebrew, CRC invalid, etc.)
		/// </summary>
		public readonly IDictionary<string, object> ExtraInfo = new Dictionary<string, object>();
	}

	public class FileIDResults : List<FileIDResult>
	{
		public FileIDResults() { }
		public FileIDResults(FileIDResult item)
		{
			base.Add(item);
		}
		public new void Sort()
		{
			base.Sort((x, y) => x.Confidence.CompareTo(y.Confidence));
		}

		/// <summary>
		/// indicates whether the client should try again after mounting the disc image for further inspection
		/// </summary>
		public bool ShouldTryDisc;
	}

	public class FileID
	{
		/// <summary>
		/// parameters for an Identify job
		/// </summary>
		public class IdentifyParams
		{
			/// <summary>
			/// The extension of the original file (with or without the .)
			/// </summary>
			public string Extension;

			/// <summary>
			/// a seekable stream which can be used
			/// </summary>
			public Stream SeekableStream;
		}

		private class IdentifyJob
		{
			public Stream Stream;
			public string Extension;
		}

		/// <summary>
		/// performs wise heuristics to identify a file.
		/// this will attempt to return early if a confident result can be produced.
		/// </summary>
		public FileIDResults Identify(IdentifyParams p)
		{
			IdentifyJob job = new IdentifyJob() { 
				Stream = p.SeekableStream
			};

			FileIDResults ret = new FileIDResults();

			string ext = p.Extension;
			if(ext != null)
			{
				ext = ext.TrimStart('.').ToUpper();
				job.Extension = ext;
			}

			if (job.Extension == "CUE")
			{
				ret.ShouldTryDisc = true;
				return ret;
			}

			if(job.Extension != null)
			{
				//first test everything associated with this extension
				if (ExtensionHandlers.TryGetValue(ext, out var handler))
				{
					foreach (var del in handler.Testers)
					{
						var fidr = del(job);
						if (fidr.FileIDType == FileIDType.None)
							continue;
						ret.Add(fidr);
					}

					ret.Sort();

					//add a low confidence result just based on extension, if it doesnt exist
					if(ret.Find( (x) => x.FileIDType == handler.DefaultForExtension) == null)
					{
						var fidr = new FileIDResult(handler.DefaultForExtension, 5);
						ret.Add(fidr);
					}
				}
			}

			ret.Sort();

			//if we didnt find anything high confidence, try all the testers (TODO)

			return ret;
		}

		/// <summary>
		/// performs wise heuristics to identify a file (simple version)
		/// </summary>
		public FileIDType IdentifySimple(IdentifyParams p)
		{
			var ret = Identify(p);
			if (ret.ShouldTryDisc)
				return FileIDType.Disc;
			if (ret.Count == 0)
				return FileIDType.None;
			else if(ret.Count == 1)
				return ret[0].FileIDType;
			else if (ret[0].Confidence == ret[1].Confidence)
				return FileIDType.Multiple;
			else return ret[0].FileIDType;
		}

		private class SimpleMagicRecord
		{
			public int Offset;
			public string Key;
			public int Length = -1;
		}

		//some of these (NES, UNIF for instance) should be lower confidence probably...
		//if you change some of the Length arguments for longer keys, please make notes about why
		private static class SimpleMagics
		{
			//the GBA nintendo logo.. we'll only use 16 bytes of it but theyre all here, for reference
			//we cant expect these roms to be normally sized, but we may be able to find other features of the header to use for extra checks
			public static readonly SimpleMagicRecord GBA = new SimpleMagicRecord {  Offset = 4, Length = 16, Key = "\x24\xFF\xAE\x51\x69\x9A\xA2\x21\x3D\x84\x82\x0A\x84\xE4\x09\xAD\x11\x24\x8B\x98\xC0\x81\x7F\x21\xA3\x52\xBE\x19\x93\x09\xCE\x20\x10\x46\x4A\x4A\xF8\x27\x31\xEC\x58\xC7\xE8\x33\x82\xE3\xCE\xBF\x85\xF4\xDF\x94\xCE\x4B\x09\xC1\x94\x56\x8A\xC0\x13\x72\xA7\xFC\x9F\x84\x4D\x73\xA3\xCA\x9A\x61\x58\x97\xA3\x27\xFC\x03\x98\x76\x23\x1D\xC7\x61\x03\x04\xAE\x56\xBF\x38\x84\x00\x40\xA7\x0E\xFD\xFF\x52\xFE\x03\x6F\x95\x30\xF1\x97\xFB\xC0\x85\x60\xD6\x80\x25\xA9\x63\xBE\x03\x01\x4E\x38\xE2\xF9\xA2\x34\xFF\xBB\x3E\x03\x44\x78\x00\x90\xCB\x88\x11\x3A\x94\x65\xC0\x7C\x63\x87\xF0\x3C\xAF\xD6\x25\xE4\x8B\x38\x0A\xAC\x72\x21\xD4\xF8\x07" };

			public static readonly SimpleMagicRecord GB = new SimpleMagicRecord {  Offset=0x104, Length = 16, Key = "\xCE\xED\x66\x66\xCC\x0D\x00\x0B\x03\x73\x00\x83\x00\x0C\x00\x0D\x00\x08\x11\x1F\x88\x89\x00\x0E\xDC\xCC\x6E\xE6\xDD\xDD\xD9\x99\xBB\xBB\x67\x63\x6E\x0E\xEC\xCC\xDD\xDC\x99\x9F\xBB\xB9\x33\x3E" };
		}

		private class ExtensionInfo
		{
			public ExtensionInfo(FileIDType defaultForExtension, FormatTester tester)
			{
				Testers = new List<FormatTester>(1);
				if(tester != null)
					Testers.Add(tester);
				DefaultForExtension = defaultForExtension;
			}

			public readonly FileIDType DefaultForExtension;
			public readonly List<FormatTester> Testers;
		}

		/// <summary>
		/// testers to try for each extension, along with a default for the extension
		/// </summary>
		private static readonly Dictionary<string, ExtensionInfo> ExtensionHandlers = new Dictionary<string, ExtensionInfo> {
			{ "GBA", new ExtensionInfo(FileIDType.GBA, (j)=>Test_Simple(j,FileIDType.GBA,SimpleMagics.GBA) ) },
			{ "GB", new ExtensionInfo(FileIDType.GB, Test_GB_GBC ) },
			{ "GBC", new ExtensionInfo(FileIDType.GBC, Test_GB_GBC ) },

			//for now
			{ "ROM", new ExtensionInfo(FileIDType.Multiple, null ) },
		};

		private delegate FileIDResult FormatTester(IdentifyJob job);

		private static readonly int[] no_offsets = { 0 };

		/// <summary>
		/// checks for the magic string (bytewise ASCII check) at the given address
		/// </summary>
		private static bool CheckMagic(Stream stream, IEnumerable<SimpleMagicRecord> recs, params int[] offsets)
		{
			if (offsets.Length == 0)
				offsets = no_offsets;

			foreach (int n in offsets)
			{
				bool ok = true;
				foreach (var r in recs)
				{
					if (!CheckMagicOne(stream, r, n))
					{
						ok = false;
						break;
					}
				}
				if (ok) return true;
			}
			return false;
		}

		private static bool CheckMagic(Stream stream, SimpleMagicRecord rec, params int[] offsets)
		{
			return CheckMagic(stream, new SimpleMagicRecord[] { rec }, offsets);
		}

		private static bool CheckMagicOne(Stream stream, SimpleMagicRecord rec, int offset)
		{
			stream.Position = rec.Offset + offset;
			string key = rec.Key;
			int len = rec.Length;
			if (len == -1)
				len = key.Length;
			for (int i = 0; i < len; i++)
			{
				int n = stream.ReadByte();
				if (n == -1) return false;
				if (n != key[i])
					return false;
			}
			
			return true;
		}

		private static int ReadByte(Stream stream, int ofs)
		{
			stream.Position = ofs;
			return stream.ReadByte();
		}

		private static FileIDResult Test_Simple(IdentifyJob job, FileIDType type, SimpleMagicRecord magic)
		{
			var ret = new FileIDResult(type);

			if (CheckMagic(job.Stream, magic))
				return new FileIDResult(type, 100);
			else
				return new FileIDResult();
		}

		private static FileIDResult Test_GB_GBC(IdentifyJob job)
		{
			if (!CheckMagic(job.Stream, SimpleMagics.GB))
				return new FileIDResult();

			var ret = new FileIDResult(FileIDType.GB, 100);
			int type = ReadByte(job.Stream, 0x143);
			if ((type & 0x80) != 0)
				ret.FileIDType = FileIDType.GBC;

			//could check cart type and rom size for extra info if necessary

			return ret;
		}
	}
}
