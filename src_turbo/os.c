#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "params.h"
#include "os.h"


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//#define OS_PAGESIZE 4096
#define OS_NUM_RND_TRIES 20
extern int OS_PAGE_MAPPING;

void os_init_pagesize();

OS *os_new(uns64 os_visible_memory, uns64 num_threads)
{

  OS *os = (OS *) calloc (1, sizeof (OS));

  switch(OS_PAGE_MAPPING) {  
  case 0: //4KB Pages Randomly allotted
    os->os_pagesize = 4096;
    os->page_alloc_isrand = 1;
    break;
  case 1: //2MB Pages Randomly allotted
    os->os_pagesize = 2097152;
    os->page_alloc_isrand = 1;
    break;
  case 2:
    os->os_pagesize = 4096; //4KB Pages First Touch
    os->page_alloc_isrand = 0;
    break;
  case 3: //8KB Pages Randomly allotted
    os->os_pagesize = 8192;
    os->page_alloc_isrand = 1;
    break;
  default:
    ASSERTM(0,"OS_PAGE_MAPPING that isnt 0,1,2 is not supported. \n");
  }
  
  uns64 num_pages = os_visible_memory / os->os_pagesize;
  os->num_pages      = num_pages;
  os->num_threads    = num_threads;
  os->lines_in_page  = os->os_pagesize/CACHE_LINE_SIZE;
  os->pt     = (PageTable *) calloc (1, sizeof (PageTable));
  os->pt->entries     = (Hash_Table *) calloc (1, sizeof(Hash_Table));
  init_hash_table(os->pt->entries, "PageTableEntries", 4315027, sizeof( PageTableEntry ));
  os->pt->max_entries = os->num_pages;

  os->ipt     = (InvPageTable *) calloc (1, sizeof (InvPageTable));
  os->ipt->entries = (InvPageTableEntry *) calloc (os->num_pages, sizeof (InvPageTableEntry));
  os->ipt->num_entries = os->num_pages;

  //Used for first touch page allocation 
  os->last_allotted_pagenum = -1;
  
  assert(os->pt->entries);
  assert(os->ipt->entries);
  
  
  printf("Initialized OS for %llu pages and %llu threads\n", num_pages, num_threads);
  switch(OS_PAGE_MAPPING) {
    
  case 0: //4KB Pages Randomly allotted
    printf("OS_PAGE_MAPPING: %d, Random %llu KB pages ",OS_PAGE_MAPPING,os->os_pagesize/1024);
    break;
  case 1: //2MB Pages Randomly allotted
    printf("OS_PAGE_MAPPING: %d, Random %llu KB pages ",OS_PAGE_MAPPING,os->os_pagesize/1024);
    break;
  case 2:
    printf("OS_PAGE_MAPPING: %d, First-touch serial %llu KB pages ",OS_PAGE_MAPPING,os->os_pagesize/1024);
    break;
  case 3: //8KB Pages Randomly allotted
    printf("OS_PAGE_MAPPING: %d, Random %llu KB pages ",OS_PAGE_MAPPING,os->os_pagesize/1024);
    break;
  default:
    ASSERTM(0,"OS_PAGE_MAPPING that isnt 0,1,2,3 is not supported. \n");
  }
  return os;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Addr os_vpn_to_pfn(OS *os, Addr vpn, uns64 tid, Flag *hit)
{
  Flag first_access;
  PageTable *pt = os->pt;
  InvPageTable *ipt = os->ipt;
  PageTableEntry *pte;
  InvPageTableEntry *ipte;
  *hit = TRUE;

  assert(vpn>>60 == 0);
  vpn = (((Addr)tid)<<60)+vpn; // embed tid information in high bits
    
  if( pt->last_xlation[tid].vpn == vpn ){
    return pt->last_xlation[tid].pfn;
  }
    
  pte = (PageTableEntry *) hash_table_access_create(pt->entries, vpn, &first_access);

  if(first_access){
    pte->pfn = os_get_victim_from_ipt(os);
    ipte = &ipt->entries[ pte->pfn ]; 
    ipte->valid = TRUE;
    ipte->dirty = FALSE;
    ipte->vpn   = vpn;
    assert( (uns)pt->entries->count <= pt->max_entries);
    pt->miss_count++;
    *hit=FALSE;
  }

  ipte = &ipt->entries[ pte->pfn ]; 
  ipte->ref = TRUE;
    
  pt->last_xlation[tid].vpn = vpn;
  pt->last_xlation[tid].pfn = pte->pfn;

  return pte->pfn;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns64    os_get_victim_from_ipt(OS *os)
{
  PageTable *pt = os->pt;
  InvPageTable *ipt = os->ipt;
  uns64 ptr = ipt->refptr;
  uns64 max = ipt->num_entries;
  Flag found=FALSE;
  uns64 victim=0;
  uns64 random_invalid_tries=OS_NUM_RND_TRIES;
  uns64 tries=0;
  tries=tries; // To avoid warning
  random_invalid_tries=random_invalid_tries; //To avoid warning

  if(os->page_alloc_isrand == 0){
    //If first_touch policy for page mapping:
    //Check if page after last_allotted_page is invalid / available
    os->last_allotted_pagenum++;
    victim = ( (os->last_allotted_pagenum) %  ipt->num_entries ) ;
    if(! ipt->entries[victim].valid ){
      found = TRUE;
    }
  }
  else{
    //If random policy for page_alloc, try random invalid page exists
    while( tries < random_invalid_tries){
      victim = rand()%max;
      if(! ipt->entries[victim].valid ){
        found = TRUE;
        break;
      }
      tries++;
    }
  }
  // try finding a victim if no invalid victim
  while(!found){
    if( ! ipt->entries[ ptr ].valid ){
      found = TRUE;
    }
	
    if( ipt->entries[ ptr ].valid && ipt->entries[ ptr ].ref == FALSE){
      found = hash_table_access_delete(pt->entries, ipt->entries[ptr].vpn);
      assert(found);
    }else{
      ipt->entries[ptr].ref = FALSE;
    }
    victim = ptr;
    ipt->refptr = (ptr+1)%max;
    ptr = ipt->refptr;
  }
  // update page writeback information
  if( ipt->entries[victim].valid){
    pt->total_evicts++;
    if(ipt->entries[victim].dirty ){
      pt->evicted_dirty_page++;
    }
  }
  return victim; 
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void os_print_stats(OS *os)
{
  char header[256];
  sprintf(header, "OS");
    
  printf("\n\n");
  printf("\n%s_PAGE_MISS       \t : %llu",  header, os->pt->miss_count);
  printf("\n%s_PAGE_EVICTS     \t : %llu",  header, os->pt->total_evicts);
  printf("\n%s_FOOTPRINT       \t : %llu",  header, (os->pt->miss_count*os->os_pagesize)/(1024*1024));
  printf("\n");

}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Addr os_v2p_lineaddr(OS *os, Addr lineaddr, uns64 tid){
  Addr vpn = lineaddr/os->lines_in_page;
  Addr lineid = lineaddr%os->lines_in_page;
  Flag pagehit;
  Addr pfn = os_vpn_to_pfn(os, vpn, tid, &pagehit);
  Addr retval = (pfn*os->lines_in_page)+lineid;
  return retval;
}
