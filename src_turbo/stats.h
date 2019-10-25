#ifndef STATS_H
#define STATS_H

#include<stdlib.h>
#include "global_types.h"
#include <stdio.h>
#include <assert.h>
#include "memOrg.h"
#include "mcache.h"


typedef struct mem_stats_t {
  unsigned long long int data_w;
  unsigned long long int mac_w;
  unsigned long long int ctr_w;
  unsigned long long int* met_w;
  
  unsigned long long int data_r;
  unsigned long long int mac_r;
  unsigned long long int ctr_r;
  unsigned long long int* met_r; 

  unsigned long long int twin; 

} mem_stats_t;

  
typedef struct cache_stats {

  unsigned long long int *write_hits;
  unsigned long long int *write_misses;
  unsigned long long int *read_hits;
  unsigned long long int *read_misses;
  unsigned long long int *dirty_evicts;
  
  unsigned long long int *write_hits_inst;
  unsigned long long int *write_misses_inst;
  unsigned long long int *read_hits_inst;
  unsigned long long int *read_misses_inst;
  unsigned long long int *dirty_evicts_inst;
 
} cache_stats;

typedef struct cache_sim_stats {
  double read_hitrt ; 
  double write_hitrt;
  double HITRT      ;
  double rmpki      ;
  double wmpki      ;
  double d_e_mpki   ; 

  //Occupancy metrics
  double data_perc  ;
  double ctr_perc  ;
  double mtree_perc  ;
  double* mtree_perc_det  ;
  double valid_perc;

} cache_sim_stats;

typedef struct overflows_stat {
  unsigned long long int overflows;
  unsigned long long int child_accesses;
} overflows_stat;

typedef enum memtype_t { DATA, MAC, MET, TWIN} memtype_t;

typedef struct maccess_type {
  memtype_t memtype;
  int mtree_level;
} maccess_type;


typedef struct read_lat_rob_stat{
  uns64 start_cycle;
  uns64 read_issued_cycle;
  uns64 read_completed_cycle;
  uns64 end_cycle;
  uns64 latest_cycle;

  uns64 twin_duration;
  uns64 mac_duration;
  uns64* met_duration;

  uns64 num_twins;
  uns64 num_macs;
  uns64* num_mets; 
} read_lat_rob_stat;

typedef struct read_lat_sim_stat{
  uns64 num_reads;
  uns64 num_macs;
  uns64* num_mets;
  uns64 num_ctrs;
  uns64 num_mtree_tot; 
  uns64 num_twins;
  
  uns64 read_latency;
  uns64 queue_latency;
  uns64 act_read_latency;
  uns64 mac_latency; 
  uns64* met_latency;
  uns64 ctr_latency;
  uns64 mtree_tot_latency;
  uns64 twin_latency; 

  
} read_lat_sim_stat;


//Function that initializes the cache stats
void init_cache_stats(cache_stats* stats, int len_array);


//Functions to calc stats

double calc_rt(unsigned long long int num, unsigned long long int denom );
double calc_mpki(unsigned long long int num, unsigned long long int denom );

  void calc_cache_stats (cache_stats* stats, Flag is_private,  
                         cache_sim_stats* sim_stats, cache_sim_stats*  sim_stats_inst,
                         unsigned long long int count_i_total, unsigned long long int count_i_inst_total);

  void calc_cache_occ (MCache** L3Cache, cache_sim_stats* sim_stats);
  void output_cache_occ (char*  cache_t, cache_sim_stats* sim_stats);

  void output_cache_stats (char* cache_t, cache_stats* stats, cache_sim_stats*  sim_stats, int is_L3);
  void print_cache_stats (FILE* Policyfile,  cache_sim_stats* sim_stats, cache_sim_stats*  sim_stats_inst, int is_L3);  

  void update_met_stats(cache_stats* MET_tot_Cache_Stats, cache_stats** MET_Cache_Stats, memOrg_t* mem_org);


  //Stats for Read Latency Breakdown
  void init_read_lat_rob_stat(read_lat_rob_stat* stat, memOrg_t* mem_org);
  void init_read_lat_sim_stat(read_lat_sim_stat* stat, memOrg_t* mem_org);
  void reset_read_lat_rob_stat(read_lat_rob_stat* stat, memOrg_t* mem_org);
  void output_read_lat_sim_stat(read_lat_sim_stat* sim_stat,  memOrg_t* mem_org);
  void print_read_lat_sim_stat(FILE* Policyfile, read_lat_sim_stat* sim_stat,  memOrg_t* mem_org);

  void lat_stat_end(read_lat_sim_stat* sim_stat, read_lat_rob_stat* rob_stat, memOrg_t* mem_org);
void lat_stat_start(read_lat_rob_stat* rob_stat, long long int cycle);
void lat_stat_process(read_lat_sim_stat* sim_stat, read_lat_rob_stat* rob_stat, maccess_type type, long long int curr_cycle, long long int final_cycle, int num_critical, memOrg_t* mem_org);
void lat_stat_end(read_lat_sim_stat* sim_stat, read_lat_rob_stat* rob_stat, memOrg_t* mem_org);

mem_stats_t* init_mem_stats (int num_mtree);
void zero_mem_stats (int num_mtree, mem_stats_t* mem_stats);
void update_mem_stats(mem_stats_t* mem_stats, maccess_type type, int r_w, ADDR memaccess_paddr, int thread_id);
void print_mem_stats(FILE* Policyfile, mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org);
void output_mem_stats(mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org);
double return_data_mpki(mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org);
double return_non_overflow_mpki(mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org);

uns64 calc_fast_cycles(mem_stats_t* mem_stats, uns64  count_i_total,memOrg_t* mem_org, int exc_overflow_access);

void log_ctr_overflow_vs_inst( FILE* fileh, uns64 count_500mn_fast_slow, memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_inst);

void print_stat_overflow_newfinal(memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_warmup, uns64 total_inst, uns64 data_mpki, uns64 non_overflow_mpki );
void timestamp_cumuoverflows_warmup(memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_warmup);
#endif // STATS_H
