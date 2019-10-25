#ifndef OS_H
#define OS_H

#include "global_types.h"
#include "hash_lib.h"
#include "memOrg.h"

#define OS_MAX_THREADS 64


typedef struct OS                OS;
typedef struct InvPageTableEntry InvPageTableEntry;
typedef struct PageTableEntry    PageTableEntry;
typedef struct PageTable         PageTable;
typedef struct InvPageTable      InvPageTable;
typedef struct VirtualPhysicalPair VirtualPhysicalPair;


struct PageTableEntry{
  Addr  pfn;
};


struct VirtualPhysicalPair{
  Addr vpn;
  Addr pfn;
};


struct PageTable{
    Hash_Table  *entries;
    VirtualPhysicalPair last_xlation[OS_MAX_THREADS];
    uns64        max_entries;
    uns64        miss_count;
    uns64        total_evicts;
    uns64        evicted_dirty_page;
};

struct InvPageTableEntry{
    Flag valid;
    Flag dirty;
    Flag ref;
  Addr  vpn;
};

struct InvPageTable{
  InvPageTableEntry  *entries;
  uns64          num_entries;
  uns64          refptr;
};


//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


struct OS {
    PageTable        *pt;
    InvPageTable     *ipt;

    uns64               lines_in_page;
    uns64               num_threads;
    uns64             num_pages;

  //Page Mapping policy state variables
  uns64 os_pagesize;
  uns64 page_alloc_isrand; //Page allocation policy Random or First-Touch
  INT_64 last_allotted_pagenum;
};



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

OS*     os_new(uns64 os_visible_memory, uns64 num_threads);
Addr     os_vpn_to_pfn(OS *os, Addr vpn, uns64 tid, Flag *hit);
void    os_print_stats(OS *os);

uns64     os_get_victim_from_ipt(OS *os);
Addr    os_v2p_lineaddr(OS *os, Addr lineaddr, uns64 tid);

//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#endif // OS_H
