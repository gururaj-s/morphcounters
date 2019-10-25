/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: bpc.h
 * Description: 
 * Author: Saileshwar
 * Created: Tue Oct  3 23:48:26 2017 (-0400)
 * Last-Updated: Wed Oct 25 21:05:49 2017 (-0400)
 *     Update #: 11
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */

#ifndef __BPC_H__

#include "global_types.h"
#include "compression.h"

void bpc_transform(uns64* input_entries, uns64 num_entries, uns64 entry_bits, uns64 dbx_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64 dbp_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64* num_symbols, uns64* symbol_bits );

uns64 bpc_symbols_comp(uns64 dbx_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64 dbp_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64 num_symbols, uns64 symbol_bits, uns64 entry_bits  );

uns64 bpc_symbol_lookup (uns64 dbx_symbol, uns64 dbp_symbol, uns64 symbol_bits);

int consec_two_1s(uns64 symbol, uns64 bits);

int single_1(uns64 symbol, uns64 bits);


#define __BPC_H__
#endif


/* bpc.h ends here */
