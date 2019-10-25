#ifndef __PARAMS_H__
#define __PARAMS_H__

/********************/
/* Processor params */
/********************/
// number of cores in mulicore 
int NUMCORES;

// processor clock frequency multiplier : multiplying the
// DRAM_CLK_FREQUENCY by the following parameter gives the processor
// clock frequency 
 int PROCESSOR_CLK_MULTIPLIER;

 int PROCESSOR_CLK_MULTIPLIER2;

//size of ROB
 int ROBSIZE ;// 128;		

// maximum commit width
 int MAX_RETIRE ;// 2;

// maximum instruction fetch width
 int MAX_FETCH ;// 4;	

// depth of pipeline
 int PIPELINEDEPTH ;// 5;
// OS CONFIG
 unsigned long long OS_PAGESIZE;
 unsigned long long OS_NUM_RND_TRIES;
/*****************************/
/* DRAM System Configuration */
/*****************************/
// total number of channels in the system
 int NUM_CHANNELS ;// 2;

// number of ranks per channel
 int NUM_RANKS ;// 2;

// number of banks per rank
 int NUM_BANKS ;// 8;

// number of rows per bank
 int NUM_ROWS ;// 32768;

// number of columns per rank
 int NUM_COLUMNS ;// 128;

// cache-line size (bytes)
 int CACHE_LINE_SIZE ;// 64;

// total number of address bits (i.e. indicates size of memory)
 int ADDRESS_BITS ;// 32;

#define MAX_THREADS 64

/***********************************/
/* DRAM Cache System Configuration */
/***********************************/
// total number of channels in the system
 int NUM_CHANNELS2 ;// 4;

// number of ranks per channel
 int NUM_RANKS2 ;// 2;

// number of banks per rank
 int NUM_BANKS2 ;// 8;

// number of rows per bank
 int NUM_ROWS2 ;// 512;

// number of columns per rank
 int NUM_COLUMNS2 ;// 128;

// cache-line size (bytes)
 int CACHE_LINE_SIZE2 ;// 64;

// total number of address bits (i.e. indicates size of memory)
 int ADDRESS_BITS2 ;// 28;


/****************************/
/* DRAM Chip Specifications */
/****************************/

// dram frequency (not datarate) in MHz
 int DRAM_CLK_FREQUENCY ;// 800;

// All the following timing parameters should be 
// entered in the config file in terms of memory 
// clock cycles.

// RAS to CAS delay
 int T_RCD ;// 44;

// PRE to RAS
 int T_RP ;// 44;

// ColumnRD to Data burst
 int T_CAS ;// 44;

// RAS to PRE delay
 int T_RAS ;// 112;

// Row Cycle time
 int T_RC ;// 156;

// ColumnWR to Data burst
 int T_CWD ;// 20;

// write recovery time (COL_WR to PRE)
 int T_WR ;// 48;

// write to read turnaround
 int T_WTR ;// 24;

// rank to rank switching time
 int T_RTRS ;// 8;

// Data transfer
 int T_DATA_TRANS ;// 16;

// Read to PRE
 int T_RTP ;// 24;

// CAS to CAS
 int T_CCD ;// 16;

// Power UP time fast
 int T_XP ;// 20;

// Power UP time slow
 int T_XP_DLL ;// 40;

// Power down entry
 int T_CKE ;// 16;

// Minimum power down duration
 int T_PD_MIN ;// 16;

// rank to rank delay (ACTs to same rank)
 int T_RRD ;// 20;

// four bank activation window
 int T_FAW ;// 128;

// refresh interval
 int T_REFI;

 // refresh cycle time
 int T_RFC;


/**********************************/
/* DRAM cache Chip Specifications */
/**********************************/

// dram frequency (not datarate) in MHz
 int DRAM_CLK_FREQUENCY2 ;// 800;

// All the following timing parameters should be 
// entered in the config file in terms of memory 
// clock cycles.

// RAS to CAS delay
 int T_RCD2 ;// 44;

// PRE to RAS
 int T_RP2 ;// 44;

// ColumnRD to Data burst
 int T_CAS2 ;// 44;

// RAS to PRE delay
 int T_RAS2 ;// 112;

// Row Cycle time
 int T_RC2 ;// 156;

// ColumnWR to Data burst
 int T_CWD2 ;// 20;

// write recovery time (COL_WR to PRE)
 int T_WR2 ;// 48;

// write to read turnaround
 int T_WTR2 ;// 24;

// rank to rank switching time
 int T_RTRS2 ;// 8;

// Data transfer
 int T_DATA_TRANS2 ;// 16;

// Read to PRE
 int T_RTP2 ;// 24;

// CAS to CAS
 int T_CCD2 ;// 16;

// Power UP time fast
 int T_XP2 ;// 20;

// Power UP time slow
 int T_XP_DLL2 ;// 40;

// Power down entry
 int T_CKE2 ;// 16;

// Minimum power down duration
 int T_PD_MIN2 ;// 16;

// rank to rank delay (ACTs to same rank)
 int T_RRD2 ;// 20;

// four bank activation window
 int T_FAW2 ;// 128;

// refresh interval
 int T_REFI2;

 // refresh cycle time
 int T_RFC2;

/****************************/
/* VOLTAGE & CURRENT VALUES */
/****************************/

float VDD;

float IDD0;

float IDD1;

float IDD2P0;

float IDD2P1;

float IDD2N;

float IDD3P;

float IDD3N;

float IDD4R;

float IDD4W;

float IDD5;

/******************************/
/* MEMORY CONTROLLER Settings */
/******************************/

// maximum capacity of write queue (per channel)
 int WQ_CAPACITY ;// 64;

//  int ADDRESS_MAPPING mode
// 1 is consecutive cache-lines to same row
// 2 is consecutive cache-lines striped across different banks 
 int ADDRESS_MAPPING ;// 1;

 // WQ associative lookup 
 int WQ_LOOKUP_LATENCY;


/**********************************/
/* DRAM Cache CONTROLLER Settings */
/**********************************/

// maximum capacity of write queue (per channel)
 int WQ_CAPACITY2 ;// 64;

//  int ADDRESS_MAPPING mode
// 1 is consecutive cache-lines to same row
// 2 is consecutive cache-lines striped across different banks 
 int ADDRESS_MAPPING2 ;// 1;

 // WQ associative lookup 
 int WQ_LOOKUP_LATENCY2;
#endif // __PARAMS_H__

