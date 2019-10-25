/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: test_filereader.c
 * Description: 
 * Author: Saileshwar
 * Created: Mon Oct  9 18:03:25 2017 (-0400)
 * Last-Updated: Mon Oct  9 22:07:33 2017 (-0400)
 *     Update #: 26
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */

#include<stdio.h>
#include "filereader.h"

//char* FILE_PATH = "../input/SPEC2006_v1/mcf.gz";
//char* FILE_PATH = "../input/SPEC2006_v1/povray.gz";
char* FILE_PATH = "../input/GAP_v1/bc_1_0.raw.usim.gz";

int NUMCORES = 4;
char* return_fgets1 = NULL;
int sim_useL3 = 1;
unsigned long long int *nonmemops;
char *opertype;
long long int *addr;
long long int *instrpc;
char newstr[MAXTRACELINESIZE];
char* traceline;

int test_mode = 0;
/***************************/

char** read_buffer;
char scratch_buffer[MAXTRACELINESIZE];

int* read_buf_index;
int* valid_buf_length;

int expt_done;
int iter = 0;
int main(int argc, char * argv[]){
  FILE **tif=NULL;  /* The handles to the trace input files. */
  tif = (FILE **)malloc(sizeof(gzFile *)*NUMCORES);
  nonmemops = (unsigned long long int*)malloc(sizeof(int)*NUMCORES);
  opertype = (char *)malloc(sizeof(char)*NUMCORES);
  addr = (long long int *)malloc(sizeof(long long int)*NUMCORES);
  instrpc = (long long int *)malloc(sizeof(long long int)*NUMCORES);

  read_buffer =  (char **)malloc(sizeof(char*)*NUMCORES);
  read_buf_index = (int*) malloc(sizeof(int)*NUMCORES);
  valid_buf_length = (int*) malloc(sizeof(int)*NUMCORES);  
  for(int i=0; i< NUMCORES;i++){
    read_buffer[i] = (char*)malloc(sizeof(char)*BUFFSIZE);
    read_buf_index[i] = -1;
    valid_buf_length[i] = -1;
  }

  if(argc > 1)
    test_mode = atoi(argv[1]);
  
  printf("SimuseL3 is %d\n",sim_useL3);
  
  for (int numc=0; numc < NUMCORES; numc++) {
    tif[numc] = gzopen(FILE_PATH, "r");

    if (!tif[numc]) {
      printf("Trying to open trace file : %s",FILE_PATH);
      printf("Missing input trace file %d.  Quitting. \n",numc);
      return -5;
    }
  }
  
  expt_done = 0;
  
  while (!expt_done) {
    iter++;
    if(iter == 10000000)
      expt_done = 1;

    for(int numc=0; numc<NUMCORES; numc++) {

      if(test_mode == 0){
        return_fgets1 = NULL;
        return_fgets1 = gzgets(tif[numc],newstr,MAXTRACELINESIZE);

        if(!return_fgets1) {
          gzrewind(tif[numc]);
          return_fgets1 = gzgets(tif[numc],newstr,MAXTRACELINESIZE) ;       
        }
        traceline = newstr;
      }
      else {
        traceline =  get_line_from_file( tif, numc, read_buffer, read_buf_index,  valid_buf_length, scratch_buffer);
      }

      //      printf("\n%s",traceline);
      if(sim_useL3 == 1){ //Reading L2 Miss Stream
        if (sscanf(traceline,"%llu %c",&nonmemops[numc],&opertype[numc]) > 0) {
          if (opertype[numc] == 'R') {
            if (sscanf(traceline,"%llu %c %Lx %Lx",&nonmemops[numc],&opertype[numc],&addr[numc],&instrpc[numc]) < 1) {
              printf("6. Panic.  Poor trace format.\n");
              return -4;
            }
          }
          else {
            if (opertype[numc] == 'W') {
              if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
                printf("7. Panic.  Poor trace format.\n");
                return -3;
              }
            }
            else {
              printf("8. Panic.  Poor trace format.\n");
              return -2;
            }
          }
        }
        else {
          printf("9. Panic.  Poor trace format.\n");
          return -1;
        }
      } else{ //Reading L3 Miss Stream
        unsigned long long int temp1,temp2,temp3;
        if ((sscanf(traceline,"%llu %llu %llu %llu %c %Lx %Lx",&nonmemops[numc],&temp1, &temp2, &temp3, &opertype[numc],&addr[numc],&instrpc[numc]) > 0)) {
          ASSERTM( (opertype[numc] == 'W') || (opertype[numc] == 'R'), "Trace contains some operand other than R or W");              
        }
        else {
          printf("Panic. Poor trace format reading L3 Miss Stream.\n");
          return -1;
        }
      }

      printf("Numc: %d, Non_mem_ops %llu, OperType %c, Addr Lx \n",numc,nonmemops[numc],opertype[numc],addr[numc]);
    }
  }
}
/* test_filereader.c ends here */
