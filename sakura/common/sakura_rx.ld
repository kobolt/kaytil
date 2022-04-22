/* From newlib-2.2.0, modified for GR-SAKURA */

/*

Copyright (c) 2005,2008,2009 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

/* Default linker script, for normal executables */
OUTPUT_ARCH(rx)
ENTRY(_start)

/* This memory layout corresponds to GR-SAKURA II (R5F563NYDDFP). */
MEMORY {
	RAM (w)   : ORIGIN = 0x00000000, LENGTH = 0x00040000 /* 256k */
	STACK (w) : ORIGIN = 0x00040000, LENGTH = 16 /* top of RAM */
	ROM (w)   : ORIGIN = 0xfff00000, LENGTH = 0x00100000 /* 1Mb */
}

SECTIONS
{
  /* NOTE: Make sure .init section is first, with the _start symbol */
  .init 0xfff00000 : AT(0xfff00000) {
    PROVIDE (_start = .);
   *(.init)
  } > ROM

  .text : {
    *(.text P .stub .text.* .gnu.linkonce.t.*)
    KEEP (*(.text.*personality*))
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.interp .hash .dynsym .dynstr .gnu.version*)
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
  } > ROM

  .rodata : {
    . = ALIGN(4);
    *(.plt)
    *(.rodata C C_2 C_1 W W_2 W_1 .rodata.* .gnu.linkonce.r.*)
    *(.rodata1)
    *(.eh_frame_hdr)
    KEEP (*(.eh_frame))
    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)
    . = ALIGN(4);
    PROVIDE(__romdatastart = .);
  } > ROM

  .data : {
    . = ALIGN(4);
    PROVIDE (__datastart = .); /* IF_ROROM */
    LONG(0); /* Sentinel.  */

    /* gcc uses crtbegin.o to find the start of the constructors, so
       we make sure it is first.  Because this is a wildcard, it
       doesn't matter if the user does not actually link against
       crtbegin.o; the linker won't look for a file to match a
       wildcard.  The wildcard also means that it doesn't matter which
       directory crtbegin.o is in.  */
    KEEP (*crtbegin*.o(.ctors))

    /* We don't want to include the .ctor section from from the
       crtend.o file until after the sorted ctors.  The .ctor section
       from the crtend file contains the end of ctors marker and it
       must be last */
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))

    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))

    KEEP (*(.jcr))
    *(.data.rel.ro.local) *(.data.rel.ro*)
    *(.dynamic)

    *(.data D .data.* .gnu.linkonce.d.*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    SORT(CONSTRUCTORS)
    *(.data1)
    *(.got.plt) *(.got)

    /* We want the small data sections together, so single-instruction offsets
       can access them all, and initialized data all before uninitialized, so
       we can shorten the on-disk segment size.  */
    . = ALIGN(4);
    *(.sdata .sdata.* .gnu.linkonce.s.* D_2 D_1)

    . = ALIGN(4);
    _edata = .;
    PROVIDE (edata = .);
    PROVIDE (__dataend = .);
  } > RAM AT> ROM

  /* Note that __romdatacopysize may be ZERO for the simulator, which
     knows how to intialize RAM directly.  It should ONLY be used for
     copying data from ROM to RAM; if you need to know the size of the
     data section, subtract the end symbol from the start symbol.  */
  /* Note that crt0 assumes this is a multiple of four; all the
     start/stop symbols are also assumed long-aligned.  */
  PROVIDE (__romdatacopysize = SIZEOF(.data));

  .bss : {
    . = ALIGN(4);
    PROVIDE (__bssstart = .);
    *(.dynbss)
    *(.sbss .sbss.*)
    *(.bss B B_2 B_1 .bss.* .gnu.linkonce.b.*)
    . = ALIGN(4);
    *(COMMON)
    . = ALIGN(4);
    PROVIDE (__bssend = .);
    _end = .;
    PROVIDE (end = .);
  } > RAM
  PROVIDE (__bsssize = SIZEOF(.bss) / 4);

  .stack (ORIGIN (STACK)) :
  {
    PROVIDE (__stack = .);
    *(.stack)
  }

  /* NOTE: Fixed vectors not used with GR-SAKURA default bootloader.

  PROVIDE (__rx_priviledged_exception_handler = 0x00000000);
  PROVIDE (__rx_access_exception_handler = 0x00000000);
  PROVIDE (__rx_undefined_exception_handler = 0x00000000);
  PROVIDE (__rx_floating_exception_handler = 0x00000000);
  PROVIDE (__rx_nonmaskable_exception_handler = 0x00000000);

  .vectors (0xFFFFFFD0) :
  {
    PROVIDE (__vectors = .);
    LONG (__rx_priviledged_exception_handler);
    LONG (__rx_access_exception_handler);
    LONG (0);
    LONG (__rx_undefined_exception_handler);
    LONG (0);
    LONG (__rx_floating_exception_handler);
    LONG (0);
    LONG (0);
    LONG (0);
    LONG (0);
    LONG (__rx_nonmaskable_exception_handler);
    LONG (_start);
  }
  */

  .rvectors :
  {
    _rvectors_start = .;
    *(.rvectors)
    _rvectors_end = .;
  } > ROM
}

