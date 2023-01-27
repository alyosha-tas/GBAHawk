using System;
using System.IO;
using Newtonsoft.Json;

//this file contains some cumbersome self-"serialization" in order to gain a modicum of control over what the serialized output looks like
//I don't want them to look like crufty json

namespace BizHawk.Client.Common
{
	public interface IOpenAdvanced
	{
		string TypeName { get; }
		string DisplayName { get; }

		/// <summary>
		/// returns a sole path to use for opening a rom (not sure if this is a good idea)
		/// </summary>
		string SimplePath { get; }
		
		void Deserialize(string str);
		void Serialize(TextWriter tw);
	}

	public static class OpenAdvancedTypes
	{
		public const string OpenRom = "OpenRom";
	}


	public static class OpenAdvancedSerializer
	{
		public static IOpenAdvanced ParseWithLegacy(string text)
		{
			return text.StartsWith("*")
				? Deserialize(text.Substring(1))
				: new OpenAdvanced_OpenRom { Path = text };
		}

		private static IOpenAdvanced Deserialize(string text)
		{
			int idx = text.IndexOf('*');
			string type = text.Substring(0, idx);
			string token = text.Substring(idx + 1);

			var ioa = type switch
			{
				OpenAdvancedTypes.OpenRom => (IOpenAdvanced)new OpenAdvanced_OpenRom(),
				_ => null
			};

			if (ioa == null)
			{
				throw new InvalidOperationException($"{nameof(IOpenAdvanced)} deserialization error");
			}

			ioa.Deserialize(token);
			return ioa;
		}

		public static string Serialize(IOpenAdvanced ioa)
		{
			var sw = new StringWriter();
			sw.Write("{0}*", ioa.TypeName);
			ioa.Serialize(sw);
			return sw.ToString();
		}
	}

	public class OpenAdvanced_OpenRom : IOpenAdvanced
	{
		public string Path;

		public string TypeName => "OpenRom";
		public string DisplayName => Path;
		public string SimplePath => Path;

		public void Deserialize(string str)
		{
			Path = str;
		}

		public void Serialize(TextWriter tw)
		{
			tw.Write(Path);
		}
	}
}