##Overview

GBAHawk is a fork of the BizHawk emulator focusing on console verification of TASes and hardware research for early nintendo consoles.

Where applicable, only the North American version of the console is supported. Any revision dependent behavior is tested on and matches my personal console.

Currently supports GB/C/A and NES consoles with mature console verificaiton pipelines. Note that NES is still listed as experimental due to limited mapper support and ongoing developement.

A SNES core is currently in developement.

##Linking

To link GB/C/A games, you must use the multi-disc bundler. This tool creates an .xml file that refertences both ROM locations.

Run GBAHawk.exe and navigate to 'Tools' -> 'Multi-Disk Bundler'. The 'Name' field is the name you want the resulting .xml file to be. Then add each ROM location in the fields below.

Hitting 'Save' will create the .xml file. Hitting 'Save and Run' will additionally launch the linked ROMs.

To perform single pack linking on the GBA, the second ROM should be the included file 'Multi-boot.gba' (which is just a binary file containing the ascii string "This isn't a ROM")

##Project Status

Currently the primary focus is on SNES developement and finalizing the NES core.