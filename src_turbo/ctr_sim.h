#ifndef __CTR_SIM_H__
#define  __CTR_SIM_H__

#include "mcache.h"
#include "memOrg.h"
#include "stats.h"
#include "global_types.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

//#define HIST_NZ // For Logging Histogram of # Nonzero Counters in CL, on Overflows i.e. x->(0-64 : Number of non-zero counters in CL), y->(fracn of overflows)

#define NUM_COMP_SCHEMES 4
#define NUM_CTR_BITLENS 3

typedef struct ctr_cl{
  //identifier
  uns64 lineaddr;
  int mtree_level;
  uns64 entry_num;

  //State
  //Only for the SPLIT128_FULL_DUAL
  uns64 super_ctr_val;

  uns64* major_ctr_val;
  uns64** minor_ctr_val;

  //Overflow Stats
  uns64* num_overflows; //Equal to number of major ctrs
  uns64* last_overflow_cycle; //Equal to number of major ctrs

  uns64 num_nonzero_numer; //Number of non-zero counters out of total ctrs in cacheline at overflow.
  uns64 num_nonzero_denom;

  uns64 num_smalldynrange_numer; // <7 DYNRANGE
  uns64 num_smalldynrange_denom; //  <7 DYNRANGE
  uns64 num_smalldynrange_numer_no_zero; // NOZERO
  uns64 num_smalldynrange_denom_no_zero; // NOZERO
  
  uns64* num_writes; //Equal to number of major ctrs
  uns64* num_reads; //Equal to number of major ctrs

  uns64* data_writes; //Equal to number of major ctrs
  uns64* data_reads; //Equal to number of major ctrs
  
  uns64 increments;
  uns64 bitlen;

  int* comp_mode ; //Last compression mode used.

} ctr_cl;


typedef struct ctr_type{
  int mtree_level;
  int num_major_ctr_per_cl;
  int num_minor_ctr_per_major_ctr;
  int design; //Type of counter

  //Only for the SPLIT128_FULL_DUAL
  uns64 super_ctr_maxval;
  double super_ctr_bitslen;


  double major_ctr_bitslen;
  uns64 major_ctr_maxval;
  uns64 minor_ctr_maxval;
  double minor_ctr_bitslen;

  double* minor_ctr_bitslen_arr;
  int num_minor_ctr_bitslen_arr;

  //Log compression chemes
  uns64 log_comp[NUM_CTR_BITLENS][NUM_COMP_SCHEMES];
  uns64 log_num_overflows;

  //Log for activations of compression schemes in Mode 6
  uns64 log_comp_calls[NUM_CTR_BITLENS];
  uns64 log_num_check_overflows;
  
  uns64 num_ctr_cls;
  int compression_enabled;

#ifdef HIST_NZ
  uns64* nz_ctr_buckets;
  uns64 nz_ctr_tot;
#endif
} ctr_type;


//Function to initialize the ctr_types
void init_ctr_types(memOrg_t* mem_org, ctr_type* ctr_types, int mtree_level);

//Function to initialize the ctr_cachelines
void init_ctr_cls(memOrg_t* mem_org, ctr_cl* ctr_cls, ctr_type* ctr_types, int mtree_level);

//Function to update the ctr_cachelines on dirty write to counter
void update_ctr_cls(memOrg_t* mem_org, ctr_cl* ctr_cls, ctr_type* ctr_types, ctr_mtree_entry ctr, overflows_stat* ctr_overflows_levelwise_ptr,  overflows_stat* ctr_overflows_levelwise_inst_ptr );

//Function to analyze overflows and print
void print_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, unsigned long long int count_i_total);

//Function to log overflows
void log_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, FILE *Policyfile);

//Function to reset the stats
void reset_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types);

//Function to reset the stats - num_overflows only
void reset_tot_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types);

//Function to ensure all dirty data/ctrs have their ctr_sim updated
void fin_stat_overflows(MCache* L3, MCache* MET, memOrg_t* mem_org,ctr_cl** ctr_cls, ctr_type** ctr_types, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_inst);


//Function to update the a generic ctr_cacheline from counters or mtree
void update_ctr_cls_wrapper(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, ctr_mtree_entry ctr, int ctr_mtree_level, overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst);

//Function to update the ctr_cachelines on memory accesses - wrapper
void update_ctr_cls_rw_wrapper(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, ADDR ctr_paddr, int ctr_mtree_level, int r_w);

//Function to update the ctr_cachelines on memory accesses
void update_ctr_cls_rw(memOrg_t* mem_org, ctr_cl* ctr_cls, ctr_type* ctr_types, ADDR ctr_paddr, int r_w);

//Function to insert required memory accesses on overflows
void insert_overflow_mem_accesses(ADDR ctr_paddr, memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise_ptr, overflows_stat* ctr_overflows_levelwise_inst_ptr,  int major_ctr_id, int num_major_ctr_per_cl);

//Function to check whether a minor_ctr_id increment leads to a overflow
int check_overflow(ctr_cl* curr_ctr_cl , uns64 major_ctr_id, uns64 minor_ctr_id, ctr_type* ctr_types);

//Get the bitlen for a particular counter physical address
uns64 get_ctr_cl_bitlen(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, ADDR ctr_paddr, int ctr_mtree_level);

void log2_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr, FILE* Policyfile);

void log_stat_ctr_vals(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr, FILE* Policyfile, int mtree_level);

void log_comp_schemes(ctr_type* ctr_types, ctr_cl* curr_ctr_cl, int major_ctr_id);

void print_logs_comp_schemes(ctr_type** ctr_types, memOrg_t* mem_org);

uns64 try_comp_wrapper(ctr_cl* curr_ctr_cl, uns64 major_ctr_id, uns64 minor_ctr_id, ctr_type* ctr_types, int add_bits, int mode);


void track_zero_ctr_stats(ctr_type* ctr_types, ctr_cl* ctr_cls);
void track_dynrange_ctr_stats(ctr_type* ctr_types, ctr_cl* ctr_cls, int major_ctr_id);

void log_stat_overflows_HistNz(memOrg_t* mem_org,ctr_type** ctr_types_arr, FILE* Policyfile);

#endif

