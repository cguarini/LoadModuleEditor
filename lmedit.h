///lmedit.h
///Header file for lmedit.c
///Reads and edits the binary R2K module through command line
///@author Chris Guarini
///cdg6285@rit.edu
///5/5/2017

//INLCUDES
//libraries
#include <stdio.h>
#include <stdlib.h>
//headers
#include "exec.h"

///MACROS
///read8, read16, read32 : Calls fread to read the file for a buffer
#define read16 fread(&ptr16,2,1,file)
#define read32 fread(&ptr32,4,1,file)
#define read8  fread(&ptr8,1,1,file)

///FUNCTION DECLARATIONS
char * strcpy(char * dest, const char * src);
void * memcpy(void * str1, const void *str2, size_t n);
int strcmp(const char *str1, const char *str2);
char * strtok ( char * str, const char * delimiters);
int isdigit(int c);
long int strtol(const char *str, char **endptr, int base);


///STRUCTS
///Command - Used to parse commands from input
typedef struct command {
  uint32_t  address;//Location to do command
  char  type;//Data size (byte, halfword, word) in bits
  long  count;//how many times to repeat command
  uint32_t  newValue;//New value to set data to
}command;
