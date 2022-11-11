//   ____                                      
//  |  _ \ _ __ ___   __ _ _ __ __ _ _ __ ___  
//  | |_) | '__/ _ \ / _` | '__/ _` | '_ ` _ \
//  |  __/| | | (_) | (_| | | | (_| | | | | | |
//  |_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|
//                   |___/                     
// C-CNC progam interface

#ifndef PROGRAM_H
#define PROGRAM_H

#include "defines.h"
#include "block.h"
#include "machine.h"

//   _____                      
//  |_   _|   _ _ __   ___  ___ 
//    | || | | | '_ \ / _ \/ __|
//    | || |_| | |_) |  __/\__ \
//    |_| \__, | .__/ \___||___/
//        |___/|_|              

// Opaque structure
typedef struct program program_t;


//   _____                 _   _                 
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE ===================================================================

// create a new program from the given filename
program_t *program_new(const char *filename);

// deallocate
void program_free(program_t *program);

// print a program description
void program_print(const program_t *program, FILE *output);

// PROCESSING ==================================================================

// parse the program: reading one line at a time and for creating a linked list of blocks (stored internally in the program object class) representing the program by taking the file
// return either EXIT_SUCCESS or EXIT_FAILURE
int program_parse(program_t *program, machine_t *cfg);

// linked-list navigation functions
block_t *program_next(program_t *program); // to get/load the next block / programm to be executed on the machine
void program_reset(program_t *program); //resetting the program back to the very first block


// GETTERS =====================================================================

char *program_filename(const program_t *p);
size_t program_length(const program_t *p); //gives the n° of blocks of a program
block_t *program_current(const program_t *p); //returns the current block
block_t *program_first(const program_t *p); // return the VF block
block_t *program_last(const program_t *p); // return the VL block



#endif // end double inclusion guard
