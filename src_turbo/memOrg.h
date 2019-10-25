#ifndef __MEM_ORG_H__
#define __MEM_ORG_H__

/**************************************/
/* COUNTER DESIGN */
/**************************************/
#include "global_types.h"

#define MONO8_CTR   1 //SGX 8 byte counters - 8 per CL
#define SPLIT16_CTR 2 //Split counters - 16 per CL (2 x 64-bit Major, 16 x 16-bit Minor)
#define SPLIT32_CTR 3 //Split counters - 32 per CL (2 x 48-bit Major, 32 x 9-bit Minor)
#define SPLIT64_CTR 4 //Split counters - 64 per CL (1 x 64-bit Major, 64 x 6-bit Minor)
#define SPLIT32_CTR_v1 5 //Split counters - 32 per CL (1 x 64-bit Major, 32 x 12-bit Minor)
#define SPLIT16_CTR_v1 6 //Split counters - 16 per CL (1 x 64-bit Major, 16 x 24-bit Minor)
#define SPLIT64_CTR_v1 7 //Split counters - 64 per CL (1 x 64-bit Major, 64 x 7-bit Minor)
#define SPLIT128_CTR 8 //Split counters - 128 per CL (1 x 64-bit Major, 128 x 3-bit Minor)
#define SPLIT128_CTR_v1 9 //Split counters - 128 per CL (1 x 64-bit Major, 128 x 3.5-bit Minor)
#define SPLIT256_CTR 10 //Split counters - 256 per CL (1 x 64-bit Major, 256 x 2-bit Minor)
#define SPLIT512_CTR 11 //Split counters - 512 per CL (1 x 64-bit Major, 512 x 1-bit Minor)

#define SPLIT128_FULL_DUAL 12 //Split counters - 128 per CL (1 x 53-bit Major, 2 x 10-bit Medium Counter, 2 x 64 x 3-bit Minor)
#define SPLIT128_UNC_DUAL 13 //Split counters - 128 per CL (1 x 53-bit Major, 2 x 10-bit Medium Counter, 2 x 64 x 3-bit Minor) -> Starts with AZC then devolves to 2-Base Uncompressed.
#define SPLIT128_UNC_7bINT_DUAL 14 //Split counters - 128 per CL (1 x 49-bit Major, 2 x 7-bit Medium Counter, 2 x 64 x 3-bit Minor) -> Starts with AZC then devolves to 2-Base Uncompressed.

/**************************************/

#define MB    (1024*1024)
#define KB    (1024)

#define ENCR_DELAY 40 //cycles

#define CACHE_LINE 64 // Bytes
#define MAC_SIZE   8  // Bytes

typedef long long int ADDR;
typedef long long int INT_64;


typedef struct ctr_mtree_entry {
  ADDR paddr;
  int mtree_level;
  INT_64 entry_num;
} ctr_mtree_entry;

typedef struct memOrg_t {

  INT_64 mem_size;
  INT_64 ctr_store_size;
  INT_64 MAC_store_size;
  INT_64 mtree_store_size;
  
  INT_64 num_Mtree_root;
  INT_64 num_Mtree_levels; 

  ADDR*   mtree_levels_start_addr; //Has the start byte_addr of each Mtree Level [0-num_Mtree_levels]
  uns64*   mtree_level_size; //Has the sizes for each of the levels

} memOrg_t;


void init_memOrg(int addr_bus_num_bits, memOrg_t * mem_org );
  
//Function to return the (byte) address of the MAC, given cacheline physical (byte) address
ADDR getMACAddr( ADDR cacheline_paddr, memOrg_t * mem_org ); 

//Function to return the (byte) address of the Counter, given cacheline physical (byte) address
ctr_mtree_entry getCounterAddr( ADDR cacheline_paddr, memOrg_t * mem_org );
  
//Function to return the (byte) address of the parent MTree entry, given Counter/Mtree entry (byte) address
ctr_mtree_entry getMtreeEntry( ctr_mtree_entry child , memOrg_t * mem_org ); 
ctr_mtree_entry getMtreeEvictParent( ADDR child_paddr, memOrg_t * mem_org );

//Function to debug:
void print(memOrg_t * mem_org);

//Function to return size of usable memory in pages
INT_64 getOSVisibleMem( memOrg_t * mem_org);

//Function returns whether address belongs to data pages or not 
int get_partition(ADDR paddr, memOrg_t * mem_org);

//Helper function to calculate log to base 2
unsigned int log_base2(unsigned int new_value);

//Get the num_children and physical address of first child, for a given counter
ADDR getCtrChild(ADDR ctr_paddr, memOrg_t * mem_org, int* num_children);

#endif
