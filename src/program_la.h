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

// compute s1, s2 when only accelerating
void s1s2_ACC(const program_t *p, data_t MAX_acc, data_t si);

// compute s1, s2 when only decelerating
void s1s2_DEC(const program_t *p, data_t MAX_acc, data_t si);

// compute fs, fe when only accelerating
void feed_ACC(const program_t *p, data_t MAX_acc, data_t si);

// compute fs, fe when only decelerating
void feed_DEC(const program_t *p, data_t MAX_acc, data_t si);

// compute s1, s2 in case A-D, D-A, A and D (also if fm not reached)
void s1s2_ordering(const program_t *p, data_t MAX_acc, data_t si);

// set the path name for all the blocks in program
void set_path_name(const program_t *p, data_t si);

// timer of whole program
void timer_LA(const program_t *p, data_t MAX_acc);

// compute the total time between 2 rapid blocks and do the reshaping of speeds
void totalTime_G00_G00(const program_t *p);

// set the new acceleration / deceleration due to velocities reshaping
void correct_ACC_DEC(const program_t *p);

void program_LA_execution(const program_t *p, machine_t *cfg);

#endif // PROGRAM_LA_H
