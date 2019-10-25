#ifndef __MEMSYS_CONT_H__
#define __MEMSYS_CONT_H__

#include "global_types.h"
#include "mcache.h"
#include "stats.h"
#include "memOrg.h"
#include "processor.h"
#include "memory_controller.h"
#include "ctr_sim.h"

//Functions added by Gururaj - to make code modular

//Used for accessing any cache - takes care of statistics
Flag cache_access(MCache** cache, unsigned long long is_private,  ADDR addr, Flag dirty, int numc, cache_stats* stats, maccess_type type, memOrg_t* mem_org);

//Used for installing to any cache -takes care of evict statistics. Returns evicted entry
MCache_Entry cache_install(MCache** cache, unsigned long long is_private, ADDR addr, Flag dirty, int numc, cache_stats* stats, maccess_type type,  memOrg_t* mem_org);

//Used for a dirty writeback from LLC
void handle_dirty_wb_LLC (long long int wb_addr, MCache_Entry evicted_entry, int mem_prio, memOrg_t* mem_org, MCache** METCache, MCache** L3Cache, MCache** MACCache,  int tid,
                          cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, cache_stats* MAC_Cache_Stats,   overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst, long long int* instrpc);

//Used for reading a Metadata cacheline - from cache/memory, takes care of mtree walk as well; Updates Stats
void read_ctr_mtree(MCache** METCache, MCache** L3Cache, MCache** MACCache, ctr_mtree_entry met_entry, int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats,cache_stats* MAC_Cache_Stats,   overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst, Flag is_critical,
                    memOrg_t* mem_org,  struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio, int policy_install_metcache);


//Used for writing a Metadata cacheline to cache/memory, 
void write_ctr_mtree(MCache** METCache, MCache** L3Cache, MCache** MACCache, ctr_mtree_entry met_entry, int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, cache_stats* MAC_Cache_Stats,   overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst,
                     memOrg_t* mem_org, struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio);

//Used for dealing with metadata writes to LLC (on MetCache Write Miss & MetCache Dirty Writeback)
void write_Met_LLC (MCache** METCache, MCache** L3Cache, MCache**  MACCache, long long int met_paddr, Flag is_partial,
                    int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats,cache_stats* MAC_Cache_Stats,   overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst, memOrg_t* mem_org, long long int* instrpc, int mem_prio);

//Used for reading or writing MACs - either to/from cache or memory. Updates stats
void write_mac(MCache** mac_cache, long long int mac_paddr,int tid, cache_stats* stats,
               memOrg_t* mem_org, struct robstructure* ROB, long long int CYCLE_VAL,  long long int* instrpc, int mem_prio);

void read_mac(MCache** mac_cache, long long int mac_paddr, int tid, cache_stats* stats, Flag is_critical,
              memOrg_t* mem_org,  struct robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio);

// insert reads into the read queue as per Baseline Memory System Design (normal or chipkill)
void insert_read_memsys(long long int physical_address, long long int arrival_cycle, int thread_id, int instruction_id, long long int instruction_pc, int mem_prio, int is_critical, maccess_type type);

// insert writes into write queue as per Baseline Memory System Design (normal or chipkill)
void insert_write_memsys(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, int mem_prio, int is_critical, maccess_type type);

//Cache stats management
void cache_stats_update_access(cache_stats* stats, Flag outcome, Flag dirty);
void cache_stats_update_install(cache_stats* stats);

//Compressed Met Cache 
MCache_Entry cache_comp_install(MCache** cache, unsigned long long is_private, ADDR addr, Flag dirty, int tid, cache_stats* stats, maccess_type type, memOrg_t* mem_org);

#endif
