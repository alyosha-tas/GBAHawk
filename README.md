A fork of the BizHawk emulator focusing on console verification of Gameboy Advance TASes and hardware research.



Also has (experimental) support for NES. NOTE: Only NTSC NES supported. Only NES 2.0 headered ROMs are supported.



To link GB/C/A games, you must use the multi-disc bundler. This tool creates an .xml file that refertences both ROM locations.

Run GBAHawk.exe and navigate to 'Tools' -> 'Multi-Disk Bundler'. The 'Name' field is the name you want the resulting .xml file to be. Then add each ROM location in the fields below.

Hitting 'Save' will create the .xml file. Hitting 'Save and Run' will additionally launch the linked ROMs.



To perform single pack linking on the GBA, the second ROM should be the included file 'Multi-boot.gba' (which is just a binary file containing the ascii string "This isn't a ROM")



