#include<stdio.h>
#include<assert.h>

#include "memOrg.h"
#include "ctr_sim.h"

int SGX_MODE = 1;
long long int CYCLE_VAL = 0;

int checkMtreeEvictFunc(ctr_mtree_entry child, memOrg_t* SGX){
  ctr_mtree_entry parent1 = getMtreeEvictParent(child.paddr, SGX);
  ctr_mtree_entry parent2 = getMtreeEntry(child, SGX);
  //printf(" Parent1 has Paddr: %d,Parent2 has Paddr: %d",parent1.paddr,parent2.paddr );

  assert(parent1.paddr == parent2.paddr);
  assert(parent1.entry_num == parent2.entry_num);
  assert(parent1.mtree_level == parent2.mtree_level);

  if(child.mtree_level == 1)
    return 1;
  else 
    return checkMtreeEvictFunc(parent1,SGX);
  
}

int main(){

  srand(42);
  memOrg_t SGX;
  ctr_cl*  ctr_cls;
  ctr_type* ctr_types;

  ADDR test_addr = 0xfff;
  ctr_mtree_entry entry;
  ctr_mtree_entry ctr;
  init_memOrg(32, &SGX); 
  
  ctr_types = (ctr_type*) calloc(1,sizeof(ctr_type));
  init_ctr_types(&SGX, ctr_types);
  
  
  ctr_cls = (ctr_cl*) calloc (ctr_types->num_ctr_cls,sizeof(ctr_cl));
  init_ctr_cls(&SGX, ctr_cls, ctr_types);


  print(&SGX);
  /*
    printf("MAC Address of %llx  is : %llx \n", test_addr, getMACAddr(test_addr, &SGX));
    printf("Counter Address of  %llx  is : %llx \n", test_addr, getCounterAddr(test_addr, &SGX).paddr); 

    entry = getCounterAddr(test_addr, &SGX);

    printf("Counter at Mtree Level: %d, Entry Number :  %llx, Addr :  %llx \n", entry.mtree_level, entry.entry_num, entry.paddr); 
  
    while (entry.mtree_level != 0){

    entry = getMtreeEntry (entry, &SGX);
    printf("Mtree Level: %d, Entry Number :  %llx, Addr :  %llx \n", entry.mtree_level, entry.entry_num, entry.paddr); 
    }
  */
  /*
  for(ADDR i = 0; i< getOSVisibleMem( &SGX); i++){
    ctr = getCounterAddr(i, &SGX);
    assert(checkMtreeEvictFunc(ctr, &SGX) == 1);
    
    
  }
  */

  for(int i=0;i<1000;i++){   
    ADDR data_addr = i*10000;
    ctr = getCounterAddr((ADDR)data_addr, &SGX);
    CYCLE_VAL = i*1000;
    reset_stat_overflows(&SGX, ctr_cls, ctr_types);
    printf("Ctr Paddr is : %llu\n",ctr.paddr);
    update_ctr_cls(&SGX, ctr_cls, ctr_types, ctr.paddr);
    
    //Initialize the ctr_cls based on ctr_types
    for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){

      for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){

	for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
	  if(ctr_cls[ctr_cl_num].minor_ctr_val[major_ctr_num][minor_ctr_num] != 0){
	    printf("Data Addr:%d & CCL NUM: %d has entry with entry_num %d, and major ctr:%d _val %d, minor ctr: %d _val %d\n",data_addr,ctr_cl_num, ctr_cls[ctr_cl_num].entry_num, major_ctr_num, ctr_cls[ctr_cl_num].major_ctr_val[major_ctr_num],minor_ctr_num, ctr_cls[ctr_cl_num].minor_ctr_val[major_ctr_num][minor_ctr_num]); 
	  }

	}

      }

    }

  }
  printf("getMtreeEvict successfully functions\n" );
  
  return 0;
  

}
