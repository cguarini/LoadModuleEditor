///lmedit.c
///Edits the binary R2K module through command line
///@author Chris Guarini
///cdg6285@rit.edu
///CS243 Project 3
///5/5/2017

//INCLUDES
//libraries
#include <stdio.h>
#include <stdlib.h>
//headers
#include "exec.h"
#include "lmedit.h"


///readW
///Reads the big endian word and returns it in the correct, 
///little endian order
///@param : word - 32bit word to read
uint32_t readW(uint32_t word){
  uint32_t littleEndian = 0;
  //convert to little endian
  for( int i = 0; i < 4; i++){
    uint32_t temp;
    //get the correct bytes
    temp = ((word << (24 - (8 * i ))));
    temp = (temp >> 24);
    littleEndian += temp;

    if(i != 3){
      //shift left a byte to make room for next byte
      littleEndian = littleEndian << 8;
    }
  }
  return littleEndian;
}

//readHW
///Reads the big endian half word and returns it in the correct
///little endian order
///@param : hw - 16 bit half word to read
uint16_t readHW(uint16_t hw){
  uint16_t littleEndian = 0;
  //convert to little endian

  //get MSByte
  littleEndian = (hw << 8);
  
  //get LSByte
  littleEndian += (hw >> 8);

  return littleEndian;
}


///main
///Fetches the file and reads the arguments and inputs
int main( int argc, char * argv[]){
    if(argc < 1){
      fprintf(stderr, "usage: lmedit file");
      return EXIT_FAILURE;
    }
    //open the file
    FILE * file = fopen(argv[1] , "r+");
    char * filename = argv[1];
    
    int readonly = 0;

    if(!file){//File not found
      file = fopen(argv[1], "r");
      readonly = 1;
      if(!file){//file not read only
        fprintf(stderr, "FILE NOT FOUND : %s\n", filename);
        return EXIT_FAILURE;
      }
    }
    //Read the file
    exec_t table; //create table struct
    //buffers used by fread
    uint8_t  ptr8;  //8 bit buffer
    uint16_t ptr16; //16 bit buffer
    uint32_t ptr32; //32 bit buffer

    //Get magic number
    read16;
    table.magic = readHW(ptr16);
    //Get the version
    read16;
    table.version = readHW(16);

    //Check if file is MIPS R2K OBJ
    if(table.magic != 0xface){
      fprintf(stderr, "error: %s is not an R2K object module (magic number %#6x)\n",
          filename, table.magic);
      return EXIT_FAILURE;
    }

    //Retrieve entry point
    read32;//skip null
    read32;
    table.entry = readW(ptr32);

    if(table.entry){//entry exists, file is load module
      printf("File %s is an R2K load module (entry point %#010x)\n",
          filename, table.entry);
    }
    else{//object module
      printf("File %s is an R2K object module\n", filename);
    }

    //Version number / Date
    uint16_t year, month, day = 0;

    //decode the year
    year = table.version >> (16-7);

    //Decode the month
    month = ((table.version << 7));
    month = (month >> 12);

    //Decode the day
    day = table.version << (11);
    day = day >> (11);

    //Print module version
    printf("Module version: 2%03d/%02d/%02d\n", year, month, day);

    //Grab sections
    for(int i = 0; i < N_EH; i++){
      //Read each section and store in data array in table
      read32;
      table.data[i] = readW(ptr32);
    }
    
    //Print sections
    char * sectionNames[] = {"text", "rdata", "data","sdata", "sbss", 
      "bss", "reloc", "ref", "symtab", "strings"};
    
    //Print sections that exist
    for(int i = 0; i < N_EH; i++){
      if(table.data[i]){
        char str[10];
        if(table.data[i] && i > 6){
          strcpy(str,"entries");
        }
        else if( table.data[i] && i < 6){
          strcpy(str,"bytes");
        }
        printf("Section %s is %d %s long\n", sectionNames[i], table.data[i], str);
        }
      }


    //Create array of starting addresses
    uint32_t startingAddresses[N_EH];
    //Table is load module
    if(table.entry){
      //Set predetermined addresses
      startingAddresses[0] = 0x00400000;
      startingAddresses[1] = 0x10000000;
      startingAddresses[EH_IX_STR] = 0x0;
      for(int i = 2; i < EH_IX_REL; i++){
        //fill up rest of addresses except for table sections
        startingAddresses[i] = startingAddresses[i-1] + table.data[i-1] + 1;
        //multiple of 8
        startingAddresses[i] = (8 - (startingAddresses[i] % 8));
      }
    }

    else{//Object module
      for(int i = 0; i < N_EH; i++){
        //Object module, all addresses are 0
        startingAddresses[i] = 0x00;
      }
    }
    
    //Load file into memory

    //find length of file
    fseek(file, 0L, SEEK_END);
    long filesize = ftell(file);
    filesize += (4 - (filesize % 4));//word align
    uint8_t * memory = (uint8_t * ) malloc(filesize);
   
    //Set to begining of file
    fseek(file, 0L, SEEK_SET);

    //Read file into memory
    fread(memory, 1, filesize, file);
    for(int i = 0; i < filesize; i++){
      printf("%02x ", memory[i]);
      if(!(i%16)){
        printf("\n");
      }
    }
    
    


    //Command loop
    int headerSize = 52;//52 bytes in header block
    int section = 0;//start at section 0, "TEXT"
    int modified = 0; //boolean, default no modifications
    int offset = headerSize; //Offset of section, default to section 0
    while(1){
start:
      printf("section[%s] > ",sectionNames[section]);
      
      char input[100];
      scanf("%[^\n]%*c", input);
      
      //Quit the program
      if(!strcmp(input,"quit")){
        if(modified){
          while(1){
            printf("Discard modifications (yes or no)? ");
            
            char response[100]; 
            scanf("%[^\n]%*c", response);
            
            if(!strcmp(response, "yes")){
              //GOTO TO BREAK NESTED LOOP
              goto endProgram;
            }
            else if(!strcmp(response, "no")){
              break;//break out of quit command
            }
            //else, response neither yes or no
            //repeat loop
          }
        }
        else{
          break;//quit the program
        }
      }
      
      //Get the size of the current section
      if(!strcmp(input, "size")){
        if(section > 5 && section < 9){
          printf("Section %s is %d entries long\n", sectionNames[section],
              table.data[section]);
        }
        else{
          printf("Section %s is %d bytes long\n", sectionNames[section],
              table.data[section]);
        }
      }

      //Change to section
      if(!strcmp(strtok(input," "), "section")){
        char * changeTo = strtok(NULL, " ");
        //loop through the names of the section searching for user input
        for(int i = 0; i < N_EH+1; i++){
          
          if(i == N_EH){
            //Section not found
            fprintf(stderr, "error: '%s' is not a valid section name\n",
                changeTo);
          }

          //Check if BSS or SBSS
          else if(!strcmp(changeTo, "bss") || !strcmp(changeTo, "sbss")){
            //Cant edit BSS or SBSS
            fprintf(stderr, "error: cannot edit %s section\n", changeTo);
            break;
          }

          //Section found
          else if(!strcmp(changeTo, sectionNames[i])){
            section = i;//set to current section
            
            //Find section's offset from start of file
            offset = headerSize;
            for(int j = 0; j < i; j++){
              if(j < 6 || j == EH_IX_STR){
                offset += table.data[j];
              }
              else if(j ==6){
                offset += (8 * table.data[j]);
              }
              else{
                offset += (12 * table.data[j]);
              }  
            }
            //stop searching for name
            break;
          }
        }

      }
      
      //Check for write / read commands
      //First part of command is always a digit
      if(isdigit(input[0])){
        
        command c;//Create command struct for parsing
        //Initialize Struct to defaults
        c.address = 0x00;
        c.count = 0x01;
        c.type = 'w';
        c.newValue = 0xFFFFFFFF;
        char * token;
        int error = 0;
        //Grab the location
        c.address = strtol(input, &token, 10);
        //printf("address %#x\n",c.address);
        while(*token){//Still input to parse
          
          //Repeat token
          if(token[0] == ','){
            if(isdigit(*(token+1))){
              c.count = strtol(token+1, &token, 10);
              //printf("repeat %ld\n",c.count);
            }
          }
          
          //Type token
          else if (token[0] == ':'){
            //Find the type
            c.type = token[1];
            token = token+2;
            //printf("type %c\n",c.type);

          }
          
          else if (token[0] == '='){
            c.newValue = strtol(token+1, &token, 10);
            //printf("newval %#x\n",c.newValue);
          }

          //Invalid Command given, stuck in loop 
          else{
            fprintf(stderr, "Invalid syntax. A[,N][:T][=V]\n");
            error = 1;//status bit so that program won't try to execute command
            break;
          }

        }

        //Finished parsing, start executing command
        //Check for errors
        
        //Check valid address
        if((c.address > table.data[section] + startingAddresses[section]) 
            || c.address < startingAddresses[section]){
          fprintf(stderr, "error: '%#x' is not a valid address\n", c.address);
          error = 1;
        }
        
        //Check if valid count
        if(c.count > table.data[section]){
          fprintf(stderr, "error: '%ld' is not a valid count\n", c.count);
          error = 1;
        }

        //Check if valid type
        if(!(c.type == 'b' || c.type == 'h' || c.type == 'w')){
          fprintf(stderr, "error: '%c' is not a valid type\n", c.type);
          error = 1;
        }

        if(error){
          goto start;
        }

        //Start execution
        //Check if read or write
        if(c.newValue == 0xFFFFFFFF){//Wasn't changed, is a read operation
          //Set amount of bytes to read
          int datasize = 0;
          
          switch (c.type){
            case 'b' :
              datasize = 1;
              break;

            case 'h' :
              datasize = 2;
              break;

            case 'w' :
              datasize = 4;
              break;
          }
        //Loop through count amount of times
          for(int i = 0; i < c.count; i++){
            //Get address
            uint32_t addr = (c.address + (i * datasize));
            //Data buffers
            uint8_t data8=0;
            uint16_t data16=0;
            uint32_t data32=0;
            
            //switch depending on data type
            switch (c.type){
              case 'b' :
                data8 = memory[c.address - startingAddresses[section] + offset + i];
                printf("\t%#010x = %#04x\n", addr, data8);
                break;
              case 'h' : 
                //grab halfword
                data16 = memory[c.address - startingAddresses[section] 
                  + offset + (i * datasize)];
                data16 = data16 << 8;
                data16 += memory[c.address - startingAddresses[section]
                  + offset + (i * datasize) + (1)];
                printf("\t%#010x = %#06x\n", addr, data16);
                break;
              case 'w' :
                //grab word
                for(int j = 0; j < datasize; j++){
                  data32 += memory[c.address - startingAddresses[section] 
                    + offset + (i * datasize) + (j)];
                  if( j < 3){//if not last byte, make room for next byte
                    data32 = data32 << 8;
                  }
                 
                 }                
                printf("\t%#010x = %#010x\n", addr, data32);
                break;
            }//END SWITCH
            
          }//END COUNT LOOP
        }//END OF READ OPERATION

        //WRITE OPERATION
        else{
          //load newvalue into perspective types
          uint8_t val8 = c.newValue;
          uint16_t val16 = c.newValue;
          uint32_t val32 = c.newValue;
          printf("%#x, %#x, %#x", val8, val16, val32);
          //Write for count amount of times
          for(int i = 0; i < c.count; i++){
            switch (c.type) { 
              case 'b' :
                memory[c.address - startingAddresses[section] + offset + (i * 1)] = val8;
                break;
              case 'h' : 
                memory[c.address - startingAddresses[section] + offset + (i * 2)] = val16;
                break;
              case 'w' :
                memory[c.address - startingAddresses[section] + offset + (i * 4)] = val32;
                break;
            }
          }
  


        }

        

      }//END OF COMMAND



    
      //end of commands
    }

endProgram:
    //END OF INSTRUCTIONS
    fclose(file);
    return EXIT_SUCCESS;
}





















