#include "memsys_control_flow.h"
//#include "debug.h"

extern unsigned long long int IS_MAC_WRITEBACK;
extern unsigned long long int MAC_PRIVATE;

extern unsigned long long int IS_MET_WRITEBACK;
extern unsigned long long int MET_PRIVATE;

extern unsigned long long int L3_PRIVATE;

extern int SGX_MODE[4];

extern ctr_cl** ctr_cls;
extern ctr_type** ctr_types;

extern struct robstructure * ROB;
extern long long int BIGNUM;
extern long long int CYCLE_VAL;
extern int COMP_CTR_MODE;

//Extern variables for PB_Cache
//extern cache_stats* PB_Cache_Stats ;
//extern MCache **PBCache ;
//extern unsigned long long int IS_PB_WRITEBACK;
//extern unsigned long long int PB_PRIVATE;

Flag cache_access(MCache** cache, unsigned long long int is_private,  ADDR addr, Flag dirty, int tid, cache_stats* stats, maccess_type type, memOrg_t* mem_org){

  /*

    Assumptions regarding structure of stats
    -stats[0] - data
    -stats[1] - metadata (counters+merkle Tree)
    -stats[2] - counters
    -stats[3] - merkle tree - n levels   
    -stats[4] - merkle tree - level 0
    ..
    -stats[4+n-1] - merkle tree - level n-1
    -stats[4+n] - mac

  */

  Flag outcome = FALSE;
  cache_stats* curr_stats;
  unsigned long long int *write_hits_stat;
  unsigned long long int *write_misses_stat;
  unsigned long long int *read_hits_stat;
  unsigned long long int *read_misses_stat;

  unsigned long long int *write_hits_inst_stat;
  unsigned long long int *write_misses_inst_stat;
  unsigned long long int *read_hits_inst_stat;
  unsigned long long int *read_misses_inst_stat;

  
  if(is_private){
    outcome = mcache_access_markdirty(cache[tid],addr>>6,dirty,tid);
  }
  else{
    outcome = mcache_access_markdirty(cache[0],addr>>6,dirty,tid);
  }


  //Update Stats
  
  //Assuming that the cache is shared (if this changes, need to redesign some of the stats data-structures in main & here)
  ASSERTM(is_private==0,"ERROR: Assertion that current cache is shared is false. Stats updation for private cache (based on type of access, not built in. Need to bring back commented code and comment the selection of stats code. Also need to revert to previous data-structure design in main for cache_stats\n");

  /*  
      if(is_private)
      curr_stats  = &(stats[tid]);
      else
      curr_stats  = &(stats[0]);
  */

  if(type.memtype == DATA ){
    //Data
    ASSERTM(get_partition(addr,mem_org) == 0,"Memory access type and actual get_partition differ on type of address: data\n");
    curr_stats = &(stats[0]);
    cache_stats_update_access(curr_stats, outcome, dirty);
  }
  else if(type.memtype == MET){
    curr_stats = &(stats[1]);
    cache_stats_update_access(curr_stats, outcome, dirty);

    if(type.mtree_level ==  mem_org->num_Mtree_levels){
      //Counters
      ASSERTM(get_partition(addr,mem_org) == 2,"Memory access type and actual get_partition differ on type of address: counters\n");
      curr_stats = &(stats[2]);
      cache_stats_update_access(curr_stats, outcome, dirty);      
    }
    else {
      //Merkle Tree
      ASSERTM(get_partition(addr,mem_org) == 1,"Memory access type and actual get_partition differ on type of address: Merkle Tree\n");
      curr_stats = &(stats[3]);
      cache_stats_update_access(curr_stats, outcome, dirty);
      //printf("type is %d, Mtree level in type is: %d and partition is %d, addr is %llu, level according to addr is: %d  \n", type.memtype , type.mtree_level, get_partition(addr,mem_org),addr, getMtreeEvictParent(addr,mem_org).mtree_level+1 );

      //Update specific merkle tree level stats
      //print_backtrace();
      ASSERTM((getMtreeEvictParent(addr,mem_org).mtree_level + 1) == type.mtree_level,"Memory access type and actual get_partition differ on type of address: Merkle Tree Level\n");
      ASSERTM((4+type.mtree_level) < (4+mem_org->num_Mtree_levels), "Accessed stats index is not allocated\n");
      curr_stats = &(stats[4+type.mtree_level]);
      cache_stats_update_access(curr_stats, outcome, dirty);
    }   
  }
  else if(type.memtype == MAC){
    ASSERTM( get_partition(addr,mem_org) == 3,"Memory access type and actual get_partition differ on type of address: MACs\n");      
    curr_stats = &(stats[4+mem_org->num_Mtree_levels]);
    cache_stats_update_access(curr_stats, outcome, dirty);
  } 
  else if(type.memtype == TWIN){
    //Do nothing
  }
  else {
    ASSERTM(0,"Access Type is neither data or metadata or MAC\n");
  }

  //  cache_stats_update_access(curr_stats, outcome, dirty);
  
  /*
    write_hits_stat   = &(curr_stats ->  write_hits[0]);
    write_misses_stat = &(curr_stats ->  write_misses[0]);
    read_hits_stat    = &(curr_stats ->  read_hits[0]);  
    read_misses_stat  = &(curr_stats ->  read_misses[0]);

    write_hits_inst_stat    = &(curr_stats ->  write_hits_inst[0]); 
    write_misses_inst_stat  = &(curr_stats ->  write_misses_inst[0]);
    read_hits_inst_stat     = &(curr_stats ->  read_hits_inst[0]);  
    read_misses_inst_stat   = &(curr_stats ->  read_misses_inst[0]);
  
    if((outcome==HIT) && (dirty == FALSE)){
    (*read_hits_stat)++;
    (*read_hits_inst_stat)++; 
    } 
    else if((outcome==HIT) && (dirty == TRUE)){
    (*write_hits_stat)++;
    (*write_hits_inst_stat)++; 
    }
    else if((outcome==MISS) && (dirty == FALSE)){
    (*read_misses_stat)++;
    (*read_misses_inst_stat)++; 
    }
    else if((outcome==MISS) && (dirty == TRUE)){
    (*write_misses_stat)++;
    (*write_misses_inst_stat)++; 
    }
    else{
    ASSERTM(0,"ERROR: All outcomes exhausted during cache stats assignment"); 
    }
  */
  return outcome;
  
}

MCache_Entry cache_install(MCache** cache, unsigned long long is_private, ADDR addr, Flag dirty, int tid, cache_stats* stats, maccess_type type, memOrg_t* mem_org){

  MCache_Entry evicted_entry;
  cache_stats* curr_stats;
  unsigned long long int *dirty_evicts_stat;
  unsigned long long int *dirty_evicts_inst_stat;
  
  if(is_private){
    evicted_entry = mcache_install_withevicts(cache[tid],addr>>6,dirty,tid);

  }
  else{
    evicted_entry = mcache_install_withevicts(cache[0],(addr>>6),dirty,tid);
  }

  
  //Update Stats
  if(evicted_entry.dirty && evicted_entry.valid){

    /*

      if(is_private)
      curr_stats  = &(stats[tid]);
      else
      curr_stats  = &(stats[0]);

    */
    ASSERTM(is_private==0,"ERROR: Assertion that current cache is shared is false. Stats updation for private cache (based on type of access, not built in. Need to bring back commented code and comment the selection of stats code. Also need to revert to previous data-structure design in main for cache_stats\n");
    
    if(type.memtype == DATA ){
      //Data
      ASSERTM(get_partition(addr,mem_org) == 0,"Memory access type and actual get_partition differ on type of address: data\n");
      curr_stats = &(stats[0]);
      cache_stats_update_install(curr_stats);
    }
    else if(type.memtype == MET){
      curr_stats = &(stats[1]);
      cache_stats_update_install(curr_stats);
      
      if(type.mtree_level ==  mem_org->num_Mtree_levels){
        //Counters
        ASSERTM(get_partition(addr,mem_org) == 2,"Memory access type and actual get_partition differ on type of address: counters\n");
        curr_stats = &(stats[2]);
        cache_stats_update_install(curr_stats);      
      }
      else {
        //Merkle Tree
        ASSERTM(get_partition(addr,mem_org) == 1,"Memory access type and actual get_partition differ on type of address: Merkle Tree\n");
        curr_stats = &(stats[3]);
        cache_stats_update_install(curr_stats);
	
        //Update specific merkle tree level stats
        ASSERTM((getMtreeEvictParent(addr,mem_org).mtree_level + 1) == type.mtree_level,"Memory access type and actual get_partition differ on type of address: Merkle Tree Level\n");
        ASSERTM((4+type.mtree_level) < (4+mem_org->num_Mtree_levels), "Accessed stats index is not allocated\n");
        curr_stats = &(stats[4+type.mtree_level]);
        cache_stats_update_install(curr_stats);
      }   
    }
    else if(type.memtype == MAC){
      ASSERTM( get_partition(addr,mem_org) == 3,"Memory access type and actual get_partition differ on type of address: MACs\n");      
      curr_stats = &(stats[4+mem_org->num_Mtree_levels]);
      cache_stats_update_install(curr_stats);
    } 
    
    else {
      ASSERTM(0,"Access Type is neither data or metadata or MAC\n");
    }

    //    cache_stats_update_install(curr_stats);


    /*
      dirty_evicts_stat   = &(curr_stats ->  dirty_evicts[0]);
      dirty_evicts_inst_stat   = &(curr_stats ->  dirty_evicts_inst[0]);
  
      (*dirty_evicts_stat)++;
      (*dirty_evicts_inst_stat)++;
    */
  }

  return evicted_entry;
}



/*
  void handle_dirty_wb_LLC (long long int wb_addr, MCache_Entry evicted_entry, int mem_prio, memOrg_t* mem_org, MCache** METCache, MCache** L3Cache, int tid,
  cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, long long int* instrpc){

  maccess_type data_access = {DATA,-1};
  maccess_type met_access = {MET,0};
  ctr_mtree_entry met_entry, evicted_met_parent;
  Flag policy_install_metcache = FALSE;
  int addr_type = get_partition(wb_addr, mem_org);

  if(addr_type == 0){ //Data Dirty Writeback

  insert_write_memsys(wb_addr, CYCLE_VAL, evicted_entry.tid, ROB[tid].tail, mem_prio, FALSE,data_access);
    
  if(SGX_MODE >= 1){
  // Added by Gururaj
  // 1. Check if counter exists on-chip
  //    - If Yes, break. If No - Fetch from memory & do the same for higher levels till you hit on-chip
  met_entry = getCounterAddr(wb_addr, mem_org);
  policy_install_metcache = FALSE; //**VARIABLE**
  read_ctr_mtree(METCache, L3Cache, met_entry, tid, MET_Cache_Stats, L3_Cache_Stats, FALSE,
  mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
      
  //Update ctr_sim corresponding to writeback data
  update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, met_entry.paddr, mem_org->num_Mtree_levels);

  // 2. Check if MAC is present in MAC_STORE - read from MACCache or Memory (write a dirty evict, if required)
  // 3. Write MAC to MACCache (if writeback) or to memory (if writethrough)
      
  //*********************************************************************************************************
  // Removed since MAC is present in ECC-Chip
  //
  //mac_paddr = getMACAddr(wb_addr,mem_org); 
  //write_mac(MACCache, mac_paddr, evicted_entry.tid, MAC_Cache_Stats,
  //          mem_org, ROB, CYCLE_VAL, instrpc, mem_prio); 
  //   
  //*********************************************************************************************************
  // 4. Write to Counter in Met_Cache (writeback) ; Take care of dirty evicts (write to memory and dirty install one higher entry)
  write_ctr_mtree(METCache,L3Cache, met_entry, tid, MET_Cache_Stats, L3_Cache_Stats,
  mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
  }
  }
  else if ( (addr_type == 1 ) || (addr_type == 2) ) {
  ASSERTM(IS_MET_WRITEBACK, "Not designed for non-writeback Metadata Caches"); //**VARIABLE **
  ASSERTM(SGX_MODE >= 1, "Writebacks for metadata in non-secure mode should not possible\n");
    
  evicted_met_parent = getMtreeEvictParent(wb_addr, mem_org);
  met_access.mtree_level = evicted_met_parent.mtree_level+1; 

  //Write the dirty metadata to the memory
  insert_write_memsys(wb_addr, CYCLE_VAL, tid, ROB[tid].tail, mem_prio, FALSE, met_access);

  //Update the counter re-encryption sim
  //       if(addr_type == 2)
  //     //printf("Number of Ctr Cls %llu\n",ctr_types->num_ctr_cls);
  //     update_ctr_cls(mem_org, ctr_cls, ctr_types, wb_addr);
  //   
  //
  //Read the next level into LLC, mark it dirty
  if(evicted_met_parent.mtree_level != 0){
  policy_install_metcache = FALSE; //** VARIABLE **
  //** VARIABLE ** - partial writethrough
  read_ctr_mtree(METCache, L3Cache, evicted_met_parent, tid, MET_Cache_Stats, L3_Cache_Stats, FALSE,
  mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
  //** VARIABLE ** - rather than keeping this write in LLC, mark it dirty in Metadata Cache

  //update the ctr_sim 
  update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, evicted_met_parent.paddr, evicted_met_parent.mtree_level);

  write_ctr_mtree(METCache, L3Cache, evicted_met_parent, tid, MET_Cache_Stats, L3_Cache_Stats,
  mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
  }
        
  }
  else {
  ASSERTM(0, "Writeback of non-data or non-metadata not allowed\n");
  }

  }

*/

//Used for reading or writing MACs - either to/from cache or memory. Updates stats */
void read_mac(MCache** mac_cache, long long int mac_paddr, int tid, cache_stats* stats, Flag is_critical,
              memOrg_t* mem_org, struct robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio){
  //data_byteaddr is the address of the data for which we need to get the MAC

  maccess_type mac_access = {MAC,-1};  
  Flag mac_outcome = FALSE;
  MCache_Entry evicted_mac;
  
  mac_outcome = cache_access(mac_cache,MAC_PRIVATE,mac_paddr,FALSE,tid,stats,mac_access, mem_org);
  //  printf("***For MAC ADDR: %llu, Cache Outcome is: %d",mac_paddr, mac_outcome); //DEBUG
  if(mac_outcome == FALSE) {//MISS in MAC_CACHE, Fetch from Memory
    insert_read_memsys(mac_paddr, CYCLE_VAL, tid, ROB[tid].tail, instrpc[tid], mem_prio, is_critical, mac_access);

    /* if(is_critical) */
    /*   ROB[tid].critical_mem_ops[ ROB[tid].tail ] ++;  */

    evicted_mac = cache_install(mac_cache,MAC_PRIVATE,mac_paddr,FALSE,tid,stats,mac_access, mem_org);

    if(evicted_mac.valid && evicted_mac.dirty){
      ASSERTM(IS_MAC_WRITEBACK, "MAC_CACHE is not writeback, yet evicted mac is dirty");
      ADDR wb_mac_paddr = (evicted_mac.tag)<<6;
      insert_write_memsys(wb_mac_paddr, CYCLE_VAL, evicted_mac.tid, ROB[tid].tail, mem_prio, FALSE, mac_access);
    }
       
  }

}

void write_mac(MCache** mac_cache, long long int mac_paddr,int tid, cache_stats* stats,
               memOrg_t* mem_org,  struct  robstructure* ROB, long long int CYCLE_VAL,  long long int* instrpc, int mem_prio){

  maccess_type mac_access = {MAC,-1};  
  Flag write_mac_access_outcome = FALSE;
  Flag dirty = TRUE;

  read_mac(mac_cache, mac_paddr, tid, stats, FALSE,
           mem_org, ROB, CYCLE_VAL, instrpc, mem_prio); 
                   
  //MACCache IS WRITETHROUGH
  if(!IS_MAC_WRITEBACK){
    insert_write_memsys(mac_paddr, CYCLE_VAL, tid, ROB[tid].tail, mem_prio, FALSE, mac_access); 
    return;
  }

  //MACCache IS WRITEBACK
  if(IS_MAC_WRITEBACK){
    write_mac_access_outcome =  cache_access(mac_cache, MAC_PRIVATE, mac_paddr,dirty,tid,stats,mac_access,mem_org);
    ASSERTM(write_mac_access_outcome == HIT, "Logic of MAC write for writeback MACCache is wrong\n");    
    return;   
  }
  
}
/*
//Used for reading a Ctr or a Merkle Tree entry from Cache or Memory
void read_ctr_mtree(MCache** METCache, MCache** L3Cache, ctr_mtree_entry met_entry, int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, Flag is_critical,
memOrg_t* mem_org,  struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio, int policy_install_metcache){

Flag met_outcome = FALSE; //Met found in MetCache
Flag met_llc_outcome = FALSE; //Met found in LLC

ASSERTM(met_entry.mtree_level != 0, "Root of the mtree is never fetched from off-chip\n");
maccess_type met_access = {MET,met_entry.mtree_level};
ASSERTM( (getMtreeEvictParent(met_entry.paddr,mem_org).mtree_level + 1) == met_entry.mtree_level,"read_ctr_mtree inputs dont match - addr mtree level not the same as asserted met_entry.mtree_level \n" );

Flag dirty = FALSE;
MCache_Entry evicted_met, evicted_LLC;
Flag is_partial = FALSE;
  
//Flags:
Flag install_in_metadata_cache = FALSE;
Flag break_mtree_walk          = FALSE;

while(met_entry.mtree_level != 0){

install_in_metadata_cache = FALSE;
break_mtree_walk          = FALSE;
    
//Check the dedicated metadata cache
met_outcome = cache_access(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access, mem_org);
    
//Hit in METCACHE, STOP Mtree_walk
if(met_outcome == TRUE){ 
break_mtree_walk = TRUE;
break;
}
else { //Miss in METCACHE, Check in LLC      
//Install in the metadata cache, if policy says so
install_in_metadata_cache = policy_install_metcache;
      
//Checking LLC for metadata
met_llc_outcome =  cache_access(L3Cache,L3_PRIVATE,met_entry.paddr,dirty,tid,L3_Cache_Stats,met_access, mem_org);
           
if(met_llc_outcome){ //Hit in LLC for metadata	
//End merkle tree walk - break while loop
break_mtree_walk = TRUE;     
}
else { //Miss in LLC for Metadata
//Insert request in the memory read queue
insert_read_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, instrpc[tid], mem_prio, is_critical, met_access);
if(is_critical)
ROB[tid].comptime[ROB[tid].tail] += BIGNUM ;  	      
//Install data in LLC
evicted_LLC = cache_install(L3Cache,L3_PRIVATE,met_entry.paddr,dirty,tid,L3_Cache_Stats,met_access, mem_org);
//Handle Dirty Evicts
if(evicted_LLC.valid && evicted_LLC.dirty){
unsigned long long int llc_wb_addr=((unsigned long long int)evicted_LLC.tag)<<6; 
handle_dirty_wb_LLC(llc_wb_addr, evicted_LLC, mem_prio, mem_org, METCache, L3Cache, tid,
MET_Cache_Stats, L3_Cache_Stats, instrpc);
}
} //end handling LLC miss 	

} //end handling MetCache miss
    
//Insert data in Metadata cache if required
if(install_in_metadata_cache){
//Install in MetCache
ASSERTM(met_outcome == FALSE, "\nShould not install Metadata in MetCache if Hit\n");
evicted_met = cache_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access, mem_org);    
//Handle Dirty Evicts of metadata to LLC
if(evicted_met.valid && evicted_met.dirty){
unsigned long long int met_wb_addr=((unsigned long long int)evicted_met.tag)<<6; 
//Write 64 Byte Dirty Metadata Line to LLC
is_partial = FALSE;
write_Met_LLC (METCache, L3Cache, met_wb_addr,is_partial, 
tid, MET_Cache_Stats, L3_Cache_Stats, mem_org, instrpc, mem_prio);
}
}

if(break_mtree_walk)
break;    

//        printf("Before: Met Address : %llu, level :%d, entry_num: %llu,  Partition %d\n",met_entry.paddr, met_entry.mtree_level, met_entry.entry_num, get_partition(met_entry.paddr,mem_org));

//    if(met_entry.mtree_level == mem_org->num_Mtree_levels){
//      ASSERTM(get_partition(met_entry.paddr,mem_org) == 2, "Counter address, does not match with counter partition\n");
//    }

//Go one level up in the Merkle Tree Walk
met_entry = getMtreeEntry(met_entry, mem_org);
met_access.mtree_level = met_entry.mtree_level;
    
if(met_entry.mtree_level != 0){
// printf("After: Met Address : %llu, level :%d, entry_num: %llu, Partition : %d,\n",met_entry.paddr, met_entry.mtree_level,  met_entry.entry_num, get_partition(met_entry.paddr,mem_org));

ASSERTM( (getMtreeEvictParent(met_entry.paddr,mem_org).mtree_level + 1) == met_entry.mtree_level,"read_ctr_mtree inputs dont match - addr mtree level not the same as asserted met_entry.mtree_level \n" );
}
    
  
//    if((met_entry.mtree_level == (mem_org->num_Mtree_levels -1)) && (get_partition(met_entry.paddr,mem_org) != 1)){
//      printf("Address : %llu\n",met_entry.paddr);
//    }

//    if(met_entry.mtree_level == (mem_org->num_Mtree_levels -1))
//      ASSERTM(get_partition(met_entry.paddr,mem_org) == 1, "Mtree address, does not match with mtree partition\n");
  
} //Merkle Tree Walk done
      
}

*/
/*
//Used for writing a metadata entry on chip, into either the memory (writethrough) or just a dirty install into the cache (writeback) 
void write_ctr_mtree(MCache** METCache, MCache** L3Cache, ctr_mtree_entry met_entry, int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats,
memOrg_t* mem_org, struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio){

maccess_type met_access = {MET,met_entry.mtree_level};
Flag met_outcome = FALSE;
Flag dirty = TRUE;
MCache_Entry evicted_met;
Flag is_partial = TRUE;
Flag is_write_allocate = FALSE;

ASSERTM(IS_MET_WRITEBACK, "Not designed for non-writeback Metadata Caches"); //**VARIABLE **

//Check if metadata present in Metadata Cache
  
met_outcome = cache_access(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access, mem_org);
    
//If Hits in MetCache, then done
if(met_outcome){ 
return;
}
//If Miss in MetCache, then write the 8-byte dirty Metadata to LLC
else {    
is_partial = TRUE;
write_Met_LLC (METCache, L3Cache, met_entry.paddr,is_partial, 
tid, MET_Cache_Stats, L3_Cache_Stats, mem_org, instrpc, mem_prio);
    
//Do clean install of line if write-allocate policy - **VARIABLE**
if(is_write_allocate){ 
//Install in MetCache
ASSERTM(met_outcome == FALSE, "\nShould not install Metadata in MetCache if Hit\n");
evicted_met = cache_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access, mem_org);    
//Handle Dirty Evicts of metadata to LLC
if(evicted_met.valid && evicted_met.dirty){
unsigned long long int met_wb_addr=((unsigned long long int)evicted_met.tag)<<6; 
//Write 64 Byte Dirty Metadata Line to LLC
is_partial = FALSE;
write_Met_LLC (METCache, L3Cache, met_wb_addr,is_partial, 
tid, MET_Cache_Stats, L3_Cache_Stats, mem_org, instrpc, mem_prio);
}
      
}
    
} //End Handling Misses in MetCache
}
*/

void write_Met_LLC (MCache** METCache, MCache** L3Cache,   MCache** MACCache,  long long int met_paddr, Flag is_partial,
                    int tid,cache_stats* MET_Cache_Stats,  cache_stats* L3_Cache_Stats, cache_stats* MAC_Cache_Stats,   overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst, memOrg_t* mem_org, long long int* instrpc, int mem_prio){
  
  maccess_type met_access = {MET,-1};
  Flag dirty = TRUE;
  ctr_mtree_entry met_parent = getMtreeEvictParent(met_paddr, mem_org);
  Flag met_llc_outcome = FALSE;
  met_access.mtree_level = met_parent.mtree_level+1; 
  Flag is_critical = FALSE;
  Flag policy_install_metcache = FALSE; //**VARIABLE**
  MCache_Entry evicted_LLC;
  ASSERTM( (met_access.mtree_level != -1 ) && (met_access.mtree_level != 0 ), "Error while getting level of mtree entry\n");

  //Checking LLC for metadata
  met_llc_outcome = cache_access(L3Cache,L3_PRIVATE,met_paddr,dirty,tid,L3_Cache_Stats,met_access, mem_org);
  
  //If it hits in LLC - then done !
  if(met_llc_outcome == TRUE)
    return;
  else{
    Flag met_llc_outcome_repeat = FALSE;
    if(is_partial){
      //Read the entire metadata cacheline from memory
      insert_read_memsys(met_paddr, CYCLE_VAL, tid, ROB[tid].tail, instrpc[tid], mem_prio, is_critical, met_access);
      
      //Read next level in Merkle Tree (Tree Walk)
      read_ctr_mtree(METCache, L3Cache, MACCache, met_parent, tid, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, is_critical,
                     mem_org,  ROB, CYCLE_VAL, instrpc, mem_prio, policy_install_metcache);
    }
    //On Miss, Mark Dirty the metadata in LLC
    met_llc_outcome_repeat = cache_access(L3Cache,L3_PRIVATE,met_paddr,dirty,tid,L3_Cache_Stats,met_access, mem_org);
 
    if(met_llc_outcome_repeat == FALSE){
      evicted_LLC = cache_install(L3Cache,L3_PRIVATE,met_paddr,dirty,tid,L3_Cache_Stats,met_access, mem_org);
      //Handle Dirty Evicts
      if(evicted_LLC.valid && evicted_LLC.dirty){
	unsigned long long int llc_wb_addr=((unsigned long long int)evicted_LLC.tag)<<6; 
	handle_dirty_wb_LLC(llc_wb_addr, evicted_LLC, mem_prio, mem_org, METCache, L3Cache, MACCache, tid,
                        MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, instrpc);
      }
    }
  } //End LLC miss handling 
}

/* //Older version with dedicated Metadata Cache //
//Used for reading a Ctr or a Merkle Tree entry from Cache or Memory
void read_ctr_mtree(MCache** METCache, ctr_mtree_entry met_entry, int tid, cache_stats** stats, Flag is_critical,
                    memOrg_t* mem_org,  struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio){

  Flag met_outcome = FALSE;
  ASSERTM(met_entry.mtree_level != 0, "Root of the mtree is never fetched from off-chip\n");
  maccess_type met_access = {MET,met_entry.mtree_level};
  maccess_type met_access_temp = {MET,met_entry.mtree_level}; 
  Flag dirty = FALSE;
  ctr_mtree_entry evicted_met_parent;
  MCache_Entry evicted_met;
  
  while(met_entry.mtree_level != 0){
                     
    met_outcome = cache_access(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,stats[met_entry.mtree_level],met_access);

    if(met_outcome){ //HIT in METCACHE, STOP Mtree_walk
      break;
    }
    else {//MISS in METCACHE, Fetch from Memory
      insert_read_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, instrpc[tid], mem_prio, is_critical, met_access);
      if(is_critical)
        ROB[tid].comptime[ROB[tid].tail] += BIGNUM ;  

      evicted_met = cache_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,stats[met_entry.mtree_level],met_access);

      //Only bother in case of Writeback METCACHE
      if(evicted_met.valid && evicted_met.dirty){
        ASSERTM(IS_MET_WRITEBACK, "METCACHE is not writeback, yet evicted met is dirty");
        ADDR wb_met_paddr = (evicted_met.tag)<<6;
        evicted_met_parent = getMtreeEvictParent(wb_met_paddr, mem_org);
        met_access_temp.mtree_level = evicted_met_parent.mtree_level+1; 
        insert_write_memsys(wb_met_paddr, CYCLE_VAL, evicted_met.tid, ROB[tid].tail, mem_prio, FALSE, met_access_temp);

        if(evicted_met_parent.mtree_level != 0){
          read_ctr_mtree(METCache, evicted_met_parent, evicted_met.tid, stats, FALSE,
                         mem_org, ROB,CYCLE_VAL,instrpc, mem_prio); 
          write_ctr_mtree(METCache,evicted_met_parent,evicted_met.tid, stats,
                          mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
        }

      } 
                     
      //Go one level up in the Merkle Tree Walk
      met_entry = getMtreeEntry(met_entry, mem_org);
      met_access.mtree_level = met_entry.mtree_level;
    }
                       
  } //Merkle Tree Walk Done
  
}
*/

/* //Older version with dedicated Metadata Cache //
//Used for writing a metadata entry on chip, into either the memory (writethrough) or just a dirty install into the cache (writeback) 
ovoid write_ctr_mtree(MCache** METCache, ctr_mtree_entry met_entry, int tid, cache_stats** stats,
                     memOrg_t* mem_org, struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio){

  maccess_type met_access = {MET,met_entry.mtree_level};
  maccess_type met_access_temp = {MET,met_entry.mtree_level}; 
  Flag write_met_access_outcome = FALSE;
  Flag dirty = TRUE;
  ctr_mtree_entry evicted_met_parent;
  MCache_Entry evicted_met;
  
  //METCache IS WRITETHROUGH
  if(!IS_MET_WRITEBACK){

    insert_write_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, mem_prio, FALSE, met_access);

    while(met_entry.mtree_level != 1){
      met_entry = getMtreeEntry(met_entry, mem_org);
      met_access.mtree_level = met_entry.mtree_level;
      insert_write_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, mem_prio, FALSE, met_access);
    } 

    return;
  }

  //METCache IS WRITEBACK
  if(IS_MET_WRITEBACK){
    write_met_access_outcome =  cache_access(METCache, MET_PRIVATE, met_entry.paddr,dirty,tid,stats[met_entry.mtree_level],met_access); 

    if(write_met_access_outcome != HIT){
      evicted_met = cache_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,stats[met_entry.mtree_level],met_access);
      //ASSERTM(evicted_met.dirty == FALSE, "Logic of MET write for writeback METCache is wrong\n");          

      if(evicted_met.valid && evicted_met.dirty){
        ADDR wb_met_paddr = (evicted_met.tag)<<6;
        
        evicted_met_parent = getMtreeEvictParent(wb_met_paddr, mem_org);
        met_access_temp.mtree_level = evicted_met_parent.mtree_level+1; 
        insert_write_memsys(wb_met_paddr, CYCLE_VAL, evicted_met.tid, ROB[tid].tail, mem_prio, FALSE, met_access_temp);

        if(evicted_met_parent.mtree_level != 0){
          read_ctr_mtree(METCache, evicted_met_parent, evicted_met.tid, stats, FALSE,
                         mem_org, ROB,CYCLE_VAL,instrpc, mem_prio); 
          write_ctr_mtree(METCache,evicted_met_parent,evicted_met.tid, stats,
                          mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
        }
        
      }

      

    } 
    return;   
  } 

}

*/

// insert reads into the read queue as per Baseline Memory System Design (normal or chipkill)
void insert_read_memsys(long long int physical_address, long long int arrival_cycle, int thread_id, int instruction_id, long long int instruction_pc, int mem_prio, int is_critical, maccess_type type){
  maccess_type twin_type =  {TWIN,-1};
  if(SGX_MODE[thread_id] != 2){
    if((is_critical != 0) && (sim_fast != 1)){
      ROB[thread_id].critical_mem_ops[ ROB[thread_id].tail ] ++;
    }
    insert_read(physical_address, arrival_cycle, thread_id, instruction_id, instruction_pc, mem_prio, is_critical, type); 
    
  }

  else if(SGX_MODE[thread_id] == 2){
    if((is_critical != 0) && (sim_fast != 1)){
      ROB[thread_id].critical_mem_ops[ ROB[thread_id].tail ] +=2;
    }
    insert_read(physical_address, arrival_cycle, thread_id, instruction_id, instruction_pc, mem_prio, is_critical, type);
    insert_read(get_twin(physical_address), arrival_cycle, thread_id, instruction_id, instruction_pc, mem_prio, is_critical, twin_type); 
  }
  else{
    
    ASSERTM(0,"insert_read not defined for SGX_MODE = 2");
  }

}

// insert writes into write queue as per Baseline Memory System Design (normal or chipkill)
void insert_write_memsys(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, int mem_prio, int is_critical, maccess_type type){
  maccess_type twin_type =  {TWIN,-1};
  if(SGX_MODE[thread_id] != 2){
    insert_write(physical_address, arrival_time, thread_id, instruction_id, mem_prio, is_critical, type); 
  }

  else if(SGX_MODE[thread_id] == 2){
    insert_write(physical_address, arrival_time, thread_id, instruction_id, mem_prio, is_critical, type);
    insert_write(get_twin(physical_address), arrival_time, thread_id, instruction_id, mem_prio, is_critical, twin_type); 
  }
  else{
    ASSERTM(0,"insert_write not defined for SGX_MODE = 2");
  }

}


void cache_stats_update_access(cache_stats* curr_stats, Flag outcome, Flag dirty){

  unsigned long long int *write_hits_stat;
  unsigned long long int *write_misses_stat;
  unsigned long long int *read_hits_stat;
  unsigned long long int *read_misses_stat;

  unsigned long long int *write_hits_inst_stat;
  unsigned long long int *write_misses_inst_stat;
  unsigned long long int *read_hits_inst_stat;
  unsigned long long int *read_misses_inst_stat;

  write_hits_stat   = &(curr_stats ->  write_hits[0]);
  write_misses_stat = &(curr_stats ->  write_misses[0]);
  read_hits_stat    = &(curr_stats ->  read_hits[0]);  
  read_misses_stat  = &(curr_stats ->  read_misses[0]);

  write_hits_inst_stat    = &(curr_stats ->  write_hits_inst[0]); 
  write_misses_inst_stat  = &(curr_stats ->  write_misses_inst[0]);
  read_hits_inst_stat     = &(curr_stats ->  read_hits_inst[0]);  
  read_misses_inst_stat   = &(curr_stats ->  read_misses_inst[0]);
  
  if((outcome==HIT) && (dirty == FALSE)){
    (*read_hits_stat)++;
    (*read_hits_inst_stat)++; 
  } 
  else if((outcome==HIT) && (dirty == TRUE)){
    (*write_hits_stat)++;
    (*write_hits_inst_stat)++; 
  }
  else if((outcome==MISS) && (dirty == FALSE)){
    (*read_misses_stat)++;
    (*read_misses_inst_stat)++; 
  }
  else if((outcome==MISS) && (dirty == TRUE)){
    (*write_misses_stat)++;
    (*write_misses_inst_stat)++; 
  }
  else{
    ASSERTM(0,"ERROR: All outcomes exhausted during cache stats assignment"); 
  }

}

void cache_stats_update_install(cache_stats* curr_stats){

  unsigned long long int *dirty_evicts_stat;
  unsigned long long int *dirty_evicts_inst_stat;

  dirty_evicts_stat   = &(curr_stats ->  dirty_evicts[0]);
  dirty_evicts_inst_stat   = &(curr_stats ->  dirty_evicts_inst[0]);
  
  (*dirty_evicts_stat)++;
  (*dirty_evicts_inst_stat)++;

}

//Functions for not caching metadata in LLC

void handle_dirty_wb_LLC (long long int wb_addr, MCache_Entry evicted_entry, int mem_prio, memOrg_t* mem_org, MCache** METCache, MCache** L3Cache, MCache** MACCache, int tid,
                          cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, cache_stats* MAC_Cache_Stats,  overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst, long long int* instrpc){

  maccess_type data_access = {DATA,-1};
  maccess_type met_access = {MET,0};
  ctr_mtree_entry met_entry, evicted_met_parent;
  Flag policy_install_metcache = FALSE;


  insert_write_memsys(wb_addr, CYCLE_VAL, evicted_entry.tid, ROB[tid].tail, mem_prio, FALSE,data_access);

  if(SGX_MODE[tid] >= 1){
    // Added by Gururaj
    // 1. Check if counter exists in metCache - required for $line encryption
    //    - If Yes, break. If No - Fetch from memory & do the same for higher levels till you hit in MetCache
    long long int mac_paddr;

    if(SGX_MODE[tid] != 5){ //Read only in mode that is not CTRFree (i.e. 1,2,3,4)
      met_entry = getCounterAddr(wb_addr, mem_org);
      read_ctr_mtree(METCache, L3Cache, MACCache, met_entry, evicted_entry.tid, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst,FALSE,
                     mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
    }
    
    // 2. Check if MAC is present in MAC_STORE - read from MACCache or Memory (write a dirty evict, if required)
    // 3. Write MAC to MACCache (if writeback) or to memory (if writethrough)
    
    //*********************************************************************************************************
    // Removed since MAC is present in ECC-Chip
    if( SGX_MODE[tid] != 4 ){  //Will write MAC in SGX_MODE = 1 or 2 or 3 or 5
      mac_paddr = getMACAddr(wb_addr,mem_org); 
      write_mac(MACCache, mac_paddr, evicted_entry.tid, MAC_Cache_Stats,
                mem_org, ROB, CYCLE_VAL, instrpc, mem_prio); 
    }
    //   
    //*********************************************************************************************************
    
    // 4. Write to Counter in Met_Cache (writeback) ; Take care of dirty evicts (write to memory and dirty install one higher entry)
    // 5. Write to Counter and all Mtree_Entries (write-through)

    if(SGX_MODE[tid] != 5){ //Read only in mode that is not CTRFree (i.e. 1,2,3,4)
    
      met_entry = getCounterAddr(wb_addr, mem_org);
    //Update ctr_sim corresponding to writeback data
      update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, met_entry, mem_org->num_Mtree_levels, ctr_overflows_levelwise,  ctr_overflows_levelwise_inst );

      write_ctr_mtree(METCache, L3Cache, MACCache, met_entry, evicted_entry.tid, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst,
                      mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
    }
  }
  
}


//Used for reading a Ctr or a Merkle Tree entry from Cache or Memory
void read_ctr_mtree(MCache** METCache, MCache** L3Cache, MCache** MACCache, ctr_mtree_entry met_entry, int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, cache_stats* MAC_Cache_Stats,  overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst, Flag is_critical,
                    memOrg_t* mem_org,  struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio, int policy_install_metcache){


  Flag met_outcome = FALSE;
  ASSERTM(met_entry.mtree_level != 0, "Root of the mtree is never fetched from off-chip\n");
  maccess_type met_access = {MET,met_entry.mtree_level};
  maccess_type met_access_temp = {MET,met_entry.mtree_level}; 
  Flag dirty = FALSE;
  ctr_mtree_entry evicted_met_parent;
  MCache_Entry evicted_met;
  
  while(met_entry.mtree_level != 0){
    //printf("%lx, %d, ctr_store : %lld\n", met_entry.paddr,met_access.mtree_level, mem_org->num_Mtree_levels, mem_org->ctr_store_size);
    met_outcome = cache_access(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access,mem_org);

    if(met_outcome){ //HIT in MET_CACHE, STOP Mtree_walk
      break;
    }
    else {//MISS in MET_CACHE, Fetch from Memory
      insert_read_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, instrpc[tid], mem_prio, is_critical, met_access);
      /* if(is_critical) */
      /*   ROB[tid].critical_mem_ops[ ROB[tid].tail ] ++;  */
      if((is_critical != 0) && (sim_fast != 1)){
        ROB[tid].comptime[ROB[tid].tail] += BIGNUM ;  
      }


      if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3)){
        evicted_met = cache_comp_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access,mem_org);
        ASSERTM(evicted_met.tag == METCache[0]->evicted_entries[0].tag, "First entry of evicted_entry should be returned");
        ASSERTM(METCache[0]->num_evicted >0, "Metcache always evicts >=1");
        for(int evict_id =0; evict_id < METCache[0]->num_evicted; evict_id++){
          evicted_met = METCache[0]->evicted_entries[evict_id];
          if(evicted_met.valid && evicted_met.dirty){
            ASSERTM(IS_MET_WRITEBACK, "MET_CACHE is not writeback, yet evicted met is dirty");
            ADDR wb_met_paddr = (evicted_met.tag)<<6;
            evicted_met_parent = getMtreeEvictParent(wb_met_paddr, mem_org);
            met_access_temp.mtree_level = evicted_met_parent.mtree_level+1; 
            insert_write_memsys(wb_met_paddr, CYCLE_VAL, evicted_met.tid, ROB[tid].tail, mem_prio, FALSE, met_access_temp);

            if(evicted_met_parent.mtree_level != 0){
              read_ctr_mtree(METCache, L3Cache, MACCache,  evicted_met_parent, evicted_met.tid, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, FALSE,
                             mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
              write_ctr_mtree(METCache, L3Cache, MACCache, evicted_met_parent,evicted_met.tid,  MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst,
                              mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
              update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, evicted_met_parent, evicted_met_parent.mtree_level, ctr_overflows_levelwise, ctr_overflows_levelwise_inst);

            }
          }
        }
      }
      else {
      
        evicted_met = cache_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access, mem_org);

        //Only bother in case of Writeback MET_CACHE
        if(evicted_met.valid && evicted_met.dirty){
          ASSERTM(IS_MET_WRITEBACK, "MET_CACHE is not writeback, yet evicted met is dirty");
          ADDR wb_met_paddr = (evicted_met.tag)<<6;
          evicted_met_parent = getMtreeEvictParent(wb_met_paddr, mem_org);
          met_access_temp.mtree_level = evicted_met_parent.mtree_level+1; 
          insert_write_memsys(wb_met_paddr, CYCLE_VAL, evicted_met.tid, ROB[tid].tail, mem_prio, FALSE, met_access_temp);

          if(evicted_met_parent.mtree_level != 0){
            read_ctr_mtree(METCache, L3Cache, MACCache,  evicted_met_parent, evicted_met.tid, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, FALSE,
                           mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
            write_ctr_mtree(METCache, L3Cache, MACCache, evicted_met_parent,evicted_met.tid,  MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst,
                            mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
            update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, evicted_met_parent, evicted_met_parent.mtree_level, ctr_overflows_levelwise, ctr_overflows_levelwise_inst );

          }

        }

      }
                     
      //Go one level up in the Merkle Tree Walk
      met_entry = getMtreeEntry(met_entry, mem_org);
      met_access.mtree_level = met_entry.mtree_level;
    }
                       
  } //Merkle Tree Walk Done

}

//Used for writing a metadata entry on chip, into either the memory (writethrough) or just a dirty install into the cache (writeback) 
void write_ctr_mtree(MCache** METCache, MCache** L3Cache,   MCache** MACCache, ctr_mtree_entry met_entry, int tid, cache_stats* MET_Cache_Stats, cache_stats* L3_Cache_Stats, cache_stats* MAC_Cache_Stats,  overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst,
                     memOrg_t* mem_org, struct  robstructure* ROB, long long int CYCLE_VAL, long long int* instrpc, int mem_prio){


  maccess_type met_access = {MET,met_entry.mtree_level};
  maccess_type met_access_temp = {MET,met_entry.mtree_level}; 
  Flag write_met_access_outcome = FALSE;
  Flag dirty = TRUE;
  ctr_mtree_entry evicted_met_parent;
  MCache_Entry evicted_met;
  
  //METCache IS WRITETHROUGH
  if(!IS_MET_WRITEBACK){

    insert_write_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, mem_prio, FALSE, met_access);

    while(met_entry.mtree_level != 1){
      met_entry = getMtreeEntry(met_entry, mem_org);
      met_access.mtree_level = met_entry.mtree_level;
      insert_write_memsys(met_entry.paddr, CYCLE_VAL, tid, ROB[tid].tail, mem_prio, FALSE, met_access);
    } 

    return;
  }

  //METCache IS WRITEBACK
  if(IS_MET_WRITEBACK){
    write_met_access_outcome =  cache_access(METCache, MET_PRIVATE, met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access,mem_org); 

    if(write_met_access_outcome != HIT){
      if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3)){
        evicted_met = cache_comp_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access,mem_org);
        ASSERTM(evicted_met.tag == METCache[0]->evicted_entries[0].tag, "First entry of evicted_entry should be returned");
        ASSERTM(METCache[0]->num_evicted >0, "Metcache always evicts >=1");
        for(int evict_id =0; evict_id < METCache[0]->num_evicted; evict_id++){
          evicted_met = METCache[0]->evicted_entries[evict_id];
          if(evicted_met.valid && evicted_met.dirty){
            ADDR wb_met_paddr = (evicted_met.tag)<<6;
        
            evicted_met_parent = getMtreeEvictParent(wb_met_paddr, mem_org);
            met_access_temp.mtree_level = evicted_met_parent.mtree_level+1; 
            insert_write_memsys(wb_met_paddr, CYCLE_VAL, evicted_met.tid, ROB[tid].tail, mem_prio, FALSE, met_access_temp);

            if(evicted_met_parent.mtree_level != 0){
              read_ctr_mtree(METCache, L3Cache, MACCache, evicted_met_parent, evicted_met.tid,MET_Cache_Stats,L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, FALSE,
                             mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, TRUE); 
              write_ctr_mtree(METCache, L3Cache, MACCache,evicted_met_parent,evicted_met.tid,MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst,
                              mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);

              update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, evicted_met_parent, evicted_met_parent.mtree_level, ctr_overflows_levelwise, ctr_overflows_levelwise_inst );
            }
          }
        } 
      }
      else{
        evicted_met = cache_install(METCache,MET_PRIVATE,met_entry.paddr,dirty,tid,MET_Cache_Stats,met_access,mem_org);

        if(evicted_met.valid && evicted_met.dirty){
          ADDR wb_met_paddr = (evicted_met.tag)<<6;
        
          evicted_met_parent = getMtreeEvictParent(wb_met_paddr, mem_org);
          met_access_temp.mtree_level = evicted_met_parent.mtree_level+1; 
          insert_write_memsys(wb_met_paddr, CYCLE_VAL, evicted_met.tid, ROB[tid].tail, mem_prio, FALSE, met_access_temp);

          if(evicted_met_parent.mtree_level != 0){
            read_ctr_mtree(METCache, L3Cache, MACCache, evicted_met_parent, evicted_met.tid,MET_Cache_Stats,L3_Cache_Stats, MAC_Cache_Stats,ctr_overflows_levelwise, ctr_overflows_levelwise_inst, FALSE,
                           mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, TRUE); 
            write_ctr_mtree(METCache, L3Cache, MACCache,evicted_met_parent,evicted_met.tid,MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats,  ctr_overflows_levelwise, ctr_overflows_levelwise_inst, 
                            mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);

            update_ctr_cls_wrapper(mem_org, ctr_cls, ctr_types, evicted_met_parent, evicted_met_parent.mtree_level, ctr_overflows_levelwise, ctr_overflows_levelwise_inst );
	  
          }
        
        }

      }
      
    } 
    return;   
  } 


}


MCache_Entry cache_comp_install(MCache** cache, unsigned long long is_private, ADDR addr, Flag dirty, int tid, cache_stats* stats, maccess_type type, memOrg_t* mem_org){

  MCache_Entry evicted_entry;
  cache_stats* curr_stats;
  unsigned long long int *dirty_evicts_stat;
  unsigned long long int *dirty_evicts_inst_stat;
  uns64 ctrcl_bitlen =0;
  
  ASSERTM(is_private == 0, "Comp MetCache is assumed to be private");
  ASSERTM(cache[0]->is_compressed, "MetCache assumed to be compressed");
  ASSERTM(type.memtype == MET, "Only designed for Met Caches");

  //Determine bitlen
  ctrcl_bitlen = get_ctr_cl_bitlen(mem_org, ctr_cls, ctr_types, addr,type.mtree_level);
  
  //Install in compressed metcache
  evicted_entry = mcache_comp_install_withevicts(cache[0],(addr>>6),dirty,tid, ctrcl_bitlen);

  //Update stats for evictions
  for(int evicted_id = 0; evicted_id < cache[0]->num_evicted ; evicted_id++){

    evicted_entry = cache[0]->evicted_entries[evicted_id];
    
    //Update Stats
    if(evicted_entry.dirty && evicted_entry.valid){
      ASSERTM(type.memtype == MET, "Only designed for Met Caches");
      curr_stats = &(stats[1]);
      cache_stats_update_install(curr_stats);
      
      if(type.mtree_level ==  mem_org->num_Mtree_levels){
        //Counters
        ASSERTM(get_partition(addr,mem_org) == 2,"Memory access type and actual get_partition differ on type of address: counters\n");
        curr_stats = &(stats[2]);
        cache_stats_update_install(curr_stats);      
      }
      else {
        //Merkle Tree
        ASSERTM(get_partition(addr,mem_org) == 1,"Memory access type and actual get_partition differ on type of address: Merkle Tree\n");
        curr_stats = &(stats[3]);
        cache_stats_update_install(curr_stats);
	
        //Update specific merkle tree level stats
        ASSERTM((getMtreeEvictParent(addr,mem_org).mtree_level + 1) == type.mtree_level,"Memory access type and actual get_partition differ on type of address: Merkle Tree Level\n");
        ASSERTM((4+type.mtree_level) < (4+mem_org->num_Mtree_levels), "Accessed stats index is not allocated\n");
        curr_stats = &(stats[4+type.mtree_level]);
        cache_stats_update_install(curr_stats);
      }
      
    }

  }

  return cache[0]->evicted_entries[0];
}
