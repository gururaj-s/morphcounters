/*                               -*- Mode: C -*- 
 * Copyright (C) 2017, Gururaj Saileshwar
 * 
 * Filename: filereader.c
 * Description: 
 * Author: Saileshwar
 * Created: Sun Oct  8 17:06:05 2017 (-0400)
 * Last-Updated: Mon Oct 30 17:54:55 2017 (-0400)
 *     Update #: 24
 */

/* Commentary: 
 * 
   * 
   * 
   */

/* Code: */

#include "filereader.h"

char* get_line_from_file(FILE** tif, int numc, char** read_buffer, int* read_buf_index,  int* valid_buf_length, char* scratch_buffer){
  char* curr_buf = read_buffer[numc];
  FILE* curr_file = tif[numc];
  int* curr_index = &(read_buf_index[numc]);
  int* curr_buf_length = &(valid_buf_length[numc]);
  char* last_token, end_last_token, next_token, end_next_token;
  int next_token_valid = -1, num_bytes_read = -1;
  char* end_token = NULL;
  char saved_string [MAXTRACELINESIZE];
  saved_string[0] = '\0';
  
  //1. See if the existing chunk has any remaining tokens.
  if(*curr_buf_length <= 0 )
           next_token_valid = 0;
         else{
           //Get next token
           ASSERTM((*curr_index >= 0 ) && (*curr_index < *curr_buf_length), "Making sure we dont access outside valid buffer");
           *curr_index += strlen(&curr_buf[*curr_index]); //curr_index points to \0 after the last token.
           (*curr_index)++; //curr_index now points to beginning of next token

           //Ascertain the next token is valid:
           if(*curr_index >= *curr_buf_length)
             next_token_valid = 0;
           else if(strlen(&curr_buf[*curr_index]) <= 0 )
             next_token_valid = 0;
           else{ //Rest of the buff is still valid.
             end_token = strchr(&curr_buf[*curr_index], '\n');
             if(end_token != NULL){ //Found a token
               next_token_valid = 1;
               *end_token = '\0';
             }
             else{ //End of string reached without a token being found
               //Save the rest of the string:
               strcpy(saved_string, &curr_buf[*curr_index]);
               next_token_valid = 0;
             }
           }
         }
  ASSERTM(next_token_valid != -1, "Token is either valid (1) or not(0)");

  //2. If token exists, then return the string
  if(next_token_valid == 1){
    return &(curr_buf[*curr_index]);
  }
  
  //3. Else:
  // 3.1 Read Next Block
  //Read 4096 bytes
  num_bytes_read = gzread (curr_file, curr_buf, (BUFFSIZE) - 1);
  if(num_bytes_read == 0){ //Reached EOF, time to rewind
    ASSERTM(gzeof(curr_file), "Read 0 bytes, and not EOF");
    gzrewind(curr_file);
    num_bytes_read = gzread (curr_file, curr_buf, (BUFFSIZE) - 1);
    ASSERTM(num_bytes_read > 0,"Rewinded but still no bytes read" );
  }
  curr_buf[num_bytes_read] = '\0';
  *curr_buf_length = num_bytes_read;

  // 3.2 Get first token.
  *curr_index = 0;
  end_token = strchr(&curr_buf[*curr_index], '\n');
  if(end_token != NULL){ //Found a token
    next_token_valid = 1;
    *end_token = '\0';
  } else{
    ASSERTM(0, "Should have found a token in a newly read block");
  }
  
  // 3.3 Append saved text to token and return the string
  strcpy(scratch_buffer,"");
  strcat(scratch_buffer,saved_string);
  strcat(scratch_buffer,&curr_buf[*curr_index]);
  return scratch_buffer;
}

/*

//If current block is valid and there are more bytes to be consumed, try getting next token
if(*valid_buf_length != -1){
//Try getting next token
last_token = &curr_read_buffer[curr_buf_index];
//Get next line (till \n)
end_last_token = last_token + strlen(last_token);
ASSERTM(*end_last_token == '\0', "End of Last token should be null");

next_token = end_last_token + 1;
end_next_token = strchr(next_token, '\n');
    
if (end_next_token) *end_next_token = '\0';  // temporarily terminate the current line
    
}
else {
//First time  

}


*/
trace_line_struct read_trace_line_1rec (FILE* filehandle){
  
  trace_line_struct return_traceline;

  int bytes_read = gzread((gzFile) filehandle, &return_traceline, sizeof(trace_line_struct));

  if(bytes_read != sizeof(trace_line_struct)){
    gzrewind(filehandle);
    bytes_read = gzread(filehandle, &return_traceline, sizeof(trace_line_struct));
    ASSERTM(bytes_read == sizeof(trace_line_struct),"Rewind is not working");
  }

  return return_traceline;   
                          
}

int write_trace_line_struct(gzFile filehandle, unsigned long long int line_nonmemops, char line_read_write, long long int trace_addr){

  trace_line_struct next_traceline = {line_nonmemops,line_read_write,trace_addr};

  return gzwrite(filehandle, &next_traceline,sizeof(trace_line_struct));

}


/* filereader.c ends here */
