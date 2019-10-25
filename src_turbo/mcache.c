#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mcache.h"

#include "debug.h"

#define MCACHE_SRRIP_MAX  7
#define MCACHE_SRRIP_INIT 1
#define MCACHE_BRRIP_INSTALL_REG  0
#define MCACHE_BRRIP_INSTALL_RARE 1
#define MCACHE_SRRIP_INSTALL      1

#define MCACHE_PSEL_MAX    1023
#define MCACHE_LEADER_SETS  32

extern memOrg_t* mem_org;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


MCache *
mcache_new(uns64 sets, uns64 assocs, uns64 repl_policy, int is_compressed )
{


  MCache *c = (MCache *) calloc (1, sizeof (MCache));
  c->sets    = sets;
  c->assocs  = assocs;
  c->repl_policy = (MCache_ReplPolicy)repl_policy;

  //If compressed cache
  c->is_compressed = is_compressed;
  if(c->is_compressed){

    c->real_assocs = c->assocs;
    c->assocs = (c->assocs * (MAX_COMPR_RATIO));
    c->set_bitlens = (uns64*) calloc (sets, sizeof(uns64));
  }
  
  c->entries  = (MCache_Entry *) calloc (sets *(c-> assocs), sizeof(MCache_Entry));

  c->fifo_ptr  = (uns64 *) calloc (sets, sizeof(uns64));

  //for metadata aware s/brrip 
  c->met_install_prio = (MCACHE_SRRIP_INSTALL);
  c->met_aware = FALSE;

  if( (c->repl_policy >= REPL_SRRIP_MET0) && (c->repl_policy <= REPL_DRRIP_3_MET7) ){    
    c->met_aware = TRUE;
    //modify c->met_install_prio and c->repl_policy
    init_met_rrip(c);
  }

  //for drrip or dip
  if( (c->repl_policy == REPL_DRRIP) || (c->repl_policy == REPL_DIP) ){
    assert(sets > 10*MCACHE_LEADER_SETS); 
    mcache_select_leader_sets(c,sets);
  }
  
  c->psel=(MCACHE_PSEL_MAX+1)/2;

  //Cache Occupancy - added by Prashant
  c->total_lines_cache = sets * assocs;
  c->total_lines_thread = (uns64 *) calloc (num_cores, sizeof(uns64));
  c->total_lines_thread_percent = (double *) calloc (num_cores, sizeof(double));
  
  return c;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void mcache_select_leader_sets(MCache *c, uns64 sets){
  uns64 done=0;

  c->is_leader_p0  = (Flag *) calloc (sets, sizeof(Flag));
  c->is_leader_p1  = (Flag *) calloc (sets, sizeof(Flag));
  
  while(done <= MCACHE_LEADER_SETS){
    uns64 randval=rand()%sets;
    if( (c->is_leader_p0[randval]==FALSE)&&(c->is_leader_p1[randval]==FALSE)){
      c->is_leader_p0[randval]=TRUE;
      done++;
    }
  }

  done=0;
  while(done <= MCACHE_LEADER_SETS){
    uns64 randval=rand()%sets;
    if( (c->is_leader_p0[randval]==FALSE)&&(c->is_leader_p1[randval]==FALSE)){
      c->is_leader_p1[randval]=TRUE;
      done++;
    }
  }
}


////////////////////////////////////////////////////////////////////
// the addr field is the lineaddress = address/cache_line_size
////////////////////////////////////////////////////////////////////

Flag mcache_access (MCache *c, Addr addr)
{
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii;
    
  c->s_count++;
    
  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    
    if(entry->valid && (entry->tag == tag)) {
      entry->last_access  = c->s_count;
      entry->ripctr       = MCACHE_SRRIP_MAX; 
      c->touched_wayid = (ii-start); 
      c->touched_setid = set; 
      c->touched_lineid = ii; 
      return HIT;
      
    }
  }

  //even on a miss, we need to know which set was accessed
  c->touched_wayid = 0;
  c->touched_setid = set;
  c->touched_lineid = start;

  c->s_miss++;
  return MISS;
}


////////////////////////////////////////////////////////////////////
// the addr field is the lineaddress = address/cache_line_size
// Ensures writeback accesses that are hits, mark the entry dirty
////////////////////////////////////////////////////////////////////

Flag mcache_access_markdirty (MCache *c, Addr addr, Flag dirty, int tid) {
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii;
    
  c->s_count++;
    
  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    
    if(entry->valid && (entry->tag == tag))
      {
        entry->last_access  = c->s_count;
        entry->ripctr       = MCACHE_SRRIP_MAX;
        c->touched_wayid = (ii-start);
        c->touched_setid = set;
        c->touched_lineid = ii;
        
        if(dirty==TRUE){ //If the operation is a WB then mark it as dirty
          
          mcache_mark_dirty(c,tag);
          entry->tid = tid;
        }
	
        return HIT;
      }
  }

  //even on a miss, we need to know which set was accessed
  c->touched_wayid = 0;
  c->touched_setid = set;
  c->touched_lineid = start;

  c->s_miss++;
  return MISS;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Flag    mcache_probe    (MCache *c, Addr addr)
{
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii;

  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag))
      {
	return TRUE;
      }
  }
  
  return FALSE;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Flag    mcache_invalidate    (MCache *c, Addr addr)
{
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii;

  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag))
      {
	entry->valid = FALSE;
	return TRUE;
      }
  }
  
  return FALSE;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

void    mcache_swap_lines(MCache *c, uns64 set, uns64 way_ii, uns64 way_jj)
{
  uns64   start = set * c->assocs;
  uns64   loc_ii   = start + way_ii;
  uns64   loc_jj   = start + way_jj;

  MCache_Entry tmp = c->entries[loc_ii];
  c->entries[loc_ii] = c->entries[loc_jj];
  c->entries[loc_jj] = tmp;

}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

Flag    mcache_mark_dirty    (MCache *c, Addr addr)
{
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii;

  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag))
      {
	entry->dirty = TRUE;
	return TRUE;
      }
  }
  
  return FALSE;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


void mcache_install (MCache *c, Addr addr)
{
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii, victim;
  
  Flag update_lrubits=TRUE;
  
  MCache_Entry *entry;

  for (ii=start; ii<end; ii++){
    entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag)){
      printf("\nCache_Size\t: %lld\n",(long long int)((c->sets)*(c->assocs)*64));
      printf("Installed entry already with addr:%llx present in set:%llu\n", addr, set);
      exit(-1);
    }
  }
  
  // find victim and install entry
  victim = mcache_find_victim(c, set);
  entry = &c->entries[victim];

  if(entry->valid){
    c->s_evict++;
  }
  
  //udpate DRRIP info and select value of ripctr
  uns64 ripctr_val=MCACHE_SRRIP_INIT;

  if(c->repl_policy==REPL_BRRIP){
    ripctr_val=mcache_brrip_get_ripctrval(c,set);
  }

  if(c->repl_policy==REPL_DRRIP){
    ripctr_val=mcache_drrip_get_ripctrval(c,set);
  }

  if(c->repl_policy==REPL_DIP){
    update_lrubits=mcache_dip_check_lru_update(c,set);
  }

  //**CHANGED**
  //If installed entry is metadata, install in decided priority
  if((c->met_aware == TRUE) && (get_partition(addr<<6, mem_org) == 1) ){
    ripctr_val = c->met_install_prio;
  }

  //put new information in
  entry->tag   = tag;
  entry->valid = TRUE;
  entry->dirty = FALSE;
  entry->ripctr  = ripctr_val;

  if(update_lrubits){
    entry->last_access  = c->s_count;   
  }

  c->fifo_ptr[set] = (c->fifo_ptr[set]+1)%c->assocs; // fifo update

  c->touched_lineid=victim;
  c->touched_setid=set;
  c->touched_wayid=victim-(set*c->assocs);

 
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// Evicts entries while installing & returns the evict
// Accounts for installing dirty entries (writebacks)
///////////////////////////////////////////////////////////////////

MCache_Entry mcache_install_withevicts (MCache *c, Addr addr, Flag dirty, int tid) {
  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii, victim;
  
  Flag update_lrubits=TRUE;
  
  MCache_Entry *entry;
  MCache_Entry evicted_entry;
  
  for (ii=start; ii<end; ii++){
    entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag)){
       printf("\nCache_Size\t: %lld\n",(long long int)((c->sets)*(c->assocs)*64));
       printf("Installed entry already with addr:%llx present in set:%llu\n", addr, set);
       print_backtrace();
       exit(-1);
    }
  }

  
  // find victim and install entry
  victim = mcache_find_victim(c, set);
  entry = &c->entries[victim];
  evicted_entry =c->entries[victim]; 

  if(entry->valid){
    c->s_evict++;
  }
  
  //Update the Occupancy Counters
  c->total_lines_thread[tid]++;
  if(entry->valid){
  	c->total_lines_thread[entry->tid]--;
  } 
  c->total_lines_thread_percent[tid]=((double)c->total_lines_thread[tid]*100)/c->total_lines_cache;
  c->total_lines_thread_percent[entry->tid]=((double)c->total_lines_thread[entry->tid]*100)/c->total_lines_cache;

  //udpate DRRIP info and select value of ripctr
  uns64 ripctr_val=MCACHE_SRRIP_INSTALL;

  if(c->repl_policy==REPL_BRRIP){
    ripctr_val=mcache_brrip_get_ripctrval(c,set);
  }

  if(c->repl_policy==REPL_DRRIP){
    ripctr_val=mcache_drrip_get_ripctrval(c,set);
  }

  if(c->repl_policy==REPL_DIP){
    update_lrubits=mcache_dip_check_lru_update(c,set);
  }
  
  //If installed entry is metadata, install in decided priority  
  if((c->met_aware == TRUE) && (get_partition(addr<<6, mem_org) == 1) ){
    ripctr_val = c->met_install_prio;
  }
  
  //put new information in
  entry->tag   = tag;
  entry->valid = TRUE;
  entry->tid   = tid; //Added by Prashant
  
  if(dirty==TRUE)
    entry->dirty = TRUE;
  else
    entry->dirty = FALSE;

  entry->ripctr  = ripctr_val;

  if(update_lrubits){
    entry->last_access  = c->s_count;   
  }



  c->fifo_ptr[set] = (c->fifo_ptr[set]+1)%c->assocs; // fifo update

  c->touched_lineid=victim;
  c->touched_setid=set;
  c->touched_wayid=victim-(set*c->assocs);

  return evicted_entry;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
Flag mcache_dip_check_lru_update(MCache *c, uns64 set){
  Flag update_lru=TRUE;

  if(c->is_leader_p0[set]){
    if(c->psel<MCACHE_PSEL_MAX){
      c->psel++;
    }
    update_lru=FALSE;
    if(rand()%100<5) update_lru=TRUE; // BIP
  }
  
  if(c->is_leader_p1[set]){
    if(c->psel){
      c->psel--;
    }
    update_lru=1;
  }

  if( (c->is_leader_p0[set]==FALSE)&& (c->is_leader_p1[set]==FALSE)){
    if(c->psel >= (MCACHE_PSEL_MAX+1)/2){
      update_lru=1; // policy 1 wins
    }else{
      update_lru=FALSE; // policy 0 wins
      if(rand()%100<5) update_lru=TRUE; // BIP
    }
  }
  
  return update_lru;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
uns64 mcache_drrip_get_ripctrval(MCache *c, uns64 set){
  uns64 ripctr_val=MCACHE_SRRIP_INIT;

  //BRRIP
  if(c->is_leader_p0[set]){
    if(c->psel<MCACHE_PSEL_MAX){
      c->psel++;
    }
    ripctr_val= (MCACHE_BRRIP_INSTALL_REG) ;//0
    if(rand()%100<5) ripctr_val= (MCACHE_BRRIP_INSTALL_RARE) ; //1 
  }
  
  //SRRIP
  if(c->is_leader_p1[set]){
    if(c->psel){
      c->psel--;
    }
    ripctr_val= (MCACHE_SRRIP_INSTALL);
  }

  if( (c->is_leader_p0[set]==FALSE)&& (c->is_leader_p1[set]==FALSE)){
    if(c->psel >= (MCACHE_PSEL_MAX+1)/2){
      ripctr_val= (MCACHE_SRRIP_INSTALL); // policy 1 wins (SRRIP)
    }else{
      ripctr_val= (MCACHE_BRRIP_INSTALL_REG); // policy 0 wins (BRRIP)
      if(rand()%100<5) ripctr_val= (MCACHE_BRRIP_INSTALL_RARE); // BIP
    }
  }
  

  return ripctr_val;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
uns64 mcache_brrip_get_ripctrval(MCache *c, uns64 set){

  uns64 ripctr_val= (MCACHE_BRRIP_INSTALL_REG);
  
  ripctr_val= (MCACHE_BRRIP_INSTALL_REG) ;//0
  
  if(rand()%100<5) ripctr_val= (MCACHE_BRRIP_INSTALL_RARE) ; //1   

  return ripctr_val;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 mcache_find_victim (MCache *c, uns64 set)
{
  uns64 ii;
  uns64 start = set   * c->assocs;    
  uns64 end   = start + c->assocs;    

  //search for invalid first
  for (ii = start; ii < end; ii++){
    if(!c->entries[ii].valid){
      return ii;
    }
  }


  switch(c->repl_policy){
  case REPL_LRU: 
    return mcache_find_victim_lru(c, set);
  case REPL_RND: 
    return mcache_find_victim_rnd(c, set);
  case REPL_SRRIP: 
    return mcache_find_victim_srrip(c, set);
  case REPL_BRRIP: 
    return mcache_find_victim_srrip(c, set);
  case REPL_DRRIP: 
    return mcache_find_victim_srrip(c, set);
  case REPL_FIFO: 
    return mcache_find_victim_fifo(c, set);
  case REPL_DIP: 
    return mcache_find_victim_lru(c, set);
  default: 
    assert(0);
  }

  return -1;

}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 mcache_find_victim_lru (MCache *c,  uns64 set)
{
  uns64 start = set   * c->assocs;    
  uns64 end   = start + c->assocs;    
  uns64 lowest=start;
  uns64 ii;

  //Initialize lowest to the first valid entry
  for (ii = start; ii < end; ii++){
    if(c->entries[ii].valid){
      lowest = ii;
      break;
    }
  }
  
  for (ii = start; ii < end; ii++){
    if(c->entries[ii].valid){
      if (c->entries[ii].last_access < c->entries[lowest].last_access){
        lowest = ii;
      }
    }
  }

  return lowest;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 mcache_find_victim_rnd (MCache *c,  uns64 set)
{
    uns64 start = set   * c->assocs;    
    uns64 victim = start + rand()%c->assocs; 
    
    return  victim;   
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 mcache_find_victim_srrip (MCache *c,  uns64 set)
{
  uns64 start = set   * c->assocs;    
  uns64 end   = start + c->assocs;    
  uns64 ii;
  uns64 victim = end; // init to impossible

  while(victim == end){
    for (ii = start; ii < end; ii++){
      if (c->entries[ii].ripctr == 0){
	victim = ii;
	break;
      }
    }
    
    if(victim == end){
      for (ii = start; ii < end; ii++){
	c->entries[ii].ripctr--;
      }
    }
  }

  return  victim;   
}





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 mcache_find_victim_fifo (MCache *c,  uns64 set)
{
  uns64 start = set   * c->assocs;    
  uns64 retval = start + c->fifo_ptr[set];
  return retval;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64 mcache_get_index(MCache *c, Addr addr){
  uns64 retval;

  switch(c->index_policy){
  case 0:
    retval=addr%c->sets;
    break;

  default:
    exit(-1);
  }

  return retval;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//Added by Gururaj
//Returns a error & exits, if cacheline is a miss in cache
//Assume that it is used only when you have checked existence of cacheline by doing a mcache_access
///////////////////////////////////////////////////////////
uns64 mcache_get_way(MCache *c, Addr addr){

  uns64 set = mcache_get_index(c, addr);
  uns64 tag = addr;
  uns64 start = set * c->assocs;
  uns64 end   = start + c->assocs;
  uns64 ii;
  
  
  for (ii=start; ii<end; ii++){
    MCache_Entry *entry = &c->entries[ii];
    
    if(entry->valid && (entry->tag == tag)) {
      return (ii - start);
    }
  }

  ASSERTM(0,"Error executing mcache_get_way function" );

  return 0;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//Added by Prashant
//Prints the initial values of cache
///////////////////////////////////////////////////////////////////
void init_cache_vars(MCache *c, char* cache_t)
{
  printf("\n%s_Cache_Size\t: %lld",cache_t, (long long int)((c->sets)*(c->assocs)*64));
  printf("\n%s_Cache_Sets\t: %llu",cache_t, (c->sets));
  printf("\n%s_Cache_Assc\t: %llu",cache_t, (c->assocs));
  printf("\n%s_Cache_Repl\t: %u\n",cache_t, (c->repl_policy));
  printf("\n%s_Cache_Met_Aware\t: %llu\n",cache_t, (c->met_aware));
  printf("\n%s_Cache_Met_Install_Prioirity\t: %llu\n",cache_t, (c->met_install_prio));

}


///////////////////////////////////////////////////////

void init_met_rrip(MCache* c){
  c->met_install_prio = MCACHE_SRRIP_INSTALL;
  printf("**Execution of unexecutable**");
  switch(c->repl_policy){
  case REPL_SRRIP_MET0:
    c->met_install_prio = 0;
    c->repl_policy = REPL_SRRIP;
    break;
  case REPL_SRRIP_MET1:
    c->met_install_prio = 1;
    c->repl_policy = REPL_SRRIP;
    break;
  case REPL_SRRIP_MET2:
    c->met_install_prio = 2;
    c->repl_policy = REPL_SRRIP;
    break;
  case REPL_SRRIP_MET3:
    c->met_install_prio = 3;
    c->repl_policy = REPL_SRRIP;
    break;
  case REPL_BRRIP_MET0:
    c->met_install_prio = 0;
    c->repl_policy = REPL_BRRIP;
    break;
  case REPL_BRRIP_MET1:
    c->met_install_prio = 1;
    c->repl_policy = REPL_BRRIP;
    break;
  case REPL_BRRIP_MET2:
    c->met_install_prio = 2;
    c->repl_policy = REPL_BRRIP;
    break;
  case REPL_BRRIP_MET3:
    c->met_install_prio = 3;
    c->repl_policy = REPL_BRRIP;
    break;
  case REPL_DRRIP_MET0:
    c->met_install_prio = 0;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_MET1:
    c->met_install_prio = 1;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_MET2:
    c->met_install_prio = 2;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_MET3:
    c->met_install_prio = 3;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET0:
    c->met_install_prio = 0;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET1:
    c->met_install_prio = 1;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET2:
    c->met_install_prio = 2;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET3:
    c->met_install_prio = 3;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET4:
    c->met_install_prio = 4;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET5:
    c->met_install_prio = 5;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET6:
    c->met_install_prio = 6;
    c->repl_policy = REPL_DRRIP;
    break;
  case REPL_DRRIP_3_MET7:
    c->met_install_prio = 7;
    c->repl_policy = REPL_DRRIP;
    break;
  default:
    c->met_install_prio = (MCACHE_SRRIP_INSTALL);
    c->repl_policy = REPL_BRRIP;
    ASSERTM(0, "Something is wrong with the met-aware replacement policy\n"); 
    break;
  }
   
}

void count_dirty_lines (MCache* Cache, char* cache_t){
  uns64 dirty_lines = 0;

  for(uns64 entry_id = 0; entry_id < (uns64) (Cache->sets*Cache->assocs); entry_id++){    
    if( (Cache->entries[entry_id]).valid && (Cache->entries[entry_id]).dirty ){      
      dirty_lines++;
    } 
  }
  
  printf ("%s_DIRTY_LINES           \t: %llu\n", cache_t, dirty_lines); 
  printf ("%s_DIRTY_LINE_PERC       \t: %f\n",  cache_t, 1.0* (double)dirty_lines / (double)((uns64) (Cache->sets*Cache->assocs)) ) ;

}



//////////// Functions for Compressed Caches ////////////


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// Installs entry and evicts as many as required. List in c->evicted_entries, c->num_evicts
// Returns beginning of that list.
///////////////////////////////////////////////////////////////////

MCache_Entry mcache_comp_install_withevicts (MCache *c, Addr addr, Flag dirty, int tid, uns64 bitlen) {

  Addr  tag  = addr; // full tags
  uns64   set  = mcache_get_index(c,addr);
  uns64   start = set * c->assocs;
  uns64   end   = start + c->assocs;
  uns64   ii, victim;
  
  Flag update_lrubits=TRUE;
  
  MCache_Entry *entry;
  MCache_Entry evicted_entry;

  ASSERTM(c->is_compressed, "Cache needs to be compressed if calling this function");
  
  for (ii=start; ii<end; ii++){
    entry = &c->entries[ii];
    if(entry->valid && (entry->tag == tag)){
      printf("\nCache_Size\t: %lld\n",(long long int)((c->sets)*(c->assocs)*64));
      printf("Installed entry already with addr:%llx present in set:%llu\n", addr, set);
      print_backtrace();
      exit(-1);
    }
  }
  
  // Get victim (func will invalidate them in the tag store, move them to evicted_entries list & number of victims, and update the set->bit_len
  
  victim = mcache_comp_find_victim(c, set, bitlen);
  entry = &c->entries[victim];

  //Evicted entries now in c->evicted_entries. (Should be invalid if comp. miss, else should be valid entries.
  evicted_entry =c->evicted_entries[0]; 
    
  //Covered in find_victim
  /* 
     if(entry->valid){
     c->s_evict++;
     }
  */
  
  //Update the Occupancy Counters
  c->total_lines_thread[tid]++;
  c->total_lines_thread_percent[tid]=((double)c->total_lines_thread[tid]*100)/c->total_lines_cache;

  //Covered in find_victim
  /*
    if(entry->valid){
    c->total_lines_thread[entry->tid]--;
    } 
  */
  /*
    c->total_lines_thread_percent[entry->tid]=((double)c->total_lines_thread[entry->tid]*100)/c->total_lines_cache;
  */
  
  //Select rip value if RIP based REPL
  uns64 ripctr_val=MCACHE_SRRIP_INSTALL;

  if(c->repl_policy==REPL_BRRIP){
    ripctr_val=mcache_brrip_get_ripctrval(c,set);
  }

  if(c->repl_policy==REPL_DRRIP){
    ripctr_val=mcache_drrip_get_ripctrval(c,set);
  }

  if(c->repl_policy==REPL_DIP){
    update_lrubits=mcache_dip_check_lru_update(c,set);
  }
  
  //If installed entry is metadata, install in decided priority  
  if((c->met_aware == TRUE) && (get_partition(addr<<6, mem_org) == 1) ){
    ripctr_val = c->met_install_prio;
  }
  
  //put new information in selected "entry"
  entry->tag   = tag;
  entry->valid = TRUE;
  entry->tid   = tid; //Added by Prashant

  //Update the bitlen in entry and total bitlen of set.
  ASSERTM((bitlen + c->set_bitlens[set]) <= (CACHE_LINE_SIZE*8*c->real_assocs), "No more space in set");
  entry->bitlen = bitlen;
  c->set_bitlens[set] += bitlen;
   
  if(dirty==TRUE)
    entry->dirty = TRUE;
   else
     entry->dirty = FALSE;

   entry->ripctr  = ripctr_val;

   //For LRU & DIP REPL
   if(update_lrubits){
     entry->last_access  = c->s_count;   
   }

   //For FIFO REPL
   c->fifo_ptr[set] = (c->fifo_ptr[set]+1)%c->assocs; // fifo update

   //Last touched
   c->touched_lineid=victim;
   c->touched_setid=set;
   c->touched_wayid=victim-(set*c->assocs);

   return evicted_entry;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//Returns first victim after setting the c->evicted_entries list and c->num_evicted
uns64 mcache_comp_find_victim (MCache *c, uns64 set, uns64 bitlen)
{
  uns64 ii;
  uns64 start = set   * c->assocs;    
  uns64 end   = start + c->assocs;    
  uns64 first_victim = 0;
  uns64 num_victims = 0;
  uns64 rand_iter = 0;
  
  ASSERTM(((c->is_compressed) && (c->repl_policy == REPL_LRU)), "Only designed for compressed cache with LRU policy");
  
  //search for invalid first, if space exists in set. 
  if((bitlen + c->set_bitlens[set]) <= (CACHE_LINE_SIZE*8*c->real_assocs)){    
    for (ii = start; ii < end; ii++){
      if(!c->entries[ii].valid){
        num_victims = 1;
        c->evicted_entries[num_victims -1] = c->entries[ii]; //Copy victim to list 
        c->num_evicted = num_victims;
        return ii;
       }
     }
     ASSERTM(0, "If space exists in set_bitlens, then should always find a invalid line");
   }
  
   //No space, then find a victim with LRU
  
  switch(c->repl_policy){
  case REPL_LRU: 
    first_victim =  mcache_find_victim_lru(c, set);
    break;
  case REPL_RND: 
    first_victim =  mcache_find_victim_rnd(c, set);
    break;
  case REPL_SRRIP: 
    first_victim =  mcache_find_victim_srrip(c, set);
    break;
  case REPL_BRRIP: 
    first_victim =  mcache_find_victim_srrip(c, set);
    break;
  case REPL_DRRIP: 
    first_victim =  mcache_find_victim_srrip(c, set);
    break;
  case REPL_FIFO: 
    first_victim =  mcache_find_victim_fifo(c, set);
    break;
  case REPL_DIP: 
    first_victim =  mcache_find_victim_lru(c, set);
    break;
  default: 
    assert(0);
  }
  
  //Mark first_victim for evcition (copy to list, invalidate, occupany_metrics,reduce set_bitlen)
  num_victims = 1;
  c->evicted_entries[num_victims -1] = c->entries[first_victim]; //Copy victim to list
  ASSERTM(c->entries[first_victim].valid, "Victim is invalid: ERROR");
  c->entries[first_victim].valid = 0; //invalidate
  c->set_bitlens[set] -=   c->entries[first_victim].bitlen; //Deallocate space in set_bitlens
  c->entries[first_victim].bitlen = 0; //no space occupied

  c->s_evict++; //Evict count
  c->total_lines_thread[c->entries[first_victim].tid]--; //Update occupancy
  c->total_lines_thread_percent[c->entries[first_victim].tid]=((double)c->total_lines_thread[c->entries[first_victim].tid]*100)/c->total_lines_cache;
  
  //Evict more entries if space does not exist in set_bitlens
  while((bitlen + c->set_bitlens[set]) > (CACHE_LINE_SIZE*8*c->real_assocs) ){
    uns64 next_victim = 0;
    
    uns64 rand_iter = 1;
    next_victim =  mcache_find_victim_rnd(c, set);

    //    printf("1.next Victim is %llu\n", next_victim);
    //If rand does not find victim on first attempt, try again.
    
    while( (c->entries[next_victim].valid == 0) && (rand_iter < c->assocs )){
      rand_iter++;
      next_victim =  mcache_find_victim_rnd(c, set);
      //      printf("2.next Victim is %llu\n", next_victim);
    }

    
    //Rare scenario that multiple rand does not find a valid victim, get first valid entry
    if((rand_iter >= c->assocs) &&  (c->entries[next_victim].valid == 0) ){
      for(uns64 i =start;i< end; i++){
        if(c->entries[i].valid){
          next_victim = i;
          //printf("3.next Victim is %llu\n", next_victim);

          break;            
        }            
      }
    } //Got next_victim by anyway
    
    if(!c->entries[next_victim].valid){
      printf("Bitlen of input is %llu, set_bitlens[] is %llu\n",bitlen,c->set_bitlens[set]);
      printf("Next Victim %llu\n",next_victim);

      for(int i=start; i< end; i++){
        if(c->entries[i].valid){
          printf("%d is valid\n",i);
        }            
      }
    }
      
    ASSERTM(c->entries[next_victim].valid, "Victim is invalid:ERROR");
    num_victims++;
    c->evicted_entries[num_victims -1] = c->entries[next_victim]; //Copy victim to list
    c->entries[next_victim].valid = 0; //invalidate
    c->set_bitlens[set] -=   c->entries[next_victim].bitlen; //Deallocate space in set_bitlens
    c->entries[next_victim].bitlen = 0; //no space occupied

    c->s_evict++; //Evict count
    c->total_lines_thread[c->entries[next_victim].tid]--; //Update occupancy
    c->total_lines_thread_percent[c->entries[next_victim].tid]=((double)c->total_lines_thread[c->entries[next_victim].tid]*100)/c->total_lines_cache;
        
  }

  ASSERTM((bitlen + c->set_bitlens[set]) <= (CACHE_LINE_SIZE*8*c->real_assocs), "still no space in set_bitlens");
  
   c->num_evicted = num_victims;
   return first_victim;

 }

//Tells if the evicted id is dirty.
int  mcache_comp_is_dirty_evict (MCache *c, int evict_id){

  ASSERTM(evict_id < c->num_evicted, "Cant access more than num_evicts");

  return c->evicted_entries[evict_id].dirty;
    
}
  
