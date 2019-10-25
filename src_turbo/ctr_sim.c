#include "ctr_sim.h"
#include "memory_controller.h"
#include "params.h"
#include "memsys_control_flow.h"
#include "compression.h"
#include "stats.h"

extern long long int CYCLE_VAL;

extern int CTR_DESIGN;
extern double CTR_SIZE;
extern int CTRS_PER_MTREE;

extern int* MTREE_CTR_DESIGN;
extern double* MTREE_ENTRY_SIZE;
extern int* MTREE_ARY;

extern int COUNTER_UPDATE_POLICY;
extern int COMP_CTR_MODE;
extern int COMPRESSED_MTREE;
extern int COMPRESSED_CTR;

extern cache_stats* MET_Cache_Stats;
extern MCache *METCache[MAX_THREADS];
extern MCache *METCache[MAX_THREADS];
extern unsigned long long int       MET_PRIVATE;

extern SGX_MODE[4];

extern int MAJOR_CTR_BITLEN; 
extern int MINOR_CTR_BITLEN ;

uns64* temp_minor_ctr_val;

/************************************************/
//Function to initialize the ctr_cachelines
/************************************************/
void init_ctr_types(memOrg_t* mem_org, ctr_type* ctr_types, int mtree_level){
  int curr_ctr_design = 0;

  //Initialize the ctr_types for the counters
  ctr_types -> mtree_level = mtree_level; //if equal to mem_org->num_mtree_levels, then indicates that it is the counter level  
  ASSERTM(mtree_level <= mem_org->num_Mtree_levels,"Number of Mtree level should be sane while init ctr_types");

  
  if(ctr_types -> mtree_level == mem_org->num_Mtree_levels){
    curr_ctr_design = CTR_DESIGN;
  }
  else {
    curr_ctr_design = MTREE_CTR_DESIGN[mtree_level];
  }
  ctr_types-> design = curr_ctr_design;

  //Default values
  ctr_types->log_num_overflows = 0;
  ctr_types->log_num_check_overflows = 0;
  
  if( (curr_ctr_design) == (MONO8_CTR) ){
    //Monolithic Counter - 8-byte
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_bitslen = 64;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT16_CTR) ){
    //Split counters - 16 per CL (2 x 64-bit Major, 16 x 16-bit Minor)
    ctr_types->num_major_ctr_per_cl = 2;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 16 ) - 1 ;
    ctr_types->minor_ctr_bitslen = 16;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT32_CTR) ){
    //Split counters - 32 per CL (2 x 48-bit Major, 32 x 9-bit Minor)
    ctr_types->num_major_ctr_per_cl = 2;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 48 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 9  ) - 1 ;
    ctr_types->minor_ctr_bitslen = 9;
    ctr_types->major_ctr_bitslen = 48 ;
  }
  else if( (curr_ctr_design) == (SPLIT64_CTR) ){
    //Split counters - 64 per CL (1 x 64-bit Major, 64 x 6-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 6  ) - 1 ;
    ctr_types->minor_ctr_bitslen = 6;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT128_CTR) ){
    //Split counters - 128 per CL (1 x 64-bit Major, 128 x 3-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 3  ) - 1 ;
    ctr_types->minor_ctr_bitslen = 3;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( ((curr_ctr_design) == (SPLIT128_FULL_DUAL)) || ((curr_ctr_design) == (SPLIT128_UNC_DUAL)) ){
    //Split counters - 128 per CL (1 x 64-bit Major, 128 x 3-bit Minor)
    ctr_types->num_major_ctr_per_cl = 2;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 10 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 3  ) - 1 ;

    ctr_types->major_ctr_bitslen = 10;
    ctr_types->minor_ctr_bitslen = 3;

    ctr_types->super_ctr_maxval = ( ((uns64)1) << 53  ) - 1 ;
    ctr_types->super_ctr_bitslen = 53;

    ASSERTM( (COMPRESSED_MTREE == 8) || (COMPRESSED_MTREE == 7), "Split128_CTR_ENC only works with COMPRESSED_MTREE == 8");
  }
  else if((curr_ctr_design) == (SPLIT128_UNC_7bINT_DUAL) ){
    //Split counters - 128 per CL (1 x 49-bit Major, 2 x 7-bit Medium Counter, 2 x 64 x 3-bit Minor)
    ctr_types->num_major_ctr_per_cl = 2;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 7 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 3  ) - 1 ;

    ctr_types->major_ctr_bitslen = 7;
    ctr_types->minor_ctr_bitslen = 3;

    ctr_types->super_ctr_maxval = ( ((uns64)1) << 49  ) - 1 ;
    ctr_types->super_ctr_bitslen = 49;

    ASSERTM( (COMPRESSED_MTREE == 8) || (COMPRESSED_MTREE == 7), "Split128_CTR_ENC only works with COMPRESSED_MTREE == 8");
  }

  else if( (curr_ctr_design) == (SPLIT32_CTR_v1) ){
    //Split counters - 32 per CL (1 x 64-bit Major, 32 x 12-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 12  ) - 1 ;
    ctr_types->minor_ctr_bitslen = 12;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT16_CTR_v1) ){
    //Split counters - 16 per CL (1 x 64-bit Major, 16 x 24-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 24  ) - 1 ;
    ctr_types->minor_ctr_bitslen = 24;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT64_CTR_v1) ){
    //Split counters - 64 per CL (1 x 64-bit Major, 64 x 7-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << 7  ) - 1 ;
    ctr_types->minor_ctr_bitslen = 7;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT128_CTR_v1) ){
    //Split counters - 128 per CL (1 x 64-bit Major, 128 x 3.5-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = 11;
    ctr_types->minor_ctr_bitslen = 3.5;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
    
  }
  else if( (curr_ctr_design) == (SPLIT256_CTR) ){
    //Split counters - 128 per CL (1 x 64-bit Major, 256 x 2-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = 2;
    ctr_types->minor_ctr_bitslen = 1.75;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }
  else if( (curr_ctr_design) == (SPLIT512_CTR) ){
    //Split counters - 128 per CL (1 x 64-bit Major, 512 x 1-bit Minor)
    ctr_types->num_major_ctr_per_cl = 1;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << 64 ) - 1 ;
    ctr_types->minor_ctr_maxval = 1;
    ctr_types->minor_ctr_bitslen = 0.875;
    ctr_types->major_ctr_bitslen = MAJOR_CTR_BITLEN;
  }

  else{
    ASSERTM(0,"Counter Design not supported \n");
  }

  //IF COMPRESSED_MTREE - as specified in main.c
  if((COMPRESSED_MTREE == 1) && ( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) ) ){ //Compression of entire Mtree
    
    if(ctr_types -> mtree_level != mem_org->num_Mtree_levels){
      ctr_types-> compression_enabled = 1;           
      /*
        if( ((curr_ctr_design) == (SPLIT128_CTR_v1)) || ((curr_ctr_design) == (SPLIT128_CTR)) ){
        //Split counters - 128 per CL (1 x 64-bit Major, 128 x 3.5-bit Minor)
        MINOR_CTR_BITLEN = 7;   
        }
        else if( (curr_ctr_design) == (SPLIT256_CTR) ){
        MINOR_CTR_BITLEN = 7; 
        }
        else if( (curr_ctr_design) == (SPLIT512_CTR) ){
        MINOR_CTR_BITLEN = 7; 
        }
        else{
        ASSERTM(0,"Only 128,256,512 arity supported with Compression for Overflows and Compressed_Mtree");
        }    
      */
      ctr_types->minor_ctr_bitslen = 7;
      ctr_types->major_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
      ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
    }    
  }
  else if((COMPRESSED_MTREE == 2) && ( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) ) ){ //Compression only for Leaf of Mtree
    
    if(ctr_types -> mtree_level == (mem_org->num_Mtree_levels-1)){
      ctr_types-> compression_enabled = 1;           
      ctr_types->minor_ctr_bitslen = 7;
      ctr_types->major_ctr_maxval =  ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
      ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
                                     }    
  }  
  else if((COMPRESSED_MTREE == 3) && ( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) ) ){ //Compression for Mtree: Leaf = 7bits, Non-Leaf = 5bits
    
    if(ctr_types -> mtree_level == (mem_org->num_Mtree_levels-1)){
      ctr_types-> compression_enabled = 1;           
      ctr_types->minor_ctr_bitslen = 7;
      ctr_types->major_ctr_maxval =  ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
      ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
                                     }
    else if(ctr_types -> mtree_level < (mem_org->num_Mtree_levels-1)){
      ctr_types-> compression_enabled = 1;           
      ctr_types->minor_ctr_bitslen = 5;
      ctr_types->major_ctr_maxval =  ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
      ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
    }    
  }  

  else if((COMPRESSED_MTREE == 4) && ( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) ) ){ //Compression of Counters and Mtree
   
    ctr_types-> compression_enabled = 1;           
    ctr_types->minor_ctr_bitslen = 7;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
  }  

  else if((COMPRESSED_MTREE == 5) && ( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) ) ){ //Compression of Counters and Mtree
   
    ctr_types-> compression_enabled = 1;           
    ctr_types->minor_ctr_bitslen = 14;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
    ctr_types->num_minor_ctr_bitslen_arr = 3;
    ctr_types->minor_ctr_bitslen_arr = (double*) calloc(ctr_types->num_minor_ctr_bitslen_arr,sizeof(double));
    ctr_types->minor_ctr_bitslen_arr[0] = 11;
    ctr_types->minor_ctr_bitslen_arr[1] = 7;
    ctr_types->minor_ctr_bitslen_arr[2] = 3;  
  }
  
  else if( ((COMPRESSED_MTREE == 6) || (COMPRESSED_MTREE == 7)  || (COMPRESSED_MTREE == 8) ) && ( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) ) ){ //Compression of Counters and Mtree with 3 modes
    //COMPRESSED_MTREE == 7 -> AZC when mode = 0, Uncompressed 3-bit when mode = 2, (mode =1 skipped), 8 when FlexMajorCounter is used for all counters, uniformly.
    ctr_types-> compression_enabled = 1;           
    ctr_types->minor_ctr_bitslen = 14;
    ctr_types->major_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->major_ctr_bitslen) ) - 1 ;   
    ctr_types->minor_ctr_maxval = ( ((uns64)1) << ((int)ctr_types->minor_ctr_bitslen) ) - 1 ;
    ctr_types->num_minor_ctr_bitslen_arr = 3;
    ctr_types->minor_ctr_bitslen_arr = (double*) calloc(ctr_types->num_minor_ctr_bitslen_arr,sizeof(double));
    ctr_types->minor_ctr_bitslen_arr[0] = 11;
    ctr_types->minor_ctr_bitslen_arr[1] = 7;
    ctr_types->minor_ctr_bitslen_arr[2] = 3;
  }  

  ASSERTM(COMPRESSED_CTR == 0, "COMPRESSION FOR CTRS NOT DESIGNED FOR YET");
  /*
  //IF COMPRESSED_CTR
  if(COMPRESSED_CTR == 1){
  if(ctr_types -> mtree_level == mem_org->num_Mtree_levels){
  ctr_types-> compression_enabled = 1;
  if( (COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3)  ){ //Compression for Overflows
  ctr_types->major_ctr_maxval = ( ((uns64)1) << (MAJOR_CTR_BITLEN) ) - 1 ;   
  ctr_types->minor_ctr_maxval = ( ((uns64)1) << (MINOR_CTR_BITLEN) ) - 1 ;
  }    
  }    
  }
  */
  
  if(ctr_types -> mtree_level == mem_org->num_Mtree_levels){
    ctr_types->num_minor_ctr_per_major_ctr = (int)( ((double)CACHE_LINE /(double) (CTR_SIZE)) / ((double)ctr_types->num_major_ctr_per_cl) ) ;
    ctr_types->num_ctr_cls = mem_org-> ctr_store_size / (CACHE_LINE);
    ASSERTM( ((uns64) (ctr_types->num_minor_ctr_per_major_ctr * ctr_types->num_major_ctr_per_cl * CTR_SIZE ))  == (CACHE_LINE), "ERROR: The sizing of counter design does not add up\n" ) ;
  }
  else {
    ctr_types->num_minor_ctr_per_major_ctr = (int)( ((double)CACHE_LINE  / (double)(MTREE_ENTRY_SIZE[ctr_types->mtree_level]) ) / ((double)ctr_types->num_major_ctr_per_cl) ) ;   
    ctr_types->num_ctr_cls = mem_org-> mtree_level_size[ctr_types -> mtree_level] / (CACHE_LINE); 
    ASSERTM( ((uns64) (ctr_types->num_minor_ctr_per_major_ctr * ctr_types->num_major_ctr_per_cl * MTREE_ENTRY_SIZE[ctr_types->mtree_level])) == (CACHE_LINE), "ERROR: The sizing of mtree counter design does not add up\n" ) ;
  }


  temp_minor_ctr_val = (uns64*)calloc(ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr, sizeof(uns64) );

#ifdef HIST_NZ
  //Initialize the bins for tracking nz ctrs on overflows
  ctr_types->nz_ctr_buckets = (uns64*)calloc(ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr + 1, sizeof(uns64));
#endif
  
}

void init_ctr_cls(memOrg_t* mem_org, ctr_cl* ctr_cls, ctr_type* ctr_types, int mtree_level){

  ASSERTM(mtree_level == ctr_types -> mtree_level ,"Number of Mtree level should be sane while init ctr_types");
  ASSERTM(ctr_types -> mtree_level <= mem_org->num_Mtree_levels,"Number of Mtree level should be same as init ctr_types");
  ASSERTM( (ctr_types->num_major_ctr_per_cl == 1) || (COMPRESSED_MTREE == 7) ||  (COMPRESSED_MTREE == 8),"Not designed for multiple major ctr in cacheline\n");

  
  //Initialize the ctr_cls based on ctr_types
  for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
    
    //identifier
    ctr_cls[ctr_cl_num].lineaddr = ( ( mem_org -> mtree_levels_start_addr[ctr_types -> mtree_level] ) >> log_base2(CACHE_LINE) ) + ctr_cl_num ;
    //    printf("For ctr_id : %llu, Ctr_cls lineaddr is : %llx",ctr_cl_num,ctr_cls[ctr_cl_num].lineaddr ); 
    ctr_cls[ctr_cl_num].mtree_level = ctr_types -> mtree_level;
    ctr_cls[ctr_cl_num].entry_num   = ctr_cl_num;
    
    //state
    ctr_cls[ctr_cl_num].major_ctr_val = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));
    ctr_cls[ctr_cl_num].minor_ctr_val = (uns64**)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64*));
    for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){
      ctr_cls[ctr_cl_num].minor_ctr_val[major_ctr_num] = (uns64*)calloc(ctr_types->num_minor_ctr_per_major_ctr,sizeof(uns64));
    }
    
    //overflow stats
    ctr_cls[ctr_cl_num].num_overflows = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));
    ctr_cls[ctr_cl_num].last_overflow_cycle = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));    

    //ctr read write stats
    ctr_cls[ctr_cl_num].num_reads = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));
    ctr_cls[ctr_cl_num].num_writes = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));

    //data read write stats
    ctr_cls[ctr_cl_num].data_reads = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));
    ctr_cls[ctr_cl_num].data_writes = (uns64*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(uns64));

    ctr_cls[ctr_cl_num].increments = 0;
    if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3) ){ //Modes where compression used for cache performance
      ctr_cls[ctr_cl_num].bitlen = MAJOR_CTR_BITLEN + best(ctr_cls[ctr_cl_num].minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr);
    }
    else{
      ctr_cls[ctr_cl_num].bitlen = CACHE_LINE_SIZE * 8;
    }

    //Comp_mode
    ctr_cls[ctr_cl_num].comp_mode = (int*)calloc(ctr_types->num_major_ctr_per_cl,sizeof(int));

    //Other initialization values
    ctr_cls[ctr_cl_num].super_ctr_val = 0;
    ctr_cls[ctr_cl_num].super_ctr_val = 0;
    
    ctr_cls[ctr_cl_num].num_nonzero_numer           = 0 ; 
    ctr_cls[ctr_cl_num].num_nonzero_denom           = 0 ;
    ctr_cls[ctr_cl_num].num_smalldynrange_numer     = 0 ;  
    ctr_cls[ctr_cl_num].num_smalldynrange_denom     = 0 ; 
    ctr_cls[ctr_cl_num].num_smalldynrange_numer_no_zero  = 0  ;
    ctr_cls[ctr_cl_num].num_smalldynrange_denom_no_zero  = 0  ;

    
  }

}

/************************************************/
//Function to update the ctr_cachelines on dirty write to counter
/************************************************/
void update_ctr_cls_wrapper(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, ctr_mtree_entry ctr, int ctr_mtree_level, overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst){
  
  if(ctr_mtree_level != 0){
    int mtree_level = getMtreeEvictParent(ctr.paddr,mem_org).mtree_level + 1;
    ASSERTM(mtree_level == ctr_mtree_level,"Input ctr_mtree_level not matching with calculated mtree_level from counter\n");
  }
 
  update_ctr_cls(mem_org, ctr_cls[ctr_mtree_level],ctr_types[ctr_mtree_level], ctr, ctr_overflows_levelwise,  ctr_overflows_levelwise_inst);
}

void update_ctr_cls(memOrg_t* mem_org, ctr_cl* ctr_cls, ctr_type* ctr_types, ctr_mtree_entry ctr, overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst){
  int max_minor_ctr_id = 0;
  uns64 max_minor_ctr_val = 0;
  
  //Assert that ctr_paddr falls in the mtree_partition that is mentioned
  if(ctr_types->mtree_level != 0){
    ASSERTM((getMtreeEvictParent(ctr.paddr,mem_org).mtree_level + 1) == ctr_types->mtree_level,"Requested ctr_paddr does not refer to selected mtree_level\n");
  }
  uns64 ctr_byte_in_line, ctr_id_in_line, major_ctr_id, minor_ctr_id;
  //Check which ctr_cls to access based on ctr_types entry
  uns64 ctr_id = ( ctr.paddr - mem_org -> mtree_levels_start_addr[ctr_types -> mtree_level] ) >> log_base2(CACHE_LINE);
  //printf("Ctr_paddr: %llu, Mtree_Level_Start: %llu\n",ctr.paddr,  mem_org -> mtree_levels_start_addr[ctr_types -> mtree_level]);
  
  //Ensure everything good (ASSERTMs)
  ASSERTM(ctr_id < ctr_types->num_ctr_cls, "The indexed cacheline is more than the number of counter cachelines provisioned in ctr_sim\n");
  //printf("For ctr_id : %llu, Ctr_cls lineaddr is : %llx, ctr_paddr shifted is %llx\n",ctr_id, ctr_cls[ctr_id].lineaddr, (ctr_paddr  >> log_base2(CACHE_LINE)));
  ASSERTM(ctr_cls[ctr_id].lineaddr == (ctr.paddr  >> log_base2(CACHE_LINE) ) , "Counter accessed, does not have a matching lineaddr\n" );
  ASSERTM(ctr_cls[ctr_id].mtree_level == ctr_types-> mtree_level , "Right level of counters not accessed\n"); 
  ASSERTM(ctr_cls[ctr_id].entry_num == ctr_id , "Counter accessed, does not have a matching lineaddr\n" );

  //Increment the right ctr_cls
  /*
    ctr_byte_in_line = ctr_paddr - (ctr_cls[ctr_id].lineaddr << log_base2(CACHE_LINE));

    if(ctr_types->mtree_level == mem_org->num_Mtree_levels){ //Counter
    ctr_id_in_line = ctr_byte_in_line / CTR_SIZE; //index of the counter within a cacheline ** THIS WILL CAUSE A BUG IF CTR_SIZE IS FRACTIONAL **
    }
    else{ //Mtree
    ctr_id_in_line = ctr_byte_in_line / MTREE_ENTRY_SIZE; //index of the counter within a cacheline  ** THIS WILL CAUSE A BUG IF CTR_SIZE IS FRACTIONAL **
    }
  */
  if(ctr_types->mtree_level == mem_org->num_Mtree_levels){ //Counter
    ctr_id_in_line = ctr.entry_num - ctr_id * (uns64)(CACHE_LINE /CTR_SIZE);
    if( ctr_id_in_line > (uns64)(CACHE_LINE/ CTR_SIZE) )
      print_backtrace();
  }
  else{ //Mtree
    ctr_id_in_line = ctr.entry_num - ctr_id * (uns64)(CACHE_LINE /MTREE_ENTRY_SIZE[ctr_types->mtree_level]);
    if( ctr_id_in_line > (uns64)(CACHE_LINE/ MTREE_ENTRY_SIZE[ctr_types->mtree_level]) )
      print_backtrace();
  
  }
 
  //printf("Ctr_id_in_line : %llu, Entry num: %llu, Ctr_id: %llu\n",ctr_id_in_line,ctr.entry_num,ctr_id);
  
  major_ctr_id = ctr_id_in_line / ctr_types->num_minor_ctr_per_major_ctr ;
  minor_ctr_id = ctr_id_in_line % ctr_types->num_minor_ctr_per_major_ctr ;

  //Track increment to the counter cacheline:
  ctr_cls[ctr_id].increments++;  
  
  //Find the max minor counter, value and iD
  if(COUNTER_UPDATE_POLICY == 1){
    for(int i=0; i < ctr_types -> num_minor_ctr_per_major_ctr ; i++){
      if(ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][i] > max_minor_ctr_val ){ 
        max_minor_ctr_val = ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][i];
        max_minor_ctr_id  = i;
      }
    }

    if(max_minor_ctr_val == ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id])
      ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id]++;
    else{
      uns64 maxdiff = max_minor_ctr_val - ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id];
      uns64 incr = 16;
      if(maxdiff < incr)
        incr = maxdiff ;
      ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id] += incr;
    }
    
  }
  else if (COUNTER_UPDATE_POLICY == 0){
    ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id]++;
  }
  else{
    ASSERTM(0,"COUNTER_UPDATE_POLICY CAN BE ONLY 0 or 1");
  }

  //Check if there's a overflow, if yes update the ctr_cl overflow stats
  if(check_overflow(&ctr_cls[ctr_id],major_ctr_id, minor_ctr_id, ctr_types)){
    //if( ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id] >= ctr_types->minor_ctr_maxval ){
    /*
      if(ctr_types->mtree_level == (mem_org->num_Mtree_levels-2)){
      ctr_cl* curr_ctr_cl = &ctr_cls[ctr_id];
      //printf("Overflowed Level %d Counter %d:, Best %d, zero_val %d, zero_runl %d, ptrs_nz %d, bpc %d, bdi %d :",ctr_types->mtree_level,ctr_id,best_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen, MAJOR_CTR_BITLEN+MAC_BITLEN),zero_val_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen),zero_runlength_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen),ptrs_non_zero_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen),bpc_comp_new(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen), bdi_comp_new(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen, MAJOR_CTR_BITLEN+MAC_BITLEN ));
      printf("overflowing counter: ");
      
      for(int i=0; i < ctr_types -> num_minor_ctr_per_major_ctr ; i++){
      printf("%llu ,",ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][i]);
      }
      printf("\n");
      } 
    */
    
    //Log statistics of which compresison scheme saved you:
    ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][minor_ctr_id]--;
    log_comp_schemes(ctr_types,&ctr_cls[ctr_id], major_ctr_id);

    //Track zero value counters in minor counter.
    track_zero_ctr_stats(ctr_types,&(ctr_cls[ctr_id]));
    //Track dynrange in minor counters
    track_dynrange_ctr_stats(ctr_types,&(ctr_cls[ctr_id]), major_ctr_id);
    
    //Increment major counter
    if(ctr_types->compression_enabled){
      uns64 max_minor_ctrval = 0;
    
      for(int x=0;x<ctr_types->num_minor_ctr_per_major_ctr; x++){
        if(max_minor_ctrval < ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][x])
          max_minor_ctrval = ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][x]; 
      }
      //Increment by max counter val.    
      ctr_cls[ctr_id].major_ctr_val[major_ctr_id] += (max_minor_ctrval + 1);
    } else {
      //Increment by 1
      ctr_cls[ctr_id].major_ctr_val[major_ctr_id] ++;
    }
       
    ctr_cls[ctr_id].num_overflows[major_ctr_id]++;
    ctr_cls[ctr_id].last_overflow_cycle[major_ctr_id] = CYCLE_VAL;

    if(ctr_types -> major_ctr_bitslen == MAJOR_CTR_BITLEN){
      ASSERTM((ctr_cls[ctr_id].major_ctr_val[major_ctr_id] <= ctr_types->major_ctr_maxval), "ERROR: Major counter cannot overflow \n");
    }
    
    //Reset minor counters
    for(int i=0; i < ctr_types -> num_minor_ctr_per_major_ctr ; i++){
      ctr_cls[ctr_id].minor_ctr_val[major_ctr_id][i] = 0;
    }
    
    //Reset value of bitlen on overflow
    if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3) ){ //Modes where compression used for cache per
      ctr_cls[ctr_id].bitlen =  MAJOR_CTR_BITLEN + best(ctr_cls[ctr_id].minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr);
    }

    //Reset choice of compression scheme
    ctr_cls[ctr_id].comp_mode[major_ctr_id] = 0;

    insert_overflow_mem_accesses(ctr.paddr, mem_org,&ctr_overflows_levelwise[ctr_types->mtree_level],&ctr_overflows_levelwise_inst[ctr_types->mtree_level],major_ctr_id,ctr_types->num_major_ctr_per_cl);


    //Update Levelwise overflow Statistics
    ctr_overflows_levelwise[ctr_types->mtree_level].overflows++;
    ctr_overflows_levelwise_inst[ctr_types->mtree_level].overflows++;    

    //Check if major counter overflows - Only for split128_ctr_enc
    if(ctr_cls[ctr_id].major_ctr_val[major_ctr_id] > ctr_types->major_ctr_maxval){
      ASSERTM(( (CTR_DESIGN == SPLIT128_FULL_DUAL) ||  (CTR_DESIGN == SPLIT128_UNC_DUAL)  ||  (CTR_DESIGN == SPLIT128_UNC_7bINT_DUAL)) && (ctr_types->mtree_level == mem_org->num_Mtree_levels), "Major counter overflows only if dual split counter design used with SPLIT128_FULL_DUAL");

      //Reset all major counters in the counter cacheline
      for(int i=0;i<ctr_types->num_major_ctr_per_cl; i++){
        ctr_cls[ctr_id].major_ctr_val[i] =0;          
      }        
        
      // Increment super-major counter by 2.
      ctr_cls[ctr_id].super_ctr_val += 2;

      //Update everything for other sets of counters in ctr cacheline
      for(int i=0;i<ctr_types->num_major_ctr_per_cl; i++){
        if(i == major_ctr_id){
          continue;
        }
        //Update Stats
        ctr_cls[ctr_id].num_overflows[i]++;
        ctr_cls[ctr_id].last_overflow_cycle[i] = CYCLE_VAL;

        //Track dynrange in minor counters
        track_dynrange_ctr_stats(ctr_types,&(ctr_cls[ctr_id]), major_ctr_id);

        //Reset minor counters
        for(int j=0; j < ctr_types -> num_minor_ctr_per_major_ctr ; j++){
          ctr_cls[ctr_id].minor_ctr_val[i][j] = 0;
        }        
        //Reset other comp_mode
        ctr_cls[ctr_id].comp_mode[i] = 0;

        //Insert mem-accesses for data cachelines
        insert_overflow_mem_accesses(ctr.paddr, mem_org,&ctr_overflows_levelwise[ctr_types->mtree_level],&ctr_overflows_levelwise_inst[ctr_types->mtree_level],i,ctr_types->num_major_ctr_per_cl);
      }        
             
      //Update Levelwise overflow Statistics
      ctr_overflows_levelwise[ctr_types->mtree_level].overflows++;
      ctr_overflows_levelwise_inst[ctr_types->mtree_level].overflows++;

    }
  }
}


/************************************************/
//Function to update the ctr_cachelines on memory_access
/************************************************/
void update_ctr_cls_rw_wrapper(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, ADDR ctr_paddr, int ctr_mtree_level, int r_w){
  
  if(ctr_mtree_level != 0){
    int mtree_level = getMtreeEvictParent(ctr_paddr,mem_org).mtree_level + 1;
    ASSERTM(mtree_level == ctr_mtree_level,"Input ctr_mtree_level not matching with calculated mtree_level from counter\n");
  }
 
  update_ctr_cls_rw(mem_org, ctr_cls[ctr_mtree_level],ctr_types[ctr_mtree_level], ctr_paddr, r_w);
}

void update_ctr_cls_rw(memOrg_t* mem_org, ctr_cl* ctr_cls, ctr_type* ctr_types, ADDR ctr_paddr, int  r_w){

  //Currently designed for encryption counters.
  ASSERTM(ctr_types->mtree_level ==  mem_org->num_Mtree_levels, "update_ctr_cls_rw designed for encryption counter level only currently \n");
  
  //Assert that ctr_paddr falls in the mtree_partition that is mentioned
  if(ctr_types->mtree_level != 0){
    ASSERTM((getMtreeEvictParent(ctr_paddr,mem_org).mtree_level + 1) == ctr_types->mtree_level,"Requested ctr_paddr does not refer to selected mtree_level\n");
  }
  
  ASSERTM(ctr_types-> mtree_level == mem_org->num_Mtree_levels, "Currently only support encryption counter level for r_w stats in ctr_cls");
  uns64 ctr_byte_in_line, ctr_id_in_line, major_ctr_id, minor_ctr_id;
  //Check which ctr_cls to access based on ctr_types entry
  uns64 ctr_id = ( ctr_paddr - mem_org -> mtree_levels_start_addr[ctr_types -> mtree_level] ) >> log_base2(CACHE_LINE);
   

  //Ensure everything good (ASSERTMs)
  ASSERTM(ctr_id < ctr_types->num_ctr_cls, "The indexed cacheline is more than the number of counter cachelines provisioned in ctr_sim\n");
  //printf("For ctr_id : %llu, Ctr_cls lineaddr is : %llx, ctr_paddr shifted is %llx\n",ctr_id, ctr_cls[ctr_id].lineaddr, (ctr_paddr  >> log_base2(CACHE_LINE)));
  ASSERTM(ctr_cls[ctr_id].lineaddr ==  (ctr_paddr  >> log_base2(CACHE_LINE) ) , "Counter accessed, does not have a matching lineaddr\n" );
  ASSERTM(ctr_cls[ctr_id].mtree_level == ctr_types-> mtree_level , "Right level of counters not accessed\n"); 
  ASSERTM(ctr_cls[ctr_id].entry_num == ctr_id , "Counter accessed, does not have a matching lineaddr\n" );

  //Increment the right ctr_cls
  ctr_byte_in_line = ctr_paddr - (ctr_cls[ctr_id].lineaddr << log_base2(CACHE_LINE));

  if(ctr_types->mtree_level == mem_org->num_Mtree_levels){ //Counter
    ctr_id_in_line = ctr_byte_in_line / CTR_SIZE; //index of the counter within a cacheline ** THIS WILL CAUSE A BUG IF CTR_SIZE IS FRACTIONAL **
  }
  else{ //Mtree
    ctr_id_in_line = ctr_byte_in_line / MTREE_ENTRY_SIZE[ctr_types->mtree_level]; //index of the counter within a cacheline ** THIS WILL CAUSE A BUG IF CTR_SIZE IS FRACTIONAL **
  }

  /*
    if(ctr_types->mtree_level == mem_org->num_Mtree_levels){ //Counter
    ctr_id_in_line = ctr.entry_num - ctr_id * (uns64)((double)CACHE_LINE /(double)CTR_SIZE);
    }
    else{ //Mtree
    ctr_id_in_line = ctr.entry_num - ctr_id * (uns64)((double)CACHE_LINE /(double)MTREE_ENTRY_SIZE);
    }
  */
  major_ctr_id = ctr_id_in_line / ctr_types->num_minor_ctr_per_major_ctr ;

  //minor_ctr_id = ctr_id_in_line % ctr_types->num_minor_ctr_per_major_ctr ; //BUGGY - To use this, need a better ctr_id_in_line derived from ctr_mtree_entry instead of ctr_paddr

  if(r_w == 0){
    ctr_cls[ctr_id].num_reads[major_ctr_id]++;
  }
  else if(r_w == 1){
    ctr_cls[ctr_id].num_writes[major_ctr_id]++;
  }
  else if(r_w == 2){
    ctr_cls[ctr_id].data_reads[major_ctr_id]++;
  }
  else if(r_w == 3){
    ctr_cls[ctr_id].data_writes[major_ctr_id]++;
  }
  else{
    ASSERTM(0,"r_w value not according to definition");
  }
  
}

/************************************************/
//Function to analyze overflows and print
/************************************************/

void print_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr, unsigned long long int count_i_total){

  uns64 mtree_num_overflows = 0;
  uns64 mtree_total_count = 0;
  uns64 mtree_increments = 0;

  uns64 total_overflows = 0;
  uns64 total_increments = 0;
  
  
  for(int mtree_level= mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    
    ctr_type* ctr_types = ctr_types_arr[mtree_level];
    ctr_cl* ctr_cls = ctr_cls_arr[mtree_level];
    
    uns64 num_overflows = 0;
    uns64 total_count = 0;
    uns64 num_reads = 0;
    uns64 num_writes = 0;
    uns64 data_reads = 0;
    uns64 data_writes = 0;
    uns64 ctr_increments = 0;

    for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
      for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){
	
        num_overflows += ctr_cls[ctr_cl_num].num_overflows[maj_ctr];
        num_reads += ctr_cls[ctr_cl_num].num_reads[maj_ctr];
        num_writes += ctr_cls[ctr_cl_num].num_writes[maj_ctr];
        data_reads += ctr_cls[ctr_cl_num].data_reads[maj_ctr];
        data_writes += ctr_cls[ctr_cl_num].data_writes[maj_ctr];
        
      }
      ctr_increments += ctr_cls[ctr_cl_num].increments;
    }
    

    for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
      
      for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){
        for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
          total_count+= 	ctr_cls[ctr_cl_num].minor_ctr_val[major_ctr_num][minor_ctr_num];
        }
        total_count+= ctr_cls[ctr_cl_num].major_ctr_val[major_ctr_num]*ctr_types->minor_ctr_maxval;
      }
    }
    
    if( mtree_level == mem_org-> num_Mtree_levels){
      printf ("NUM_READS_DATA            \t: %llu\n", data_reads );
      printf ("NUM_WRITES_DATA            \t: %llu\n", data_writes); 
      printf ("NUM_READS_CTR            \t: %llu\n", num_reads );
      printf ("NUM_WRITES_CTR            \t: %llu\n", num_writes); 
      //printf ("TOT_COUNTVAL_CTR            \t: %llu\n", total_count ); 
      
      //printf ("NUM_OVERFLOW_PMI_CTR       \t: %9.2f\n", 1000000*((double)num_overflows) / ((double) count_i_total) ); 
      printf ("\n");
      printf ("NUM_CTR_INCREMENTS_CTR           \t: %llu\n", ctr_increments );
      printf ("NUM_OVERFLOWS_CTR           \t: %llu\n", num_overflows );
      printf ("WRITE_TOLERANCE_CTR           \t: %f\n", (double)ctr_increments/(double)num_overflows );
           
      printf ("\n");

      mtree_num_overflows = 0;
      mtree_total_count = 0;
      mtree_increments = 0;

    }
    else {
      //printf ("TOT_COUNTVAL_CTR_MTREE_%d            \t: %llu\n", mtree_level, total_count ); 
      //printf ("NUM_OVERFLOW_PMI_MTREE_%d       \t: %9.2f\n", mtree_level, 1000000*((double)num_overflows) / ((double) count_i_total) ); 
      printf ("NUM_CTR_INCREMENTS_MTREE_%d      \t: %llu\n", mtree_level ,ctr_increments );
      printf ("NUM_OVERFLOWS_MTREE_%d           \t: %llu\n", mtree_level ,num_overflows); 
      printf ("WRITE_TOLERANCE_MTREE_%d           \t: %f\n",  mtree_level ,(double)ctr_increments/(double)num_overflows );

      printf ("\n");
      mtree_num_overflows += num_overflows;
      mtree_total_count += total_count;
      mtree_increments += ctr_increments;
    }
    
    total_increments += ctr_increments;
    total_overflows += num_overflows;
  }
  //printf ("TOT_COUNTVAL_MTREE            \t: %llu\n", mtree_total_count ); 
  //printf ("NUM_OVERFLOW_PMI_MTREE       \t: %9.2f\n", 1000000*((double)mtree_num_overflows) / ((double) count_i_total) ); 
  printf ("NUM_CTR_INCREMENTS_MTREE      \t: %llu\n", mtree_increments );
  printf ("NUM_OVERFLOWS_MTREE           \t: %llu\n", mtree_num_overflows );
  printf ("WRITE_TOLERANCE_MTREE          \t: %f\n", (double)mtree_increments/(double)mtree_num_overflows );
  
  printf ("\n");

  printf("**************\n");
  printf ("NUM_CTR_INCREMENTS_TOTAL      \t: %llu\n", total_increments );
  printf ("NUM_OVERFLOWS_TOTAL           \t: %llu\n", total_overflows);
  printf ("WRITE_TOLERANCE_TOTAL         \t: %f\n", (double)total_increments/(double)total_overflows );
  
  printf ("\n");

  printf ("\n");

  //Printing Largest Major counter
  printf("*** MAJOR_CTR_GROWTH STATS***\n");
  uns64 largest_major_ctr_overall = 0;
  uns64 largest_major_ctr_overall_sup = 0;
  uns64 largest_major_ctr_overall_maj = 0;
  
  for(int mtree_level= mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    uns64 largest_major_ctr_levelwise = 0;    
    ctr_type* ctr_types = ctr_types_arr[mtree_level];
    ctr_cl* ctr_cls = ctr_cls_arr[mtree_level];

    if(ctr_types->num_major_ctr_per_cl == 1){ //Non-Dual counters
      for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
        if(largest_major_ctr_levelwise < ctr_cls[ctr_cl_num].major_ctr_val[0])
          largest_major_ctr_levelwise = ctr_cls[ctr_cl_num].major_ctr_val[0];               
      }
    } else{
      for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){    
        for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){
          uns64 effective_majctr = (ctr_cls[ctr_cl_num].super_ctr_val << (int)ctr_types->major_ctr_bitslen) + ctr_cls[ctr_cl_num].major_ctr_val[major_ctr_num];
          if(largest_major_ctr_levelwise < effective_majctr)
            largest_major_ctr_levelwise = effective_majctr;
        }
      }
    }

    printf("MAX_MAJ_CTR_FINLEVEL_%d         \t: %llu\n",mem_org->num_Mtree_levels - mtree_level, largest_major_ctr_levelwise);
    if(largest_major_ctr_overall < largest_major_ctr_levelwise)
      largest_major_ctr_overall = largest_major_ctr_levelwise;    
  }
  printf("MAX_MAJ_CTR_OVERALL         \t: %llu\n\n", largest_major_ctr_overall);

  //Printing Nonzero & DynRange Stats Levelwise
  printf("*** NONZERO_DYNRANGE STATS***\n");
  uns64 tot_nonzero_ctr_num =0,tot_nonzero_ctr_den =0,tot_dynrange_ctr_num =0,tot_dynrange_ctr_den =0,tot_dynrange_ctr_num_no_zero =0,tot_dynrange_ctr_den_no_zero =0;
  double tot_nonzero_ctr_perc = 0, tot_dynrange_ctr_perc = 0, tot_dynrange_ctr_perc_no_zero = 0 ;
  
  for(int mtree_level= mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    uns64 nonzero_ctr_num =0,nonzero_ctr_den =0,dynrange_ctr_num =0,dynrange_ctr_den =0,dynrange_ctr_num_no_zero =0,dynrange_ctr_den_no_zero =0;
    ctr_type* ctr_types = ctr_types_arr[mtree_level];
    ctr_cl* ctr_cls = ctr_cls_arr[mtree_level];

    for(uns64 ctr_cl_num = 0; ctr_cl_num < ctr_types->num_ctr_cls; ctr_cl_num++){
      nonzero_ctr_num += ctr_cls[ctr_cl_num].num_nonzero_numer;
      nonzero_ctr_den += ctr_cls[ctr_cl_num].num_nonzero_denom;
      dynrange_ctr_num += ctr_cls[ctr_cl_num].num_smalldynrange_numer;
      dynrange_ctr_den += ctr_cls[ctr_cl_num].num_smalldynrange_denom;
      dynrange_ctr_num_no_zero += ctr_cls[ctr_cl_num].num_smalldynrange_numer_no_zero;
      dynrange_ctr_den_no_zero += ctr_cls[ctr_cl_num].num_smalldynrange_denom_no_zero;
    }
    
    tot_nonzero_ctr_den  +=  nonzero_ctr_den ;
    if(nonzero_ctr_den)
      tot_nonzero_ctr_perc +=  nonzero_ctr_den * 100* ( (double)nonzero_ctr_num/(double)nonzero_ctr_den)/((double)ctr_types->num_major_ctr_per_cl * (double)ctr_types->num_minor_ctr_per_major_ctr) ;

    if(dynrange_ctr_den){
      tot_dynrange_ctr_den += dynrange_ctr_den ;
      tot_dynrange_ctr_perc +=   100* dynrange_ctr_num;
    }

    if(dynrange_ctr_den_no_zero){    
      tot_dynrange_ctr_den_no_zero  += dynrange_ctr_den_no_zero  ;
      tot_dynrange_ctr_perc_no_zero  +=  100*  dynrange_ctr_num_no_zero ;
    }
    
    printf ("AVG_NONZERO_CTR_FINLEVEL_%d         \t: %.2f\n",mem_org->num_Mtree_levels - mtree_level, (double)nonzero_ctr_num/(double)nonzero_ctr_den );
    printf ("AVG_NONZERO_PERC_CTR_FINLEVEL_%d    \t: %.2f\n",mem_org->num_Mtree_levels - mtree_level, 100* ( (double)nonzero_ctr_num/(double)nonzero_ctr_den)/((double)ctr_types->num_major_ctr_per_cl * (double)ctr_types->num_minor_ctr_per_major_ctr));
        
    printf ("AVG_DYNRANGE_CTR_FINLEVEL_%d         \t: %.2f\n",mem_org->num_Mtree_levels - mtree_level, (double)dynrange_ctr_num/(double)dynrange_ctr_den );
    printf ("AVG_DYNRANGE_PERC_CTR_FINLEVEL_%d    \t: %.2f\n",mem_org->num_Mtree_levels - mtree_level, 100* ( (double)dynrange_ctr_num/(double)dynrange_ctr_den));
    printf ("AVG_No0_DYNRANGE_CTR_FINLEVEL_%d         \t: %.2f\n",mem_org->num_Mtree_levels - mtree_level, (double)dynrange_ctr_num_no_zero/(double)dynrange_ctr_den_no_zero );
    printf ("AVG_No0_DYNRANGE_PERC_CTR_FINLEVEL_%d    \t: %.2f\n\n",mem_org->num_Mtree_levels - mtree_level, 100* ( (double)dynrange_ctr_num_no_zero/(double)dynrange_ctr_den_no_zero)); 
  }

  printf ("AVG_NONZERO_PERC_CTR_OVERALL    \t: %.2f\n", (double)tot_nonzero_ctr_perc / (double)tot_nonzero_ctr_den);        
  printf ("AVG_DYNRANGE_PERC_CTR_OVERALL    \t: %.2f\n\n",(double)tot_dynrange_ctr_perc / (double)tot_dynrange_ctr_den);
  printf ("AVG_No0_DYNRANGE_PERC_CTR_OVERALL    \t: %.2f\n\n",(double)tot_dynrange_ctr_perc_no_zero / (double)tot_dynrange_ctr_den_no_zero);

}


/************************************************/
//Function to reset the stats
/************************************************/

void reset_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr){

  for(int mtree_level= mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    
    ctr_type* ctr_types = ctr_types_arr[mtree_level];
    ctr_cl* ctr_cls = ctr_cls_arr[mtree_level];
  
    //Initialize the ctr_cls based on ctr_types
    for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
      
      for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){
	
        ctr_cls[ctr_cl_num].major_ctr_val[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].num_overflows[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].last_overflow_cycle[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].num_reads[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].num_writes[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].data_reads[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].data_writes[major_ctr_num] = 0;
        
        for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
          ctr_cls[ctr_cl_num].minor_ctr_val[major_ctr_num][minor_ctr_num] = 0;	
        }
      }
      //Reset value of bitlen
      if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3) ){ //Modes where compression used for cache per
        ctr_cls[ctr_cl_num].bitlen =  MAJOR_CTR_BITLEN + best(ctr_cls[ctr_cl_num].minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr);
      }

      
    }
  }
}


void reset_tot_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr){

  for(int mtree_level= mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    
    ctr_type* ctr_types = ctr_types_arr[mtree_level];
    ctr_cl* ctr_cls = ctr_cls_arr[mtree_level];
  
    //Initialize the ctr_cls based on ctr_types
    for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
      
      for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){
	
        //ctr_cls[ctr_cl_num].major_ctr_val[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].num_overflows[major_ctr_num] = 0;
        //ctr_cls[ctr_cl_num].last_overflow_cycle[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].increments = 0;               

        ctr_cls[ctr_cl_num].num_reads[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].num_writes[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].data_reads[major_ctr_num] = 0;
        ctr_cls[ctr_cl_num].data_writes[major_ctr_num] = 0;

        ctr_cls[ctr_cl_num].num_nonzero_numer = 0; //Number of non-zero counters out of total ctrs in cacheline at overflow.
        ctr_cls[ctr_cl_num].num_nonzero_denom = 0;

        ctr_cls[ctr_cl_num].num_smalldynrange_numer          = 0   ; // <7 DYNRANGE
        ctr_cls[ctr_cl_num].num_smalldynrange_denom          = 0   ; //  <7 DYNRANGE
        ctr_cls[ctr_cl_num].num_smalldynrange_numer_no_zero  = 0   ; // NOZERO
        ctr_cls[ctr_cl_num].num_smalldynrange_denom_no_zero  = 0   ; // NOZERO

        //for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
        //  ctr_cls[ctr_cl_num].minor_ctr_val[major_ctr_num][minor_ctr_num] = 0;	
        //}
      }
      /*
      //Reset value of bitlen
      //      if( (COMP_CTR_MODE == 2) ||  (COMP_CTR_MODE == 3) ){ //Modes where compression used for cache per
      //        ctr_cls[ctr_cl_num].bitlen =  MAJOR_CTR_BITLEN + best(ctr_cls[ctr_cl_num].minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr);
      //      }
      */
      
    }

#ifdef HIST_NZ
    //Reset the HistNz logs in ctr_types
    for (int i=0; i<= (ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr) ; i++){
      ctr_types -> nz_ctr_buckets[i] = 0;      
    }
    ctr_types->nz_ctr_tot = 0;
#endif
    
  }
    
} 
  

/************************************************/
//Function to ensure all dirty data/ctrs have their ctr_sim updated
/************************************************/

void fin_stat_overflows(MCache* L3, MCache* MET, memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types,overflows_stat* ctr_overflows_levelwise,  overflows_stat* ctr_overflows_levelwise_inst ){

  //Read each entry in L3 cache
  for(uns64 entry_id = 0; entry_id < (uns64) (L3->sets*L3->assocs); entry_id++){    

    if( (L3->entries[entry_id]).valid && (L3->entries[entry_id]).dirty){
      uns64 paddr = (L3->entries[entry_id].tag) << 6;
      int entry_type = get_partition(paddr, mem_org);
      
      //Check partition and update the counters in cache_sim_stats  
      if(entry_type == 0){
        //Find and Update counter
        ctr_mtree_entry ctr = getCounterAddr( paddr, mem_org);
        update_ctr_cls_wrapper(mem_org, ctr_cls,ctr_types, ctr, mem_org->num_Mtree_levels,ctr_overflows_levelwise, ctr_overflows_levelwise_inst);
      }
      else if((entry_type == 1) || (entry_type == 2) ){
        //Find and Update counter
        ctr_mtree_entry ctr = getMtreeEvictParent( paddr, mem_org);
        update_ctr_cls_wrapper(mem_org, ctr_cls,ctr_types, ctr,ctr.mtree_level, ctr_overflows_levelwise, ctr_overflows_levelwise_inst);
      }       
    }
    
  }
  
}

/************************************************/
//Function to analyze overflows and log the stats
/************************************************/

void log_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr, FILE* Policyfile){

  fprintf(Policyfile,"Address\t");          

  fprintf(Policyfile,"Increments\t");
  fprintf(Policyfile,"Overflows\t");
  fprintf(Policyfile,"Maxval_MinorCtr\t");
  fprintf(Policyfile,"Ctr_Reads\t");
  fprintf(Policyfile,"Ctr_Writes\t");
  fprintf(Policyfile,"Data_Reads\t");
  fprintf(Policyfile,"Data_Writes\t");

  fprintf(Policyfile,"Norm_Increments\t");
  fprintf(Policyfile,"Norm_Overflows\t");
  fprintf(Policyfile,"Norm_Ctr_Reads\t");
  fprintf(Policyfile,"Norm_Ctr_Writes\t");
  fprintf(Policyfile,"Norm_Data_Reads\t");
  fprintf(Policyfile,"Norm_Data_Writes\t");

  //  fprintf(Policyfile,"Norm_Even_stat\t");
  fprintf(Policyfile,"Norm_Even_stat_v1\t");


  ctr_type* ctr_types = ctr_types_arr[mem_org->num_Mtree_levels];
  ctr_cl* ctr_cls = ctr_cls_arr[mem_org->num_Mtree_levels];

  uns64 increments = 0;
  uns64 num_overflows = 0;
  uns64 total_count = 0;
  uns64 num_reads = 0;
  uns64 num_writes = 0;
  uns64 data_reads = 0;
  uns64 data_writes = 0;

  for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
    fprintf(Policyfile,"CL_%d\t",minor_ctr_num);
  }
  fprintf(Policyfile,"\n");
  
  
  for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num++){
    for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){

      //Total Count to be deprecated in place of dirty evicts / memory cacheline writes
      /*
        for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
        total_count+= 	ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num];
        }
        total_count+= ctr_cls[ctr_cl_num].major_ctr_val[maj_ctr]*ctr_types->minor_ctr_maxval;
      */
      num_overflows += ctr_cls[ctr_cl_num].num_overflows[maj_ctr];
      num_reads += ctr_cls[ctr_cl_num].num_reads[maj_ctr];
      num_writes += ctr_cls[ctr_cl_num].num_writes[maj_ctr];
      data_reads += ctr_cls[ctr_cl_num].data_reads[maj_ctr];
      data_writes += ctr_cls[ctr_cl_num].data_writes[maj_ctr];      
    }
    increments += ctr_cls[ctr_cl_num].increments;
      
  }

  if(increments == 0)
    increments++;
  if(num_overflows == 0)
    num_overflows++;
  if(num_reads == 0)
    num_reads++;
  if(num_writes == 0)
    num_writes++;
  if(data_reads == 0)
    data_writes++;
  if(data_writes == 0)
    data_writes++;
  
  uns64 cacheline_addr = 0;
  uns64 rand_skip = 10;

  for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num=ctr_cl_num+rand_skip){
    for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){

      uns64 page_sum=0, page_max=0;
      double page_avg=0;
      double even_stat = 0;
      
      for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
        page_sum+= ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num];
        if(page_max< ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num]){
          page_max = ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num];
        }          
      }
      page_avg = (double)page_sum/(double)ctr_types->num_minor_ctr_per_major_ctr;

      
      if(ctr_cls[ctr_cl_num].num_overflows[maj_ctr] ||  ctr_cls[ctr_cl_num].num_reads[maj_ctr] || ctr_cls[ctr_cl_num].num_writes[maj_ctr] || ctr_cls[ctr_cl_num].data_reads[maj_ctr] || ctr_cls[ctr_cl_num].data_writes[maj_ctr] ){
        //Address
        fprintf(Policyfile,"%llu\t",cacheline_addr);
        cacheline_addr+= ctr_types->num_minor_ctr_per_major_ctr;
        
        //Increments
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].increments); 

        //Overflows
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].num_overflows[maj_ctr]); 
       
        //Max Minor Counter Value 
        fprintf(Policyfile,"%llu\t",page_max); 
        
        //Reads
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].num_reads[maj_ctr]); 

        //Writes
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].num_writes[maj_ctr]); 

        //Data Reads
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].data_reads[maj_ctr]); 

        //Data Writes
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].data_writes[maj_ctr]); 

        //Norm_Overflows
        fprintf(Policyfile,"%f\t",(double)ctr_cls[ctr_cl_num].increments/(double)increments); 
      
        //Norm_Overflows
        fprintf(Policyfile,"%f\t",(double)ctr_cls[ctr_cl_num].num_overflows[maj_ctr]/(double)num_overflows); 
      
        //Norm_Reads
        fprintf(Policyfile,"%f\t",(double)ctr_cls[ctr_cl_num].num_reads[maj_ctr]/(double)num_reads); 

        //Norm_Writes
        fprintf(Policyfile,"%f\t",(double)ctr_cls[ctr_cl_num].num_writes[maj_ctr]/(double)num_writes); 

        //Norm_Reads
        fprintf(Policyfile,"%f\t",(double)ctr_cls[ctr_cl_num].data_reads[maj_ctr]/(double)data_reads); 

        //Norm_Writes
        fprintf(Policyfile,"%f\t",(double)ctr_cls[ctr_cl_num].data_writes[maj_ctr]/(double)data_writes); 
        /*
        //Norm_Even_Stat
        even_stat = 0;
        if(page_sum != 0){
        for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
        even_stat += fabs( ( (double)ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num] - (double)page_avg  )/(double)page_sum );
        }
        fprintf(Policyfile,"%f\t",(double)even_stat); 
        }
        else{
        fprintf(Policyfile,"%f\t",0); 
        }
        */
        //Norm_Even_Stat
        if(page_avg != 0)
          fprintf(Policyfile,"%f\t",(double)page_max/(double)page_avg);
        else
          fprintf(Policyfile,"%d\t",0);                 
       
        for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
          fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num]);                        
        }
        //New Line
        fprintf(Policyfile,"\n");      

      }
    }

    //To accomodate printing of stats for 1 in 10 counter cachelines
    cacheline_addr+= ctr_types->num_minor_ctr_per_major_ctr *ctr_types->num_major_ctr_per_cl*(rand_skip -1) ;
    
  }
  
  
}



void log2_stat_overflows(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr, FILE* Policyfile){

  fprintf(Policyfile,"Ctr_Mt_Addr\t");          
  fprintf(Policyfile,"Ctr_Mt_Reads\t");
  fprintf(Policyfile,"Ctr_Mt_Writes\t");
  
  fprintf(Policyfile,"Norm_Ctr_Mt_Reads\t");
  fprintf(Policyfile,"Norm_Ctr_Mt_Writes\t");
  
  /*
    fprintf(Policyfile,"Norm_Data_Reads\t");
    fprintf(Policyfile,"Norm_Data_Writes\t");
  */
  
  //  fprintf(Policyfile,"Norm_Even_stat\t");
  fprintf(Policyfile,"Norm_Even_stat_v1\t");


  ctr_type* ctr_types = ctr_types_arr[mem_org->num_Mtree_levels];
  ctr_cl* ctr_cls = ctr_cls_arr[mem_org->num_Mtree_levels];
    
  uns64 num_overflows = 0;
  uns64 total_count = 0;
  uns64 num_reads = 0;
  uns64 num_writes = 0;
  uns64 data_reads = 0;
  uns64 data_writes = 0;

  /*
    for(uns64 ctr_cl_num =0; ctr_cl_num < MTREE_ARY ; ctr_cl_num++){
    for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){
    fprintf(Policyfile,"Ctr_Rd_%d\t",ctr_cl_num);
    }
    }
  */
  for(uns64 ctr_cl_num =0; ctr_cl_num < MTREE_ARY[ctr_types->mtree_level] ; ctr_cl_num++){
    for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){
      fprintf(Policyfile,"Ctr_Wr_%d\t",ctr_cl_num);
    }
  }
  fprintf(Policyfile,"\n");

  //  printf("Testing %d is MTREEARY for level %d\n",MTREE_ARY[ctr_types->mtree_level], ctr_types->mtree_level);
  
  for(uns64 ctr_mtree_num = 0; ctr_mtree_num < (ctr_types->num_ctr_cls/MTREE_ARY[ctr_types->mtree_level]); ctr_mtree_num++){
    for(int ctr_cl = 0; ctr_cl < MTREE_ARY[ctr_types->mtree_level]; ctr_cl++){
      uns64 ctr_cl_num = ctr_mtree_num*MTREE_ARY[ctr_types->mtree_level] + ctr_cl;

      for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){
        num_reads += ctr_cls[ctr_cl_num].num_reads[maj_ctr];
        num_writes += ctr_cls[ctr_cl_num].num_writes[maj_ctr];
      }      
    }
      
  }
  if(num_reads == 0)
    num_reads++;
  if(num_writes == 0)
    num_writes++;
    
  for(uns64 ctr_mtree_num = 0; ctr_mtree_num < (ctr_types->num_ctr_cls / MTREE_ARY[ctr_types->mtree_level]); ctr_mtree_num++){
 
    uns64 page_sum=0, page_max=0;
    double page_avg=0;
    double even_stat = 0;

    uns64 ctr_cls_reads = 0;
    
    for(int ctr_cl = 0; ctr_cl < MTREE_ARY[ctr_types->mtree_level]; ctr_cl++){

      uns64 ctr_cl_num = ctr_mtree_num*MTREE_ARY[ctr_types->mtree_level] + ctr_cl;

      for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){

        ctr_cls_reads += ctr_cls[ctr_cl_num].num_reads[maj_ctr];
        page_sum+= ctr_cls[ctr_cl_num].num_writes[maj_ctr];
        if(page_max< ctr_cls[ctr_cl_num].num_writes[maj_ctr]){
          page_max = ctr_cls[ctr_cl_num].num_writes[maj_ctr];
        }

        
      }
    }
    page_avg = (double)page_sum/(double)(MTREE_ARY[ctr_types->mtree_level]);

      
    if(page_sum || ctr_cls_reads ){
      //Address
      fprintf(Policyfile,"%llu\t",ctr_mtree_num);    
    
      //Reads
      fprintf(Policyfile,"%llu\t",ctr_cls_reads); 

      //Writes
      fprintf(Policyfile,"%llu\t",page_sum); 

      //Norm_Reads
      fprintf(Policyfile,"%f\t",(double)ctr_cls_reads/(double)num_reads); 

      //Norm_Writes
      fprintf(Policyfile,"%f\t",(double)page_sum/(double)num_writes); 

      //Norm_Even_Stat
      if(page_avg != 0)
        fprintf(Policyfile,"%f\t",(double)page_max/(double)page_avg);
      else
        fprintf(Policyfile,"%d\t",0);                 
      /*
        for(int ctr_cl = 0; ctr_cl < MTREE_ARY; ctr_cl++){    
        uns64 ctr_cl_num = ctr_mtree_num*MTREE_ARY + ctr_cl;        
        for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].num_reads[maj_ctr]); //ctr_reads
        }
        }
      */
      for(int ctr_cl = 0; ctr_cl < MTREE_ARY[ctr_types->mtree_level]; ctr_cl++){    
        uns64 ctr_cl_num = ctr_mtree_num*MTREE_ARY[ctr_types->mtree_level] + ctr_cl;        
        for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){
          fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].num_writes[maj_ctr]); //ctr_writes
        }
      }
      //New Line
      fprintf(Policyfile,"\n");      

    }
  }
  
}

void log_stat_overflows_HistNz(memOrg_t* mem_org,ctr_type** ctr_types_arr, FILE* Policyfile){
#ifdef HIST_NZ
  
  int num_leaf_minor_ctrs = (ctr_types_arr[mem_org->num_Mtree_levels]->num_major_ctr_per_cl * ctr_types_arr[mem_org->num_Mtree_levels]->num_minor_ctr_per_major_ctr );
  uns64* overall_HistNz =  (uns64*)calloc(num_leaf_minor_ctrs + 1, sizeof(uns64));
  uns64 overall_total = 0;
  uns64* overall_HistNz_woparent =  (uns64*)calloc(num_leaf_minor_ctrs + 1, sizeof(uns64));
  uns64 overall_total_woparent = 0;
  
  for(int mtree_level = mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    ctr_type* ctr_types = ctr_types_arr[mtree_level];

    ASSERTM(num_leaf_minor_ctrs  >= (ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr),"Array index out of bounds\n" );

    for (int i=0; i<= (ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr) ; i++){
      overall_HistNz[i] += ctr_types -> nz_ctr_buckets[i] ;
      overall_total     += ctr_types -> nz_ctr_buckets[i] ;
      if(mtree_level != (mem_org->num_Mtree_levels - 1)){
        overall_HistNz_woparent[i] += ctr_types -> nz_ctr_buckets[i] ;
        overall_total_woparent   += ctr_types -> nz_ctr_buckets[i] ;
      }
    }    
  }

  //Print Header
  fprintf(Policyfile,"Nz_Ctrs\t");          
  fprintf(Policyfile,"Cumu_OF_Frac\t");          
  fprintf(Policyfile,"Cumu_OF_Val\t");
  fprintf(Policyfile,"ExcParent_OF_Frac\t");          
  fprintf(Policyfile,"ExcParent_OF_Val\t");          

  for(int mtree_level = mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
    fprintf(Policyfile,"Ctr_%d_OF_Frac\t",mem_org->num_Mtree_levels - mtree_level);          
    fprintf(Policyfile,"Ctr_%d_OF_Val\t",mem_org->num_Mtree_levels - mtree_level);
  }
  fprintf(Policyfile,"\n");

  for (int i=0; i<= (num_leaf_minor_ctrs) ; i++){
    //Print Line
    fprintf(Policyfile,"%d\t",i);          
    fprintf(Policyfile,"%.4f\t",(double)overall_HistNz[i]/(double)overall_total);
    fprintf(Policyfile,"%llu\t",overall_HistNz[i]);
    fprintf(Policyfile,"%.4f\t",(double)overall_HistNz_woparent[i]/(double)overall_total_woparent);
    fprintf(Policyfile,"%llu\t",overall_HistNz_woparent[i]);  

    for(int mtree_level = mem_org->num_Mtree_levels; mtree_level >=0; mtree_level--){
      ctr_type* ctr_types = ctr_types_arr[mtree_level];
      if(i <= (ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr) ){
        fprintf(Policyfile,"%.4f\t",(double)ctr_types->nz_ctr_buckets[i]/(double)ctr_types->nz_ctr_tot);
        fprintf(Policyfile,"%llu\t",ctr_types->nz_ctr_buckets[i]);
      }
      else {
        fprintf(Policyfile,"0.0\t0\t");
      }
    }
    fprintf(Policyfile,"\n");    
  }
#endif
}


 
void insert_overflow_mem_accesses(ADDR ctr_paddr, memOrg_t* mem_org, overflows_stat* ctr_overflows_levelwise_ptr,  overflows_stat* ctr_overflows_levelwise_inst_ptr, int major_ctr_id, int num_major_ctr_per_cl){
  int num_children = 0;
  ADDR first_child_paddr = getCtrChild(ctr_paddr, mem_org, &num_children);
  maccess_type twin_type =  {TWIN,-1};
  int num_children_per_major_ctr = num_children / num_major_ctr_per_cl;
  int start_child = num_children_per_major_ctr * major_ctr_id;
  
  for(int i= start_child; i< (start_child + num_children_per_major_ctr); i++){
    ADDR access_paddr = first_child_paddr + i*CACHE_LINE_SIZE;
    int part = get_partition(first_child_paddr, mem_org);
    
    //insert data read
    //if((part == 0) && ( (rand()%2)  == 0) ){
    if(part == 0){ 
      insert_read(access_paddr, CYCLE_VAL, 0, 0, 0,0, FALSE, twin_type);
      ctr_overflows_levelwise_ptr->child_accesses++;
      ctr_overflows_levelwise_inst_ptr->child_accesses++;      
    }
    else if( ((part == 1 ) || (part == 2 ) ) && (cache_access(METCache,MET_PRIVATE,access_paddr,FALSE,0,MET_Cache_Stats,twin_type,mem_org) == MISS) ){ 
      insert_read(access_paddr, CYCLE_VAL, 0, 0, 0,0, FALSE, twin_type);
      ctr_overflows_levelwise_ptr->child_accesses++;
      ctr_overflows_levelwise_inst_ptr->child_accesses++;      

    }
    
    //insert write
    insert_write(access_paddr, CYCLE_VAL, 0, 0, 0, FALSE, twin_type); 
    ctr_overflows_levelwise_ptr->child_accesses++;
    ctr_overflows_levelwise_inst_ptr->child_accesses++;      
  }

}

int check_overflow(ctr_cl* curr_ctr_cl , uns64 major_ctr_id, uns64 minor_ctr_id, ctr_type* ctr_types){

  ASSERTM( (ctr_types->num_major_ctr_per_cl == 1) || (COMPRESSED_MTREE == 7) ||  (COMPRESSED_MTREE == 8),"Not designed for multiple major ctr in cacheline\n");
  ctr_types->log_num_check_overflows++; //Log number of times a counter incremented (checked for an overflow occurence).
  
  if( curr_ctr_cl->minor_ctr_val[major_ctr_id][minor_ctr_id] > ctr_types->minor_ctr_maxval ){ //Bug: Will not trigger when minor_ctr_maxval is 2^64 - 1. Counter will wrap around to 0 on overflow condition. But this should not happen in my lifetime :)
    
    return 1; //Overflows if minor counter incremented has exceeded the limit of overflow
  }

  if(ctr_types->compression_enabled){
    int add_bits = 0;
    //Take care of additional bits for MAC, Major Counter
    int SGX_MODE_SAMPLE = SGX_MODE[0];

    for(int i=0;i<NUMCORES;i++){
      ASSERTM(SGX_MODE_SAMPLE == SGX_MODE[i], "Some cores in secure, others non-secure");     
    }

    ASSERTM(SGX_MODE_SAMPLE != 5, "In mode 5, counters arent used");
    
    if( (SGX_MODE_SAMPLE == 1) || (SGX_MODE_SAMPLE == 2)){ //MAC not in ECC-Chip
      add_bits = MAJOR_CTR_BITLEN + MAC_BITLEN;
    }
    else if( (SGX_MODE_SAMPLE == 3) || (SGX_MODE_SAMPLE == 4)){ //MAC not in ECC-Chip
      add_bits = MAJOR_CTR_BITLEN + MAC_BITLEN;
    }

    //Check if compressible
    if((COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3) || (COMP_CTR_MODE == 2) ){ //Perform compression on the Counter Cacheline

      if(COMPRESSED_MTREE < 5){
        uns64 comp_ctr_cl_bits = best_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen,add_bits);         
        
        if((COMP_CTR_MODE == 2) || (COMP_CTR_MODE == 3)) { //Update the bitlen in the ctr_cl
          curr_ctr_cl->bitlen = comp_ctr_cl_bits + MAJOR_CTR_BITLEN ;
        }

        if((COMP_CTR_MODE == 1) || (COMP_CTR_MODE == 3)) {
          if((comp_ctr_cl_bits + add_bits) > (CACHE_LINE_SIZE * 8)){
            return 1; //Overflows if compressed size is more than cacheline size.
          }
        }
        
      } else if (COMPRESSED_MTREE == 5){
        ASSERTM(COMP_CTR_MODE == 1, "Not designed for 2 or 3");
        add_bits += (int)ceil(log2((int)ctr_types->num_minor_ctr_bitslen_arr));
        for(int x= 0 ; x<ctr_types->num_minor_ctr_bitslen_arr; x++){
          uns64 comp_ctr_cl_bits = best_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen_arr[x],add_bits);         
          if((comp_ctr_cl_bits + add_bits) <= (CACHE_LINE_SIZE * 8)){
            return 0; //Overflows if compressed size is more than cacheline size.
          }
        }

        //No minor_ctr_bitlen fits cacheline
        return 1; // Overflow
        
      }
      else if ( (COMPRESSED_MTREE == 6) ||  (COMPRESSED_MTREE == 7)  ||  (COMPRESSED_MTREE == 8)  ) {        
        add_bits += 1; //For the compressed flag.

        for(int x=curr_ctr_cl->comp_mode[major_ctr_id]; x<ctr_types->num_minor_ctr_bitslen_arr; x++){

          //Skip BPC (x= 1) in COMPRESSED_MTREE = 7 mode
          if( ((COMPRESSED_MTREE == 7) ||  (COMPRESSED_MTREE == 8)) && (x == 1)) { 
            continue;
          }
          ctr_types->log_comp_calls[x]++; //increment count of times compression used

          //Try Different Compression Modes. x=0 ->AZC, x=2 -> Uncompressed.

          if(try_comp_wrapper(curr_ctr_cl,major_ctr_id, minor_ctr_id, ctr_types,add_bits,x)){
            //Compresssion Worked!
            return 0;
          }
        }
        
        //Not compressible within 3 compression modes.
        if(COMPRESSED_MTREE == 8) {
          //Rebase minor counters
          if(try_rebase(&(curr_ctr_cl->major_ctr_val[major_ctr_id]),curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, ctr_types->major_ctr_maxval) ){

            //Reset the comp_mode and try compressing again.
            curr_ctr_cl -> comp_mode[major_ctr_id] = 0;
            
            for(int x=curr_ctr_cl->comp_mode[major_ctr_id]; x<ctr_types->num_minor_ctr_bitslen_arr; x++){

              //Skip BPC (x= 1) in COMPRESSED_MTREE = 7 mode
              if( ((COMPRESSED_MTREE == 7) ||  (COMPRESSED_MTREE == 8)) && (x == 1)) { 
                continue;
              }
              ctr_types->log_comp_calls[x]++; //increment count of times compression used

              //Try Different Compression Modes. x=0 ->AZC, x=2 -> Uncompressed.
              if(try_comp_wrapper(curr_ctr_cl, major_ctr_id, minor_ctr_id,ctr_types,add_bits,x)){
                return 0;
              }
            }

          }
        }
        
        //Failed to compress the cacheline, incur an overflow.
        return 1; 
      }
      else {
        ASSERTM(0, "COMPRESSED_MTREE >7 not designed");
      }           
    }

    
  }
  return 0;
}
 

uns64 get_ctr_cl_bitlen(memOrg_t* mem_org, ctr_cl** ctr_cls, ctr_type** ctr_types, ADDR ctr_paddr, int ctr_mtree_level){
  uns64 ctr_cl_bitlen = 0 , ctr_id =0;
  ctr_cl* curr_ctr_cl ;
  ctr_type* curr_ctr_type; 

  ASSERTM( (ctr_mtree_level <= mem_org->num_Mtree_levels) && (ctr_mtree_level > 0), "Invalid ctr_mtree_level");
  
  curr_ctr_cl =  ctr_cls[ctr_mtree_level];
  curr_ctr_type =  ctr_types[ctr_mtree_level];

  if(curr_ctr_type->mtree_level != 0){
    ASSERTM((getMtreeEvictParent(ctr_paddr,mem_org).mtree_level + 1) == curr_ctr_type->mtree_level,"Requested ctr_paddr does not refer to selected mtree_level\n");
  }


  ctr_id = ( ctr_paddr - mem_org -> mtree_levels_start_addr[curr_ctr_type -> mtree_level] ) >> log_base2(CACHE_LINE);
  
  ctr_cl_bitlen =  MAJOR_CTR_BITLEN + best(curr_ctr_cl[ctr_id].minor_ctr_val[0], curr_ctr_type->num_minor_ctr_per_major_ctr);

  return ctr_cl_bitlen;
  
}


void log_stat_ctr_vals(memOrg_t* mem_org, ctr_cl** ctr_cls_arr, ctr_type** ctr_types_arr, FILE* Policyfile, int mtree_level){

  fprintf(Policyfile,"Address\t");          
  fprintf(Policyfile,"Increments\t");
  fprintf(Policyfile,"Overflows\t");
  fprintf(Policyfile,"Maxval_MinorCtr\t");

  ASSERTM(mtree_level <= mem_org->num_Mtree_levels, "Illegal mtree_level parameter to log_func");
  ctr_type* ctr_types = ctr_types_arr[mtree_level];
  ctr_cl* ctr_cls = ctr_cls_arr[mtree_level];

  for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
    fprintf(Policyfile,"CL_%d\t",minor_ctr_num);
  }
  fprintf(Policyfile,"\n");

  uns64 cacheline_addr = 0;
  uns64 rand_skip = 1;

  for(uns64 ctr_cl_num = 0; ctr_cl_num <ctr_types->num_ctr_cls; ctr_cl_num=ctr_cl_num+rand_skip){
    for(int maj_ctr = 0; maj_ctr < ctr_types->num_major_ctr_per_cl; maj_ctr++){

      uns64 page_sum=0, page_max=0;
      double page_avg=0;
      double even_stat = 0;
      
      for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
        page_sum+= ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num];
        if(page_max< ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num]){
          page_max = ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num];
        }          
      }
      
      if(ctr_cls[ctr_cl_num].num_overflows[maj_ctr] ||  ctr_cls[ctr_cl_num].num_reads[maj_ctr] || ctr_cls[ctr_cl_num].num_writes[maj_ctr] || ctr_cls[ctr_cl_num].data_reads[maj_ctr] || ctr_cls[ctr_cl_num].data_writes[maj_ctr] ){
        //Address
        fprintf(Policyfile,"%llu\t",cacheline_addr);
        cacheline_addr+= ctr_types->num_minor_ctr_per_major_ctr;
        
        //Increments
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].increments); 

        //Overflows
        fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].num_overflows[maj_ctr]); 
       
        //Max Minor Counter Value 
        fprintf(Policyfile,"%llu\t",page_max); 
       
        for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
          fprintf(Policyfile,"%llu\t",ctr_cls[ctr_cl_num].minor_ctr_val[maj_ctr][minor_ctr_num]);                        
        }
        //New Line
        fprintf(Policyfile,"\n");      

      }
    }

    //To accomodate printing of stats for 1 in 10 counter cachelines
    cacheline_addr+= ctr_types->num_minor_ctr_per_major_ctr *ctr_types->num_major_ctr_per_cl*(rand_skip -1) ;
    
  }
  
  
}


void log_comp_schemes(ctr_type* ctr_types, ctr_cl* curr_ctr_cl, int major_ctr_id){
  /*
    if(COMPRESSED_MTREE < 5){
    assert(0);
    }
    else if (COMPRESSED_MTREE == 5){
    ctr_types->log_num_overflows++;
    
    for(int x= 0 ; x<ctr_types->num_minor_ctr_bitslen_arr; x++){
    if(zero_val_comp(curr_ctr_cl->minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen_arr[x]) <= ( CACHE_LINE*8 - MAJOR_CTR_BITLEN - MAC_BITLEN -2 - 2) ) {
    ctr_types->log_comp[x][0]++;
    }
    if(ptrs_non_zero_comp(curr_ctr_cl->minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen_arr[x]) <= ( CACHE_LINE*8 - MAJOR_CTR_BITLEN - MAC_BITLEN -2 - 2) ) {
    ctr_types->log_comp[x][1]++;
    }
    if(bpc_comp_new(curr_ctr_cl->minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen_arr[x]) <= ( CACHE_LINE*8 - MAJOR_CTR_BITLEN - MAC_BITLEN -2 - 2) ) {
    ctr_types->log_comp[x][2]++;
    }
    if(bdi_comp_new(curr_ctr_cl->minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen_arr[x],MAJOR_CTR_BITLEN+MAC_BITLEN+2) <= ( CACHE_LINE*8 - MAJOR_CTR_BITLEN - MAC_BITLEN -2 - 2) ) {
    ctr_types->log_comp[x][3]++;
    }
    }
    }
    else {
    assert(0);
    }
  */

  if ( (COMPRESSED_MTREE == 6) ||  (COMPRESSED_MTREE == 7) ||  (COMPRESSED_MTREE == 8) ){
    ctr_types->log_num_overflows++;
    int x = curr_ctr_cl -> comp_mode[major_ctr_id] ;
    ctr_types->log_comp[x][0]++;
  }  
  
} 


void print_logs_comp_schemes(ctr_type** ctr_types, memOrg_t* mem_org){
  /*
    printf("*** EFFECTIVENESS OF DIFFERENT COMP SCHEMES\n");
    uns64 overflows = 0;
    uns64 log[NUM_CTR_BITLENS][NUM_COMP_SCHEMES];
    char* comp_names[NUM_COMP_SCHEMES] = {"dyn0","ptrnz", "bpc","bdi"};
  
    for(int b=0; b< NUM_CTR_BITLENS; b++){
    for (int a=0; a<NUM_COMP_SCHEMES; a++){

    log[b][a] = 0;
    for(int i = mem_org->num_Mtree_levels ; i > 0; i--){
    log[b][a]+= ctr_types[i]->log_comp[b][a];
    }
    }
    }

    for(int i = mem_org->num_Mtree_levels ; i > 0; i--){
    overflows += ctr_types[i]->log_num_overflows;   
    }
  
    printf("NUM_OVERFLOWS_ALLCTRS_ALL      \t: %llu\n",overflows);

    for(int b=0; b< NUM_CTR_BITLENS; b++){
    for (int a=0; a<NUM_COMP_SCHEMES; a++){  
    printf("NUM_OF_COVERED_ALLCTRS_C%s_B%d      \t: %llu\n",comp_names[a],(int)ctr_types[mem_org->num_Mtree_levels-1]->minor_ctr_bitslen_arr[b],log[b][a]);
    }
    }

    printf("\n");

    for(int i = mem_org->num_Mtree_levels ; i > 0; i--){

    printf("NUM_OVERFLOWS_CTRS%d      \t: %llu\n",i,ctr_types[i]->log_num_overflows);
    for(int b=0; b< NUM_CTR_BITLENS; b++){
    for (int a=0; a<NUM_COMP_SCHEMES; a++){  
    printf("NUM_OF_COVERED_CTRS%d_C%s_B%d      \t: %llu\n",i,comp_names[a],(int)ctr_types[i]->minor_ctr_bitslen_arr[b], ctr_types[i]->log_comp[b][a]);
    }
    }

    printf("\n");
    }
  */  

  if ( (COMPRESSED_MTREE == 6) ||  (COMPRESSED_MTREE == 7) ||  (COMPRESSED_MTREE == 8)  ) {

    printf("*** EFFECTIVENESS OF DIFFERENT COMP SCHEMES\n");
    uns64 overflows = 0;
    uns64 log[NUM_CTR_BITLENS];
    char* comp_names[NUM_CTR_BITLENS] = {"dyn0","bpc","uncomp"};
  
    for(int b=0; b< NUM_CTR_BITLENS; b++){

      log[b] = 0;
      for(int i = mem_org->num_Mtree_levels ; i > 0; i--){
        log[b]+= ctr_types[i]->log_comp[b][0];
      }
    }

    for(int i = mem_org->num_Mtree_levels ; i > 0; i--){
      overflows += ctr_types[i]->log_num_overflows;   
    }
  
    printf("NUM_OVERFLOWS_ALLCTRS_ALL      \t: %llu\n",overflows);

    for(int b=0; b< NUM_CTR_BITLENS; b++){
      printf("NUM_OF_COVERED_ALLCTRS_C%s_B%d      \t: %llu\n",comp_names[b],(int)ctr_types[mem_org->num_Mtree_levels-1]->minor_ctr_bitslen_arr[b],log[b]);
    }

    printf("\n");

    for(int i = mem_org->num_Mtree_levels ; i > 0; i--){

      printf("NUM_OVERFLOWS_CTRS%d      \t: %llu\n",i,ctr_types[i]->log_num_overflows);
      for(int b=0; b< NUM_CTR_BITLENS; b++){
        printf("NUM_OF_COVERED_CTRS%d_C%s_B%d      \t: %llu\n",i,comp_names[b],(int)ctr_types[i]->minor_ctr_bitslen_arr[b], ctr_types[i]->log_comp[b][0]);
      }
      
      printf("\n");
  
    }  

    printf("*** NUMBER OF TIMES COMPRESSION HW RUN\n");
    uns64 check_overflows = 0, log_comp_act_all = 0;
    uns64 log_comp_act[NUM_CTR_BITLENS];
  
    for(int b=0; b< NUM_CTR_BITLENS; b++){

      log_comp_act[b] = 0;
      for(int i = mem_org->num_Mtree_levels ; i > 0; i--){
        log_comp_act[b]+= ctr_types[i]->log_comp_calls[b];
        log_comp_act_all += ctr_types[i]->log_comp_calls[b];
      }
    }

    for(int i = mem_org->num_Mtree_levels ; i > 0; i--){
      check_overflows += ctr_types[i]->log_num_check_overflows;   
    }

    printf("FRAC_COMP_HW_ACT_ALLCTRS_ALL      \t: %f\n",(double)log_comp_act_all/(double)check_overflows);    
    printf("NUM_INCR_ALLCTRS_ALL      \t: %llu\n",check_overflows);

    for(int b=0; b< NUM_CTR_BITLENS; b++){
      printf("NUM_OF_COMP_HW_ACT_ALLCTRS_C%s_B%d      \t: %llu\n",comp_names[b],(int)ctr_types[mem_org->num_Mtree_levels-1]->minor_ctr_bitslen_arr[b],log_comp_act[b]);
      printf("FRAC_COMP_HW_ACT_ALLCTRS_C%s_B%d      \t: %f\n",comp_names[b],(int)ctr_types[mem_org->num_Mtree_levels-1]->minor_ctr_bitslen_arr[b],(double)log_comp_act[b]/(double)check_overflows);
    }

    printf("\n");
    /*
      for(int i = mem_org->num_Mtree_levels ; i > 0; i--){

      printf("NUM_OVERFLOWS_CTRS%d      \t: %llu\n",i,ctr_types[i]->log_num_overflows);
      for(int b=0; b< NUM_CTR_BITLENS; b++){
      printf("NUM_OF_COVERED_CTRS%d_C%s_B%d      \t: %llu\n",i,comp_names[b],(int)ctr_types[i]->minor_ctr_bitslen_arr[b], ctr_types[i]->log_comp[b]);
      }
      
      printf("\n");
  
      } 
    */ 
    

  }
}

uns64 try_comp_wrapper(ctr_cl* curr_ctr_cl, uns64 major_ctr_id, uns64 minor_ctr_id, ctr_type* ctr_types, int add_bits, int mode){

  //AZC_FULL operation when mode=0 and UNC_DUAL counter type
  if( ((ctr_types->design == SPLIT128_UNC_DUAL) || (ctr_types->design == SPLIT128_UNC_7bINT_DUAL) ) && (mode == 0 ) ){

    uns64 comp_ctr_cl_bits =0  ;

    for(int i=0;i<ctr_types->num_major_ctr_per_cl;i++){
      for(int j=0; j<ctr_types->num_minor_ctr_per_major_ctr ;j++){
        temp_minor_ctr_val[i* ctr_types->num_minor_ctr_per_major_ctr + j] = curr_ctr_cl->minor_ctr_val[i][j];
      }
    }

    comp_ctr_cl_bits = try_comp(&temp_minor_ctr_val[0], ctr_types->num_minor_ctr_per_major_ctr * ctr_types->num_major_ctr_per_cl, (int)ctr_types->minor_ctr_bitslen_arr[mode],add_bits,major_ctr_id*ctr_types->num_minor_ctr_per_major_ctr + minor_ctr_id, mode, 1);

    //If compressible within cacheline, then exit with success.
    if((comp_ctr_cl_bits + add_bits) <= (int)((CACHE_LINE_SIZE * 8)) ){
      curr_ctr_cl->comp_mode[major_ctr_id] = mode;
      return 1; //compression worked !
    }
    else{
      return 0;
    }
    
  } else {     //Regular operation
    uns64 comp_ctr_cl_bits = try_comp(curr_ctr_cl->minor_ctr_val[major_ctr_id], ctr_types->num_minor_ctr_per_major_ctr, (int)ctr_types->minor_ctr_bitslen_arr[mode],add_bits/ctr_types->num_major_ctr_per_cl, minor_ctr_id, mode, ctr_types->num_major_ctr_per_cl);

    //If compressible within cacheline, then exit with success.
    if((comp_ctr_cl_bits + add_bits/ctr_types->num_major_ctr_per_cl) <= (int)((CACHE_LINE_SIZE * 8)/ctr_types->num_major_ctr_per_cl)){
      curr_ctr_cl->comp_mode[major_ctr_id] = mode;
      return 1; //compression worked !
    }
    else{
      return 0;
    }
      
  }
  
}


void track_zero_ctr_stats(ctr_type* ctr_types, ctr_cl* ctr_cls){
  int num_non_zero = 0;

  for(uns64 major_ctr_num =0; major_ctr_num < ctr_types->num_major_ctr_per_cl; major_ctr_num++){
    for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
      if(ctr_cls -> minor_ctr_val[major_ctr_num][minor_ctr_num])
        num_non_zero ++; 	
    }
  }

  ctr_cls -> num_nonzero_numer += num_non_zero;
  (ctr_cls -> num_nonzero_denom)++;

#ifdef HIST_NZ
  ASSERTM(num_non_zero <= (ctr_types->num_major_ctr_per_cl * ctr_types->num_minor_ctr_per_major_ctr), "The number of non_zero ctrs should be less than equal to total number of minor counters"); 
  ctr_types->nz_ctr_buckets[num_non_zero]++;
  ctr_types->nz_ctr_tot++;
#endif
  
}

void track_dynrange_ctr_stats(ctr_type* ctr_types, ctr_cl* ctr_cls, int major_ctr_id){
  uns64 min_ctr = ctr_cls -> minor_ctr_val[major_ctr_id][0];
  uns64 max_ctr = ctr_cls -> minor_ctr_val[major_ctr_id][0];
  int no_zero = 1;
  
  for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
    if(min_ctr > ctr_cls -> minor_ctr_val[major_ctr_id][minor_ctr_num])
      min_ctr = ctr_cls -> minor_ctr_val[major_ctr_id][minor_ctr_num];

    if(max_ctr < ctr_cls -> minor_ctr_val[major_ctr_id][minor_ctr_num])
      max_ctr = ctr_cls -> minor_ctr_val[major_ctr_id][minor_ctr_num];

    if(ctr_cls -> minor_ctr_val[major_ctr_id][minor_ctr_num] == 0)
      no_zero = 0;    
  }

  
  (ctr_cls -> num_smalldynrange_denom)++;
  (ctr_cls -> num_smalldynrange_denom_no_zero)++;

  if(max_ctr - min_ctr < 7)
    (ctr_cls -> num_smalldynrange_numer)++;

  ctr_cls -> num_smalldynrange_numer_no_zero += no_zero;
  /*
    printf("Ctr CL:\n");  
    for(uns64 minor_ctr_num =0; minor_ctr_num < ctr_types->num_minor_ctr_per_major_ctr; minor_ctr_num++){
    printf("%d ",ctr_cls -> minor_ctr_val[major_ctr_id][minor_ctr_num]);
    
    }
    printf("\n");
  */
}

