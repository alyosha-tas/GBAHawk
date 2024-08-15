using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;

using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA.Common
{
	public interface IGBAGPUViewable : IEmulatorService
	{
		GBAGPUMemoryAreas GetMemoryAreas();

		/// <summary>
		/// calls correspond to entering hblank (maybe) and in a regular frame, the sequence of calls will be 160, 161, ..., 227, 0, ..., 159
		/// </summary>
		void SetScanlineCallback(Action callback, int scanline);
	}

	public class GBAGPUMemoryAreas
	{
		public IntPtr vram;
		public IntPtr oam;
		public IntPtr mmio;
		public IntPtr palram;
	}

	public class GBACommonFunctions
	{
		public static bool pokemon_check(string romHashSHA1)
		{
			if ((romHashSHA1 == "SHA1:424740BE1FC67A5DDB954794443646E6AEEE2C1B") || // Pokemon Ruby (Germany) (Rev 1)
				(romHashSHA1 == "SHA1:1C2A53332382E14DAB8815E3A6DD81AD89534050") || // "" (Germany)	
				(romHashSHA1 == "SHA1:F28B6FFC97847E94A6C21A63CACF633EE5C8DF1E") || // "" (USA)
				(romHashSHA1 == "SHA1:5B64EACF892920518DB4EC664E62A086DD5F5BC8") || // "" (USA, Europe) (Rev 2)
				(romHashSHA1 == "SHA1:610B96A9C9A7D03D2BAFB655E7560CCFF1A6D894") || // "" (Europe) (Rev 1)
				(romHashSHA1 == "SHA1:A6EE94202BEC0641C55D242757E84DC89336D4CB") || // "" (France)
				(romHashSHA1 == "SHA1:BA888DFBA231A231CBD60FE228E894B54FB1ED79") || // "" (France) (Rev 1)
				(romHashSHA1 == "SHA1:5C5E546720300B99AE45D2AA35C646C8B8FF5C56") || // "" (Japan)
				(romHashSHA1 == "SHA1:1F49F7289253DCBFECBC4C5BA3E67AA0652EC83C") || // "" (Spain)
				(romHashSHA1 == "SHA1:9AC73481D7F5D150A018309BBA91D185CE99FB7C") || // "" (Spain) (Rev 1)
				(romHashSHA1 == "SHA1:2B3134224392F58DA00F802FAA1BF4B5CF6270BE") || // "" (Italy)
				(romHashSHA1 == "SHA1:015A5D380AFE316A2A6FCC561798EBFF9DFB3009") || // "" (Italy) (Rev 1)

				(romHashSHA1 == "SHA1:1692DB322400C3141C5DE2DB38469913CEB1F4D4") || // Pokemon Emerald (Italy)
				(romHashSHA1 == "SHA1:F3AE088181BF583E55DAF962A92BB46F4F1D07B7") || // "" (USA, Europe)
				(romHashSHA1 == "SHA1:FE1558A3DCB0360AB558969E09B690888B846DD9") || // "" (Spain)
				(romHashSHA1 == "SHA1:D7CF8F156BA9C455D164E1EA780A6BF1945465C2") || // "" (Japan)
				(romHashSHA1 == "SHA1:61C2EB2B380B1A75F0C94B767A2D4C26CD7CE4E3") || // "" (Germany)
				(romHashSHA1 == "SHA1:CA666651374D89CA439007BED54D839EB7BD14D0") || // "" (France)

				(romHashSHA1 == "SHA1:5A087835009D552D4C5C1F96BE3BE3206E378153") || // Pokemon Sapphire (Germany)
				(romHashSHA1 == "SHA1:7E6E034F9CDCA6D2C4A270FDB50A94DEF5883D17") || // "" (Germany) (Rev 1)
				(romHashSHA1 == "SHA1:4722EFB8CD45772CA32555B98FD3B9719F8E60A9") || // "" (Europe) (Rev 1)
				(romHashSHA1 == "SHA1:89B45FB172E6B55D51FC0E61989775187F6FE63C") || // "" (USA, Europe) (Rev 2)
				(romHashSHA1 == "SHA1:3CCBBD45F8553C36463F13B938E833F652B793E4") || // "" (USA)
				(romHashSHA1 == "SHA1:3233342C2F3087E6FFE6C1791CD5867DB07DF842") || // "" (Japan)
				(romHashSHA1 == "SHA1:0FE9AD1E602E2FAFA090AEE25E43D6980625173C") || // "" (Rev 1)
				(romHashSHA1 == "SHA1:3A6489189E581C4B29914071B79207883B8C16D8") || // "" (Spain)
				(romHashSHA1 == "SHA1:C269B5692B2D0E5800BA1DDF117FDA95AC648634") || // "" (France)
				(romHashSHA1 == "SHA1:860E93F5EA44F4278132F6C1EE5650D07B852FD8") || // "" (France) (Rev 1)
				(romHashSHA1 == "SHA1:73EDF67B9B82FF12795622DCA412733755D2C0FE") || // "" (Italy) (Rev 1)
				(romHashSHA1 == "SHA1:F729DD571FB2C09E72C5C1D68FE0A21E72713D34"))   // "" (Italy))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		public static bool EEPROM_64K_check(string romHashSHA1)
		{
			if ((romHashSHA1 == "SHA1:0E55F66EFCAD4FE80B75942C1D42A86FB8CBCB74") || // 2 Games in 1 - Disney Princesas + Lizzie McGuire (Spain)	GBA		
				(romHashSHA1 == "SHA1:5A5C56306CFCE32FD04B330666272D2E9C88057F") || // 2 Games in 1 - Disney Princess + Lizzie McGuire (Europe)	GBA		
				(romHashSHA1 == "SHA1:F02D179ACCBEB9B68BCAEF8720EB0AE426DDE6EE") || // 2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA)	GBA		saveType=1
				(romHashSHA1 == "SHA1:3733ADAFF49866A8FE01E23526F4F52CA39CBF58") || // 2 Games in 1! - Dragon Ball Z - Buu's Fury + Dragon Ball GT - Transformation (USA)	GBA		saveType=1
				(romHashSHA1 == "SHA1:EC1BB449B3D52B421E0AC3DFE3FDBCE22822251A") || // 2 in 1 Game Pack - Tony Hawk's Underground + Kelly Slater's Pro Surfer (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:F116CC8E6A7F6DBA49564031A2AC63D837D4F11B") || // 2 in 1 - V-Rally 3 + Stuntman (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:C3988CDA910F181AD989D3E157F4EBDC405ECDA3") || // Advance GT2 (Japan) (En)	GBA				
				(romHashSHA1 == "SHA1:9A0623A8680C4CF9B745F940B99DA0A192764828") || // Aka-chan Doubutsuen (Japan)	GBA
				(romHashSHA1 == "SHA1:634624C77C90A8ff40BEC9046133C50041EFC49B") || // Aka-chan Doubutsuen (Japan) (rev 1)	GBA			
				(romHashSHA1 == "SHA1:A870E1321A8AD3317CD695FCC0C713441C25919F") || // Alex Ferguson's Player Manager 2002 (Europe) (En,Fr,De,Es,It,Nl)	GBA		
				(romHashSHA1 == "SHA1:1FC03CCA393DDBC633D309AAE95B42C89E4D0E96") || // American Bass Challenge (USA)	GBA			
				(romHashSHA1 == "SHA1:1B2708FA0015803DE0299E968A1C9F14D12AD7E8") || // Angel Collection - Mezase! Gakuen no Fashion Leader (Japan)	GBA		
				(romHashSHA1 == "SHA1:3619766CEDF93045A9213B0D251A169439990CF1") || // Angel Collection 2 - Pichimo ni Narou (Japan)	GBA		
				(romHashSHA1 == "SHA1:CF01DFC9F25C805B9A72524903F742F11570A8EB") || // Angelique (Japan)	GBA		
				(romHashSHA1 == "SHA1:9A4FC3533BBDB28FD5945BD1EE7D84D992EEE12F") || // Astro Boy - Omega Factor (Europe) (En,Ja,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:4D652C24979DD7A08EE683B32D1D8986A5511A87") || // Astro Boy - Omega Factor (USA) (En,Ja,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:CA16DC4044B9AE98A26C826BB3CC19A7E8315315") || // Astro Boy - Tetsuwan Atom - Atom Heart no Himitsu (Japan)	GBA		
				(romHashSHA1 == "SHA1:9D864E9B3CCCE4E5E1B1C566AFA0D06088E88DFD") || // Avatar - The Last Airbender (USA)	GBA		
				(romHashSHA1 == "SHA1:4A62DFAAA562B23D4F851D3BB483D0FBF67894AF") || // Avatar - The Legend of Aang (Europe) (En,Fr,De,Nl)	GBA			
				(romHashSHA1 == "SHA1:95B266E8ECDAE1938CD180F597E5F21CB86D8B96") || // Azumanga Daiou Advance (Japan)	GBA		
				(romHashSHA1 == "SHA1:8E4FC778307BD70A0179F788EB58C4CC06886139") || // B-Densetsu! Battle B-Daman - Fire Spirits! (Japan)	GBA		
				(romHashSHA1 == "SHA1:49BEBA2E36459AF92917ACF911CBF2492F1B07F1") || // B-Densetsu! Battle B-Daman - Moero! B-Damashii!! (Japan)	GBA		
				(romHashSHA1 == "SHA1:8F5B7C20E8BE0F1E7919915B905C2B182A993CEC") || // Banjo-Kazooie - Grunty's Revenge (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:F335FEDE72CD0273FCCE925A20CA272EDE08C15B") || // Banjo-Kazooie - Grunty's Revenge (Europe) (En,Fr,De)	GBA		
				(romHashSHA1 == "SHA1:F2D3D4F8226C17C5391ABCB2A2CBCE10C59AB4C7") || // Banjo-Kazooie - Grunty's Revenge (Europe) (En,Fr,De,Es,It) (Beta)	GBA		
				(romHashSHA1 == "SHA1:5A0B6FE8AC36E05DE9C2E9B31D8B68EF125F4D96") || // Banjo-Kazooie - Grunty's Revenge (Europe) (En,Fr,De,Es,It) (Beta) (early)	GBA		
				(romHashSHA1 == "SHA1:2A38116E3219CD0A9216C9ADF8383BA84489F3B0") || // Banjo-Kazooie - La Vendetta di Grunty (Italy)	GBA		
				(romHashSHA1 == "SHA1:3B948A1CB1790C16C7A3FFF9C0CFBF95A973200D") || // Banjo-Kazooie - La Venganza de Grunty (Spain)	GBA	
				(romHashSHA1 == "SHA1:2FE05E2C9DFC02EFE415C735F95FA1D4CD12EEB7") || // Barnyard (Europe) (En,Fr,De,Es,It,Nl)	GBA		
				(romHashSHA1 == "SHA1:7A6E2BF247BA7AC2420A5EEF39B77A5D871785B5") || // Barnyard (USA)	GBA			
				(romHashSHA1 == "SHA1:4CD862E02892CCC3E2A170EF401BC778C9042A35") || // Bass Tsuri Shiyouze! - Tournament wa Senryaku da! (Japan)	GBA		
				(romHashSHA1 == "SHA1:7016E0667201C965B6AE78C114B6CC5CB5EC7FF7") || // Battle B-Daman (USA)	GBA		
				(romHashSHA1 == "SHA1:D57084C399BBD4B37D145B712FF6CA22DB9B1C49") || // Battle B-Daman - Fire Spirits! (USA)	GBA	
				(romHashSHA1 == "SHA1:5CAFAA89770E4010026AD691E6E33965F27F8852") || // Best Friends - Hunde & Katzen (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:8E6D3AEC2506CCB71E72FEB30C52B22CAB7AB563") || // Beyblade G-Revolution (Europe) (En,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:A89F7B4EB77DC986022201DB51A676451BA7C5E4") || // Beyblade G-Revolution (USA)	GBA		
				(romHashSHA1 == "SHA1:CD527C8C24E20E33913FC45199E64B3E6138A6E5") || // Beyblade V-Force - Ultimate Blader Jam (USA)	GBA		
				(romHashSHA1 == "SHA1:6D20A586E7BB52B4A9352BBEAEC9D7BE65E93369") || // Beyblade V-Force - Ultimate Blader Jam (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:697F85513AB5704B8B1D554E087A45BFDB92EC33") || // Black Matrix Zero (Japan)	GBA			
				(romHashSHA1 == "SHA1:625E4C19F045202DE4D95262B5CD30F15C4469FE") || // Bomber Man Max 2 - Bomber Man Version (Japan)	GBA		
				(romHashSHA1 == "SHA1:7277EA99C0015DF9AE1A29A9B45909F168624901") || // Bomber Man Max 2 - Max Version (Japan)	GBA			
				(romHashSHA1 == "SHA1:B2C64452FE8C879C1ABD5CF709E3E26971EEB166") || // Bomberman Max 2 - Blue Advance (USA)	GBA		
				(romHashSHA1 == "SHA1:EB25E28AE1047145FD2CBCC3ECBB63330FBC8FE0") || // Bomberman Max 2 - Blue Advance (Europe) (En,Fr,De)	GBA		
				(romHashSHA1 == "SHA1:1428BB0A78AEBB112A2702ED8561BDFE9BCEDCF1") || // Bomberman Max 2 - Red Advance (USA)	GBA		
				(romHashSHA1 == "SHA1:4D514FE19B59D42BE9A118C99E2008EFEE2AEDCB") || // Bomberman Max 2 - Red Advance (Europe) (En,Fr,De)	GBA			
				(romHashSHA1 == "SHA1:82E8E42FE7B5CEF46079ED18AD38B2F48C26D52D") || // Bouken-ou Beet - Busters Road (Japan)	GBA	
				(romHashSHA1 == "SHA1:69B6326D19143F3CA2C2C64AC22A8B73935AF555") || // Bratz - Forever Diamondz (Europe) (En,Fr,Es,It)	GBA		
				(romHashSHA1 == "SHA1:FD590CC4C3AD26FF32F80F66890D8CFC968B23FB") || // Bratz - Forever Diamondz (Germany)	GBA		
				(romHashSHA1 == "SHA1:68222B1E029D931C2A9776B7040928A921A9C12B") || // Bratz - Forever Diamondz (USA)	GBA		
				(romHashSHA1 == "SHA1:314ECED054604866153883EE20E0966E1443E0D9") || // Broken Sword - The Shadow of the Templars (USA) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:25C6CF94B1B54A1AC5058460FAAD00CF93D272F5") || // Broken Sword - The Shadow of the Templars (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:5C9982A230EBF6C93B1706A242D2149FC052A37C") || // Car Battler Joe (USA)	GBA			
				(romHashSHA1 == "SHA1:E91289D5EF58F984F0377D956AAD1D3BF48667BB") || // Cardcaptor Sakura - Sakura Card de Mini Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:0DAB5140EBC541E03DD00A8CE87CCE4F0326B0E6") || // Catz (Europe) (En,Fr,De,It)	GBA		
				(romHashSHA1 == "SHA1:8783F76311A8E23173EB4DADABC5176246B1B7B1") || // Catz (USA, Europe)	GBA			
				(romHashSHA1 == "SHA1:1DA3124A73B2F22ED29F53B057E759191DC1F3F3") || // Chaoji Maliou Shijie (China)	GBA		
				(romHashSHA1 == "SHA1:6F30307DF4603EE54DB4AD50490380FEB93AAA44") || // CIMA - The Enemy (USA)	GBA		
				(romHashSHA1 == "SHA1:5B72DF21B913363B7F98E6942850C1593A3F0A4F") || // Classic NES Series - Excitebike (USA, Europe)	GBA		saveType=1;mirroringEnabled=1
				(romHashSHA1 == "SHA1:D47BE5A160FC8E7B9F789AD7BA8867EF65B4D448") || // Classic NES Series - Legend of Zelda (USA, Europe)	GBA		saveType=1;mirroringEnabled=1
				(romHashSHA1 == "SHA1:AB608FC378CD0F8FB905B1B8D5B85400EE00CCFB") || // Classic NES Series - Zelda II - The Adventure of Link (USA, Europe)	GBA		saveType=1;mirroringEnabled=1	
				(romHashSHA1 == "SHA1:23934042B3F7AC414DEF68246C281E20F735A1D7") || // Crash Superpack (USA)	GBA		
				(romHashSHA1 == "SHA1:3C2820877F810D1CB4D56A4881B38A457356BDD1") || // Crayon Shin-chan - Arashi o Yobu Cinemaland no Daibouken! (Japan)	GBA		
				(romHashSHA1 == "SHA1:E17BAA57D339F7D85F562441111696E88088FCDD") || // Crayon Shin-chan - Densetsu o Yobu Omake no Miyako Shockgaan! (Japan)	GBA			
				(romHashSHA1 == "SHA1:DA03610FCE2C684C1DD49DACCDA6392B7B86D51E") || // Creatures (Europe) (En,Fr,De)	GBA		
				(romHashSHA1 == "SHA1:2CFE90927401FC2D7F62FCDD2648B2AA0DFA31BF") || // Creatures (Europe) (En,Es,It)	GBA		
				(romHashSHA1 == "SHA1:0B5CE051AA1FD83AB0EC79122B07B02BB41096A5") || // Custom Robo GX (Japan)	GBA			
				(romHashSHA1 == "SHA1:5D98B5179F4E9AE7AE72D6A43BED5F046F881A7B") || // Dave Mirra Freestyle BMX 2 (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:3B86100FCE6FDA95B46C0D9E02D08052E69C2C2C") || // Dave Mirra Freestyle BMX 2 (USA)	GBA		
				(romHashSHA1 == "SHA1:4C3A3ECBA81F7A7DF5A05D294BBE796109DD5D4D") || // Dave Mirra Freestyle BMX 2 (Europe) (En,Fr,De,Es,It) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:DCCE47B536ACE90A918D94D06698C92538D7E4D1") || // Dave Mirra Freestyle BMX 3 (USA, Europe)	GBA			
				(romHashSHA1 == "SHA1:4FC0E2240DDD11D4A1F9A80EAFF3B945C8F27C8A") || // Diadroids World - Evil Teikoku no Yabou (Japan)	GBA		
				(romHashSHA1 == "SHA1:177A4D01A88A512CBC8DC5927B55999169359BBC") || // DigiCommunication Nyo - Datou! Black Gemagema Dan (Japan)	GBA		saveType=1	
				(romHashSHA1 == "SHA1:729099DD230581D86BDF86E91359D74B79FE862C") || // Dogz - Fashion (Europe)	GBA		
				(romHashSHA1 == "SHA1:2DB620FBC629E957E8B73DD169BF83950814FD51") || // Dogz - Fashion (USA)	GBA		
				(romHashSHA1 == "SHA1:D5A8887C0EA1DB4F447648E4FAECEAF07EC99493") || // Dogz 2 (Europe) (En,Fr,De,It)	GBA		
				(romHashSHA1 == "SHA1:BD12AAADFAC73E7FDE336C8B25ECE78447CC1D87") || // Dogz 2 (USA)	GBA		
				(romHashSHA1 == "SHA1:A1897F4719BEA1D3F6B6E68FAE752EED7E3DF021") || // Dogz 2 (Europe)	GBA				
				(romHashSHA1 == "SHA1:5633769B002ED9AE00B090EED1FF4D2AF80246A0") || // Dokidoki Cooking Series 1 - Komugi-chan no Happy Cake (Japan)	GBA		
				(romHashSHA1 == "SHA1:11DA3A8E99457787F1263FF3396CB85B083583F0") || // Dokidoki Cooking Series 2 - Gourmet Kitchen - Suteki na Obentou (Japan)	GBA		
				(romHashSHA1 == "SHA1:B0A4D59447C8D7C321BEA4DC7253B0F581129EDE") || // Donkey Kong Country 2 (USA, Australia)	GBA		
				(romHashSHA1 == "SHA1:2243E9B8C299744E351BDD9E28BC2212F0840782") || // Donkey Kong Country 2 (Europe) (En,Fr,De,Es,It)	GBA			
				(romHashSHA1 == "SHA1:3E98BD11ABED560B1669185E6FC326303E82CA01") || // Doraemon - Dokodemo Walker (Japan)	GBA			
				(romHashSHA1 == "SHA1:9F5CB4E49F6B724FB5D907D291C5F179FCF1C360") || // Downtown - Nekketsu Monogatari EX (Japan)	GBA			
				(romHashSHA1 == "SHA1:D87DB7A5C39A384917B5F8F628F3338F2C5A4DC7") || // Dr. Sudoku (Europe)	GBA		
				(romHashSHA1 == "SHA1:0DA4E12281A616AE63B95479BADF932DDB30DF5B") || // Dr. Sudoku (USA)	GBA		
				(romHashSHA1 == "SHA1:8DD4B6E95DCA4554A919BFF55128481A9A95B611") || // Dragon Ball GT - Transformation (USA)	GBA		saveType=1
				(romHashSHA1 == "SHA1:F1C4B07554D2A3B1AD2F325307051E775CE68087") || // Dragon Ball Z - Buu's Fury (USA)	GBA		saveType=1
				(romHashSHA1 == "SHA1:DB36FF52FCD63F753F9D66439AA3D2216701C326") || // Dragon Ball Z - The Legacy of Goku II (Europe) (En,Fr,De,Es,It)	GBA		saveType=1
				(romHashSHA1 == "SHA1:18E0715DEC419F3501C301511530D2EDCD590F8B") || // Dragon Ball Z - The Legacy of Goku II (USA)	GBA		saveType=1
				(romHashSHA1 == "SHA1:4E18F54B8BACE6ED32B6315B159E903F61D69339") || // Dragon Ball Z - The Legacy of Goku II International (Japan)	GBA		saveType=1	
				(romHashSHA1 == "SHA1:430A7062844888E68BD5587ED53A769BA548ADB3") || // Dragon Quest Monsters - Caravan Heart (Japan)	GBA			
				(romHashSHA1 == "SHA1:92BB07AB53B4C704F611934C2B1B97C820B88693") || // Eragon (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:B48BF75B6F9B2C69BF0B2B9B1920D9DDB69E66AA") || // Eragon (USA)	GBA		
				(romHashSHA1 == "SHA1:8C005E02B64CCF07DE5905311DB68983B0A3B76D") || // Erementar Gerad - Tozasareshi Uta (Japan)	GBA		
				(romHashSHA1 == "SHA1:D9320757C7543476614FC9F91E22FD96A30A32F4") || // ESPN Great Outdoor Games - Bass Tournament (Europe)	GBA		
				(romHashSHA1 == "SHA1:4227CBC4F8C66D3688C1E13647503FB02CB64327") || // ESPN Winter X-Games Snowboarding 2002 (Japan) (En)	GBA			
				(romHashSHA1 == "SHA1:E7216240814E26527CE078BE2C3B835973CD7DCC") || // Exciting Bass (Japan)	GBA			
				(romHashSHA1 == "SHA1:842CAE5E8156F9DA5C7271A265F8DD0C763AEFE3") || // Famicom Mini - Dai-2-ji Super Robot Taisen (Japan) (Promo)	GBA			
				(romHashSHA1 == "SHA1:F7A425F77A26A26B8E3F6ADDE2BB89AE1F63377E") || // Famicom Mini 05 - Zelda no Densetsu 1 - The Hyrule Fantasy (Japan)	GBA		
				(romHashSHA1 == "SHA1:2E1DF85E7D41198BD56735C5C9746904464D7150") || // Famicom Mini 23 - Metroid (Japan)	GBA		saveType=1;mirroringEnabled=1
				(romHashSHA1 == "SHA1:CCA6EB41EA8EDC8115994FAD2A0B329AF44BB659") || // Famicom Mini 24 - Hikari Shinwa - Palthena no Kagami (Japan)	GBA		saveType=1;mirroringEnabled=1
				(romHashSHA1 == "SHA1:203AF9B451880595E8344BB508E839DC65B83930") || // Famicom Mini 25 - The Legend of Zelda 2 - Link no Bouken (Japan)	GBA		saveType=1;mirroringEnabled=1
				(romHashSHA1 == "SHA1:D3E54CFC3E0B08F54BD8B4520E0B661FBF0A75D8") || // Famicom Mini 30 - SD Gundam World - Gachapon Senshi Scramble Wars (Japan)	GBA		saveType=1;mirroringEnabled=1	
				(romHashSHA1 == "SHA1:6AA75B95B5FF6F0D9E81FA682D668FE0FC0F78DA") || // Famista Advance (Japan)	GBA		
				(romHashSHA1 == "SHA1:8AF0853249F6F4DA90AC1044C387E067EF85972A") || // Fantastic Children (Japan)	GBA		
				(romHashSHA1 == "SHA1:826C247EF4E5B831AA0D75E97CFDFE06C48AD7AF") || // Fantastic Maerchen - Cake-ya-san Monogatari (Japan)	GBA			
				(romHashSHA1 == "SHA1:8C1A3208D6BD34DF966617FCC5F539B14CC5992A") || // Frogger - Kodaibunmei no Nazo (Japan)	GBA		
				(romHashSHA1 == "SHA1:19ED8E33FEE941BAE582ADBC67BF8B6BC53CB069") || // Frogger's Journey - The Forgotten Relic (USA)	GBA			
				(romHashSHA1 == "SHA1:542B83AF98FB489E420E6E8547D76351993A56C9") || // Frontier Stories (Japan)	GBA		
				(romHashSHA1 == "SHA1:21BA1D85C204F58F2867B9FDE8498318C8423BE7") || // Fruits Mura no Doubutsu-tachi (Japan) (Rev 2)	GBA		
				(romHashSHA1 == "SHA1:8D582AD27C888E3DBBFFBD421F40B54D15F977EF") || // Fruits Mura no Doubutsu-tachi (Japan)	GBA			
				(romHashSHA1 == "SHA1:FB6B864B13F41AF2521C7E9C7BEC0B490962452F") || // Futari wa Pretty Cure Max Heart - Maji Maji! Fight de IN Janai (Japan)	GBA	
				(romHashSHA1 == "SHA1:F7E55C8A94EEC49EF9EED39AC076688DE2EE5A42") || // Gakkou no Kaidan - Hyakuyoubako no Fuuin (Japan)	GBA		
				(romHashSHA1 == "SHA1:5BD69C56B2B61DB05464BEBC2B579457B986C01B") || // Gakkou o Tsukurou!! Advance (Japan)	GBA		
				(romHashSHA1 == "SHA1:BF4664EC225ED66E72E98393B9B6EC10B3CC9B10") || // Gambler Densetsu Tetsuya - Yomigaeru Densetsu (Japan)	GBA		
				(romHashSHA1 == "SHA1:88B7832624BDB1DA62B5E8F2EBCEECC4628C7924") || // Ganbare! Dodge Fighters (Japan)	GBA		
				(romHashSHA1 == "SHA1:87576A7D83BC49DBC533C18363E8F6D86019583F") || // Gekitou Densetsu Noah - Dream Management (Japan)	GBA		
				(romHashSHA1 == "SHA1:CD3D70E6CF3C7DEC72D4F1F3678113D44A00FACC") || // Gekitou! Car Battler Go!! (Japan)	GBA		
				(romHashSHA1 == "SHA1:10710BC9E66D9C92CCAF77D3343C270A0E3F0E35") || // Gensou Maden Saiyuuki - Hangyaku no Toushin-taishi (Japan)	GBA		
				(romHashSHA1 == "SHA1:0491BF0E9FE5B240C1DE93460C41BFEB63B2ED1B") || // Get! - Boku no Mushi Tsukamaete (Japan)	GBA		
				(romHashSHA1 == "SHA1:10EEC7513511F0B891F32E44EFF383A3A090F787") || // Goemon - New Age Shutsudou! (Japan)	GBA		
				(romHashSHA1 == "SHA1:7F80CB2A3FA2C7FE2A7A1F9A7532EEC7F72A682D") || // Greg Hastings' Tournament Paintball Max'd (USA)	GBA		
				(romHashSHA1 == "SHA1:F30AB501B09026AE045FAF8573220D4F9C113376") || // Gunstar Future Heroes (Europe) (En,Ja,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:6894ED29AF5A1603C72E3B55047A5C9F5D5DDB6B") || // Gunstar Super Heroes (Japan)	GBA		
				(romHashSHA1 == "SHA1:5F3CE44716754BF0423AEDDE7CB1693CEA328A0D") || // Gunstar Super Heroes (USA)	GBA		
				(romHashSHA1 == "SHA1:4B39DDE92AA2E26433A9F0C90CA6235A508BA87C") || // Hajime no Ippo - The Fighting! (Japan)	GBA		
				(romHashSHA1 == "SHA1:500483016098601C9E6E8EDB1D7B2042CAB4F091") || // Hamster Monogatari 3 GBA (Japan)	GBA		
				(romHashSHA1 == "SHA1:E92AFA9FA008283CBD2A4E143B717BA93F0EB9D2") || // Hamster Monogatari Collection (Japan)	GBA		
				(romHashSHA1 == "SHA1:DA0B8DAB120C72E2DE1FF4F623E10A58F3CF177E") || // Hamster Paradise - Pure Heart (Japan)	GBA		
				(romHashSHA1 == "SHA1:747F95C137935CBB9643E818CDEBFCA9BD74D67C") || // Hamster Paradise Advanchu (Japan)	GBA		
				(romHashSHA1 == "SHA1:5BE308501F0CFE0C30C60EF1FBF886707CDACD29") || // Harry Potter and the Prisoner of Azkaban (USA, Europe) (En,Fr,De,Es,It,Nl,Da)	GBA		
				(romHashSHA1 == "SHA1:A0E1A8F0C31FFD6E82EF99C95F903347A6EAE623") || // Harry Potter Collection (Europe) (En,Fr,De,Es,It,Nl,Pt,Sv,No,Da)	GBA		
				(romHashSHA1 == "SHA1:7D29E25CFE75069FFBF84A7B5B7F8E8703FE1A32") || // Harry Potter to Azkaban no Shuujin (Japan)	GBA		
				(romHashSHA1 == "SHA1:5242170AD6FE8F1169E423F13F77AD18F54D3659") || // Hello Kitty Collection - Miracle Fashion Maker (Japan)	GBA		
				(romHashSHA1 == "SHA1:7288A474B8D8EEBB6FC8FDB90FA298B031E3E1C8") || // Hello! Idol Debut - Kids Idol Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:515F150EF1E393E308BF98E2C1CCBBE4F598F6DF") || // Higanbana (Japan) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:E51A7A07867134A790E5B5C8DB5EA8B2892A5F2D") || // High Heat Major League Baseball 2003 (Japan) (En)	GBA		
				(romHashSHA1 == "SHA1:9EAEE2C694B0BE651BE0E319DF1C18FDBD721267") || // High Heat Major League Baseball 2003 (USA)	GBA		
				(romHashSHA1 == "SHA1:FEFC7A73DDCEAF28EBB95B88F5F39A5489024D4E") || // Himawari Doubutsu Byouin - Pet no Oishasan Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:10E3BAA340A34F6276D892C66FA3CE4002D6E5E4") || // Hitsuji no Kimochi. (Japan)	GBA		
				(romHashSHA1 == "SHA1:1CB9F418005404A244E1E86DEDE6AFF0E58CC114") || // Horsez (USA)	GBA		
				(romHashSHA1 == "SHA1:928628F8486E9390BEC5D039ED30751F56C40D58") || // Hyper Sports 2002 Winter (Japan)	GBA		
				(romHashSHA1 == "SHA1:86F8398FEABA9CFA0890DEFCD5301F190807838B") || // International Superstar Soccer (Europe)	GBA		
				(romHashSHA1 == "SHA1:B5D8A4CD3B092BF1E2AA480412E03D17B9E26F68") || // International Superstar Soccer Advance (Europe)	GBA		
				(romHashSHA1 == "SHA1:C49742F964A698CCCE0520E24368A9D425A29B4D") || // It's Mr. Pants (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:69291E5D230A8663C1FFD0298430651E64ACAFDE") || // J.League Winning Eleven Advance 2002 (Japan)	GBA		
				(romHashSHA1 == "SHA1:433F41B3C553AD9F943D9FCD2FC37ACFE564241F") || // Jikkyou World Soccer Pocket (Japan)	GBA		
				(romHashSHA1 == "SHA1:C1354FA67E2B5BDBE647EB60D301C14392ED2B4D") || // Jikkyou World Soccer Pocket 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:E1114ED7A3AF288733C5D3E8DD038713CB7FF8D2") || // K-1 Pocket Grand Prix (Japan)	GBA		
				(romHashSHA1 == "SHA1:4901C3426D12ABE6257A300E742CA9AC2E9565A6") || // K-1 Pocket Grand Prix 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:03953917BEAD3D30BA63676565CCB854008EF8DA") || // Kaiketsu Zorori to Mahou no Yuuenchi - Ohimesama o Sukue! (Japan)	GBA		
				(romHashSHA1 == "SHA1:418D61DDD992CEEAA10A102DEAF9EB15A2FB7328") || // Kawa no Nushi Tsuri 5 - Fushigi no Mori kara (Japan)	GBA		
				(romHashSHA1 == "SHA1:8FCB335821AAFD366431103CE2CC14E08898CF75") || // Kawaii Pet Shop Monogatari 3 (Japan)	GBA		
				(romHashSHA1 == "SHA1:9F6B694A159A3D4E3DEFAAA17E58410D4228CB26") || // Kidou Senshi Gundam Seed - Tomo to Kimi to Koko de. (Japan)	GBA		
				(romHashSHA1 == "SHA1:1A93EDF81CA484201C97E365777186714694C077") || // Kidou Tenshi Angelic Layer - Misaki to Yume no Tenshi-tachi (Japan)	GBA			
				(romHashSHA1 == "SHA1:D49FF33CD9FC488C5A018D9F3476FC96C82D778E") || // Kikaika Guntai - Mech Platoon (Japan)	GBA		
				(romHashSHA1 == "SHA1:161B95088373FBFB2B1B40EC63ECC2B4F8D540FB") || // Kisekko Gurumii - Chesty to Nuigurumi-tachi no Mahou no Bouken (Japan)	GBA		
				(romHashSHA1 == "SHA1:ABBBE9355759D94E98210E91A67585D0AF8CFC8F") || // Kiss x Kiss Seirei Gakuen (Japan)	GBA		
				(romHashSHA1 == "SHA1:323465C8A36517A1E8B0DDE2D500BBCD4C396D4E") || // Klonoa Heroes - Densetsu no Star Medal (Japan)	GBA		
				(romHashSHA1 == "SHA1:9452C587A00706D6758C503752F2D647D9104817") || // Koinu-chan no Hajimete no Osanpo - Koinu no Kokoro Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:EB352A51E2E7CD54906A60ABB4098D4B22AC6312") || // Konchuu Monster Battle Master (Japan)	GBA		
				(romHashSHA1 == "SHA1:C1EA541B031E41E6AACFB6EA16EAACDB24483E18") || // Konchuu Monster Battle Stadium (Japan)	GBA		
				(romHashSHA1 == "SHA1:AE82F41B61A054EAC8EEC4850C0AD0E6C1A1F77A") || // Konjiki no Gashbell!! - Unare! Yuujou no Zakeru (Japan)	GBA		
				(romHashSHA1 == "SHA1:5CE26F86DA63D9C662B6ACE75E327D033B152078") || // Konjiki no Gashbell!! The Card Battle for GBA (Japan)	GBA		
				(romHashSHA1 == "SHA1:3F7855C918CB7050E4EFB0355855EA14F2FBB7FA") || // Lea - Passion Veterinaire (France) (En,Fr)	GBA		
				(romHashSHA1 == "SHA1:722B70F7AC87BE0B8B6576FE8C8A82279F955E50") || // Legend of Spyro, The - A New Beginning (Europe) (En,Fr,De,Es,It,Nl)	GBA		
				(romHashSHA1 == "SHA1:F5EA1DE26E8E7E342B9D579ADB427934B2EC22A4") || // Legend of Spyro, The - A New Beginning (USA)	GBA		
				(romHashSHA1 == "SHA1:A272055ABBBF6C26B0CD54C87395D01699589161") || // Legend of Zelda, The - A Link to the Past & Four Swords (USA, Australia)	GBA		
				(romHashSHA1 == "SHA1:3BB7F97ADA7D60D8D334B271084EFFD1DB853B32") || // Legend of Zelda, The - A Link to the Past & Four Swords (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:63FCAD218F9047B6A9EDBB68C98BD0DEC322D7A1") || // Legend of Zelda, The - The Minish Cap (USA) (Demo) (Kiosk)	GBA		
				(romHashSHA1 == "SHA1:B4BD50E4131B027C334547B4524E2DBBD4227130") || // Legend of Zelda, The - The Minish Cap (USA)	GBA		
				(romHashSHA1 == "SHA1:CFF199B36FF173FB6FAF152653D1BCCF87C26FB7") || // Legend of Zelda, The - The Minish Cap (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:B8138BAA3540AA5B122FB3901770CA98517F2411") || // Let's Ride! - Dreamer (USA)	GBA		
				(romHashSHA1 == "SHA1:703F325EFCCBAB71E6AA16AC6D22DD9DE3E0F9B6") || // Let's Ride! - Friends Forever (USA)	GBA		
				(romHashSHA1 == "SHA1:29FA6CFF3DF328E45925343B8C73C1DB2C3E16C7") || // Let's Ride! - Sunshine Stables (USA)	GBA			
				(romHashSHA1 == "SHA1:2398C5A41BA0DCE5467981A56CD38AF52F5A9F35") || // Lilliput Oukoku - Lillimoni to Issho Puni! (Japan)	GBA		
				(romHashSHA1 == "SHA1:8D121CFEB6C88A4CF2D34F8E9979811D60543D6D") || // Little Patissier - Cake no Oshiro (Japan)	GBA		
				(romHashSHA1 == "SHA1:50786B17708D91C05EC3C3058A3A1106F524FAE3") || // Lizzie McGuire (Europe) (En,Fr,De,Es)	GBA		
				(romHashSHA1 == "SHA1:084ADD8893BAE03836D33A1C7BDEDE5F12741663") || // Lizzie McGuire - On the Go! (USA)	GBA		
				(romHashSHA1 == "SHA1:F330DE5440F561E9FB0F808876261EE124230F9F") || // Lord of the Rings, The - The Fellowship of the Ring (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:4B77BA458B593CA35166E9A9792792AC92E263DF") || // Lord of the Rings, The - The Fellowship of the Ring (USA)	GBA		
				(romHashSHA1 == "SHA1:C2B709A07E8BDBAB265500EA4AE740C123BF944B") || // Lord of the Rings, The - The Fellowship of the Ring (USA) (rev 1)	GBA		
				(romHashSHA1 == "SHA1:483B2C37330CF914C121801ACD9F15776F92EB29") || // Lunar Legend (Japan)	GBA		
				(romHashSHA1 == "SHA1:D7109FA4A03D50E24BD9E61446BDEB03B42732FF") || // Lunar Legend (USA)	GBA		
				(romHashSHA1 == "SHA1:4771F0A36D52F59F02E9DA1F87CC89D9E179BA9F") || // Madden NFL 06 (USA)	GBA		
				(romHashSHA1 == "SHA1:FD62EDCF3C30B8F2A821A5DBFE67AA50F150FEAE") || // Madden NFL 07 (USA)	GBA		
				(romHashSHA1 == "SHA1:E2F4E66FA582303D06F79E572E19E80462A7CC42") || // Madden NFL 2003 (USA)	GBA		
				(romHashSHA1 == "SHA1:8C932F469D7D66F604FA6A34ADFA3ACEA5B7F63E") || // Madden NFL 2004 (USA)	GBA		
				(romHashSHA1 == "SHA1:8D549EACD9C6F182743FE9CE442AB3C2F9048939") || // Madden NFL 2005 (USA)	GBA		
				(romHashSHA1 == "SHA1:870CDE605033525BE0D250F87BFA68D7D56AE372") || // Magi Nation (Japan)	GBA		
				(romHashSHA1 == "SHA1:78C57D1062830074166DA3192B01D0D0AE9AFAEB") || // Manga-ka Debut Monogatari (Japan) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:5F7C1B88B8C7CAA57A0AF81CD3AE74171ABE50D1") || // Manga-ka Debut Monogatari (Japan)	GBA		
				(romHashSHA1 == "SHA1:0DBDE8BCAA9E4BDC201674A0D66C6CE64242EB4B") || // Marie, Elie & Anis no Atelier - Soyokaze kara no Dengon (Japan)	GBA		
				(romHashSHA1 == "SHA1:FA2314C2FBE0DB1AB17175F8BE7CCEB0AB084EFC") || // Mario & Luigi - Superstar Saga (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:7C303CDDE5061EE329296948060B875CB50BA410") || // Mario & Luigi - Superstar Saga (USA, Australia)	GBA		
				(romHashSHA1 == "SHA1:EDC538AC505BFCD337ABDD03B3A6F2744D81EAAB") || // Mario & Luigi RPG (Japan)	GBA		
				(romHashSHA1 == "SHA1:F99EB117FF81995B22E19A34437938113E32C69A") || // Mario Party Advance (Japan)	GBA		
				(romHashSHA1 == "SHA1:FA917D74D592260C3D6142A969FCBD94156F956B") || // Mario Party Advance (USA)	GBA		
				(romHashSHA1 == "SHA1:5D67695C08AFCE4098264BD986A55739CB6636CB") || // Mario Party Advance (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:E969A8099E16F181FA7EE4D9CDA1A2DC0A029938") || // Matantei Loki Ragnarok - Gensou no Labyrinth (Japan)	GBA		
				(romHashSHA1 == "SHA1:3489569FA8A3C6A750C5CB0048915DA78C5f40AB") || // Matantei Loki Ragnarok - Gensou no Labyrinth (Japan) (Rev 1)	GBA	
				(romHashSHA1 == "SHA1:2DE038BB6505458F5E70A759EACCB62253AF3132") || // Mazes of Fate (USA) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:A5190C06F34B1617A6BD102C5D4829F8462D589B") || // Mech Platoon (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:8A9BD790C9CD831366A00CDDE72A9F3FEC89CB61") || // Mech Platoon (USA)	GBA		
				(romHashSHA1 == "SHA1:CD3D674E88F40A0707B150C4293588A659001D29") || // Medabots - Metabee (Europe)	GBA		
				(romHashSHA1 == "SHA1:CA185B65AB50EF89A10C8DB00D9CD76626B81610") || // Medabots - Metabee (USA)	GBA		
				(romHashSHA1 == "SHA1:FEE2D1DB994E1A13FC2BC9943198136192412185") || // Medabots - Metabee (Spain)	GBA		
				(romHashSHA1 == "SHA1:90FE7F2927C592AABC9B33D0DF00D92046C5CB92") || // Medabots - Rokusho (Spain)	GBA		
				(romHashSHA1 == "SHA1:FC44B80F3E71EFFC33D8E05A74888CB885C32EB0") || // Medabots - Rokusho (Europe)	GBA		
				(romHashSHA1 == "SHA1:C4572428EA97B302F699A3B4EBA2A1F0E87C1C9C") || // Medabots - Rokusho (USA)	GBA		
				(romHashSHA1 == "SHA1:CB1E070C2CF9BA7C7CE7C102E69E7815941CA5F1") || // Medarot Ni Core - Kabuto (Japan)	GBA		
				(romHashSHA1 == "SHA1:868157B4D047F7F5F2A9C30577D082DC59B763B2") || // Medarot Ni Core - Kuwagata (Japan)	GBA		
				(romHashSHA1 == "SHA1:5D6F8FB1F52803A54E9857E53D0B88173CF8F48A") || // Mega Man & Bass (Europe)	GBA		
				(romHashSHA1 == "SHA1:7610847B331870D4338E5AC894B36E55E2BEC5A0") || // Mega Man & Bass (USA)	GBA		
				(romHashSHA1 == "SHA1:8E13B9EE89A2ED665212DAA401BA9331AD11BDA9") || // Mega Man Zero 4 (Europe)	GBA		
				(romHashSHA1 == "SHA1:596993205A1895A6F51E80749407FB069B907628") || // Mega Man Zero 4 (USA)	GBA		
				(romHashSHA1 == "SHA1:B6FCF6A7985E3FF834D0BF9B384F750EAE067EFC") || // Meine Tierarztpraxis (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:09D511ECB93BCEB5CC73E23B8711392F31D97A32") || // Meine Tierpension (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:D971B2FBBCB0F6CB378A226902FE44F8CFEB99EB") || // Meitantei Conan - Akatsuki no Monument (Japan)	GBA		
				(romHashSHA1 == "SHA1:3636C94C319C26903E1F05E3CD63D18F0BFC439C") || // Meitantei Conan - Nerawareta Tantei (Japan)	GBA		
				(romHashSHA1 == "SHA1:B827FF8B0119F763BDC32062E77E6ED8D44985BA") || // Metal Max 2 Kai (Japan) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:73C58F67E1FF42CFFA30970B6F226C352635F07D") || // Metal Max 2 Kai (Japan)	GBA		
				(romHashSHA1 == "SHA1:DBF9E8515D9993C6FBAB921AF65CBBDEABD4AA93") || // Metalgun Slinger (Japan)	GBA		
				(romHashSHA1 == "SHA1:DE0E1F47F4CBFBB1EF8EBD67289AC978E8F28317") || // Mijn Dierenpension (Netherlands) (En,Nl)	GBA		
				(romHashSHA1 == "SHA1:245D9370179D21B7EA69ECE2DC9B6CEB6294F99D") || // Mijn Dierenpraktijk (Netherlands) (En,Nl)	GBA		
				(romHashSHA1 == "SHA1:D57D2035445683B842E6FB5C7F330C82AFB7EFCF") || // Minna no Shiiku Series - Boku no Kabuto, Kuwagata (Japan)	GBA		
				(romHashSHA1 == "SHA1:D3B1F600EB4B0D10D144C30A5710AC2CF1E46296") || // Minna no Soft Series - Numpla Advance (Japan)	GBA		
				(romHashSHA1 == "SHA1:5DD2C153D0DEC9DF127256A0426C12F188D813D6") || // Minna no Soft Series - Zooo (Japan)	GBA		
				(romHashSHA1 == "SHA1:ACB39C3DA3C93247E6C2FD7FBDA7DF2FC720CDD7") || // Momotarou Dentetsu G Gold Deck o Tsukure! (Japan)	GBA		
				(romHashSHA1 == "SHA1:E85CEA8428D51E40CF1B7A90643E94604973679F") || // My Animal Centre in Africa (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:1FB49BB2293DBBD061A5D72D65D413B22360CCED") || // Nakayoshi Pet Advance Series 1 - Kawaii Hamster (Japan)	GBA		
				(romHashSHA1 == "SHA1:AEF6AAA2D351BAD2C260B85C308D584B91039A99") || // Nakayoshi Pet Advance Series 2 - Kawaii Koinu (Japan)	GBA		
				(romHashSHA1 == "SHA1:B713F9658F8F5C24C1BC1C840BD74AE4B3EDD2D3") || // Nakayoshi Pet Advance Series 3 - Kawaii Koneko (Japan)	GBA		
				(romHashSHA1 == "SHA1:F0AC77BEC4F385FEEA5305BB1C67EFEFFCFC6AC2") || // Nakayoshi Pet Advance Series 3 - Kawaii Koneko (Japan) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:0C78A24D5A8A296D05A8B47CF5623DE0F031748E") || // Nakayoshi Pet Advance Series 4 - Kawaii Koinu Mini - Wankoto Asobou!! Kogata-ken (Japan)	GBA		
				(romHashSHA1 == "SHA1:77743DE7541E0A9D09E3AAC864C4E3F9F1A51D2B") || // Nakayoshi Youchien - Sukoyaka Enji Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:5E0CDBD4B6ABA17E26C262D8DB7C64FC3441D263") || // Nakayoshi Youchien - Sukoyaka Enji Ikusei Game (Japan) (Rev 1)	GBA	
				(romHashSHA1 == "SHA1:B231938A269F7D9A058560BC685D13B010D81812") || // Neoromance Game - Harukanaru Toki no Naka de (Japan) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:BD87E466168127D11231C75C47DFE09D42C03198") || // Neoromance Game - Harukanaru Toki no Naka de (Japan)	GBA		
				(romHashSHA1 == "SHA1:47F54DA7424514CF83C794C49811EEFBB35DA72D") || // NHL 2002 (USA)	GBA		
				(romHashSHA1 == "SHA1:690A6F8A08B2D45C4E37C6FF28C90DDA85D91174") || // Ochaken no Bouken-jima - Honwaka Yume no Island (Japan)	GBA		
				(romHashSHA1 == "SHA1:36EF5248F9873C7812D93572242D01512D67F41C") || // Ohanaya-san Monogatari GBA - Iyashikei Ohanaya-san Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:883277CD8926FCEF9418B59F3F64E7A5BB218206") || // One Piece - Dragon Dream (Japan)	GBA		
				(romHashSHA1 == "SHA1:F33C77986A7F6CF461A71933D0006E18736F7E17") || // One Piece - Going Baseball - Haejeok Yaku (Korea)	GBA		
				(romHashSHA1 == "SHA1:18CF10529BB6E2D113CA586D2DAE3EB23AD496CF") || // One Piece - Going Baseball - Kaizoku Yakyuu (Japan)	GBA		
				(romHashSHA1 == "SHA1:F652C211CC64D578B7F15D6AFD112E2916181E7C") || // Onimusha Tactics (Japan)	GBA		
				(romHashSHA1 == "SHA1:247853BAE3771F75725E5D212F6B3A416B749E07") || // Onimusha Tactics (Europe)	GBA		
				(romHashSHA1 == "SHA1:83ABEE3AA535590F666D7E9A6971640D8A41AA9D") || // Onimusha Tactics (USA)	GBA		
				(romHashSHA1 == "SHA1:47B04F671AF34E8449856BDFC3ACC76076C2273A") || // Onmyou Taisenki Zeroshiki (Japan)	GBA		
				(romHashSHA1 == "SHA1:CF47E9FDB8DEC4833D8DD1AE00FA2F222BD1D7DA") || // Oshare Princess (Japan)	GBA		
				(romHashSHA1 == "SHA1:649001F85CDCD2515183FC1364D08DE52C918FEB") || // Oshare Princess 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:923E4B22C5A9DE6C95A02A4B948A962B09DBE8C3") || // Oshare Princess 3 (Japan)	GBA		
				(romHashSHA1 == "SHA1:262389579D4D60337097015F236C9AA098364E69") || // Oshare Wanko (Japan)	GBA		
				(romHashSHA1 == "SHA1:0C5261A97FE9555C5705D78BE73402D4FA34EBF8") || // Paard & Pony - Mijn Manege (Netherlands) (En,Nl)	GBA		
				(romHashSHA1 == "SHA1:38AE3B180AD31E7FD0787F5B7FE983A4BACD6C03") || // Paard & Pony - Paard in Galop (Netherlands) (En,Nl)	GBA		
				(romHashSHA1 == "SHA1:2D024EA3621486DD0CF59B801A364649FB751C49") || // Paws & Claws - Best Friends - Dogs & Cats (USA)	GBA		
				(romHashSHA1 == "SHA1:5C75C67DAE18BD77CA2BB05A76782A14806D45A7") || // Paws & Claws - Pet Resort (USA)	GBA		
				(romHashSHA1 == "SHA1:9755A6EAF65D10131E591C4C75B9C77554CD1DAD") || // Petz Vet (USA)	GBA		
				(romHashSHA1 == "SHA1:87332FF941902A413D3AA0A4E3951122BA111A96") || // Pferd & Pony - Best Friends - Mein Pferd (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:32EC6383F46A3195BE58ABB82D8FF391B1634371") || // Pferd & Pony - Lass Uns Reiten 2 (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:27F5E01E2BDDEC90E26C853A29405FE113F87180") || // Pferd & Pony - Mein Gestuet (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:51152C4CA5A2E8C8680DB8F70501452C967D8320") || // Pferd & Pony - Mein Pferdehof (Germany) (En,De)	GBA		
				(romHashSHA1 == "SHA1:8B8B31A2BC023121205F4BB87F20240FC0A98C66") || // Phantasy Star Collection (Europe)	GBA		
				(romHashSHA1 == "SHA1:9F2DC591C9B1526F9F965B1C375FB4EA7101FD16") || // Phantasy Star Collection (USA)	GBA		
				(romHashSHA1 == "SHA1:4BAFBC7A57851DDD330620E9F448714FFA7ACB3B") || // Pia Carrot e Youkoso!! 3.3 (Japan)	GBA		
				(romHashSHA1 == "SHA1:58B8D9468FD43E876D234491B2BF1A6775D95D23") || // Pikapika Nurse Monogatari - Nurse Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:39963429361B42438D3BC1106AF0D345F3146B52") || // Pinky Monkey Town (Japan)	GBA		
				(romHashSHA1 == "SHA1:D48F95CB74D8097C143870CF3026B9426D15E4F8") || // Pippa Funnell - Stable Adventure (Europe) (En,Fr)	GBA		
				(romHashSHA1 == "SHA1:B7FD71D22DA6BDC16B8F780FC8D10587C3DFF41A") || // Pippa Funnell 2 (Europe) (En,Fr)	GBA		
				(romHashSHA1 == "SHA1:3F066EB236E8FA22E8507CA9DC502BE85CCAFEC3") || // Premier Action Soccer (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:709071848ABD706FD172381C27516C79F8EB796B") || // Pro Mahjong Tsuwamono GBA (Japan)	GBA		
				(romHashSHA1 == "SHA1:E2A86CEB717D56C882AD4EEBC015DC1329D8F1BC") || // PukuPuku Tennen Kairanban (Japan)	GBA		
				(romHashSHA1 == "SHA1:AD0A0693FF0C6B4E14150F68BC094851E4927322") || // PukuPuku Tennen Kairanban - Koi no Cupid Daisakusen (Japan)	GBA		
				(romHashSHA1 == "SHA1:B2BD07BF70D3BBD05C031BAA482E891F245EBAD2") || // PukuPuku Tennen Kairanban - Youkoso! Illusion Land he (Japan)	GBA		
				(romHashSHA1 == "SHA1:4E81530B84BDEFD3D4BA7AF878F19301FCA3CE91") || // Puzzle & Tantei Collection (Japan)	GBA		
				(romHashSHA1 == "SHA1:37714E724942C9C0882D1F7FC48A8DC449A68F83") || // Rayman - 10th Anniversary (Europe) (En,Fr,De,Es,It,Nl,Sv,No,Da,Fi)	GBA		
				(romHashSHA1 == "SHA1:1983C17260F5707B26702C3AB1B6C86332365E05") || // Rayman - 10th Anniversary (USA) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:E2556BCE89C4B542786C741DC49EF28AFAB917FF") || // Rebelstar - Tactical Command (USA)	GBA		
				(romHashSHA1 == "SHA1:669C47E62F2388CE0BA676F3DC641B5AD2DDB5C5") || // Rebelstar - Tactical Command (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:64D0B723AE4EB3F3589B535FB935618F2769229F") || // River City Ransom EX (USA)	GBA		
				(romHashSHA1 == "SHA1:F8EB056745E8B58C1EF4BFEF995C77A27AADC57A") || // Rockman & Forte (Japan)	GBA		
				(romHashSHA1 == "SHA1:DC554ADCD18EB78132B4FB724A1C69779F75D114") || // Rockman Zero 4 (Japan)	GBA		
				(romHashSHA1 == "SHA1:49084091F79DFEC46B4FBCE62E897985CAFBC147") || // Sabre Wulf (Europe) (En,Fr,De)	GBA		
				(romHashSHA1 == "SHA1:D8803E742253E4B6712BDF07D49D3D7ED7FB23D6") || // Sabre Wulf (USA)	GBA		
				(romHashSHA1 == "SHA1:D4023E0A54A48999A46BF6B63C3BB91E3A795738") || // Samsara Naga 1x2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:B18E525A3A4B008D76A9C5A7B47CDD4A03B0BAEC") || // Samsara Naga 1x2 (Japan) (Rev 2)	GBA		
				(romHashSHA1 == "SHA1:1BA3B2D7A34488CDA9645A3D470CF6249DE6C5D8") || // Sanrio Puroland - All Characters (Japan)	GBA		
				(romHashSHA1 == "SHA1:7DD42BEAACBD24CB832745A18E57CEFDA4A76827") || // SD Gundam G Generation Advance (Japan)	GBA		
				(romHashSHA1 == "SHA1:B4EF625996F413FC563BD3319AFF1DC6C1D2DCFE") || // Sea Trader - Rise of Taipan (USA)	GBA		
				(romHashSHA1 == "SHA1:CB200350B4DC01DCDB0A52AF11286AACA15CD786") || // Sheep (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:FF674A429E8A76CAB47ACCAF6CA579194866733F") || // Shikakui Atama o Maruku Suru. Advance - Kanji, Keisan (Japan)	GBA		
				(romHashSHA1 == "SHA1:C1CF1478D79DAF6C2717DF6F19CAD806BD7F6429") || // Shikakui Atama o Maruku Suru. Advance - Kokugo, Sansuu, Shakai, Rika (Japan)	GBA	
				(romHashSHA1 == "SHA1:B2F100650AF2C8D6EC02C0A71774EEBA22D6BE2F") || // Shimura Ken no Baka Tonosama - Bakushou Tenka Touitsu Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:C7C94199BAC6A1F69356413031B5A49CB30C93D1") || // Shimura Ken no Baka Tonosama - Bakushou Tenka Touitsu Game (Japan) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:069B7E991A35A78B321959F2F19470952C331DFC") || // Shin chan - Aventuras en Cineland (Spain)	GBA		
				(romHashSHA1 == "SHA1:A328AE26F28D9500DD98C8484E35F44EDC5EF4EA") || // Shin chan contra los Munecos de Shock Gahn (Spain)	GBA		
				(romHashSHA1 == "SHA1:E70FC457B56C8E27AB63E491706B88232F598452") || // Shin Megami Tensei Devil Children - Puzzle de Call! (Japan)	GBA		
				(romHashSHA1 == "SHA1:C14E7612698342CCD495E4AEF764D4083C5A4765") || // Shingata Medarot - Kabuto Version (Japan)	GBA		
				(romHashSHA1 == "SHA1:573E980DFFDEE50D53B6F2A79D1DB2D650731558") || // Shingata Medarot - Kuwagata Version (Japan)	GBA			
				(romHashSHA1 == "SHA1:AD31CB6237A888B51585CCD58077B5F8317CE192") || // SimCity 2000 (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:8999C32301EF811DA086E9599D7E7D6CF1533A70") || // SimCity 2000 (USA)	GBA		
				(romHashSHA1 == "SHA1:D3842F8EE2F82998FC3BAB0C6DD18F7CFC442B11") || // SimCity 2000 (USA) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:C5EBA83DB30B6B9AA2B770070F729D8D194A55B9") || // SimCity 2000 (Europe) (En,Fr,De,Es,It) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:227857E4AE380668CBC467F5A7A30C6A3A77A180") || // Sims, The (Japan)	GBA		
				(romHashSHA1 == "SHA1:FB5662172FADEBF4577F6F6F163741FC041C11E6") || // Sims, The - Bustin' Out (USA, Europe) (En,Fr,De,Es,It,Nl)	GBA		
				(romHashSHA1 == "SHA1:B3677ED0C4DDC146897A449C03DF996E8FC0DF3C") || // Slot! Pro 2 Advance - GoGo Juggler & New Tairyou (Japan)	GBA		
				(romHashSHA1 == "SHA1:47440FA1DC5525595A1D95BDE4CCDD5755365138") || // Slot! Pro Advance - Takarabune & Ooedo Sakurafubuki 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:286FE14E15BC34E3605186876878E655EF43294C") || // Spyro - Attack of the Rhynocs (USA)	GBA		
				(romHashSHA1 == "SHA1:923EDAAF9CF7DBBD3C68CC4FFB171C4E308687AC") || // Spyro 2 - Season of Flame (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:53E77C7E4A895E0652EC2CA2C9B4A590E84ABDCA") || // Spyro 2 - Season of Flame (USA)	GBA		
				(romHashSHA1 == "SHA1:0F4AA43D6E0AD5B5007A6114F679EE0FDF77793F") || // Spyro Adventure (Europe) (En,Fr,De,Es,It,Nl)	GBA		
				(romHashSHA1 == "SHA1:335C216BEC0FF27BCCDADB4E97C623485EA5902A") || // Street Fighter Alpha 3 (Europe)	GBA		
				(romHashSHA1 == "SHA1:7FD4258BCD2BF639E1EC98B7A7EC7218FCF5859E") || // Street Fighter Alpha 3 (USA)	GBA		
				(romHashSHA1 == "SHA1:5E8233351ED25A6879F5FF7596B29B45DCA24C12") || // Street Fighter Zero 3 Upper (Japan)	GBA		
				(romHashSHA1 == "SHA1:D98F17F4D276E179A057326507636DA1D5BE93F4") || // Sugar Sugar Rune - Heart Ga Ippai! Moegi Gakuen (Japan)	GBA		
				(romHashSHA1 == "SHA1:B137E310696BE2A148E6201320FCDA3551F32E8A") || // Summon Night - Craft Sword Monogatari (Japan)	GBA		
				(romHashSHA1 == "SHA1:3F5253FCF57E07CE52472BD29A61D16B98A12376") || // Summon Night - Craft Sword Monogatari - Hajimari no Ishi (Japan)	GBA		
				(romHashSHA1 == "SHA1:738C64FEC6ECF9BCCB91500641FBB8BA37E865DA") || // Summon Night - Craft Sword Monogatari 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:3FA7E703D48E070D9C762C2404E91A566A50166E") || // Summon Night - Swordcraft Story (USA)	GBA		
				(romHashSHA1 == "SHA1:CBC4382B1FFF2E1B2D3112AE8FBF290AEC662121") || // Summon Night - Swordcraft Story 2 (USA)	GBA		
				(romHashSHA1 == "SHA1:CD2A45C2AF945D5861684060F087DB44024DFE59") || // Super Donkey Kong 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:5101DDF223D1D918928FE1F306B63A42ADA14A5E") || // Super Mario Advance 2 - Super Mario World (USA, Australia)	GBA		
				(romHashSHA1 == "SHA1:199A5A01BD3F56FBA9A701062EDBCF57E1528A0C") || // Super Mario Advance 2 - Super Mario World (Europe) (En,Fr,De,Es)	GBA		
				(romHashSHA1 == "SHA1:8F3D3C33C77872DB9818620F5E581EC0FA342D72") || // Super Mario Advance 2 - Super Mario World + Mario Brothers (Japan)	GBA		
				(romHashSHA1 == "SHA1:7352D2BD064D9EBAEC579E264228AA21C7345B80") || // Super Mario Advance 3 - Yoshi's Island (USA)	GBA		
				(romHashSHA1 == "SHA1:BD52EB4B4EBE438E9B9ECAAC792BD389725CDE41") || // Super Mario Advance 3 - Yoshi's Island (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:8E6D22AD9E4634E43F3AA0D2A1A33330E2EB1E04") || // Super Mario Advance 3 - Yoshi's Island + Mario Brothers (Japan)	GBA		
				(romHashSHA1 == "SHA1:96610003FDB49A14B35AEEDF0C8CEBF5F80D8E4A") || // Sweet Cookie Pie (Japan)	GBA			
				(romHashSHA1 == "SHA1:17E0F822E2CE33A27B809C592DC47F1BB4C0277F") || // Sylvanian Families - Fashion Designer ni Naritai! - Kurumi-risu no Onnanoko (Japan)	GBA		
				(romHashSHA1 == "SHA1:E52C6B7076BC291035F5D68BEFA6F579E6B4DCF1") || // Sylvanian Families - Yousei no Stick to Fushigi no Ki - Marron-inu no Onnanoko (Japan)	GBA		
				(romHashSHA1 == "SHA1:2667F3F61E9AA5A2852299DEE6BC66D222549C0A") || // Sylvanian Families 4 - Meguru Kisetsu no Tapestry (Japan)	GBA		
				(romHashSHA1 == "SHA1:77823C36966C2330628CA2F03FCF681A1D3F9AFA") || // Tales of Phantasia (Japan)	GBA		
				(romHashSHA1 == "SHA1:262E9393119CB1991A27A3DDD5A070AE3C60CEE9") || // Tales of Phantasia (USA, Australia)	GBA		
				(romHashSHA1 == "SHA1:2CC3C21C99C48DC445F499E6CC8AA9B90B2C795B") || // Tales of Phantasia (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:2FEB4CFF9485C68758C1FAC847C6EB907E747A01") || // Tales of the World - Narikiri Dungeon 2 (Japan)	GBA		
				(romHashSHA1 == "SHA1:263B5BA40B1E0AFBC2C23F478CC83F794846A47F") || // Tales of the World - Narikiri Dungeon 3 (Japan)	GBA		
				(romHashSHA1 == "SHA1:C7BDA17313FDEF597CCEC98502E71C7E61281C9B") || // Tales of the World - Summoner's Lineage (Japan)	GBA		
				(romHashSHA1 == "SHA1:B84BD4C83AB1EF78BC659B3917EF7C89DEF8BA6C") || // Tantei Jinguuji Saburou - Shiroi Kage no Shoujo (Japan)	GBA		
				(romHashSHA1 == "SHA1:EB7741AA8E5A26DC3426AC4E541638122496E8C1") || // Teenage Mutant Ninja Turtles 2 - Battle Nexus (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:F42D0131E908942BF84D2323739CE015EE930142") || // Teenage Mutant Ninja Turtles 2 - Battle Nexus (USA)	GBA		
				(romHashSHA1 == "SHA1:15CBFF45CE0CB35F3E6A6059ADA2710F0CEC05C3") || // Teenage Mutant Ninja Turtles Double Pack (USA) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:68309670F0BB0A5496DDE3FCBB193EEF10C06F87") || // Teenage Mutant Ninja Turtles Double Pack (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:76AD4D03B90ED8E2C1E657BC43954942C1FED18B") || // Tokimeki Yume Series 1 - Ohanaya-san ni Narou! (Japan)	GBA		
				(romHashSHA1 == "SHA1:8772376A8F30DB060915CDC2C19EAECC17BAC043") || // Tomato Adventure (Japan)	GBA		
				(romHashSHA1 == "SHA1:9EF930C0DBF6EA2AE3C0DA95659CA0E2F4FA7B88") || // Tony Hawk's American Sk8land (USA)	GBA		
				(romHashSHA1 == "SHA1:A51F1D72F136EB43F70620350F7487CC57D7A695") || // Tony Hawk's American Sk8land (Europe) (Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:14D46D7544F8503273223CAB91DBB2C698987A54") || // Tony Hawk's American Sk8land (Europe)	GBA		
				(romHashSHA1 == "SHA1:FBDEF1C6C4704EAE2103BBA4DC85F9284C785DFD") || // Tony Hawk's Pro Skater 3 (Germany)	GBA		
				(romHashSHA1 == "SHA1:62BA6F28F5BBD8FE42ECABECDCFF4D6B96AE4F21") || // Tony Hawk's Pro Skater 3 (France)	GBA		
				(romHashSHA1 == "SHA1:D1D6806AC99A29DD3195491A213BEC9023A3ADB4") || // Tony Hawk's Pro Skater 3 (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:F4BA0BE44899EB8D52869120FBBC4F46E2A3C668") || // Tony Hawk's Pro Skater 4 (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:54584741D76D2BD136CC56D5B858F6A3BCD81E0B") || // Tony Hawk's Underground (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:C684D839C32515A99E62B183B607E2259120CFC9") || // Tony Hawk's Underground 2 (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:EA25201B0DB54C18E0476ACB017DAF57BCDD2AB0") || // Touhai Densetsu Akagi - Yami ni Mai Orita Tensai (Japan)	GBA		
				(romHashSHA1 == "SHA1:5DC55476F13E941B5464E3ECB7C67299FDE681AD") || // Toukon Heat (Japan)	GBA			
				(romHashSHA1 == "SHA1:8B9F58C8C8A596DE233A97E7E104E6C2DF3B9595") || // Twin Series 4 - Hamu Hamu Monster EX - Hamster Monogatari RPG + Fantasy Puzzle - Hamster Monogatari - Mahou no Meikyuu 1.2.3 (Japan)	GBA		
				(romHashSHA1 == "SHA1:EAF8E1918BCD2AB3A0ABB6A88B63BA0C98EFC1FF") || // Twin Series 5 - Mahou no Kuni no Cake-ya-san Monogatari + Wanwan Meitantei EX (Japan)	GBA		
				(romHashSHA1 == "SHA1:84267CE3D86100688048A8D4F166FA1B2D50E6D5") || // Ty the Tasmanian Tiger 2 - Bush Rescue (USA, Europe) (En,Fr,De)	GBA		
				(romHashSHA1 == "SHA1:07FAFA1C96CC039A1788D6526D52F7D3EC0BA3C3") || // Ty the Tasmanian Tiger 3 - Night of the Quinkan (USA)	GBA		
				(romHashSHA1 == "SHA1:54DFF080E83B5C09FA314AFBE710205CBC30C28D") || // Uchuu Daisakusen Choco Vader - Uchuu kara no Shinryakusha (Japan)	GBA			
				(romHashSHA1 == "SHA1:CE7BE617DE6558C823B6CA77AF8CE2E14EA230C7") || // Ui-Ire - World Soccer Winning Eleven (Japan)	GBA		
				(romHashSHA1 == "SHA1:74650C32D727C85D5E78C903949F6937EA9F3EE7") || // Ultimate Arcade Games (USA)	GBA		
				(romHashSHA1 == "SHA1:22FB775D4B2673FF00877170FBC028B9696E4F8E") || // Ultimate Card Games (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:CD1CAA649BDFA43C838AAC8370D0E00CA746AB8A") || // Ultimate Card Games (USA, Europe) (Rev 1)	GBA		
				(romHashSHA1 == "SHA1:B44A36EF8B76D220172750650095ED2A3F3693EA") || // Ultimate Card Games (USA, Europe) (Rev 2)	GBA		
				(romHashSHA1 == "SHA1:951E7BC5D62DC10795806EC686B9F60116CAE8C7") || // Wagamama Fairy Mirumo de Pon! - Ougon Maracas no Densetsu (Japan)	GBA		
				(romHashSHA1 == "SHA1:993A4BD4C63318A11ABD54C006ECBA57C646107C") || // Wanko Mix Chiwanko World (Japan)	GBA		
				(romHashSHA1 == "SHA1:36786BFEDB96E003B2A9C17376FF7277E0F9660D") || // Wannyan Doubutsu Byouin - Doubutsu no Oishasan Ikusei Game (Japan)	GBA		
				(romHashSHA1 == "SHA1:D146E7D83965B8F05CD5C18C2B297A1B376AAF16") || // Wannyan Doubutsu Byouin - Doubutsu no Oishasan Ikusei Game (Japan) (Rev 1)	GBA	
				(romHashSHA1 == "SHA1:3FADDB2187EC863BD181ACCD8F8B04C36005AEB6") || // WinX Club (Europe) (En,Fr,De,Es,It)	GBA		
				(romHashSHA1 == "SHA1:3B3F5C633F48AC218E9DA6AC78305EAB9C17AB21") || // WinX Club (USA)	GBA		
				(romHashSHA1 == "SHA1:5D0C7EA259F44FF7F4645E2E80201C531D2F54E2") || // World Advance Soccer - Shouri e no Michi (Japan)	GBA		
				(romHashSHA1 == "SHA1:C75851D14A96DEA46770945AAFEAF9A8F6C12271") || // World Poker Tour (USA)	GBA		
				(romHashSHA1 == "SHA1:2F2983D33C547F09CCA512B06880BC1FAF1D4F54") || // World Poker Tour (Europe) (En,Fr,De)	GBA		
				(romHashSHA1 == "SHA1:D5ED374C8D353CF27EE9160F1C4C27A7DABE5845") || // Yaoxi Dao (China)	GBA		
				(romHashSHA1 == "SHA1:4AC5874B98F0FA3C3AAD4C3EBE927F7FBC5B3267") || // Yggdra Union (Japan)	GBA		
				(romHashSHA1 == "SHA1:E419C835200457F67D5AB7E3C236583980B922D9") || // Yggdra Union - We'll Never Fight Alone (Europe)	GBA		
				(romHashSHA1 == "SHA1:B289083B01F2F8FC726C664CF872D72B3F4986E3") || // Yggdra Union - We'll Never Fight Alone (USA)	GBA		
				(romHashSHA1 == "SHA1:ED6578BB6A4F201AE5B269DF5CE842158A34A421") || // Youkaidou (Japan)	GBA		
				(romHashSHA1 == "SHA1:DAB129544C5AB496DF4FD1DFBFA33E76142D2D8A") || // Yu Yu Hakusho - Ghostfiles - Tournament Tactics (USA, Europe)	GBA		
				(romHashSHA1 == "SHA1:96B67004D7592AE118936E8988C96880D9880F60") || // Zatchbell! - Electric Arena (USA)	GBA		
				(romHashSHA1 == "SHA1:6C5404A1EFFB17F481F352181D0F1C61A2765C5D") || // Zelda no Densetsu - Fushigi no Boushi (Japan)	GBA		
				(romHashSHA1 == "SHA1:FB3803F6A806154DC93541D17A6D53203CB339EE") || // Zelda no Densetsu - Kamigami no Triforce & 4tsu no Tsurugi (Japan)	GBA		
				(romHashSHA1 == "SHA1:42501ED8395DFCC87CC112D20183A9394D0B232F") || // Zero One (Japan)	GBA		
				(romHashSHA1 == "SHA1:7ED2B25DFF8783A924C5D6DFE2ADD95D766F32F0") || // Zero One SP (Japan)	GBA		
				(romHashSHA1 == "SHA1:9096450C4B29AB6E317D94CA09EE82F5B74DD4B1"))   // Zooo (Europe) (En,Fr,De,Es,It)	GBA		

			{
				return true;
			}
			else
			{
				return false;
			}
		}

		public static readonly uint[] GBP_Check = { 0xFF8840D0, 0xFF6000C8, 0xFF6000C8, 0xFF6000C8, 0xFF6000C8, 0xFF6000C8, 0xFFA870E0, 0xFFF8F8F8 };
		public static readonly string GBP_SCreen_Hash = "SHA1:EBE758CA95050270173C716B2BDB97DABE0F7303";

		public static readonly uint[] GBP_TRansfer_List = { 0x0000494E, 0x0000494E, 0xB6B1494E, 0xB6B1544E, 0xABB1544E, 0xABB14E45, 0xB1BA4E45, 0xB1BA4F44, 0xB0BB4F44,
															0xB0BB8002, 0x10000010, 0x20000013, 0x30000003, 0x30000003, 0x30000003, 0x30000003, 0x30000003, 0x00000000 };


		public static bool Check_Video_GBP(int[] scr)
		{
			bool GBP_Screen_Detect = false;

			if ((uint)scr[240 * 80 + 60] == 0xFF8840D0)
			{
				GBP_Screen_Detect = true;

				for (int i = 1; i < 8; i++)
				{
					if ((uint)scr[240 * 80 + 60 + i] != GBP_Check[i])
					{
						GBP_Screen_Detect = false;
					}
				}
				if (GBP_Screen_Detect)
				{
					ReadOnlySpan<byte> byteRef = MemoryMarshal.AsBytes(scr.AsSpan());

					var ScreenSHA1 = SHA1Checksum.ComputePrefixedHex(byteRef);

					if (ScreenSHA1 != GBP_SCreen_Hash)
					{
						GBP_Screen_Detect = false;
					}
				}
			}

			return GBP_Screen_Detect;
		}


	}

	public class GBA_ControllerDeck
	{
		public GBA_ControllerDeck(string controller1Name, bool is_subrame = false)
		{
			Port1 = ControllerCtors.TryGetValue(controller1Name, out var ctor1)
				? ctor1(1)
				: throw new InvalidOperationException($"Invalid controller type: {controller1Name}");

			Definition = new(Port1.Definition.Name)
			{
				BoolButtons = Port1.Definition.BoolButtons
					.ToList()
			};

			foreach (var kvp in Port1.Definition.Axes) Definition.Axes.Add(kvp);

			if (is_subrame)
			{
				Definition.AddAxis("Reset Cycle", 0.RangeTo(280896), 280896);
			}

			Definition.HapticsChannels.Add("P1 Rumble");

			Definition.MakeImmutable();
		}

		public ushort ReadPort1(IController c)
		{
			return Port1.Read(c);
		}

		public (ushort X, ushort Y) ReadAcc1(IController c)
			=> Port1.ReadAcc(c);

		public byte ReadSolar1(IController c)
		{
			return Port1.SolarSense(c);
		}

		public ControllerDefinition Definition { get; }

		public void SyncState(Serializer ser)
		{
			Port1.SyncState(ser);
		}

		private readonly IPort Port1;

		private static IReadOnlyDictionary<string, Func<int, IPort>> _controllerCtors;

		public static IReadOnlyDictionary<string, Func<int, IPort>> ControllerCtors => _controllerCtors
			??= new Dictionary<string, Func<int, IPort>>
			{
				[typeof(StandardControls).DisplayName()] = portNum => new StandardControls(portNum),
				[typeof(StandardTilt).DisplayName()] = portNum => new StandardTilt(portNum),
				[typeof(StandardSolar).DisplayName()] = portNum => new StandardSolar(portNum),
				[typeof(StandardZGyro).DisplayName()] = portNum => new StandardZGyro(portNum)
			};

		public static string DefaultControllerName => typeof(StandardControls).DisplayName();
	}

	public class GBALink_ControllerDeck
	{
		public GBALink_ControllerDeck(string controller1Name, string controller2Name)
		{
			Port1 = GBA_ControllerDeck.ControllerCtors.TryGetValue(controller1Name, out var ctor1)
				? ctor1(1)
				: throw new InvalidOperationException($"Invalid controller type: {controller1Name}");
			Port2 = GBA_ControllerDeck.ControllerCtors.TryGetValue(controller2Name, out var ctor2)
				? ctor2(2)
				: throw new InvalidOperationException($"Invalid controller type: {controller2Name}");

			Definition = new ControllerDefinition(Port1.Definition.Name)
			{
				BoolButtons = Port1.Definition.BoolButtons
					.Concat(Port2.Definition.BoolButtons)
					.Concat(new[] { "Toggle Cable" })
					.ToList()
			};

			foreach (var kvp in Port1.Definition.Axes) Definition.Axes.Add(kvp);
			foreach (var kvp in Port2.Definition.Axes) Definition.Axes.Add(kvp);

			Definition.HapticsChannels.Add("P1 Rumble");
			Definition.HapticsChannels.Add("P2 Rumble");

			Definition.MakeImmutable();
		}

		public ushort ReadPort1(IController c)
		{
			return Port1.Read(c);
		}

		public ushort ReadPort2(IController c)
		{
			return Port2.Read(c);
		}

		public (ushort X, ushort Y) ReadAcc1(IController c)
			=> Port1.ReadAcc(c);

		public (ushort X, ushort Y) ReadAcc2(IController c)
			=> Port2.ReadAcc(c);

		public byte ReadSolar1(IController c)
		{
			return Port1.SolarSense(c);
		}

		public byte ReadSolar2(IController c)
		{
			return Port2.SolarSense(c);
		}

		public ControllerDefinition Definition { get; }

		public void SyncState(Serializer ser)
		{
			Port1.SyncState(ser);

			Port2.SyncState(ser);
		}

		private readonly IPort Port1;
		private readonly IPort Port2;
	}


	/// <summary>
	/// Represents a GBA add on
	/// </summary>
	public interface IPort
	{
		ushort Read(IController c);

		(ushort X, ushort Y) ReadAcc(IController c);

		byte SolarSense(IController c);

		ControllerDefinition Definition { get; }

		void SyncState(Serializer ser);

		int PortNum { get; }
	}

	[DisplayName("Gameboy Advance Controller")]
	public class StandardControls : IPort
	{
		public StandardControls(int portNum)
		{
			PortNum = portNum;
			Definition = new("Gameboy Advance Controller")
			{
				BoolButtons = BaseDefinition
				.Select(b => "P" + PortNum + " " + b)
				.ToList()
			};
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		public (ushort X, ushort Y) ReadAcc(IController c)
			=> (0, 0);

		public byte SolarSense(IController c)
			=> 0xFF;

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			//nothing
		}
	}

	[DisplayName("Gameboy Advance Controller + Tilt")]
	public class StandardTilt : IPort
	{
		public StandardTilt(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("Gameboy Advance Controller + Tilt")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			}.AddXYPair($"P{PortNum} Tilt {{0}}", AxisPairOrientation.RightAndUp, (-90).RangeTo(90), 0);
		}

		public int PortNum { get; }

		public float theta, phi, theta_prev, phi_prev, phi_prev_2;

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		public (ushort X, ushort Y) ReadAcc(IController c)
		{
			theta_prev = theta;
			phi_prev_2 = phi_prev;
			phi_prev = phi;

			theta = (float)(c.AxisValue(Definition.Axes[1]) * Math.PI / 180.0);
			phi = (float)(c.AxisValue(Definition.Axes[0]) * Math.PI / 180.0);

			// acc x is the result of rotating around body y AFTER rotating around body x
			// therefore this control scheme gives decreasing sensitivity in X as Y rotation increases
			var temp = (float)(Math.Cos(theta) * Math.Sin(phi));
			// additional acceleration components are dominated by axial components due to off axis rotation.
			// They vary widely based on physical hand movements, but this roughly matches what I observe in a real GBP
			var temp2 = (float)((phi - 2 * phi_prev + phi_prev_2) * 59.7275 * 59.7275 * 0.1);
			var accX = (ushort)(0x3A0 - Math.Floor(temp * 256) - temp2);

			// acc y is just the sine of the angle
			var temp3 = (float)Math.Sin(theta);
			// here we add in the acceleration generated by the point of rotation being far away from the accelerometer
			// this term dominates other facators due to the cartridge being far from the players hands in whatever system is being used.
			// It roughly matches what I observe in a real GBP
			var temp4 = (float)(Math.Pow((theta - theta_prev) * 59.7275, 2) * 0.15);
			var accY = (ushort)(0x3A0 - Math.Floor(temp3 * 256) + temp4);

			return (accX, accY);
		}

		public byte SolarSense(IController c)
			=> 0xFF;

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			// since we need rate of change of angle, need to savestate them
			ser.Sync(nameof(theta), ref theta);
			ser.Sync(nameof(phi), ref phi);
			ser.Sync(nameof(phi_prev), ref phi_prev);
		}
	}


	[DisplayName("Gameboy Advance Controller + Solar")]
	public class StandardSolar : IPort
	{
		public StandardSolar(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("Gameboy Advance Controller + Solar")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			}.AddAxis($"P{PortNum} Solar", (0x50).RangeTo(0xF0), 0xA0);
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		public (ushort X, ushort Y) ReadAcc(IController c)
			=> (0, 0);

		public byte SolarSense(IController c)
		{
			return (byte)(0x140 - (int)(c.AxisValue(Definition.Axes[0])));
		}

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			// nothing
		}
	}

	[DisplayName("Gameboy Advance Controller + Z Gyro")]
	public class StandardZGyro : IPort
	{
		public StandardZGyro(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("Gameboy Advance Controller + Z Gyro")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			}.AddAxis($"P{PortNum} Z Gyro", (-90).RangeTo(90), 0);
		}

		public int PortNum { get; }

		public float theta, theta_prev;

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		// repurposing from X/Y controls, X is Z axis, Y is just zero
		// seems to act like a rate gyro
		public (ushort X, ushort Y) ReadAcc(IController c)
		{
			theta_prev = theta;

			theta = c.AxisValue(Definition.Axes[0]);

			float d_theta = (float)((theta - theta_prev) * 59.7275);

			var accX = (ushort)(0x6C0 + Math.Floor(d_theta));

			return (accX, 0);
		}

		public byte SolarSense(IController c)
			=> 0xFF;

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			// since we need rate of change of angle, need to savestate them
			ser.Sync(nameof(theta), ref theta);
		}
	}
}
