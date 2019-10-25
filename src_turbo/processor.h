#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__
#include "stats.h"

struct robstructure
{
  int head;
  int tail;
  int inflight;
  long long int * comptime;
  long long int * mem_address;
  int * optype;
  long long int * instrpc;
  int * critical_mem_ops;
  read_lat_rob_stat * read_lat_stat;
  int tracedone;
};

#endif //__PROCESSOR_H__

