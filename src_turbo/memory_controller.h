#ifndef __MEMORY_CONTROLLER_H__
#define __MEMORY_CONTROLLER_H__

#include "memOrg.h"
#include "stats.h"

#define MAX_NUM_CHANNELS 32
#define MAX_NUM_RANKS 16
#define MAX_NUM_BANKS 32

#define MAX_NUM_CHANNELS_DRAM 16
#define MAX_NUM_RANKS_DRAM 16
#define MAX_NUM_BANKS_DRAM 32

//uns64 *committed; // total committed instructions in each core
//long long int *fetched;   // total fetched instructions in each core

extern int sim_fast;

//////////////////////////////////////////////////
//	Memory Controller Data Structures	//
//////////////////////////////////////////////////

// DRAM Address Structure
typedef struct draddr
{
  long long int actual_address; // physical_address being accessed
  int channel;	// channel id
  int rank;	// rank id
  int bank;	// bank id
  long long int row;	// row/page id
  int column;	// column id
} dram_address_t;

// DRAM Commands 
typedef enum {ACT_CMD, COL_READ_CMD, PRE_CMD, COL_WRITE_CMD, PWR_DN_SLOW_CMD, PWR_DN_FAST_CMD, PWR_UP_CMD, REF_CMD, NOP} command_t; 

// Request Types
typedef enum {READ, WRITE} optype_t;

// Single request structure self-explanatory
typedef struct req
{
  unsigned long long int physical_address;
  dram_address_t dram_addr;
  long long int arrival_time;     
  long long int dispatch_time; // when COL_RD or COL_WR is issued for this request
  long long int completion_time; //final completion time
  long long int latency; // dispatch_time-arrival_time
  int thread_id; // core that issued this request
  command_t next_command; // what command needs to be issued to make forward progress with this request
  int command_issuable; // can this request be issued in the current cycle
  optype_t operation_type; // Read/Write
  int request_served; // if request has it's final command issued or not
  int instruction_id; // 0 to ROBSIZE-1
  long long int instruction_pc; // phy address of instruction that generated this request (valid only for reads)
  void * user_ptr; // user_specified data
  struct req * next;
  int memory_priority;
  //For high speed USIMM - added by Prashant
  unsigned long long int channel_time;
  unsigned long long int bank_time; 
  int critical;
  maccess_type type;
} request_t;

// Bankstates
typedef enum 
{
  IDLE, PRECHARGING, REFRESHING, ROW_ACTIVE, PRECHARGE_POWER_DOWN_FAST, PRECHARGE_POWER_DOWN_SLOW, ACTIVE_POWER_DOWN
} bankstate_t;

// Structure to hold the state of a bank
typedef struct bnk
{
  bankstate_t state;
  long long int active_row;
  long long int next_pre;
  long long int next_act;
  long long int next_read;
  long long int next_write;
  long long int next_powerdown;
  long long int next_powerup;
  long long int next_refresh;
}bank_t;

int CACHE_SIZE;

// contains the states of all banks in the system 
bank_t dram_state[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];

/* //removed by Prashant for High Speed USIMM

// command issued this cycle to this channel
int command_issued_current_cycle[MAX_NUM_CHANNELS];

// cas command issued this cycle to this channel
int cas_issued_current_cycle[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS]; // 1/2 for COL_READ/COL_WRITE
*/

// Per channel read queue
request_t * read_queue_head[MAX_NUM_CHANNELS];

// Per channel write queue
request_t * write_queue_head[MAX_NUM_CHANNELS];

/* //Removed by Prashant for High Speed USIMM
// issuables_for_different commands
int cmd_precharge_issuable[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
int cmd_all_bank_precharge_issuable[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
int cmd_powerdown_fast_issuable[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
int cmd_powerdown_slow_issuable[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
int cmd_powerup_issuable[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
int cmd_refresh_issuable[MAX_NUM_CHANNELS][MAX_NUM_RANKS];


// refresh variables
long long int next_refresh_completion_deadline[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int last_refresh_completion_deadline[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int forced_refresh_mode_on[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int refresh_issue_deadline[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int issued_forced_refresh_commands[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int num_issued_refreshes[MAX_NUM_CHANNELS][MAX_NUM_RANKS];

*/

long long int read_queue_length[MAX_NUM_CHANNELS];
long long int write_queue_length[MAX_NUM_CHANNELS];

// Stats
long long int num_read_merge ;
long long int num_write_merge ;
long long int stats_reads_merged_per_channel[MAX_NUM_CHANNELS];
long long int stats_writes_merged_per_channel[MAX_NUM_CHANNELS];
long long int stats_reads_seen[MAX_NUM_CHANNELS];
long long int stats_writes_seen[MAX_NUM_CHANNELS];
long long int stats_reads_completed[MAX_NUM_CHANNELS];
long long int stats_writes_completed[MAX_NUM_CHANNELS];

double stats_average_read_latency[MAX_NUM_CHANNELS];
double stats_average_read_queue_latency[MAX_NUM_CHANNELS];
double stats_average_write_latency[MAX_NUM_CHANNELS];
double stats_average_write_queue_latency[MAX_NUM_CHANNELS];

long long int stats_page_hits[MAX_NUM_CHANNELS];
double stats_read_row_hit_rate[MAX_NUM_CHANNELS];

// Time spent in various states
long long int stats_time_spent_in_active_standby[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_time_spent_in_active_power_down[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_time_spent_in_precharge_power_down_fast[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_time_spent_in_precharge_power_down_slow[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_time_spent_in_power_up[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int last_activate[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int last_refresh[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
double average_gap_between_activates[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
double average_gap_between_refreshes[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_time_spent_terminating_reads_from_other_ranks[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_time_spent_terminating_writes_to_other_ranks[MAX_NUM_CHANNELS][MAX_NUM_RANKS];

// Command Counters
long long int stats_num_activate_read[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
long long int stats_num_activate_write[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
long long int stats_num_activate_spec[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
long long int stats_num_activate[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_num_precharge[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
long long int stats_num_read[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
long long int stats_num_write[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];
long long int stats_num_powerdown_slow[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_num_powerdown_fast[MAX_NUM_CHANNELS][MAX_NUM_RANKS];
long long int stats_num_powerup[MAX_NUM_CHANNELS][MAX_NUM_RANKS];



// functions

// to get log with base 2
unsigned int log_base2(unsigned int new_value);

// initialize memory_controller variables
void init_memory_controller_vars();

// called every cycle to update the read/write queues
void update_memory();

// activate to bank allowed or not
int is_activate_allowed(int channel, int rank, int bank);

// precharge to bank allowed or not
int is_precharge_allowed(int channel, int rank, int bank);

// all bank precharge allowed or not
int is_all_bank_precharge_allowed(int channel, int rank);

// autoprecharge allowed or not
int is_autoprecharge_allowed(int channel,int rank,int bank);

// power_down fast allowed or not
int is_powerdown_fast_allowed(int channel,int rank);

// power_down slow allowed or not
int is_powerdown_slow_allowed(int channel,int rank);

// powerup allowed or not
int is_powerup_allowed(int channel,int rank);

// refresh allowed or not
int is_refresh_allowed(int channel,int rank);


// issues command to make progress on a request
int issue_request_command(request_t * req);

// power_down command
int issue_powerdown_command(int channel, int rank, command_t cmd);

// powerup command
int issue_powerup_command(int channel, int rank);

// precharge a bank
int issue_activate_command(int channel, int rank, int bank, long long int row);

// precharge a bank
int issue_precharge_command(int channel, int rank, int bank);

// precharge all banks in a rank
int issue_all_bank_precharge_command(int channel, int rank);

// refresh all banks
int issue_refresh_command(int channel, int rank);

// autoprecharge all banks
int issue_autoprecharge(int channel, int rank, int bank);

// find if there is a matching write request
int read_matches_write_or_read_queue(long long int physical_address);

// find if there is a matching request in the write queue
int write_exists_in_write_queue(long long int physical_address);

// enqueue a read into the corresponding read queue (returns ptr to new node)
request_t* insert_read(long long int physical_address, long long int arrival_cycle, int thread_id, int instruction_id, long long int instruction_pc, int mem_prio, int is_critical, maccess_type type);

// enqueue a write into the corresponding write queue (returns ptr to new_node)
request_t* insert_write(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id, int mem_prio, int is_critical, maccess_type type);

// update stats counters
void gather_stats(int channel);

// print statistics
void print_stats();

// calculate power for each channel
float calculate_power(int channel, int rank, int print_stats_type, int chips_per_rank);



int CACHE_SIZE_DRAM;

// contains the states of all banks in the system 
bank_t dram_state_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];

// command issued this cycle to this channel
int command_issued_current_cycle_DRAM[MAX_NUM_CHANNELS_DRAM];

// cas command issued this cycle to this channel
int cas_issued_current_cycle_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM]; // 1/2 for COL_READ/COL_WRITE

// Per channel read queue
request_t * read_queue_head_DRAM[MAX_NUM_CHANNELS_DRAM];

// Per channel write queue
request_t * write_queue_head_DRAM[MAX_NUM_CHANNELS_DRAM];

// issuables_for_different commands
int cmd_precharge_issuable_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
int cmd_all_bank_precharge_issuable_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int cmd_powerdown_fast_issuable_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int cmd_powerdown_slow_issuable_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int cmd_powerup_issuable_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int cmd_refresh_issuable_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];


// refresh variables
long long int next_refresh_completion_deadline_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int last_refresh_completion_deadline_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int forced_refresh_mode_on_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int refresh_issue_deadline_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int issued_forced_refresh_commands_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
int num_issued_refreshes_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];

long long int read_queue_length_DRAM[MAX_NUM_CHANNELS_DRAM];
long long int write_queue_length_DRAM[MAX_NUM_CHANNELS_DRAM];

// Stats
long long int num_read_merge_DRAM ;
long long int num_write_merge_DRAM ;
long long int stats_reads_merged_per_channel_DRAM[MAX_NUM_CHANNELS_DRAM];
long long int stats_writes_merged_per_channel_DRAM[MAX_NUM_CHANNELS_DRAM];
long long int stats_reads_seen_DRAM[MAX_NUM_CHANNELS_DRAM];
long long int stats_writes_seen_DRAM[MAX_NUM_CHANNELS_DRAM];
long long int stats_reads_completed_DRAM[MAX_NUM_CHANNELS_DRAM];
long long int stats_writes_completed_DRAM[MAX_NUM_CHANNELS_DRAM];

double stats_average_read_latency_DRAM[MAX_NUM_CHANNELS_DRAM];
double stats_average_read_queue_latency_DRAM[MAX_NUM_CHANNELS_DRAM];
double stats_average_write_latency_DRAM[MAX_NUM_CHANNELS_DRAM];
double stats_average_write_queue_latency_DRAM[MAX_NUM_CHANNELS_DRAM];

long long int stats_page_hits_DRAM[MAX_NUM_CHANNELS_DRAM];
double stats_read_row_hit_rate_DRAM[MAX_NUM_CHANNELS_DRAM];

// Time spent in various states
long long int stats_time_spent_in_active_standby_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_time_spent_in_active_power_down_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_time_spent_in_precharge_power_down_fast_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_time_spent_in_precharge_power_down_slow_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_time_spent_in_power_up_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int last_activate_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int last_refresh_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
double average_gap_between_activates_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
double average_gap_between_refreshes_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_time_spent_terminating_reads_from_other_ranks_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_time_spent_terminating_writes_to_other_ranks_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];

// Command Counters
long long int stats_num_activate_read_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
long long int stats_num_activate_write_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
long long int stats_num_activate_spec_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
long long int stats_num_activate_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_num_precharge_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
long long int stats_num_read_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
long long int stats_num_write_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM][MAX_NUM_BANKS_DRAM];
long long int stats_num_powerdown_slow_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_num_powerdown_fast_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];
long long int stats_num_powerup_DRAM[MAX_NUM_CHANNELS_DRAM][MAX_NUM_RANKS_DRAM];



// functions

                                // to get log with base 2
unsigned int log_base2(unsigned int new_value);

// initialize memory_controller variables
void init_memory_controller_vars_DRAM();

// called every cycle to update the read/write queues
void update_memory_DRAM();

// activate to bank allowed or not
int is_activate_allowed_DRAM(int channel, int rank, int bank);

// precharge to bank allowed or not
int is_precharge_allowed_DRAM(int channel, int rank, int bank);

// all bank precharge allowed or not
int is_all_bank_precharge_allowed_DRAM(int channel, int rank);

// autoprecharge allowed or not
int is_autoprecharge_allowed_DRAM(int channel,int rank,int bank);

// power_down fast allowed or not
int is_powerdown_fast_allowed_DRAM(int channel,int rank);

// power_down slow allowed or not
int is_powerdown_slow_allowed_DRAM(int channel,int rank);

// powerup allowed or not
int is_powerup_allowed_DRAM(int channel,int rank);

// refresh allowed or not
int is_refresh_allowed_DRAM(int channel,int rank);


// issues command to make progress on a request
int issue_request_command_DRAM(request_t * req);

// power_down command
int issue_powerdown_command_DRAM(int channel, int rank, command_t cmd);

// powerup command
int issue_powerup_command_DRAM(int channel, int rank);

// precharge a bank
int issue_activate_command_DRAM(int channel, int rank, int bank, long long int row);

// precharge a bank
int issue_precharge_command_DRAM(int channel, int rank, int bank);

// precharge all banks in a rank
int issue_all_bank_precharge_command_DRAM(int channel, int rank);

// refresh all banks
int issue_refresh_command_DRAM(int channel, int rank);

// autoprecharge all banks
int issue_autoprecharge_DRAM(int channel, int rank, int bank);

// find if there is a matching write request
int read_matches_write_or_read_queue_DRAM(long long int physical_address);

// find if there is a matching request in the write queue
int write_exists_in_write_queue_DRAM(long long int physical_address);

// enqueue a read into the corresponding read queue (returns ptr to new node)
request_t* insert_read_DRAM(long long int physical_address, long long int arrival_cycle, int thread_id, int instruction_id, long long int instruction_pc);

// enqueue a write into the corresponding write queue (returns ptr to new_node)
request_t* insert_write_DRAM(long long int physical_address, long long int arrival_time, int thread_id, int instruction_id);

// update stats counters
void gather_stats_DRAM(int channel);

// print statistics
void print_stats_DRAM();

// calculate power for each channel
float calculate_power_DRAM(int channel, int rank, int print_stats_type, int chips_per_rank);

// get_twin
long long int get_twin(long long int physical_address);
#endif // __MEM_CONTROLLER_HH__

