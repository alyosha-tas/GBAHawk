#nullable disable

using System.Collections.Generic;

// ReSharper disable IdentifierTypo
// ReSharper disable InconsistentNaming
// ReSharper disable StringLiteralTypo
namespace BizHawk.Emulation.Common
{
	public static class FirmwareDatabase
	{
		public static IEnumerable<FirmwareFile> FirmwareFiles => FirmwareFilesByHash.Values;

		public static readonly IReadOnlyDictionary<string, FirmwareFile> FirmwareFilesByHash;

		public static readonly IReadOnlyCollection<FirmwareOption> FirmwareOptions;

		public static readonly IReadOnlyCollection<FirmwareRecord> FirmwareRecords;

		public static readonly IReadOnlyList<FirmwarePatchOption> AllPatches;

		static FirmwareDatabase()
		{
			List<FirmwarePatchOption> allPatches = new();
			Dictionary<string, FirmwareFile> filesByHash = new();
			List<FirmwareOption> options = new();
			List<FirmwareRecord> records = new();

			FirmwareFile File(
				string hash,
				long size,
				string recommendedName,
				string desc,
				string additionalInfo = "")
					=> filesByHash[hash] = new(
						hash: hash,
						size: size,
						recommendedName: recommendedName,
						desc: desc,
						additionalInfo: additionalInfo);

			void Option(string systemId, string id, in FirmwareFile ff)
				=> options.Add(new(new(systemId, id), ff.Hash, ff.Size));

			void Firmware(string systemId, string id, string desc)
				=> records.Add(new(new(systemId, id), desc));

			var gbaNormal = File("300C20DF6731A33952DED8C436F7F186D25D3492", 16384, "GBA_bios.rom", "Bios (World)");
			var gbaJDebug = File("AA98A2AD32B86106340665D1222D7D973A1361C7", 16384, "GBA_bios_Debug-(J).rom", "Bios (J Debug)");
			Firmware("GBA", "Bios", "Bios");
			Option("GBA", "Bios", in gbaNormal);
			Option("GBA", "Bios", in gbaJDebug);

			Firmware("GBC", "World", "Game Boy Color Boot Rom");
			Option("GBC", "World", File("1293D68BF9643BC4F36954C1E80E38F39864528D", 2304, "cgb.bin", "Game Boy Color Boot Rom"));

			// GB
			Firmware("GB", "World", "Game Boy Boot Rom");
			Option("GB", "World", File("4ED31EC6B0B175BB109C0EB5FD3D193DA823339F", 256, "GB_dmg.bin", "Game Boy Boot Rom"));

			// NES
			Firmware("NES", "World", "Game Genie Rom");
			Option("NES", "World", File("AAA9833C3DE2213F2B0326769B715651FD13FD84", 0x1000, "Game_Genie.bin", "Game Genie Rom"));

			// SNES
			Firmware("SNES", "IPL", "APU Boot Rom");
			Option("SNES", "IPL", File("97E352553E94242AE823547CD853EECDA55C20F0", 0x40, "IPL.bin", "APU Boot Rom"));

			Firmware("SNES", "CX4", "CX4 Rom");
			Option("SNES", "CX4", File("A002F4EFBA42775A31185D443F3ED1790B0E949A", 3072, "SNES_cx4.rom", "CX4 Rom"));

			Firmware("SNES", "DSP1", "DSP1 Rom");
			Option("SNES", "DSP1", File("188D471FEFEA71EB53F0EE7064697FF0971B1014", 8192, "SNES_dsp1.rom", "DSP1 Rom"));

			Firmware("SNES", "DSP1b", "DSP1b Rom");
			Option("SNES", "DSP1b", File("78B724811F5F18D8C67669D9390397EB1A47A5E2", 8192, "SNES_dsp1b.rom", "DSP1b Rom"));

			Firmware("SNES", "DSP2", "DSP2 Rom");
			Option("SNES", "DSP2", File("198C4B1C3BFC6D69E734C5957AF3DBFA26238DFB", 8192, "SNES_dsp2.rom", "DSP2 Rom"));

			Firmware("SNES", "DSP3", "DSP3 Rom");
			Option("SNES", "DSP3", File("558DA7CB3BD3876A6CA693661FFC6C110E948CF9", 8192, "SNES_dsp3.rom", "DSP3 Rom"));

			Firmware("SNES", "DSP4", "DSP4 Rom");
			Option("SNES", "DSP4", File("AF6478AECB6F1B67177E79C82CA04C56250A8C72", 8192, "SNES_dsp4.rom", "DSP4 Rom"));

			Firmware("SNES", "ST010", "ST010 Rom");
			Option("SNES", "ST010", File("6472828403DE3589433A906E2C3F3D274C0FF008", 53248, "SNES_st010.rom", "ST010 Rom"));

			Firmware("SNES", "ST011", "ST011 Rom");
			Option("SNES", "ST011", File("FECBAE2CEC76C710422486BAA186FFA7CA1CF925", 53248, "SNES_st011.rom", "ST011 Rom"));

			Firmware("SNES", "ST018", "ST018 Rom");
			Option("SNES", "ST018", File("91383B92745CC7CC4F15409AC5BC2C2F699A43F1", 163840, "SNES_st018.rom", "ST018 Rom"));

			AllPatches = allPatches;
			FirmwareFilesByHash = filesByHash;
			FirmwareOptions = options;
			FirmwareRecords = records;
		}
	} // static class FirmwareDatabase
}
