/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: compression.c
 * Description: 
 * Author: Saileshwar
 * Created: Mon Sep 18 16:30:46 2017 (-0400)
 * Last-Updated: Fri Mar  9 22:22:16 2018 (-0500)
 *     Update #: 179
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */

#include <math.h>

#include "compression.h"
#include "global_types.h"
#include "memOrg.h"

#include"/home/gattaca4/gururaj/LOCAL_LIB/zlib-1.2.11_install/include/zlib.h" //added to support GAP traces
#include "bpc.h"

extern int MAJOR_CTR_BITLEN; 
extern int MINOR_CTR_BITLEN ;
extern int COMP_CTR_MODE;


Bytef  zlib_dest[MAXINPUTLINESIZE*8];
Bytef  zlib_src[MAXINPUTLINESIZE*8];
uns64  dbx_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE];
uns64  dbp_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE];

uns64 sum_writes(uns64* line_data, int num_lines){
  uns64 return_val=0;
  
  for(int i=0;i<num_lines;i++){
    return_val += line_data[i] ;    
  }

  return return_val;
  
}

double skew_writes(uns64* line_data, int num_lines){
  double return_val=0;
  uns64 page_writes_avg = 0;
  uns64 page_writes_max = 0;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > page_writes_max)
      page_writes_max = line_data[i];
    
    page_writes_avg += line_data[i];
  }

  if(page_writes_avg)
    return_val = (double)page_writes_max / ((double)page_writes_avg /(double) num_lines);
  else
    return_val = 0;
  
  return return_val;  
}

double skew_growth(uns64* line_data, int num_lines){
  double return_val=0;
  uns64 page_writes_min = -1;
  uns64 page_writes_max = 0;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > page_writes_max)
      page_writes_max = line_data[i];

    if( (line_data[i] < page_writes_min) && (line_data[i] > 0) )
      page_writes_min = line_data[i];    
  }

  if(page_writes_min > 0)
    return (double)page_writes_max/(double)page_writes_min;
  else{
    return 0.01;
  }
}

uns64 exact_counter_bits(uns64* line_data, int num_lines){ 
  uns64 return_val=0;
  
  for(int i=0;i<num_lines;i++){
    if(line_data[i]>0)
      return_val += ceil(log2(line_data[i]));
  }
  
  return return_val;
  
}

uns64 num_cont_nonzeros(uns64* line_data, int num_lines){
  uns64 return_val = 0;
  int in_nz_chunk = 0;
  
  for(int i=0;i<num_lines;i++){
    if((line_data[i] != 0 ) && (in_nz_chunk == 0) ){
      return_val++;
      in_nz_chunk = 1;     
    }
    else if((line_data[i] == 0 ) && (in_nz_chunk == 1) ){
      in_nz_chunk =0;         
    }
  }
  return return_val;
}


uns64 num_val(uns64* line_data, int num_lines, uns64 val){

  uns64 return_val=0;
  
  for(int i=0;i<num_lines;i++){
    if(line_data[i]==val) return_val++;    
  }

  return return_val;
}



uns64 num_nonzeros(uns64* line_data, int num_lines){

  uns64 return_val=0;
  
  for(int i=0;i<num_lines;i++){
    if(line_data[i]) return_val++;    
  }

  return return_val;
}


uns64 zero_val_compression(uns64* line_data, int num_lines){

  uns64 return_val=0;  
  uns64 min_bits = 1;
  uns64 reg_bits = 14;
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i]){
      return_val += (min_bits+reg_bits);
    }
    else{
      return_val += min_bits;
    }
  }

  return return_val;
}

uns64 zero_one_compression(uns64* line_data, int num_lines){
  
  uns64 return_val=0;  
  uns64 min_bits = 1;
  uns64 imm_bits = 2;
  uns64 reg_bits = 14;
   
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (min_bits);
    }
    else if (line_data[i] == 1) {
      return_val += min_bits;
    }
    else{
      return_val += (imm_bits+reg_bits);
    }
  }    
  return return_val;
}

uns64 zero_one_freq_val_compression(uns64* line_data, int num_lines){
  
  uns64 return_val=0;  
  uns64 min_bits = 2;
  uns64 reg_bits = 14;
 
  uns64 maxFreq_val = maxFreqVal(line_data, num_lines);
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (min_bits);
    }
    else if (line_data[i] == 1) {
      return_val += min_bits;
    }
    else if (line_data[i] == maxFreq_val){
      return_val += min_bits;
    }
    else{
      return_val += (min_bits+reg_bits);
    }
  }
  return_val += reg_bits; // for maxFreqVal
    
  return return_val;
}

uns64 zero_one_max_val_compression(uns64* line_data, int num_lines){
  
  uns64 return_val=0;  
  uns64 min_bits = 2;
  uns64 reg_bits = 14;
 
  uns64 maxVal = 0;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > maxVal)
      maxVal = line_data[i];    
  }
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (min_bits);
    }
    else if (line_data[i] == 1) {
      return_val += min_bits;
    }
    else if (line_data[i] == maxVal){
      return_val += min_bits;
    }
    else{
      return_val += (min_bits+reg_bits);
    }
  }
  return_val += reg_bits; // for maxVal
    
  return return_val;
}


uns64 zero_runlength_val(uns64* line_data, int num_lines){
  int min_bits = 1;
  uns64 reg_bits = 14;
  uns64 zero_bits = 6;
  int in_z_chunk = 0;

  uns64 return_val =0;  
  
  for(int i=0;i<num_lines;i++){

    //Zero chunk starting
    if( (line_data[i] == 0)  && (in_z_chunk == 0) ){
      in_z_chunk = 1 ;
    }
    //In middle of zero chunk
    else if((line_data[i] == 0 ) && (in_z_chunk == 1) ){     
      in_z_chunk = 1;     
    }
    //Zero chunk ended
    else if((line_data[i] != 0 ) && (in_z_chunk == 1) ){
      in_z_chunk =0;
      return_val += min_bits+zero_bits ;
      return_val += min_bits+reg_bits ;
    }
    else if( (line_data[i] != 0)  && (in_z_chunk == 0) ){
      return_val += min_bits+reg_bits ;
    }
    else{
      ASSERTM(0,"Something wrong");
    }
  }

  if((line_data[num_lines -1] == 0 ) && (in_z_chunk == 1) ){     
    return_val += min_bits+zero_bits ;    
  } 
  
  return return_val;
}


// Returns maximum repeating element in arr[0..n-1].
uns64 maxFreqVal(uns64* arr, int num_elements){

  int maxFreqIndex = 0;
  int maxFreq = 0;
  
  for(int i=0; i < num_elements; i++){
    int freq_a_i =0;
    if(arr[i] != 0){
      for(int j=0 ; j < num_elements; j++){
        if(arr[j] == arr[i])
          freq_a_i++;
      }
      if(freq_a_i > maxFreq){
        maxFreq = freq_a_i;
        maxFreqIndex = i;
      }
    }
  }
  // Return index of the maximum element
  return arr[maxFreqIndex];  
} 


uns64 bdi_7bit_maxval_dedicated(uns64* line_data, int num_lines){
  uns64 return_val=0;  

  uns64 base_bits = 14;
    
  uns64 imm_bits = 14;
  
  uns64 delta_bits = 7; 
 
  uns64 maxVal = 0;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > maxVal)
      maxVal = line_data[i];    
  }
  
  return_val += 462; //50 bits for major, 14 bits for minor base, 7 bits per minor offset 
  
  for(int i=0;i<num_lines;i++){
    if( !( (long)line_data[i] > ( (long)maxVal - ((long)1<<delta_bits) ) ) ){
      return_val += (imm_bits);
    }
  }
    
  return return_val;
  
}

uns64 bdi_5bit_maxval_2base(uns64* line_data, int num_lines){
  uns64 return_val=0;  

  uns64 base_bits = 14;

  uns64 zero_ind_bits = 2; //10
    
  uns64 imm_bits = 14;
  uns64 imm_ind_bits = 2;//11
  
  uns64 delta_bits = 5; 
  uns64 delta_ind_bits = 2;//00 or 01
 
  uns64 maxVal = 0;
  uns64 minVal = -1;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > maxVal)
      maxVal = line_data[i];
    if((line_data[i] < minVal) && (line_data[i] > 0) )
      minVal = line_data[i];    
  }

  return_val += base_bits; //base is maxVal
  return_val += base_bits; //base is minVal
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (zero_ind_bits);
    }
    else if ( (long)line_data[i] > ( (long)maxVal - ((long)1<<delta_bits) ) ) {
      return_val += (delta_bits+ delta_ind_bits);
    }
    else if ( (long)line_data[i] < ( (long)minVal + ((long)1<<delta_bits) ) ) {
      return_val += (delta_bits+ delta_ind_bits);
    }
    else{
      return_val += (imm_bits+imm_ind_bits);
    }
  }
    
  return return_val;
  
}


uns64 bdi_5bit_maxval(uns64* line_data, int num_lines){
  uns64 return_val=0;  

  uns64 base_bits = 14;

  uns64 zero_ind_bits = 2; //10
    
  uns64 imm_bits = 14;
  uns64 imm_ind_bits = 2;//11
  
  uns64 delta_bits = 5; 
  uns64 delta_ind_bits = 1;//0
 
  uns64 maxVal = 0;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > maxVal)
      maxVal = line_data[i];    
  }

  return_val += base_bits; //base is maxVal
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (zero_ind_bits);
    }
    else if ( (long)line_data[i] > ( (long)maxVal - ((long)1<<delta_bits) ) ) {
      return_val += (delta_bits+ delta_ind_bits);
    }
    else{
      return_val += (imm_bits+imm_ind_bits);
    }
  }
    
  return return_val;
  
}


uns64 bdi_3bit_maxval(uns64* line_data, int num_lines){
  uns64 return_val=0;  

  uns64 base_bits = 14;

  uns64 zero_ind_bits = 2; //10
    
  uns64 imm_bits = 14;
  uns64 imm_ind_bits = 2;//11
  
  uns64 delta_bits = 3; 
  uns64 delta_ind_bits = 1;//0
 
  uns64 maxVal = 0;

  for(int i=0;i<num_lines;i++){
    if(line_data[i] > maxVal)
      maxVal = line_data[i];    
  }

  return_val += base_bits; //base is maxVal
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (zero_ind_bits);
    }
    else if ( (long)line_data[i] > ( (long)maxVal - ((long)1<<delta_bits) ) ) {
      return_val += (delta_bits+ delta_ind_bits);
    }
    else{
      return_val += (imm_bits+imm_ind_bits);
    }
  }
    
  return return_val;
  
}

uns64 bdi_3bit_maxFreqVal(uns64* line_data, int num_lines){
  uns64 return_val=0;  

  uns64 base_bits = 14;

  uns64 zero_ind_bits = 2; //10
    
  uns64 imm_bits = 14;
  uns64 imm_ind_bits = 2;//11
  
  uns64 delta_bits = 3; 
  uns64 delta_ind_bits = 1;//0
 
  uns64 maxFreqValue = maxFreqVal(line_data, num_lines);

  return_val += base_bits; //base is maxFreqVal
  
  for(int i=0;i<num_lines;i++){
    
    if(line_data[i] == 0){
      return_val += (zero_ind_bits);
    }
    else if ( ( (long)line_data[i] >= ( (long)maxFreqValue - ((long)1<<delta_bits)/2 ) ) &&  ((long)line_data[i] < ( (long)maxFreqValue + ((long)1<<delta_bits)/2 -1 ) )  )  {
      return_val += (delta_bits+ delta_ind_bits);
    }
    else{
      return_val += (imm_bits+imm_ind_bits);
    }
  }
    
  return return_val;
  
}

uns64 zlib_comp(uns64* line_data, int num_lines){
  uns64 return_val=0;  
  uLongf destLen = MAXINPUTLINESIZE * 8;
  uLong sourceLen = (num_lines * MINOR_CTR_BITLEN)/8;

  
  int byte_buf_index = 0;
  uns64 curr_long_bits = 0;
  uns64 tot_line_bits = 0;
  
  uns64 bytef_bits = sizeof(Bytef)*8;
  uns64 used_bits = 0;
  uns64 curr_iter_bits = bytef_bits;

  zlib_src[byte_buf_index] = 0;
  
  for(int i=0;i<num_lines;i++){
    uns64 curr_data = line_data[i];
    curr_long_bits = 0;
    curr_iter_bits = (bytef_bits-used_bits);
    curr_long_bits += curr_iter_bits;

    //Copy bytewise as much as you can until you hit the end of the MINOR_CTR_BITLEN
    while(curr_long_bits <= MINOR_CTR_BITLEN){
      //Consume curr_iter_bits
      zlib_src[byte_buf_index] |=  ( curr_data & ( ((uns64)1<<curr_iter_bits) - 1) ) << used_bits;
      tot_line_bits += curr_iter_bits;
      //printf("(Loop)Tot_line_bits : %d\n", tot_line_bits); 

      //Push consumed curr_data out
      curr_data = curr_data >> curr_iter_bits;
      byte_buf_index++;

      //Reset curr_iter_bits
      used_bits = 0;
      curr_iter_bits = (bytef_bits-used_bits);      
      curr_long_bits += curr_iter_bits;
      
      //ready next byte
      zlib_src[byte_buf_index] = 0;
      
    }

    //Eat up the remaining MINOR_CTR_BITLEN as space from the next byte in zlib_src
    uns64 minor_bits_left = MINOR_CTR_BITLEN - (curr_long_bits - bytef_bits);
    zlib_src[byte_buf_index] = curr_data & ( ((uns64)1<<minor_bits_left) - 1);
    tot_line_bits += minor_bits_left;
    //printf("(Leftover)Tot_line_bits : %d\n", tot_line_bits); 
    used_bits = minor_bits_left;    
  }

  ASSERTM( tot_line_bits == num_lines * MINOR_CTR_BITLEN, "Total packed bits is not matching with the expected num_lines * bits per counter");
            
  compress (zlib_dest, &destLen, zlib_src, sourceLen);

  //Print the packed data
  /*
    for(int i=0; i < sourceLen; i++){
    printf("%d: "BYTE_TO_BINARY_PATTERN,i, BYTE_TO_BINARY(zlib_src[i]));
    printf("\n");
    }
  */
  return_val = destLen * 8;
  return return_val;
}

uns64 zlib_long_comp(uns64* line_data, int num_lines){
  uns64 return_val=0;  
  uLongf destLen = MAXINPUTLINESIZE * 8;
  int minor_ctr_bits = 16;
  uLong sourceLen = (num_lines * minor_ctr_bits)/8;
  
  int byte_buf_index = 0;
  uns64 curr_long_bits = 0;
  uns64 tot_line_bits = 0;
  
  uns64 bytef_bits = sizeof(Bytef)*8;
  uns64 used_bits = 0;
  uns64 curr_iter_bits = bytef_bits;

  zlib_src[byte_buf_index] = 0;
  
  for(int i=0;i<num_lines;i++){
    uns64 curr_data = line_data[i];
    curr_long_bits = 0;
    curr_iter_bits = (bytef_bits-used_bits);
    curr_long_bits += curr_iter_bits;

    //Copy bytewise as much as you can until you hit the end of the minor_ctr_bits
    while(curr_long_bits <= minor_ctr_bits){
      //Consume curr_iter_bits
      zlib_src[byte_buf_index] |=  ( curr_data & ( ((uns64)1<<curr_iter_bits) - 1) ) << used_bits;
      tot_line_bits += curr_iter_bits;
      //printf("(Loop)Tot_line_bits : %d\n", tot_line_bits); 

      //Push consumed curr_data out
      curr_data = curr_data >> curr_iter_bits;
      byte_buf_index++;

      //Reset curr_iter_bits
      used_bits = 0;
      curr_iter_bits = (bytef_bits-used_bits);      
      curr_long_bits += curr_iter_bits;
      
      //ready next byte
      zlib_src[byte_buf_index] = 0;
      
    }

    //Eat up the remaining minor_ctr_bits as space from the next byte in zlib_src
    uns64 minor_bits_left = minor_ctr_bits - (curr_long_bits - bytef_bits);
    zlib_src[byte_buf_index] = curr_data & ( ((uns64)1<<minor_bits_left) - 1);
    tot_line_bits += minor_bits_left;
    //printf("(Leftover)Tot_line_bits : %d\n", tot_line_bits); 
    used_bits = minor_bits_left;    
  }

  ASSERTM( tot_line_bits == num_lines * minor_ctr_bits, "Total packed bits is not matching with the expected num_lines * bits per counter");
            
  compress (zlib_dest, &destLen, zlib_src, sourceLen);

  //Print the packed data
  /*
    for(int i=0; i < sourceLen; i++){
    printf("%d: "BYTE_TO_BINARY_PATTERN,i, BYTE_TO_BINARY(zlib_src[i]));
    printf("\n");
    }
  */
  return_val = destLen * 8;
  return return_val;
}

uns64 bpc_comp(uns64* line_data, int num_lines){
  uns64 return_val = 0;
  uns64 bpc_num_symbols = 0;
  uns64 bpc_symbol_bits = 0;
  
  bpc_transform(line_data, num_lines, MINOR_CTR_BITLEN, dbx_symbols, dbp_symbols,  &bpc_num_symbols, &bpc_symbol_bits);

  return_val = bpc_symbols_comp(dbx_symbols, dbp_symbols, bpc_num_symbols, bpc_symbol_bits, MINOR_CTR_BITLEN);

  return return_val;
  
}

uns64 best_2base(uns64* line_data, int num_lines){
  uns64 return_val = -10;

  if(bdi_7bit_maxval_dedicated( line_data, num_lines) < return_val)
    return_val = bdi_7bit_maxval_dedicated( line_data, num_lines);
 
  if(bdi_5bit_maxval_2base( line_data, num_lines) < return_val)
    return_val = bdi_5bit_maxval_2base( line_data, num_lines);
   
  if(bdi_3bit_maxval( line_data, num_lines) < return_val)
    return_val = bdi_3bit_maxval( line_data, num_lines);

  if(zero_one_freq_val_compression( line_data, num_lines) < return_val)
    return_val = zero_one_freq_val_compression( line_data, num_lines);
  //  if(zero_one_compression( line_data, num_lines) < return_val)
  //  return_val = zero_one_compression( line_data, num_lines);

  if(zero_val_compression( line_data, num_lines) < return_val)
    return_val = zero_val_compression( line_data, num_lines);

  if(zero_runlength_val( line_data, num_lines) < return_val)
    return_val = zero_runlength_val( line_data, num_lines);

  return_val+=2;
  return return_val;
  
}

uns64 best(uns64* line_data, int num_lines){
  uns64 return_val = -10;
  uns64 bdi_7bit_val = 0;
  
  if(bdi_7bit_maxval_dedicated( line_data, num_lines) < return_val){
    return_val = bdi_7bit_maxval_dedicated( line_data, num_lines);
    bdi_7bit_val = return_val;
  }
  if(bdi_5bit_maxval( line_data, num_lines) < return_val)
    return_val = bdi_5bit_maxval( line_data, num_lines);
   
  if(bdi_3bit_maxval( line_data, num_lines) < return_val)
    return_val = bdi_3bit_maxval( line_data, num_lines);

  if(zero_one_freq_val_compression( line_data, num_lines) < return_val)
    return_val = zero_one_freq_val_compression( line_data, num_lines);
  //  if(zero_one_compression( line_data, num_lines) < return_val)
  //  return_val = zero_one_compression( line_data, num_lines);

  if(zero_val_compression( line_data, num_lines) < return_val)
    return_val = zero_val_compression( line_data, num_lines);

  if(zero_runlength_val( line_data, num_lines) < return_val)
    return_val = zero_runlength_val( line_data, num_lines);

  return_val+=3;

  if((COMP_CTR_MODE ==2) || (COMP_CTR_MODE == 3)){

    //BDI with 512 bits is less than return_val + size of return_val
    if((bdi_7bit_val +3) <= (return_val+6)){
      return (bdi_7bit_val +3);
    }
    else{
      
      return (ceil((double)(return_val)/(double)8)*8+6);
    }
  }
  else {
    return return_val;
  }
}

//Functions for sparse counter compression.
uns64 zero_val_comp(uns64* line_data, int num_lines, int ctr_bits){

  uns64 return_val=0;  

  uns64 min_bits = 1;
  //uns64 ctr_bits = 7;
  
  for(int i=0;i<num_lines;i++){

    if( line_data[i] >= ( ((uns64)1) << ctr_bits)  )
      return 10000000;
  
    if(line_data[i]){
      return_val += (min_bits+ctr_bits);
    }
    else{
      return_val += min_bits;
    }
  }

  return return_val;
}

uns64 zero_runlength_comp(uns64* line_data, int num_lines, int ctr_bits){
  int min_bits = 1;
  //uns64 reg_bits = 14;
  uns64 zero_bits = 4;
  int in_z_chunk = 0;
  int chunk_len = 0;  
  uns64 return_val =0;  
  
  for(int i=0;i<num_lines;i++){

    if( line_data[i] >= ( ((uns64)1) << ctr_bits)  )
      return 10000000;
    
    //Zero chunk starting
    if( (line_data[i] == 0)  && (in_z_chunk == 0) ){
      in_z_chunk = 1 ;
      chunk_len++;
    }
    //In middle of zero chunk
    else if((line_data[i] == 0 ) && (in_z_chunk == 1) ){     
      in_z_chunk = 1;
      chunk_len++;
    }
    //Zero chunk ended
    else if((line_data[i] != 0 ) && (in_z_chunk == 1) ){
      int zero_runlen_cap = ((uns64)1)<<zero_bits ;
      int num_chunks = ceil((double)chunk_len/(double)zero_runlen_cap);
      in_z_chunk =0;
      chunk_len =0;
      
      return_val += num_chunks*(min_bits+zero_bits) ;
      return_val += min_bits+ctr_bits;      
    }
    else if( (line_data[i] != 0)  && (in_z_chunk == 0) ){
      return_val += min_bits+ctr_bits ;
    }
    else{
      ASSERTM(0,"Something wrong");
    }
  }

  //End of the stream, and zero chunk not ended
  if((line_data[num_lines -1] == 0 ) && (in_z_chunk == 1) ){     
    int zero_runlen_cap = ((uns64)1)<<zero_bits ;
    int num_chunks = ceil((double)chunk_len/(double)zero_runlen_cap);
    in_z_chunk =0;
    chunk_len =0;
    
    return_val += num_chunks*(min_bits+zero_bits) ;    
  } 
  
  return return_val;
}



uns64 ptrs_non_zero_comp(uns64* line_data, int num_lines, int ctr_bits){
  int ptr_bits = ceil(log2(num_lines));
  //uns64 reg_bits = 14;
  //uns64 zero_bits = 6;
  //int in_z_chunk = 0;

  uns64 return_val =0;  
  
  for(int i=0;i<num_lines;i++){
    if(line_data[i] >=  (((uns64) 1) << ctr_bits))
      return 1000000;

    if(line_data[i]){
      return_val += (ptr_bits+ctr_bits);
    }    
  }
  
  return return_val;
}

uns64 bpc_comp_new(uns64* line_data, int num_lines, int ctr_bits){
  uns64 return_val = 0;
  uns64 bpc_num_symbols = 0;
  uns64 bpc_symbol_bits = 0;

  ASSERTM(0, "Check if this works with COMPRESSED_MTREE == 8 and Flex Major Ctr");
  for(int i=0; i<num_lines; i++){
    if(line_data[i] >=  (((uns64) 1) << ctr_bits))
      return 1000000;
  }
    
  bpc_transform(line_data, num_lines, ctr_bits, dbx_symbols, dbp_symbols,  &bpc_num_symbols, &bpc_symbol_bits);

  return_val = bpc_symbols_comp(dbx_symbols, dbp_symbols, bpc_num_symbols, bpc_symbol_bits, ctr_bits);

  return return_val;
  
}

uns64 bdi_comp_new(uns64* line_data, int num_lines, int ctr_bits, int add_bits){
  uns64 return_val = ctr_bits;
  uns64 bdi_bits = (CACHE_LINE  * 8 - add_bits) / num_lines; 
  uns64 maxVal= 0;
  
  for(int i=0; i<num_lines; i++){
    if(line_data[i] >=  (((uns64) 1) << ctr_bits))
      return 1000000;

    if(line_data[i] > maxVal)
      maxVal = line_data[i];    
  }

  for(int i=0;i<num_lines;i++){
    if( (long)line_data[i] <= ( (long)maxVal - ((long)1<<bdi_bits) ) ) {
      return 1000000;
    }
    else {
      return_val += bdi_bits;
    }
  }
  
  return return_val;
  
}

uns64 best_comp(uns64* line_data, int num_lines, int ctr_bits, int add_bits){
  uns64 return_val = -10;
  uns64 comp_val = 0;
  if( (comp_val = zero_val_comp( line_data, num_lines, ctr_bits)) < return_val){
    return_val = comp_val;
  }
  //  if( (comp_val = zero_runlength_comp( line_data, num_lines, ctr_bits)) < return_val){
  //    return_val = comp_val;
  //}
  //  if( (comp_val = ptrs_non_zero_comp( line_data, num_lines, ctr_bits)) < return_val){
  //    return_val = comp_val;
  //  }

  
  if( (comp_val = bpc_comp_new( line_data, num_lines, ctr_bits)) < return_val){
    return_val = comp_val;
  }
  
  
  if( (comp_val = bdi_comp_new( line_data, num_lines, ctr_bits, add_bits)) < return_val){
    return_val = comp_val;
  }
  return_val += 2; //For specifying the compression config.
  //  return_val += 3; //For specifying the compression config.
  return return_val; 
}

uns64 ada_dyn0_comp(uns64* line_data, int num_lines, int ctr_bits, int add_bits, int num_major_ctrs_per_cl){
  
  int non_zero_bits = (CACHE_LINE*8)/num_major_ctrs_per_cl - add_bits - num_lines;
  ASSERTM(non_zero_bits > 0, "Subtraction doesnt add up");
  int non_zero_line_bit = 0;
  int num_non_zero_lines =0;

  
  //Check if there are any lines higher than the specified ctr_bits
  for(int i=0; i<num_lines; i++){
    //if(line_data[i] >=  (((uns64) 1) << ctr_bits))
    //return 1000000;
    if(line_data[i] != 0)
      num_non_zero_lines++;
  }

  non_zero_line_bit = non_zero_bits  / num_non_zero_lines;
  if(non_zero_line_bit > 63) //Capping at 63 bits
    non_zero_line_bit = 63;
  
  //Check if there are any lines higher than the possible non_zero_line_bits
  for(int i=0; i<num_lines; i++){
    if(line_data[i] >  (((uns64) 1) << non_zero_line_bit)){
      //printf("Nonzero bits: %d, non_zero_line_bit: %d\n", non_zero_bits, non_zero_line_bit);
      return 1000000;
    }
  }

  return( num_lines + (non_zero_line_bit * num_non_zero_lines));
  
}

uns64 uncomp(uns64* line_data, int num_lines, int ctr_bits, int inc_line_id){

  //Check if incr. line is compressible
  if(line_data[inc_line_id] >=  (((uns64) 1) << ctr_bits))
    return 1000000;


  for(int i=0; i< num_lines; i++){
    if(line_data[i] >=  (((uns64) 1) << ctr_bits))
      return 1000000;
  }
  
  return (num_lines * ctr_bits);

}

uns64 try_comp(uns64* line_data, int num_lines, int ctr_bits, int add_bits, int inc_line_id, int comp_mode, int num_major_ctr_per_cl){

  if(comp_mode == 0) //ada_dyn0
    return ada_dyn0_comp(line_data, num_lines, ctr_bits, add_bits, num_major_ctr_per_cl);
 
  else if(comp_mode == 1) //bpc
    return bpc_comp_new( line_data, num_lines, ctr_bits);

  //else if(comp_mode == 1) //no_bpc
  //return 1000000;

  else if(comp_mode == 2) //uncompressed
    return uncomp(line_data, num_lines, ctr_bits, inc_line_id);

  //  else if(comp_mode == 2) //bdi
  //    return bdi_comp_new(line_data, num_lines, 7,add_bits);

  else{
    ASSERTM(0, "Comp Attempt Not supported");
  }
}

uns64 try_rebase(uns64* major_ctr, uns64* line_data, int num_lines, uns64 max_major_ctr_val){

  uns64 min_minorctr_val = 1000000000;
  for(int i=0; i<num_lines; i++){
    if(line_data[i] < min_minorctr_val)
      min_minorctr_val = line_data[i];
  }

  //If minimum minor ctr val is > 0 and incrementing the major ctr will not cause it to overflow.
  if((min_minorctr_val > 0) && ((*major_ctr + min_minorctr_val) <= max_major_ctr_val) ){
    //Potential to rebase exists.
    *major_ctr += min_minorctr_val;
    for(int i=0; i<num_lines; i++){
      line_data[i] = line_data[i] - min_minorctr_val;
    }
    return 1;
  }
  else {
    //No rebasing possible.
    return 0;
  }   
}




/* compression.c ends here */
