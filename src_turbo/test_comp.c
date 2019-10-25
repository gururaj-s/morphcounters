/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: main.c
 * Description: 
 * Author: Saileshwar
 * Created: Thu Sep 14 15:45:19 2017 (-0400)
 * Last-Updated: Thu Oct 26 14:45:42 2017 (-0400)
 *     Update #: 111
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */
#include "global_types.h"
#include "compression.h"
#include<stdio.h>
#include<stdlib.h>
#include<zlib.h>
#include "bpc.h"

#define NUM_LINES 128

char buf[MAXINPUTLINESIZE];

uns64 addr;

FILE *ifile = NULL;
char* input_filename =NULL;
int LINEWISE = 0;
int PAGEWISE = 0;
int DEBUG = 0;
int TEST = 0;

int MINOR_CTR_BITLEN = 7;
int MAJOR_CTR_BITLEN = 50;
int COMP_CTR_MODE = 1;

//uns64 line_data[128] = {0,0,0,0,123,0,0,0,0,0,0,0,35,123,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,45,0,0,0,32,0,0,0,0,0,0,123,0,0,0,0,0,0,0,35,123,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,45,0,0,0,32,0,0,0,0,0,0,123,0,0,0,0,0,0,0,35,123,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,45,0,0,0,32,0,0,23,45,0,0,0,32,0,0};
//uns64 line_data[128] =  { 2,   4,   9,   0,   5,   4,   15,  8,   20,  10,  8,   7,   11,  12,  8,   2,   4,   1,   2,   11,  3,   1,   11,  6,   0,   11,  7,   5,   4 , 20,  5,   4,   9,   5,   10,  12,  7,   2,   1,   16,  12,  5,   12,  5,   0,   16,  21,  6,   8,   15,  14,  0,   2,   0,   6,   9,   7,   7,   3,   5,   6 ,  6,   8,   11,  20,  11,  5,   3,   6,   9,   0,   18,  6,   0,   9,   15,  15,  4,   0,   14,  9,   7,   20,  6,   9,   7,   3,   4,   6,   10,  5,   3,   9, 10,  3,   16,  0,   8,   19,  1,   4,   8,   0,   6,   10,  5,   14,  9,   3,   7,   6,   7,   0,   6,   10,  3,   6,   0,   0,   9,   25,  10,  8,   15,  9 , 8,   5,   5 } ;

uns64 line_data[128] = {1 ,0 ,1 ,0 ,1 ,0 ,0 ,3 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,3 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,4 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,2 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,5 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,3 ,0 ,0 ,2 ,1 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,3 ,0 ,0 ,1 };

int main(int argc, char **argv){
  for(int i=0; i< NUM_LINES;i++){
    printf("%d , ",line_data[i]);
  }

  printf("\nBest: %d\n", best_comp(line_data, NUM_LINES, 7));
  printf("Zero Val %d\n", zero_val_comp(line_data, NUM_LINES, 7));
  printf("Zero Runlen %d\n", zero_runlength_comp(line_data, NUM_LINES, 7));
  printf("Ptr Val %d\n", ptrs_non_zero_comp(line_data, NUM_LINES, 7));
  printf("BPC Val %d\n", bpc_comp_new(line_data, NUM_LINES, 7));
  
}
/* main.c ends here */
