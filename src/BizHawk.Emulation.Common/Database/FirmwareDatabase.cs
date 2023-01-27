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

			var gbaNormal = File("300C20DF6731A33952DED8C436F7F186D25D3492", 16384, "GBA_bios.rom", "Bios (World)");
			var gbaJDebug = File("AA98A2AD32B86106340665D1222D7D973A1361C7", 16384, "GBA_bios_Debug-(J).rom", "Bios (J Debug)");
			Firmware("GBA", "Bios", "Bios");
			Option("GBA", "Bios", in gbaNormal);
			Option("GBA", "Bios", in gbaJDebug);

			Firmware("GBC", "World", "Game Boy Color Boot Rom");
			Option("GBC", "World", File("1293D68BF9643BC4F36954C1E80E38F39864528D", 2304, "cgb.bin", "Game Boy Color Boot Rom"), FirmwareOptionStatus.Ideal);

			AllPatches = allPatches;
			FirmwareFilesByHash = filesByHash;
			FirmwareOptions = options;
			FirmwareRecords = records;
		}
	} // static class FirmwareDatabase
}
