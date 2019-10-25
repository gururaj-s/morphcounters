/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: compression.h
 * Description: 
 * Author: Saileshwar
 * Created: Mon Sep 18 17:17:37 2017 (-0400)
 * Last-Updated: Fri Mar  9 22:30:21 2018 (-0500)
 *     Update #: 54
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */


#ifndef __COMPRESSION_H__

#include "global_types.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define MAX_32_BITMULTIPLE_SYMBOLLEN 16

#define MAC_BITLEN (56) //in bits

#define MAXINPUTLINESIZE 16384

uns64 sum_writes(uns64* line_data, int num_lines);
double skew_writes(uns64* line_data, int num_lines);
double skew_growth(uns64* line_data, int num_lines);
uns64 exact_counter_bits(uns64* line_data, int num_lines);
uns64 num_cont_nonzeros(uns64* line_data, int num_lines);
uns64 num_val(uns64* line_data, int num_lines, uns64 val);
uns64 num_nonzeros(uns64* line_data, int num_lines);
uns64 zero_val_compression(uns64* line_data, int num_lines); //I
uns64 zero_runlength_val(uns64* line_data, int num_lines); //II
uns64 zero_one_compression(uns64* line_data, int num_lines);//III
uns64 zero_one_freq_val_compression(uns64* line_data, int num_lines);//III
uns64 zero_one_max_val_compression(uns64* line_data, int num_lines);//IV
uns64 maxFreqVal(uns64* arr, int num_elements);
uns64 bdi_3bit_maxval(uns64* line_data, int num_lines);
uns64 bdi_5bit_maxval(uns64* line_data, int num_lines);
uns64 bdi_5bit_maxval_2base(uns64* line_data, int num_lines);
uns64 bdi_7bit_maxval_dedicated(uns64* line_data, int num_lines);
uns64 bdi_3bit_maxFreqVal(uns64* line_data, int num_lines);
uns64 zlib_comp(uns64* line_data, int num_lines);
uns64 zlib_long_comp(uns64* line_data, int num_lines);
uns64 best(uns64* line_data, int num_lines);
uns64 best_2base(uns64* line_data, int num_lines);

//HIGHARY COMPRESSION Functions
uns64 zero_val_comp(uns64* line_data, int num_lines, int ctr_bits);
uns64 zero_runlength_comp(uns64* line_data, int num_lines, int ctr_bits);
uns64 ptrs_non_zero_comp(uns64* line_data, int num_lines, int ctr_bits);
uns64 bpc_comp_new(uns64* line_data, int num_lines, int ctr_bits);
uns64 bdi_comp_new(uns64* line_data, int num_lines, int ctr_bits, int add_bits);

uns64 best_comp(uns64* line_data, int num_lines, int ctr_bits, int add_bits);

//FINAL HIGHARY COMPRESSION Functions
uns64 try_comp(uns64* line_data, int num_lines, int ctr_bits, int add_bits, int inc_line_id, int comp_mode, int num_major_ctr_per_cl);
uns64 ada_dyn0_comp(uns64* line_data, int num_lines, int ctr_bits, int add_bits, int num_major_ctrs_per_cl);
uns64 uncomp(uns64* line_data, int num_lines, int ctr_bits, int inc_line_id);
uns64 try_rebase(uns64* major_ctr, uns64* line_data, int num_lines, uns64 max_major_ctr_val);


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                    \
  (byte & 0x80 ? '1' : '0'),                    \
    (byte & 0x40 ? '1' : '0'),                  \
    (byte & 0x20 ? '1' : '0'),                  \
    (byte & 0x10 ? '1' : '0'),                  \
    (byte & 0x08 ? '1' : '0'),                  \
    (byte & 0x04 ? '1' : '0'),                  \
    (byte & 0x02 ? '1' : '0'),                  \
    (byte & 0x01 ? '1' : '0')

uns64 bpc_comp_sanjay(uns64* chunk_data, int num_chunks, int chunk_bitlen);
// 16 of uns32 numbers
// 16 chunks
// 32 bitlength

#define __COMPRESSION_H__
#endif


/* compression.h ends here */
