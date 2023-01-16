# Kaytil
Z80 CP/M 2.2 emulator focused on running Kaypro II compatible games in a Linux terminal. (Now also available on a GR-SAKURA, Raspberry Pi Pico, DOS or Windows.)

Some features:
* Z80 emulation passes ZEXDOC tests.
* Full CP/M support with CBIOS adapted to emulator.
* Instruction trace and memory dumps for easier debugging.
* Uses IBM 3740 8-inch floppy disk images. (Use [cpmtools](http://www.moria.de/~michael/cpmtools/) to create images and copy files from the host system.)
* Converts ADM-3A escape codes to ANSI (vt100/xterm) escape codes.

Some supported games:
* Ladder (Yahoo Software, Kaypro II Version)
* Aliens (Yahoo Software, Kaypro II Version)
* CatChum (Yahoo Software, Kaypro II Version)
* Colossal Cave Adventure (Michael Goetz, Kaypro II Version)
* Sargon Chess (Hayden Books, CP/M Version)
* Deadline (Infocom, CP/M Version)
* Zork I (Infocom, CP/M Version)
* Zork II (Infocom, CP/M Version)
* Zork III (Infocom, CP/M Version)
* Nemesis (SuperSoft, CP/M Version)

## Linux Version
Building is just a matter of doing:
```
make
```
Disk images are specified on the command line, use the "-h" option for more information.

A useful trick is to disable Ctrl+C in sending a interrupt signal in your terminal:
```
stty intr ''
```
This makes the Ctrl+C pass into programs in the emulator, making it possible to for instance break in MBASIC or exiting certain other programs.

There is also an alternative version based on curses available by using a different Makefile:
```
make -f Makefile.curses
```
This is useful if running on a different kind of terminal that is not ANSI compatible.

## Gadget Renesas GR-SAKURA Version
Building this requires the RX GCC toolchain.
Then use the appropriate Makefile:
```
make -f Makefile.sakura
```
The resulting "kaytil.bin" file can be copied to the fake USB disk when the GR-SAKURA is in the bootloader mode.
Disk images must be named "A.IMG", "B.IMG", "C.IMG" and "D.IMG" and then placed on the root of a FAT16 formatted MicroSD card.
The console is available on the first UART, marked by pin 0 (RX) and pin 1 (TX) running at 115200 baud.

## Raspberry Pi Pico Version
Building this requires CMake, the ARM GCC toolchain and [Pico SDK](https://github.com/raspberrypi/pico-sdk).
In typical CMake fashion, create a build folder and call cmake pointing to the "pico/" subdirectory containing the CMakeLists.txt file:
```
mkdir build
cd build
PICO_SDK_PATH=/path/to/pico-sdk cmake /path/to/kaytil/pico/
make
```
The resulting "kaytil.elf" file can be flashed with SWD, or the "kaytil.uf2" file can be copied through USB in the BOOTSEL mode.
Disk images are part of the binary itself, so replace the "disk_a.img", "disk_b.img", "disk_c.img" or "disk_d.img" files in the "pico/" subdirectory BEFORE building!
The console is available on the "standard" UART at pin 1 and 2 running at 115200 baud.

## DOS (DJGPP) Version
Building this requires the [DJGPP toolchain](https://delorie.com/) and will only run on a fast 32-bit system in protected mode.

It has been tested with these packages:
* djdev205.zip
* gcc121b.zip
* bnu2351b.zip
* mak43br2.zip
* csdpmi7b.zip

Then use the appropriate Makefile:
```
make -f Makefile.dos
```
This version functions the same way as the Linux version with regards to command line arguments and disk images. Note that the CWSDPMI.EXE extender is also needed to run the program on systems without the DJGPP toolchain.

## Windows (MinGW) Version
Building this requires the [MinGW toolchain](https://www.mingw-w64.org/) and [PDCurses](https://pdcurses.org/).

Assuming that MinGW and PDCurses directories exist on the same directory level as the source directory, then from the source directory use these commands:
```
set PATH=..\mingw32\bin;%PATH%
set PDCURSES_SRCDIR=../PDCurses-3.9
mingw32-make.exe -f %PDCURSES_SRCDIR%/wincon/Makefile
mingw32-make.exe -f Makefile.mingw
```
This version also functions the same way as the Linux version with regards to command line arguments and disk images.

## Assembling CP/M 2.2 and CBIOS
In order to assemble the "cpm22.asm" and "cbios.asm" files you will need to use the real CP/M Macro Assembler.
The easiest way is to use the [YAZE emulator](https://www.mathematik.uni-ulm.de/users/ag/yaze-ag/) which already has this available.

Run the following commands in YAZE to read the files from the host system, assemble them, and then write them back:
```
r cbios.asm
r cpm22.asm
mac cbios.asm
mac cpm22.asm
w cbios.hex
w cpm22.hex
```

Converting from the Intel hex format to binary can be done with "srec_cat" from the [SRecord](http://srecord.sourceforge.net/) package as seen in the Makefile(s).

## Further Reading
Information on my blog:
* [Z80 CP/M 2.2 Emulator](https://kobolt.github.io/article-179.html)
* [CP/M on the GR-SAKURA](https://kobolt.github.io/article-184.html)
* [CP/M on the Raspberry Pi Pico](https://kobolt.github.io/article-196.html)
* [More CP/M Emulator Ports](https://kobolt.github.io/article-205.html)

YouTube video:
* [Kaypro II games](https://www.youtube.com/watch?v=uovRWOjvs98)

