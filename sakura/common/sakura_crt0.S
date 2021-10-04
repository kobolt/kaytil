/* From newlib-2.2.0, modified for GR-SAKURA */

/* Copyright (c) 2008, 2009, 2011 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met: 

     Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse 
     or promote products derived from this software without specific 
     prior written permission.

   This software is provided by the copyright holders and contributors
   "AS IS" and any express or implied warranties, including, but not
   limited to, the implied warranties of merchantability and fitness for
   a particular purpose are disclaimed.  In no event shall Red Hat
   incorporated be liable for any direct, indirect, incidental, special,
   exemplary, or consequential damages (including, but not limited to,
   procurement of substitute goods or services; loss of use, data, or
   profits; or business interruption) however caused and on any theory of
   liability, whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.  */


        /* NOTE: Put _start in own .init section and make sure this is at
           the 0xfff00000 address during linking! */
	.section .init,"ax"

	.global _start
_start:
.LFB2:
	mvtc	#0, psw
	/* Enable the DN bit - this should have been done for us by
           the CPU reset, but it is best to make sure for ourselves.  */	
	mvtc    #0x100, fpsw
	mov	#__stack, r0

        /* NOTE: Fixed vectors not used with GR-SAKURA default bootloader,
           and the "intb" register should contain the relocatable vectors. */
	mvtc	#_relocatable_vector_table, intb

	/* Copy the .data section from ROM into RAM.  */
	mov	#__datastart, r1
	mov	#__romdatastart, r2
	mov	#__romdatacopysize, r3
#ifdef __RX_ALLOW_STRING_INSNS__
	smovf
#else
	cmp	#0, r3
	beq	2f

1:	mov.b	[r2+], r5
	mov.b	r5, [r1+]
	sub	#1, r3
	bne	1b
2:	
#endif

	/* Initialise the contents of the .bss section.  */
	mov	#__bssstart, r1
	mov	#0, r2
	mov	#__bsssize, r3
	sstr.l

	/* Initialize data pointer register(s).
           Note: The registers used here should agree with the definition of
	   GP_BASE_REGNUM in gcc/config/rx/rx.h.
           Note: We are assuming that -mint-register has not been used when
	   compiling the application.  */
#ifdef _RX_PID
	/* Initialise the constant data pointer and small data pointers.  */
	mov	#__pid_base, r13
	mov	#__gp, r12
#else
	/* Initialise the small data area pointer.  */
	mov	#__gp, r13
#endif
	
	mov	#0, r1 /* argc */
	mov	#0, r2 /* argv */
	mov	#0, r3 /* envv */
	mov	#_main,r7
	jsr	r7

	mov	#_exit,r7
	jsr	r7

        .section .rodata
        .balign 4
        .global __pid_base
	.weak   __pid_base
__pid_base:   

        .section .sdata
        .balign 4
        .global __gp
	.weak   __gp
__gp:   

	.section .data
	.global ___dso_handle
	.weak   ___dso_handle
___dso_handle:
	.long	0

	.section ".rvectors","a"
	.macro	_iv	x
	.weak	\x
	.long	\x
	.endm
#define IV(x) _iv _##x##_handler
#define IVx() .long 0

	/* To use a vector, simply define a global function named foo_handler()
	   for any IV(foo) listed below (i.e. cmt0_cmi0_handler) */

	.global _relocatable_vector_table
_relocatable_vector_table:
	IV()                  /* 0x0000 - Reserved */
	IV()                  /* 0x0004 - Reserved */
	IV()                  /* 0x0008 - Reserved */
	IV()                  /* 0x000c - Reserved */
	IV()                  /* 0x0010 - Reserved */
	IV()                  /* 0x0014 - Reserved */
	IV()                  /* 0x0018 - Reserved */
	IV()                  /* 0x001c - Reserved */
	IV()                  /* 0x0020 - Reserved */
	IV()                  /* 0x0024 - Reserved */
	IV()                  /* 0x0028 - Reserved */
	IV()                  /* 0x002c - Reserved */
	IV()                  /* 0x0030 - Reserved */
	IV()                  /* 0x0034 - Reserved */
	IV()                  /* 0x0038 - Reserved */
	IV()                  /* 0x003c - Reserved */
	IV(bsc_buserr)        /* 0x0040 - BSC_BUSERR */
	IV()                  /* 0x0044 - Reserved */
	IV()                  /* 0x0048 - Reserved */
	IV()                  /* 0x004c - Reserved */
	IV()                  /* 0x0050 - Reserved */
	IV(fcu_fiferr)        /* 0x0054 - FCU_FCUERR */
	IV()                  /* 0x0058 - Reserved */
	IV(fcu_frdyi)         /* 0x005c - FCU_FRDYI */
	IV()                  /* 0x0060 - Reserved */
	IV()                  /* 0x0064 - Reserved */
	IV()                  /* 0x0068 - Reserved */
	IV(icu_swint)         /* 0x006c - ICU_SWINT */
	IV(cmt0_cmi0)         /* 0x0070 - CMT0_CMI0 */
	IV(cmt1_cmi1)         /* 0x0074 - CMT1_CMI1 */
	IV(cmt2_cmi2)         /* 0x0078 - CMT2_CMI2 */
	IV(cmt3_cmi3)         /* 0x007c - CMT3_CMI3 */
	IV(ether_eint)        /* 0x0080 - ETHER_EINT */
	IV(usb0_d0fifo0)      /* 0x0084 - USB0_D0FIFO0 */
	IV(usb0_d1fifo0)      /* 0x0088 - USB0_D1FIFO0 */
	IV(usb0_usbi0)        /* 0x008c - USB0_USBI0 */
	IV(usb1_d0fifo1)      /* 0x0090 - USB1_D0FIFO1 */
	IV(usb1_d1fifo1)      /* 0x0094 - USB1_D1FIFO1 */
	IV(usb1_usbi1)        /* 0x0098 - USB1_USBI1 */
	IV(rspi0_spri0)       /* 0x009c - RSPI0 SPRI0 */
	IV(rspi0_spti0)       /* 0x00a0 - RSPI0_SPTI0 */
	IV(rspi0_spii0)       /* 0x00a4 - RSPI0_SPII0 */
	IV(rspi1_spri1)       /* 0x00a8 - RSPI1_SPRI1 */
	IV(rspi1_spti1)       /* 0x00ac - RSPI1_SPTI1 */
	IV(rspi1_spii1)       /* 0x00b0 - RSPI1_SPII1 */
	IV(rspi2_spri2)       /* 0x00b4 - RSPI2_SPRI2 */
	IV(rspi2_spti2)       /* 0x00b8 - RSPI2_SPTI2 */
	IV(rspi2_spii2)       /* 0x00bc - RSPI2_SPII2 */
	IV(can0_rxf0)         /* 0x00c0 - CAN0_RXF0 */
	IV(can0_txf0)         /* 0x00c4 - CAN0_TXF0 */
	IV(can0_rxm0)         /* 0x00c8 - CAN0_RXM0 */
	IV(can0_txm0)         /* 0x00cc - CAN0_TXM0 */
	IV(can1_rxf1)         /* 0x00d0 - CAN1_RXF1 */
	IV(can1_txf1)         /* 0x00d4 - CAN1_TXF1 */
	IV(can1_rxm1)         /* 0x00d8 - CAN1_RXM1 */
	IV(can2_txm2)         /* 0x00dc - CAN1_TXM1 */
	IV(can2_rxf2)         /* 0x00e0 - CAN2_RXF2 */
	IV(can2_txf2)         /* 0x00e4 - CAN2_TXF2 */
	IV(can2_rxm2)         /* 0x00e8 - CAN2_RXM2 */
	IV(can2_txm2)         /* 0x00ec - CAN2_TXM2 */
	IV()                  /* 0x00f0 - Reserved */
	IV()                  /* 0x00f4 - Reserved */
	IV(rtc_cup)           /* 0x00f8 - RTC_CUP */
	IV()                  /* 0x00fc - Reserved */
	IV(icu_irq0)          /* 0x0100 - ICU IRQ0 */
	IV(icu_irq1)          /* 0x0104 - ICU IRQ1 */
	IV(icu_irq2)          /* 0x0108 - ICU IRQ2 */
	IV(icu_irq3)          /* 0x010c - ICU IRQ3 */
	IV(icu_irq4)          /* 0x0110 - ICU IRQ4 */
	IV(icu_irq5)          /* 0x0114 - ICU IRQ5 */
	IV(icu_irq6)          /* 0x0118 - ICU IRQ6 */
	IV(icu_irq7)          /* 0x011c - ICU IRQ7 */
	IV(icu_irq8)          /* 0x0120 - ICU IRQ8 */
	IV(icu_irq9)          /* 0x0124 - ICU IRQ9 */
	IV(icu_irq10)         /* 0x0128 - ICU IRQ10 */
	IV(icu_irq11)         /* 0x012c - ICU IRQ11 */
	IV(icu_irq12)         /* 0x0130 - ICU IRQ12 */
	IV(icu_irq13)         /* 0x0134 - ICU IRQ13 */
	IV(icu_irq14)         /* 0x0138 - ICU IRQ14 */
	IV(icu_irq15)         /* 0x013c - ICU IRQ15 */
	IV()                  /* 0x0140 - Reserved */
	IV()                  /* 0x0144 - Reserved */
	IV()                  /* 0x0148 - Reserved */
	IV()                  /* 0x014c - Reserved */
	IV()                  /* 0x0150 - Reserved */
	IV()                  /* 0x0154 - Reserved */
	IV()                  /* 0x0158 - Reserved */
	IV()                  /* 0x015c - Reserved */
	IV()                  /* 0x0160 - Reserved */
	IV()                  /* 0x0164 - Reserved */
	IV(usb_usbr0)         /* 0x0168 - USB USBR0 */
	IV(usb_usbr1)         /* 0x016c - USB USBR1 */
	IV(rtc_alm)           /* 0x0170 - RTC_ALM */
	IV(rtc_prd)           /* 0x0174 - RTC_PRD */
	IV()                  /* 0x0178 - Reserved */
	IV()                  /* 0x017c - Reserved */
	IV()                  /* 0x0180 - Reserved */
	IV()                  /* 0x0184 - Reserved */
	IV(ad_adi0)           /* 0x0188 - AD_ADI0 */
	IV()                  /* 0x018c - Reserved */
	IV()                  /* 0x0190 - Reserved */
	IV()                  /* 0x0194 - Reserved */
	IV(s12ad_s12adi0)     /* 0x0198 - S12AD_S12ADI0 */
	IV()                  /* 0x019c - Reserved */
	IV()                  /* 0x01a0 - Reserved */
	IV()                  /* 0x01a4 - Reserved */
	IV(icu_group0)        /* 0x01a8 - ICU GROUP0 */
	IV(icu_group1)        /* 0x01ac - ICU GROUP1 */
	IV(icu_group2)        /* 0x01b0 - ICU GROUP2 */
	IV(icu_group3)        /* 0x01b4 - ICU GROUP3 */
	IV(icu_group4)        /* 0x01b8 - ICU GROUP4 */
	IV(icu_group5)        /* 0x01bc - ICU GROUP5 */
	IV(icu_group6)        /* 0x01c0 - ICU GROUP6 */
	IV()                  /* 0x01c4 - Reserved */
	IV(icu_group12)       /* 0x01c8 - ICU GROUP12 */
	IV()                  /* 0x01cc - Reserved */
	IV()                  /* 0x01d0 - Reserved */
	IV()                  /* 0x01d4 - Reserved */
	IV()                  /* 0x01d8 - Reserved */
	IV()                  /* 0x01dc - Reserved */
	IV()                  /* 0x01e0 - Reserved */
	IV()                  /* 0x01e4 - Reserved */
	IV(sci2_scix0)        /* 0x01e8 - SCI2_SCIX0 */
	IV(sci2_scix1)        /* 0x01ec - SCI2_SCIX1 */
	IV(sci2_scix2)        /* 0x01f0 - SCI2_SCIX2 */
	IV(sci2_scix3)        /* 0x01f4 - SCI2_SCIX3 */
	IV(tpu0_tgi0a)        /* 0x01f8 - TPU0 TGI0A */
	IV(tpu0_tgi0b)        /* 0x01fc - TPU0 TGI0B */
	IV(tpu0_tgi0c)        /* 0x0200 - TPU0 TGI0C */
	IV(tpu0_tgi0d)        /* 0x0204 - TPU0 TGI0D */
	IV(tpu1_tgi1a)        /* 0x0208 - TPU1 TGI1A */
	IV(tpu1_tgi1b)        /* 0x020c - TPU1 TGI1B */
	IV(tpu2_tgi2a)        /* 0x0210 - TPU2 TGI2A */
	IV(tpu2_tgi2b)        /* 0x0214 - TPU2 TGI2B */
	IV(tpu3_tgi3a)        /* 0x0218 - TPU3 TGI3A */
	IV(tpu3_tgi3b)        /* 0x021c - TPU3 TGI3B */
	IV(tpu3_tgi3c)        /* 0x0220 - TPU3 TGI3C */
	IV(tpu3_tgi3d)        /* 0x0224 - TPU3 TGI3D */
	IV(tpu4_tgi4a)        /* 0x0228 - TPU4 TGI4A */
	IV(tpu4_tgi4b)        /* 0x022c - TPU4 TGI4B */
	IV(tpu5_tgi5a)        /* 0x0230 - TPU5 TGI5A */
	IV(tpu5_tgi5b)        /* 0x0234 - TPU5 TGI5B */
	IV(tpu6_tgi6a)        /* 0x0238 - TPU6 TGI6A */
	IV(tpu6_tgi6b)        /* 0x023c - TPU6 TGI6B */
	IV(tpu6_tgi6c)        /* 0x0240 - TPU6 TGI6C */
	IV(tpu6_tgi6d)        /* 0x0244 - TPU6 TGI6D */
	IV(mtu0_tgie0)        /* 0x0248 - MTU0 TGIE0 */
	IV(mtu0_tgif0)        /* 0x024c - MTU0 TGIF0 */
	IV(tpu7_tgi7a)        /* 0x0250 - TPU7 TGI7A */
	IV(tpu7_tgi7b)        /* 0x0254 - TPU7 TGI7B */
	IV(tpu8_tgi8a)        /* 0x0258 - TPU8 TGI8A */
	IV(tpu8_tgi8b)        /* 0x025c - TPU8 TGI8B */
	IV(tpu9_tgi9a)        /* 0x0260 - TPU9 TGI9A */
	IV(tpu9_tgi9b)        /* 0x0264 - TPU9 TGI9B */
	IV(tpu9_tgi9c)        /* 0x0268 - TPU9 TGI9C */
	IV(tpu9_tgi9d)        /* 0x026c - TPU9 TGI9D */
	IV(tpu10_tgi10a)      /* 0x0270 - TPU10 TGI10A */
	IV(tpu10_tgi10b)      /* 0x0274 - TPU10 TGI10B */
	IV(mtu4_tgic4)        /* 0x0278 - MTU4 TGIC4 */
	IV(mtu4_tgid4)        /* 0x027c - MTU4 TGID4 */
	IV(mtu4_tciv4)        /* 0x0280 - MTU4 TCIV4 */
	IV(mtu5_tgiu5)        /* 0x0284 - MTU5 TGIU5 */
	IV(mtu5_tgiv5)        /* 0x0288 - MTU5 TGIV5 */
	IV(mtu5_tgiw5)        /* 0x028c - MTU5 TGIW5 */
	IV(tpu11_tgi11a)      /* 0x0290 - TPU11 TGI11A */
	IV(tpu11_tgi11b)      /* 0x0294 - TPU11 TGI11B */
	IV(poe_oei1)          /* 0x0298 - POE OEI1 */
	IV(poe_oei2)          /* 0x029c - POE OEI2 */
	IV()                  /* 0x02a0 - Reserved */
	IV()                  /* 0x02a4 - Reserved */
	IV(tmr0_cmia0)        /* 0x02a8 - TMR0_CMIA0 */
	IV(tmr0_cmib0)        /* 0x02ac - TMR0_CMIB0 */
	IV(tmr0_ovi0)         /* 0x02b0 - TMR0_OVI0 */
	IV(tmr1_cmia1)        /* 0x02b4 - TMR1_CMIA1 */
	IV(tmr1_cmib1)        /* 0x02b8 - TMR1_CMIB1 */
	IV(tmr1_ovi1)         /* 0x02bc - TMR1_OVI1 */
	IV(tmr2_cmia2)        /* 0x02c0 - TMR2_CMIA2 */
	IV(tmr2_cmib2)        /* 0x02c4 - TMR2_CMIB2 */
	IV(tmr2_ovi2)         /* 0x02c8 - TMR2_OVI2 */
	IV(tmr3_cmia3)        /* 0x02cc - TMR3_CMIA3 */
	IV(tmr3_cmib3)        /* 0x02d0 - TMR3_CMIB3 */
	IV(tmr3_ovi3)         /* 0x02d4 - TMR3_OVI3 */
	IV(riic0_eei0)        /* 0x02d8 - RIIC0 EEI0 */
	IV(riic0_rxi0)        /* 0x02dc - RIIC0 RXI0 */
	IV(riic0_txi0)        /* 0x02e0 - RIIC0 TXI0 */
	IV(riic0_tei0)        /* 0x02e4 - RIIC0 TEI0 */
	IV(riic1_eei1)        /* 0x02e8 - RIIC1 EEI1 */
	IV(riic1_rxi1)        /* 0x02ec - RIIC1 RXI1 */
	IV(riic1_txi1)        /* 0x02f0 - RIIC1 TXI1 */
	IV(riic1_tei1)        /* 0x02f4 - RIIC1 TEI1 */
	IV(riic2_eei2)        /* 0x02f8 - RIIC2 EEI2 */
	IV(riic2_rxi2)        /* 0x02fc - RIIC2 RXI2 */
	IV(riic2_txi2)        /* 0x0300 - RIIC2 TXI2 */
	IV(riic2_tei2)        /* 0x0304 - RIIC2 TEI2 */
	IV(riic3_eei3)        /* 0x0308 - RIIC3 EEI3 */
	IV(riic3_rxi3)        /* 0x030c - RIIC3 RXI3 */
	IV(riic3_txi3)        /* 0x0310 - RIIC3 TXI3 */
	IV(riic3_tei3)        /* 0x0314 - RIIC3 TEI3 */
	IV(dmac_dmac0i)       /* 0x0318 - DMAC DMAC0I */
	IV(dmac_dmac1i)       /* 0x031c - DMAC DMAC1I */
	IV(dmac_dmac2i)       /* 0x0320 - DMAC DMAC2I */
	IV(dmac_dmac3i)       /* 0x0324 - DMAC DMAC3I */
	IV(exdmac_exdmac0i)   /* 0x0328 - EXDMAC EXDMAC0I */
	IV(exdmac_exdmac1i)   /* 0x032c - EXDMAC EXDMAC1I */
	IV()                  /* 0x0330 - Reserved */
	IV()                  /* 0x0334 - Reserved */
	IV()                  /* 0x0338 - Reserved */
	IV()                  /* 0x033c - Reserved */
	IV()                  /* 0x0340 - Reserved */
	IV()                  /* 0x0344 - Reserved */
	IV()                  /* 0x0348 - Reserved */
	IV()                  /* 0x034c - Reserved */
	IV()                  /* 0x0350 - Reserved */
	IV()                  /* 0x0354 - Reserved */
	IV(sci0_rxi0)         /* 0x0358 - SCI0_RXI0 */
	IV(sci0_txi0)         /* 0x035c - SCI0_TXI0 */
	IV(sci0_tei0)         /* 0x0360 - SCI0_TEI0 */
	IV(sci1_rxi1)         /* 0x0364 - SCI1_RXI1 */
	IV(sci1_txi1)         /* 0x0368 - SCI1_TXI1 */
	IV(sci1_tei1)         /* 0x036c - SCI1_TEI1 */
	IV(sci2_rxi2)         /* 0x0370 - SCI2_RXI2 */
	IV(sci2_txi2)         /* 0x0374 - SCI2_TXI2 */
	IV(sci2_tei2)         /* 0x0378 - SCI2_TEI2 */
	IV(sci3_rxi3)         /* 0x037c - SCI3_RXI3 */
	IV(sci3_txi3)         /* 0x0380 - SCI3_TXI3 */
	IV(sci3_tei3)         /* 0x0384 - SCI3_TEI3 */
	IV(sci4_rxi4)         /* 0x0388 - SCI4_RXI4 */
	IV(sci4_txi4)         /* 0x038c - SCI4_TXI4 */
	IV(sci4_tei4)         /* 0x0390 - SCI4_TEI4 */
	IV(sci5_rxi5)         /* 0x0394 - SCI5_RXI5 */
	IV(sci5_txi5)         /* 0x0398 - SCI5_TXI5 */
	IV(sci5_tei5)         /* 0x039c - SCI5_TEI5 */
	IV(sci6_rxi6)         /* 0x03a0 - SCI6_RXI6 */
	IV(sci6_txi6)         /* 0x03a4 - SCI6_TXI6 */
	IV(sci6_tei6)         /* 0x03a8 - SCI6_TEI6 */
	IV(sci7_rxi7)         /* 0x03ac - SCI7_RXI7 */
	IV(sci7_txi7)         /* 0x03b0 - SCI7_TXI7 */
	IV(sci7_tei7)         /* 0x03b4 - SCI7_TEI7 */
	IV(sci8_rxi8)         /* 0x03b8 - SCI8_RXI8 */
	IV(sci8_txi8)         /* 0x03bc - SCI8_TXI8 */
	IV(sci8_tei8)         /* 0x03c0 - SCI8_TEI8 */
	IV(sci9_rxi9)         /* 0x03c4 - SCI9_RXI9 */
	IV(sci9_txi9)         /* 0x03c8 - SCI9_TXI9 */
	IV(sci9_tei9)         /* 0x03cc - SCI9_TEI9 */
	IV(sci10_rxi10)       /* 0x03d0 - SCI10_RXI10 */
	IV(sci10_txi10)       /* 0x03d4 - SCI10_TXI10 */
	IV(sci10_tei10)       /* 0x03d8 - SCI10_TEI10 */
	IV(sci11_rxi11)       /* 0x03dc - SCI11_RXI11 */
	IV(sci11_txi11)       /* 0x03e0 - SCI11_TXI11 */
	IV(sci11_tei11)       /* 0x03e4 - SCI11_TEI11 */
	IV(sci12_rxi12)       /* 0x03e8 - SCI12_RXI12 */
	IV(sci12_txi12)       /* 0x03ec - SCI12_TXI12 */
	IV(sci12_tei12)       /* 0x03f0 - SCI12_TEI12 */
	IV(ieb_iebint)        /* 0x03f4 - IEB IEBINT */
	IV()                  /* 0x03f8 - Reserved */
	IV()                  /* 0x03fc - Reserved */

