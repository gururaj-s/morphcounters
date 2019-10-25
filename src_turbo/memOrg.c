// Copyright (C391) 2017, Gururaj Saileshwar
// 
// Filename: memOrg.cpp
// Description: Defines the layout of memory - MAC,Counters etc.
// Author: Gururaj
// Created: Mon Jan 23 21:17:57 2017 (-0500)
// Last-Updated: Thu Mar 15 02:20:18 2018 (-0400)
//     Update #: 119
// 

// Commentary: 
// -Functions can be used to get the address of a particular MAC,Counter, Mtree entry
// -Functions (to be written) can also return what areas memory can be usable by data
// 
//  Memory organisation is DATA : MTREE+SOME_EMPTY_SPACE : CTRs : MACs

// Code:
#include "memOrg.h"
#include "global_types.h"
#include "params.h"
//#include "debug.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

extern int SGX_MODE[4];

//Design of Counters
int CTR_DESIGN = (MONO8_CTR);
double CTR_SIZE   = 8;
int CTRS_PER_MTREE = 8; //Arity below leaf-level 

//Design of Mtree Counters - Leaf,Parent, GrandParent, GreatGrandParent, All others

int MTREE_CTR_DESIGN_VAR[5] = {SPLIT64_CTR_v1,SPLIT64_CTR_v1,SPLIT64_CTR_v1,SPLIT64_CTR_v1,SPLIT64_CTR_v1};
double MTREE_ENTRY_SIZE_VAR[5];

int* MTREE_CTR_DESIGN;
double* MTREE_ENTRY_SIZE;
int* MTREE_ARY;

/*
  int MTREE_CTR_DESIGN = (SPLIT32_CTR_v1);
  double MTREE_ENTRY_SIZE = 2;
  int MTREE_ARY  = 32; //Num counters in the current mtree line, decide the arity above leaf-level mtree.
*/

//Helper function to calculate log to base 2
unsigned int log_base2(unsigned int new_value)
{
  int i;
  for (i = 0; i < 32; i++) {
    new_value >>= 1;
    if (new_value == 0)
      break;
  }
  return i;
}


//Constructor
void init_memOrg(int addr_bus_num_bits, memOrg_t * mem_org){

  //Initialize CTR_SIZE, CTRS_PER_MTREE
  if(CTR_DESIGN == MONO8_CTR){
    CTR_SIZE=8;
    CTRS_PER_MTREE=8; 
  }  
  else if(CTR_DESIGN == SPLIT16_CTR){
    CTR_SIZE=4;
    CTRS_PER_MTREE=8;
  }
  else if( CTR_DESIGN == SPLIT32_CTR ){
    CTR_SIZE=2;
    CTRS_PER_MTREE=16;
  }
  else if (CTR_DESIGN == SPLIT64_CTR){
    CTR_SIZE=1;
    CTRS_PER_MTREE=64;
  }
  else if (CTR_DESIGN == SPLIT128_CTR){
    CTR_SIZE=0.5;
    CTRS_PER_MTREE=128;
  }
  else if ( (CTR_DESIGN == SPLIT128_FULL_DUAL) || (CTR_DESIGN == SPLIT128_UNC_DUAL)  || (CTR_DESIGN == SPLIT128_UNC_7bINT_DUAL) ){
    CTR_SIZE=0.5;
    CTRS_PER_MTREE=128;
  }  
  else if(CTR_DESIGN == SPLIT32_CTR_v1){
    CTR_SIZE=2;
    CTRS_PER_MTREE=32;
  } 
  else if(CTR_DESIGN == SPLIT16_CTR_v1){
    CTR_SIZE=4;
    CTRS_PER_MTREE=16;
  }
  else if (CTR_DESIGN == SPLIT64_CTR_v1){
    CTR_SIZE=1;
    CTRS_PER_MTREE=64;
  }
  else if (CTR_DESIGN == SPLIT128_CTR_v1){
    CTR_SIZE=0.5;
    CTRS_PER_MTREE=128;
  }  
  else if (CTR_DESIGN == SPLIT256_CTR){
    CTR_SIZE=0.25;
    CTRS_PER_MTREE=256;
  }  
  else if (CTR_DESIGN == SPLIT512_CTR){
    CTR_SIZE=0.125;
    CTRS_PER_MTREE=512;
  }  
  else {
    ASSERTM(0,"CTR Design not supported");
  }

  
  //Initialize the Merkle Tree Design
  for(int i=0;i<5;i++){
    
    if(MTREE_CTR_DESIGN_VAR[i] == MONO8_CTR){
      MTREE_ENTRY_SIZE_VAR[i]=8;
    }  
    else if(MTREE_CTR_DESIGN_VAR[i] == SPLIT16_CTR){
      MTREE_ENTRY_SIZE_VAR[i]=4;
    }
    else if( MTREE_CTR_DESIGN_VAR[i] == SPLIT32_CTR ){
      MTREE_ENTRY_SIZE_VAR[i]=2;
    }
    else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT64_CTR){
      MTREE_ENTRY_SIZE_VAR[i]=1;
    }
    else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT128_CTR){
      MTREE_ENTRY_SIZE_VAR[i]=0.5;
    }
    else if(MTREE_CTR_DESIGN_VAR[i] == SPLIT32_CTR_v1){
      MTREE_ENTRY_SIZE_VAR[i]=2;
    } 
    else if(MTREE_CTR_DESIGN_VAR[i] == SPLIT16_CTR_v1){
      MTREE_ENTRY_SIZE_VAR[i]=4;
    }
    else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT64_CTR_v1){
      MTREE_ENTRY_SIZE_VAR[i]=1;
    }
    else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT128_CTR_v1){
      MTREE_ENTRY_SIZE_VAR[i]=0.5;
    }
    else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT256_CTR){
      MTREE_ENTRY_SIZE_VAR[i]=0.25;
    }
    else if (MTREE_CTR_DESIGN_VAR[i] == SPLIT512_CTR){
      MTREE_ENTRY_SIZE_VAR[i]=0.125;
    }
    else {
      ASSERTM(0,"MTREE CTR Design not supported");
    }
    
  }

  INT_64 Mtree_size;
  mem_org -> mem_size = ((INT_64) 1 ) << addr_bus_num_bits;
  mem_org -> ctr_store_size = ( (((INT_64) 1 ) << addr_bus_num_bits ) /CACHE_LINE) * CTR_SIZE;
  mem_org -> MAC_store_size = (((INT_64) 1 ) << addr_bus_num_bits ) / CACHE_LINE * MAC_SIZE;

  //Determine number of Mtree Levels
  uns64 num_mtree_levels = 0;
  uns64 level_size = mem_org -> ctr_store_size;
  uns64 mtree_total_size = 0;
  
  while(level_size > CACHE_LINE){
    int index = num_mtree_levels;
    if(index >= 4)
      index =4;

    uns64 num_mtree_prevlevel_elements = level_size / CACHE_LINE;
    uns64 prev_level_size = num_mtree_prevlevel_elements * MTREE_ENTRY_SIZE_VAR[index];

    level_size = prev_level_size;
    num_mtree_levels++;
    mtree_total_size += level_size;
  }

  mem_org -> num_Mtree_levels = num_mtree_levels;
  int temp_index = (num_mtree_levels-1);
  if(temp_index >=4)
    temp_index = 4;
  mem_org -> num_Mtree_root = level_size / MTREE_ENTRY_SIZE_VAR[temp_index];
  
  uns64 mtree_leaf_size = mem_org -> ctr_store_size / CACHE_LINE * MTREE_ENTRY_SIZE_VAR[0];

  if(mtree_leaf_size * 2 > mtree_total_size)
    Mtree_size = mtree_leaf_size*2;
  else
    Mtree_size = mtree_total_size;

  mem_org ->mtree_store_size = Mtree_size;
  
  MTREE_CTR_DESIGN = (int*) calloc(mem_org -> num_Mtree_levels, sizeof(int));
  MTREE_ENTRY_SIZE =  (double*) calloc(mem_org -> num_Mtree_levels, sizeof(double));
  MTREE_ARY =  (int*) calloc(mem_org -> num_Mtree_levels, sizeof(int));

  for(int i=0; i<mem_org -> num_Mtree_levels; i++){
    int index = i;
    if(index >=4){
      index = 4;
    }
    MTREE_CTR_DESIGN[mem_org->num_Mtree_levels - 1 - i] = MTREE_CTR_DESIGN_VAR[index];
    MTREE_ENTRY_SIZE[mem_org->num_Mtree_levels - 1 - i] = MTREE_ENTRY_SIZE_VAR[index]; 
    MTREE_ARY[mem_org->num_Mtree_levels - 1 - i] = (int) (1.0*CACHE_LINE/MTREE_ENTRY_SIZE[mem_org->num_Mtree_levels - 1 - i]);    
  }
  /*
    MTREE_ARY = (int) (1.0*CACHE_LINE/MTREE_ENTRY_SIZE);
  
    int twoexp_ary = log_base2(MTREE_ARY);

    INT_64 num_counters = mem_org -> mem_size / CACHE_LINE;
    INT_64 num_mtree_leaves = num_counters / CTRS_PER_MTREE;
  


    mem_org -> num_Mtree_root   =  ((INT_64)1) << ( ((INT_64)log_base2(num_mtree_leaves)) % ((INT_64) twoexp_ary) );

    //mem_org -> num_Mtree_levels = ((INT_64) log_base2(num_counters*CTR_SIZE/MTREE_ENTRY_SIZE) ) / twoexp_ary; //  corrected since using 1 Byte counters instead of 8 Byte counters (reduces merkle tree by 1 level)

    mem_org -> num_Mtree_levels =  ((INT_64)log_base2(num_mtree_leaves)) /((INT_64) twoexp_ary) + 1;
    
    if(mem_org -> num_Mtree_root == 0){
    mem_org -> num_Mtree_levels -= 1;
    mem_org -> num_Mtree_root = MTREE_ARY;
    }

  
    //  Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);
    Mtree_size = num_mtree_leaves * MTREE_ENTRY_SIZE * 2;  
  */
  
  //Initialize size of each level in bytes

  
  mem_org -> mtree_level_size = (uns64*)  calloc(mem_org -> num_Mtree_levels, sizeof(uns64));
  mem_org -> mtree_level_size[0]= (CACHE_LINE); //Root is always single cacheline
  
  for(int i=1;i<mem_org->num_Mtree_levels; i++){
    if(i==1){
      mem_org -> mtree_level_size[i] =  mem_org -> mtree_level_size[i-1] * mem_org -> num_Mtree_root; 
    }
    else{
      mem_org -> mtree_level_size[i] = mem_org -> mtree_level_size[i-1]*MTREE_ARY[i-1];      
    }
    printf("Level %d size with arity %d: %llu B \n",i,MTREE_ARY[i],mem_org -> mtree_level_size[i]);
  }
  
  //ASSERTM( ((uns64)(mem_org -> mtree_level_size[mem_org->num_Mtree_levels-1]/MTREE_ENTRY_SIZE)) == (mem_org ->ctr_store_size/CACHE_LINE),"Arithmetic regarding sizes of mtree & counters does not adds up\n");
  ASSERTM( ((uns64)((mem_org -> mtree_level_size[mem_org->num_Mtree_levels-1]/MTREE_ENTRY_SIZE[mem_org->num_Mtree_levels-1])*(CTRS_PER_MTREE*CTR_SIZE)) ) == mem_org ->ctr_store_size,"Arithmetic regarding sizes of mtree & counters does not adds up\n");

  //Initialize mtree_levels_start_addr
  mem_org -> mtree_levels_start_addr = (ADDR*) calloc(mem_org -> num_Mtree_levels + 1, sizeof(ADDR));
  mem_org -> mtree_levels_start_addr[0] = mem_org->mem_size - mem_org->MAC_store_size - mem_org->ctr_store_size - Mtree_size; 
  mem_org -> mtree_levels_start_addr[mem_org -> num_Mtree_levels] =  mem_org -> mem_size - mem_org -> MAC_store_size - mem_org -> ctr_store_size; 
  
  for(int i=1; i < mem_org -> num_Mtree_levels; i++){
    if(i==1)
      mem_org -> mtree_levels_start_addr[i] = mem_org -> mtree_levels_start_addr[0] + CACHE_LINE;

    else{
      //mem_org -> mtree_levels_start_addr[i] =  mem_org -> mtree_levels_start_addr[i-1] + (uns64)((mem_org -> num_Mtree_root)*( ((ADDR)1)<<(twoexp_ary*(i-1)) )*MTREE_ENTRY_SIZE); 
      mem_org -> mtree_levels_start_addr[i] =  mem_org -> mtree_levels_start_addr[i-1] +  mem_org ->mtree_level_size[i-1];
    }
    ASSERTM((mem_org -> mtree_levels_start_addr[i] - mem_org -> mtree_levels_start_addr[i-1] ) == mem_org ->mtree_level_size[i-1], "Address calculation tallies with mtree level sizes\n");
    
  }

  printf("**MemOrg initialized with %llu levels\n", mem_org-> num_Mtree_levels);
  
  print(mem_org);
}

//Return the MAC address, given a cacheline addr
ADDR getMACAddr( ADDR cacheline_paddr, memOrg_t * mem_org ){

  ADDR start_MAC_paddr = mem_org -> mem_size - mem_org -> MAC_store_size;
  ADDR cacheline_num   = cacheline_paddr / CACHE_LINE; 


  ADDR result_MAC_paddr = start_MAC_paddr + cacheline_num * MAC_SIZE ;

  return result_MAC_paddr;
  
}


//Return the Counter address, given a cacheline addr
ctr_mtree_entry getCounterAddr( ADDR cacheline_paddr, memOrg_t * mem_org ){

  ADDR start_Counter_paddr = mem_org -> mem_size - mem_org -> MAC_store_size - mem_org -> ctr_store_size;
  ADDR cacheline_num   = cacheline_paddr >> ((uns64)log2(CACHE_LINE));

  ADDR result_Counter_paddr = start_Counter_paddr + (ADDR)(cacheline_num * CTR_SIZE);
  ADDR temp_result_Counter_paddr = start_Counter_paddr + (double)cacheline_num*(double)CTR_SIZE;
  
  //printf("GETCTRADDR0: Ctr_paddr: %llu, Mtree_Level_Start: %llu, Entry: %llu, tempCtrpaddr %lld\n",result_Counter_paddr,start_Counter_paddr,cacheline_num, temp_result_Counter_paddr);

  ADDR ctr_level_end_addr;

  ctr_mtree_entry dest;
  dest.mtree_level = mem_org -> num_Mtree_levels;
  dest.entry_num   = cacheline_num;
  dest.paddr       = result_Counter_paddr;

  //printf("GETCTRADDR1: Ctr_paddr: %llu, Mtree_Level_Start: %llu, Entry: %llu\n",dest.paddr,  mem_org -> mtree_levels_start_addr[dest.mtree_level], dest.entry_num);

  ctr_level_end_addr = mem_org -> mtree_levels_start_addr[dest.mtree_level] + mem_org -> ctr_store_size;

  //  if(!((dest.paddr >= start_Counter_paddr) && (dest.paddr < ctr_level_end_addr))){
  //printf("Counter Addr : %llu, dest entry: %llu, src entry: %llu, lower bound on mtree level :%llu, %llu, tight upper bound on mtree level :%llu\n",dest.paddr, dest.entry_num, cacheline_num, dest.mtree_level,  start_Counter_paddr, ctr_level_end_addr);
  //}    
  ASSERTM(((dest.paddr >= start_Counter_paddr) && (dest.paddr < ctr_level_end_addr)),"Calculation mistake with getCounterAddr\n");	      

  return dest;

}



//Return   the Mtree address, given a counter/Mtree addr
ctr_mtree_entry getMtreeEntry( ctr_mtree_entry src, memOrg_t * mem_org ){

  //Assumed the size of Counter Store is 0.125*Mem_Size
  //Assumed the size of Mtree is 0.25 of the Counter Store
  //Leaf level of Mtree is 0.125*CTR_Store
  //Upper levels will be less than another 0.125*CTR_Store

  INT_64 num_counters = mem_org -> mem_size / CACHE_LINE;
  INT_64 num_mtree_leaves = num_counters / CTRS_PER_MTREE;

  //  INT_64 Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);
  //INT_64 Mtree_size =  (INT_64) (num_mtree_leaves * MTREE_ENTRY_SIZE * 2);  
  INT_64 Mtree_size = mem_org -> mtree_store_size;
  
  ADDR Mtree_start = mem_org -> mem_size - mem_org -> MAC_store_size - mem_org -> ctr_store_size - Mtree_size; 

  //  ADDR Ctr_start = mem_org -> mem_size -  mem_org -> MAC_store_size - mem_org -> ctr_store_size;
    
  if(src.mtree_level == 0)
    return src;

  else if ( (src.mtree_level > 0) && (src.mtree_level <= mem_org -> num_Mtree_levels ) ){
    ADDR dest_level_end_addr   = 0 ;
    ctr_mtree_entry dest ;
    //    int twoexp_ary = log_base2(MTREE_ARY);
    if(src.mtree_level == mem_org ->num_Mtree_levels){
      //src is counter
      dest.entry_num = src.entry_num / CTRS_PER_MTREE;
    }
    else{
      //src is merkle tree
      dest.entry_num = src.entry_num / MTREE_ARY[src.mtree_level];
    } 
    dest.mtree_level = --src.mtree_level;
    /*
      ADDR Mtree_level_start ;

      if(dest.mtree_level > 0)
      //      Mtree_level_start = Mtree_start + CACHE_LINE + mem_org-> num_Mtree_root * MTREE_ARY*( pow(MTREE_ARY, dest.mtree_level-1 ) - 1 )/(MTREE_ARY - 1) * MTREE_ENTRY_SIZE  ;
      Mtree_level_start = Mtree_start + CACHE_LINE + mem_org-> num_Mtree_root * MTREE_ARY*( ( ((INT_64) 1) << ( log_base2(MTREE_ARY)*(dest.mtree_level-1) ) ) - 1 )/(MTREE_ARY - 1) * MTREE_ENTRY_SIZE  ;
      else
      Mtree_level_start = Mtree_start;

      dest.paddr = Mtree_level_start + dest.entry_num * MTREE_ENTRY_SIZE ;
    */

    dest.paddr = mem_org -> mtree_levels_start_addr[dest.mtree_level] + (uns64) (dest.entry_num * MTREE_ENTRY_SIZE[dest.mtree_level]) ;

    dest_level_end_addr = mem_org -> mtree_levels_start_addr[dest.mtree_level] + mem_org -> mtree_level_size[dest.mtree_level];
    if(!((dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < dest_level_end_addr))){
      printf("Dest Addr : %llu, dest entry: %llu, src addr: %llu, src entry :%llu, lower bound on mtree level :%llu, %llu, tight upper bound on mtree level :%llu\n",dest.paddr, dest.entry_num,src.entry_num, src.paddr, dest.mtree_level, mem_org->mtree_levels_start_addr[dest.mtree_level], dest_level_end_addr);
    }    
    ASSERTM( (dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < dest_level_end_addr),"Calculation mistake with getMtreeEntry\n");	      

    //TODO:ADD assert about mtree_level of dest with paddr checl.
    //    std::cout << "Mtree_start " << Mtree_start << ", Mtree_level_start;
    if(!((dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < mem_org->mtree_levels_start_addr[dest.mtree_level+1]) && ((dest.mtree_level + 1) <= mem_org->num_Mtree_levels))){
      printf("Dest Addr : %llu, dest entry: %llu, src entry :%llu, lower bound on mtree level :%llu, %llu,  upper bound on mtree level :%llu, %llu \n",dest.paddr, dest.entry_num,src.entry_num, dest.mtree_level, mem_org->mtree_levels_start_addr[dest.mtree_level],dest.mtree_level+1,mem_org->mtree_levels_start_addr[dest.mtree_level+1]);
    }
    
    ASSERTM( (dest.paddr >= mem_org->mtree_levels_start_addr[dest.mtree_level]) && (dest.paddr < mem_org->mtree_levels_start_addr[dest.mtree_level+1]) && ((dest.mtree_level + 1) <= mem_org->num_Mtree_levels),"Calculation mistake with getMtreeEntry\n");



    return dest;

  }

  else {
    printf( "Invalid invocation of getMtreeEntry\n");
    exit(1);
  }
  
}



void print(memOrg_t * mem_org ){
  printf( "Counter Design : %d \n", CTR_DESIGN);
  printf( "CTR SIZE : %f \n", CTR_SIZE);
  printf( "CTRS_PER_MTREE : %d \n", CTRS_PER_MTREE);
  printf("\n");

  for(int i=mem_org->num_Mtree_levels -1 ; i>= 0;i--){
    printf( "MTREE Level %d Counter Design : %d \n",i, MTREE_CTR_DESIGN[i]);
    printf( "MTREE Level %d Entry size : %f \n",i, MTREE_ENTRY_SIZE[i]);
    printf( "MTREE Level %d Arity : %d \n",i, MTREE_ARY[i]);
  }

  printf( "Memory Size is : %llu \n", mem_org -> mem_size);
  printf( "CTR Store Size is : %llu \n", mem_org -> ctr_store_size);
  printf( "MAC Store Size is : %llu \n", mem_org -> MAC_store_size); 
  printf( "Num_Mtree_root is : %llu \n", mem_org -> num_Mtree_root);
  printf( "Num_Mtree_levels is : %llu \n", mem_org -> num_Mtree_levels);
  printf( "\n" );
  for(int i=0 ; i < mem_org-> num_Mtree_levels; i++){
    printf( "Mtree Level %d Size: %llu \n", i, mem_org->mtree_level_size[i]);
  }

  printf("Start Addr Counters: %llu \n", mem_org->mtree_levels_start_addr[ mem_org-> num_Mtree_levels]);

  printf( "\n" );
  for(int i=0 ; i < mem_org-> num_Mtree_levels; i++){
    printf( "Start Addr Mtree Level %d: %llu \n", i, mem_org->mtree_levels_start_addr[i]);
  }

  printf("Start Addr Counters: %llu \n", mem_org->mtree_levels_start_addr[ mem_org-> num_Mtree_levels]);
 
}


//Function to return size of usable memory in pages
INT_64 getOSVisibleMem( memOrg_t * mem_org){
  //INT_64 Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);

  INT_64 num_counters = mem_org -> mem_size / CACHE_LINE;
  INT_64 num_mtree_leaves = num_counters / CTRS_PER_MTREE;
  //INT_64 Mtree_size =   num_mtree_leaves * MTREE_ENTRY_SIZE * 2;
  INT_64 Mtree_size =   mem_org->mtree_store_size;  
  INT_64 usable_memory = mem_org -> mem_size - mem_org -> MAC_store_size  -  mem_org -> ctr_store_size - Mtree_size;
  
  return (usable_memory); //Return memory in Bytes visible to OS
  
}

//Function returns whether address belongs to data pages or not 
int get_partition(ADDR paddr, memOrg_t * mem_org){
  //  INT_64 Mtree_size = (mem_org -> ctr_store_size / MTREE_ARY * 2);

  INT_64 num_counters = mem_org -> mem_size / CACHE_LINE;
  INT_64 num_mtree_leaves = num_counters / CTRS_PER_MTREE;
  //INT_64 Mtree_size =   num_mtree_leaves * MTREE_ENTRY_SIZE * 2;
  INT_64 Mtree_size =   mem_org->mtree_store_size;  


  INT_64 usable_memory = mem_org -> mem_size - mem_org -> MAC_store_size  -  mem_org -> ctr_store_size - Mtree_size;
  if( (SGX_MODE[0] == 0) && (SGX_MODE[1] == 0) && (SGX_MODE[2] == 0) && (SGX_MODE[3] == 0) ){
    return 0;
  }

  if(paddr < usable_memory)
    return 0; //Usable memory
  else if( paddr < (usable_memory + Mtree_size) )
    return 1; //Merkle Tree Entries
  else if( paddr < (usable_memory + Mtree_size + mem_org -> ctr_store_size ) )
    return 2; //Counters
  else if( paddr < (usable_memory + Mtree_size + mem_org -> ctr_store_size + mem_org -> MAC_store_size ) )
    return 3; //MACs
  else {
    printf("********ERROR in get_partition in memOrg.c ********\n");
    //printf("padddr: %llu, usable : %lld, mtree_store: %lld,  ctr_store; %lld , mac_store: %lld\n", paddr, usable_memory,Mtree_size, mem_org->ctr_store_size, mem_org->MAC_store_size);
    print_backtrace();
    exit(1);
  }

}


ctr_mtree_entry getMtreeEvictParent( ADDR met_paddr, memOrg_t * mem_org ){
  ctr_mtree_entry parent,child;
  int met_partition = get_partition(met_paddr, mem_org);
  if((met_partition != 1) && (met_partition != 2)){
     print_backtrace();
  }
  ASSERTM( (met_partition == 1) || (met_partition == 2), "ERROR: getMtreEvictParent has been called with paddr not lying within MET" );

  int src_level = -1;
  int src_entry_num = -1;
  
  for(int i= mem_org -> num_Mtree_levels; i >= 0 ; i--){

    if(met_paddr >= mem_org -> mtree_levels_start_addr[i]){
      src_level = i;
      break;
    }

  }
  ASSERTM(src_level != -1, "ERROR: Unable to assert src_level in getMtreeEvictParent");

  if(src_level == mem_org-> num_Mtree_levels){
    src_entry_num = (uns64) ( (met_paddr - mem_org -> mtree_levels_start_addr[src_level])/CTR_SIZE );
  }
  else{
    src_entry_num = (uns64) ( (met_paddr - mem_org -> mtree_levels_start_addr[src_level])/MTREE_ENTRY_SIZE[src_level] );
  }
  
  child.paddr       = met_paddr;
  child.mtree_level = src_level;
  child.entry_num   = src_entry_num;

  //  printf("Input Child addr: %llu, level: %d, entry_num: %llu\n",child.paddr,child.mtree_level,child.entry_num);
  parent = getMtreeEntry(child, mem_org);
  return parent;
  
}


ADDR getCtrChild( ADDR met_paddr, memOrg_t * mem_org, int *num_children ){
  ctr_mtree_entry parent;
  ADDR child_paddr = 0, child_level_startaddr = 0;
  int met_partition = get_partition(met_paddr, mem_org);
  //  print_backtrace();
  ASSERTM( (met_partition == 1) || (met_partition == 2), "ERROR: getMtreEvictParent has been called with paddr not lying within MET" );

  //Ensure met_paddr is aligned to cacheline
  met_paddr = met_paddr >> 6;
  met_paddr = met_paddr << 6;

  
  int src_level = -1;
  uns64 src_entry_num = -1;
  
  for(int i= mem_org -> num_Mtree_levels; i >= 0 ; i--){

    if(met_paddr >= mem_org -> mtree_levels_start_addr[i]){
      src_level = i;
      break;
    }

  }
  ASSERTM(src_level != -1, "ERROR: Unable to assert src_level in getMtreeEvictParent");

  if(src_level == mem_org-> num_Mtree_levels){
    src_entry_num = (met_paddr - mem_org -> mtree_levels_start_addr[src_level])/CTR_SIZE;
    ASSERTM( ((uns64)(src_entry_num*CTR_SIZE)) % CACHE_LINE_SIZE == 0, "src_entry_num should be first in cacheline\n");
  }
  else{
    src_entry_num = (met_paddr - mem_org -> mtree_levels_start_addr[src_level])/MTREE_ENTRY_SIZE[src_level];
      
    ASSERTM( ((uns64)(src_entry_num * MTREE_ENTRY_SIZE[src_level])) % CACHE_LINE_SIZE == 0, "src_entry_num should be first in cacheline\n");
  }

         
  if(src_level == mem_org-> num_Mtree_levels){
    child_level_startaddr = 0; //DATA start addr
    *num_children = (uns64)( CACHE_LINE_SIZE / CTR_SIZE);
  }
  else {
    child_level_startaddr  = mem_org -> mtree_levels_start_addr[src_level + 1];
    *num_children = (uns64) (CACHE_LINE_SIZE / MTREE_ENTRY_SIZE[src_level]);
  }

  child_paddr = child_level_startaddr + src_entry_num * CACHE_LINE_SIZE;
  return child_paddr;

}


// memOrg.cpp ends here
