#ifndef A_OUT_H
#define A_OUT_H

#include "types.h"

struct header {
  u32      a_magic;	    /* Use macros N_MAGIC, etc for access */
  u32      a_tsize;    	/* size of text, in bytes */
  u32      a_dsize;	   	/* size of data, in bytes */
  u32      a_bsize;	   	/* size of uninitialized data area for file, in bytes */
  u32      a_syms;	    	/* size of symbol table data in file, in bytes */
  u32      a_entry;		/* start address */
  u32      a_trsize;		/* size of relocation info for text, in bytes */
  u32      a_drsize;		/* size of relocation info for data, in bytes */
}; 

#define NMAGIC 0x6400CC

#endif
