//   ____                                      
//  |  _ \ _ __ ___   __ _ _ __ __ _ _ __ ___  
//  | |_) | '__/ _ \ / _` | '__/ _` | '_ ` _ \
//  |  __/| | | (_) | (_| | | | (_| | | | | | |
//  |_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|
//                   |___/                     
//   _                _               _                    _ 
//  | |    ___   ___ | | __      __ _| |__   ___  __ _  __| |
//  | |   / _ \ / _ \| |/ /____ / _` | '_ \ / _ \/ _` |/ _` |
//  | |__| (_) | (_) |   <_____| (_| | | | |  __/ (_| | (_| |
//  |_____\___/ \___/|_|\_\     \__,_|_| |_|\___|\__,_|\__,_|
//
// Add here program-related functions for implementing the look-ahead behaviour

#ifndef PROGRAM_LA_H
#define PROGRAM_LA_H

#include "defines.h"
#include "program.h"
#include "block_la.h"

// * For example:
// * int program_look_ahead(program_t *p);

// setting speed to 0 if vm_i = 0 or if this is the very last block
void settingVel_Zero(const program_t *p);

// check for G00 consecutive blocks and set speed to 0
void guards_G00(const program_t *p);


#endif // PROGRAM_LA_H
