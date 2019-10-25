
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<inttypes.h>
#include<stdbool.h>
#include"/home/gattaca4/gururaj/LOCAL_LIB/zlib-1.2.11_install/include/zlib.h" //added to support GAP traces

#include "processor.h"
#include "configfile.h"
#include "memory_controller.h"
#include "scheduler.h"
#include "params.h"
#include "randomize.h"
#include "mcache.h"
#include "global_types.h"
#include "os.h"
#include "memOrg.h"
#include "stats.h"
#include "memsys_control_flow.h"
#include "ctr_sim.h"
#include "filereader.h"

#define NON_TR_ARGS 16

//Execute 500 Million inst each
//#define MAXEXECUTE 10000000
#define MAXEXECUTE 100000000
#define WARMUPINST 1

//#define MAXEXECUTE 1000000000
//#define WARMUPINST  500000000

#define MAXMOD 100
#define L3_LATENCY 40



//Notify if it uses virtual or physical address
#define USE_PHY_ADDR 0

char temp[20];
char* return_fgets = NULL;
char* return_fgets1 = NULL;
char* cfg_filename   = "../input/SGX_Baseline_16Gmem.cfg";


char** read_buffer;
char scratch_buffer[MAXTRACELINESIZE];

int* read_buf_index;
int* valid_buf_length;

/*************************************/
//Variables for reseting statistics after warmup

int num_warmup_completed = 0;
int warmup_completed = 0;

/***************************************/
//Added by Gururaj
unsigned long long int       max_inst_warmup   =  (WARMUPINST);
unsigned long long int       max_inst_exec     =  (MAXEXECUTE);
unsigned long long int       maxexecute_2nd    = 0;

unsigned long long int       MET_SIZE_KB       = 128;
unsigned long long int       MET_ASSOC         = 8;
unsigned long long int       MET_REPL          = 0; //0:LRU 1:RND 2:SRRIP
unsigned long long int       MET_PRIVATE       = 0; // 0-shared
unsigned long long int       IS_MET_WRITEBACK  = 1; // 1-writeback,0-writethrough

unsigned long long int       MAC_SIZE_BYTES    = 16*64; //4 Cachelines of MACs
unsigned long long int       MAC_ASSOC         = 16;
unsigned long long int       MAC_REPL          = 0; //0:LRU 1:RND 2:SRRIP
unsigned long long int       MAC_PRIVATE       = 0; // 0-shared
unsigned long long int       IS_MAC_WRITEBACK  = 1; // 1-writeback,0-writethrough

//////////////////

unsigned long long int       L3_SIZE_KB       = 8192;
unsigned long long int       L3_ASSOC         = 8;
unsigned long long int       L3_PRIVATE       = 0; // default is shared L3
unsigned long long int       L3_REPL          = 3; //0:LRU 1:RND 2:SRRIP 6: BRRIP
unsigned long long int       L3_PERFECT       = 0; //simulate 100% hit rate for L3

int PART_SIZE = 8;
//int SGX_MODE = 1;
int SGX_MODE[4] = {1,1,1,1};

int OS_PAGE_MAPPING = 0; //(0 - Random Mapping -4 KB, 1 - Random Mapping - 2MB, 2 - First touch Mapping -4 KB)

//For deciding CTR_DESIGN
extern int CTR_DESIGN;
extern int MTREE_CTR_DESIGN_VAR[5];
extern int* MTREE_CTR_DESIGN;

int COUNTER_UPDATE_POLICY = 0; //0-normal, 1 - smooth

//Added by Gururaj
cache_stats* L3_Cache_Stats  = NULL;
cache_stats* MAC_Cache_Stats = NULL;
cache_stats* MET_Cache_Stats = NULL;
//cache_stats* MET_tot_Cache_Stats = NULL;

cache_sim_stats* L3_stats  ;
cache_sim_stats* MAC_stats  ;
cache_sim_stats* MET_stats  ;
//cache_sim_stats* MET_tot_stats ;
cache_sim_stats* L3_stats_inst  ;
cache_sim_stats* MAC_stats_inst  ;
cache_sim_stats* MET_stats_inst  ;
//cache_sim_stats* MET_tot_stats_inst  ;

read_lat_sim_stat* lat_sim_stat;

mem_stats_t* mem_stats = NULL;
mem_stats_t* mem_stats_inst = NULL;

//Variables for memory access type
maccess_type data_access = {DATA,-1};
maccess_type mac_access = {MAC,-1};
maccess_type met_access = {MET,0};

//Variables for customizing data logger
unsigned int DETAILED_PROC_STATS = 0;
unsigned int DETAILED_MEM_STATS = 0;
unsigned int DETAILED_INST_STATS = 0;
unsigned int DETAILED_CACHE_STATS = 0;
unsigned int DETAILED_MET_STATS = 1;

/***** PARAM FOR ROB*****/
long long int BIGNUM = 10000000;

/********************** MEMORY LATENCY AND ROW BUFFER HIT COUNTERS***********/
long long int *memory_rd_inst=NULL;
long long int *memory_rd=NULL;
long long int *memory_wr_inst=NULL;
long long int *memory_wr=NULL;
long long int *memory_req_inst=NULL;
long long int *memory_req=NULL;

long long int *memory_rdqueue_inst=NULL;
long long int *memory_rdqueue=NULL;
long long int *memory_wrqueue_inst=NULL;
long long int *memory_wrqueue=NULL;
long long int *memory_reqqueue_inst=NULL;
long long int *memory_reqqueue=NULL;

long long int *memory_rdhit_inst=NULL;
long long int *memory_rdhit=NULL;
long long int *memory_wrhit_inst=NULL;
long long int *memory_wrhit=NULL;
long long int *memory_reqhit_inst=NULL;
long long int *memory_reqhit=NULL;

long long int *memory_rd_ch_inst=NULL;
long long int *memory_rd_ch=NULL;
long long int *memory_wr_ch_inst=NULL;
long long int *memory_wr_ch=NULL;
long long int *memory_req_ch_inst=NULL;
long long int *memory_req_ch=NULL;

long long int *memory_rdqueue_ch_inst=NULL;
long long int *memory_rdqueue_ch=NULL;
long long int *memory_wrqueue_ch_inst=NULL;
long long int *memory_wrqueue_ch=NULL;
long long int *memory_reqqueue_ch_inst=NULL;
long long int *memory_reqqueue_ch=NULL;

long long int *memory_rdhit_ch_inst=NULL;
long long int *memory_rdhit_ch=NULL;
long long int *memory_wrhit_ch_inst=NULL;
long long int *memory_wrhit_ch=NULL;
long long int *memory_reqhit_ch_inst=NULL;
long long int *memory_reqhit_ch=NULL;

long long int memory_rd_total_inst=0;
long long int memory_rd_total=0;
long long int memory_wr_total_inst=0;
long long int memory_wr_total=0;
long long int memory_req_total_inst=0;
long long int memory_req_total=0;

long long int memory_rdqueue_total_inst=0;
long long int memory_rdqueue_total=0;
long long int memory_wrqueue_total_inst=0;
long long int memory_wrqueue_total=0;
long long int memory_reqqueue_total_inst=0;
long long int memory_reqqueue_total=0;

long long int memory_rdhit_total_inst=0;
long long int memory_rdhit_total=0;
long long int memory_wrhit_total_inst=0;
long long int memory_wrhit_total=0;
long long int memory_reqhit_total_inst=0;
long long int memory_reqhit_total=0;

/*********************Variables added by Prashant for stats*********************************/
//Stall Loggers
long long int robf_stalls=0;
long long int wrqf_stalls=0;
long long int robn_stalls=0;
long long int wrqn_stalls=0;

/******************** POWER LOGGERS **************************/
double usimm_total_rdcycle = 0;
double usimm_total_wrcycle = 0;
double usimm_total_rdother_cycle = 0;
double usimm_total_wrother_cycle = 0;
double usimm_total_pre_pdn_fastcycle = 0;
double usimm_total_pre_pdn_slowcycle = 0;
double usimm_total_act_pdncycle = 0;
double usimm_total_act_stbycycle = 0;
double usimm_total_pre_stbycycle = 0;

double usimm_total_bck = 0;
double usimm_total_act = 0;
double usimm_total_rd = 0;
double usimm_total_wr = 0;
double usimm_total_rd_term = 0;
double usimm_total_wr_term = 0;
double usimm_total_rdoth_term = 0;
double usimm_total_wroth_term = 0;
double usimm_ref_pwr = 0;
double usimm_total_pwr = 0;

/******************************************************************************************/
/* SIMULATOR VARIABLES*/
int expt_done=0;  
int trace_compressed=1;
int optimized_gzread=1;
int struct_traces = 1;

int sim_mode = 1; //Indicates whether the simulation mode is (0 - just normal timing, 1 - non-timing, or 2 - nontiming followed by timing.

int sim_fast = 1; //Indicates whether ROB needs to be stalled or not (1 - dont stall on reads)
int sim_useL3 = 0; //SPEC - 0 (dont use L3), GAP - 1(use L3).

int COMP_CTR_MODE = 0; //Mode that decides how compressed counters are used.
int COMPRESSED_MTREE = 1; // Ensures that Mtree is compressed, when COMP_CTR_MODE == 1 is used.
int COMPRESSED_CTR = 0; //Ensures that ctrs are regular

int MAJOR_CTR_BITLEN = 50;
int MINOR_CTR_BITLEN = 7;

long long int CYCLE_VAL=0;
long long int CYCLE_VAL_TEMP=0;
/******************* ACCERALATED CORE CYCLES **********************************************/

long long int *CYCLE_VAL_CORE=NULL;
int *core_mod;
int *core_run;
long long int CYCLE_FINEGRAIN=0;

/******************************************************************************************/

unsigned int num_cores = 0;
int priority_thread=0;
int priority_thread_old=0;
int priority_weight=-1;
int priority=0;

/*******************************************************************************************/

unsigned long long int allocation_counter[4]; //for 4 processors
unsigned long long int virtual_hit[4];
unsigned long long int extra_space[4];
unsigned long long int allocation_limit[4];
unsigned long long int extra_space_limit[4];
double utility[4];
int index_max, index_min, index_max2, index_min2;

/****************************************************************************************/

long long int get_current_cycle()
{
  return CYCLE_VAL;
}

struct robstructure *ROB;

//Memory Organization
memOrg_t *mem_org;
ctr_cl**  ctr_cls;
ctr_type** ctr_types;
overflows_stat* ctr_overflows_levelwise_warmup;
overflows_stat* ctr_overflows_levelwise;
overflows_stat*   ctr_overflows_levelwise_inst;


MCache *L3Cache[MAX_THREADS];
MCache *METCache[MAX_THREADS];
MCache *MACCache[MAX_THREADS];

int LAT_6EC7ED;

FILE **tif=NULL;  /* The handles to the trace input files. */
FILE *config_file=NULL;
FILE *vi_file=NULL;

/* Logging currently disabled */
FILE *Policyfile=NULL;
FILE *Policyfile2=NULL;
FILE *Policyfile3=NULL;
FILE *Policyfile4=NULL;
FILE *PolicyfileOverflows=NULL;
#ifdef HIST_NZ
FILE *PolicyfileHistNz=NULL;
char PolicyFileHistNz_name[256] ;
#endif

char* PolicyFile2_suffix = "_pgwise";
char PolicyFile2_name[256] ;
char* PolicyFile3_suffix = "_treeleaf";
char PolicyFile3_name[256] ;
char* PolicyFile4_suffix = "_treeparent";
char PolicyFile4_name[256] ;
char* PolicyFileOverflows_suffix = "_overflows";
char PolicyFileOverflows_name[256] ;
  
/*****************************/

int *prefixtable;
// Moved the following to memory_controller.h so that they are visible
// from the scheduler.
uns64 *committed;
long long int *fetched;
long long int *time_done;
uns64 total_time_done=0;
float core_power=0;

int main(int argc, char * argv[])
{

  printf("--------------------------------------------\n");
  printf("-- USIMM: the Utah SImulated Memory Module --\n");
  printf("--              Version: 1.3               --\n");
  printf("---------------------------------------------\n");

  printf("Using ZLIB Version: %s\n",ZLIB_VERSION);
  srand(43);
  
  int ii=0;
  unsigned int l3sets=0;
  unsigned int l3outcome=0;

  unsigned int dirty=0;
  MCache_Entry evicted_entry;


  //Added by Gururaj
  unsigned int metsets=0;
  unsigned int macsets=0;
  ADDR mac_paddr =0;
  ctr_mtree_entry met_entry;


  //Additional stats for post-warmup
  unsigned long long int *inst_done_stat;
  long long int CYCLE_VAL_stat =0;
  long long int *CYCLE_VAL_CORE_stat=NULL;

  int numc=0;
  int num_ret=0;
  int num_fetch=0;
  int num_done=0;
  /*********** Adding Read and Write Queues into USIMM ******/
  int writeqfull=0;
  /**********************************************************/
  int fnstart;
  int currMTapp;
  long long int maxtd;
  char newstr[MAXTRACELINESIZE];
  char* traceline;
  trace_line_struct next_traceline ;
  
  unsigned long long int *nonmemops;
  char *opertype;
  long long int *addr;
  long long int *instrpc;
  int chips_per_rank=-1;
  uns64 total_inst_fetched = 0;
  uns64 data_mpki = 0;
  uns64 non_overflow_mpki = 0;  
  int fragments=1;
  //Variable to check if Physical Addresses are being used as input
  //int use_phy_addr=USE_PHY_ADDR;
  unsigned long long int phy_addr=0;
  INT_64 os_visible_memory = 0;
  OS *os;


  //To keep track of how much is done
  unsigned long long int inst_comp=0;
  unsigned long int Mcount=0;
  //mem flag
  int mem_flag=1;
  int mem_prio=0;

  // To keep track of instructions per core
  unsigned long long int *inst_exec;
  unsigned long long int *inst_done;
  int *flag; 
  //unsigned long long int maxexecute=MAXEXECUTE;
  unsigned long long int* maxexecute= (unsigned long long int*)calloc(NUMCORES,sizeof(unsigned long long int));
  for(int i=0;i<NUMCORES;i++){
    maxexecute[i] = (MAXEXECUTE);
  }

  double total_ipc=0;


  //Keep Track of Occupancy , Hit Rate and Eviction Rate per Thread in Cumulative Manner
  double *occupancy_logger;
  unsigned long long int *hits;
  unsigned long long int *cacherequests;
  double *hitrate_logger;
  unsigned long long int total_requests=1;

  //Keep Track of Hit Rate and Eviction Rate per Thread in an Instantaneous Manner
  double *occupancy_logger_inst;
  unsigned long long int *hits_inst;
  unsigned long long int *cacherequests_inst;
  double *hitrate_logger_inst;
  unsigned long long int total_requests_inst=1;

  //IPC and ROB Stall Loggers

  double *ipc_logger;
  double *ipc_logger_inst;
  unsigned long long int *count_i;
  unsigned long long int *count_i_inst;
  unsigned long long int *rob_logger;
  unsigned long long int *rob_logger_inst;
  double ipc_logger_total=0;
  double ipc_logger_inst_total=0;
  uns64 count_i_total=0;
  unsigned long long int count_i_inst_total=0;
  uns64 count_i_fast_slow = 0; //Variable that keeps track of total instructions executed throughout the simulation (including fast and slow modes).
  uns64 count_500mn_fast_slow = -1; //Number of 500mn chunks in count_i_fast_slow
    
  /* Initialization code. */
  printf("Initializing.\n");
  // ./sim A B C D E F G ...


  //1.A  = (sim_mode) (0/1 equals sim_fast, 2->sim_fast = 1 then 0.)
  //2.B  = L3_SIZE (in KB)
  //3.C  = L3_ASSOC
  //4.D  = COMPRESSED_MTREE (0 - none, 1 - compression for all Mtree levels, 2 - only for leaf. 
  //5.E  = maxexecute_2nd (For 2nd round of simulation)
  //6.F = Metadata Size in KB
  //7.G= OS_PAGE_MAPPING (0 - Random Mapping -4 KB, 1 -Rand Mapping 2MB, 2 - First Touch 4KB 
  //8.H  = Counter Design (1- SGX-Mono8, 4 Split64, refer memOrg.h for list)
  //9.I.4  = Mtree Counter Design (1- SGX-Mono8, 4 Split64, refer memOrg.h for list) [Default beyond Great Grandparent]]
  //10.J = max_inst_exec num_instructions to simulate
  //11 K = SGX_MODES for Core 0-3 (0 for non-sgx, 1 for commercial sgx, 2 for Chipkill, 3 for Synergy, 4 for Synergy++, 5 for freeCtr_wMAC)
  //12-15 I.0-3 = Mtree Counter Design (1- SGX-Mono8, 4 Split64, refer memOrg.h for list) [Leaf,Parent,GrandParent, Great-GrandParent]
  //16. L = Logfile name
 
  //17. M = trace files


  /***************************/
  /*DEPRECATED **/
  //5.E  = sim_useL3 (0 for SPEC, 1 for GAP)
  
  //4.D  = comp_ctr_mode (0 - none, 1 - compression for overflows, 2 - compression for metcache_perf, 3- compression for overflows + metcache_perf
  //COMP_CTR_MODE = atoi(argv[4]);
  
  // *DEPRECATED* 6. F= Maximum Instructions to Exec (in Millions)
  //  max_inst_exec   = atoi(argv[6])*1000000;
  //  maxexecute = max_inst_exec;
  max_inst_exec   = (MAXEXECUTE);
  max_inst_warmup = (WARMUPINST);

  /*DEPRECATED
  //1. A = L3_REPL (0:LRU 1:RND 2:SRRIP, 3:DRRIP)
  L3_REPL = atoi(argv[1]);
  */

  /* DEPRECATED
  //4.D  = trace_compressed (for compressed traces (=1) )
  trace_compressed = atoi(argv[4]);
  */
  trace_compressed = 1; // by default traces are compressed.

  /* DEPRECATED  
  //5.E  = sim_useL3 (0 for SPEC, 1 for GAP)
  sim_useL3 = atoi(argv[5]);
  */
  
  Policyfile = (FILE *)malloc(sizeof(FILE)*2);
  Policyfile2 = (FILE *)malloc(sizeof(FILE)*2);
  Policyfile3 = (FILE *)malloc(sizeof(FILE)*2);
  Policyfile4 = (FILE *)malloc(sizeof(FILE)*2);

  /* ******************** */
  //1.A  = sim_fast (enforce ROB stalls on reads if 0, else run without timing accuracy)
  sim_mode = atoi(argv[1]);                                          
  
  //sim_fast = atoi(argv[1]);

  //2. B = L3_SIZE (in KB)
  L3_SIZE_KB = atoi(argv[2]);

  //3. C = L3_ASSOC 
  L3_ASSOC = atoi(argv[3]);

  // added by choucc 
  // add an argument for refreshing selection
  if (argc <= (NON_TR_ARGS)+1) {
    printf("Need at least one input configuration file and one trace file as argument.  Quitting.\n");
    return -3;
  }

  
  //4.D  = comp_ctr_mode (0 - none, 1 - compression for overflows, 2 - compression for metcache_perf, 3- compression for overflows + metcache_perf
  COMPRESSED_MTREE = atoi(argv[4]);
  if(COMPRESSED_MTREE != 0){
    COMP_CTR_MODE = 1;
  }
  else {
    COMP_CTR_MODE = 0;
  }

  //5.E  = maxexecute_2nd in Millions (For 2nd round of simulation) 
  maxexecute_2nd = atoll(argv[5]) * 1000000 ;
    
  //6. F = Metadata Size in KB
  MET_SIZE_KB = atoi(argv[6]);

  //7. G= OS_PAGE_MAPPING (0 - Random Mapping -4 KB, 1 -Rand Mapping 2MB, 2 - First Touch 4KB, 3 - Rand Mapping 8KB. 
  OS_PAGE_MAPPING = atoi(argv[7]);
  
  //8. H = Counter Design (1- SGX-Mono8, 4 Split64, refer memOrg.h for list)
  CTR_DESIGN = atoi(argv[8]);
  
  //9. I = Mtree Counter Design (1- SGX-Mono8, 4 Split64, refer memOrg.h for list)
  MTREE_CTR_DESIGN_VAR[4] = atoi(argv[9]); //ASSUMED that 0 -3 are leaf,parent,grandparent, great-grandparent, 4- rest of levels

  
  //10. J = Maximum Instructions to Exec (arg in Millions)
  max_inst_exec = atoll(argv[10]) * 1000000 ;

  config_file = fopen(cfg_filename, "r");
  if (!config_file) {
    printf("Missing system configuration file.  Quitting. \n");
    return -4;
  }
 
  NUMCORES = argc-((NON_TR_ARGS)+1);
  num_cores = NUMCORES;

  //11 K = SGX_MODES for Core 0-3 (0 for non-sgx, 1 for commercial sgx, 2 for Chipkill, 3 for Synergy, 4 for Synergy++, 5 for freeCtr_wMAC)  
  ASSERTM(NUMCORES == 4,"Have configured this to work with only 4 cores, since SGX_MODES is 4-long array\n");   
  for(int i=0; i<NUMCORES;i++){
    SGX_MODE[i] = atoi(argv[11]); //Default is 1
  }

  //12-15 L = MTREE_CTR_DESIGN_VAR for leaf,parent,grandparent, great-grandparent
  for(int i=0; i<4;i++){
    MTREE_CTR_DESIGN_VAR[i] = atoi(argv[12+i]);
  }

  /* Manage sim_mode */
  if( (sim_mode == 0) || (sim_mode == 1)){
    sim_fast = sim_mode;
  }
  else if (sim_mode == 2){
    sim_fast = 1; // To start with. Will change to 0 after fast finishes.
  }
  else {
    ASSERTM(0,"Not configured to support modes other than 0,1,2");
  }

  /***************************/
  read_buffer =  (char **)malloc(sizeof(char*)*NUMCORES);
  read_buf_index = (int*) malloc(sizeof(int)*NUMCORES);
  valid_buf_length = (int*) malloc(sizeof(int)*NUMCORES);  
  for(int i=0; i< NUMCORES;i++){
    read_buffer[i] = (char*)malloc(sizeof(char)*BUFFSIZE);
    read_buf_index[i] = -1;
    valid_buf_length[i] = -1;
  }
  
  /***************************/
  /*
  //Opening files to dump overflows counter-wise at end of program
  Policyfile = fopen(argv[16], "w");
  if (!Policyfile) {
    printf("Missing output Policy Logger file.  Quitting. \n");
    return -5;
  }
  strcpy(PolicyFile2_name, argv[16]);
  strcat(PolicyFile2_name, "_pgwise");

  Policyfile2 = fopen(PolicyFile2_name, "w");
  if (!Policyfile2) {
    printf("Missing output Policy Logger file.  Quitting. \n");
    return -5;
  }

  strcpy(PolicyFile3_name, argv[16]);
  strcat(PolicyFile3_name, "_treeleaf");

  Policyfile3 = fopen(PolicyFile3_name, "w");
  if (!Policyfile3) {
    printf("Missing output Policy Logger file.  Quitting. \n");
    return -5;
  }

   
  strcpy(PolicyFile4_name, argv[16]);
  strcat(PolicyFile4_name, "_treeparent");

  Policyfile4 = fopen(PolicyFile4_name, "w");
  if (!Policyfile4) {
    printf("Missing output Policy Logger file.  Quitting. \n");
    return -5;
  }

  */

  //Opening file to dump total overflows (level-wise), vs Inst Count (throughout the program execution)
  strcpy(PolicyFileOverflows_name, argv[16]);
  strcat(PolicyFileOverflows_name, "_overflows");
  
  PolicyfileOverflows = fopen(PolicyFileOverflows_name, "w");
  if (!PolicyfileOverflows) {
    printf("Missing output Policy Logger file.  Quitting. \n");
    return -5;
  }

#ifdef HIST_NZ
  //Opening file to dump Histogram of NonZero Counters at Overflow time.
  strcpy(PolicyFileHistNz_name, argv[16]);
  strcat(PolicyFileHistNz_name,"_HistNz");
  
  PolicyfileHistNz = fopen(PolicyFileHistNz_name, "w");
  if (!PolicyfileHistNz) {
    printf("Missing output Policy Logger file.  Quitting. \n");
    return -5;
  }
#endif  

  /************************/

  //Init Max-Execute
  for(int i=0;i<(NUMCORES);i++){
    maxexecute[i] = (unsigned long long int)(max_inst_exec);
    //printf("Maxexecute for Core %d init to %llu\n",i,maxexecute[i]);
  }
  
  ROB = (struct robstructure *)malloc(sizeof(struct robstructure)*NUMCORES);

  if(trace_compressed == 1) //For compressed
    tif = (FILE **)malloc(sizeof(gzFile *)*NUMCORES);
  else
    tif = (FILE **)malloc(sizeof(FILE *)*NUMCORES);
   
  committed  = (uns64 *)malloc(sizeof(uns64)*NUMCORES);
  fetched = (long long int *)malloc(sizeof(long long int)*NUMCORES);
  time_done = (long long int *)malloc(sizeof(long long int)*NUMCORES);
  nonmemops = (unsigned long long int*)malloc(sizeof(int)*NUMCORES);

  opertype = (char *)malloc(sizeof(char)*NUMCORES);
  addr = (long long int *)malloc(sizeof(long long int)*NUMCORES);
  instrpc = (long long int *)malloc(sizeof(long long int)*NUMCORES);
  prefixtable = (int *)malloc(sizeof(int)*NUMCORES);
  inst_exec = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  inst_done = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  flag = (int*)malloc(sizeof(int)*NUMCORES);


  //occupancy, hitrate  rate loggers
  occupancy_logger = (double*)malloc(sizeof(double)*NUMCORES);
  hitrate_logger = (double*)malloc(sizeof(double)*NUMCORES);
  hits = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  cacherequests = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);


  occupancy_logger_inst = (double*)malloc(sizeof(double)*NUMCORES);
  hitrate_logger_inst = (double*)malloc(sizeof(double)*NUMCORES);
  hits_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  cacherequests_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);

  
  //IPC and ROB stall loggers
  ipc_logger_inst = (double*)malloc(sizeof(double)*NUMCORES);
  ipc_logger = (double*)malloc(sizeof(double)*NUMCORES);
  count_i_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  count_i = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  rob_logger_inst = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  rob_logger = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);


  //Initialize variables required for post-warmup stats
  inst_done_stat = (unsigned long long int*)malloc(sizeof(unsigned long long int)*NUMCORES);
  CYCLE_VAL_CORE_stat = (long long int*)calloc(NUMCORES,sizeof(long long int));

  currMTapp = -1;
  // added by choucc 
  // add an argument for refreshing selection

  //Open each file
  for (numc=0; numc < NUMCORES; numc++) {
  
    if(trace_compressed == 1) //For compressed
      tif[numc] = gzopen(argv[numc+ (NON_TR_ARGS)+1], "r");
    else
      tif[numc] = fopen(argv[numc+ (NON_TR_ARGS)+1], "r");

    if (!tif[numc]) {
      printf("Trying to open trace file : %s",argv[numc+ (NON_TR_ARGS)+1]);
      printf("Missing input trace file %d.  Quitting. \n",numc);
      return -5;
    }

    /* The addresses in each trace are given a prefix that equals
       their core ID.  If the input trace starts with "MT", it is
       assumed to be part of a multi-threaded app.  The addresses
       from this trace file are given a prefix that equals that of
       the last seen input trace file that starts with "MT0".  For
       example, the following is an acceptable set of inputs for
       multi-threaded apps CG (4 threads) and LU (2 threads):
       usimm 1channel.cfg MT0CG MT1CG MT2CG MT3CG MT0LU MT1LU */
    prefixtable[numc] = numc;

    /* Find the start of the filename.  It's after the last "/". */
    for (fnstart = strlen(argv[numc+(NON_TR_ARGS)+1]) ; fnstart >= 0; fnstart--) {
      if (argv[numc+ (NON_TR_ARGS)+1][fnstart] == '/') {
        break;
      }
    }
    fnstart++;  /* fnstart is either the letter after the last / or the 0th letter. */

    if ((strlen(argv[numc+ (NON_TR_ARGS)+1])-fnstart) > 2) {
      if ((argv[numc+ (NON_TR_ARGS)+1][fnstart+0] == 'M') && (argv[numc+ (NON_TR_ARGS)][fnstart+1] == 'T')) {
        if (argv[numc+(NON_TR_ARGS)+1][fnstart+2] == '0') {
          currMTapp = numc;
        }
        else {
          if (currMTapp < 0) {
            printf("Poor set of input parameters.  Input file %s starts with \"MT\", but there is no preceding input file starting with \"MT0\".  Quitting.\n", argv[numc+(NON_TR_ARGS)+1]);
            return -6;
          }
          else 
            prefixtable[numc] = currMTapp;
        }
      }
    }
   
    printf("Core %d: Input trace file %s : Addresses will have prefix %d\n", numc, argv[numc+ (NON_TR_ARGS)+1], prefixtable[numc]);

    committed[numc]=0;
    fetched[numc]=0;
    time_done[numc]=0;
    inst_exec[numc]=0;
    inst_done[numc]=0;
    inst_done_stat[numc]=0;
    flag[numc]=0;
    ROB[numc].head=0;
    ROB[numc].tail=0;
    ROB[numc].inflight=0;
    ROB[numc].tracedone=0;
    
  }

  read_config_file(config_file);
   

  vi_file = fopen("../input/4Gb_x8.vi", "r");
  chips_per_rank= 9;
  printf("Reading vi file: 4Gb_x8.vi\t\n%d Chips per Rank\n",chips_per_rank);

  if (!vi_file) {
    printf("Missing DRAM chip parameter file.  Quitting. \n");
    return -5;
  }

  assert((log_base2(NUM_CHANNELS) + log_base2(NUM_RANKS) + log_base2(NUM_BANKS) + log_base2(NUM_ROWS) + log_base2(NUM_COLUMNS) + log_base2(CACHE_LINE_SIZE)) == ADDRESS_BITS );
  read_config_file(vi_file);
  fragments=1;
  T_RFC=T_RFC/fragments;
  
  printf("Fragments: %d of length %d\n",fragments, T_RFC);

  print_params();

  ASSERTM(L3_PRIVATE==0, "Design assumes that L3 is shared");
  ASSERTM(MAC_PRIVATE==0, "Design assumes that MAC Cache is shared");
  ASSERTM(MET_PRIVATE==0, "Design assumes that MET Cache is shared"); 
  
  // Memory Loggers are initialized here
  memory_rd_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_rd = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_wr_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_wr = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_req_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_req = (long long int*)calloc(NUMCORES,sizeof(long long int));

  memory_rdqueue_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_rdqueue = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_wrqueue_inst= (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_wrqueue = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_reqqueue_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_reqqueue = (long long int*)calloc(NUMCORES,sizeof(long long int));

  memory_rdhit_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_rdhit = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_wrhit_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_wrhit = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_reqhit_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
  memory_reqhit = (long long int*)calloc(NUMCORES,sizeof(long long int));

  memory_rd_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_rd_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_wr_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_wr_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_req_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_req_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));

  memory_rdqueue_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_rdqueue_ch= (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_wrqueue_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_wrqueue_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_reqqueue_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_reqqueue_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));

  memory_rdhit_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_rdhit_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_wrhit_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_wrhit_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_reqhit_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
  memory_reqhit_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));

  // Initialize the cycle val for each core
  CYCLE_VAL_CORE = (long long int*)calloc(NUMCORES,sizeof(long long int));

  core_run = (int*)calloc(NUMCORES,sizeof(int));
  core_mod = (int*)calloc(NUMCORES,sizeof(int));

  for(int i=0; i<NUMCORES; i++) {
    ROB[i].comptime = (long long int*)malloc(sizeof(long long int)*ROBSIZE);
    ROB[i].mem_address = (long long int*)malloc(sizeof(long long int)*ROBSIZE);
    ROB[i].instrpc = (long long int*)malloc(sizeof(long long int)*ROBSIZE);
    ROB[i].optype = (int*)malloc(sizeof(int)*ROBSIZE);
    ROB[i].critical_mem_ops = (int*)calloc(ROBSIZE,sizeof(int));
    ROB[i].read_lat_stat  = (read_lat_rob_stat*)calloc(ROBSIZE,sizeof(read_lat_rob_stat)); 
    
  }

  
  //Configure L3 SRAM Cache Params : Added By Prashant Nair
  l3sets= (L3_SIZE_KB*1024)/(L3_ASSOC*CACHE_LINE_SIZE);

  for(ii=0; ii<NUMCORES; ii++){
    L3Cache[ii] = mcache_new(l3sets, L3_ASSOC, L3_REPL,0);
  }

  //Configure Metadata Cache & MAC Cache : Added by Gururaj - DONE_TODO
  // A) Create MetCache (32KB, 8-way metcache)
  // B) Create MAC_STRORE (4 cacheline, fully associative)            

    
  metsets = (MET_SIZE_KB*1024)/(MET_ASSOC*CACHE_LINE_SIZE);
  macsets = MAC_SIZE_BYTES/(MAC_ASSOC*CACHE_LINE_SIZE);

  assert(macsets == 1);
  for(ii=0; ii<NUMCORES; ii++){
    if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3) ){
      METCache[ii] = mcache_new(metsets, MET_ASSOC, MET_REPL,1);
    } else {
      METCache[ii] = mcache_new(metsets, MET_ASSOC, MET_REPL,0);
    }
    
    MACCache[ii] = mcache_new(macsets, MAC_ASSOC, MAC_REPL,0);
  }
  //////////////////////
  
  for(ii=0; ii<NUMCORES; ii++){

    occupancy_logger[ii]=0;
    hitrate_logger[ii]=0;
    hits[ii]=0;
    cacherequests[ii]=1;
    total_requests=1;


    occupancy_logger_inst[ii]=0;
    hitrate_logger_inst[ii]=0;
    hits_inst[ii]=0;
    cacherequests_inst[ii]=1;
    total_requests_inst=1;

    ipc_logger_inst[ii] = 0;
    ipc_logger[ii] = 0;
    count_i_inst[ii] = 0;
    count_i[ii] = 0;
    rob_logger_inst[ii] = 0;
    rob_logger[ii] = 0;



    if(ii==0)
      core_mod[ii]=100;
    else
      core_mod[ii]=100;
  }


  //Initializing Data-Structure for Memory Organization Management for SGX
  mem_org = (memOrg_t*) malloc(sizeof(memOrg_t));
  init_memOrg(ADDRESS_BITS, mem_org);
  
  ctr_types = (ctr_type**) calloc((mem_org->num_Mtree_levels + 1),sizeof(ctr_type*));
  ctr_cls = (ctr_cl**) calloc ((mem_org->num_Mtree_levels + 1),sizeof(ctr_cl*)); 
  ctr_overflows_levelwise_warmup = (overflows_stat*) calloc ((mem_org->num_Mtree_levels + 1),sizeof(overflows_stat));
  ctr_overflows_levelwise = (overflows_stat*) calloc ((mem_org->num_Mtree_levels + 1),sizeof(overflows_stat));
  ctr_overflows_levelwise_inst = (overflows_stat*) calloc((mem_org->num_Mtree_levels + 1),sizeof( overflows_stat));
  
  for(int i=mem_org->num_Mtree_levels; i>=0 ; i--){
    ctr_types[i] = (ctr_type*) calloc (1,sizeof(ctr_type));
    init_ctr_types(mem_org, ctr_types[i], i);
    ctr_cls[i] = (ctr_cl*) calloc (ctr_types[i]->num_ctr_cls,sizeof(ctr_cl));
    init_ctr_cls(mem_org, ctr_cls[i], ctr_types[i], i);
  }

  printf("ENCR CTR COMPRESSED: %d\n",ctr_types[mem_org->num_Mtree_levels]->compression_enabled);
  printf("ENCR CTR OVERFLOW BITS: %f\n",log2(ctr_types[mem_org->num_Mtree_levels]->minor_ctr_maxval));
  printf("ENCR CTR DESIGN: %d\n",CTR_DESIGN);

  for(int i=mem_org->num_Mtree_levels-1; i>=0 ; i--){
    printf("MTREE CTR %d COMPRESSED: %d\n",i, ctr_types[i]->compression_enabled);
    printf("MTREE CTR %d OVERFLOW BITS: %f\n",i, log2(ctr_types[i]->minor_ctr_maxval));
    printf("MTREE CTR %d DESIGN: %d\n",i, MTREE_CTR_DESIGN[i]);
  }
  //Added by Gururaj - to ensure CTRs,MACs & other memory not visible to OS.
  os_visible_memory = getOSVisibleMem (mem_org);
  printf("OS_VISBILE_MEMORY: %llu\n",os_visible_memory/(1024*1024));
  

  //Cache Stats
  /*
    Assumptions regarding structure of stats
    -stats[0] - data
    -stats[1] - metadata (counters+merkle Tree)
    -stats[2] - counters
    -stats[3] - merkle tree - n levels   
    -stats[4] - merkle tree - level 0
    ..
    -stats[4+n-1] - merkle tree - level n-1
  */
  L3_Cache_Stats = (cache_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_stats));
  MAC_Cache_Stats = (cache_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_stats));
  MET_Cache_Stats = (cache_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_stats));
  
  L3_stats  = (cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
  MAC_stats = (cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
  MET_stats = (cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));

  L3_stats_inst  =(cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
  MAC_stats_inst =(cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
  MET_stats_inst =(cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));

  //Initializing Cache Stats
  
  for(int ii=0;ii< (5+mem_org->num_Mtree_levels) ;ii++){
    init_cache_stats(&(L3_Cache_Stats[ii]),1);
    init_cache_stats(&(MAC_Cache_Stats[ii]),1);
    init_cache_stats(&(MET_Cache_Stats[ii]),1);
  }

  //Initializing Read Latency Sim Stats
  lat_sim_stat = (read_lat_sim_stat*) calloc(1, sizeof(read_lat_sim_stat)); 
  init_read_lat_sim_stat(lat_sim_stat,mem_org);

  
  for(int i=0; i<NUMCORES; i++) {

    for(int j = 0 ; j<ROBSIZE; j++){
      init_read_lat_rob_stat(&(ROB[i].read_lat_stat[j]), mem_org);
    }

  }
  //////////////////////////////////

  //Initialize Memory Stats - added by Gururaj
  mem_stats = init_mem_stats(mem_org->num_Mtree_levels);
  mem_stats_inst = init_mem_stats(mem_org->num_Mtree_levels);
  ///////////////////


  init_memory_controller_vars();
  init_scheduler_vars();
  printf("\nL3_Cache_Type\t: %llu",L3_PRIVATE);
  init_cache_vars(L3Cache[0], "L3");
  init_cache_vars(METCache[0], "Meta");
  init_cache_vars(MACCache[0], "MAC");
  printf("\nPART_SIZE\t: %d", PART_SIZE);
  printf("\nPriority \t: %d\n", priority);

          
  for(int i=0;i<NUMCORES;i++){
    switch(SGX_MODE[i]){
    case 0: printf("\nSGX_MODE[%d]\t: %d (NO_SGX)\n",i, SGX_MODE[i]);
      break;
      
    case 1: printf("\nSGX_MODE[%d]\t: %d (COMMERCIAL_SGX)\n",i, SGX_MODE[i]);
      break;
         
    case 2: printf("\nSGX_MODE[%d]\t: %d (COMMERCIAL_SGX with CHIPKILL ECC)\n",i, SGX_MODE[i]);
      break;
         
    case 3: printf("\nSGX_MODE[%d]\t: %d (COMMERCIAL_SGX with SYNERGY)\n",i, SGX_MODE[i]);
      break;
     
    case 4: printf("\nSGX_MODE[%d]\t: %d (COMMERCIAL_SGX with SYNERGY++)\n",i, SGX_MODE[i]);
      break;

    case 5: printf("\nSGX_MODE[%d]\t: %d (COMMERCIAL_SGX with FREE CTR)\n",i, SGX_MODE[i]);
      break;

    default: ASSERTM(0,"Not designed to handle SGX_MODE != (0..5)\n"); 
    }
  }
  
  if(IS_MET_WRITEBACK)
    printf("\nMET_CACHE\t: WRITEBACK\n");
  else
    printf("\nMET_CACHE\t: WRITETHROUGH\n");

  if(IS_MAC_WRITEBACK)
    printf("\nMAC_CACHE\t: WRITEBACK\n");
  else
    printf("\nMAC_CACHE\t: WRITETHROUGH\n");

  os = os_new((uns64)os_visible_memory,NUMCORES);

     
  /*********************************** ALL INITIALIZATIONS ARE DONE UNTIL HERE ****************************************/
  /* Must start by reading one line of each trace file. */
  for(numc=0; numc<NUMCORES; numc++) {
    //Read trace line
    //if((sim_fast == 1) && (numc != 0)){
    //Dont do anything. Just use already existing traceline.
    //}
    //else {
    return_fgets = NULL;   
    if( (!optimized_gzread) || (!trace_compressed)){
      if(trace_compressed == 1){ //For compressed
        return_fgets = gzgets(tif[numc],newstr,MAXTRACELINESIZE);
        traceline = newstr;
      }
      else {
        return_fgets = fgets(newstr,MAXTRACELINESIZE,tif[numc]);
        traceline = newstr;
      }
    }
    else {
      if(!struct_traces){
        traceline =  get_line_from_file( tif, numc, read_buffer, read_buf_index,  valid_buf_length, scratch_buffer);
        return_fgets = traceline;
      } else{
        next_traceline = read_trace_line_1rec(tif[numc]);
        return_fgets = 1;
      }
    }

    //  }
    if(return_fgets) {
      inst_comp++;
      inst_exec[numc]++;
      count_i[numc]++;
      count_i_inst[numc]++;
      if(sim_fast != 1){
        count_i_total++;
        count_i_fast_slow++;
      }
      count_i_inst_total++;

      if(sim_useL3 == 1){ //Reading L2 Miss Stream
        if ((sscanf(traceline,"%llu %c",&nonmemops[numc],&opertype[numc]) > 0)) {
          if (opertype[numc] == 'R') {
            if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
              printf("1. Panic.  Poor trace format.\n");
              return -4;
            }
          }
          else {
            if (opertype[numc] == 'W') {
              if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
                printf("2. Panic.  Poor trace format.\n");
                return -3;
              }
            }
            else {
              printf("3. Panic.  Poor trace format.\n");
              return -2;
            }
          }
        }
        else {
          printf("4. Panic.  Poor trace format.\n");
          return -1;
        }
      }
      else{ //Reading L3 Miss Stream

        if(!struct_traces){
          if ((sscanf(traceline,"%llu %c %Lx",&nonmemops[numc], &opertype[numc],&addr[numc]) > 0)) {
            ASSERTM( (opertype[numc] == 'W') || (opertype[numc] == 'R'), "Trace contains some operand other than R or W");              
          }
          else {
            printf("For numc %d, traceline is: %s\n", numc, traceline);
            printf("1. Panic. Poor trace format reading L3 Miss Stream.\n");
            return -1;
          }
        } else {
          nonmemops[numc] = next_traceline.nonmemops;
          opertype[numc] =  next_traceline.read_write;
          addr[numc] =  next_traceline.addr;
        }
      }
          
      phy_addr=os_v2p_lineaddr(os,addr[numc],numc);
      addr[numc]=phy_addr;
      addr[numc]=addr[numc]<<6; //As the cache line is 64 bytes for parsec and spec

      inst_comp=nonmemops[numc]+inst_comp;
      inst_exec[numc]=nonmemops[numc]+inst_exec[numc];

      //Instruction Logger
      count_i[numc]=nonmemops[numc]+count_i[numc];
      count_i_inst[numc]=nonmemops[numc]+count_i_inst[numc];
      count_i_inst_total=nonmemops[numc]+count_i_inst_total;
      if(sim_fast != 1){
        count_i_total=nonmemops[numc]+count_i_total;
        count_i_fast_slow=nonmemops[numc]+count_i_fast_slow;
      }
       
      if(flag[numc]==0){
        inst_done[numc]=inst_exec[numc];	
      }

      if((flag[numc]==0) && (inst_done[numc] > maxexecute[numc])){
        flag[numc]=1;
        time_done[numc]=CYCLE_VAL;
        num_done++;			
      }
    }
    else {
      ROB[numc].tracedone=1;
    }

    //printf("End of loop: For core %d, traceline is %s \n", numc, traceline, newstr);
  }

  
  printf("\n** Starting simulation. **\n");
  
  
  if(sim_fast == 1){
    //Starting Fast Simulation (Warmup)
    while (!expt_done) {
      for (numc = 0; numc < NUMCORES; numc++) {
        //Commit all the non-memops for the current traceline
        committed[numc]+= nonmemops[numc];        
        count_i_total += nonmemops[numc];
        count_i_fast_slow +=nonmemops[numc];
        
        //Perform appropriate action on Read / Write
        if (opertype[numc] == 'R') {
          long long int wb_addr = 0;
          l3outcome=FALSE;
          dirty=FALSE;
          mem_prio = 0;

          if(sim_useL3 == 1){
            //L3 SRAM CACHE OPERATIONS 
            l3outcome = cache_access(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);

          } else { //We have L3 miss traces already
            l3outcome = MISS;
          }
                    
          ///////////// LLC HIT! :) //////////////////////////////////////////////////
          if(l3outcome==HIT) {
            hits[numc]++;
            hits_inst[numc]++;
          }
               
          ///////////////////// WHAT FOLLOWS IS THE CASE IF ITS A LLC MISS////////////
          if(l3outcome==MISS) {
            ////////////////1. READ CACHELINE FROM MEMORY /////////////////////////
            insert_read_memsys(addr[numc], CYCLE_VAL, numc, ROB[numc].tail, instrpc[numc], mem_prio, TRUE, data_access); // Read data from memory                   
            if(SGX_MODE[numc] >= 1){
              //Parts removed since MAC assumed to be in 9th ECC Chip
              //******************************************************
              //// 1. Read MAC from MACCache or from memory
              if(!((SGX_MODE[numc]  == 3) || (SGX_MODE[numc] == 4))){ //Will read MAC from mem in all SGX modes except 3,4 (i.e. 1,2,5)
                mac_paddr = getMACAddr(addr[numc],mem_org);
                read_mac(MACCache, mac_paddr, numc, MAC_Cache_Stats, TRUE,
                         mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
              }
              //*******************************************************            
                      
              // 2. Check if counter/Mtree entry exists in metCache, if Yes - break, if No - go one level up in Counter Cache
              if( !(SGX_MODE[numc] == 5) ){ //Use counters only in all modes except 5.
                int policy_install_metcache = TRUE;//** VARIABLE **
                met_entry = getCounterAddr(addr[numc], mem_org);
                //printf("Data address is: %llu, ctr address is %llu\n", addr[numc],met_entry.paddr);
                read_ctr_mtree(METCache, L3Cache, MACCache, met_entry, numc, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, TRUE,
                               mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
              }
            }
                 
            ////////////////2. INSTALL CACHELINE IN L3 CACHE /////////////////////////
            if(sim_useL3 == 1){
              evicted_entry = cache_install(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);
                  
              ////////////////3. WRITEBACK DIRTY EVICTS TO MEMORY /////////////////////////
              if(evicted_entry.valid && evicted_entry.dirty){ //Write the dirty line in memory 
                wb_addr=((unsigned long long int)evicted_entry.tag)<<6; //Bug squashed by Gururaj
                handle_dirty_wb_LLC(wb_addr, evicted_entry, mem_prio, mem_org, METCache, L3Cache, MACCache, numc,
                                    MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, instrpc);                    
              }
            }
          } 
                
        }              
        else {  // This must be a 'W'.  We are confirming that while reading the trace. 
          if (opertype[numc] == 'W') {
            long long int wb_addr = 0;
            l3outcome=FALSE;
            dirty=TRUE;
            mem_prio = 0;

            if(sim_useL3 == 1){
              //L3 SRAM CACHE OPERATIONS
              l3outcome = cache_access(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);
            }
            else {  //We have L3 miss traces already
              l3outcome = MISS;
            }
                      
            ///////////// LLC HIT! :) //////////////////////////////////////////////////
            if(l3outcome==HIT){
              //Do Nothing , writes are not in the critical path 
              hits[numc]++; 
            }
            ///////////////////// WHAT FOLLOWS IS THE CASE IF ITS A LLC MISS////////////
            if(l3outcome==MISS) {

              if(sim_useL3 == 1){
                evicted_entry = cache_install(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);
              }
              else { //We have L3 miss traces already
                //need to pass the L3 dirty writeback forward
                evicted_entry.valid = TRUE;
                evicted_entry.dirty = TRUE;
                evicted_entry.tag   = addr[numc] >> 6; //tag is a lineaddr
                evicted_entry.tid   = numc;
              }
                        
              if(evicted_entry.valid && evicted_entry.dirty) { //Write the dirty line in memory

                wb_addr=((unsigned long long int)evicted_entry.tag)<<6; //Bug squashed by Gururaj

                if(!write_exists_in_write_queue(wb_addr)) {
                  handle_dirty_wb_LLC(wb_addr, evicted_entry, mem_prio, mem_org, METCache, L3Cache, MACCache, numc,
                                      MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, instrpc);
                }                       
              }
                   
            }
                      
          }
          else {
            printf("5. Panic.  Poor trace format. \n");
            return -1;
          }
          
        }
        // Done consuming one line of the trace file.  
        committed[numc]++;
        count_i_total++;
        count_i_fast_slow++;
        
        //When will expt be done ?
        if(committed[numc] >= maxexecute[numc]){
          if(flag[numc] == 0){
            printf("\n Core %d reached end of its fast execution with %.3f Bn inst committed \n", numc, (double)committed[numc]/(double)1000000000);
            flag[numc]=1;
            time_done[numc]=CYCLE_VAL;
            num_done++;
          }

          if(num_done == NUMCORES){
            expt_done = 1;
          }
        }
        
        //Read in the next line of trace
        //if((sim_fast == 1) && (numc != 0)){
        //Dont do anything. Just use the existing traceline.
        //}
        //else {        
        return_fgets1 = NULL;
        if( (!optimized_gzread) || (!trace_compressed)){

          if(trace_compressed == 1){ //For compressed
            return_fgets1 = gzgets(tif[numc],newstr,MAXTRACELINESIZE) ;
            traceline = newstr;
          }
          else {
            return_fgets1 = fgets(newstr,MAXTRACELINESIZE,tif[numc]) ;
            traceline = newstr;
          }
          
          if(!return_fgets1) {
            if(trace_compressed == 1){ //For compressed
              gzrewind(tif[numc]);
            } else{                                
              rewind(tif[numc]);
            }
            if(trace_compressed == 1){ //For compressed
              return_fgets1 = gzgets(tif[numc],newstr,MAXTRACELINESIZE) ;
              traceline = newstr;
            }
            else{
              return_fgets1 = fgets(newstr,MAXTRACELINESIZE,tif[numc]) ;
              traceline = newstr;
            }
          }
        
          ASSERTM(return_fgets1,"Trace rewinded but still not getting next line successfully.");
        }           
        else {
          if(!struct_traces){
            traceline =  get_line_from_file( tif, numc, read_buffer, read_buf_index,  valid_buf_length, scratch_buffer);
            
            return_fgets1 = traceline;
          } else{
            next_traceline = read_trace_line_1rec(tif[numc]);
            return_fgets1 = 1;
          }
        }        
        //}
        if(return_fgets1) {
          //printf("Y%d",numc);
          if(sim_useL3 == 1){ //Reading L2 Miss Stream
            if (sscanf(traceline,"%llu %c",&nonmemops[numc],&opertype[numc]) > 0) {
              if (opertype[numc] == 'R') {
                if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
                  printf("6. Panic.  Poor trace format.\n");
                  return -4;
                }
              }
              else {
                if (opertype[numc] == 'W') {
                  if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
                    printf("7. Panic.  Poor trace format.\n");
                    return -3;
                  }
                }
                else {
                  printf("8. Panic.  Poor trace format.\n");
                  return -2;
                }
              }
            }
            else {
              printf("9. Panic.  Poor trace format.\n");
              return -1;
            }
          } else{ //Reading L3 Miss Stream
            if(!struct_traces){
              if ((sscanf(traceline,"%llu %c %Lx",&nonmemops[numc], &opertype[numc],&addr[numc]) > 0)) {
                ASSERTM( (opertype[numc] == 'W') || (opertype[numc] == 'R'), "Trace contains some operand other than R or W");              
              }
              else {
                printf("2. Panic. Poor trace format reading L3 Miss Stream.\n");
                return -1;
              }
            } else {
              nonmemops[numc] = next_traceline.nonmemops;
              opertype[numc] =  next_traceline.read_write;
              addr[numc] =  next_traceline.addr;
            }                          
          }
                    
          phy_addr=os_v2p_lineaddr(os,addr[numc],numc); //Returns physical_lineaddr
          addr[numc]=phy_addr;
          addr[numc]=addr[numc]<<6; //We need physical_addr in addr for passing to mem hierarchy. Hence, leftshift


        }
        else {
          ASSERTM(0,"Trace rewinded but still not getting next line successfully.");
        }

        //printf("End of cycle - Core %d, traceline %s, nonmemops %d, addr %lld \n",numc, traceline, nonmemops[numc], addr[numc] );
      } //Finished cycling through NUMCORES

      //For every iteration of the while-loop, dump overflow stats if Inst_Count is multiple of 500Mn
      if(count_500mn_fast_slow != (count_i_fast_slow /500000000)){
        count_500mn_fast_slow = (count_i_fast_slow /500000000);
        //TODO Call dump overflows logger
        log_ctr_overflow_vs_inst(PolicyfileOverflows, count_500mn_fast_slow,mem_org,ctr_overflows_levelwise,ctr_overflows_levelwise_inst);
      }

      
      if (count_i_total > (NUMCORES * Mcount * 10000000) ) {
        fflush(stdout); 
        Mcount++;
        //printf("Committed %llu , %llu, count_i_tot %llu \n", committed[0], committed[1],count_i_total);
        printf(".");

        if((Mcount % 100) == 0){
          printf(" %.2f Bn \n", ((double)count_i_total/(double)(NUMCORES))/((double)1000000000));
        }
              
      }
          
    }
  }

  //Finished experiment with sim_fast, now run slow if sim_mode commands it.

  printf("Fast Simulation Done for %f Bn instructions for %d cores\n",((double)count_i_total)/((double)1000000000), NUMCORES);  

  if(sim_mode == 2){
    sim_fast = 0;
    printf("Starting Slow Simulation for %llu instructions for %d cores\n",maxexecute_2nd, NUMCORES);  
    //Zero out inst_done, reset maxexecute till which to execute
    for(int i=0;i<NUMCORES;i++){
      inst_done[i] = 0;
      inst_exec[i] = 0;
      maxexecute[i] = maxexecute_2nd;
      flag[i]=0;
    }
    //Reset expt done variables
    num_done = 0;
    expt_done = 0;

    //Reset memory stats    
    //output_mem_stats(mem_stats,count_i_total, mem_org);
    
    //Zero out the memstats and icount
    zero_mem_stats(mem_org->num_Mtree_levels, mem_stats);
    count_i_total = 0;

    //Zero out other statss:
    //Cache stats
    for(int ii=0;ii< (5+mem_org->num_Mtree_levels) ;ii++){
      init_cache_stats(&(L3_Cache_Stats[ii]),1);
      init_cache_stats(&(MAC_Cache_Stats[ii]),1);
      init_cache_stats(&(MET_Cache_Stats[ii]),1);
    }

    //overflow stats
    reset_tot_overflows(mem_org,ctr_cls,ctr_types);
    timestamp_cumuoverflows_warmup(mem_org,ctr_overflows_levelwise,ctr_overflows_levelwise_warmup);
    
  }
  
  if(sim_fast != 1){
    while (!expt_done) {
      /* For each core, retire instructions if they have finished. */
      for (numc = 0; numc < NUMCORES; numc++) {
        num_ret = 0;
        while ((num_ret < MAX_RETIRE) && ROB[numc].inflight && core_run[numc]) {
          /* Keep retiring until retire width is consumed or ROB is empty. */
          if (ROB[numc].comptime[ROB[numc].head] < CYCLE_VAL_CORE[numc]) {  
            /* Keep retiring instructions if they are done. */
            ROB[numc].head = (ROB[numc].head + 1) % ROBSIZE;
            ROB[numc].inflight--;
            committed[numc]++;
            num_ret++;
          }
          else{  /* Instruction not complete.  Stop retirement for this core. */
            //printf("ROB[%d].Head has unsatisfactory comptime %llu\n",numc,ROB[numc].comptime[ROB[numc].head]);
            break;
          }
        }  /* End of while loop that is retiring instruction for one core. */
      }  /* End of for loop that is retiring instructions for all cores. */
      
      if((CYCLE_VAL%PROCESSOR_CLK_MULTIPLIER == 0) && mem_flag){ 
        if(sim_fast != 1){ //Only then worried about memory timing analysis
          /* Execute function to find ready instructions. */
          update_memory();

          /* Execute user-provided function to select ready instructions for issue. */
          /* Based on this selection, update DRAM data structures and set 
             instruction completion times. */

          for(int c=0; c < NUM_CHANNELS; c++){
            schedule(c);
          }
        }
        mem_flag=0;
      }

      /* For each core, bring in new instructions from the trace file to
         fill up the ROB. */
      writeqfull =0;
      for(int c=0; c<NUM_CHANNELS; c++){
        if(write_queue_length[c] >= WQ_CAPACITY)
          {
            //printf("Write queue is full!\n");
            writeqfull = 1;
            break;
          }
      }

      for (numc = 0; numc < NUMCORES; numc++){
        if(core_run[numc]){
          if (!ROB[numc].tracedone) { /* Try to fetch if EOF has not been encountered. */
            num_fetch = 0;
            //ROB Stall loggers
            if((ROB[numc].inflight == ROBSIZE) || (writeqfull)){
              //printf("STALLED: Committed[%d]: %llu, CYCLE_VAL_CORE[%d]: %llu \n",numc,committed[numc], numc, CYCLE_VAL_CORE[numc]);
              rob_logger[numc]++;
              rob_logger_inst[numc]++;
            }
            while ((num_fetch < MAX_FETCH) && (ROB[numc].inflight != ROBSIZE) && (!writeqfull)) {
              /* Keep fetching until fetch width or ROB capacity or WriteQ or ReadQ are fully consumed. */
              /* Read the corresponding trace file and populate the tail of the ROB data structure. */
              /* If Memop, then populate read/write queue.  Set up completion time. */
              if (nonmemops[numc]) {  /* Have some non-memory-ops to consume. */
              
                ROB[numc].optype[ROB[numc].tail] = 'N';
                ROB[numc].comptime[ROB[numc].tail] = CYCLE_VAL_CORE[numc]+PIPELINEDEPTH;
                nonmemops[numc]--;
                ROB[numc].tail = (ROB[numc].tail +1) % ROBSIZE;
                ROB[numc].inflight++;
                fetched[numc]++;
                num_fetch++;
              }
              else { /* Done consuming non-memory-ops.  Must now consume the memory rd or wr. */
                if (opertype[numc] == 'R') {
                  ROB[numc].mem_address[ROB[numc].tail] = addr[numc];
                  ROB[numc].optype[ROB[numc].tail] = opertype[numc];
                  if(sim_fast != 1){ 
                    ROB[numc].comptime[ROB[numc].tail] = CYCLE_VAL_CORE[numc] + BIGNUM;
                  }
                  else { //We dont care about modelling ROB Stalls                      
                    ROB[numc].comptime[ROB[numc].tail] = CYCLE_VAL_CORE[numc]+PIPELINEDEPTH;
                  }
                  //printf("Comptime of MemRead is %llu\n",ROB[numc].comptime[ROB[numc].tail]);
                  ROB[numc].instrpc[ROB[numc].tail] = instrpc[numc];
                  long long int wb_addr = 0;
                  l3outcome=FALSE;
                  dirty=FALSE;
                  cacherequests[numc]++;
                  cacherequests_inst[numc]++;
                  mem_prio = 0;

                  total_requests++;
                  total_requests_inst++;

                  if(sim_useL3 == 1){
                    //L3 SRAM CACHE OPERATIONS 
                    l3outcome = cache_access(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);

                  } else { //We have L3 miss traces already
                    l3outcome = MISS;
                  }
                    
                  ///////////// LLC HIT! :) //////////////////////////////////////////////////
                  if(l3outcome==HIT) {
                    hits[numc]++;
                    hits_inst[numc]++;
                    ROB[numc].comptime[ROB[numc].tail] = CYCLE_VAL_CORE[numc]+((L3_LATENCY*100)/core_mod[numc])+PIPELINEDEPTH;
                  }
               
                  ///////////////////// WHAT FOLLOWS IS THE CASE IF ITS A LLC MISS////////////
                  if(l3outcome==MISS) {
                    ////////////////1. READ CACHELINE FROM MEMORY /////////////////////////
                    int lat = 0; //if sim_useL3 != 1, then will not cause a read_matches_write_or_read_queue() call
                    if(sim_useL3 == 1){
                      lat = read_matches_write_or_read_queue(addr[numc]);
                    }
                    if(lat){ //Request hit in the existing queues
                      ROB[numc].comptime[ROB[numc].tail] = CYCLE_VAL_CORE[numc]+(lat*100/core_mod[numc])+PIPELINEDEPTH;
                    }
                    else {
                      insert_read_memsys(addr[numc], CYCLE_VAL, numc, ROB[numc].tail, instrpc[numc], mem_prio, TRUE, data_access); // Read data from memory
                      lat_stat_start(&(ROB[numc].read_lat_stat[ ROB[numc].tail ]), CYCLE_VAL_CORE[numc]); 
                    
                      if(SGX_MODE[numc] >= 1){
                        //Parts removed since MAC assumed to be in 9th ECC Chip
                        //******************************************************
                        //// 1. Read MAC from MACCache or from memory
                        if(!((SGX_MODE[numc]  == 3) || (SGX_MODE[numc] == 4))){ //Will read MAC from mem in all modes except 3,4 (i.e. 1,2,5)

                          mac_paddr = getMACAddr(addr[numc],mem_org);
                          read_mac(MACCache, mac_paddr, numc, MAC_Cache_Stats, TRUE,
                                   mem_org, ROB,CYCLE_VAL,instrpc, mem_prio);
                        }
                        //*******************************************************            
                        if(!(SGX_MODE[numc] == 5)){ //Use counters in all modes except 5.
                          // 2. Check if counter/Mtree entry exists in metCache, if Yes - break, if No - go one level up in Counter Cache
                          int policy_install_metcache = TRUE;//** VARIABLE **
                          met_entry = getCounterAddr(addr[numc], mem_org);
                          //printf("Data address is: %llu, ctr address is %llu\n", addr[numc],met_entry.paddr);
                          read_ctr_mtree(METCache, L3Cache, MACCache, met_entry, numc, MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, TRUE,
                                         mem_org, ROB,CYCLE_VAL,instrpc, mem_prio, policy_install_metcache);
                        }
                      }
                    } 
                    ////////////////2. INSTALL CACHELINE IN L3 CACHE /////////////////////////
                    if(sim_useL3 == 1){
                      evicted_entry = cache_install(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);
                  
                      ////////////////3. WRITEBACK DIRTY EVICTS TO MEMORY /////////////////////////
                      if(evicted_entry.valid && evicted_entry.dirty){ //Write the dirty line in memory 
                        wb_addr=((unsigned long long int)evicted_entry.tag)<<6; //Bug squashed by Gururaj
                        handle_dirty_wb_LLC(wb_addr, evicted_entry, mem_prio, mem_org, METCache, L3Cache, MACCache, numc,
                                            MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, instrpc);                    
                      }
                    }
                  } 
                
                }
              
                else {  /* This must be a 'W'.  We are confirming that while reading the trace. */
                  if (opertype[numc] == 'W') {
                    ROB[numc].mem_address[ROB[numc].tail] = addr[numc];
                    ROB[numc].optype[ROB[numc].tail] = opertype[numc];
                    ROB[numc].comptime[ROB[numc].tail] = CYCLE_VAL_CORE[numc]+PIPELINEDEPTH;
                    /* Also, add this to the write queue. */
                    long long int wb_addr = 0;
                    l3outcome=FALSE;
                    dirty=TRUE;
                    cacherequests[numc]++;
                    total_requests++;
                    mem_prio = 0;

                    if(sim_useL3 == 1){
                      //L3 SRAM CACHE OPERATIONS
                      l3outcome = cache_access(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);
                    }
                    else {  //We have L3 miss traces already
                      l3outcome = MISS;
                    }
                      
                    ///////////// LLC HIT! :) //////////////////////////////////////////////////
                    if(l3outcome==HIT){
                      //Do Nothing , writes are not in the critical path 
                      hits[numc]++; 
                    }
                    ///////////////////// WHAT FOLLOWS IS THE CASE IF ITS A LLC MISS////////////
                    if(l3outcome==MISS) {

                      if(sim_useL3 == 1){
                        evicted_entry = cache_install(L3Cache,L3_PRIVATE,addr[numc],dirty,numc,L3_Cache_Stats,data_access, mem_org);
                      }
                      else { //We have L3 miss traces already
                        //need to pass the L3 dirty writeback forward
                        evicted_entry.valid = TRUE;
                        evicted_entry.dirty = TRUE;
                        evicted_entry.tag   = addr[numc] >> 6; //tag is a lineaddr
                        evicted_entry.tid   = numc;
                      }
                        
                      if(evicted_entry.valid && evicted_entry.dirty) { //Write the dirty line in memory

                        wb_addr=((unsigned long long int)evicted_entry.tag)<<6; //Bug squashed by Gururaj

                        if(!write_exists_in_write_queue(wb_addr)) {
                          handle_dirty_wb_LLC(wb_addr, evicted_entry, mem_prio, mem_org, METCache, L3Cache, MACCache, numc,
                                              MET_Cache_Stats, L3_Cache_Stats, MAC_Cache_Stats, ctr_overflows_levelwise, ctr_overflows_levelwise_inst, instrpc);
                        }                       
                      }
                      for(int c=0; c<NUM_CHANNELS; c++){
                        if(write_queue_length[c] >= WQ_CAPACITY){
                          writeqfull = 1;
                          break;	
                        } 
                      }
                   
                    }
                      
                  }
                  else {
                    printf("5. Panic.  Poor trace format. \n");
                    return -1;
                  }
               
                }
             
                ROB[numc].tail = (ROB[numc].tail +1) % ROBSIZE;
                ROB[numc].inflight++;
                fetched[numc]++;
                num_fetch++;

                /* Done consuming one line of the trace file.  Read in the next. */
                return_fgets1 = NULL;
                if( (!optimized_gzread) || (!trace_compressed)){

                  if(trace_compressed == 1){ //For compressed
                    return_fgets1 = gzgets(tif[numc],newstr,MAXTRACELINESIZE) ;
                    traceline = newstr;
                  }
                  else {
                    return_fgets1 = fgets(newstr,MAXTRACELINESIZE,tif[numc]) ;
                    traceline = newstr;
                  }
                
                } else {
                  if(!struct_traces){
                    traceline =  get_line_from_file( tif, numc, read_buffer, read_buf_index,  valid_buf_length, scratch_buffer);
                    return_fgets = traceline;
                  } else{
                    next_traceline = read_trace_line_1rec(tif[numc]);
                    return_fgets1 = 1;
                  }

                }
              
                if(return_fgets1) {
                  inst_comp++;
                  inst_exec[numc]++;
                  count_i[numc]++;
                  count_i_inst[numc]++;
                  count_i_total++;
                  count_i_inst_total++;
                  count_i_fast_slow++;

                  if(sim_useL3 == 1){ //Reading L2 Miss Stream
                    if (sscanf(traceline,"%llu %c",&nonmemops[numc],&opertype[numc]) > 0) {
                      if (opertype[numc] == 'R') {
                        if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
                          printf("6. Panic.  Poor trace format.\n");
                          return -4;
                        }
                      }
                      else {
                        if (opertype[numc] == 'W') {
                          if (sscanf(traceline,"%llu %c %Lx",&nonmemops[numc],&opertype[numc],&addr[numc]) < 1) {
                            printf("7. Panic.  Poor trace format.\n");
                            return -3;
                          }
                        }
                        else {
                          printf("8. Panic.  Poor trace format.\n");
                          return -2;
                        }
                      }
                    }
                    else {
                      printf("9. Panic.  Poor trace format.\n");
                      return -1;
                    }

                  } else{ //Reading L3 Miss Stream
                    if(!struct_traces){
                      if ((sscanf(traceline,"%llu %c %Lx",&nonmemops[numc], &opertype[numc],&addr[numc]) > 0)) {
                        ASSERTM( (opertype[numc] == 'W') || (opertype[numc] == 'R'), "Trace contains some operand other than R or W");              
                      }
                      else {
                        printf("3. Panic. Poor trace format reading L3 Miss Stream.\n");
                        return -1;
                      }
                    } else {
                      nonmemops[numc] = next_traceline.nonmemops;
                      opertype[numc] =  next_traceline.read_write;
                      addr[numc] =  next_traceline.addr;
                    }
                  }

                    
                  phy_addr=os_v2p_lineaddr(os,addr[numc],numc); //Returns physical_lineaddr
                  addr[numc]=phy_addr;
                  addr[numc]=addr[numc]<<6; //We need physical_addr in addr for passing to mem hierarchy. Hence, leftshift

                  inst_comp=nonmemops[numc]+inst_comp;
                  inst_exec[numc]=nonmemops[numc]+inst_exec[numc];

                  //Instruction Logger
                  count_i[numc]=nonmemops[numc]+count_i[numc];
                  count_i_inst[numc]=nonmemops[numc]+count_i_inst[numc];
                  count_i_total=nonmemops[numc]+count_i_total;
                  count_i_inst_total=nonmemops[numc]+count_i_inst_total;
                  count_i_fast_slow=nonmemops[numc]+count_i_fast_slow;
                  
                  if(flag[numc]==0){
                    inst_done[numc]=inst_exec[numc];
                  }
                  
                  if((flag[numc]==0) && (inst_done[numc] > maxexecute[numc])){
                    printf("Core %d reached end of its execution with %llu inst executed including warmup \n", numc, inst_done[numc]);
                    flag[numc]=1;
                    time_done[numc]=CYCLE_VAL;
                    num_done++;			
                  }

                }
                else {
                  ROB[numc].tracedone=1;
                  break;  /* Break out of the while loop fetching instructions. */
                }
              }  /* Done consuming the next rd or wr. */
            } 
            /* One iteration of the fetch while loop done. */
          } /* Closing brace for if(trace not done). */
          else { /* Input trace is done.  Check to see if all inflight instrs have finished. */
            if(trace_compressed == 1){
              gzrewind(tif[numc]);
            } else {                                    
              rewind(tif[numc]);
            }                                
            ROB[numc].tracedone=0;
          }
        }
      } /* End of for loop that goes through all cores. */
//      
//      //Resets stats when executing instructions reaches warmup COUNTERS
//      if(!warmup_completed){
//        num_warmup_completed = 0;
//        for (int coreID=0; coreID < NUMCORES; coreID++) {
//          if(inst_done[coreID] > max_inst_warmup) 
//            num_warmup_completed++;
//        }
//      
//        //Reset Statistics
//        if(num_warmup_completed == NUMCORES){
//          warmup_completed = 1;
//
//          CYCLE_VAL_stat = CYCLE_VAL;
//          for (int coreID=0; coreID < NUMCORES; coreID++) {
//            inst_done_stat[coreID]= inst_done[coreID];	
//            CYCLE_VAL_CORE_stat[coreID] = CYCLE_VAL_CORE[coreID];
//          }
//	
//          //Ctr_simulation
//          //reset_stat_overflows(mem_org,ctr_cls,ctr_types);
//	
//          /*
//            maxexecute = (max_inst_exec - max_inst_warmup);
//          */
//          for (int coreID=0; coreID < NUMCORES; coreID++) {
//            maxexecute[coreID] = inst_done[coreID]+ (max_inst_exec - max_inst_warmup);
//          }
//          /*
//          //Instructions
//          inst_comp=0;
//          Mcount=0;
//
//          */
//	
//          /*
//          // Per core instructions
//          for (int coreID=0; coreID < NUMCORES; coreID++) {
//          inst_exec[coreID] = 0;
//          inst_done[coreID] = 0;
//          flag[coreID] = 0; 
//          }
//          total_ipc=0;
//          */
//	
//          //Per core loggers
//          for(int coreID=0; coreID<NUMCORES; coreID++){
//            occupancy_logger[coreID]=0;
//            hitrate_logger[coreID]=0;
//            hits[coreID]=0;
//            cacherequests[coreID]=1;
//            total_requests=1;
//   
//            occupancy_logger_inst[coreID]=0;
//            hitrate_logger_inst[coreID]=0;
//            hits_inst[coreID]=0;
//            cacherequests_inst[coreID]=1;
//            total_requests_inst=1;
//	  
//            ipc_logger_inst[coreID] = 0;
//            ipc_logger[coreID] = 0;
//            count_i_inst[coreID] = 0;
//            count_i[coreID] = 0;
//            rob_logger_inst[coreID] = 0;
//            rob_logger[coreID] = 0;
//          }
//	
//          ipc_logger_total=0;
//          ipc_logger_inst_total=0;
//          count_i_total=0;
//          count_i_inst_total=0;
//	
//          /*
//          //Cycles
//          CYCLE_VAL=0;
//          CYCLE_VAL_TEMP=0;
//          for(int coreID=0; coreID<NUMCORES; coreID++){
//          CYCLE_VAL_CORE[coreID]=0;
//          }
//          */
//	
//	
//          /*
//          //Simulatiion State (e.g. ROB)
//          num_ret=0;
//          num_fetch=0;
//          num_done=0;
//          writeqfull=0;
//          total_inst_fetched = 0;
//
//          //Reseting the ROB
//
// 
//          //Stall Loggers
//          robf_stalls=0;
//          wrqf_stalls=0;
//          robn_stalls=0;
//          wrqn_stalls=0;
//
//          //Reset ROB Components:
//          for(int i=0; i<NUMCORES; i++) {
//          free(ROB[i].comptime);
//          free(ROB[i].mem_address);
//          free(ROB[i].instrpc);
//          free(ROB[i].optype);
//          free(ROB[i].critical_mem_ops);
//          free(ROB[i].read_lat_stat);
//
//          ROB[i].comptime = (long long int*)malloc(sizeof(long long int)*ROBSIZE);
//          ROB[i].mem_address = (long long int*)malloc(sizeof(long long int)*ROBSIZE);
//          ROB[i].instrpc = (long long int*)malloc(sizeof(long long int)*ROBSIZE);
//          ROB[i].optype = (int*)malloc(sizeof(int)*ROBSIZE);
//          ROB[i].critical_mem_ops = (int*)calloc(ROBSIZE,sizeof(int));
//          ROB[i].read_lat_stat  = (read_lat_rob_stat*)calloc(ROBSIZE,sizeof(read_lat_rob_stat));  
//
//          ROB[i].head=0;
//          ROB[i].tail=0;
//          ROB[i].inflight=0;
//          ROB[i].tracedone=0;
//          }
//	
//
//
//          for(int coreID=0; coreID<NUMCORES; coreID++){
//          committed[coreID] = 0; 
//          fetched[coreID]   = 0;
//          time_done[coreID] = 0;
//          nonmemops[coreID] = 0;
//          l1_hits[coreID] =   0;
//          l2_hits[coreID] =   0;
//          }
//
//          */
//          //Cache Stats
//          for(int ii=0;ii< (5+mem_org->num_Mtree_levels) ;ii++){
//            init_cache_stats(&(L3_Cache_Stats[ii]),1);
//            init_cache_stats(&(MAC_Cache_Stats[ii]),1);
//            init_cache_stats(&(MET_Cache_Stats[ii]),1);
//          }
//
//          /*
//            for(int ii=0; ii< mem_org->num_Mtree_levels + 1; ii++){
//            for(int jj=0;jj<NUMCORES;jj++){
//            init_cache_stats(&(MET_Cache_Stats[ii][jj]),1);  
//            }
//            }
//
//            for(int ii=0;ii<NUMCORES;ii++){
//            init_cache_stats(&(L3_Cache_Stats[ii]),1);
//            init_cache_stats(&(MAC_Cache_Stats[ii]),1);
//            init_cache_stats(&(MET_tot_Cache_Stats[ii]),1); 
//            }
//          */
//
//          free(L3_stats);
//          free(MAC_stats);       
//          free(L3_stats_inst);
//          free(MAC_stats_inst);
//          //free(MET_tot_stats);
//          //free(MET_tot_stats_inst);
//          free(MET_stats);
//          free(MET_stats_inst);
//	
//          L3_stats  = (cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
//          MAC_stats = (cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
//          MET_stats = (cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
//	
//          L3_stats_inst  =(cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
//          MAC_stats_inst =(cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
//          MET_stats_inst =(cache_sim_stats*) calloc((5+mem_org->num_Mtree_levels),sizeof(cache_sim_stats));
//
//	
//          /*
//            L3_stats  = (cache_sim_stats*) calloc(1, sizeof(cache_sim_stats));
//            MAC_stats = (cache_sim_stats*) calloc(1, sizeof(cache_sim_stats)); ; 
//            L3_stats_inst  =(cache_sim_stats*) calloc(1, sizeof(cache_sim_stats)); 
//            MAC_stats_inst =(cache_sim_stats*) calloc(1, sizeof(cache_sim_stats)); 
//            MET_tot_stats = (cache_sim_stats*) calloc(1, sizeof(cache_sim_stats)); ; 
//            MET_tot_stats_inst =(cache_sim_stats*) calloc(1, sizeof(cache_sim_stats)); 
//            MET_stats  = (cache_sim_stats*) calloc((mem_org->num_Mtree_levels+1) , sizeof(cache_sim_stats));
//            MET_stats_inst =(cache_sim_stats*) calloc((mem_org->num_Mtree_levels+1) , sizeof(cache_sim_stats));
//          */
//
//          free(lat_sim_stat);
//          lat_sim_stat = (read_lat_sim_stat*) calloc(1, sizeof(read_lat_sim_stat)); 
//          init_read_lat_sim_stat(lat_sim_stat,mem_org);
//          for(int i=0; i<NUMCORES; i++) {
//            for(int j = 0 ; j<ROBSIZE; j++){
//              init_read_lat_rob_stat(&(ROB[i].read_lat_stat[j]), mem_org);
//            }
//          }
//
//          //Memory
//          mem_stats = init_mem_stats(mem_org->num_Mtree_levels);
//          mem_stats_inst = init_mem_stats(mem_org->num_Mtree_levels);
//
//          //Memory loggers
//          free(memory_rd_inst);
//          free(memory_rd);
//          free(memory_wr_inst);
//          free(memory_wr);
//          free(memory_req_inst);
//          free(memory_req);
//
//          free(memory_rdqueue_inst);
//          free(memory_rdqueue);
//          free(memory_wrqueue_inst);
//          free(memory_wrqueue);
//          free(memory_reqqueue_inst);
//          free(memory_reqqueue);
//
//          free(memory_rdhit_inst);
//          free(memory_rdhit);
//          free(memory_wrhit_inst);
//          free(memory_wrhit);
//          free(memory_reqhit_inst);
//          free(memory_reqhit);
//          free(memory_rd_ch_inst);
//          free(memory_rd_ch);
//          free(memory_wr_ch_inst);
//          free(memory_wr_ch);
//          free(memory_req_ch_inst);
//          free(memory_req_ch);
//
//          free(memory_rdqueue_ch_inst);
//          free(memory_rdqueue_ch);
//          free(memory_wrqueue_ch_inst);
//          free(memory_wrqueue_ch);
//          free(memory_reqqueue_ch_inst);
//          free(memory_reqqueue_ch);
//
//          free(memory_rdhit_ch_inst);
//          free(memory_rdhit_ch);
//          free(memory_wrhit_ch_inst);
//          free(memory_wrhit_ch);
//          free(memory_reqhit_ch_inst);
//          free(memory_reqhit_ch);
//
//          memory_rd_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_rd = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_wr_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_wr = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_req_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_req = (long long int*)calloc(NUMCORES,sizeof(long long int));
//	
//          memory_rdqueue_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_rdqueue = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_wrqueue_inst= (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_wrqueue = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_reqqueue_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_reqqueue = (long long int*)calloc(NUMCORES,sizeof(long long int));
//	
//          memory_rdhit_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_rdhit = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_wrhit_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_wrhit = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_reqhit_inst = (long long int*)calloc(NUMCORES,sizeof(long long int));
//          memory_reqhit = (long long int*)calloc(NUMCORES,sizeof(long long int));
//	
//          memory_rd_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_rd_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_wr_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_wr_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_req_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_req_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//	
//          memory_rdqueue_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_rdqueue_ch= (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_wrqueue_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_wrqueue_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_reqqueue_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_reqqueue_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//	
//          memory_rdhit_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_rdhit_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_wrhit_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_wrhit_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_reqhit_ch_inst = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//          memory_reqhit_ch = (long long int*)calloc(NUM_CHANNELS,sizeof(long long int));
//
//          memory_rd_total_inst=0;
//          memory_rd_total=0;
//          memory_wr_total_inst=0;
//          memory_wr_total=0;
//          memory_req_total_inst=0;
//          memory_req_total=0;
//	
//          memory_rdqueue_total_inst=0;
//          memory_rdqueue_total=0;
//          memory_wrqueue_total_inst=0;
//          memory_wrqueue_total=0;
//          memory_reqqueue_total_inst=0;
//          memory_reqqueue_total=0;
//	
//          memory_rdhit_total_inst=0;
//          memory_rdhit_total=0;
//          memory_wrhit_total_inst=0;
//          memory_wrhit_total=0;
//          memory_reqhit_total_inst=0;
//          memory_reqhit_total=0;
//
//          /******************** POWER LOGGERS **************************/
//          usimm_total_rdcycle = 0;
//          usimm_total_wrcycle = 0;
//          usimm_total_rdother_cycle = 0;
//          usimm_total_wrother_cycle = 0;
//          usimm_total_pre_pdn_fastcycle = 0;
//          usimm_total_pre_pdn_slowcycle = 0;
//          usimm_total_act_pdncycle = 0;
//          usimm_total_act_stbycycle = 0;
//          usimm_total_pre_stbycycle = 0;
//	
//          usimm_total_bck = 0;
//          usimm_total_act = 0;
//          usimm_total_rd = 0;
//          usimm_total_wr = 0;
//          usimm_total_rd_term = 0;
//          usimm_total_wr_term = 0;
//          usimm_total_rdoth_term = 0;
//          usimm_total_wroth_term = 0;
//          usimm_ref_pwr = 0;
//          usimm_total_pwr = 0;
//	
//	
//          //Initialize the simulation state for the different components
//          /* TODO - Memory Controller & Scheduler Stats internally not refreshed */
//
//          /*
//            init_memory_controller_vars();
//            init_scheduler_vars();	
//          */
//
//          printf (" \n\n** Warmup Completed for %llu instructions **\n\n ", max_inst_warmup);
//        }
//      
//      }
//
//      
      if (num_done == NUMCORES) {
        /* Traces have been consumed and in-flight windows are empty.  Must confirm that write queues have been drained. */
          expt_done=1;  /* All traces have been consumed and the write queues are drained. */
        }
    
      for(int tt=0; tt < NUMCORES; tt++){
        core_run[tt]=0;
        if((rand()%MAXMOD)<core_mod[tt]){
          CYCLE_VAL_CORE[tt]++;
          core_run[tt]=1;
        }
      }

      if((rand()%MAXMOD)<100){

        CYCLE_VAL++;  /* Advance the simulation cycle. */
        CYCLE_VAL_TEMP++;
        mem_flag=1;
      }

      if((CYCLE_VAL_TEMP%1000000==0) && (CYCLE_VAL_TEMP!=0)) {
        fflush(stdout); 
        Mcount++;
        //printf("Committed %llu ", committed[0]);
        printf(".");

        /**********************************************/
        /* Start the logger - currently commented out */
        /*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//					Update Logger File here
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      
if(Mcount==1)
{

fprintf(Policyfile,"CYCLE-VAL\t");
          
fprintf(Policyfile,"Avg-read-latency\t");
fprintf(Policyfile,"Avg-queue-latency\t");
fprintf(Policyfile,"Act-read-latency\t");
fprintf(Policyfile,"Avg-TWIN-ovrhead\t"); 
fprintf(Policyfile,"Avg-MAC-ovrhead\t");
fprintf(Policyfile,"Avg-CTR-ovrhead\t");
fprintf(Policyfile,"Avg-MTREE-ovrhead\t");

fprintf(Policyfile,"TWINS/read\t"); 
fprintf(Policyfile,"MACS/read\t");
fprintf(Policyfile,"CTRS/read\t");
fprintf(Policyfile,"MTREE/read\t");
          

if(DETAILED_MET_STATS){
for(int i=mem_org->num_Mtree_levels-1; i>0; i--){
fprintf(Policyfile,"Avg-MTREE%d-ovrhead\t",i);  //mac latency/read
fprintf(Policyfile,"MTREE%d/read\t",i);   //mum_mtree/read
}
}
          
          
          
if(DETAILED_PROC_STATS){
for(int ww=0; ww < NUMCORES; ww++)
{
fprintf(Policyfile,"IPC-inst-%d\t",ww);
fprintf(Policyfile,"IPC-cumu-%d\t",ww);
	
fprintf(Policyfile,"ROBStall-inst-%d\t",ww);
fprintf(Policyfile,"ROBStall-cumu-%d\t",ww);

fprintf(Policyfile,"Hitrate-inst-%d\t",ww);
fprintf(Policyfile,"Hitrate-cumu-%d\t",ww);

fprintf(Policyfile,"Occupancy-%d\t",ww);
fprintf(Policyfile,"Occupancy_per_MILLION_CYCLES-%d\t",ww);

fprintf(Policyfile,"Mem-RdLat-Inst-%d\t",ww);
fprintf(Policyfile,"Mem-RdLat-%d\t",ww);

fprintf(Policyfile,"Mem-WrLat-Inst-%d\t",ww);
fprintf(Policyfile,"Mem-WrLat-%d\t",ww);
                
fprintf(Policyfile,"Mem-Lat-Inst-%d\t",ww);
fprintf(Policyfile,"Mem-Lat-%d\t",ww);

fprintf(Policyfile,"Mem-RdHit-Inst-%d\t",ww);
fprintf(Policyfile,"Mem-RdHit-%d\t",ww);

fprintf(Policyfile,"Mem-WrHit-Inst-%d\t",ww);
fprintf(Policyfile,"Mem-WrHit-%d\t",ww);

fprintf(Policyfile,"Mem-Hit-Inst-%d\t",ww);
fprintf(Policyfile,"Mem-Hit-%d\t",ww);
}
}

//L3 Cache Stats

if(DETAILED_INST_STATS){fprintf(Policyfile,"L3-Cache-readhitrate-inst\t");}
fprintf(Policyfile,"L3-Cache-readhitrate\t");
if(DETAILED_INST_STATS){fprintf(Policyfile,"L3-Cache-writehitrate-inst\t");}
fprintf(Policyfile,"L3-Cache-writehitrate\t");
if(DETAILED_INST_STATS){fprintf(Policyfile,"L3-Cache-totalhitrate-inst\t");}
fprintf(Policyfile,"L3-Cache-totalhitrate\t");

if(DETAILED_INST_STATS){fprintf(Policyfile,"L3-Cache-rmpki-inst\t");}
fprintf(Policyfile,"L3-Cache-rmpki\t");
if(DETAILED_INST_STATS){fprintf(Policyfile,"L3-Cache-wmpki-inst\t");}
fprintf(Policyfile,"L3-Cache-wmpki\t");
if(DETAILED_INST_STATS){fprintf(Policyfile,"L3-Cache-dirt-evict-pki-inst\t");}
fprintf(Policyfile,"L3-Cache-dirt-evict-pki\t");

//MAC Cache Stats
if(DETAILED_INST_STATS){fprintf(Policyfile,"MAC-Cache-readhitrate-inst\t");}
fprintf(Policyfile,"MAC-Cache-readhitrate\t");
if(DETAILED_CACHE_STATS){
if(DETAILED_INST_STATS){fprintf(Policyfile,"MAC-Cache-writehitrate-inst\t");}
fprintf(Policyfile,"MAC-Cache-writehitrate\t");
if(DETAILED_INST_STATS){fprintf(Policyfile,"MAC-Cache-totalhitrate-inst\t");}
fprintf(Policyfile,"MAC-Cache-totalhitrate\t"); 
} 
if(DETAILED_INST_STATS){fprintf(Policyfile,"MAC-Cache-rmpki-inst\t");}
fprintf(Policyfile,"MAC-Cache-rmpki\t");
if(DETAILED_CACHE_STATS){
if(DETAILED_INST_STATS){fprintf(Policyfile,"MAC-Cache-wmpki-inst\t");}
fprintf(Policyfile,"MAC-Cache-wmpki\t");
} 
if(DETAILED_INST_STATS){fprintf(Policyfile,"MAC-Cache-dirt-evict-pki-inst\t");}
fprintf(Policyfile,"MAC-Cache-dirt-evict-pki\t");


//MET_tot Cache Stats
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET_tot-Cache-readhitrate-inst\t");}
fprintf(Policyfile,"MET_tot-Cache-readhitrate\t");
if(DETAILED_CACHE_STATS){
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET_tot-Cache-writehitrate-inst\t");}
fprintf(Policyfile,"MET_tot-Cache-writehitrate\t");
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET_tot-Cache-totalhitrate-inst\t");}
fprintf(Policyfile,"MET_tot-Cache-totalhitrate\t"); 
} 
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET_tot-Cache-rmpki-inst\t");}
fprintf(Policyfile,"MET_tot-Cache-rmpki\t");
if(DETAILED_CACHE_STATS){
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET_tot-Cache-wmpki-inst\t");}
fprintf(Policyfile,"MET_tot-Cache-wmpki\t");
} 
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET_tot-Cache-dirt-evict-pki-inst\t");}
fprintf(Policyfile,"MET_tot-Cache-dirt-evict-pki\t");

if(DETAILED_MET_STATS){          
//MET Cache Stats
for(int ww=mem_org->num_Mtree_levels; ww >= 0 ; ww--) {

if(DETAILED_INST_STATS){fprintf(Policyfile,"MET%d-Cache-readhitrate-inst\t",ww); }
fprintf(Policyfile,"MET%d-Cache-readhitrate\t",ww);
if(DETAILED_CACHE_STATS){
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET%d-Cache-writehitrate-inst\t",ww);}
fprintf(Policyfile,"MET%d-Cache-writehitrate\t",ww);
if(DETAILED_INST_STATS){fprintf(Policyfile,"MET%d-Cache-totalhitrate-inst\t",ww);}
 fprintf(Policyfile,"MET%d-Cache-totalhitrate\t",ww);
 }
 if(DETAILED_INST_STATS){fprintf(Policyfile,"MET%d-Cache-rmpki-inst\t",ww);}
 fprintf(Policyfile,"MET%d-Cache-rmpki\t",ww);
 if(DETAILED_CACHE_STATS){
   if(DETAILED_INST_STATS){fprintf(Policyfile,"MET%d-Cache-wmpki-inst\t",ww);}
   fprintf(Policyfile,"MET%d-Cache-wmpki\t",ww);
 }
 if(DETAILED_INST_STATS){fprintf(Policyfile,"MET%d-Cache-dirt-evict-pki-inst\t",ww);}
 fprintf(Policyfile,"MET%d-Cache-dirt-evict-pki\t",ww);

 }
 }

 fprintf(Policyfile,"MPKI DATA_R\t");
 fprintf(Policyfile,"MPKI DATA_W\t");

 fprintf(Policyfile,"MPKI MAC_R\t");
 fprintf(Policyfile,"MPKI MAC_W\t");

 fprintf(Policyfile,"MPKI CTR_R\t");
 fprintf(Policyfile,"MPKI CTR_W\t");

 for(int ww=0; ww < mem_org->num_Mtree_levels; ww++) {
   fprintf(Policyfile,"MPKI MTREE_%d_R\t",ww);
   fprintf(Policyfile,"MPKI MTREE_%d_W\t",ww);
 }

 fprintf(Policyfile,"MPKI TWIN\t");
       
 if(DETAILED_MEM_STATS){
            
   for(int ww=0; ww < NUM_CHANNELS; ww++)
     {
       if(DETAILED_INST_STATS){fprintf(Policyfile,"Mem-RdLat-CH-Inst-%d\t",ww);}
       fprintf(Policyfile,"Mem-RdLat-CH-%d\t",ww);
                
       if(DETAILED_INST_STATS){fprintf(Policyfile,"Mem-WrLat-CH-Inst-%d\t",ww);}
       fprintf(Policyfile,"Mem-WrLat-CH-%d\t",ww);

       if(DETAILED_INST_STATS){fprintf(Policyfile,"Mem-Lat-CH-Inst-%d\t",ww);}
       fprintf(Policyfile,"Mem-Lat-CH-%d\t",ww);

       if(DETAILED_INST_STATS){fprintf(Policyfile,"Mem-RdHit-CH-Inst-%d\t",ww);}
       fprintf(Policyfile,"Mem-RdHit-CH-%d\t",ww);

       if(DETAILED_INST_STATS){fprintf(Policyfile,"Mem-WrHit-CH-Inst-%d\t",ww);}
       fprintf(Policyfile,"Mem-WrHit-CH-%d\t",ww);

       if(DETAILED_INST_STATS){fprintf(Policyfile,"Mem-Hit-CH-Inst-%d\t",ww);}
       fprintf(Policyfile,"Mem-Hit-CH-%d\t",ww);
     }

 }
             
 fprintf(Policyfile,"Mem-RdLat-Total-Inst\t");
 fprintf(Policyfile,"Mem-RdLat-Total\t");

 fprintf(Policyfile,"Mem-WrLat-Total-Inst\t");
 fprintf(Policyfile,"Mem-WrLat-Total\t");

 fprintf(Policyfile,"Mem-Lat-Total-Inst\t");
 fprintf(Policyfile,"Mem-Lat-Total\t");

 fprintf(Policyfile,"Mem-RdHit-Total-Inst\t");
 fprintf(Policyfile,"Mem-RdHit-Total\t");

 fprintf(Policyfile,"Mem-WrHit-Total-Inst\t");
 fprintf(Policyfile,"Mem-WrHit-Total\t");

 fprintf(Policyfile,"Mem-Hit-Total-Inst\t");
 fprintf(Policyfile,"Mem-Hit-Total\t");

                   
 fprintf(Policyfile,"Total-IPC-inst\t");
 fprintf(Policyfile,"Total-IPC-cumu\t");
                   
 fprintf(Policyfile,"\n");
 }
               
// Storing The Loged Data Starts
fprintf(Policyfile,"%llu\t",CYCLE_VAL);

print_read_lat_sim_stat(Policyfile,lat_sim_stat,mem_org);
      
if(DETAILED_PROC_STATS){
  for(int ww=0; ww < NUMCORES; ww++)
    {
      if(cacherequests[ww]==0)
        {
          hitrate_logger[ww]=0;
        }
      else
        {
          hitrate_logger[ww] = (double)(hits[ww])/(double)(cacherequests[ww]);
        }
		
      if(cacherequests_inst[ww]==0)
        {
          hitrate_logger_inst[ww]=0;
        }
      else
        {
          hitrate_logger_inst[ww] = (double)(hits_inst[ww])/(double)(cacherequests_inst[ww]);
        }


      ipc_logger[ww] = (double)(count_i[ww])/CYCLE_VAL;
      ipc_logger_inst[ww] = (double)(count_i_inst[ww])/CYCLE_VAL_TEMP;

      if(L3_PRIVATE){
        occupancy_logger_inst[ww] = L3Cache[ww]->total_lines_thread_percent[ww];
        occupancy_logger[ww] = occupancy_logger_inst[ww] + occupancy_logger[ww];  
      }
      else{
        occupancy_logger_inst[ww] = L3Cache[0]->total_lines_thread_percent[ww];
        occupancy_logger[ww] = occupancy_logger_inst[ww] + occupancy_logger[ww];  
      }

      fprintf(Policyfile,"%f\t",ipc_logger_inst[ww]);
      fprintf(Policyfile,"%f\t",ipc_logger[ww]);
      fprintf(Policyfile,"%llu\t",rob_logger_inst[ww]);
      fprintf(Policyfile,"%llu\t",rob_logger[ww]);
      fprintf(Policyfile,"%f\t",hitrate_logger_inst[ww]);
      fprintf(Policyfile,"%f\t",hitrate_logger[ww]);
      fprintf(Policyfile,"%f\t",occupancy_logger_inst[ww]);
      fprintf(Policyfile,"%f\t",occupancy_logger[ww]/Mcount);

      ///////////////////////////////////////


      if(memory_rd_inst[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)memory_rdqueue_inst[ww]/memory_rd_inst[ww]);
        memory_rdqueue_inst[ww]=0;
      }

      if(memory_rd[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)memory_rdqueue[ww]/memory_rd[ww]);
      }

      if(memory_wr_inst[ww]==0){
        fprintf(Policyfile,"0\t");
        memory_wrqueue_inst[ww]=0;
      }
      else{
        fprintf(Policyfile,"%f\t",(double)memory_wrqueue_inst[ww]/memory_wr_inst[ww]);
        memory_wrqueue_inst[ww]=0;
      }

      if(memory_wr[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)memory_wrqueue[ww]/memory_wr[ww]);
      }

      if(memory_req_inst[ww]==0){
        fprintf(Policyfile,"0\t");
        memory_reqqueue_inst[ww]=0;
      }
      else{
        fprintf(Policyfile,"%f\t",(double)memory_reqqueue_inst[ww]/memory_req_inst[ww]);
        memory_reqqueue_inst[ww]=0;
      }

      if(memory_req[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)memory_reqqueue[ww]/memory_req[ww]);
      }
      ///////////////////////////////////////

      if(memory_rd_inst[ww]==0){
        fprintf(Policyfile,"0\t");
        memory_rdhit_inst[ww]=0;
        memory_rd_inst[ww]=0;
      }
      else{
        fprintf(Policyfile,"%f\t",(double)(memory_rd_inst[ww]-memory_rdhit_inst[ww])/memory_rd_inst[ww]);
        memory_rdhit_inst[ww]=0;
        memory_rd_inst[ww]=0;
      }

      if(memory_rd[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)(memory_rd[ww]-memory_rdhit[ww])/memory_rd[ww]);
      }

      if(memory_wr_inst[ww]==0){
        fprintf(Policyfile,"0\t");
        memory_wrhit_inst[ww]=0;
        memory_wr_inst[ww]=0;
      }
      else{
        fprintf(Policyfile,"%f\t",(double)(memory_wr_inst[ww]-memory_wrhit_inst[ww])/memory_wr_inst[ww]);
        memory_wrhit_inst[ww]=0;
        memory_wr_inst[ww]=0;
      }

      if(memory_wr[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)(memory_wr[ww]-memory_wrhit[ww])/memory_wr[ww]);
      }

      if(memory_req_inst[ww]==0){
        fprintf(Policyfile,"0\t");
        memory_reqhit_inst[ww]=0;
        memory_req_inst[ww]=0;
      }
      else{
        fprintf(Policyfile,"%f\t",(double)(memory_req_inst[ww]-memory_reqhit_inst[ww])/memory_req_inst[ww]);
        memory_reqhit_inst[ww]=0;
        memory_req_inst[ww]=0;
      }

      if(memory_req[ww]==0){
        fprintf(Policyfile,"0\t");
      }
      else{
        fprintf(Policyfile,"%f\t",(double)(memory_req[ww]-memory_reqhit[ww])/memory_req[ww]);
      }
      ///////////////////////////////////////

      cacherequests_inst[ww]=1;
      hits_inst[ww]=0;
      ipc_logger_inst[ww]=0;
      rob_logger_inst[ww]=0;
      count_i_inst[ww]=0;

    }

 }     
        */   
//Calculates stats in line 2 of the function call
/*      calc_cache_stats (L3_Cache_Stats,L3_PRIVATE, 
        L3_stats, L3_stats_inst,
        count_i_total, count_i_inst_total);
*/

        
        for(int ii=0;ii< (5+mem_org->num_Mtree_levels) ;ii++){
          calc_cache_stats (&L3_Cache_Stats[ii],L3_PRIVATE, 
                    &L3_stats[ii], &L3_stats_inst[ii],
                    count_i_total, count_i_inst_total);
        }
/*
//Prints Calculated Stats
print_cache_stats (Policyfile, L3_stats, L3_stats_inst, 1);

*L3_stats_inst = (const cache_sim_stats){0}; 
L3_Cache_Stats[0].read_hits_inst[0]=0;
L3_Cache_Stats[0].write_hits_inst[0]=0;
L3_Cache_Stats[0].read_misses_inst[0]=0;
L3_Cache_Stats[0].write_misses_inst[0]=0;
L3_Cache_Stats[0].dirty_evicts_inst[0]=0;
*/
//Calculates stats in line 2 of the function call
/*
  calc_cache_stats (MAC_Cache_Stats,MAC_PRIVATE, 
  MAC_stats, MAC_stats_inst,
  count_i_total, count_i_inst_total);

*/

for(int ii=0;ii< (5+mem_org->num_Mtree_levels) ;ii++){
          calc_cache_stats (&MAC_Cache_Stats[ii],MAC_PRIVATE, 
                    &MAC_stats[ii], &MAC_stats_inst[ii],
                    count_i_total, count_i_inst_total);
 }
/*
//Prints Calculated Stats
print_cache_stats (Policyfile, MAC_stats, MAC_stats_inst, 0);
*MAC_stats_inst = (const cache_sim_stats){0}; 
MAC_Cache_Stats[0].read_hits_inst[0]=0;
MAC_Cache_Stats[0].write_hits_inst[0]=0;
MAC_Cache_Stats[0].read_misses_inst[0]=0;
MAC_Cache_Stats[0].write_misses_inst[0]=0;
MAC_Cache_Stats[0].dirty_evicts_inst[0]=0; 

*/
for(int ii=0;ii< (5+mem_org->num_Mtree_levels) ;ii++){
  calc_cache_stats (&MET_Cache_Stats[ii],MET_PRIVATE, 
                    &MET_stats[ii], &MET_stats_inst[ii],
                    count_i_total, count_i_inst_total);
 }
/*
  update_met_stats(MET_tot_Cache_Stats,MET_Cache_Stats, mem_org);

  calc_cache_stats (MET_tot_Cache_Stats,MET_PRIVATE, 
  MET_tot_stats, MET_tot_stats_inst,
  count_i_total, count_i_inst_total);
*/
/*
//Prints Calculated Stats
print_cache_stats (Policyfile, MET_tot_stats, MET_tot_stats_inst, 0);
*MET_tot_stats_inst = (const cache_sim_stats){0}; 
MET_tot_Cache_Stats[0].read_hits_inst[0]=0;
MET_tot_Cache_Stats[0].write_hits_inst[0]=0;
MET_tot_Cache_Stats[0].read_misses_inst[0]=0;
MET_tot_Cache_Stats[0].write_misses_inst[0]=0;
MET_tot_Cache_Stats[0].dirty_evicts_inst[0]=0; 

*/         
//Calculates stats in line 2 of the function call
/*
  for(int xx=mem_org->num_Mtree_levels; xx >= 0  ; xx--){
  calc_cache_stats (MET_Cache_Stats[xx],MET_PRIVATE, 
  &(MET_stats[xx]), &(MET_stats_inst[xx]),
  count_i_total, count_i_inst_total);
  } //Need to comment this } if uncommenting rest
*/
/*
//Prints Calculated Stats
if(DETAILED_MET_STATS){print_cache_stats (Policyfile, &(MET_stats[xx]), &(MET_stats_inst[xx]), 0);}

*MET_stats_inst = (const cache_sim_stats){0}; 
MET_Cache_Stats[xx][0].read_hits_inst[0]=0;
MET_Cache_Stats[xx][0].write_hits_inst[0]=0;
MET_Cache_Stats[xx][0].read_misses_inst[0]=0;
MET_Cache_Stats[xx][0].write_misses_inst[0]=0;
MET_Cache_Stats[xx][0].dirty_evicts_inst[0]=0; 
}

//Print the MPKI stats
print_mem_stats(Policyfile, mem_stats_inst, count_i_inst_total, mem_org);
zero_mem_stats(mem_org->num_Mtree_levels, mem_stats_inst);
      
if(DETAILED_MEM_STATS){
for(int ww=0; ww< NUM_CHANNELS; ww++)
{
if(memory_rd_ch_inst[ww]==0){
fprintf(Policyfile,"0\t");
memory_rdqueue_ch_inst[ww]=0;
}
else{
fprintf(Policyfile,"%f\t",(double)memory_rdqueue_ch_inst[ww]/memory_rd_ch_inst[ww]);
memory_rdqueue_ch_inst[ww]=0;
}

if(memory_rd_ch[ww]==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)memory_rdqueue_ch[ww]/memory_rd_ch[ww]);
}

if(memory_wr_ch_inst[ww]==0){
fprintf(Policyfile,"0\t");
memory_wrqueue_ch_inst[ww]=0;
}
else{
fprintf(Policyfile,"%f\t",(double)memory_wrqueue_ch_inst[ww]/memory_wr_ch_inst[ww]);
memory_wrqueue_ch_inst[ww]=0;
}

if(memory_wr_ch[ww]==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)memory_wrqueue_ch[ww]/memory_wr_ch[ww]);
}

if(memory_req_ch_inst[ww]==0){
fprintf(Policyfile,"0\t");
memory_reqqueue_ch_inst[ww]=0;
}
else{
fprintf(Policyfile,"%f\t",(double)memory_reqqueue_ch_inst[ww]/memory_req_ch_inst[ww]);
memory_reqqueue_ch_inst[ww]=0;
}

if(memory_req_ch[ww]==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)memory_reqqueue_ch[ww]/memory_req_ch[ww]);
}
///////////////////////////////////////

if(memory_rd_ch_inst[ww]==0){
fprintf(Policyfile,"0\t");
memory_rdhit_ch_inst[ww]=0;
memory_rd_ch_inst[ww]=0;
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_rd_ch_inst[ww]-memory_rdhit_ch_inst[ww])/memory_rd_ch_inst[ww]);
memory_rdhit_ch_inst[ww]=0;
memory_rd_ch_inst[ww]=0;
}

if(memory_rd_ch[ww]==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_rd_ch[ww]-memory_rdhit_ch[ww])/memory_rd_ch[ww]);
}

if(memory_wr_ch_inst[ww]==0){
fprintf(Policyfile,"0\t");
memory_wrhit_ch_inst[ww]=0;
memory_wr_ch_inst[ww]=0;
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_wr_ch_inst[ww]-memory_wrhit_ch_inst[ww])/memory_wr_ch_inst[ww]);
memory_wrhit_ch_inst[ww]=0;
memory_wr_ch_inst[ww]=0;
}

if(memory_wr_ch[ww]==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_wr_ch[ww]-memory_wrhit_ch[ww])/memory_wr_ch[ww]);
}

if(memory_req_ch_inst[ww]==0){
fprintf(Policyfile,"0\t");
memory_reqhit_ch_inst[ww]=0;
memory_req_ch_inst[ww]=0;
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_req_ch_inst[ww]-memory_reqhit_ch_inst[ww])/memory_req_ch_inst[ww]);
memory_reqhit_ch_inst[ww]=0;
memory_req_ch_inst[ww]=0;
}

if(memory_req_ch[ww]==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_req_ch[ww]-memory_reqhit_ch[ww])/memory_req_ch[ww]);
}
}
}
////////////////////////////////////////////////
      
if(memory_rd_total_inst==0){
fprintf(Policyfile,"0\t");
memory_rdqueue_total_inst=0;
}
else{
fprintf(Policyfile,"%f\t",(double)memory_rdqueue_total_inst/memory_rd_total_inst);
memory_rdqueue_total_inst=0;
}

if(memory_rd_total==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)memory_rdqueue_total/memory_rd_total);
}

if(memory_wr_total_inst==0){
fprintf(Policyfile,"0\t");
memory_wrqueue_total_inst=0;
}
else{
fprintf(Policyfile,"%f\t",(double)memory_wrqueue_total_inst/memory_wr_total_inst);
memory_wrqueue_total_inst=0;
}

if(memory_wr_total==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)memory_wrqueue_total/memory_wr_total);
}
  
if(memory_req_total_inst==0){
fprintf(Policyfile,"0\t");
memory_reqqueue_total_inst=0;
}
else{
fprintf(Policyfile,"%f\t",(double)memory_reqqueue_total_inst/memory_req_total_inst);
memory_reqqueue_total_inst=0;
}
  
if(memory_req_total==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)memory_reqqueue_total/memory_req_total);
}
/////////////////////////////////////////// 
if(memory_rd_total_inst==0){
fprintf(Policyfile,"0\t");
memory_rdhit_total_inst=0;
memory_rd_total_inst=0;
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_rd_total_inst-memory_rdhit_total_inst)/memory_rd_total_inst);
memory_rdhit_total_inst=0;
memory_rd_total_inst=0;
}

if(memory_rd_total==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_rd_total-memory_rdhit_total)/memory_rd_total);
}

if(memory_wr_total_inst==0){
fprintf(Policyfile,"0\t");
memory_wrhit_total_inst=0;
memory_wr_total_inst=0;
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_wr_total_inst-memory_wrhit_total_inst)/memory_wr_total_inst);
memory_wrhit_total_inst=0;
memory_wr_total_inst=0;
}

if(memory_wr_total==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_wr_total-memory_wrhit_total)/memory_wr_total);
}

if(memory_req_total_inst==0){
fprintf(Policyfile,"0\t");
memory_reqhit_total_inst=0;
memory_req_total_inst=0;
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_req_total_inst-memory_reqhit_total_inst)/memory_req_total_inst);
memory_reqhit_total_inst=0;
memory_req_total_inst=0;
}

if(memory_req_total==0){
fprintf(Policyfile,"0\t");
}
else{
fprintf(Policyfile,"%f\t",(double)(memory_req_total-memory_reqhit_total)/memory_req_total);
}

ipc_logger_total = (double)(count_i_total)/CYCLE_VAL;
ipc_logger_inst_total = (double)(count_i_inst_total)/CYCLE_VAL_TEMP;


fprintf(Policyfile,"%f\t",ipc_logger_inst_total);
fprintf(Policyfile,"%f\t",ipc_logger_total);
fprintf(Policyfile,"\n");
total_requests_inst=1;
count_i_inst_total=0;
fflush(Policyfile);
   
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Finish the LOGGER ****************** */
  
  if(Mcount>0) {
    if(Mcount%100==0) {
      printf("\t%lu M ",Mcount);
      for(int uu=0; uu < NUMCORES; uu++){
        printf("| C%d - %f ",uu, ((double)inst_exec[uu])/CYCLE_VAL);
        if(L3_PRIVATE){
          printf(",INST %llu M , O %f ",inst_done[uu]/1000000, L3Cache[uu]->total_lines_thread_percent[uu]); 
        }
        else{
          printf(",INST %llu M ", inst_done[uu]/1000000 );
        } 
      }
      printf("\n");

    }
        }
  
        if(CYCLE_VAL%50000000==0) {
          priority_thread++;
          if(priority_thread==NUMCORES) {
            priority_thread=0;
          }
        }
        CYCLE_VAL_TEMP=0;
      }
          
      //For every iteration of the while-loop, dump overflow stats if Inst_Count is multiple of 500Mn
      if(count_500mn_fast_slow != (count_i_fast_slow /500000000)){
        count_500mn_fast_slow = (count_i_fast_slow /500000000);
        //TODO Call dump overflows logger
        log_ctr_overflow_vs_inst(PolicyfileOverflows, count_500mn_fast_slow,mem_org,ctr_overflows_levelwise,ctr_overflows_levelwise_inst);
      }
        
    }
  }
 
/* Code to make sure all stats are calculating only the non-warmup duration */
 for (numc=0; numc < NUMCORES; numc++) {
    time_done[numc] = time_done[numc] - CYCLE_VAL_stat;
    inst_done[numc] = inst_done[numc] - inst_done_stat[numc];
  }
  CYCLE_VAL = CYCLE_VAL - CYCLE_VAL_stat;

  /* Done */

  /* Code to make sure that the write queue drain time is included in
     the execution time of the thread that finishes last. */
  maxtd = time_done[0];
  for (numc=1; numc < NUMCORES; numc++) {
    if (time_done[numc] > maxtd) {
      maxtd = time_done[numc];
    }
  }
  core_power = 0;
  for (numc=0; numc < NUMCORES; numc++) {
    /* A core has peak power of 10 W in a 4-channel config.  Peak power is consumed while the thread is running, else the core is perfectly power gated. */
    core_power = core_power + (10*((float)time_done[numc]/(float)CYCLE_VAL));
  }
  if (NUM_CHANNELS == 1) {
    /* The core is more energy-efficient in our single-channel configuration. */
    core_power = core_power/2.0 ;
  }

  printf("Done with loop. Printing stats.\n");
  printf("Cycles %lld\n", CYCLE_VAL);

  printf("Printing Stats for %llu instructions per thread\n", maxexecute[0] - max_inst_warmup);
  for (numc=0; numc < NUMCORES; numc++) {
    printf("Core %d: Inst %lld: At time %lld\n", numc, inst_done[numc], time_done[numc]);
    printf("IPC-%d                 \t: %f\n", numc, ((double)inst_done[numc])/(time_done[numc]));
    total_ipc= total_ipc+((double)inst_done[numc])/(time_done[numc]);
    total_time_done += time_done[numc];
    total_inst_fetched += inst_done[numc];
  }

  if(sim_fast == 1){
    uns64 free_overflow_exec_time = calc_fast_cycles(mem_stats,count_i_total, mem_org,1);
    total_time_done = calc_fast_cycles(mem_stats,count_i_total, mem_org,0);
    total_inst_fetched = count_i_total;
    total_ipc = ((double)count_i_total) / ((double) total_time_done); 
    printf("\nFREE_OVERFLOW_IPC   \t : %f\n",((double)count_i_total) /((double)free_overflow_exec_time) );
 
  }

  printf("\nTOTAL_IPC             \t : %f\n",total_ipc);
  printf("\nUSIMM_CYCLES_R        \t : %llu\n",total_time_done);
  printf("USIMM_INST_R            \t : %llu\n\n",total_inst_fetched);

  total_time_done = 0;
  //  total_inst_fetched = 0;


  for (numc=0; numc < NUMCORES; numc++) {
    printf("Core %d: Completed %llu : At time : %lld\n", numc, inst_done[numc], CYCLE_VAL);
    //    printf("Core %d: Fetched %lld : Committed %lld : At time : %lld\n", numc, fetched[numc], committed[numc], CYCLE_VAL);
    //    total_inst_fetched = total_inst_fetched + fetched[numc];

  }


  
printf("\n");
printf("USIMM_CYCLES_END           \t : %lld\n",CYCLE_VAL);
printf("USIMM_INST             \t : %lld\n",total_inst_fetched);
printf("USIMM_INST_DOUBLECHECK             \t : %lld\n",count_i_total);
printf("CUMUL_IPC              \t : %f\n\n",(double)total_inst_fetched/CYCLE_VAL);
printf("USIMM_RD_MERGED        \t : %lld\n",num_read_merge);
printf("USIMM_WR_MERGED        \t : %lld\n\n",num_write_merge);
printf("USIMM_L3_REPL          \t : %lld\n",L3_REPL);
printf("USIMM_L3_PRIVATE       \t : %lld\n",L3_PRIVATE);
printf("\n");

//printf("****** Memory Read Latency Breakdown **\n\n");
//output_read_lat_sim_stat(lat_sim_stat,mem_org);
           
for (numc=0; numc < NUMCORES; numc++) {
  printf("USIMM_ROB_LOG-%d       \t : %llu\n",numc,rob_logger[numc]);
  printf("USIMM_HITRT_LOG-%d     \t : %f\n",numc,hitrate_logger[numc]);
  printf("USIMM_OCCUPANCY_LOG-%d \t : %f\n",numc,occupancy_logger[numc]/Mcount);
  printf("USIMM_MEMRD_LAT-%d     \t : %f\n",numc,(double)memory_rdqueue[numc]/memory_rd[numc]);
  printf("USIMM_MEMWR_LAT-%d     \t : %f\n",numc,(double)memory_wrqueue[numc]/memory_wr[numc]);
  printf("USIMM_MEM_LAT-%d       \t : %f\n",numc,(double)memory_reqqueue[numc]/memory_req[numc]);
  printf("USIMM_MEMRD_HIT-%d     \t : %f\n",numc,(double)(memory_rd[numc]-memory_rdhit[numc])/memory_rd[numc]);
  printf("USIMM_MEMWR_HIT-%d     \t : %f\n",numc,(double)(memory_wr[numc]-memory_wrhit[numc])/memory_wr[numc]);
  printf("USIMM_MEM_HIT-%d       \t : %f\n",numc,(double)(memory_req[numc]-memory_reqhit[numc])/memory_req[numc]);
 }
printf("\n");

printf("****** L3 Cache Statistics **\n\n");

output_cache_stats("L3_DATA",&L3_Cache_Stats[0], &L3_stats[0], 1 );
output_cache_stats("L3_CTR_MTREE",&L3_Cache_Stats[1], &L3_stats[1], 1 );
output_cache_stats("L3_CTR",&L3_Cache_Stats[2], &L3_stats[2], 1 );
output_cache_stats("L3_MTREE",&L3_Cache_Stats[3], &L3_stats[3], 1 );
  
for(int xx=mem_org->num_Mtree_levels -1 ; xx >= 0  ; xx--){
  sprintf(temp,"L3_MTREE_%d",xx);
  output_cache_stats(temp,&L3_Cache_Stats[4+xx], &L3_stats[4+xx], 1 );
 }

           
printf("****** MAC Cache Statistics **\n\n");
output_cache_stats("MAC",&MAC_Cache_Stats[4+mem_org->num_Mtree_levels], &MAC_stats[4+mem_org->num_Mtree_levels], 1 );

printf("****** Counter Cache Total Statistics **\n\n");
output_cache_stats("MET_CTR_MTREE",&MET_Cache_Stats[1], &MET_stats[1], 1 );
output_cache_stats("MET_CTR",&MET_Cache_Stats[2], &MET_stats[2], 1 );
output_cache_stats("MET_MTREE",&MET_Cache_Stats[3], &MET_stats[3], 1 );
  
for(int xx=mem_org->num_Mtree_levels -1 ; xx >= 0  ; xx--){
  sprintf(temp,"MET_MTREE_%d",xx);
  output_cache_stats(temp,&MET_Cache_Stats[4+xx], &MET_stats[4+xx], 1 );
 }

printf("****** Memory Traffic PKI Statistics **\n\n");
output_mem_stats(mem_stats,count_i_total, mem_org); 
data_mpki = return_data_mpki(mem_stats,count_i_total, mem_org);
non_overflow_mpki = return_non_overflow_mpki(mem_stats,count_i_total, mem_org);
printf("\n*** ENDS ***\n");

printf("****** Ctr Overflow Statistics **\n\n");
if( (SGX_MODE[0] != 0) || (SGX_MODE[1] != 0) || (SGX_MODE[2] != 0) || (SGX_MODE[3] != 0) ){
  fin_stat_overflows(L3Cache[0],METCache[0], mem_org, ctr_cls, ctr_types, ctr_overflows_levelwise, ctr_overflows_levelwise_inst);
  print_stat_overflows(mem_org, ctr_cls, ctr_types, count_i_total);
  print_stat_overflow_newfinal(mem_org, ctr_overflows_levelwise, ctr_overflows_levelwise_warmup, total_inst_fetched, data_mpki, non_overflow_mpki);

#ifdef HIST_NZ
  log_stat_overflows_HistNz(mem_org, ctr_types, PolicyfileHistNz);
#endif

  /*
  //Dump counter-wise overflows in Files

  log_stat_overflows(mem_org, ctr_cls, ctr_types,Policyfile);
  //log2_stat_overflows(mem_org, ctr_cls, ctr_types,Policyfile2); //** TODO : FIX BUG **
  if( (mem_org->num_Mtree_levels - 1) > 0 ){
  log_stat_ctr_vals(mem_org, ctr_cls, ctr_types,Policyfile3, mem_org->num_Mtree_levels - 1); //print vals of Tree Leaf
  }
  if( (mem_org->num_Mtree_levels - 2) > 0 ){    
  log_stat_ctr_vals(mem_org, ctr_cls, ctr_types,Policyfile4, mem_org->num_Mtree_levels - 2); //print vals of Tree Leaf
  }
  */
  //  count_dirty_lines(L3Cache[0],"L3");
  // count_dirty_lines(METCache[0],"MET");

  printf("\n*** ENDS ***\n");
 }


print_logs_comp_schemes(ctr_types, mem_org);

//Calculate Cache Occupancy Stats
if(sim_useL3 == 1){
  calc_cache_occ (L3Cache,&L3_stats[0]);
  if( (SGX_MODE[0] != 0) || (SGX_MODE[1] != 0) || (SGX_MODE[2] != 0) || (SGX_MODE[3] != 0) ){
    if( (SGX_MODE[0] != 5) || (SGX_MODE[1] != 5) || (SGX_MODE[2] != 5) || (SGX_MODE[3] != 5) ){
      calc_cache_occ (METCache,&MET_stats[0]);
    }
  }
  printf("****** Cache Occupancy Statistics **\n\n");
  output_cache_occ("L3",&L3_stats[0]);
  if( (SGX_MODE[0] == 1) || (SGX_MODE[1] == 1) || (SGX_MODE[2] == 1) || (SGX_MODE[3] == 1) ){
    //  if( (SGX_MODE == 1) ){
    output_cache_occ("MET",&MET_stats[0]);
  }
  printf("\n*** ENDS ***\n");
 }
  
  
printf("USIMM_MEMRD_LAT_TOTAL \t : %f\n",(double)memory_rdqueue_total/memory_rd_total);
printf("USIMM_MEMWR_LAT_TOTAL \t : %f\n",(double)memory_wrqueue_total/memory_wr_total);
printf("USIMM_MEM_LAT_TOTAL   \t : %f\n",(double)memory_reqqueue_total/memory_req_total);
printf("USIMM_MEMRD_HIT_TOTAL \t : %f\n",(double)(memory_rd_total-memory_rdhit_total)/memory_rd_total);
printf("USIMM_MEMWR_HIT_TOTAL \t : %f\n",(double)(memory_wr_total-memory_wrhit_total)/memory_wr_total);
printf("USIMM_MEM_HIT_TOTAL   \t : %f\n",(double)(memory_req_total-memory_reqhit_total)/memory_req_total);

printf("\n");


             
for(int ww=0; ww<NUM_CHANNELS; ww++)           {
  printf("USIMM_MEMRD_LAT_CH%d     \t : %f\n",ww,(double)memory_rdqueue_ch[ww]/memory_rd_ch[ww]);
  printf("USIMM_MEMWR_LAT_CH%d     \t : %f\n",ww,(double)memory_wrqueue_ch[ww]/memory_wr_ch[ww]);
  printf("USIMM_MEM_LAT_CH%d       \t : %f\n",ww,(double)memory_reqqueue_ch[ww]/memory_req_ch[ww]);
  printf("USIMM_MEMRD_HIT_CH%d     \t : %f\n",ww,(double)(memory_rd_ch[ww]-memory_rdhit_ch[ww])/memory_rd_ch[ww]);
  printf("USIMM_MEMWR_HIT_CH%d     \t : %f\n",ww,(double)(memory_wr_ch[ww]-memory_wrhit_ch[ww])/memory_wr_ch[ww]);
  printf("USIMM_MEM_HIT_CH%d       \t : %f\n",ww,(double)(memory_req_ch[ww]-memory_reqhit_ch[ww])/memory_req_ch[ww]);
 }
/* Print all other memory system stats. */
scheduler_stats();
print_stats();

/*Logging currently disabled */
/*
//File Handlers for "Overflow by Counter" Dumps
fclose(Policyfile);
fclose(Policyfile2);
fclose(Policyfile3);
fclose(Policyfile4);
*/

//File Handler for Loggin Level-wise Overflows vs Time.
 fclose(PolicyfileOverflows);

#ifdef HIST_NZ
 fclose(PolicyfileHistNz);
#endif
 /******************************/
 
 /*Print Cycle Stats*/
 for(int c=0; c<NUM_CHANNELS; c++)
  for(int r=0; r<NUM_RANKS ;r++)
    calculate_power(c,r,0,chips_per_rank);

printf ("USIMM_TOTAL_RDCYC           \t: %9.2f\n", usimm_total_rdcycle);
printf ("USIMM_TOTAL_WRCYC           \t: %9.2f\n", usimm_total_wrcycle);
printf ("USIMM_TOTAL_RDOTHCYC        \t: %9.2f\n", usimm_total_rdother_cycle);
printf ("USIMM_TOTAL_WROTHCYC        \t: %9.2f\n", usimm_total_wrother_cycle);
printf ("USIMM_TOTAL_PRE_PDN_FASTCYC \t: %9.2f\n", usimm_total_pre_pdn_fastcycle);
printf ("USIMM_TOTAL_PRE_PDN_SLOWCYC \t: %9.2f\n", usimm_total_pre_pdn_slowcycle);
printf ("USIMM_TOTAL_ACT_PDNCYC      \t: %9.2f\n", usimm_total_act_pdncycle); 
printf ("USIMM_TOTAL_ACT_STBYCYC     \t: %9.2f\n", usimm_total_act_stbycycle); 
printf ("USIMM_TOTAL_PRE_STBYCYC     \t: %9.2f\n", usimm_total_pre_stbycycle); 
printf ("---------------------------------------------------------------\n\n");
                                                                                

printf ("\n#-------------------------------------- Power Stats ----------------------------------------------\n");
printf ("Note:  1. termRoth/termWoth is the power dissipated in the ODT resistors when Read/Writes terminate \n");
printf ("          in other ranks on the same channel\n");
printf ("#-------------------------------------------------------------------------------------------------\n\n");


/*Print Power Stats*/
float total_system_power =0;
for(int c=0; c<NUM_CHANNELS; c++)
  for(int r=0; r<NUM_RANKS ;r++)
    total_system_power += calculate_power(c,r,1,chips_per_rank);
           
printf ("USIMM_TOTAL_BCKmW        \t: %9.2f\n", usimm_total_bck); 
printf ("USIMM_TOTAL_ACTmW        \t: %9.2f\n", usimm_total_act);
printf ("USIMM_TOTAL_RDmW         \t: %9.2f\n", usimm_total_rd); 
printf ("USIMM_TOTAL_WRmW         \t: %9.2f\n", usimm_total_wr); 
printf ("USIMM_TOTAL_RD_TERMmW    \t: %9.2f\n", usimm_total_rd_term); 
printf ("USIMM_TOTAL_WR_TERMmW    \t: %9.2f\n", usimm_total_wr_term); 
printf ("USIMM_TOTAL_RDOTH_TERMmW \t: %9.2f\n", usimm_total_rdoth_term); 
printf ("USIMM_TOTAL_WROTH_TERMmW \t: %9.2f\n", usimm_total_wroth_term); 
printf ("USIMM_TOTAL_REFmW        \t: %9.2f\n", usimm_ref_pwr); 
printf ("---------------------------------------------------------------\n");
printf ("USIMM_TOTAL_PWRmW        \t: %9.2f\n", usimm_total_pwr);
printf ("---------------------------------------------------------------\n\n");
   
printf("USIMM_MEM_POWER_W         \t: %f\n",total_system_power/1000);
printf("USIMM_OTHER_POWER_W       \t: %f\n",(double)10*NUM_CHANNELS);
printf("USIMM_CORE_W              \t: %f\n",core_power);
printf("USIMM_SYS_POWER_W         \t: %f\n", 10 + core_power + total_system_power/1000);
printf("USIMM_EDP_Js              \t: %2.9f\n", (10 + core_power + total_system_power/1000)*(float)((double)CYCLE_VAL/(double)3200000000) * (float)((double)CYCLE_VAL/(double)3200000000));

os_print_stats(os);
return 0;
}


