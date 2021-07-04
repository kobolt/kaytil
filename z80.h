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
      };
      uint8_t a; /* Accumulator */
    };
    uint16_t af; /* Combined AF */
  };

  union {
    struct {
      uint8_t c; /* General Purpose C */
      uint8_t b; /* General Purpose B */
    };
    uint16_t bc; /* General Purpose BC */
  };

  union {
    struct {
      uint8_t e; /* General Purpose E */
      uint8_t d; /* General Purpose D */
    };
    uint16_t de; /* General Purpose DE */
  };

  union {
    struct {
      uint8_t l; /* General Purpose L */
      uint8_t h; /* General Purpose H */
    };
    uint16_t hl; /* General Purpose HL */
  };

  union {
    struct {
      uint8_t ixl; /* IX Low Part */
      uint8_t ixh; /* IX High Part */
    };
    uint16_t ix; /* Index Register X */
  };

  union {
    struct {
      uint8_t iyl; /* IY Low Part */
      uint8_t iyh; /* IY High Part */
    };
    uint16_t iy; /* Index Register Y */
  };

  union {
    struct {
      uint8_t f_; /* Alternate F' */
      uint8_t a_; /* Alternate A' */
    };
    uint16_t af_; /* Alternate AF' */
  };

  union {
    struct {
      uint8_t c_; /* Alternate C' */
      uint8_t b_; /* Alternate B' */
    };
    uint16_t bc_; /* Alternate BC' */
  };

  union {
    struct {
      uint8_t e_; /* Alternate E' */
      uint8_t d_; /* Alternate D' */
    };
    uint16_t de_; /* Alternate DE' */
  };

  union {
    struct {
      uint8_t l_; /* Alternate L' */
      uint8_t h_; /* Alternate H' */
    };
    uint16_t hl_; /* Alternate HL' */
  };

} z80_t;

void z80_init(z80_t *z80);
void z80_execute(z80_t *z80, mem_t *mem);

void z80_trace_init(void);
void z80_trace_dump(FILE *fh);
void z80_dump(FILE *fh, z80_t *z80, mem_t *mem);

#endif /* _Z80_H */
