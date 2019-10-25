/*
#include <execinfo.h>
#include <stdio.h>

void print_backtrace() {
  void* callstack[128];
  int i, frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
  for (i = 0; i < frames; ++i) {
    printf("%s\n", strs[i]);
  }
  free(strs);
}

*/

#ifndef __DEBUG_H__

#include <stdio.h>
#include <signal.h>
#include <execinfo.h>

//void bt_sighandler(int sig, struct sigcontext ctx) {

void print_backtrace(){

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;
  /*
  if (sig == SIGSEGV)
    printf("Got signal %d, faulty address is %p, "
           "from %p\n", sig, ctx.cr2, ctx.eip);
  else
    printf("Got signal %d\n", sig);
  */

  trace_size = backtrace(trace, 16);
  /* overwrite sigaction with caller's address */
//  trace[1] = (void *)ctx.eip;
  messages = backtrace_symbols(trace, trace_size);
  /* skip first stack frame (points here) */
  printf("[bt] Execution path:\n");
  for (i=1; i<trace_size; ++i)
    {
      printf("[bt] #%d %s\n", i, messages[i]);

      char syscom[256];
      sprintf(syscom,"addr2line %p -e ../bin/usimm", trace[i]); //last parameter is the name of this app
      system(syscom);
    }

  //  exit(0);
}

#define __DEBUG_H__
#endif
