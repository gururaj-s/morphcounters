#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "params.h"
#include "processor.h"
#include "memOrg.h" //Added by Gururaj
#include "stats.h"

// ROB Structure, used to release stall on instructions 
// when the read request completes
extern struct robstructure * ROB;
//Sim Stats - Added by Gururaj
extern read_lat_sim_stat* lat_sim_stat;
extern memOrg_t*          mem_org;
extern long long int BIGNUM ;
long long int add_encr_delay;


// Current Processor Cycle
extern long long int CYCLE_VAL;
extern long long int *CYCLE_VAL_CORE;
extern int *core_mod;

//***************** Extern Variables for Memory Activity Logging *******************************
extern long long int *memory_rd_inst;
extern long long int *memory_rd;
extern long long int *memory_wr_inst;
extern long long int *memory_wr;
extern long long int *memory_req_inst;
extern long long int *memory_req;
  
extern long long int *memory_rdqueue_inst;
extern long long int *memory_rdqueue;
extern long long int *memory_wrqueue_inst;
extern long long int *memory_wrqueue;
extern long long int *memory_reqqueue_inst;
extern long long int *memory_reqqueue;

extern long long int *memory_rdhit_inst;
extern long long int *memory_rdhit;
extern long long int *memory_wrhit_inst;
extern long long int *memory_wrhit;
extern long long int *memory_reqhit_inst;
extern long long int *memory_reqhit;

extern long long int *memory_rd_ch_inst;
extern long long int *memory_rd_ch;
extern long long int *memory_wr_ch_inst;
extern long long int *memory_wr_ch;
extern long long int *memory_req_ch_inst;
extern long long int *memory_req_ch;

extern long long int *memory_rdqueue_ch_inst;
extern long long int *memory_rdqueue_ch;
extern long long int *memory_wrqueue_ch_inst;
extern long long int *memory_wrqueue_ch;
extern long long int *memory_reqqueue_ch_inst;
extern long long int *memory_reqqueue_ch;

extern long long int *memory_rdhit_ch_inst;
extern long long int *memory_rdhit_ch;
extern long long int *memory_wrhit_ch_inst;
extern long long int *memory_wrhit_ch;
extern long long int *memory_reqhit_ch_inst;
extern long long int *memory_reqhit_ch;

extern long long int memory_rd_total_inst;
extern long long int memory_rd_total;
extern long long int memory_wr_total_inst;
extern long long int memory_wr_total;
extern long long int memory_req_total_inst;
extern long long int memory_req_total;

extern long long int memory_rdqueue_total_inst;
extern long long int memory_rdqueue_total;
extern long long int memory_wrqueue_total_inst;
extern long long int memory_wrqueue_total;
extern long long int memory_reqqueue_total_inst;
extern long long int memory_reqqueue_total;

extern long long int memory_rdhit_total_inst;
extern long long int memory_rdhit_total;
extern long long int memory_wrhit_total_inst;
extern long long int memory_wrhit_total;
extern long long int memory_reqhit_total_inst;
extern long long int memory_reqhit_total;

//****************************************************************************************

/* A basic FCFS policy augmented with a not-so-clever close-page policy.
   If the memory controller is unable to issue a command this cycle, find
   a bank that recently serviced a column-rd/wr and close it (precharge it). */


/* Make a scoreboard structure that keeps track of when the resource is free */

typedef struct 
{
    unsigned long long int service_time;
    long long int opened_row;
} scoreboard;

/* Make a scoreboard structure for channels */
typedef struct
{
    unsigned long long int end_time;
} scoreboard_ch;

scoreboard scoreboard_t[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
scoreboard_ch scoreboard_ch_t[MAX_NUM_CHANNELS];

void init_scheduler_vars()
{
	// initialize all scheduler variables here
	int i, j, k;
	for (i=0; i<MAX_NUM_CHANNELS; i++) {
	  for (j=0; j<MAX_NUM_RANKS; j++) {
	    for (k=0; k<MAX_NUM_BANKS; k++) {
          scoreboard_t[i][j][k].service_time = 0;
          scoreboard_t[i][j][k].opened_row = -1;
	    }
	  }
      scoreboard_ch_t[i].end_time = 0;
	}

	return;
}

// write queue high water mark; begin draining writes if write queue exceeds this value
#define HI_WM 40

// end write queue drain once write queue has this many writes in it
#define LO_WM 20

// 1 means we are in write-drain mode for that channel
int drain_writes[MAX_NUM_CHANNELS];

void schedule(int channel)
{
	request_t * rd_ptr = NULL;
	request_t * wr_ptr = NULL;


	// if in write drain mode, keep draining writes until the
	// write queue occupancy drops to LO_WM
	if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
	  drain_writes[channel] = 1; // Keep draining.
	}
	else {
	  drain_writes[channel] = 0; // No need to drain.
	}

	// initiate write drain if either the write queue occupancy
	// has reached the HI_WM , OR, if there are no pending read
	// requests
	if(write_queue_length[channel] > HI_WM)
	{
		drain_writes[channel] = 1;
	}
	else {
	  if (!read_queue_length[channel])
	    drain_writes[channel] = 1;
	}


	// If in write drain mode, look through all the write queue
	// elements (already arranged in the order of arrival), and
	// issue the command for the first request that is ready
	if(drain_writes[channel])
	{

		LL_FOREACH(write_queue_head[channel], wr_ptr)
		{
            if(wr_ptr->command_issuable == 3){
                if(CYCLE_VAL >= wr_ptr->channel_time){
	                wr_ptr->completion_time = CYCLE_VAL + T_WR;
			        wr_ptr->latency = wr_ptr->completion_time - wr_ptr->arrival_time;
                    wr_ptr->request_served = 1;
                    
                    //Update the memory loggers
                    memory_wr_inst[wr_ptr->thread_id]++;
                    memory_wr[wr_ptr->thread_id]++;
                    memory_req_inst[wr_ptr->thread_id]++;
                    memory_req[wr_ptr->thread_id]++;

                    memory_wr_ch_inst[channel]++;
                    memory_wr_ch[channel]++;
                    memory_req_ch_inst[channel]++;
                    memory_req_ch[channel]++;

                    memory_wr_total_inst++;
                    memory_wr_total++;
                    memory_req_total_inst++;
                    memory_req_total++;

                    memory_wrqueue_inst[wr_ptr->thread_id] = wr_ptr->latency + memory_wrqueue_inst[wr_ptr->thread_id];
                    memory_wrqueue[wr_ptr->thread_id] = wr_ptr->latency + memory_wrqueue[wr_ptr->thread_id];
                    memory_reqqueue_inst[wr_ptr->thread_id] = wr_ptr->latency + memory_reqqueue_inst[wr_ptr->thread_id];
                    memory_reqqueue[wr_ptr->thread_id] = wr_ptr->latency + memory_reqqueue[wr_ptr->thread_id];	

                    memory_wrqueue_ch_inst[channel] = wr_ptr->latency + memory_wrqueue_ch_inst[channel];
                    memory_wrqueue_ch[channel] = wr_ptr->latency + memory_wrqueue_ch[channel];
                    memory_reqqueue_ch_inst[channel] = wr_ptr->latency + memory_reqqueue_ch_inst[channel];
                    memory_reqqueue_ch[channel] = wr_ptr->latency + memory_reqqueue_ch[channel];	

                    memory_wrqueue_total_inst = wr_ptr->latency + memory_wrqueue_total_inst;
                    memory_wrqueue_total = wr_ptr->latency + memory_wrqueue_total;
                    memory_reqqueue_total_inst = wr_ptr->latency + memory_reqqueue_total_inst;
                    memory_reqqueue_total = wr_ptr->latency + memory_reqqueue_total;

                    stats_writes_completed[channel]++;

                    stats_num_write[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]++;
                    
                    stats_average_write_latency[channel] = ((stats_writes_completed[channel]-1)*stats_average_write_latency[channel] + wr_ptr->latency)/stats_writes_completed[channel];
                    stats_average_write_queue_latency[channel] = ((stats_writes_completed[channel]-1)*stats_average_write_queue_latency[channel] + (wr_ptr->dispatch_time - wr_ptr->arrival_time))/stats_writes_completed[channel];

                    for(int i=0; i<NUM_RANKS ;i++)
                    {
                        if(i!=wr_ptr->dram_addr.rank)
                        stats_time_spent_terminating_writes_to_other_ranks[channel][i] += T_DATA_TRANS;
                    }
                }
            }

            if(wr_ptr->command_issuable == 2){
                // Check if the time for executing these commands is over
                if(CYCLE_VAL >= wr_ptr->bank_time){
                    wr_ptr->command_issuable =3;
                    wr_ptr->channel_time = scoreboard_ch_t[channel].end_time + T_DATA_TRANS;
                    scoreboard_ch_t[channel].end_time += T_DATA_TRANS;
                }
            }

            if(wr_ptr->command_issuable == 1){
                wr_ptr->command_issuable=2;
		        wr_ptr->dispatch_time = CYCLE_VAL;
                //Check if the row is open
                if(scoreboard_t[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].opened_row == wr_ptr->dram_addr.row){
                    wr_ptr->bank_time = scoreboard_t[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].service_time + T_CCD;
                    scoreboard_t[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].service_time += T_CCD; 
                }
                else{
        			stats_num_activate[channel][wr_ptr->dram_addr.rank]++;
	                stats_num_activate_write[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]++;
				    stats_num_activate_spec[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]++;
                    stats_num_precharge[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]++;
                    //Memory logger for write activates
                    memory_wrhit_inst[wr_ptr->thread_id]++;
                    memory_wrhit[wr_ptr->thread_id]++;
                    memory_wrhit_ch_inst[channel]++;
                    memory_wrhit_ch[channel]++;
                    memory_wrhit_total_inst++;
                    memory_wrhit_total++;

                    // Memory Req Activates
                    memory_reqhit_inst[wr_ptr->thread_id]++;
                    memory_reqhit[wr_ptr->thread_id]++;
                    memory_reqhit_ch_inst[channel]++;
                    memory_reqhit_ch[channel]++;
                    memory_reqhit_total_inst++;
                    memory_reqhit_total++;

	        		average_gap_between_activates[channel][wr_ptr->dram_addr.rank] = ((average_gap_between_activates[channel][wr_ptr->dram_addr.rank]*(stats_num_activate[channel][wr_ptr->dram_addr.rank]-1)) + (CYCLE_VAL-last_activate[channel][wr_ptr->dram_addr.rank]))/stats_num_activate[channel][wr_ptr->dram_addr.rank];
                    
                    last_activate[channel][wr_ptr->dram_addr.rank] = CYCLE_VAL;

                    wr_ptr->bank_time = scoreboard_t[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].service_time + T_CWD + T_RP + T_RCD;
                    scoreboard_t[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].service_time += T_CWD + T_RP + T_RCD;
                
                }
                scoreboard_t[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank].opened_row = wr_ptr->dram_addr.row;
            }
		}
	}

	// Draining Reads
	// Simple FCFS 
	if(!drain_writes[channel])
	{
		LL_FOREACH(read_queue_head[channel],rd_ptr)
		{
            if(rd_ptr->command_issuable == 3){
                if(CYCLE_VAL >= rd_ptr->channel_time){
                    rd_ptr->completion_time = CYCLE_VAL+ T_CAS;
		        	rd_ptr->latency = rd_ptr->completion_time - rd_ptr->arrival_time;
        			rd_ptr->request_served = 1;

                    //Added by Gururaj
                    // Check if current request is a critical request (source or dependency), or not
                    if(rd_ptr->critical){ //Either Critical Read of $line or Critical read of dependancy metadata
                      ROB[rd_ptr->thread_id].critical_mem_ops[rd_ptr->instruction_id]--;

                      //Added by Gururaj to incorporate encryption delays when fetching counter from memory
                      add_encr_delay =0;
                      if(ROB[rd_ptr->thread_id].critical_mem_ops[rd_ptr -> instruction_id] == 0){ //Last critical request finished
                        if(ROB[rd_ptr->thread_id].comptime[rd_ptr->instruction_id] - CYCLE_VAL_CORE[rd_ptr->thread_id] > BIGNUM ){ // Need to add encryption / authentication latency
                          add_encr_delay = ENCR_DELAY;
                        }
                      }

                      lat_stat_process(lat_sim_stat, &(ROB[rd_ptr->thread_id].read_lat_stat[rd_ptr->instruction_id]), rd_ptr->type,
                                       CYCLE_VAL_CORE[rd_ptr->thread_id], CYCLE_VAL_CORE[rd_ptr->thread_id]+ T_CAS,
                                       ROB[rd_ptr->thread_id].critical_mem_ops[rd_ptr -> instruction_id], mem_org);
              
              
                      if(ROB[rd_ptr->thread_id].critical_mem_ops[rd_ptr -> instruction_id] == 0){ //Last critical request finished
                        // update the ROB with the completion time
                        ROB[rd_ptr->thread_id].comptime[rd_ptr->instruction_id]=CYCLE_VAL_CORE[rd_ptr->thread_id]+PIPELINEDEPTH+((T_CAS*100)/core_mod[rd_ptr->thread_id]) + add_encr_delay;
                      }
                    }

                    //For non-critical read requests (non-critical metadata reads), do not tamper with ROB (already updated in main.c)

                    
                    //Update the memory loggers
                    memory_rd_inst[rd_ptr->thread_id]++;
                    memory_rd[rd_ptr->thread_id]++;
                    memory_req_inst[rd_ptr->thread_id]++;
                    memory_req[rd_ptr->thread_id]++;

                    memory_rd_ch_inst[channel]++;
                    memory_rd_ch[channel]++;
                    memory_req_ch_inst[channel]++;
                    memory_req_ch[channel]++;

                    memory_rd_total_inst++;
                    memory_rd_total++;
                    memory_req_total_inst++;
                    memory_req_total++;

                    memory_rdqueue_inst[rd_ptr->thread_id] = rd_ptr->latency + memory_rdqueue_inst[rd_ptr->thread_id];
                    memory_rdqueue[rd_ptr->thread_id] = rd_ptr->latency + memory_rdqueue[rd_ptr->thread_id];
                    memory_reqqueue_inst[rd_ptr->thread_id] = rd_ptr->latency + memory_reqqueue_inst[rd_ptr->thread_id];
                    memory_reqqueue[rd_ptr->thread_id] = rd_ptr->latency + memory_reqqueue[rd_ptr->thread_id];	

                    memory_rdqueue_ch_inst[channel] = rd_ptr->latency + memory_rdqueue_ch_inst[channel];
                    memory_rdqueue_ch[channel] = rd_ptr->latency + memory_rdqueue_ch[channel];
                    memory_reqqueue_ch_inst[channel] = rd_ptr->latency + memory_reqqueue_ch_inst[channel];
                    memory_reqqueue_ch[channel] = rd_ptr->latency + memory_reqqueue_ch[channel];	

                    memory_rdqueue_total_inst = rd_ptr->latency + memory_rdqueue_total_inst;
                    memory_rdqueue_total = rd_ptr->latency + memory_rdqueue_total;
                    memory_reqqueue_total_inst = rd_ptr->latency + memory_reqqueue_total_inst;
                    memory_reqqueue_total = rd_ptr->latency + memory_reqqueue_total;

                    stats_reads_completed[channel] ++;
                    stats_num_read[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]++;
 
                    stats_average_read_latency[channel] = ((stats_reads_completed[channel]-1)*stats_average_read_latency[channel] + rd_ptr->latency)/stats_reads_completed[channel];
                    stats_average_read_queue_latency[channel] = ((stats_reads_completed[channel]-1)*stats_average_read_queue_latency[channel] + (rd_ptr->dispatch_time - rd_ptr->arrival_time))/stats_reads_completed[channel];
                   
                    for(int i=0; i<NUM_RANKS ;i++)
              {
                if(i != rd_ptr->dram_addr.rank)
                  stats_time_spent_terminating_reads_from_other_ranks[channel][i] += T_DATA_TRANS;
              }
                }
            }

            if(rd_ptr->command_issuable == 2){
                // Check if the time for executing these commands is over
                if(CYCLE_VAL >= rd_ptr->bank_time){
                    rd_ptr->command_issuable =3;
                    rd_ptr->channel_time = scoreboard_ch_t[channel].end_time + T_DATA_TRANS;
                    scoreboard_ch_t[channel].end_time += T_DATA_TRANS;
                }
            }

            if(rd_ptr->command_issuable == 1){
                rd_ptr->command_issuable=2;
			    rd_ptr->dispatch_time = CYCLE_VAL;
                //Check if the row is open
                if(scoreboard_t[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].opened_row == rd_ptr->dram_addr.row){
                    rd_ptr->bank_time = scoreboard_t[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].service_time + T_CCD;
                    scoreboard_t[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].service_time += T_CCD; 
                }
                else{
        			stats_num_activate[channel][rd_ptr->dram_addr.rank]++;
                   	stats_num_activate_read[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]++;
				    stats_num_activate_spec[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]++;
	                stats_num_precharge[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]++;

                    //Memory logger for write activates
                    memory_rdhit_inst[rd_ptr->thread_id]++;
                    memory_rdhit[rd_ptr->thread_id]++;
                    memory_rdhit_ch_inst[channel]++;
                    memory_rdhit_ch[channel]++;
                    memory_rdhit_total_inst++;
                    memory_rdhit_total++;

                    // Memory Req Activates
                    memory_reqhit_inst[rd_ptr->thread_id]++;
                    memory_reqhit[rd_ptr->thread_id]++;
                    memory_reqhit_ch_inst[channel]++;
                    memory_reqhit_ch[channel]++;
                    memory_reqhit_total_inst++;
                    memory_reqhit_total++;

                    average_gap_between_activates[channel][rd_ptr->dram_addr.rank] = ((average_gap_between_activates[channel][rd_ptr->dram_addr.rank]*(stats_num_activate[channel][rd_ptr->dram_addr.rank]-1)) + (CYCLE_VAL-last_activate[channel][rd_ptr->dram_addr.rank]))/stats_num_activate[channel][rd_ptr->dram_addr.rank];

	        		last_activate[channel][rd_ptr->dram_addr.rank] = CYCLE_VAL;
		            
                    rd_ptr->bank_time = scoreboard_t[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].service_time + T_RTP + T_RP + T_RCD;
                    scoreboard_t[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].service_time +=  T_RTP + T_RP + T_RCD;
                }
                scoreboard_t[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank].opened_row = rd_ptr->dram_addr.row;
            }
 	
		}
	}
}

void scheduler_stats()
{
  /* Nothing to print for now. */
}

