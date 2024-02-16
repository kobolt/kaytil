#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "z80.h"
#include "mem.h"
#include "io.h"
#include "panic.h"

/* Helper macros to access named unions and structs. */
#define a(x)    (x)->u_af.s_af.a
#define af(x)   (x)->u_af.af
#define af_(x)  (x)->u_af_.af_
#define b(x)    (x)->u_bc.s_bc.b
#define bc(x)   (x)->u_bc.bc
#define bc_(x)  (x)->u_bc_.bc_
#define c(x)    (x)->u_bc.s_bc.c
#define d(x)    (x)->u_de.s_de.d
#define de(x)   (x)->u_de.de
#define de_(x)  (x)->u_de_.de_
#define e(x)    (x)->u_de.s_de.e
#define f(x)    (x)->u_af.s_af.u_f.f
#define flag(x) (x)->u_af.s_af.u_f.flag
#define h(x)    (x)->u_hl.s_hl.h
#define hl(x)   (x)->u_hl.hl
#define hl_(x)  (x)->u_hl_.hl_
#define ix(x)   (x)->u_ix.ix
#define ixh(x)  (x)->u_ix.s_ix.ixh
#define ixl(x)  (x)->u_ix.s_ix.ixl
#define iy(x)   (x)->u_iy.iy
#define iyh(x)  (x)->u_iy.s_iy.iyh
#define iyl(x)  (x)->u_iy.s_iy.iyl
#define l(x)    (x)->u_hl.s_hl.l



typedef enum {
  /* 8-Bit registers */
  DT_R_B,
  DT_R_C,
  DT_R_D,
  DT_R_E,
  DT_R_H,
  DT_R_L,
  DT_R_HLI,
  DT_R_A,
  DT_R_IXH,
  DT_R_IXL,
  DT_R_IXI,
  DT_R_IYH,
  DT_R_IYL,
  DT_R_IYI,

  /* Register pairs */
  DT_RP_BC,
  DT_RP_DE,
  DT_RP_HL,
  DT_RP_SP,
  DT_RP_AF,
  DT_RP_IX,
  DT_RP_IY,

  /* Conditions */
  DT_CC_NZ,
  DT_CC_Z,
  DT_CC_NC,
  DT_CC_C,
  DT_CC_PO,
  DT_CC_PE,
  DT_CC_P,
  DT_CC_M,

  /* Arithmetic/Logic operations */
  DT_ALU_ADD,
  DT_ALU_ADC,
  DT_ALU_SUB,
  DT_ALU_SBC,
  DT_ALU_AND,
  DT_ALU_XOR,
  DT_ALU_OR,
  DT_ALU_CP,

  /* Rotation/Shift operations */
  DT_ROT_RLC,
  DT_ROT_RRC,
  DT_ROT_RL,
  DT_ROT_RR,
  DT_ROT_SLA,
  DT_ROT_SRA,
  DT_ROT_SLL,
  DT_ROT_SRL,

  /* Interrupt modes */
  DT_IM_0,
  DT_IM_01,
  DT_IM_1,
  DT_IM_2,

  /* Block instructions */
  DT_BLI_LDI,
  DT_BLI_LDD,
  DT_BLI_LDIR,
  DT_BLI_LDDR,
  DT_BLI_CPI,
  DT_BLI_CPD,
  DT_BLI_CPIR,
  DT_BLI_CPDR,
  DT_BLI_INI,
  DT_BLI_IND,
  DT_BLI_INIR,
  DT_BLI_INDR,
  DT_BLI_OUTI,
  DT_BLI_OUTD,
  DT_BLI_OTIR,
  DT_BLI_OTDR,
} dt_t;



static dt_t dt_r[8] =
  {DT_R_B, DT_R_C, DT_R_D, DT_R_E, DT_R_H, DT_R_L, DT_R_HLI, DT_R_A};

static dt_t dt_rix[8] =
  {DT_R_B, DT_R_C, DT_R_D, DT_R_E, DT_R_IXH, DT_R_IXL, DT_R_IXI, DT_R_A};

static dt_t dt_riy[8] =
  {DT_R_B, DT_R_C, DT_R_D, DT_R_E, DT_R_IYH, DT_R_IYL, DT_R_IYI, DT_R_A};

static dt_t dt_rp[4] =
  {DT_RP_BC, DT_RP_DE, DT_RP_HL, DT_RP_SP};

static dt_t dt_rpaf[4] =
  {DT_RP_BC, DT_RP_DE, DT_RP_HL, DT_RP_AF};

static dt_t dt_rpix[4] =
  {DT_RP_BC, DT_RP_DE, DT_RP_IX, DT_RP_SP};

static dt_t dt_rpiy[4] =
  {DT_RP_BC, DT_RP_DE, DT_RP_IY, DT_RP_SP};

static dt_t dt_cc[8] =
  {DT_CC_NZ, DT_CC_Z, DT_CC_NC, DT_CC_C, DT_CC_PO, DT_CC_PE, DT_CC_P, DT_CC_M};

static dt_t dt_alu[8] = 
  {DT_ALU_ADD, DT_ALU_ADC, DT_ALU_SUB, DT_ALU_SBC,
   DT_ALU_AND, DT_ALU_XOR, DT_ALU_OR,  DT_ALU_CP};

static dt_t dt_rot[8] =
  {DT_ROT_RLC, DT_ROT_RRC, DT_ROT_RL,  DT_ROT_RR,
   DT_ROT_SLA, DT_ROT_SRA, DT_ROT_SLL, DT_ROT_SRL};

static dt_t dt_im[8] =
  {DT_IM_0, DT_IM_01, DT_IM_1, DT_IM_2, DT_IM_0, DT_IM_01, DT_IM_1, DT_IM_2};

static dt_t dt_bli[4][4] =
  {{DT_BLI_LDI,  DT_BLI_CPI,  DT_BLI_INI,  DT_BLI_OUTI},
   {DT_BLI_LDD,  DT_BLI_CPD,  DT_BLI_IND,  DT_BLI_OUTD},
   {DT_BLI_LDIR, DT_BLI_CPIR, DT_BLI_INIR, DT_BLI_OTIR},
   {DT_BLI_LDDR, DT_BLI_CPDR, DT_BLI_INDR, DT_BLI_OTDR}};



#ifdef DISABLE_Z80_TRACE
#define z80_trace(...)
#else

#define Z80_TRACE_BUFFER_SIZE 20
#define Z80_TRACE_BUFFER_ENTRY 80

static char z80_trace_buffer[Z80_TRACE_BUFFER_SIZE][Z80_TRACE_BUFFER_ENTRY];
static int z80_trace_buffer_index = 0;

static char *dt_text(dt_t sym)
{
  switch (sym) {
  case DT_R_B:      return "B";
  case DT_R_C:      return "C";
  case DT_R_D:      return "D";
  case DT_R_E:      return "E";
  case DT_R_H:      return "H";
  case DT_R_L:      return "L";
  case DT_R_HLI:    return "(HL)";
  case DT_R_A:      return "A";
  case DT_R_IXH:    return "IXH";
  case DT_R_IXL:    return "IXL";
  case DT_R_IXI:    return "(IX)";
  case DT_R_IYH:    return "IYH";
  case DT_R_IYL:    return "IYL";
  case DT_R_IYI:    return "(IY)";
  case DT_RP_BC:    return "BC";
  case DT_RP_DE:    return "DE";
  case DT_RP_HL:    return "HL";
  case DT_RP_SP:    return "SP";
  case DT_RP_AF:    return "AF";
  case DT_RP_IX:    return "IX";
  case DT_RP_IY:    return "IY";
  case DT_CC_NZ:    return "NZ";
  case DT_CC_Z:     return "Z";
  case DT_CC_NC:    return "NC";
  case DT_CC_C:     return "C";
  case DT_CC_PO:    return "PO";
  case DT_CC_PE:    return "PE";
  case DT_CC_P:     return "P";
  case DT_CC_M:     return "M";
  case DT_ALU_ADD:  return "ADD A,";
  case DT_ALU_ADC:  return "ADC A,";
  case DT_ALU_SUB:  return "SUB A,";
  case DT_ALU_SBC:  return "SBC A,";
  case DT_ALU_AND:  return "AND A,";
  case DT_ALU_XOR:  return "XOR A,";
  case DT_ALU_OR:   return "OR A,";
  case DT_ALU_CP:   return "CP A,";
  case DT_ROT_RLC:  return "RLC";
  case DT_ROT_RRC:  return "RRC";
  case DT_ROT_RL:   return "RL";
  case DT_ROT_RR:   return "RR";
  case DT_ROT_SLA:  return "SLA";
  case DT_ROT_SRA:  return "SRA";
  case DT_ROT_SLL:  return "SLL";
  case DT_ROT_SRL:  return "SRL";
  case DT_IM_0:     return "0";
  case DT_IM_01:    return "0/1";
  case DT_IM_1:     return "1";
  case DT_IM_2:     return "2";
  case DT_BLI_LDI:  return "LDI";
  case DT_BLI_LDD:  return "LDD";
  case DT_BLI_LDIR: return "LDIR";
  case DT_BLI_LDDR: return "LDDR";
  case DT_BLI_CPI:  return "CPI";
  case DT_BLI_CPD:  return "CPD";
  case DT_BLI_CPIR: return "CPIR";
  case DT_BLI_CPDR: return "CPDR";
  case DT_BLI_INI:  return "INI";
  case DT_BLI_IND:  return "IND";
  case DT_BLI_INIR: return "INIR";
  case DT_BLI_INDR: return "INDR";
  case DT_BLI_OUTI: return "OUTI";
  case DT_BLI_OUTD: return "OUTD";
  case DT_BLI_OTIR: return "OTIR";
  case DT_BLI_OTDR: return "OTDR";
  default:
    return "?";
  }
}



static void z80_trace(z80_t *z80, mem_t *mem, int no, const char *format, ...)
{
  va_list args;
  char buffer[Z80_TRACE_BUFFER_ENTRY + 2];
  int n = 0;

  n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n,
    "%04x   ", z80->pc);

  switch (no) {
  case 1:
    n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n,
      "%02x            ",
      mem_read(mem, z80->pc));
    break;
  case 2:
    n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n,
      "%02x %02x         ",
      mem_read(mem, z80->pc), mem_read(mem, z80->pc + 1));
    break;
  case 3:
    n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n,
      "%02x %02x %02x      ",
      mem_read(mem, z80->pc), mem_read(mem, z80->pc + 1),
      mem_read(mem, z80->pc + 2));
    break;
  case 4:
  default:
    n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n,
      "%02x %02x %02x %02x   ",
      mem_read(mem, z80->pc), mem_read(mem, z80->pc + 1),
      mem_read(mem, z80->pc + 2), mem_read(mem, z80->pc + 3));
    break;
  }

  va_start(args, format);
  n += vsnprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n, format, args);
  va_end(args);

  while (36 - n > 0) {
    buffer[n] = ' ';
    n++;
    if (n >= Z80_TRACE_BUFFER_ENTRY) {
      break;
    }
  }

  n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n,
    "%02x %04x %04x %04x %04x %04x %04x ",
    a(z80), bc(z80), de(z80), hl(z80), z80->sp, ix(z80), iy(z80));
    
  n += snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n, "%c%c-%c-%c%c%c",
    flag(z80).s  ? 'S' : '.',
    flag(z80).z  ? 'Z' : '.',
    flag(z80).h  ? 'H' : '.',
    flag(z80).pv ? 'P' : '.',
    flag(z80).n  ? 'N' : '.',
    flag(z80).c  ? 'C' : '.');

  snprintf(&buffer[n], Z80_TRACE_BUFFER_ENTRY - n, "\n");

  strncpy(z80_trace_buffer[z80_trace_buffer_index],
    buffer, Z80_TRACE_BUFFER_ENTRY);
  z80_trace_buffer_index++;
  if (z80_trace_buffer_index >= Z80_TRACE_BUFFER_SIZE) {
    z80_trace_buffer_index = 0;
  }
}



void z80_trace_init(void)
{
  for (int i = 0; i < Z80_TRACE_BUFFER_SIZE; i++) {
    z80_trace_buffer[i][0] = '\0';
  }
  z80_trace_buffer_index = 0;
}



void z80_trace_dump(FILE *fh)
{
  fprintf(stderr, "PC:    Code:         Mnemonics:     "
                  "A: BC:  DE:  HL:  SP:  IX:  IY:  Flags:\n");
  for (int i = z80_trace_buffer_index; i < Z80_TRACE_BUFFER_SIZE; i++) {
    if (z80_trace_buffer[i][0] != '\0') {
      fprintf(fh, z80_trace_buffer[i]);
    }
  }
  for (int i = 0; i < z80_trace_buffer_index; i++) {
    if (z80_trace_buffer[i][0] != '\0') {
      fprintf(fh, z80_trace_buffer[i]);
    }
  }
}



void z80_dump(FILE *fh, z80_t *z80, mem_t *mem)
{
  fprintf(fh,
    "%04x   %02x %02x %02x %02x   -              "
    "%02x %04x %04x %04x %04x %04x %04x ",
    z80->pc, mem_read(mem, z80->pc),     mem_read(mem, z80->pc + 1),
             mem_read(mem, z80->pc + 2), mem_read(mem, z80->pc + 3),
    a(z80), bc(z80), de(z80), hl(z80), z80->sp, ix(z80), iy(z80));

  fprintf(fh, "%c%c-%c-%c%c%c",
    flag(z80).s  ? 'S' : '.',
    flag(z80).z  ? 'Z' : '.',
    flag(z80).h  ? 'H' : '.',
    flag(z80).pv ? 'P' : '.',
    flag(z80).n  ? 'N' : '.',
    flag(z80).c  ? 'C' : '.');

  fprintf(fh, "\n");
}
#endif /* DISABLE_Z80_TRACE */



static uint8_t reset_bit(uint8_t value, int bit_no)
{
  return value & ~(1 << bit_no);
}



static uint8_t set_bit(uint8_t value, int bit_no)
{
  return value | (1 << bit_no);
}



static bool parity_even(uint8_t value)
{
  value ^= value >> 4;
  value ^= value >> 2;
  value ^= value >> 1;
  return (~value) & 1;
}



static void z80_add(z80_t *z80, uint8_t value)
{
  uint16_t result = a(z80) + value;
  flag(z80).h = ((a(z80) & 0xF) + (value & 0xF)) & 0x10 ? 1 : 0;
  flag(z80).pv = ((a(z80) & 0x80) == (value  & 0x80)) &&
                 ((value  & 0x80) != (result & 0x80)) ? 1 : 0;
  flag(z80).n = 0;
  flag(z80).c = (result & 0x100) ? 1 : 0;
  a(z80) = result;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
}



static void z80_adc(z80_t *z80, uint8_t value)
{
  uint16_t result = a(z80) + value + flag(z80).c;
  flag(z80).h = (((a(z80) & 0xF) + (value & 0xF)) + flag(z80).c) & 0x10 ? 1 : 0;
  flag(z80).pv = ((a(z80) & 0x80) == (value  & 0x80)) &&
                 ((value  & 0x80) != (result & 0x80)) ? 1 : 0;
  flag(z80).n = 0;
  flag(z80).c = (result & 0x100) ? 1 : 0;
  a(z80) = result;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
}



static void z80_sub(z80_t *z80, uint8_t value)
{
  uint16_t result = a(z80) - value;
  flag(z80).h = ((a(z80) & 0xF) - (value & 0xF)) & 0x10 ? 1 : 0;
  flag(z80).pv = ((a(z80) & 0x80) != (value  & 0x80)) &&
                 ((value  & 0x80) == (result & 0x80)) ? 1 : 0;
  flag(z80).n = 1;
  flag(z80).c = (result & 0x100) ? 1 : 0;
  a(z80) = result;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
}



static void z80_sbc(z80_t *z80, uint8_t value)
{
  uint16_t result = a(z80) - value - flag(z80).c;
  flag(z80).h = (((a(z80) & 0xF) - (value & 0xF)) - flag(z80).c) & 0x10 ? 1 : 0;
  flag(z80).pv = ((a(z80) & 0x80) != (value  & 0x80)) &&
                 ((value  & 0x80) == (result & 0x80)) ? 1 : 0;
  flag(z80).n = 1;
  flag(z80).c = (result & 0x100) ? 1 : 0;
  a(z80) = result;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
}



static void z80_and(z80_t *z80, uint8_t value)
{
  a(z80) &= value;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
  flag(z80).h = 1;
  flag(z80).pv = parity_even(a(z80));
  flag(z80).n = 0;
  flag(z80).c = 0;
}



static void z80_or(z80_t *z80, uint8_t value)
{
  a(z80) |= value;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(a(z80));
  flag(z80).n = 0;
  flag(z80).c = 0;
}



static void z80_xor(z80_t *z80, uint8_t value)
{
  a(z80) ^= value;
  flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
  flag(z80).z = (a(z80) == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(a(z80));
  flag(z80).n = 0;
  flag(z80).c = 0;
}



static void z80_compare(z80_t *z80, uint8_t value)
{
  uint16_t result = a(z80) - value;
  flag(z80).h = ((a(z80) & 0xF) - (value & 0xF)) & 0x10 ? 1 : 0;
  flag(z80).pv = ((a(z80) & 0x80) != (value  & 0x80)) &&
                 ((value  & 0x80) == (result & 0x80)) ? 1 : 0;
  flag(z80).n = 1;
  flag(z80).c = (result & 0x100) ? 1 : 0;
  flag(z80).s = (result & 0x80) ? 1 : 0;
  flag(z80).z = (result == 0) ? 1 : 0;
}



static void z80_add_16(z80_t *z80, uint16_t *reg, uint16_t value)
{
  uint32_t result = *reg + value;
  flag(z80).h = ((*reg & 0xFFF) + (value & 0xFFF)) & 0x1000 ? 1 : 0;
  flag(z80).n = 0;
  flag(z80).c = (result & 0x10000) ? 1 : 0;
  *reg = result;
}



static void z80_adc_16(z80_t *z80, uint16_t *reg, uint16_t value)
{
  uint32_t result = *reg + value + flag(z80).c;
  flag(z80).h = ((*reg & 0xFFF) + (value & 0xFFF)) & 0x1000 ? 1 : 0;
  flag(z80).pv = ((*reg  & 0x8000) == (value  & 0x8000)) &&
                 ((value & 0x8000) != (result & 0x8000)) ? 1 : 0;
  flag(z80).n = 0;
  flag(z80).c = (result & 0x10000) ? 1 : 0;
  *reg = result;
  flag(z80).s = (*reg & 0x8000) ? 1 : 0;
  flag(z80).z = (*reg == 0) ? 1 : 0;
}



static void z80_sbc_16(z80_t *z80, uint16_t *reg, uint16_t value)
{
  uint32_t result = *reg - value - flag(z80).c;
  flag(z80).h = ((*reg & 0xFFF) - (value & 0xFFF)) & 0x1000 ? 1 : 0;
  flag(z80).pv = ((*reg  & 0x8000) != (value  & 0x8000)) &&
                 ((value & 0x8000) == (result & 0x8000)) ? 1 : 0;
  flag(z80).n = 1;
  flag(z80).c = (result & 0x10000) ? 1 : 0;
  *reg = result;
  flag(z80).s = (*reg & 0x8000) ? 1 : 0;
  flag(z80).z = (*reg == 0) ? 1 : 0;
}



static void z80_bit(z80_t *z80, uint8_t value, int bit_no)
{
  flag(z80).z = !((value >> bit_no) & 0x1) ? 1 : 0;
  flag(z80).h = 1;
  flag(z80).n = 0;
}



static void z80_cpd(z80_t *z80, mem_t *mem)
{
  bool old_c = flag(z80).c;
  z80_compare(z80, mem_read(mem, hl(z80)));
  hl(z80)--;
  bc(z80)--;
  flag(z80).pv = (bc(z80) != 0) ? 1 : 0;
  flag(z80).c = old_c;
}



static void z80_cpi(z80_t *z80, mem_t *mem)
{
  bool old_c = flag(z80).c;
  z80_compare(z80, mem_read(mem, hl(z80)));
  hl(z80)++;
  bc(z80)--;
  flag(z80).pv = (bc(z80) != 0) ? 1 : 0;
  flag(z80).c = old_c;
}



static void z80_ldd(z80_t *z80, mem_t *mem)
{
  mem_write(mem, de(z80), mem_read(mem, hl(z80)));
  de(z80)--;
  hl(z80)--;
  bc(z80)--;
  flag(z80).h = 0;
  flag(z80).pv = (bc(z80) != 0) ? 1 : 0;
  flag(z80).n = 0;
}



static void z80_ldi(z80_t *z80, mem_t *mem)
{
  mem_write(mem, de(z80), mem_read(mem, hl(z80)));
  de(z80)++;
  hl(z80)++;
  bc(z80)--;
  flag(z80).h = 0;
  flag(z80).pv = (bc(z80) != 0) ? 1 : 0;
  flag(z80).n = 0;
}



static uint8_t z80_inc(z80_t *z80, uint8_t input)
{
  uint8_t output;
  output = input + 1;
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = !(output & 0xF) ? 1 : 0;
  flag(z80).pv = (input == 0x7F) ? 1 : 0;
  flag(z80).n = 0;
  return output;
}



static uint8_t z80_dec(z80_t *z80, uint8_t input)
{
  uint8_t output;
  output = input - 1;
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = !(input & 0xF) ? 1 : 0;
  flag(z80).pv = (input == 0x80) ? 1 : 0;
  flag(z80).n = 1;
  return output;
}



static uint8_t z80_rlc(z80_t *z80, uint8_t input)
{
  uint8_t output = input << 1;
  if (input & 0x80) {
    output |= 0x01;
  }
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x80) ? 1 : 0;
  return output;
}



static uint8_t z80_rrc(z80_t *z80, uint8_t input)
{
  uint8_t output = input >> 1;
  if (input & 0x01) {
    output |= 0x80;
  }
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x01) ? 1 : 0;
  return output;
}



static uint8_t z80_rl(z80_t *z80, uint8_t input)
{
  uint8_t output = input << 1;
  if (flag(z80).c) {
    output |= 0x01;
  }
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x80) ? 1 : 0;
  return output;
}



static uint8_t z80_rr(z80_t *z80, uint8_t input)
{
  uint8_t output = input >> 1;
  if (flag(z80).c) {
    output |= 0x80;
  }
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x01) ? 1 : 0;
  return output;
}



static uint8_t z80_sla(z80_t *z80, uint8_t input)
{
  uint8_t output = input << 1;
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x80) ? 1 : 0;
  return output;
}



static uint8_t z80_sra(z80_t *z80, uint8_t input)
{
  uint8_t output = input >> 1;
  output |= input & 0x80;
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x01) ? 1 : 0;
  return output;
}



static uint8_t z80_sll(z80_t *z80, uint8_t input)
{
  uint8_t output = input << 1;
  output |= 0x01;
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x80) ? 1 : 0;
  return output;
}



static uint8_t z80_srl(z80_t *z80, uint8_t input)
{
  uint8_t output = input >> 1;
  flag(z80).s = (output & 0x80) ? 1 : 0;
  flag(z80).z = (output == 0) ? 1 : 0;
  flag(z80).h = 0;
  flag(z80).pv = parity_even(output);
  flag(z80).n = 0;
  flag(z80).c = (input & 0x01) ? 1 : 0;
  return output;
}



void z80_init(z80_t *z80)
{
  memset(z80, 0, sizeof(z80_t));
}



void z80_execute(z80_t *z80, mem_t *mem)
{
  /* Implementing decoding logic described at: http://z80.info/decoding.htm */
  uint8_t mc[4];

  mc[0] = mem_read(mem, z80->pc);
  mc[1] = mem_read(mem, z80->pc + 1);
  mc[2] = mem_read(mem, z80->pc + 2);
  mc[3] = mem_read(mem, z80->pc + 3);

  if (mc[0] == 0xDD && mc[1] == 0xCB) {
    /* DDCB prefixed */
    int8_t displacement = mc[2];
    uint8_t x =  mc[3] >> 6;
    uint8_t y = (mc[3] >> 3) & 7;
    uint8_t z =  mc[3] & 7;

    if (0 == x && 6 == z) {
      dt_t dt = dt_rot[y];
      z80_trace(z80, mem, 4, "%s (IX)", dt_text(dt));
      switch (dt) {
      case DT_ROT_RLC: 
        mem_write(mem, ix(z80) + displacement,
          z80_rlc(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_RRC: 
        mem_write(mem, ix(z80) + displacement,
          z80_rrc(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_RL:  
        mem_write(mem, ix(z80) + displacement,
          z80_rl(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_RR:  
        mem_write(mem, ix(z80) + displacement,
          z80_rr(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_SLA: 
        mem_write(mem, ix(z80) + displacement,
          z80_sla(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_SRA: 
        mem_write(mem, ix(z80) + displacement,
          z80_sra(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_SLL: 
        mem_write(mem, ix(z80) + displacement,
          z80_sll(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      case DT_ROT_SRL: 
        mem_write(mem, ix(z80) + displacement,
          z80_srl(z80, mem_read(mem, ix(z80) + displacement)));
        break;
      default: break;
      }
      z80->pc += 4;

    } else if (1 == x) {
      z80_trace(z80, mem, 4, "BIT %d,(IX)", y);
      z80_bit(z80, mem_read(mem, ix(z80) + displacement), y);
      z80->pc += 4;

    } else if (2 == x && 6 == z) {
      z80_trace(z80, mem, 4, "RES %d,(IX)", y);
      mem_write(mem, ix(z80) + displacement,
        reset_bit(mem_read(mem, ix(z80) + displacement), y));
      z80->pc += 4;

    } else if (3 == x && 6 == z) {
      z80_trace(z80, mem, 4, "SET %d,(IX)", y);
      mem_write(mem, ix(z80) + displacement,
        set_bit(mem_read(mem, ix(z80) + displacement), y));
      z80->pc += 4;

    } else {
      panic("Unhandled DDCB opcode: %02x [x=%d z=%d y=%d]\n", mc[3], x, z, y);
      z80->pc += 4;
    }

  } else if (mc[0] == 0xFD && mc[1] == 0xCB) {
    /* FDCB prefixed */
    int8_t displacement = mc[2];
    uint8_t x =  mc[3] >> 6;
    uint8_t y = (mc[3] >> 3) & 7;
    uint8_t z =  mc[3] & 7;

    if (0 == x && 6 == z) {
      dt_t dt = dt_rot[y];
      z80_trace(z80, mem, 4, "%s (IY)", dt_text(dt));
      switch (dt) {
      case DT_ROT_RLC: 
        mem_write(mem, iy(z80) + displacement,
          z80_rlc(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_RRC: 
        mem_write(mem, iy(z80) + displacement,
          z80_rrc(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_RL:  
        mem_write(mem, iy(z80) + displacement,
          z80_rl(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_RR:  
        mem_write(mem, iy(z80) + displacement,
          z80_rr(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_SLA: 
        mem_write(mem, iy(z80) + displacement,
          z80_sla(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_SRA: 
        mem_write(mem, iy(z80) + displacement,
          z80_sra(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_SLL: 
        mem_write(mem, iy(z80) + displacement,
          z80_sll(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      case DT_ROT_SRL: 
        mem_write(mem, iy(z80) + displacement,
          z80_srl(z80, mem_read(mem, iy(z80) + displacement)));
        break;
      default: break;
      }
      z80->pc += 4;

    } else if (1 == x) {
      z80_trace(z80, mem, 4, "BIT %d,(IY)", y);
      z80_bit(z80, mem_read(mem, iy(z80) + displacement), y);
      z80->pc += 4;

    } else if (2 == x && 6 == z) {
      z80_trace(z80, mem, 4, "RES %d,(IY)", y);
      mem_write(mem, iy(z80) + displacement,
        reset_bit(mem_read(mem, iy(z80) + displacement), y));
      z80->pc += 4;

    } else if (3 == x && 6 == z) {
      z80_trace(z80, mem, 4, "SET %d,(IY)", y);
      mem_write(mem, iy(z80) + displacement,
        set_bit(mem_read(mem, iy(z80) + displacement), y));
      z80->pc += 4;

    } else {
      panic("Unhandled FDCB opcode: %02x [x=%d z=%d y=%d]\n", mc[3], x, z, y);
      z80->pc += 4;
    }

  } else if (mc[0] == 0xCB) {
    /* CB prefixed */
    uint8_t x =  mc[1] >> 6;
    uint8_t y = (mc[1] >> 3) & 7;
    uint8_t z =  mc[1] & 7;

    if (0 == x) {
      dt_t dt_op = dt_rot[y];
      dt_t dt_reg = dt_r[z];
      z80_trace(z80, mem, 2, "%s %s", dt_text(dt_op), dt_text(dt_reg));
      uint8_t value = 0;
      switch (dt_reg) {
      case DT_R_B:   value = b(z80); break;
      case DT_R_C:   value = c(z80); break;
      case DT_R_D:   value = d(z80); break;
      case DT_R_E:   value = e(z80); break;
      case DT_R_H:   value = h(z80); break;
      case DT_R_L:   value = l(z80); break;
      case DT_R_HLI: value = mem_read(mem, hl(z80)); break;
      case DT_R_A:   value = a(z80); break;
      default: break;
      }
      switch (dt_op) {
      case DT_ROT_RLC: value = z80_rlc(z80, value); break;
      case DT_ROT_RRC: value = z80_rrc(z80, value); break;
      case DT_ROT_RL:  value = z80_rl(z80, value); break;
      case DT_ROT_RR:  value = z80_rr(z80, value); break;
      case DT_ROT_SLA: value = z80_sla(z80, value); break;
      case DT_ROT_SRA: value = z80_sra(z80, value); break;
      case DT_ROT_SLL: value = z80_sll(z80, value); break;
      case DT_ROT_SRL: value = z80_srl(z80, value); break;
      default: break;
      }
      switch (dt_reg) {
      case DT_R_B:   b(z80) = value; break;
      case DT_R_C:   c(z80) = value; break;
      case DT_R_D:   d(z80) = value; break;
      case DT_R_E:   e(z80) = value; break;
      case DT_R_H:   h(z80) = value; break;
      case DT_R_L:   l(z80) = value; break;
      case DT_R_HLI: mem_write(mem, hl(z80), value); break;
      case DT_R_A:   a(z80) = value; break;
      default: break;
      }
      z80->pc += 2;

    } else if (1 == x) {
      dt_t dt = dt_r[z];
      z80_trace(z80, mem, 2, "BIT %d,%s", y, dt_text(dt));
      switch (dt) {
      case DT_R_B:   z80_bit(z80, b(z80), y); break;
      case DT_R_C:   z80_bit(z80, c(z80), y); break;
      case DT_R_D:   z80_bit(z80, d(z80), y); break;
      case DT_R_E:   z80_bit(z80, e(z80), y); break;
      case DT_R_H:   z80_bit(z80, h(z80), y); break;
      case DT_R_L:   z80_bit(z80, l(z80), y); break;
      case DT_R_HLI: z80_bit(z80, mem_read(mem, hl(z80)), y); break;
      case DT_R_A:   z80_bit(z80, a(z80), y); break;
      default: break;
      }
      z80->pc += 2;

    } else if (2 == x) {
      dt_t dt = dt_r[z];
      z80_trace(z80, mem, 2, "RES %d,%s", y, dt_text(dt));
      switch (dt) {
      case DT_R_B:   b(z80) = reset_bit(b(z80), y); break;
      case DT_R_C:   c(z80) = reset_bit(c(z80), y); break;
      case DT_R_D:   d(z80) = reset_bit(d(z80), y); break;
      case DT_R_E:   e(z80) = reset_bit(e(z80), y); break;
      case DT_R_H:   h(z80) = reset_bit(h(z80), y); break;
      case DT_R_L:   l(z80) = reset_bit(l(z80), y); break;
      case DT_R_HLI:
        mem_write(mem, hl(z80),
          reset_bit(mem_read(mem, hl(z80)), y)); break;
      case DT_R_A:   a(z80) = reset_bit(a(z80), y); break;
      default: break;
      }
      z80->pc += 2;

    } else if (3 == x) {
      dt_t dt = dt_r[z];
      z80_trace(z80, mem, 2, "SET %d,%s", y, dt_text(dt));
      switch (dt) {
      case DT_R_B:   b(z80) = set_bit(b(z80), y); break;
      case DT_R_C:   c(z80) = set_bit(c(z80), y); break;
      case DT_R_D:   d(z80) = set_bit(d(z80), y); break;
      case DT_R_E:   e(z80) = set_bit(e(z80), y); break;
      case DT_R_H:   h(z80) = set_bit(h(z80), y); break;
      case DT_R_L:   l(z80) = set_bit(l(z80), y); break;
      case DT_R_HLI:
        mem_write(mem, hl(z80),
          set_bit(mem_read(mem, hl(z80)), y)); break;
      case DT_R_A:   a(z80) = set_bit(a(z80), y); break;
      default: break;
      }
      z80->pc += 2;

    } else {
      panic("Unhandled CB opcode: %02x [x=%d z=%d y=%d]\n", mc[1], x, z, y);
      z80->pc += 2;
    }

  } else if (mc[0] == 0xED) {
    /* ED prefixed */
    uint8_t x =  mc[1] >> 6;
    uint8_t y = (mc[1] >> 3) & 7;
    uint8_t z =  mc[1] & 7;
    uint8_t p = y >> 1;
    uint8_t q = y % 2;

    if (1 == x && 1 == z && 6 != y) {
      dt_t dt = dt_r[y];
      z80_trace(z80, mem, 2, "OUT (C),%s", dt_text(dt));
      switch (dt) {
      case DT_R_B: io_write(c(z80), b(z80), b(z80), mem); break;
      case DT_R_C: io_write(c(z80), b(z80), c(z80), mem); break;
      case DT_R_D: io_write(c(z80), b(z80), d(z80), mem); break;
      case DT_R_E: io_write(c(z80), b(z80), e(z80), mem); break;
      case DT_R_H: io_write(c(z80), b(z80), h(z80), mem); break;
      case DT_R_L: io_write(c(z80), b(z80), l(z80), mem); break;
      case DT_R_A: io_write(c(z80), b(z80), a(z80), mem); break;
      default: break;
      }
      z80->pc += 2;

    } else if (1 == x && 2 == z && 0 == q) {
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 2, "SBC HL,%s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: z80_sbc_16(z80, &hl(z80), bc(z80)); break;
      case DT_RP_DE: z80_sbc_16(z80, &hl(z80), de(z80)); break;
      case DT_RP_HL: z80_sbc_16(z80, &hl(z80), hl(z80)); break;
      case DT_RP_SP: z80_sbc_16(z80, &hl(z80), z80->sp); break;
      default: break;
      }
      z80->pc += 2;

    } else if (1 == x && 2 == z && 1 == q) {
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 2, "ADC HL,%s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: z80_adc_16(z80, &hl(z80), bc(z80)); break;
      case DT_RP_DE: z80_adc_16(z80, &hl(z80), de(z80)); break;
      case DT_RP_HL: z80_adc_16(z80, &hl(z80), hl(z80)); break;
      case DT_RP_SP: z80_adc_16(z80, &hl(z80), z80->sp); break;
      default: break;
      }
      z80->pc += 2;

    } else if (1 == x && 3 == z && 0 == q) {
      uint16_t address = (mc[3] << 8) + mc[2];
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 4, "LD (%04x),%s", address, dt_text(dt));
      switch (dt) {
      case DT_RP_BC: 
        mem_write(mem, address,     c(z80));
        mem_write(mem, address + 1, b(z80));
        break;
      case DT_RP_DE:
        mem_write(mem, address,     e(z80)); 
        mem_write(mem, address + 1, d(z80)); 
        break;
      case DT_RP_HL:
        mem_write(mem, address,     l(z80));
        mem_write(mem, address + 1, h(z80));
        break;
      case DT_RP_SP:
        mem_write(mem, address,     z80->sp % 256);
        mem_write(mem, address + 1, z80->sp / 256);
        break;
      default:
        break;
      }
      z80->pc += 4;

    } else if (1 == x && 3 == z && 1 == q) {
      uint16_t address = (mc[3] << 8) + mc[2];
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 4, "LD %s,(%04x)", dt_text(dt), address);
      switch (dt) {
      case DT_RP_BC:
        c(z80) = mem_read(mem, address);
        b(z80) = mem_read(mem, address + 1);
        break;
      case DT_RP_DE:
        e(z80) = mem_read(mem, address);
        d(z80) = mem_read(mem, address + 1);
        break;
      case DT_RP_HL:
        l(z80) = mem_read(mem, address);
        h(z80) = mem_read(mem, address + 1);
        break;
      case DT_RP_SP:
        z80->sp =  mem_read(mem, address);
        z80->sp += mem_read(mem, address + 1) << 8;
        break;
      default:
        break;
      }
      z80->pc += 4;

    } else if (1 == x && 4 == z) {
      z80_trace(z80, mem, 2, "NEG");
      uint8_t value = a(z80);
      a(z80) = 0;
      z80_sub(z80, value);
      z80->pc += 2;

    } else if (1 == x && 6 == z) {
      dt_t dt = dt_im[y];
      z80_trace(z80, mem, 2, "IM %s", dt_text(dt));
      panic("Unimplemented IM (%02x)\n", dt);
      z80->pc += 2;

    } else if (1 == x && 7 == z && 0 == y) {
      z80_trace(z80, mem, 2, "LD I,A");
      z80->i = a(z80);
      z80->pc += 2;

    } else if (1 == x && 7 == z && 1 == y) {
      z80_trace(z80, mem, 2, "LD R,A");
      z80->r = a(z80);
      z80->pc += 2;

    } else if (1 == x && 7 == z && 2 == y) {
      z80_trace(z80, mem, 2, "LD A,I");
      a(z80) = z80->i;
      z80->pc += 2;

    } else if (1 == x && 7 == z && 3 == y) {
      z80_trace(z80, mem, 2, "LD A,R");
      a(z80) = z80->r;
      z80->pc += 2;

    } else if (1 == x && 7 == z && 4 == y) {
      z80_trace(z80, mem, 2, "RRD");
      uint8_t value_a = a(z80);
      uint8_t value_hl = mem_read(mem, hl(z80));
      a(z80) = (a(z80) & 0xF0) | (value_hl & 0x0F);
      mem_write(mem, hl(z80),
        ((value_a << 4) & 0xF0) | ((value_hl >> 4) & 0x0F));
      flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
      flag(z80).z = (a(z80) == 0) ? 1 : 0;
      flag(z80).h = 0;
      flag(z80).pv = parity_even(a(z80));
      flag(z80).n = 0;
      z80->pc += 2;

    } else if (1 == x && 7 == z && 5 == y) {
      z80_trace(z80, mem, 2, "RLD");
      uint8_t value_a = a(z80);
      uint8_t value_hl = mem_read(mem, hl(z80));
      a(z80) = (a(z80) & 0xF0) | ((value_hl >> 4) & 0x0F);
      mem_write(mem, hl(z80),
        ((value_hl << 4) & 0xF0) | (value_a & 0x0F));
      flag(z80).s = (a(z80) & 0x80) ? 1 : 0;
      flag(z80).z = (a(z80) == 0) ? 1 : 0;
      flag(z80).h = 0;
      flag(z80).pv = parity_even(a(z80));
      flag(z80).n = 0;
      z80->pc += 2;

    } else if (2 == x && 3 >= z && 4 <= y) {
      dt_t dt = dt_bli[y-4][z];
      z80_trace(z80, mem, 2, "%s", dt_text(dt));
      switch (dt) {
      case DT_BLI_LDI: z80_ldi(z80, mem); break;
      case DT_BLI_LDD: z80_ldd(z80, mem); break;
      case DT_BLI_LDIR:
        z80_ldi(z80, mem);
        if (flag(z80).pv) {
          z80->pc -= 2; /* Repeat */
        }
        break;
      case DT_BLI_LDDR:
        z80_ldd(z80, mem);
        if (flag(z80).pv) {
          z80->pc -= 2; /* Repeat */
        }
        break;
      case DT_BLI_CPI: z80_cpi(z80, mem); break;
      case DT_BLI_CPD: z80_cpd(z80, mem); break;
      case DT_BLI_CPIR:
        z80_cpi(z80, mem);
        if (flag(z80).pv && flag(z80).z == 0) {
          z80->pc -= 2; /* Repeat */
        }
        break;
      case DT_BLI_CPDR:
        z80_cpd(z80, mem);
        if (flag(z80).pv && flag(z80).z == 0) {
          z80->pc -= 2; /* Repeat */
        }
        break;
      case DT_BLI_INI:  panic("Unimplemented INI\n");  break;
      case DT_BLI_IND:  panic("Unimplemented IND\n");  break;
      case DT_BLI_INIR: panic("Unimplemented INIR\n"); break;
      case DT_BLI_INDR: panic("Unimplemented INDR\n"); break;
      case DT_BLI_OUTI: panic("Unimplemented OUTI\n"); break;
      case DT_BLI_OUTD: panic("Unimplemented OUTD\n"); break;
      case DT_BLI_OTIR: panic("Unimplemented OTIR\n"); break;
      case DT_BLI_OTDR: panic("Unimplemented OTDR\n"); break;
      default: break;
      }
      z80->pc += 2;

    } else {
      panic("Unhandled ED opcode: %02x [x=%d z=%d (y=%d | q=%d p=%d)]\n",
        mc[1], x, z, y, q, p);
      z80->pc += 2;
    }

  } else if (mc[0] == 0xDD) {
    /* DD prefixed */
    uint8_t x =  mc[1] >> 6;
    uint8_t y = (mc[1] >> 3) & 7;
    uint8_t z =  mc[1] & 7;
    uint8_t p = y >> 1;
    uint8_t q = y % 2;

    if (0 == x && 1 == z && 0 == q && 2 == p) {
      uint16_t value = (mc[3] << 8) + mc[2];
      z80_trace(z80, mem, 4, "LD IX,%04x", value);
      ix(z80) = value;
      z80->pc += 4;

    } else if (0 == x && 1 == z && 1 == q) {
      dt_t dt = dt_rpix[p];
      z80_trace(z80, mem, 2, "ADD IX,%s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: z80_add_16(z80, &ix(z80), bc(z80)); break;
      case DT_RP_DE: z80_add_16(z80, &ix(z80), de(z80)); break;
      case DT_RP_IX: z80_add_16(z80, &ix(z80), ix(z80)); break;
      case DT_RP_SP: z80_add_16(z80, &ix(z80), z80->sp); break;
      default: break;
      }
      z80->pc += 2;

    } else if (0 == x && 2 == z && 4 == y) {
      uint16_t address = (mc[3] << 8) + mc[2];
      z80_trace(z80, mem, 4, "LD (%04x),IX", address);
      mem_write(mem, address,     ixl(z80));
      mem_write(mem, address + 1, ixh(z80));
      z80->pc += 4;

    } else if (0 == x && 2 == z && 5 == y) {
      uint16_t address = (mc[3] << 8) + mc[2];
      z80_trace(z80, mem, 4, "LD IX,(%04x)", address);
      ixl(z80) = mem_read(mem, address);
      ixh(z80) = mem_read(mem, address + 1);
      z80->pc += 4;

    } else if (0 == x && 3 == z && 4 == y) {
      z80_trace(z80, mem, 2, "INC IX");
      ix(z80)++;
      z80->pc += 2;

    } else if (0 == x && 3 == z && 5 == y) {
      z80_trace(z80, mem, 2, "DEC IX");
      ix(z80)--;
      z80->pc += 2;

    } else if (0 == x && 4 == z && 4 == y) {
      z80_trace(z80, mem, 2, "INC IXH");
      ixh(z80) = z80_inc(z80, ixh(z80));
      z80->pc += 2;

    } else if (0 == x && 4 == z && 5 == y) {
      z80_trace(z80, mem, 2, "INC IXL");
      ixl(z80) = z80_inc(z80, ixl(z80));
      z80->pc += 2;

    } else if (0 == x && 4 == z && 6 == y) {
      int8_t displacement = mc[2];
      z80_trace(z80, mem, 3, "INC (IX)");
      mem_write(mem, ix(z80) + displacement, 
        z80_inc(z80, mem_read(mem, ix(z80) + displacement)));
      z80->pc += 3;

    } else if (0 == x && 5 == z && 4 == y) {
      z80_trace(z80, mem, 2, "DEC IXH");
      ixh(z80) = z80_dec(z80, ixh(z80));
      z80->pc += 2;

    } else if (0 == x && 5 == z && 5 == y) {
      z80_trace(z80, mem, 2, "DEC IXL");
      ixl(z80) = z80_dec(z80, ixl(z80));
      z80->pc += 2;

    } else if (0 == x && 5 == z && 6 == y) {
      int8_t displacement = mc[2];
      z80_trace(z80, mem, 3, "DEC (IX)");
      mem_write(mem, ix(z80) + displacement, 
        z80_dec(z80, mem_read(mem, ix(z80) + displacement)));
      z80->pc += 3;

    } else if (0 == x && 6 == z && 4 == y) {
      uint8_t value = mc[2];
      z80_trace(z80, mem, 3, "LD IXH,%02x", value);
      ixh(z80) = value;
      z80->pc += 3;

    } else if (0 == x && 6 == z && 5 == y) {
      uint8_t value = mc[2];
      z80_trace(z80, mem, 3, "LD IXL,%02x", value);
      ixl(z80) = value;
      z80->pc += 3;

    } else if (0 == x && 6 == z && 6 == y) {
      int8_t displacement = mc[2];
      uint8_t value = mc[3];
      z80_trace(z80, mem, 4, "LD (IX),%02x", value);
      mem_write(mem, ix(z80) + displacement, value);
      z80->pc += 4;

    } else if (1 == x && 6 != z) {
      int8_t displacement = mc[2];
      dt_t dt_dst = dt_rix[y];
      dt_t dt_src = dt_rix[z];
      z80_trace(z80, mem, 3, "LD %s,%s", dt_text(dt_dst), dt_text(dt_src));
      uint8_t value = 0;
      switch (dt_src) {
      case DT_R_B:   value = b(z80); break;
      case DT_R_C:   value = c(z80); break;
      case DT_R_D:   value = d(z80); break;
      case DT_R_E:   value = e(z80); break;
      case DT_R_IXH: value = ixh(z80); break;
      case DT_R_IXL: value = ixl(z80); break;
      case DT_R_A:   value = a(z80); break;
      default: break;
      }
      switch (dt_dst) {
      case DT_R_B:   b(z80) = value; break;
      case DT_R_C:   c(z80) = value; break;
      case DT_R_D:   d(z80) = value; break;
      case DT_R_E:   e(z80) = value; break;
      case DT_R_IXH: ixh(z80) = value; break;
      case DT_R_IXL: ixl(z80) = value; break;
      case DT_R_IXI:
        if (dt_src == DT_R_IXH) value = h(z80);
        if (dt_src == DT_R_IXL) value = l(z80);
        mem_write(mem, ix(z80) + displacement, value);
        break;
      case DT_R_A:   a(z80) = value; break;
      default: break;
      }
      if (dt_dst == DT_R_IXI) {
        z80->pc += 3;
      } else {
        z80->pc += 2;
      }

    } else if (1 == x && 6 == z) {
      int8_t displacement = mc[2];
      dt_t dt = dt_r[y];
      z80_trace(z80, mem, 3, "LD %s,(IX)", dt_text(dt));
      switch (dt) {
      case DT_R_B: b(z80) = mem_read(mem, ix(z80) + displacement); break;
      case DT_R_C: c(z80) = mem_read(mem, ix(z80) + displacement); break;
      case DT_R_D: d(z80) = mem_read(mem, ix(z80) + displacement); break;
      case DT_R_E: e(z80) = mem_read(mem, ix(z80) + displacement); break;
      case DT_R_H: h(z80) = mem_read(mem, ix(z80) + displacement); break;
      case DT_R_L: l(z80) = mem_read(mem, ix(z80) + displacement); break;
      case DT_R_A: a(z80) = mem_read(mem, ix(z80) + displacement); break;
      default: break;
      }
      z80->pc += 3;

    } else if (2 == x) {
      int8_t displacement = mc[2];
      dt_t dt_op  = dt_alu[y];
      dt_t dt_reg = dt_rix[z];
      z80_trace(z80, mem, 3, "%s%s", dt_text(dt_op), dt_text(dt_reg));
      uint8_t value = 0;
      switch (dt_reg) {
      case DT_R_IXH: value = ixh(z80); break;
      case DT_R_IXL: value = ixl(z80); break;
      case DT_R_IXI: value = mem_read(mem, ix(z80) + displacement); break;
      default: break;
      }
      switch (dt_op) {
      case DT_ALU_ADD: z80_add(z80, value); break;
      case DT_ALU_ADC: z80_adc(z80, value); break;
      case DT_ALU_SUB: z80_sub(z80, value); break;
      case DT_ALU_SBC: z80_sbc(z80, value); break;
      case DT_ALU_AND: z80_and(z80, value); break;
      case DT_ALU_XOR: z80_xor(z80, value); break;
      case DT_ALU_OR:  z80_or(z80, value); break;
      case DT_ALU_CP:  z80_compare(z80, value); break;
      default: break;
      }
      if (dt_reg == DT_R_IXI) {
        z80->pc += 3;
      } else {
        z80->pc += 2;
      }

    } else if (3 == x && 1 == z && 4 == y) {
      z80_trace(z80, mem, 2, "POP IX");
      ix(z80)  = mem_read(mem, z80->sp);
      z80->sp++;
      ix(z80) += mem_read(mem, z80->sp) << 8;
      z80->sp++;
      z80->pc += 2;

    } else if (3 == x && 5 == z && 4 == y) {
      z80_trace(z80, mem, 2, "PUSH IX");
      z80->sp--;
      mem_write(mem, z80->sp, ix(z80) / 256);
      z80->sp--;
      mem_write(mem, z80->sp, ix(z80) % 256);
      z80->pc += 2;

    } else {
      panic("Unhandled DD opcode: %02x [x=%d z=%d (y=%d | q=%d p=%d)]\n",
        mc[1], x, z, y, q, p);
      z80->pc += 2;
    }

  } else if (mc[0] == 0xFD) {
    /* FD prefixed */
    uint8_t x =  mc[1] >> 6;
    uint8_t y = (mc[1] >> 3) & 7;
    uint8_t z =  mc[1] & 7;
    uint8_t p = y >> 1;
    uint8_t q = y % 2;

    if (0 == x && 1 == z && 0 == q && 2 == p) {
      uint16_t value = (mc[3] << 8) + mc[2];
      z80_trace(z80, mem, 4, "LD IY,%04x", value);
      iy(z80) = value;
      z80->pc += 4;

    } else if (0 == x && 1 == z && 1 == q) {
      dt_t dt = dt_rpiy[p];
      z80_trace(z80, mem, 2, "ADD IY,%s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: z80_add_16(z80, &iy(z80), bc(z80)); break;
      case DT_RP_DE: z80_add_16(z80, &iy(z80), de(z80)); break;
      case DT_RP_IY: z80_add_16(z80, &iy(z80), iy(z80)); break;
      case DT_RP_SP: z80_add_16(z80, &iy(z80), z80->sp); break;
      default: break;
      }
      z80->pc += 2;

    } else if (0 == x && 2 == z && 4 == y) {
      uint16_t address = (mc[3] << 8) + mc[2];
      z80_trace(z80, mem, 4, "LD (%04x),IY", address);
      mem_write(mem, address,     iyl(z80));
      mem_write(mem, address + 1, iyh(z80));
      z80->pc += 4;

    } else if (0 == x && 2 == z && 5 == y) {
      uint16_t address = (mc[3] << 8) + mc[2];
      z80_trace(z80, mem, 4, "LD IY,(%04x)", address);
      iyl(z80) = mem_read(mem, address);
      iyh(z80) = mem_read(mem, address + 1);
      z80->pc += 4;

    } else if (0 == x && 3 == z && 4 == y) {
      z80_trace(z80, mem, 2, "INC IY");
      iy(z80)++;
      z80->pc += 2;

    } else if (0 == x && 3 == z && 5 == y) {
      z80_trace(z80, mem, 2, "DEC IY");
      iy(z80)--;
      z80->pc += 2;

    } else if (0 == x && 4 == z && 4 == y) {
      z80_trace(z80, mem, 2, "INC IYH");
      iyh(z80) = z80_inc(z80, iyh(z80));
      z80->pc += 2;

    } else if (0 == x && 4 == z && 5 == y) {
      z80_trace(z80, mem, 2, "INC IYL");
      iyl(z80) = z80_inc(z80, iyl(z80));
      z80->pc += 2;

    } else if (0 == x && 4 == z && 6 == y) {
      int8_t displacement = mc[2];
      z80_trace(z80, mem, 3, "INC (IY)");
      mem_write(mem, iy(z80) + displacement, 
        z80_inc(z80, mem_read(mem, iy(z80) + displacement)));
      z80->pc += 3;

    } else if (0 == x && 5 == z && 4 == y) {
      z80_trace(z80, mem, 2, "DEC IYH");
      iyh(z80) = z80_dec(z80, iyh(z80));
      z80->pc += 2;

    } else if (0 == x && 5 == z && 5 == y) {
      z80_trace(z80, mem, 2, "DEC IYL");
      iyl(z80) = z80_dec(z80, iyl(z80));
      z80->pc += 2;

    } else if (0 == x && 5 == z && 6 == y) {
      int8_t displacement = mc[2];
      z80_trace(z80, mem, 3, "DEC (IY)");
      mem_write(mem, iy(z80) + displacement, 
        z80_dec(z80, mem_read(mem, iy(z80) + displacement)));
      z80->pc += 3;

    } else if (0 == x && 6 == z && 4 == y) {
      uint8_t value = mc[2];
      z80_trace(z80, mem, 3, "LD IYH,%02x", value);
      iyh(z80) = value;
      z80->pc += 3;

    } else if (0 == x && 6 == z && 5 == y) {
      uint8_t value = mc[2];
      z80_trace(z80, mem, 3, "LD IYL,%02x", value);
      iyl(z80) = value;
      z80->pc += 3;

    } else if (0 == x && 6 == z && 6 == y) {
      int8_t displacement = mc[2];
      uint8_t value = mc[3];
      z80_trace(z80, mem, 4, "LD (IY),%02x", value);
      mem_write(mem, iy(z80) + displacement, value);
      z80->pc += 4;

    } else if (1 == x && 6 != z) {
      int8_t displacement = mc[2];
      dt_t dt_dst = dt_riy[y];
      dt_t dt_src = dt_riy[z];
      z80_trace(z80, mem, 3, "LD %s,%s", dt_text(dt_dst), dt_text(dt_src));
      uint8_t value = 0;
      switch (dt_src) {
      case DT_R_B:   value = b(z80); break;
      case DT_R_C:   value = c(z80); break;
      case DT_R_D:   value = d(z80); break;
      case DT_R_E:   value = e(z80); break;
      case DT_R_IYH: value = iyh(z80); break;
      case DT_R_IYL: value = iyl(z80); break;
      case DT_R_A:   value = a(z80); break;
      default: break;
      }
      switch (dt_dst) {
      case DT_R_B:   b(z80) = value; break;
      case DT_R_C:   c(z80) = value; break;
      case DT_R_D:   d(z80) = value; break;
      case DT_R_E:   e(z80) = value; break;
      case DT_R_IYH: iyh(z80) = value; break;
      case DT_R_IYL: iyl(z80) = value; break;
      case DT_R_IYI:
        if (dt_src == DT_R_IYH) value = h(z80);
        if (dt_src == DT_R_IYL) value = l(z80);
        mem_write(mem, iy(z80) + displacement, value);
        break;
      case DT_R_A:   a(z80) = value; break;
      default: break;
      }
      if (dt_dst == DT_R_IYI) {
        z80->pc += 3;
      } else {
        z80->pc += 2;
      }

    } else if (1 == x && 6 == z) {
      int8_t displacement = mc[2];
      dt_t dt = dt_r[y];
      z80_trace(z80, mem, 3, "LD %s,(IY)", dt_text(dt));
      switch (dt) {
      case DT_R_B: b(z80) = mem_read(mem, iy(z80) + displacement); break;
      case DT_R_C: c(z80) = mem_read(mem, iy(z80) + displacement); break;
      case DT_R_D: d(z80) = mem_read(mem, iy(z80) + displacement); break;
      case DT_R_E: e(z80) = mem_read(mem, iy(z80) + displacement); break;
      case DT_R_H: h(z80) = mem_read(mem, iy(z80) + displacement); break;
      case DT_R_L: l(z80) = mem_read(mem, iy(z80) + displacement); break;
      case DT_R_A: a(z80) = mem_read(mem, iy(z80) + displacement); break;
      default: break;
      }
      z80->pc += 3;

    } else if (2 == x) {
      int8_t displacement = mc[2];
      dt_t dt_op  = dt_alu[y];
      dt_t dt_reg = dt_riy[z];
      z80_trace(z80, mem, 3, "%s%s", dt_text(dt_op), dt_text(dt_reg));
      uint8_t value = 0;
      switch (dt_reg) {
      case DT_R_IYH: value = iyh(z80); break;
      case DT_R_IYL: value = iyl(z80); break;
      case DT_R_IYI: value = mem_read(mem, iy(z80) + displacement); break;
      default: break;
      }
      switch (dt_op) {
      case DT_ALU_ADD: z80_add(z80, value); break;
      case DT_ALU_ADC: z80_adc(z80, value); break;
      case DT_ALU_SUB: z80_sub(z80, value); break;
      case DT_ALU_SBC: z80_sbc(z80, value); break;
      case DT_ALU_AND: z80_and(z80, value); break;
      case DT_ALU_XOR: z80_xor(z80, value); break;
      case DT_ALU_OR:  z80_or(z80, value); break;
      case DT_ALU_CP:  z80_compare(z80, value); break;
      default: break;
      }
      if (dt_reg == DT_R_IYI) {
        z80->pc += 3;
      } else {
        z80->pc += 2;
      }

    } else if (3 == x && 1 == z && 4 == y) {
      z80_trace(z80, mem, 2, "POP IY");
      iy(z80)  = mem_read(mem, z80->sp);
      z80->sp++;
      iy(z80) += mem_read(mem, z80->sp) << 8;
      z80->sp++;
      z80->pc += 2;

    } else if (3 == x && 5 == z && 4 == y) {
      z80_trace(z80, mem, 2, "PUSH IY");
      z80->sp--;
      mem_write(mem, z80->sp, iy(z80) / 256);
      z80->sp--;
      mem_write(mem, z80->sp, iy(z80) % 256);
      z80->pc += 2;

    } else {
      panic("Unhandled FD opcode: %02x [x=%d z=%d (y=%d | q=%d p=%d)]\n",
        mc[1], x, z, y, q, p);
      z80->pc += 2;
    }

  } else {
    /* Unprefixed */
    uint8_t x =  mc[0] >> 6;
    uint8_t y = (mc[0] >> 3) & 7;
    uint8_t z =  mc[0] & 7;
    uint8_t p = y >> 1;
    uint8_t q = y % 2;

    if (0 == x && 0 == z && 0 == y) {
      z80_trace(z80, mem, 1, "NOP");
      z80->pc += 1;

    } else if (0 == x && 0 == z && 1 == y) {
      z80_trace(z80, mem, 1, "EX AF,AF'");
      uint16_t value = af(z80);
      af(z80)  = af_(z80);
      af_(z80) = value;
      z80->pc += 1;

    } else if (0 == x && 0 == z && 2 == y) {
      int8_t displacement = mc[1];
      z80_trace(z80, mem, 2, "DJNZ %04x", z80->pc + displacement + 2);
      b(z80)--;
      if (b(z80) == 0) {
        z80->pc += 2;
      } else {
        z80->pc += displacement + 2;
      }

    } else if (0 == x && 0 == z && 3 == y) {
      int8_t displacement = mc[1];
      z80_trace(z80, mem, 2, "JR %04x", z80->pc + displacement + 2);
      z80->pc += displacement + 2;

    } else if (0 == x && 0 == z && 4 <= y) {
      int8_t displacement = mc[1];
      dt_t dt = dt_cc[y-4];
      z80_trace(z80, mem, 2, "JR %s,%04x", dt_text(dt),
        z80->pc + displacement + 2);
      bool go = false;
      switch (dt) {
      case DT_CC_NZ: go = flag(z80).z  == 0; break;
      case DT_CC_Z:  go = flag(z80).z  == 1; break;
      case DT_CC_NC: go = flag(z80).c  == 0; break;
      case DT_CC_C:  go = flag(z80).c  == 1; break;
      case DT_CC_PO: go = flag(z80).pv == 0; break;
      case DT_CC_PE: go = flag(z80).pv == 1; break;
      case DT_CC_P:  go = flag(z80).s  == 0; break;
      case DT_CC_M:  go = flag(z80).s  == 1; break;
      default: break;
      }
      if (go) {
        z80->pc += displacement + 2;
      } else {
        z80->pc += 2;
      }

    } else if (0 == x && 1 == z && 0 == q) {
      uint16_t value = (mc[2] << 8) + mc[1];
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 3, "LD %s,%04x", dt_text(dt), value);
      switch (dt) {
      case DT_RP_BC: bc(z80) = value; break;
      case DT_RP_DE: de(z80) = value; break;
      case DT_RP_HL: hl(z80) = value; break;
      case DT_RP_SP: z80->sp = value; break;
      default: break;
      }
      z80->pc += 3;

    } else if (0 == x && 1 == z && 1 == q) {
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 1, "ADD HL,%s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: z80_add_16(z80, &hl(z80), bc(z80)); break;
      case DT_RP_DE: z80_add_16(z80, &hl(z80), de(z80)); break;
      case DT_RP_HL: z80_add_16(z80, &hl(z80), hl(z80)); break;
      case DT_RP_SP: z80_add_16(z80, &hl(z80), z80->sp); break;
      default: break;
      }
      z80->pc += 1;

    } else if (0 == x && 2 == z && 0 == q && 0 == p) {
      z80_trace(z80, mem, 1, "LD (BC),A");
      mem_write(mem, bc(z80), a(z80));
      z80->pc += 1;

    } else if (0 == x && 2 == z && 0 == q && 1 == p) {
      z80_trace(z80, mem, 1, "LD (DE),A");
      mem_write(mem, de(z80), a(z80));
      z80->pc += 1;

    } else if (0 == x && 2 == z && 0 == q && 2 == p) {
      uint16_t address = (mc[2] << 8) + mc[1];
      z80_trace(z80, mem, 3, "LD (%04x),HL", address);
      mem_write(mem, address, l(z80));
      mem_write(mem, address + 1, h(z80));
      z80->pc += 3;

    } else if (0 == x && 2 == z && 0 == q && 3 == p) {
      uint16_t address = (mc[2] << 8) + mc[1];
      z80_trace(z80, mem, 3, "LD (%04x),A", address);
      mem_write(mem, address, a(z80));
      z80->pc += 3;

    } else if (0 == x && 2 == z && 1 == q && 0 == p) {
      z80_trace(z80, mem, 1, "LD A,(BC)");
      a(z80) = mem_read(mem, bc(z80));
      z80->pc += 1;

    } else if (0 == x && 2 == z && 1 == q && 1 == p) {
      z80_trace(z80, mem, 1, "LD A,(DE)");
      a(z80) = mem_read(mem, de(z80));
      z80->pc += 1;

    } else if (0 == x && 2 == z && 1 == q && 2 == p) {
      uint16_t address = (mc[2] << 8) + mc[1];
      z80_trace(z80, mem, 3, "LD HL,(%04x)", address);
      l(z80) = mem_read(mem, address);
      h(z80) = mem_read(mem, address + 1);
      z80->pc += 3;

    } else if (0 == x && 2 == z && 1 == q && 3 == p) {
      uint16_t address = (mc[2] << 8) + mc[1];
      z80_trace(z80, mem, 3, "LD A,(%04x)", address);
      a(z80) = mem_read(mem, address);
      z80->pc += 3;

    } else if (0 == x && 3 == z && 0 == q) {
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 1, "INC %s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: bc(z80)++; break;
      case DT_RP_DE: de(z80)++; break;
      case DT_RP_HL: hl(z80)++; break;
      case DT_RP_SP: z80->sp++; break;
      default: break;
      }
      z80->pc += 1;

    } else if (0 == x && 3 == z && 1 == q) {
      dt_t dt = dt_rp[p];
      z80_trace(z80, mem, 1, "DEC %s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC: bc(z80)--; break;
      case DT_RP_DE: de(z80)--; break;
      case DT_RP_HL: hl(z80)--; break;
      case DT_RP_SP: z80->sp--; break;
      default: break;
      }
      z80->pc += 1;

    } else if (0 == x && 4 == z) {
      dt_t dt = dt_r[y];
      z80_trace(z80, mem, 1, "INC %s", dt_text(dt));
      switch (dt) {
      case DT_R_B: b(z80) = z80_inc(z80, b(z80)); break;
      case DT_R_C: c(z80) = z80_inc(z80, c(z80)); break;
      case DT_R_D: d(z80) = z80_inc(z80, d(z80)); break;
      case DT_R_E: e(z80) = z80_inc(z80, e(z80)); break;
      case DT_R_H: h(z80) = z80_inc(z80, h(z80)); break;
      case DT_R_L: l(z80) = z80_inc(z80, l(z80)); break;
      case DT_R_HLI:
        mem_write(mem, hl(z80), z80_inc(z80, mem_read(mem, hl(z80))));
        break;
      case DT_R_A: a(z80) = z80_inc(z80, a(z80)); break;
      default: break;
      }
      z80->pc += 1;

    } else if (0 == x && 5 == z) {
      dt_t dt = dt_r[y];
      z80_trace(z80, mem, 1, "DEC %s", dt_text(dt));
      switch (dt) {
      case DT_R_B: b(z80) = z80_dec(z80, b(z80)); break;
      case DT_R_C: c(z80) = z80_dec(z80, c(z80)); break;
      case DT_R_D: d(z80) = z80_dec(z80, d(z80)); break;
      case DT_R_E: e(z80) = z80_dec(z80, e(z80)); break;
      case DT_R_H: h(z80) = z80_dec(z80, h(z80)); break;
      case DT_R_L: l(z80) = z80_dec(z80, l(z80)); break;
      case DT_R_HLI:
        mem_write(mem, hl(z80), z80_dec(z80, mem_read(mem, hl(z80))));
        break;
      case DT_R_A: a(z80) = z80_dec(z80, a(z80)); break;
      default: break;
      }
      z80->pc += 1;

    } else if (0 == x && 6 == z) {
      uint8_t value = mc[1];
      dt_t dt = dt_r[y];
      z80_trace(z80, mem, 2, "LD %s,%02x", dt_text(dt), value);
      switch (dt) {
      case DT_R_B:   b(z80) = value; break;
      case DT_R_C:   c(z80) = value; break;
      case DT_R_D:   d(z80) = value; break;
      case DT_R_E:   e(z80) = value; break;
      case DT_R_H:   h(z80) = value; break;
      case DT_R_L:   l(z80) = value; break;
      case DT_R_HLI: mem_write(mem, hl(z80), value); break;
      case DT_R_A:   a(z80) = value; break;
      default: break;
      }
      z80->pc += 2;

    } else if (0 == x && 7 == z && 0 == y) {
      z80_trace(z80, mem, 1, "RLCA");
      bool bit = a(z80) & 0x80;
      a(z80) <<= 1;
      if (bit) {
        a(z80) |= 0x01;
      }
      flag(z80).c = bit;
      flag(z80).h = 0;
      flag(z80).n = 0;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 1 == y) {
      z80_trace(z80, mem, 1, "RRCA");
      bool bit = a(z80) & 0x01;
      a(z80) >>= 1;
      if (bit) {
        a(z80) |= 0x80;
      }
      flag(z80).c = bit;
      flag(z80).h = 0;
      flag(z80).n = 0;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 2 == y) {
      z80_trace(z80, mem, 1, "RLA");
      bool bit = a(z80) & 0x80;
      a(z80) <<= 1;
      if (flag(z80).c) {
        a(z80) |= 0x01;
      }
      flag(z80).c = bit;
      flag(z80).h = 0;
      flag(z80).n = 0;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 3 == y) {
      z80_trace(z80, mem, 1, "RRA");
      bool bit = a(z80) & 0x01;
      a(z80) >>= 1;
      if (flag(z80).c) {
        a(z80) |= 0x80;
      }
      flag(z80).c = bit;
      flag(z80).h = 0;
      flag(z80).n = 0;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 4 == y) {
      z80_trace(z80, mem, 1, "DAA");
      uint8_t diff;
      bool out_c;
      bool out_h;
      if (flag(z80).c) {
        if ((a(z80) & 0x0F) < 0x0A) {
          diff = flag(z80).h ? 0x66 : 0x60;
        } else {
          diff = 0x66;
        }
        out_c = 1;
      } else {
        if ((a(z80) & 0x0F) < 0x0A) {
          if (((a(z80) >> 4) & 0x0F) < 0xA) {
            diff = flag(z80).h ? 0x06 : 0x00;
          } else {
            diff = flag(z80).h ? 0x66 : 0x60;
          }
        } else {
          diff = (((a(z80) >> 4) & 0x0F) < 0x9) ? 0x06 : 0x66;
        }
        if ((a(z80) & 0x0F) < 0xA) {
          out_c = (((a(z80) >> 4) & 0x0F) < 0x0A) ? 0 : 1;
        } else {
          out_c = (((a(z80) >> 4) & 0x0F) < 0x09) ? 0 : 1;
        }
      }
      if (flag(z80).n) {
        if (flag(z80).h) {
          out_h = ((a(z80) & 0x0F) < 0x06) ? 1 : 0;
        } else {
          out_h = 0;
        }
      } else {
        out_h = ((a(z80) & 0x0F) < 0x0A) ? 0 : 1;
      }
      flag(z80).c = 0;
      (flag(z80).n) ? z80_sub(z80, diff) : z80_add(z80, diff);
      flag(z80).pv = parity_even(a(z80));
      flag(z80).c = out_c;
      flag(z80).h = out_h;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 5 == y) {
      z80_trace(z80, mem, 1, "CPL");
      a(z80) = ~a(z80);
      flag(z80).h = 1;
      flag(z80).n = 1;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 6 == y) {
      z80_trace(z80, mem, 1, "SCF");
      flag(z80).h = 0;
      flag(z80).n = 0;
      flag(z80).c = 1;
      z80->pc += 1;

    } else if (0 == x && 7 == z && 7 == y) {
      z80_trace(z80, mem, 1, "CCF");
      flag(z80).h = flag(z80).c;
      flag(z80).n = 0;
      flag(z80).c = ~flag(z80).c;
      z80->pc += 1;

    } else if (1 == x && !(6 == z && 6 == y)) {
      dt_t dt_dst = dt_r[y];
      dt_t dt_src = dt_r[z];
      z80_trace(z80, mem, 1, "LD %s,%s", dt_text(dt_dst), dt_text(dt_src));
      uint8_t value = 0;
      switch (dt_src) {
      case DT_R_B:   value = b(z80); break;
      case DT_R_C:   value = c(z80); break;
      case DT_R_D:   value = d(z80); break;
      case DT_R_E:   value = e(z80); break;
      case DT_R_H:   value = h(z80); break;
      case DT_R_L:   value = l(z80); break;
      case DT_R_HLI: value = mem_read(mem, hl(z80)); break;
      case DT_R_A:   value = a(z80); break;
      default: break;
      }
      switch (dt_dst) {
      case DT_R_B:   b(z80) = value; break;
      case DT_R_C:   c(z80) = value; break;
      case DT_R_D:   d(z80) = value; break;
      case DT_R_E:   e(z80) = value; break;
      case DT_R_H:   h(z80) = value; break;
      case DT_R_L:   l(z80) = value; break;
      case DT_R_HLI: mem_write(mem, hl(z80), value); break;
      case DT_R_A:   a(z80) = value; break;
      default: break;
      }
      z80->pc += 1;

    } else if (1 == x && (6 == z && 6 == y)) {
      z80_trace(z80, mem, 1, "HALT");
      panic("Unimplemented HALT\n");
      z80->pc += 1;

    } else if (2 == x) {
      dt_t dt_op  = dt_alu[y];
      dt_t dt_reg = dt_r[z];
      z80_trace(z80, mem, 1, "%s%s", dt_text(dt_op), dt_text(dt_reg));
      uint8_t value = 0;
      switch (dt_reg) {
      case DT_R_B:   value = b(z80); break;
      case DT_R_C:   value = c(z80); break;
      case DT_R_D:   value = d(z80); break;
      case DT_R_E:   value = e(z80); break;
      case DT_R_H:   value = h(z80); break;
      case DT_R_L:   value = l(z80); break;
      case DT_R_HLI: value = mem_read(mem, hl(z80)); break;
      case DT_R_A:   value = a(z80); break;
      default: break;
      }
      switch (dt_op) {
      case DT_ALU_ADD: z80_add(z80, value); break;
      case DT_ALU_ADC: z80_adc(z80, value); break;
      case DT_ALU_SUB: z80_sub(z80, value); break;
      case DT_ALU_SBC: z80_sbc(z80, value); break;
      case DT_ALU_AND: z80_and(z80, value); break;
      case DT_ALU_XOR: z80_xor(z80, value); break;
      case DT_ALU_OR:  z80_or(z80, value); break;
      case DT_ALU_CP:  z80_compare(z80, value); break;
      default: break;
      }
      z80->pc += 1;

    } else if (3 == x && 1 == z && 0 == q) {
      dt_t dt = dt_rpaf[p];
      z80_trace(z80, mem, 1, "POP %s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC:
        c(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        b(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        break;
      case DT_RP_DE:
        e(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        d(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        break;
      case DT_RP_HL:
        l(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        h(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        break;
      case DT_RP_AF:
        f(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        a(z80) = mem_read(mem, z80->sp);
        z80->sp++;
        break;
      default:
        break;
      }
      z80->pc += 1;

    } else if (3 == x && 0 == z) {
      dt_t dt = dt_cc[y];
      z80_trace(z80, mem, 1, "RET %s", dt_text(dt));
      bool go = false;
      switch (dt) {
      case DT_CC_NZ: go = flag(z80).z  == 0; break;
      case DT_CC_Z:  go = flag(z80).z  == 1; break;
      case DT_CC_NC: go = flag(z80).c  == 0; break;
      case DT_CC_C:  go = flag(z80).c  == 1; break;
      case DT_CC_PO: go = flag(z80).pv == 0; break;
      case DT_CC_PE: go = flag(z80).pv == 1; break;
      case DT_CC_P:  go = flag(z80).s  == 0; break;
      case DT_CC_M:  go = flag(z80).s  == 1; break;
      default: break;
      }
      if (go) {
        z80->pc  = mem_read(mem, z80->sp);
        z80->sp++;
        z80->pc += mem_read(mem, z80->sp) << 8;
        z80->sp++;
      } else {
        z80->pc += 1;
      }

    } else if (3 == x && 1 == z && 1 == q && p == 0) {
      z80_trace(z80, mem, 1, "RET");
      z80->pc  = mem_read(mem, z80->sp);
      z80->sp++;
      z80->pc += mem_read(mem, z80->sp) << 8;
      z80->sp++;

    } else if (3 == x && 1 == z && 1 == q && 1 == p) {
      z80_trace(z80, mem, 1, "EXX");
      uint16_t value;
      value    = bc(z80);
      bc(z80)  = bc_(z80);
      bc_(z80) = value;
      value    = de(z80);
      de(z80)  = de_(z80);
      de_(z80) = value;
      value    = hl(z80);
      hl(z80)  = hl_(z80);
      hl_(z80) = value;
      z80->pc += 1;

    } else if (3 == x && 1 == z && 1 == q && 2 == p) {
      z80_trace(z80, mem, 1, "JP HL");
      z80->pc  = l(z80);
      z80->pc += h(z80) << 8;

    } else if (3 == x && 1 == z && 1 == q && 3 == p) {
      z80_trace(z80, mem, 1, "LD SP,HL");
      z80->sp = hl(z80);
      z80->pc += 1;

    } else if (3 == x && 2 == z) {
      uint16_t address = (mc[2] << 8) + mc[1];
      dt_t dt = dt_cc[y];
      z80_trace(z80, mem, 3, "JP %s,%04x", dt_text(dt), address);
      bool go = false;
      switch (dt) {
      case DT_CC_NZ: go = flag(z80).z  == 0; break;
      case DT_CC_Z:  go = flag(z80).z  == 1; break;
      case DT_CC_NC: go = flag(z80).c  == 0; break;
      case DT_CC_C:  go = flag(z80).c  == 1; break;
      case DT_CC_PO: go = flag(z80).pv == 0; break;
      case DT_CC_PE: go = flag(z80).pv == 1; break;
      case DT_CC_P:  go = flag(z80).s  == 0; break;
      case DT_CC_M:  go = flag(z80).s  == 1; break;
      default: break;
      }
      if (go) {
        z80->pc = address;
      } else {
        z80->pc += 3;
      }

    } else if (3 == x && 3 == z && 0 == y) {
      uint16_t address = (mc[2] << 8) + mc[1];
      z80_trace(z80, mem, 3, "JP %04x", address);
      z80->pc = address;

    } else if (3 == x && 3 == z && 2 == y) {
      uint8_t value = mc[1];
      z80_trace(z80, mem, 2, "OUT (%02x),A", value);
      io_write(value, a(z80), a(z80), mem);
      z80->pc += 2;

    } else if (3 == x && 3 == z && 3 == y) {
      uint8_t value = mc[1];
      z80_trace(z80, mem, 2, "IN A,(%02x)", value);
      a(z80) = io_read(value, a(z80));
      z80->pc += 2;

    } else if (3 == x && 3 == z && 4 == y) {
      z80_trace(z80, mem, 1, "EX (SP),HL");
      uint16_t value;
      value  = mem_read(mem, z80->sp);
      value += mem_read(mem, z80->sp + 1) << 8;
      mem_write(mem, z80->sp, l(z80));
      mem_write(mem, z80->sp + 1, h(z80));
      hl(z80) = value;
      z80->pc += 1;

    } else if (3 == x && 3 == z && 5 == y) {
      z80_trace(z80, mem, 1, "EX DE,HL");
      uint16_t value = hl(z80);
      hl(z80) = de(z80);
      de(z80) = value;
      z80->pc += 1;

    } else if (3 == x && 3 == z && 6 == y) {
      z80_trace(z80, mem, 1, "DI");
      z80->iff1 = 0;
      z80->iff2 = 0;
      z80->pc += 1;

    } else if (3 == x && 3 == z && 7 == y) {
      z80_trace(z80, mem, 1, "EI");
      z80->iff1 = 1;
      z80->iff2 = 1;
      z80->pc += 1;

    } else if (3 == x && 4 == z) {
      uint16_t address = (mc[2] << 8) + mc[1];
      dt_t dt = dt_cc[y];
      z80_trace(z80, mem, 3, "CALL %s,%04x", dt_text(dt), address);
      bool go = false;
      switch (dt) {
      case DT_CC_NZ: go = flag(z80).z  == 0; break;
      case DT_CC_Z:  go = flag(z80).z  == 1; break;
      case DT_CC_NC: go = flag(z80).c  == 0; break;
      case DT_CC_C:  go = flag(z80).c  == 1; break;
      case DT_CC_PO: go = flag(z80).pv == 0; break;
      case DT_CC_PE: go = flag(z80).pv == 1; break;
      case DT_CC_P:  go = flag(z80).s  == 0; break;
      case DT_CC_M:  go = flag(z80).s  == 1; break;
      default: break;
      }
      if (go) {
        z80->sp--;
        mem_write(mem, z80->sp, (z80->pc + 3) / 256);
        z80->sp--;
        mem_write(mem, z80->sp, (z80->pc + 3) % 256);
        z80->pc = address;
      } else {
        z80->pc += 3;
      }

    } else if (3 == x && 5 == z && 1 == q && 0 == p) {
      uint16_t address = (mc[2] << 8) + mc[1];
      z80_trace(z80, mem, 3, "CALL %04x", address);
      z80->sp--;
      mem_write(mem, z80->sp, (z80->pc + 3) / 256);
      z80->sp--;
      mem_write(mem, z80->sp, (z80->pc + 3) % 256);
      z80->pc = address;

    } else if (3 == x && 5 == z && 0 == q) {
      dt_t dt = dt_rpaf[p];
      z80_trace(z80, mem, 1, "PUSH %s", dt_text(dt));
      switch (dt) {
      case DT_RP_BC:
        z80->sp--;
        mem_write(mem, z80->sp, b(z80));
        z80->sp--;
        mem_write(mem, z80->sp, c(z80));
        break;
      case DT_RP_DE:
        z80->sp--;
        mem_write(mem, z80->sp, d(z80));
        z80->sp--;
        mem_write(mem, z80->sp, e(z80));
        break;
      case DT_RP_HL:
        z80->sp--;
        mem_write(mem, z80->sp, h(z80));
        z80->sp--;
        mem_write(mem, z80->sp, l(z80));
        break;
      case DT_RP_AF:
        z80->sp--;
        mem_write(mem, z80->sp, a(z80));
        z80->sp--;
        mem_write(mem, z80->sp, f(z80));
        break;
      default:
        break;
      }
      z80->pc += 1;

    } else if (3 == x && 6 == z) {
      dt_t dt = dt_alu[y];
      uint8_t value = mc[1];
      z80_trace(z80, mem, 2, "%s %02x", dt_text(dt), value);
      switch (dt) {
      case DT_ALU_ADD: z80_add(z80, value); break;
      case DT_ALU_ADC: z80_adc(z80, value); break;
      case DT_ALU_SUB: z80_sub(z80, value); break;
      case DT_ALU_SBC: z80_sbc(z80, value); break;
      case DT_ALU_AND: z80_and(z80, value); break;
      case DT_ALU_XOR: z80_xor(z80, value); break;
      case DT_ALU_OR:  z80_or(z80, value); break;
      case DT_ALU_CP:  z80_compare(z80, value); break;
      default: break;
      }
      z80->pc += 2;

    } else if (3 == x && 7 == z) {
      uint8_t zero_address = y * 8;
      z80_trace(z80, mem, 1, "RST %02x", zero_address);
      z80->sp--;
      mem_write(mem, z80->sp, (z80->pc + 1) / 256);
      z80->sp--;
      mem_write(mem, z80->sp, (z80->pc + 1) % 256);
      z80->pc = zero_address;
      panic("Unimplemented RST\n");

    } else {
      panic("Unhandled opcode: %02x [x=%d z=%d (y=%d | q=%d p=%d)]\n",
        mc[0], x, z, y, q, p);
      z80->pc += 1;
    }
  }
}



