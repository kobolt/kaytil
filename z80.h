#ifndef _Z80_H
#define _Z80_H

#include <stdint.h>
#include <stdbool.h>
#include "mem.h"

typedef struct z80_s {
  uint16_t pc; /* Program Counter */
  uint16_t sp; /* Stack Pointer */
  uint8_t i;   /* Interrupt Page Address */
  uint8_t r;   /* Memory Refresh */
  bool iff1;   /* Interrupt Flip-Flop #1 */
  bool iff2;   /* Interrupt Flip-Flop #2 */

  union {
    struct {
      union {
        struct {
          uint8_t c  : 1; /* Carry */
          uint8_t n  : 1; /* Add/Subtract */
          uint8_t pv : 1; /* Parity/Overflow */
          uint8_t x1 : 1; /* Unused #1 */
          uint8_t h  : 1; /* Half Carry */
          uint8_t x2 : 1; /* Unused #2 */
          uint8_t z  : 1; /* Zero */
          uint8_t s  : 1; /* Sign */
        } flag;
        uint8_t f; /* Flag */
      } u_f;
      uint8_t a; /* Accumulator */
    } s_af;
    uint16_t af; /* Combined AF */
  } u_af;

  union {
    struct {
      uint8_t c; /* General Purpose C */
      uint8_t b; /* General Purpose B */
    } s_bc;
    uint16_t bc; /* General Purpose BC */
  } u_bc;

  union {
    struct {
      uint8_t e; /* General Purpose E */
      uint8_t d; /* General Purpose D */
    } s_de;
    uint16_t de; /* General Purpose DE */
  } u_de;

  union {
    struct {
      uint8_t l; /* General Purpose L */
      uint8_t h; /* General Purpose H */
    } s_hl;
    uint16_t hl; /* General Purpose HL */
  } u_hl;

  union {
    struct {
      uint8_t ixl; /* IX Low Part */
      uint8_t ixh; /* IX High Part */
    } s_ix;
    uint16_t ix; /* Index Register X */
  } u_ix;

  union {
    struct {
      uint8_t iyl; /* IY Low Part */
      uint8_t iyh; /* IY High Part */
    } s_iy;
    uint16_t iy; /* Index Register Y */
  } u_iy;

  union {
    struct {
      uint8_t f_; /* Alternate F' */
      uint8_t a_; /* Alternate A' */
    } s_af_;
    uint16_t af_; /* Alternate AF' */
  } u_af_;

  union {
    struct {
      uint8_t c_; /* Alternate C' */
      uint8_t b_; /* Alternate B' */
    } s_bc_;
    uint16_t bc_; /* Alternate BC' */
  } u_bc_;

  union {
    struct {
      uint8_t e_; /* Alternate E' */
      uint8_t d_; /* Alternate D' */
    } s_de_;
    uint16_t de_; /* Alternate DE' */
  } u_de_;

  union {
    struct {
      uint8_t l_; /* Alternate L' */
      uint8_t h_; /* Alternate H' */
    } s_hl_;
    uint16_t hl_; /* Alternate HL' */
  } u_hl_;

} z80_t;

void z80_init(z80_t *z80);
void z80_execute(z80_t *z80, mem_t *mem);

void z80_trace_init(void);
void z80_trace_dump(FILE *fh);
void z80_dump(FILE *fh, z80_t *z80, mem_t *mem);

#endif /* _Z80_H */
