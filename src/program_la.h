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

#include "block_la.h"
#include "defines.h"
#include "program.h"

// * For example:
// * int program_look_ahead(program_t *p);

// check for G00 blocks and set speeds to 0.0 where needed
void guards_G00(const program_t *p);

void program_LA_execution(const program_t *p, machine_t *cfg);

#endif // PROGRAM_LA_H
