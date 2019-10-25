#ifndef MCACHE_H
#define MCACHE_H

#include "global_types.h"
#include "memOrg.h"
#include "params.h"

///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//Added by Prashant
extern unsigned int num_cores;
///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//Maximum compression ratio
#define MAX_COMPR_RATIO (16)


typedef enum MCache_ReplPolicy_Enum {
    REPL_LRU=0,
    REPL_RND=1,
    REPL_SRRIP=2, 
    REPL_DRRIP=3, 
    REPL_FIFO=4, 
    REPL_DIP=5,
    REPL_BRRIP=6,
    REPL_SRRIP_MET0=7,
    REPL_SRRIP_MET1=8,
    REPL_SRRIP_MET2=9,
    REPL_SRRIP_MET3=10,
    REPL_BRRIP_MET0=11,
    REPL_BRRIP_MET1=12,
    REPL_BRRIP_MET2=13,
    REPL_BRRIP_MET3=14,
    REPL_DRRIP_MET0=15,
    REPL_DRRIP_MET1=16,
    REPL_DRRIP_MET2=17,
    REPL_DRRIP_MET3=18,
    REPL_DRRIP_3_MET0=19,
    REPL_DRRIP_3_MET1=20,
    REPL_DRRIP_3_MET2=21,
    REPL_DRRIP_3_MET3=22,
    REPL_DRRIP_3_MET4=23,
    REPL_DRRIP_3_MET5=24,
    REPL_DRRIP_3_MET6=25,
    REPL_DRRIP_3_MET7=26,
    NUM_REPL_POLICY=27
} MCache_ReplPolicy;


typedef struct MCache_Entry MCache_Entry;
typedef struct MCache MCache;

///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

struct MCache_Entry {
    Flag    valid;
    Flag    dirty;
    Addr    tag;
    uns64     ripctr;
    uns64   last_access;

  //Added by Gururaj
  uns64 bitlen; //Mainly for compressed caches to track size of entry
  //Added by Prashant
    int     tid;
};


struct MCache{
  uns64 sets;
  uns64 assocs;

  MCache_ReplPolicy repl_policy; //0:LRU  1:RND 2:SRRIP
  uns64 index_policy; // how to index cache

  Flag *is_leader_p0; // leader SET for D(RR)IP
  Flag *is_leader_p1; // leader SET for D(RR)IP
  uns64 psel;

  MCache_Entry *entries;
  uns64 *fifo_ptr; // for fifo replacement (per set)
  int touched_wayid;
  int touched_setid;
  int touched_lineid;

  uns64 s_count; // number of accesses
  uns64 s_miss; // number of misses
  uns64 s_evict; // number of evictions

  //Metadata replacement policy variables
  uns64 met_install_prio;
  uns64 met_aware;

  //Cache Occupancy - added by Prashant
  uns64 total_lines_cache;
  uns64 *total_lines_thread;
  double *total_lines_thread_percent;

  int is_compressed;  
  uns64 real_assocs; // only for compressed cache
  uns64* set_bitlens;
  MCache_Entry evicted_entries[MAX_COMPR_RATIO];
  uns64 num_evicted;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

MCache *mcache_new(uns64 sets, uns64 assocs, uns64 repl, int is_compressed );
Flag    mcache_access        (MCache *c, Addr addr);
void    mcache_install       (MCache *c, Addr addr);
Flag    mcache_probe         (MCache *c, Addr addr);
Flag    mcache_invalidate    (MCache *c, Addr addr);
Flag    mcache_mark_dirty    (MCache *c, Addr addr);
uns64     mcache_get_index     (MCache *c, Addr addr);
uns64     mcache_get_way       (MCache *c, Addr addr);


uns64     mcache_find_victim   (MCache *c, uns64 set);
uns64     mcache_find_victim_lru   (MCache *c, uns64 set);
uns64     mcache_find_victim_rnd   (MCache *c, uns64 set);
uns64     mcache_find_victim_srrip   (MCache *c, uns64 set);
uns64     mcache_find_victim_fifo    (MCache *c, uns64 set);
void    mcache_swap_lines(MCache *c, uns64 set, uns64 way_i, uns64 way_j);

void    mcache_select_leader_sets(MCache *c,uns64 sets);
uns64     mcache_drrip_get_ripctrval(MCache *c, uns64 set);
uns64     mcache_brrip_get_ripctrval(MCache *c, uns64 set);
Flag    mcache_dip_check_lru_update(MCache *c, uns64 set);


//Added by Gururaj:
Flag         mcache_access_markdirty        (MCache *c, Addr addr, Flag dirty, int tid);
MCache_Entry mcache_install_withevicts      (MCache *c, Addr addr, Flag dirty, int tid);
void         count_dirty_lines              (MCache *c, char* cache_t);

//Added by Prashant:
void	init_cache_vars(MCache *c, char* cache_t);

//Metadata-Aware Caching
void init_met_rrip(MCache* c);

// install, evict functions for compressed caches
MCache_Entry mcache_comp_install_withevicts(MCache *c, Addr addr, Flag dirty, int tid, uns64 bitlen);
uns64     mcache_comp_find_victim (MCache *c, uns64 set, uns64 bitlen);
int  mcache_comp_is_dirty_evict (MCache *c, int evict_id);

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

#endif // MCACHE_H
