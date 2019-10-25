#include <stdio.h>
#include "stats.h"
#include "params.h"
#include "ctr_sim.h"

#define MEM_LATENCY 100

extern unsigned int DETAILED_INST_STATS;
extern unsigned int DETAILED_CACHE_STATS;
extern unsigned int DETAILED_MET_STATS;
extern memOrg_t* mem_org;
extern ctr_cl** ctr_cls;
extern ctr_type** ctr_types;
extern int SGX_MODE[4];


void init_cache_stats(cache_stats* stats, int num_array){
  stats->write_hits = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array);
  stats->write_misses = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array); 
  stats->read_hits = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array);
  stats->read_misses = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array); 
  stats->dirty_evicts = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array); 

  stats->write_hits_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array);
  stats->write_misses_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array);  
  stats->read_hits_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array);
  stats->read_misses_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array);
  stats->dirty_evicts_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*num_array) ; 

  //Initialize everything to 1
  for(int i=0; i<num_array; i++){
    stats->write_hits[i]   = 1;
    stats->write_misses[i] = 1;
    stats->read_hits[i]    = 1;
    stats->read_misses[i]  = 1;
    stats->dirty_evicts[i] = 1;
                          
    stats->write_hits_inst[i]   = 1;
    stats->write_misses_inst[i] = 1;
    stats->read_hits_inst[i]    = 1;
    stats->read_misses_inst[i]  = 1;
    stats->dirty_evicts_inst[i] = 1;
    
  }

}


double calc_rt(unsigned long long int num, unsigned long long int denom ){

if(denom == 0 )
  return 0;
 else
   return ( (double)( ((double)num) / ((double) denom) ) );
}
                  
double calc_mpki(unsigned long long int num, unsigned long long int denom ){

  if(denom == 0)
    return 0;
  else
    return ( (double)( ((double)(num*1000.0)) / ((double) denom) ) );
}


void calc_cache_stats (cache_stats* stats, Flag is_private,
                       cache_sim_stats* sim_stats, cache_sim_stats*  sim_stats_inst,
                       unsigned long long int count_i_total, unsigned long long int count_i_inst_total){

  ASSERTM(is_private == 0, "Not designed to support private caches");

  sim_stats->read_hitrt    = calc_rt( stats[0].read_hits[0], (stats[0].read_hits[0]+stats[0].read_misses[0]) );
  sim_stats->write_hitrt   = calc_rt( stats[0].write_hits[0], (stats[0].write_hits[0]+stats[0].write_misses[0]) );
  sim_stats->HITRT         = calc_rt( stats[0].read_hits[0] + stats[0].write_hits[0] , 
                                      (stats[0].read_hits[0]+stats[0].read_misses[0]) + (stats[0].write_hits[0]+stats[0].write_misses[0]) );
  sim_stats->rmpki         = calc_mpki(stats[0].read_misses[0], count_i_total);
  sim_stats->wmpki         = calc_mpki(stats[0].write_misses[0], count_i_total);
  sim_stats->d_e_mpki      = calc_mpki(stats[0].dirty_evicts[0], count_i_total); 
  
  sim_stats_inst->read_hitrt    = calc_rt(stats[0].read_hits_inst[0], (stats[0].read_hits_inst[0]+stats[0].read_misses_inst[0]) );
  sim_stats_inst->write_hitrt   = calc_rt(stats[0].write_hits_inst[0], (stats[0].write_hits_inst[0]+stats[0].write_misses_inst[0]) );
  sim_stats_inst->HITRT         = calc_rt(stats[0].read_hits_inst[0] + stats[0].write_hits_inst[0] ,  
                                          (stats[0].read_hits_inst[0]+stats[0].read_misses_inst[0]) + (stats[0].write_hits_inst[0]+stats[0].write_misses_inst[0]) );
  
  sim_stats_inst->rmpki         = calc_mpki(stats[0].read_misses_inst[0], count_i_inst_total);
  sim_stats_inst->wmpki         = calc_mpki(stats[0].write_misses_inst[0], count_i_inst_total);
  sim_stats_inst->d_e_mpki      = calc_mpki(stats[0].dirty_evicts_inst[0], count_i_inst_total);

}


void output_cache_stats (char* cache_t, cache_stats* stats, cache_sim_stats*  sim_stats, int is_L3){

  printf("USIMM_%s_RDHITS	  \t : %lld\n", cache_t, stats->read_hits[0]);
  printf("USIMM_%s_RDMISS	  \t : %lld\n", cache_t, stats->read_misses[0]);
  printf("USIMM_%s_WRHITS	  \t : %lld\n", cache_t, stats->write_hits[0]);
  printf("USIMM_%s_WRMISS	  \t : %lld\n", cache_t, stats->write_misses[0]);
  printf("USIMM_%s_D_EVICTS	  \t : %lld\n", cache_t, stats->dirty_evicts[0]);


  printf("USIMM_%s_RD_HITRT       \t : %f\n", cache_t,sim_stats->read_hitrt);
  printf("USIMM_%s_WR_HITRT       \t : %f\n", cache_t,sim_stats->write_hitrt);
  printf("USIMM_%s_TOT_HITRT       \t : %f\n", cache_t,sim_stats->HITRT);
  /*
  printf("USIMM_%s_RMPKI       \t : %f\n", cache_t, sim_stats->rmpki);     
  printf("USIMM_%s_WMPKI       \t : %f\n", cache_t,sim_stats->wmpki);     
  printf("USIMM_%s_D-EVT-MPKI       \t : %f\n", cache_t,sim_stats->d_e_mpki);       
  */
  printf("\n");

}


void output_cache_occ (char* cache_t, cache_sim_stats* sim_stats){

  printf("USIMM_%s_DATA_PERC       \t : %f\n",cache_t, sim_stats->data_perc);
  printf("USIMM_%s_CTR_PERC       \t : %f\n",cache_t, sim_stats->ctr_perc);
  printf("USIMM_%s_MTREE_PERC       \t : %f\n",cache_t,sim_stats->mtree_perc);
  printf("USIMM_%s_VALID_PERC       \t : %f\n",cache_t,sim_stats->valid_perc);

  if(DETAILED_MET_STATS){   
    for(int i=mem_org->num_Mtree_levels-1; i>0; i--){
      printf("USIMM_%s_MTREE_%d_PERC   \t : %f\n",cache_t,i,sim_stats->mtree_perc_det[i]); 
    }
  }  
}

//Prints the stats in order of parameters to File - ensure headers in this order in main.c
void print_cache_stats (FILE* Policyfile, cache_sim_stats* sim_stats, cache_sim_stats*  sim_stats_inst, int is_L3){

  
  if(DETAILED_INST_STATS){fprintf(Policyfile,"%f\t",sim_stats_inst->read_hitrt);}
  fprintf(Policyfile,"%f\t",sim_stats->read_hitrt);

  if(DETAILED_CACHE_STATS || is_L3){
    if(DETAILED_INST_STATS){fprintf(Policyfile,"%f\t",sim_stats_inst->write_hitrt);}
    fprintf(Policyfile,"%f\t",sim_stats->write_hitrt);

    if(DETAILED_INST_STATS){fprintf(Policyfile,"%f\t",sim_stats_inst->HITRT);}
    fprintf(Policyfile,"%f\t",sim_stats->HITRT);
  }
  
  if(DETAILED_INST_STATS){fprintf(Policyfile,"%f\t",sim_stats_inst->rmpki);}
  fprintf(Policyfile,"%f\t",sim_stats->rmpki);     

  if(DETAILED_CACHE_STATS || is_L3){
    
    if(DETAILED_INST_STATS){fprintf(Policyfile,"%f\t",sim_stats_inst->wmpki);}
    fprintf(Policyfile,"%f\t",sim_stats->wmpki);     
  }
  
  if(DETAILED_INST_STATS){fprintf(Policyfile,"%f\t",sim_stats_inst->d_e_mpki);}
  fprintf(Policyfile,"%f\t",sim_stats->d_e_mpki);     
  
}



//Add all the MET Cache Stats into MET_tot Cache Stats
void update_met_stats(cache_stats* MET_tot_Cache_Stats, cache_stats** MET_Cache_Stats, memOrg_t* mem_org){
  MET_tot_Cache_Stats[0].write_hits[0]        = 0;
  MET_tot_Cache_Stats[0].write_misses[0]      = 0;
  MET_tot_Cache_Stats[0].read_hits[0]         = 0;
  MET_tot_Cache_Stats[0].read_misses[0]       = 0;
  MET_tot_Cache_Stats[0].dirty_evicts[0]      = 0;

  MET_tot_Cache_Stats[0].write_hits_inst[0]   = 0;
  MET_tot_Cache_Stats[0].write_misses_inst[0] = 0;
  MET_tot_Cache_Stats[0].read_hits_inst[0]    = 0;
  MET_tot_Cache_Stats[0].read_misses_inst[0]  = 0;
  MET_tot_Cache_Stats[0].dirty_evicts_inst[0] = 0;
    

  for(int i=0;i< mem_org->num_Mtree_levels + 1; i++){
    MET_tot_Cache_Stats[0].write_hits[0]        +=   MET_Cache_Stats[i][0].write_hits[0];       
    MET_tot_Cache_Stats[0].write_misses[0]      +=   MET_Cache_Stats[i][0].write_misses[0];     
    MET_tot_Cache_Stats[0].read_hits[0]         +=   MET_Cache_Stats[i][0].read_hits[0];        
    MET_tot_Cache_Stats[0].read_misses[0]       +=   MET_Cache_Stats[i][0].read_misses[0];      
    MET_tot_Cache_Stats[0].dirty_evicts[0]      +=   MET_Cache_Stats[i][0].dirty_evicts[0];     

    MET_tot_Cache_Stats[0].write_hits_inst[0]   +=   MET_Cache_Stats[i][0].write_hits_inst[0];  
    MET_tot_Cache_Stats[0].write_misses_inst[0] +=   MET_Cache_Stats[i][0].write_misses_inst[0];
    MET_tot_Cache_Stats[0].read_hits_inst[0]    +=   MET_Cache_Stats[i][0].read_hits_inst[0];   
    MET_tot_Cache_Stats[0].read_misses_inst[0]  +=   MET_Cache_Stats[i][0].read_misses_inst[0]; 
    MET_tot_Cache_Stats[0].dirty_evicts_inst[0] +=   MET_Cache_Stats[i][0].dirty_evicts_inst[0];
    
  }
}


void init_read_lat_rob_stat(read_lat_rob_stat* stat, memOrg_t* mem_org){

  stat->met_duration = (uns64*) calloc(mem_org->num_Mtree_levels+1, sizeof(uns64));
  stat->num_mets = (uns64*) calloc(mem_org->num_Mtree_levels+1, sizeof(uns64));
}

void init_read_lat_sim_stat(read_lat_sim_stat* stat, memOrg_t* mem_org){

  stat->met_latency = (uns64*) calloc(mem_org->num_Mtree_levels, sizeof(uns64));
  stat->num_mets = (uns64*) calloc(mem_org->num_Mtree_levels, sizeof(uns64));
}

//Reset read latency rob stat
void reset_read_lat_rob_stat(read_lat_rob_stat* stat, memOrg_t* mem_org){

  stat->start_cycle=0;
  stat->read_issued_cycle=0;
  stat->read_completed_cycle=0;
  stat->end_cycle=0;
  stat->latest_cycle=0;
  
  stat->mac_duration=0;
  stat->num_macs =0;

  stat->num_twins =0;
  stat->twin_duration =0; 
  
  for(int i=0; i<mem_org->num_Mtree_levels+1; i++){
    stat->met_duration[i]=0;
    stat->num_mets[i]=0;
  }
  
}

void output_read_lat_sim_stat(read_lat_sim_stat* sim_stat,  memOrg_t* mem_org){

  printf("USIMM_MEMRD-TOT-LAT    \t : %f\n", ((double)sim_stat->read_latency) /((double) sim_stat->num_reads) ); //latency per read
  printf("USIMM_MEMRD-Q-LAT      \t : %f\n", ((double)sim_stat->queue_latency) /((double) sim_stat->num_reads) ); //queue latency per read
  printf("USIMM_MEMRD-SERV-LAT   \t : %f\n", ((double)sim_stat->act_read_latency) /((double) sim_stat->num_reads) ); //act read latency per read 
  printf("USIMM_MEMRD-TWIN-LAT    \t : %f\n", ((double)sim_stat->twin_latency) /((double) sim_stat->num_reads) ); //twin latency/read
  printf("USIMM_MEMRD-MAC-LAT    \t : %f\n", ((double)sim_stat->mac_latency) /((double) sim_stat->num_reads) ); //mac latency/read
  printf("USIMM_MEMRD-CTR-LAT    \t : %f\n", ((double)sim_stat->ctr_latency) /((double) sim_stat->num_reads) ); //mac latency/read
  printf("USIMM_MEMRD-MTREE-LAT  \t : %f\n", ((double)sim_stat->mtree_tot_latency) /((double) sim_stat->num_reads) ); //mtree_tot latency/read

  printf("\n");
  
  printf("USIMM_TWINS-PR-RD      \t : %f\n", ((double)sim_stat->num_twins) /((double) sim_stat->num_reads) );    //mum_macs/read  
  printf("USIMM_MACS-PR-RD       \t : %f\n", ((double)sim_stat->num_macs) /((double) sim_stat->num_reads) );    //mum_macs/read
  printf("USIMM_CTRS-PR-RD       \t : %f\n", ((double)sim_stat->num_ctrs) /((double) sim_stat->num_reads) );    //mum_macs/read
  printf("USIMM_MTREE-PR-RD      \t : %f\n", ((double)sim_stat->num_mtree_tot) /((double) sim_stat->num_reads) );    //mum_macs/read 

  printf("\n");
  
  if(DETAILED_MET_STATS){
    for(int i=mem_org->num_Mtree_levels-1; i>0; i--){
      printf("USIMM_MEMRD-MTREE_%d-LAT   \t : %f\n",i, ((double)sim_stat->met_latency[i]) /((double) sim_stat->num_reads) ); //mtree latency/read
      printf("USIMM_MTREE_%d-PR-D        \t : %f\n",i, ((double)sim_stat->num_mets[i]) /((double) sim_stat->num_reads) );    //mum_mtree/read
    }
  }

  printf("\n");
  

}


void print_read_lat_sim_stat(FILE* Policyfile, read_lat_sim_stat* sim_stat,  memOrg_t* mem_org){
  fprintf(Policyfile,"%f\t", ((double)sim_stat->read_latency) /((double) sim_stat->num_reads) ); //latency per read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->queue_latency) /((double) sim_stat->num_reads) ); //queue latency per read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->act_read_latency) /((double) sim_stat->num_reads) ); //act read latency per read

  fprintf(Policyfile,"%f\t", ((double)sim_stat->twin_latency) /((double) sim_stat->num_reads) ); //mac latency/read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->mac_latency) /((double) sim_stat->num_reads) ); //mac latency/read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->ctr_latency) /((double) sim_stat->num_reads) ); //mac latency/read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->mtree_tot_latency) /((double) sim_stat->num_reads) ); //mtree_tot latency/read

  fprintf(Policyfile,"%f\t", ((double)sim_stat->num_twins) /((double) sim_stat->num_reads) );    //mum_macs/read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->num_macs) /((double) sim_stat->num_reads) );    //mum_macs/read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->num_ctrs) /((double) sim_stat->num_reads) );    //mum_macs/read
  fprintf(Policyfile,"%f\t", ((double)sim_stat->num_mtree_tot) /((double) sim_stat->num_reads) );    //mum_macs/read

  if(DETAILED_MET_STATS){
    for(int i=mem_org->num_Mtree_levels-1; i>0; i--){
      fprintf(Policyfile,"%f\t", ((double)sim_stat->met_latency[i]) /((double) sim_stat->num_reads) ); //mtree latency/read
      fprintf(Policyfile,"%f\t", ((double)sim_stat->num_mets[i]) /((double) sim_stat->num_reads) );    //mum_mtree/read
    }
  }
}


void lat_stat_start(read_lat_rob_stat* rob_stat, long long int cycle){
  rob_stat-> start_cycle = cycle;
}

void lat_stat_process(read_lat_sim_stat* sim_stat, read_lat_rob_stat* rob_stat, maccess_type type, long long int curr_cycle, long long int final_cycle, int num_critical, memOrg_t* mem_org){

  if(type.memtype == DATA ){
    rob_stat->read_issued_cycle = curr_cycle;
    rob_stat->read_completed_cycle = final_cycle; 
  }

  else if(type.memtype == MAC){
    if( (final_cycle > rob_stat->read_completed_cycle) && (rob_stat->read_completed_cycle != 0) ){
      rob_stat->mac_duration = final_cycle - rob_stat->latest_cycle;
      rob_stat->num_macs++;
    }
  }
  else if(type.memtype == MET){
    if( (final_cycle > rob_stat->read_completed_cycle) && (rob_stat->read_completed_cycle != 0) ){
      rob_stat->met_duration[type.mtree_level] = final_cycle - rob_stat->latest_cycle;
      rob_stat->num_mets[type.mtree_level]++; 
    }
  }
  else if(type.memtype == TWIN){
    if( (final_cycle > rob_stat->read_completed_cycle) && (rob_stat->read_completed_cycle != 0) ){
      rob_stat->twin_duration = final_cycle - rob_stat->latest_cycle;
         rob_stat->num_twins++;
    }
  } 
  else{
    ASSERTM(0, "Bad things happened in lat_process");
  }

  rob_stat->latest_cycle = final_cycle;

  if(num_critical == 0){
    rob_stat->end_cycle = final_cycle;
    lat_stat_end(sim_stat, rob_stat, mem_org);
  }
    
}


void lat_stat_end(read_lat_sim_stat* sim_stat, read_lat_rob_stat* rob_stat, memOrg_t* mem_org){
  sim_stat->num_reads++;
  sim_stat->num_macs += rob_stat->num_macs;
  
  sim_stat->read_latency += rob_stat->end_cycle - rob_stat->start_cycle;
  sim_stat->queue_latency += rob_stat->read_issued_cycle - rob_stat->start_cycle;
  sim_stat->act_read_latency += rob_stat->read_completed_cycle - rob_stat->read_issued_cycle;
  sim_stat->mac_latency += rob_stat->mac_duration;

  for(int i=0; i <mem_org->num_Mtree_levels;i++ ){
    sim_stat->num_mets[i]+=rob_stat->num_mets[i];
    sim_stat->met_latency[i]+=rob_stat->met_duration[i];

    sim_stat->num_mtree_tot += rob_stat->num_mets[i];
    sim_stat->mtree_tot_latency += rob_stat->met_duration[i];
  }

  sim_stat->num_ctrs += rob_stat->num_mets[mem_org->num_Mtree_levels];
  sim_stat->ctr_latency += rob_stat->met_duration[mem_org->num_Mtree_levels];

  sim_stat->num_twins += rob_stat->num_twins;
  sim_stat->twin_latency += rob_stat->twin_duration;

  //Reset ROB rob_stat
  reset_read_lat_rob_stat(rob_stat,mem_org);
  
}

//insert_write, insert_read


mem_stats_t* init_mem_stats (int num_mtree){
  
  mem_stats_t* new_mem_stats = (mem_stats_t*) malloc(sizeof(mem_stats_t));
  new_mem_stats->twin = 0;

  new_mem_stats->data_w = 0;
  new_mem_stats->ctr_w = 0;
  new_mem_stats->mac_w = 0;

  new_mem_stats->data_r = 0;
  new_mem_stats->ctr_r = 0;
  new_mem_stats->mac_r = 0;

  new_mem_stats->met_w = (unsigned long long int*) calloc(num_mtree, sizeof(unsigned long long int));  
  new_mem_stats->met_r = (unsigned long long int*) calloc(num_mtree, sizeof(unsigned long long int));  

  return new_mem_stats;
 
}

void zero_mem_stats (int num_mtree, mem_stats_t* mem_stats){
  mem_stats->twin = 0;

  mem_stats->ctr_r = 0;
  mem_stats->data_r = 0;
  mem_stats->mac_r = 0;

  mem_stats->ctr_w = 0;
  mem_stats->data_w = 0;
  mem_stats->mac_w = 0;

  for(int i=0;i<num_mtree;i++){
    mem_stats->met_r[i] = 0;
    mem_stats->met_w[i] = 0; 
  }
}

void update_mem_stats(mem_stats_t* mem_stats, maccess_type type, int r_w, ADDR memaccess_paddr, int thread_id){
  if(r_w == 0){ //read
    switch(type.memtype){
    case DATA:
      mem_stats->data_r++;
      if(SGX_MODE[thread_id] >=1){
        update_ctr_cls_rw_wrapper(mem_org,ctr_cls, ctr_types,getCounterAddr(memaccess_paddr, mem_org).paddr,  mem_org->num_Mtree_levels, r_w+2);
      }
      break;
    case MAC:
      mem_stats->mac_r++;
      break;
    case TWIN:
      mem_stats->twin++;
      break;
    case MET:
      if(type.mtree_level ==  mem_org->num_Mtree_levels){
        mem_stats->ctr_r++;
        update_ctr_cls_rw_wrapper(mem_org,ctr_cls, ctr_types,memaccess_paddr, type.mtree_level, r_w);
      }
      else
        mem_stats->met_r[type.mtree_level]++;
      break;
    default:
      ASSERTM(0,"SOMETHING's WRONG\n");
    } 
  }
  else { //write
    switch(type.memtype){
    case DATA:
      mem_stats->data_w++;
      if(SGX_MODE[thread_id] >=1){
        update_ctr_cls_rw_wrapper(mem_org,ctr_cls, ctr_types,getCounterAddr(memaccess_paddr, mem_org).paddr, mem_org->num_Mtree_levels, r_w+2);
      }
      break;
    case MAC:
      mem_stats->mac_w++;
      break;
    case TWIN:
      mem_stats->twin++;
      break;
    case MET:
      if(type.mtree_level ==  mem_org->num_Mtree_levels){
        mem_stats->ctr_w++;
        update_ctr_cls_rw_wrapper(mem_org,ctr_cls, ctr_types,memaccess_paddr, type.mtree_level,r_w);
      }
      else
        mem_stats->met_w[type.mtree_level]++;
      break;

    default:
      ASSERTM(0,"SOMETHING's WRONG\n");
    }
  }
  
}

void print_mem_stats(FILE* Policyfile, mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org){
  
  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->data_r)) / ((double) inst));    //mum_macs/read
  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->data_w)) / ((double) inst));    //mum_macs/read

  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->mac_r)) / ((double) inst));    //mum_macs/read
  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->mac_w)) / ((double) inst));    //mum_macs/read

  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->ctr_r)) / ((double) inst));    //mum_macs/read
  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->ctr_w)) / ((double) inst));    //mum_macs/read

  for (int i=0;i<mem_org->num_Mtree_levels;i++){
    fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->met_r[i])) / ((double) inst));    //mum_macs/read
    fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->met_w[i])) / ((double) inst));    //mum_macs/read
  }

  fprintf(Policyfile,"%f\t", 1000*((double)(mem_stats->twin)) / ((double) inst));    //mum_macs/read

  //Print occupancy - TODO
  
}

void output_mem_stats(mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org){
  unsigned long long int mtree_rmpki = 0, mtree_wmpki = 0;
  for (int i=0;i<mem_org->num_Mtree_levels;i++){
    mtree_rmpki += mem_stats->met_r[i];
    mtree_wmpki += mem_stats->met_w[i]; 
  }
  
  printf ("\n****MEM TRAFFIC STATS ********\n");
  printf("USIMM_NEW_DATA_MPKI   \t : %f\n", 1000*((double)(mem_stats->data_r+mem_stats->data_w)) / ((double) inst) ); 
  printf("USIMM_NEW_MAC_MPKI   \t : %f\n", 1000*((double)(mem_stats->mac_r + mem_stats->mac_w) ) / ((double) inst) ); 
  printf("USIMM_NEW_CTR_MPKI   \t : %f\n", 1000*((double) (mem_stats->ctr_r + mem_stats->ctr_w) ) / ((double) inst) ); 
  printf("USIMM_NEW_MTREE_MPKI   \t : %f\n", 1000*((double) (mtree_rmpki+mtree_wmpki)) / ((double) inst) ); 
  printf("USIMM_NEW_TWIN_MPKI   \t : %f\n", 1000*((double) (mem_stats->twin) ) / ((double) inst) ); 
  
  printf ("\n************DETAILED ***********\n");
  
  printf("USIMM_NEW_DATA_RMPKI   \t : %f\n", 1000*((double)mem_stats->data_r) / ((double) inst) ); 
  printf("USIMM_NEW_DATA_WMPKI   \t : %f\n", 1000*((double)mem_stats->data_w) / ((double) inst) ); 

  printf("USIMM_NEW_MAC_RMPKI   \t : %f\n", 1000*((double)mem_stats->mac_r) / ((double) inst) ); 
  printf("USIMM_NEW_MAC_WMPKI   \t : %f\n", 1000*((double)mem_stats->mac_w) / ((double) inst) ); 

  printf("USIMM_NEW_CTR_RMPKI   \t : %f\n", 1000*((double)mem_stats->ctr_r) / ((double) inst) ); 
  printf("USIMM_NEW_CTR_WMPKI   \t : %f\n", 1000*((double)mem_stats->ctr_w) / ((double) inst) ); 

  printf("USIMM_NEW_MTREE_RMPKI   \t : %f\n", 1000*((double)mtree_rmpki) / ((double) inst) ); 
  printf("USIMM_NEW_MTREE_WMPKI   \t : %f\n", 1000*((double)mtree_wmpki) / ((double) inst) ); 

  
  for (int i=0;i<mem_org->num_Mtree_levels;i++){
    printf("USIMM_NEW_MTREE_%d_RMPKI   \t : %f\n",i, 1000*((double)mem_stats->met_r[i]) / ((double) inst) ); 
    printf("USIMM_NEW_MTREE_%d_WMPKI   \t : %f\n",i, 1000*((double)mem_stats->met_w[i]) / ((double) inst) ); 
  }

  
  for (int i=0;i<mem_org->num_Mtree_levels;i++){
    printf("USIMM_UPDATED_MTREE_MPKI_%d   \t : %f\n",(mem_org->num_Mtree_levels -1 - i), 1000*((double) (mem_stats->met_r[i] + mem_stats->met_w[i]) ) / ((double) inst) ); 
  }
  
}

double return_data_mpki(mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org){
  return ((double)(1000*((double)(mem_stats->data_r+mem_stats->data_w)) / ((double) inst) ));  
}


double return_non_overflow_mpki(mem_stats_t* mem_stats, unsigned long long int inst, memOrg_t* mem_org){
  double tot_mpki = 0;
  unsigned long long int mtree_rmpki = 0, mtree_wmpki = 0;
  for (int i=0;i<mem_org->num_Mtree_levels;i++){
    mtree_rmpki += mem_stats->met_r[i];
    mtree_wmpki += mem_stats->met_w[i]; 
  }

  tot_mpki += (double)(  1000*((double)(mem_stats->data_r+mem_stats->data_w)) / ((double) inst)); //data
  tot_mpki += (double)(  1000*((double)(mem_stats->mac_r+mem_stats->mac_w)) / ((double) inst)); //mac
  tot_mpki += (double)(  1000*((double)(mem_stats->ctr_r+mem_stats->ctr_w)) / ((double) inst)); //ctr
  tot_mpki += (double)(  1000*((double) (mtree_rmpki+mtree_wmpki)) / ((double) inst) ); //mtree

  return tot_mpki;
  
}


void calc_cache_occ (MCache** L3Cache, cache_sim_stats* sim_stats){


//Initialize met multiple levels
  sim_stats->mtree_perc_det = calloc(mem_org->num_Mtree_levels, sizeof(double));
  sim_stats->data_perc = 0;
  sim_stats->ctr_perc = 0;
  sim_stats->mtree_perc = 0;
  uns64 tot_valid = 0;

  MCache* Cache = L3Cache[0];


  //Read each entry in cache
  for(uns64 entry_id = 0; entry_id < (uns64) (Cache->sets*Cache->assocs); entry_id++){    

    if( (Cache->entries[entry_id]).valid ){
      tot_valid++;
      
      uns64 paddr = (Cache->entries[entry_id].tag) << 6;
      int entry_type = get_partition(paddr, mem_org);
      
      //Check partition and update the counters in cache_sim_stats
      
      if(entry_type == 0){
	sim_stats->data_perc++;    
      }
      else if(entry_type == 2){
	sim_stats->ctr_perc++;
      }
      else if(entry_type == 1){
	sim_stats->mtree_perc++;      
	
	//If metadata, then check the level of its parent and add 1 to it and index the stat appropriate and increment
	if(DETAILED_MET_STATS){
	  ctr_mtree_entry met = getMtreeEvictParent(paddr, mem_org);
	  sim_stats->mtree_perc_det[met.mtree_level+1]++;
	}
	
      }
      else {
	ASSERTM(0,"Cache contents polluted !\n");
      }    
    }

  }
  

  //Divide each stat by overall entries in cache
  ASSERTM(tot_valid != 0, "Cache should have some valid lines\n");

  sim_stats->data_perc  = sim_stats->data_perc / (tot_valid) *1.0; 
  sim_stats->ctr_perc  = sim_stats->ctr_perc / (tot_valid) *1.0; 
  sim_stats->mtree_perc  = sim_stats->mtree_perc / (tot_valid) *1.0; 

  for(int i=0;i< mem_org->num_Mtree_levels ; i++){
    sim_stats->mtree_perc_det[i]= sim_stats->mtree_perc_det[i]/(tot_valid) *1.0; 
  }

  ASSERTM((Cache->sets*Cache->assocs) != 0, "Cache should non-zero number of entries\n");
  sim_stats->valid_perc = tot_valid / (Cache->sets*Cache->assocs) ;
  
}

uns64 calc_fast_cycles(mem_stats_t* mem_stats, uns64  count_i_total,memOrg_t* mem_org, int exc_overflow_access){
 
  uns64 mem_accesses = 0, non_mem_cycles = 0, mem_cycles = 0, total_cycles =0;
  unsigned long long int mtree_rmpki = 0, mtree_wmpki = 0;
  for (int i=0;i<mem_org->num_Mtree_levels;i++){
    mtree_rmpki += mem_stats->met_r[i];
    mtree_wmpki += mem_stats->met_w[i]; 
  }

  mem_accesses += (mem_stats->data_r+mem_stats->data_w);
  mem_accesses += (mem_stats->mac_r + mem_stats->mac_w);
  mem_accesses += (mem_stats->ctr_r + mem_stats->ctr_w);
  mem_accesses += (mtree_rmpki+mtree_wmpki) ;

  if(exc_overflow_access == 0){
    mem_accesses +=  (mem_stats->twin);    
  }

  non_mem_cycles = count_i_total / (NUMCORES * MAX_FETCH) ;
  mem_cycles = ( mem_accesses * (MEM_LATENCY) );

  if( ((double)non_mem_cycles) > ( ((double)((NUMCORES) - 1))*((double)(mem_cycles))/((double)(NUMCORES)) ) )
    total_cycles = non_mem_cycles + (mem_cycles / (NUMCORES));
  else {
    total_cycles = mem_cycles;
  }
    
  return total_cycles;
}

void log_ctr_overflow_vs_inst(FILE* fileh, uns64 count_500mn_fast_slow, memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_inst){

  if(count_500mn_fast_slow == 0){
    //First Call - Log Header
    fprintf(fileh,"Count_500Mn\t");

    //Overflows (Total and Level-Wise)    
    //Inst
    fprintf(fileh,"All_O_Inst\t");
    fprintf(fileh,"Ctr_O_Inst\t");
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){     
      fprintf(fileh,"Parent_%d_O_Inst\t",(mem_org->num_Mtree_levels -1 - i) );
    }
    //Cumu
    fprintf(fileh,"All_O_Cumu\t");
    fprintf(fileh,"Ctr_O_Cumu\t");
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){     
      fprintf(fileh,"Parent_%d_O_Cumu\t",(mem_org->num_Mtree_levels -1 - i) );
    }        
    //Child Accesses
    //Inst
    fprintf(fileh,"All_CA_Inst\t");
    fprintf(fileh,"Ctr_CA_Inst\t");
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){     
      fprintf(fileh,"Parent_%d_CA_Inst\t",(mem_org->num_Mtree_levels -1 - i) );
    }
    //Cumu
    fprintf(fileh,"All_CA_Cumu\t");
    fprintf(fileh,"Ctr_CA_Cumu\t");
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){     
      fprintf(fileh,"Parent_%d_CA_Cumu\t",(mem_org->num_Mtree_levels -1 - i) );
    }           
    fprintf(fileh,"\n");    
  }
  else {
    //Log Stats
    overflows_stat all_levels_cumu, all_levels_inst;
    all_levels_cumu.overflows =0;
    all_levels_cumu.child_accesses =0;
    all_levels_inst.overflows =0;
    all_levels_inst.child_accesses =0;
    for(int i=0; i<=mem_org->num_Mtree_levels;i++){
      all_levels_cumu.overflows += ctr_overflows_levelwise[i].overflows;
      all_levels_cumu.child_accesses += ctr_overflows_levelwise[i].child_accesses;
      all_levels_inst.overflows += ctr_overflows_levelwise_inst[i].overflows;
      all_levels_inst.child_accesses  += ctr_overflows_levelwise_inst[i].child_accesses;
    }

    //First Call - Log Header
    fprintf(fileh,"%lld\t",count_500mn_fast_slow);    
    
    //Overflows (Total and Level-Wise)    
    //Inst
    fprintf(fileh,"%lld\t",all_levels_inst.overflows);
    fprintf(fileh,"%lld\t",ctr_overflows_levelwise_inst[mem_org->num_Mtree_levels].overflows);
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){
      fprintf(fileh,"%lld\t",ctr_overflows_levelwise_inst[i].overflows);
    }    
    //Cumu
    fprintf(fileh,"%lld\t",all_levels_cumu.overflows);
    fprintf(fileh,"%lld\t",ctr_overflows_levelwise[mem_org->num_Mtree_levels].overflows);
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){
      fprintf(fileh,"%lld\t",ctr_overflows_levelwise[i].overflows);
    }    

    //Child Accesses
    //Inst
    fprintf(fileh,"%lld\t",all_levels_inst.child_accesses);
    fprintf(fileh,"%lld\t",ctr_overflows_levelwise_inst[mem_org->num_Mtree_levels].child_accesses);
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){
      fprintf(fileh,"%lld\t",ctr_overflows_levelwise_inst[i].child_accesses);
    }    
    //Cumu
    fprintf(fileh,"%lld\t",all_levels_cumu.child_accesses);
    fprintf(fileh,"%lld\t",ctr_overflows_levelwise[mem_org->num_Mtree_levels].child_accesses);
    for(int i=mem_org->num_Mtree_levels-1; i>=0; i--){
      fprintf(fileh,"%lld\t",ctr_overflows_levelwise[i].child_accesses);
    }    
    
    fprintf(fileh,"\n");

    //Reset All the Inst Counts
    for(int i=mem_org->num_Mtree_levels; i>=0; i--){
      ctr_overflows_levelwise_inst[i].overflows = 0;
      ctr_overflows_levelwise_inst[i].child_accesses = 0;
    }    
  }
}

void print_stat_overflow_newfinal(memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_warmup, uns64 total_inst, uns64 data_mpki, uns64 non_overflow_mpki ){

  printf("*** CTR_OVERFLOWS_NEWFINAL STATS***\n");

  //Log Stats
  overflows_stat all_levels_cumu, all_levels_inst;
  double overflows_per_bn_total=0, child_accesses_per_bn_total = 0;
  
  all_levels_cumu.overflows =0;
  all_levels_cumu.child_accesses =0;
  for(int i=mem_org->num_Mtree_levels; i >= 0; i--){

    printf("CUMU_OVERFLOWS_CTR_FINLEVEL_%d         \t: %llu\n",mem_org->num_Mtree_levels - i,(ctr_overflows_levelwise[i].overflows - ctr_overflows_levelwise_warmup[i].overflows ));
    printf("CUMU_CHILDACCESES_CTR_FINLEVEL_%d         \t: %llu\n\n",mem_org->num_Mtree_levels - i,(ctr_overflows_levelwise[i].child_accesses - ctr_overflows_levelwise_warmup[i].child_accesses));

    all_levels_cumu.overflows += (ctr_overflows_levelwise[i].overflows - ctr_overflows_levelwise_warmup[i].overflows) ;
    all_levels_cumu.child_accesses += (ctr_overflows_levelwise[i].child_accesses - ctr_overflows_levelwise_warmup[i].child_accesses);
  }

  printf("CUMU_OVERFLOWS_CTR_OVERALL         \t: %llu\n",all_levels_cumu.overflows);
  printf("CUMU_CHILDACCESES_CTR_OVERALL      \t: %llu\n\n",all_levels_cumu.child_accesses);

  //Normalized Overflow Numbers:
  for(int i=mem_org->num_Mtree_levels; i >= 0; i--){
    double overflows_per_bn =  ((double)(ctr_overflows_levelwise[i].overflows - ctr_overflows_levelwise_warmup[i].overflows )/(double)total_inst)*(double)1000000000 ;
    double child_accesses_per_bn =  ((double)(ctr_overflows_levelwise[i].child_accesses - ctr_overflows_levelwise_warmup[i].child_accesses )/(double)total_inst)*(double)1000000000 ;
    

    printf("CUMU_OVERFLOWS_PER_BNINST_CTR_FINLEVEL_%d         \t: %.3f\n",mem_org->num_Mtree_levels - i,overflows_per_bn);
    printf("CUMU_CHILD_ACCESSES_PER_BNINST_CTR_FINLEVEL_%d         \t: %.3f\n",mem_org->num_Mtree_levels - i,child_accesses_per_bn);
    
  }

  overflows_per_bn_total =  ((double)(all_levels_cumu.overflows)/(double)total_inst)*(double)1000000000 ;
  child_accesses_per_bn_total =  ((double)(all_levels_cumu.child_accesses )/(double)total_inst)*(double)1000000000 ;
  
  printf("CUMU_OVERFLOWS_PER_BNINST_CTR_OVERALL         \t: %.3f\n",overflows_per_bn_total);
  printf("CUMU_CHILDACCESES_PER_BININST_CTR_OVERALL      \t: %.3f\n\n",child_accesses_per_bn_total);
  printf("CUMU_OVERFLOWS_PER_MN_DATMEMACCESS_CTR_OVERALL         \t: %.3f\n",(double)overflows_per_bn_total /(double) data_mpki);
  printf("CUMU_OVERFLOWS_PER_MN_TOTMEMACCESS_CTR_OVERALL         \t: %.3f\n",(double)overflows_per_bn_total /(double) non_overflow_mpki);
  
}

void timestamp_cumuoverflows_warmup(memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise, overflows_stat* ctr_overflows_levelwise_warmup){


  for(int i=mem_org->num_Mtree_levels; i >= 0; i--){

    ctr_overflows_levelwise_warmup[i].overflows      = ctr_overflows_levelwise[i].overflows;
    ctr_overflows_levelwise_warmup[i].child_accesses = ctr_overflows_levelwise[i].child_accesses;
  }
  
}
