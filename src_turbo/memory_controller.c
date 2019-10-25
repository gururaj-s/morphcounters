#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "utlist.h"

#include "utils.h"

#include "params.h"
#include "memory_controller.h"
#include "scheduler.h"
#include "processor.h"
#include "memOrg.h" //- Added by Gururaj
#include "stats.h"

// ROB Structure, used to release stall on instructions 
// when the read request completes
extern struct robstructure * ROB;
extern struct memOrg_t*     mem_org;//- Added by Gururaj
extern mem_stats_t* mem_stats;
extern mem_stats_t* mem_stats_inst;

// Current Processor Cycle
extern long long int CYCLE_VAL;
extern long long int *CYCLE_VAL_CORE;
extern int *core_mod;

//MemOrg
extern int CTR_SIZE;

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

//************* Extern Variable for Power Logging ****************************************

extern double usimm_total_rdcycle;
extern double usimm_total_wrcycle;
extern double usimm_total_rdother_cycle;
extern double usimm_total_wrother_cycle;
extern double usimm_total_pre_pdn_fastcycle;
extern double usimm_total_pre_pdn_slowcycle;
extern double usimm_total_act_pdncycle;
extern double usimm_total_act_stbycycle;
extern double usimm_total_pre_stbycycle;

extern double usimm_total_bck;
extern double usimm_total_act;
extern double usimm_total_rd;
extern double usimm_total_wr;
extern double usimm_total_rd_term;
extern double usimm_total_wr_term;
extern double usimm_total_rdoth_term;
extern double usimm_total_wroth_term;
extern double usimm_ref_pwr;
extern double usimm_total_pwr;

//****************************************************************************************



// initialize dram variables and statistics
void init_memory_controller_vars()
{
    num_read_merge =0;
	num_write_merge =0;
	for(int i=0; i<NUM_CHANNELS; i++)
	{

		for(int j=0; j<NUM_RANKS; j++)
		{
			for (int k=0; k<NUM_BANKS; k++)
			{
				stats_num_activate_read[i][j][k]=0;
				stats_num_activate_write[i][j][k]=0;
				stats_num_activate_spec[i][j][k]=0;
				stats_num_precharge[i][j][k]=0;
				stats_num_read[i][j][k]=0;
				stats_num_write[i][j][k]=0;
			}
		
			stats_time_spent_in_active_power_down[i][j]=0;
			stats_time_spent_in_precharge_power_down_slow[i][j]=0;
			stats_time_spent_in_precharge_power_down_fast[i][j]=0;
            stats_time_spent_terminating_reads_from_other_ranks[i][j]=0;
            stats_time_spent_terminating_writes_to_other_ranks[i][j]=0;
            last_activate[i][j]=0;
			
            average_gap_between_activates[i][j]=0;

			stats_num_powerdown_slow[i][j]=0;
			stats_num_powerdown_fast[i][j]=0;
			stats_num_powerup[i][j]=0;

			stats_num_activate[i][j]=0;

		}

		read_queue_head[i]=NULL;
		write_queue_head[i]=NULL;

		read_queue_length[i]=0;
		write_queue_length[i]=0;

		// Stats
		stats_reads_merged_per_channel[i]=0;
		stats_writes_merged_per_channel[i]=0;
		
		stats_reads_seen[i]=0;
		stats_writes_seen[i]=0;
		stats_reads_completed[i]=0;
		stats_writes_completed[i]=0;
		stats_average_read_latency[i]=0;
		stats_average_read_queue_latency[i]=0;
		stats_average_write_latency[i]=0;
		stats_average_write_queue_latency[i]=0;
	}
}

/********************************************************/
/*	Utility Functions				*/
/********************************************************/


// Removed by Gururaj - included elsewhere in the project
/* unsigned int log_base2(unsigned int new_value) */
/* { */
/* 	int i; */
/* 	for (i = 0; i < 32; i++) { */
/* 		new_value >>= 1; */
/* 		if (new_value == 0) */
/* 			break; */
/* 	} */
/* 	return i; */
/* } */



// Function to decompose the incoming DRAM address into the
// constituent channel, rank, bank, row and column ids. 
// Note : To prevent memory leaks, call free() on the pointer returned
// by this function after you have used the return value.
dram_address_t * calc_dram_addr(long long int physical_address)
{


	long long int input_a, temp_b, temp_a;

	int channelBitWidth = log_base2(NUM_CHANNELS);
	int rankBitWidth = log_base2(NUM_RANKS);
	int bankBitWidth = log_base2(NUM_BANKS);
	int rowBitWidth = log_base2(NUM_ROWS);
	int colBitWidth = log_base2(NUM_COLUMNS);
	int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);



	dram_address_t * this_a = (dram_address_t*)malloc(sizeof(dram_address_t));

	this_a->actual_address = physical_address;

	input_a = physical_address;

	input_a = input_a >> byteOffsetWidth;		  // strip out the cache_offset

    
    //Modified by Gururaj - to accodomate different mappings for different types of accesses

    if(ADDRESS_MAPPING == 1) {

      //If the address is in in the "usable data" range
      if( get_partition(physical_address, mem_org) == 0){ //Usable Data

        temp_b = input_a;
        input_a = input_a >> colBitWidth;
        temp_a  = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b;		//strip out the column address 
		
        temp_b = input_a;   				
        input_a = input_a >> channelBitWidth;
        temp_a  = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; 		// strip out the channel address

        temp_b = input_a;
        input_a = input_a >> bankBitWidth;
        temp_a  = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b;		// strip out the bank address 

        temp_b = input_a;			
        input_a = input_a >> rankBitWidth;
        temp_a  = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b;     		// strip out the rank address

        temp_b = input_a;		
        input_a = input_a >> rowBitWidth;
        temp_a  = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b;		// strip out the row number

      }
      //Else if the address is in the "user invisible" metadata range
      else if( get_partition(physical_address, mem_org) == 1){ //MTree Entry
        temp_b = input_a;
        input_a = input_a >> colBitWidth;
        temp_a  = input_a << colBitWidth;
        this_a->column = temp_a ^ temp_b;		//strip out the column address 
		
        temp_b = input_a;   				
        input_a = input_a >> channelBitWidth;
        temp_a  = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; 		// strip out the channel address

        temp_b = input_a;
        input_a = input_a >> bankBitWidth;
        temp_a  = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b;		// strip out the bank address 

        temp_b = input_a;			
        input_a = input_a >> rankBitWidth;
        temp_a  = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b;     		// strip out the rank address


        temp_b = input_a;		
        input_a = input_a >> rowBitWidth;
        temp_a  = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b;		// strip out the row number
      
      }
      else if( get_partition(physical_address, mem_org) == 2){ //Counter

        temp_b = input_a;
        input_a = input_a >> (colBitWidth - log_base2(CACHE_LINE_SIZE/CTR_SIZE));
        temp_a  = input_a << (colBitWidth - log_base2(CACHE_LINE_SIZE/CTR_SIZE));
        this_a->column = temp_a ^ temp_b;		//strip out 4 bits of the column address - lower bits 
		
        temp_b = input_a;   				
        input_a = input_a >> channelBitWidth;
        temp_a  = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; 		// strip out the channel address

        temp_b = input_a;
        input_a = input_a >> bankBitWidth;
        temp_a  = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b;		// strip out the bank address 

        temp_b = input_a;			
        input_a = input_a >> rankBitWidth;
        temp_a  = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b;     		// strip out the rank address

        temp_b = input_a;
        input_a = input_a >> (log_base2(CACHE_LINE_SIZE/CTR_SIZE));
        temp_a  = input_a << (log_base2(CACHE_LINE_SIZE/CTR_SIZE));
        this_a->column = ( (temp_a ^ temp_b) << (colBitWidth - log_base2(CACHE_LINE_SIZE/CTR_SIZE)) ) ^ (this_a->column) ;		//strip out the column address - upper 3 bits
      
        temp_b = input_a;		
        input_a = input_a >> rowBitWidth;
        temp_a  = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b;		// strip out the row number

        this_a ->channel = (this_a->channel + 3)% (((INT_64) 1) << channelBitWidth);      
        this_a ->bank    = (this_a ->bank + 3)% (((INT_64) 1) << bankBitWidth);
      
      }
      else if( get_partition(physical_address, mem_org) == 3){ //MAC

        temp_b = input_a;
        input_a = input_a >> (colBitWidth - log_base2(CACHE_LINE_SIZE/MAC_SIZE));
        temp_a  = input_a << (colBitWidth - log_base2(CACHE_LINE_SIZE/MAC_SIZE));
        this_a->column = temp_a ^ temp_b;		//strip out 4 bits of the column address - lower bits 
		
        temp_b = input_a;   				
        input_a = input_a >> channelBitWidth;
        temp_a  = input_a << channelBitWidth;
        this_a->channel = temp_a ^ temp_b; 		// strip out the channel address

        temp_b = input_a;
        input_a = input_a >> bankBitWidth;
        temp_a  = input_a << bankBitWidth;
        this_a->bank = temp_a ^ temp_b;		// strip out the bank address 

        temp_b = input_a;			
        input_a = input_a >> rankBitWidth;
        temp_a  = input_a << rankBitWidth;
        this_a->rank = temp_a ^ temp_b;     		// strip out the rank address

        temp_b = input_a;
        input_a = input_a >> (log_base2(CACHE_LINE_SIZE/MAC_SIZE));
        temp_a  = input_a << (log_base2(CACHE_LINE_SIZE/MAC_SIZE));
        this_a->column = ( (temp_a ^ temp_b) << (colBitWidth - log_base2(CACHE_LINE_SIZE/MAC_SIZE)) ) ^ (this_a->column) ;		//strip out the column address - upper 3 bits
      
        temp_b = input_a;		
        input_a = input_a >> rowBitWidth;
        temp_a  = input_a << rowBitWidth;
        this_a->row = temp_a ^ temp_b;		// strip out the row number

        this_a ->channel = (this_a->channel + 1)% (((INT_64) 1) << channelBitWidth);      
        this_a ->bank    = (this_a ->bank + 1)% (((INT_64) 1) << bankBitWidth);
      
      }
    }

    else if(ADDRESS_MAPPING == 2) {

      /* FORMAT:
         < Row # | Rank # | Bank # | Channel # | Column # | Byte_In_Cacheline >
      */
      temp_b = input_a;
      input_a = input_a >> colBitWidth;
      temp_a  = input_a << colBitWidth;
      this_a->column = temp_a ^ temp_b;		//strip out the column address

      
      temp_b = input_a;   	
      input_a = input_a >> channelBitWidth;
      temp_a  = input_a << channelBitWidth;
      this_a->channel = temp_a ^ temp_b; 		// strip out the channel address

      temp_b = input_a;
      input_a = input_a >> bankBitWidth;
      temp_a  = input_a << bankBitWidth;
      this_a->bank = temp_a ^ temp_b;		// strip out the bank address 


      temp_b = input_a;
      input_a = input_a >> rankBitWidth;
      temp_a  = input_a << rankBitWidth;
      this_a->rank = temp_a ^ temp_b;     		// strip out the rank address

      temp_b = input_a;
      input_a = input_a >> rowBitWidth;
      temp_a  = input_a << rowBitWidth;
      this_a->row = temp_a ^ temp_b;			// strip out the row number
    
    }

	else
      {
        /* FORMAT:
           < Row # | Column # | Rank # | Bank # | Channel # | Byte_In_Cacheline >
        */

		temp_b = input_a;   	
		input_a = input_a >> channelBitWidth;
		temp_a  = input_a << channelBitWidth;
		this_a->channel = temp_a ^ temp_b; 		// strip out the channel address


		temp_b = input_a;
		input_a = input_a >> bankBitWidth;
		temp_a  = input_a << bankBitWidth;
		this_a->bank = temp_a ^ temp_b;		// strip out the bank address 


		temp_b = input_a;
		input_a = input_a >> rankBitWidth;
		temp_a  = input_a << rankBitWidth;
		this_a->rank = temp_a ^ temp_b;     		// strip out the rank address


		temp_b = input_a;
		input_a = input_a >> colBitWidth;
		temp_a  = input_a << colBitWidth;
		this_a->column = temp_a ^ temp_b;		//strip out the column address


		temp_b = input_a;
		input_a = input_a >> rowBitWidth;
		temp_a  = input_a << rowBitWidth;
		this_a->row = temp_a ^ temp_b;			// strip out the row number
      }
	return(this_a);
}

// Function to create a new request node to be inserted into the read
// or write queue.
void * init_new_node(long long int physical_address, long long int arrival_time, optype_t type, int thread_id, int instruction_id, long long int instruction_pc, int mem_prio, int is_critical, maccess_type mtype)

{
  request_t * new_node = NULL;

  new_node = (request_t*)malloc(sizeof(request_t));

  if(new_node == NULL)
    {
      printf("FATAL : Malloc Error\n");

      exit(-1);
    }
  else
    {

      new_node->physical_address = physical_address;

      new_node->arrival_time = arrival_time;

      new_node->dispatch_time = -100;

      new_node->completion_time = -100;

      new_node->latency = -100;

      new_node->thread_id = thread_id;

      new_node->next_command = NOP;

      new_node->command_issuable = 1;

      new_node->operation_type = type;

      new_node->request_served = 0;

      new_node->instruction_id = instruction_id;

      new_node->instruction_pc = instruction_pc;

      new_node->memory_priority = mem_prio;
      
      new_node->critical = is_critical;

      new_node->type = mtype;

      new_node->bank_time = 0;

      new_node->channel_time = 0;

      new_node->next = NULL;

      dram_address_t * this_node_addr = calc_dram_addr(physical_address);

      new_node->dram_addr.actual_address = physical_address;
      new_node->dram_addr.channel = this_node_addr->channel;
      new_node->dram_addr.rank = this_node_addr->rank;
      new_node->dram_addr.bank = this_node_addr->bank;
      new_node->dram_addr.row = this_node_addr->row;
      new_node->dram_addr.column = this_node_addr->column;

      free(this_node_addr);

      new_node->user_ptr = NULL;

      return (new_node);
    }
}

// Function that checks to see if an incoming read can be served by a
// write request pending in the write queue and return
// WQ_LOOKUP_LATENCY if there is a match. Also the function goes over
// the read_queue to see if there is a pending read for the same
// address and avoids duplication. The 2nd read is assumed to be
// serviced when the original request completes.

#define RQ_LOOKUP_LATENCY 1
int read_matches_write_or_read_queue(long long int physical_address)
{
	//get channel info
	dram_address_t * this_addr = calc_dram_addr(physical_address);
	int channel = this_addr->channel;
	free(this_addr);

	request_t * wr_ptr = NULL;
	request_t * rd_ptr = NULL;

	LL_FOREACH(write_queue_head[channel], wr_ptr)
	{
		if(wr_ptr->dram_addr.actual_address == physical_address)
		{
		  num_read_merge ++;
		  stats_reads_merged_per_channel[channel]++;
		  return WQ_LOOKUP_LATENCY;
		}
	}

	LL_FOREACH(read_queue_head[channel], rd_ptr)
	{
		if(rd_ptr->dram_addr.actual_address == physical_address)
		{
		  num_read_merge ++;
		  stats_reads_merged_per_channel[channel]++;
		  return RQ_LOOKUP_LATENCY;
		}
	}
	return 0;
}

// Function to merge writes to the same address
int write_exists_in_write_queue(long long int physical_address)
{
	//get channel info
	dram_address_t * this_addr = calc_dram_addr(physical_address);
	int channel = this_addr->channel;
	free(this_addr);
	
	request_t * wr_ptr = NULL;

	LL_FOREACH(write_queue_head[channel], wr_ptr)
	{
		if(wr_ptr->dram_addr.actual_address == physical_address)
		{
		  num_write_merge ++;
		  stats_writes_merged_per_channel[channel]++;
		  return 1;
		}
	}
	return 0;

}

// Insert a new read to the read queue
request_t * insert_read(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, long long int instruction_pc, int mem_prio, int is_critical,maccess_type type)
{
  //Update Stats
  update_mem_stats(mem_stats, type, 0, physical_address, thread_id);
  update_mem_stats(mem_stats_inst, type, 0, physical_address, thread_id);

  if(sim_fast != 1){
    //printf("SIM_FAST is != 1 is queing up requests in queue\n");
    optype_t this_op = READ;

     //get channel info
	dram_address_t * this_addr = calc_dram_addr(physical_address);
	int channel = this_addr->channel;
	free(this_addr);

	stats_reads_seen[channel] ++;

	request_t * new_node = init_new_node(physical_address, arrival_time, this_op, thread_id, instruction_id, instruction_pc, mem_prio,  is_critical, type);

    LL_APPEND(read_queue_head[channel], new_node);

	read_queue_length[channel] ++;

	//UT_MEM_DEBUG("\nCyc: %lld New READ:%lld Core:%d Chan:%d Rank:%d Bank:%d Row:%lld RD_Q_Length:%lld\n", CYCLE_VAL, new_node->id, new_node->thread_id, new_node->dram_addr.channel,  new_node->dram_addr.rank,  new_node->dram_addr.bank,  new_node->dram_addr.row, read_queue_length[channel]);
	
	return new_node;
  }
  else { //Dont care about read/write queues maintenance.
    return NULL;
  }
}

// Insert a new write to the write queue
request_t * insert_write(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, int mem_prio, int is_critical, maccess_type type)
{
  //Update Stats
  update_mem_stats(mem_stats, type, 1 , physical_address, thread_id);
  update_mem_stats(mem_stats_inst, type, 1 , physical_address, thread_id);


  if(sim_fast != 1){  
    //printf("SIM_FAST is != 1 is queing up requests in queue\n");
    optype_t this_op = WRITE;

    dram_address_t * this_addr = calc_dram_addr(physical_address);
    int channel = this_addr->channel;
    free(this_addr);

    stats_writes_seen[channel] ++;
  
    request_t * new_node = init_new_node(physical_address, arrival_time, this_op, thread_id, instruction_id, 0, mem_prio, is_critical, type);
  
    LL_APPEND(write_queue_head[channel], new_node);
  
    write_queue_length[channel] ++;
  
    //UT_MEM_DEBUG("\nCyc: %lld New WRITE:%lld Core:%d Chan:%d Rank:%d Bank:%d Row:%lld WR_Q_Length:%lld\n", CYCLE_VAL, new_node->id, new_node->thread_id, new_node->dram_addr.channel,  new_node->dram_addr.rank,  new_node->dram_addr.bank,  new_node->dram_addr.row, write_queue_length[channel]);
  
    return new_node;
  }
  else { //Dont care about read/write queues maintenance.
    return NULL;
  }
  
}

// Remove finished requests from the queues.
void clean_queues(int channel)
{

	request_t * rd_ptr =  NULL;
	request_t * rd_tmp = NULL;
	request_t * wrt_ptr = NULL;
	request_t * wrt_tmp = NULL;

	// Delete all READ requests whose completion time has been determined i.e. COL_RD has been issued
	LL_FOREACH_SAFE(read_queue_head[channel],rd_ptr,rd_tmp) 
	{
		if(rd_ptr->request_served == 1)
		{
			assert(rd_ptr->completion_time != -100);

			LL_DELETE(read_queue_head[channel],rd_ptr);

			if(rd_ptr->user_ptr)
				free(rd_ptr->user_ptr);

			free(rd_ptr);

			read_queue_length[channel]--;

			assert(read_queue_length[channel]>=0);

		}
	}

	// Delete all WRITE requests whose completion time has been determined i.e COL_WRITE has been issued
	LL_FOREACH_SAFE(write_queue_head[channel],wrt_ptr,wrt_tmp) 
	{
		if(wrt_ptr->request_served == 1)
		{
			LL_DELETE(write_queue_head[channel],wrt_ptr);

			if(wrt_ptr->user_ptr)
				free(wrt_ptr->user_ptr);

			free(wrt_ptr);

			write_queue_length[channel]--;

			assert(write_queue_length[channel]>=0);
		}
	}
}


void print_stats()
{
	long long int activates_for_reads = 0;
	long long int activates_for_spec = 0;
	long long int activates_for_writes = 0;
	long long int read_cmds = 0;
	long long int write_cmds = 0;

	for(int c=0 ; c < NUM_CHANNELS ; c++)
	{
		activates_for_writes = 0;
		activates_for_reads = 0;
		activates_for_spec = 0;
		read_cmds = 0;
		write_cmds = 0;
		for(int r=0;r<NUM_RANKS ;r++)
		{
			for(int b=0; b<NUM_BANKS ; b++)
			{
				activates_for_writes += stats_num_activate_write[c][r][b];
				activates_for_reads += stats_num_activate_read[c][r][b];
				activates_for_spec += stats_num_activate_spec[c][r][b];
				read_cmds += stats_num_read[c][r][b];
				write_cmds += stats_num_write[c][r][b];
			}
		}
		
		printf("-------- Channel %d Stats-----------\n",c);
		printf("Total Reads Serviced :          %-7lld\n", stats_reads_completed[c]);
		printf("Total Writes Serviced :         %-7lld\n", stats_writes_completed[c]);
		printf("Average Read Latency :          %7.5f\n", (double)stats_average_read_latency[c]);
		printf("Average Read Queue Latency :    %7.5f\n", (double)stats_average_read_queue_latency[c]);
		printf("Average Write Latency :         %7.5f\n", (double)stats_average_write_latency[c]);
		printf("Average Write Queue Latency :   %7.5f\n", (double)stats_average_write_queue_latency[c]);
		printf("Read Page Hit Rate :            %7.5f\n",((double)(read_cmds-activates_for_reads-activates_for_spec)/read_cmds));
		printf("Write Page Hit Rate :           %7.5f\n",((double)(write_cmds-activates_for_writes)/write_cmds));
		printf("------------------------------------\n");
	}
}

void update_memory()
{
	for(int channel=0;channel<NUM_CHANNELS;channel++)
	{
		// remove finished requests
		clean_queues(channel);
	}
}



//------------------------------------------------------------
// Calculate Power: It calculates and returns average power used by every Rank on Every 
// Channel during the course of the simulation 
// Units : Time- ns; Current mA; Voltage V; Power mW; 
//------------------------------------------------------------

float calculate_power(int channel, int rank, int print_stats_type, int chips_per_rank)
{
	/*
	Power is calculated using the equations from Technical Note "TN-41-01: Calculating Memory System Power for DDR"
	The current values IDD* are taken from the data sheets. 
	These are average current values that the chip will draw when certain actions occur as frequently as possible.
	i.e., the worst case power consumption
	Eg: when ACTs happen every tRC
		pds_<component> is the power calculated by directly using the current values from the data sheet. 'pds' stands for 
	PowerDataSheet. This will the power drawn by the chip when operating under the activity that is assumed in the data 
	sheet. This mostly represents the worst case power
		These pds_<*> components need to be derated in accordance with the activity that is observed. Eg: If ACTs occur slower
	than every tRC, then pds_act will be derated to give "psch_act" (SCHeduled Power consumed by Activate) 
	*/

/*------------------------------------------------------------
// total_power is the sum of of 13 components listed below 
// Note: CKE is the ClocK Enable to every chip.
// Note: Even though the reads and write are to a different rank on the same channel, the Pull-Up and the Pull-Down resistors continue 
// 		to burn some power. psch_termWoth and psch_termWoth stand for the power dissipated in the rank in question when the reads and
// 		writes are to other ranks on the channel

    psch_act 						-> Power dissipated by activating a row
    psch_act_pdn 				-> Power dissipated when CKE is low (disabled) and all banks are precharged
    psch_act_stby 			-> Power dissipated when CKE is high (enabled) and at least one back is active (row is open)
    psch_pre_pdn_fast  	-> Power dissipated when CKE is low (disabled) and all banks are precharged and chip is in fast power down
    psch_pre_pdn_slow  	-> Power dissipated when CKE is low (disabled) and all banks are precharged and chip is in fast slow  down
    psch_pre_stby 			-> Power dissipated when CKE is high (enabled) and at least one back is active (row is open)
    psch_termWoth 			-> Power dissipated when a Write termiantes at the other set of chips.
    psch_termRoth 			-> Power dissipated when a Read  termiantes at the other set of chips
    psch_termW 					-> Power dissipated when a Write termiantes at the set of chips in question
    psch_dq 						-> Power dissipated when a Read  termiantes at the set of chips in question (Data Pins on the chip are called DQ)
    psch_ref 						-> Power dissipated during Refresh
    psch_rd 						-> Power dissipated during a Read  (does ot include power to open a row)
    psch_wr 						-> Power dissipated during a Write (does ot include power to open a row)

------------------------------------------------------------*/

	float pds_act ;						
	float pds_act_pdn;
	float pds_act_stby;
	float pds_pre_pdn_fast;
	float pds_pre_pdn_slow;
	float pds_pre_stby;
	float pds_wr;
	float pds_rd;
	float pds_ref;
	float pds_dq;
	float pds_termW;
	float pds_termRoth;
	float pds_termWoth;

	float psch_act;
	float psch_pre_pdn_slow;
	float psch_pre_pdn_fast;
	float psch_act_pdn;
	float psch_act_stby;
	float psch_pre_stby;
	float psch_rd;
	float psch_wr;
	float psch_ref;
	float psch_dq;
	float psch_termW;
	float psch_termRoth;
	float psch_termWoth;

	float total_chip_power;
	float total_rank_power;

	long long int writes =0 , reads=0;

	static int print_total_cycles=0;


	/*----------------------------------------------------
	//Calculating DataSheet Power
	----------------------------------------------------*/

	pds_act = (IDD0 - (IDD3N * T_RAS + IDD2N *(T_RC - T_RAS))/T_RC) * VDD;
	
	pds_pre_pdn_slow = IDD2P0 * VDD;

	pds_pre_pdn_fast = IDD2P1 * VDD;

	pds_act_pdn = IDD3P * VDD;

	pds_pre_stby = IDD2N * VDD;
	pds_act_stby = IDD3N * VDD;

	pds_wr = (IDD4W - IDD3N) * VDD;

	pds_rd = (IDD4R - IDD3N) * VDD;

	pds_ref = (IDD5- IDD3N) * VDD;


	/*----------------------------------------------------
	//On Die Termination (ODT) Power:
	//Values obtained from Micron Technical Note
	//This is dependent on the termination configuration of the simulated configuration
	//our simulator uses the same config as that used in the Tech Note
	----------------------------------------------------*/
	pds_dq = 3.2 * 10;

	pds_termW = 0;

	pds_termRoth = 24.9 * 10;

	pds_termWoth = 20.8 * 11;

	/*----------------------------------------------------
  //Derating worst case power to represent system activity
	----------------------------------------------------*/

	//average_gap_between_activates was initialised to 0. So if it is still
	//0, then no ACTs have happened to this rank.
	//Hence activate-power is also 0
	if (average_gap_between_activates[channel][rank] == 0)
	{
		psch_act = 0;
	} else {
		psch_act = pds_act * T_RC/(average_gap_between_activates[channel][rank]);
	}
	
	psch_act_pdn = pds_act_pdn * ((double)stats_time_spent_in_active_power_down[channel][rank]/CYCLE_VAL);
	psch_pre_pdn_slow = pds_pre_pdn_slow * ((double)stats_time_spent_in_precharge_power_down_slow[channel][rank]/CYCLE_VAL);
	psch_pre_pdn_fast = pds_pre_pdn_fast * ((double)stats_time_spent_in_precharge_power_down_fast[channel][rank]/CYCLE_VAL);

	psch_act_stby = pds_act_stby * ((double)stats_time_spent_in_active_standby[channel][rank]/CYCLE_VAL);

	/*----------------------------------------------------
  //pds_pre_stby assumes that the system is powered up and every 
	//row has been precharged during every cycle 
	// In reality, the chip could have been in a power-down mode
	//or a row could have been active. The time spent in these modes 
	//should be deducted from total time
	----------------------------------------------------*/
	psch_pre_stby = pds_pre_stby * ((double)(CYCLE_VAL - stats_time_spent_in_active_standby[channel][rank]- stats_time_spent_in_precharge_power_down_slow[channel][rank] - stats_time_spent_in_precharge_power_down_fast[channel][rank] - stats_time_spent_in_active_power_down[channel][rank]))/CYCLE_VAL;

	/*----------------------------------------------------
  //Calculate Total Reads ans Writes performed in the system
	----------------------------------------------------*/
	
	for(int i=0;i<NUM_BANKS;i++)
	{
		writes+= stats_num_write[channel][rank][i];
		reads+=stats_num_read[channel][rank][i];
	}

    /*----------------------------------------------------
    // pds<rd/wr> assumes that there is rd/wr happening every cycle
	// T_DATA_TRANS is the number of cycles it takes for one rd/wr
	----------------------------------------------------*/
	psch_wr = pds_wr * (writes*T_DATA_TRANS)/CYCLE_VAL;

	psch_rd = pds_rd * (reads*T_DATA_TRANS)/CYCLE_VAL;

	/*----------------------------------------------------
    //pds_ref assumes that there is always a refresh happening.
	//in reality, refresh consumes only T_RFC out of every t_REFI
	----------------------------------------------------*/
	psch_ref = pds_ref * T_RFC/T_REFI; 

	psch_dq = pds_dq * (reads*T_DATA_TRANS)/CYCLE_VAL;

	psch_termW = pds_termW * (writes*T_DATA_TRANS)/CYCLE_VAL;


	psch_termRoth = pds_termRoth *  ((double)stats_time_spent_terminating_reads_from_other_ranks[channel][rank]/CYCLE_VAL);
	psch_termWoth = pds_termWoth * ((double)stats_time_spent_terminating_writes_to_other_ranks[channel][rank]/CYCLE_VAL);


	total_chip_power = psch_act + psch_termWoth + psch_termRoth + psch_termW + psch_dq + psch_ref + psch_rd + psch_wr + psch_pre_stby + psch_act_stby + psch_pre_pdn_fast + psch_pre_pdn_slow + psch_act_pdn  ;
	total_rank_power = total_chip_power * chips_per_rank;

	double time_in_pre_stby = (((double)(CYCLE_VAL - stats_time_spent_in_active_standby[channel][rank]- stats_time_spent_in_precharge_power_down_slow[channel][rank] - stats_time_spent_in_precharge_power_down_fast[channel][rank] - stats_time_spent_in_active_power_down[channel][rank]))/CYCLE_VAL);

	if (print_total_cycles ==0) {


		printf ("\n#-----------------------------Simulated Cycles Break-Up-------------------------------------------\n");
		printf ("Note:  1.(Read Cycles + Write Cycles + Read Other + Write Other) should add up to %% cycles during which\n");
		printf ("          the channel is busy. This should be the same for all Ranks on a Channel\n");
		printf ("       2.(PRE_PDN_FAST + PRE_PDN_SLOW + ACT_PDN + ACT_STBY + PRE_STBY) should add up to 100%%\n");
		printf ("       3.Power Down means Clock Enable, CKE = 0. In Standby mode, CKE = 1\n");
		printf ("#-------------------------------------------------------------------------------------------------\n");
		printf ("Total Simulation Cycles                      %11lld\n",CYCLE_VAL );
		printf ("---------------------------------------------------------------\n\n");

		print_total_cycles = 1;
	}

	if (print_stats_type == 0) {
		printf ("USIMM_C%d_R%d_RDCYC           \t: %9.2f\n", channel, rank, (double)reads*T_DATA_TRANS/CYCLE_VAL ); 
		printf ("USIMM_C%d_R%d_WRCYC           \t: %9.2f\n", channel, rank, (double)writes*T_DATA_TRANS/CYCLE_VAL ); 
		printf ("USIMM_C%d_R%d_RDOTHCYC        \t: %9.2f\n", channel, rank, ((double)stats_time_spent_terminating_reads_from_other_ranks[channel][rank]/CYCLE_VAL) ); 
		printf ("USIMM_C%d_R%d_WROTHCYC        \t: %9.2f\n", channel, rank, ((double)stats_time_spent_terminating_writes_to_other_ranks[channel][rank]/CYCLE_VAL) ); 
		printf ("USIMM_C%d_R%d_PRE_PDN_FASTCYC \t: %9.2f\n", channel, rank, ((double)stats_time_spent_in_precharge_power_down_fast[channel][rank]/CYCLE_VAL) ); 
		printf ("USIMM_C%d_R%d_PRE_PDN_SLOWCYC \t: %9.2f\n", channel, rank,	((double)stats_time_spent_in_precharge_power_down_slow[channel][rank]/CYCLE_VAL) ); 
		printf ("USIMM_C%d_R%d_ACT_PDNCYC      \t: %9.2f\n", channel, rank, ((double)stats_time_spent_in_active_power_down[channel][rank]/CYCLE_VAL) ); 
		printf ("USIMM_C%d_R%d_ACT_STBYCYC     \t: %9.2f\n", channel, rank, ((double)stats_time_spent_in_active_standby[channel][rank]/CYCLE_VAL) ); 
		printf ("USIMM_C%d_R%d_PRE_STBYCYC     \t: %9.2f\n",channel, rank, time_in_pre_stby ); 
		printf ("---------------------------------------------------------------\n\n");
        usimm_total_rdcycle += (double)reads*T_DATA_TRANS/CYCLE_VAL ;
        usimm_total_wrcycle += (double)writes*T_DATA_TRANS/CYCLE_VAL ;
        usimm_total_rdother_cycle += (double)stats_time_spent_terminating_reads_from_other_ranks[channel][rank]/CYCLE_VAL;
        usimm_total_wrother_cycle += (double)stats_time_spent_terminating_writes_to_other_ranks[channel][rank]/CYCLE_VAL;
        usimm_total_pre_pdn_fastcycle += (double)stats_time_spent_in_precharge_power_down_fast[channel][rank]/CYCLE_VAL;
        usimm_total_pre_pdn_slowcycle += (double)stats_time_spent_in_precharge_power_down_slow[channel][rank]/CYCLE_VAL;
        usimm_total_act_pdncycle += (double)stats_time_spent_in_active_power_down[channel][rank]/CYCLE_VAL;
        usimm_total_act_stbycycle += (double)stats_time_spent_in_active_standby[channel][rank]/CYCLE_VAL;
        usimm_total_pre_stbycycle += time_in_pre_stby;

	} else if (print_stats_type == 1) {
		/*----------------------------------------------------
		// Total Power is the sum total of all the components calculated above
		----------------------------------------------------*/

		printf ("USIMM_C%d_R%d_BCKmW        \t: %9.2f\n",channel, rank, psch_act_pdn+psch_act_stby+psch_pre_pdn_slow+psch_pre_pdn_fast+psch_pre_stby); 
		printf ("USIMM_C%d_R%d_ACTmW        \t: %9.2f\n",channel, rank, psch_act); 
		printf ("USIMM_C%d_R%d_RDmW         \t: %9.2f\n",channel, rank, psch_rd); 
		printf ("USIMM_C%d_R%d_WRmW         \t: %9.2f\n",channel, rank, psch_wr); 
		printf ("USIMM_C%d_R%d_RD_TERMmW    \t: %9.2f\n",channel, rank, psch_dq); 
		printf ("USIMM_C%d_R%d_WR_TERMmW    \t: %9.2f\n",channel, rank, psch_termW); 
		printf ("USIMM_C%d_R%d_RDOTH_TERMmW \t: %9.2f\n",channel, rank, psch_termRoth); 
		printf ("USIMM_C%d_R%d_WROTH_TERMmW \t: %9.2f\n",channel, rank, psch_termWoth); 
		printf ("USIMM_C%d_R%d_REFmW        \t: %9.2f\n",channel, rank, psch_ref); 
		printf ("---------------------------------------------------------------\n");
		printf ("USIMM_C%d_R%d_TOT_RK_PWRmW \t: %9.2f\n",channel, rank, total_rank_power);
		printf ("---------------------------------------------------------------\n\n");

        usimm_total_bck += psch_act_pdn+psch_act_stby+psch_pre_pdn_slow+psch_pre_pdn_fast+psch_pre_stby;
        usimm_total_act += psch_act;
        usimm_total_rd += psch_rd;
        usimm_total_wr += psch_wr;
        usimm_total_rd_term += psch_dq;
        usimm_total_wr_term += psch_termW;
        usimm_total_rdoth_term += psch_termRoth;
        usimm_total_wroth_term += psch_termWoth;
        usimm_ref_pwr += psch_ref;
        usimm_total_pwr += total_rank_power;

	} else {
		printf ("PANIC: FN_CALL_ERROR: In calculate_power(), print_stats_type can only be 1 or 0\n");
		assert (-1);
	}
	return total_rank_power;
}


//Double check this:
long long int get_twin(long long int physical_address){

  long long int input_a = physical_address;
  int channelBitWidth = log_base2(NUM_CHANNELS);
  int rankBitWidth = log_base2(NUM_RANKS);
  int bankBitWidth = log_base2(NUM_BANKS);
  int colBitWidth = log_base2(NUM_COLUMNS);
  int byteOffsetWidth = log_base2(CACHE_LINE_SIZE);

  long long int rank_len_stream = (1 << rankBitWidth) - 1;
  long long int rank_swap;

  if(ADDRESS_MAPPING == 1) {

    //If the address is in in the "usable data" range
    if( get_partition(physical_address, mem_org) == 0){ //Usable Data
      rank_swap = rank_len_stream << (byteOffsetWidth + colBitWidth + channelBitWidth + bankBitWidth);
      return (input_a ^ rank_swap);
    }
    //Else if the address is in the "user invisible" metadata range
    else if( get_partition(physical_address, mem_org) == 1){ //MTree Entry
      rank_swap = rank_len_stream << (byteOffsetWidth + colBitWidth + channelBitWidth + bankBitWidth);    
      return (input_a ^ rank_swap);
    }
    else if( get_partition(physical_address, mem_org) == 2){ //Counter
      rank_swap = rank_len_stream << (byteOffsetWidth + colBitWidth - log_base2(CACHE_LINE_SIZE/CTR_SIZE) + channelBitWidth + bankBitWidth);
      return (input_a ^ rank_swap);
    }      
    else if( get_partition(physical_address, mem_org) == 3){ //MAC
      rank_swap = rank_len_stream << (byteOffsetWidth + colBitWidth - log_base2(CACHE_LINE_SIZE/MAC_SIZE) + channelBitWidth + bankBitWidth);
      return (input_a ^ rank_swap);
    }

  }

  else if (ADDRESS_MAPPING == 2) {
    rank_swap = rank_len_stream << (byteOffsetWidth + colBitWidth + channelBitWidth + bankBitWidth);
    return (input_a ^ rank_swap);
  }
  
  else{       
    rank_swap = rank_len_stream << (byteOffsetWidth + channelBitWidth + bankBitWidth);
    return (input_a ^ rank_swap);
    
  }

  ASSERTM(0,"Control should not reach here in get_twin");
  return 0;
}
      
