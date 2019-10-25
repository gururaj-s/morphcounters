/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: bpc.c
 * Description: 
 * Author: Saileshwar
 * Created: Tue Oct  3 23:53:26 2017 (-0400)
 * Last-Updated: Wed Oct 25 21:19:24 2017 (-0400)
 *     Update #: 68
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Code: */

#include "bpc.h"
#include "compression.h"

uns64 delta[MAXINPUTLINESIZE];


void bpc_transform(uns64* input_entries, uns64 num_entries, uns64 entry_bits, uns64 dbx_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64 dbp_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64* num_symbols, uns64* symbol_bits ){
  *symbol_bits = num_entries - 1; //63 bit symbol
  *num_symbols = entry_bits +1;

  //delta
  uns64 base = input_entries[0];
  ASSERTM((num_entries %32) == 0, "Num Entries needs to be multiple of 32");
  uns64 num_delta = num_entries -1 ;
  uns64 delta_bits = entry_bits +1;  
  uns64 delta_bits_filter = (((uns64) 1) << delta_bits) - 1; 
  
  for (int i=1; i< num_entries; i++){
    delta[i-1] = input_entries[i] - base;
    base = input_entries[i];
  }

  //Extract delta_bits sized uns64
  for (int i=0; i< num_delta; i++){
    delta[i] = delta[i]  & delta_bits_filter;
  }
  
  //bit-plane transformation
  uns64 num_bp = delta_bits;
  uns64 bp_bits = num_delta;

  //perform the bit-plane transform
  for (int i=0; i<num_bp;i++){
    //build dbp_symbols[i]
    for(int x=0; x< MAX_32_BITMULTIPLE_SYMBOLLEN; x++){
      dbp_symbols[x][i] = 0;
    }
    //dbp_symbols[i] =0;
    for (int j =0; j< num_delta; j++){
      //jth bit of dbp_symbols[i] is ith bit of delta[num_delta - 1 -j]
      //Get ith bit of delta[num_delta - 1 -j]
      uns64 i_bit_delta_j = ( delta[num_delta - 1 - j] &  (((uns64) 1) << i) ) >> i;
      ASSERTM ( i_bit_delta_j <= 1, "single bit number");
      //Put in (j%32) th bit of dbp_symbols[j/32][i]
      dbp_symbols[j/32][i] = dbp_symbols[j/32][i] | (i_bit_delta_j << (j%32));
    }
  }

  
  //print
  /*
    printf("Delta : \n");
    for (int i=0; i< num_delta; i++){
    printf("%d: ",i);
    printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY((char)(delta[i]>>8)));
    printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY((char)(delta[i])));
    printf("\n");
    }
    printf ("\n");

  
    printf("DBP : \n");
    for (int i=num_bp-1; i>=0 ; i--){
    printf("%d: ",i);
    for (int j =3;j>=0;j--){

    for(int x = num_delta/32; x>=0;x--){
    printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY((char)(dbp_symbols[x][i]>>j*8)));
    printf(" ");
    }
    }
    printf("\n");
    }
    printf ("\n");
  */
  
  //bit-wise xor to output dbx symbols
  uns64 num_dbx = num_bp;
  uns64 dbx_bits = bp_bits;
  ASSERTM(num_dbx == *num_symbols, "Math adds up");
  ASSERTM(dbx_bits == *symbol_bits, "Math adds up");

  for(int x = num_delta/32; x>=0;x--){
    uns64 base_bp = dbp_symbols[x][num_bp - 1];
    dbx_symbols[x][num_dbx - 1] = dbp_symbols[x][num_bp - 1];
    for(int i= num_dbx - 2; i >= 0 ; i--){
      dbx_symbols[x][i] =  dbp_symbols[x][i] ^ base_bp;
      base_bp = dbp_symbols[x][i];
    }
  }
  //print
  /*
    printf("DBX : \n");
    for (int i=num_dbx-1; i>=0 ; i--){
    printf("%d: ",i);
    for(int x = num_delta/32; x>=0;x--){  
    for (int j =3;j>=0;j--){
    printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY((char)(dbx_symbols[x][i]>>j*8)));
    printf(" ");
    }
    }
    printf("\n");
    }
    printf ("\n");
  */
  return;
}

uns64 bpc_symbols_comp(uns64 dbx_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64 dbp_symbols[MAX_32_BITMULTIPLE_SYMBOLLEN][MAXINPUTLINESIZE], uns64 num_symbols, uns64 symbol_bits, uns64 entry_bits ){
  uns64 return_val = 0;
  //For the base
  return_val += entry_bits;
  //  printf("BPC Return Vals :\n");

  //printf("Zero run-len: %d\n",((int)ceil(log2(entry_bits -1))) );
  
  int zero_runlen = 0;
  //For each symbol in num_symbols, try the bpc_lookup
  for(int i =0; i<num_symbols ; i++){
    for(int x = symbol_bits/32; x>=0;x--){
      //printf("symbol %d, %llu\n",i,dbx_symbols[i]);
      int symbol_chunk_bits = 32;
      int num_symbol_chunks = ceil(symbol_bits /32.0);
      if(x == symbol_bits / 32)
        symbol_chunk_bits = 31;
      
      if(dbx_symbols[x][i] == 0){
        zero_runlen++;
        continue;
      }

      //current symbol is non-zero, but check if previous symbols were zeros
      if(zero_runlen > 1){
        //multiple symbols were 0s
        //return_val += (2 + ((int)ceil(log2(entry_bits -1))) ); //2b + 3bits
        return_val += (2 + ((int)ceil(log2(entry_bits*num_symbol_chunks -1))) ); //2b + 3bits
        zero_runlen = 0;
      }
      else if (zero_runlen == 1){
        //last symbol only was 0
        return_val += 3;
        zero_runlen = 0;
      }
      else{
        //Last symbol was not 0
      }

      //printf("%d: %llu\n",i,return_val);
      //return_val += bpc_symbol_lookup (dbx_symbols[i], dbp_symbols[i], symbol_bits);
      return_val += bpc_symbol_lookup (dbx_symbols[x][i], dbp_symbols[x][i], symbol_chunk_bits);
      //printf("%d: %llu\n",i,return_val);

      //print Return_val for each chunk
      /*
        for(int j =3; j>= 0; j--){
        printf(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY((char)(dbx_symbols[x][i]>>j*8)));
        
        printf(".");
        }
        printf(" : Return val is:  %d\n", return_val);
      */
    }
  }

  return return_val;
}

uns64 bpc_symbol_lookup (uns64 dbx_symbol, uns64 dbp_symbol, uns64 symbol_bits){
  uns64 return_val = 0;

  //printf("Consec Two 1s %d ", (int)ceil(log2(symbol_bits+1)));
  
  //All 1s
  if(dbx_symbol == (((uns64)1 <<symbol_bits) - 1))
    return_val = 5;
  
  //DBX!=0 but DBP =0
  else if( (dbx_symbol != 0) && (dbp_symbol == 0) )
    return_val = 5;
  
  else if(consec_two_1s(dbx_symbol, symbol_bits) ) {
    //Two 1s
    //printf("**11**");
    return_val = 5+ ((int)ceil(log2(symbol_bits+1)));
  }
  else if(single_1(dbx_symbol, symbol_bits)){
    //Single 1
    //printf("**1**");
    //return_val = 11;
    return_val = 5+ ((int)ceil(log2(symbol_bits+1)));
  }
  else{
    return_val = (symbol_bits + 1);
  }
  return return_val;
}
/* bpc.c ends here */

int consec_two_1s(uns64 symbol, uns64 bits){
  uns64 bit_val = 0;
  int state = 0;
  
  for (int i =0 ; i < bits; i++){
    
    bit_val = symbol & ((uns64) 1);
    symbol = symbol >> 1;

    if( (state == 0) && (bit_val == 0))
      continue;
    else if( (state == 0) && (bit_val == 1) ){
      state = 1;
      continue;
    }
    else if ( (state == 1) && (bit_val == 0) ){
      return 0;
    }
    else if ( (state == 1) && (bit_val == 1) ){
      state = 2;
      continue;
    }
    else if ( (state == 2) && (bit_val == 1) ){
      return 0;
    }
    else if ( (state == 2) && (bit_val == 0) ){
      continue;
    }
  }

  //printf ("\nstate: %d\n",state);
  
  if(state == 2)
    return 1;
  else
    return 0;  
}


int single_1(uns64 symbol, uns64 bits){
  uns64 bit_val = 0;
  int state = 0;
  
  for (int i =0 ; i < bits; i++){
    
    bit_val = symbol & ((uns64) 1);
    symbol = symbol >> 1;

    if( (state == 0) && (bit_val == 0))
      continue;
    else if( (state == 0) && (bit_val == 1) ){
      state = 1;
      continue;
    }
    else if ( (state == 1) && (bit_val == 0) ){
      continue;
    }
    else if ( (state == 1) && (bit_val == 1) ){
      return 0;
    }
  }

  if(state == 1)
    return 1;
  else
    return 0;   

}
