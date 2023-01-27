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
				string additionalInfo = "",
				bool isBad = false)
					=> filesByHash[hash] = new(
						hash: hash,
						size: size,
						recommendedName: recommendedName,
						desc: desc,
						additionalInfo: additionalInfo,
						isBad: isBad);

			void Option(string systemId, string id, in FirmwareFile ff, FirmwareOptionStatus status = FirmwareOptionStatus.Acceptable)
				=> options.Add(new(new(systemId, id), ff.Hash, ff.Size, ff.IsBad ? FirmwareOptionStatus.Bad : status));

			void Firmware(string systemId, string id, string desc)
				=> records.Add(new(new(systemId, id), desc));

			void FirmwareAndOption(string hash, long size, string systemId, string id, string name, string desc)
			{
				Firmware(systemId, id, desc);
				Option(systemId, id, File(hash, size, name, desc), FirmwareOptionStatus.Ideal);
			}

			// FDS has two OK variants  (http://tcrf.net/Family_Computer_Disk_System)
			var fdsNintendo = File("57FE1BDEE955BB48D357E463CCBF129496930B62", 8192, "FDS_disksys-nintendo.rom", "Bios (Nintendo)");
			var fdsTwinFc = File("E4E41472C454F928E53EB10E0509BF7D1146ECC1", 8192, "FDS_disksys-nintendo.rom", "Bios (TwinFC)");
			Firmware("NES", "Bios_FDS", "Bios");
			Option("NES", "Bios_FDS", in fdsNintendo, FirmwareOptionStatus.Ideal);
			Option("NES", "Bios_FDS", in fdsTwinFc);

			var gbaNormal = File("300C20DF6731A33952DED8C436F7F186D25D3492", 16384, "GBA_bios.rom", "Bios (World)");
			var gbaJDebug = File("AA98A2AD32B86106340665D1222D7D973A1361C7", 16384, "GBA_bios_Debug-(J).rom", "Bios (J Debug)");
			Firmware("GBA", "Bios", "Bios");
			Option("GBA", "Bios", in gbaNormal);
			Option("GBA", "Bios", in gbaJDebug);

			// MSX
			FirmwareAndOption("2F997E8A57528518C82AB3693FDAE243DBBCC508", 32768, "MSX", "bios_test_ext", "MSX_cbios_main_msx1.rom", "MSX BIOS (C-BIOS v0.29a)");
			//FirmwareAndOption("E998F0C441F4F1800EF44E42CD1659150206CF79", 16384, "MSX", "bios_pal", "MSX_8020-20bios.rom", "MSX BIOS (Philips VG-8020)");
			//FirmwareAndOption("DF48902F5F12AF8867AE1A87F255145F0E5E0774", 16384, "MSX", "bios_jp", "MSX_4000bios.rom", "MSX BIOS (FS-4000)");
			FirmwareAndOption("409E82ADAC40F6BDD18EB6C84E8B2FBDC7FB5498", 32768, "MSX", "bios_basic_usa", "MSX.rom", "MSX BIOS and BASIC");
			FirmwareAndOption("3656BB3BBC17D280D2016FE4F6FF3CDED3082A41", 32768, "MSX", "bios_basic_usa", "MSX.rom", "MSX 1.0 BIOS and BASIC");
			FirmwareAndOption("302AFB5D8BE26C758309CA3DF611AE69CCED2821", 32768, "MSX", "bios_basic_jpn", "MSX_jpn.rom", "MSX 1.0 JPN BIOS and BASIC");

			Firmware("GBC", "World", "Game Boy Color Boot Rom");
			Option("GBC", "World", File("1293D68BF9643BC4F36954C1E80E38F39864528D", 2304, "cgb.bin", "Game Boy Color Boot Rom"), FirmwareOptionStatus.Ideal);

			AllPatches = allPatches;
			FirmwareFilesByHash = filesByHash;
			FirmwareOptions = options;
			FirmwareRecords = records;
		}
	} // static class FirmwareDatabase
}
