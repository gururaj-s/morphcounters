#include<stdio.h>
#include<assert.h>

#include "memOrg.h"
int SGX_MODE = 1;

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

  
  memOrg_t SGX;
  ADDR test_addr = 0xfff;
  ctr_mtree_entry entry;
  ctr_mtree_entry ctr;
  //init_memOrg(32, &SGX); 
  init_memOrg(34, &SGX); 
  
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

  test_addr = 0;
  entry = getCounterAddr(test_addr, &SGX);
  
  while( entry.mtree_level != 0){
    printf("level: %d, Address %llu\n", entry.mtree_level, entry.paddr);
    entry = getMtreeEntry(entry, &SGX);
  }

  /*
  for(ADDR i = 0; i< getOSVisibleMem( &SGX); i++){
    ctr = getCounterAddr(i, &SGX);
    assert(checkMtreeEvictFunc(ctr, &SGX) == 1);

  }

  printf("getMtreeEvict successfully functions\n" );
  */
  return 0;
  

}
