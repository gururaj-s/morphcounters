/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: filereader.h
 * Description: 
 * Author: Saileshwar
 * Created: Sun Oct  8 17:18:04 2017 (-0400)
 * Last-Updated: Mon Oct 30 17:54:33 2017 (-0400)
 *     Update #: 24
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */

#ifndef FILEREADER_H
#define FILEREADER_H

#include"/home/gattaca4/gururaj/LOCAL_LIB/zlib-1.2.11_install/include/zlib.h" //added to support GAP
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "global_types.h"
#include "params.h"
#include <string.h>



//#define BUFFSIZE 4097
#define BUFFSIZE (2*1024*1024+1)
#define MAXTRACELINESIZE 256

typedef struct trace_line_struct {
  unsigned long long int nonmemops;
  char read_write;
  long long int addr;
} trace_line_struct ;


int write_trace_line_struct(gzFile filehandle, unsigned long long int line_nonmemops, char line_read_write, long long int trace_addr);
trace_line_struct read_trace_line_1rec (FILE* filehandle);

char* get_line_from_file(FILE** tif, int numc, char** read_buffer, int* read_buf_index,  int* valid_buf_length, char* scratch_buffer);



#endif

/* filereader.h ends here */
