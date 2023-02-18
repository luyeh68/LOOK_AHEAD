//   ____  _            _
//  | __ )| | ___   ___| | __
//  |  _ \| |/ _ \ / __| |/ /
//  | |_) | | (_) | (__|   <
//  |____/|_|\___/ \___|_|\_\

//   _                _               _                    _
//  | |    ___   ___ | | __      __ _| |__   ___  __ _  __| |
//  | |   / _ \ / _ \| |/ /____ / _` | '_ \ / _ \/ _` |/ _` |
//  | |__| (_) | (_) |   <_____| (_| | | | |  __/ (_| | (_| |
//  |_____\___/ \___/|_|\_\     \__,_|_| |_|\___|\__,_|\__,_|
//
// Implement here block-related functions for look-ahead

#ifndef BLOCK_LA_H
#define BLOCK_LA_H

#include "block.h"

// ============================= ALGORITHMS ====================================
// Initial velocity of current block
data_t block_LA_initialFeed(const block_t *b);

// Maintenance velocity of current block
data_t block_LA_maintenanceFeed(const block_t *b);

// Final velocity of current block
data_t block_LA_finalFeed(const block_t *b);

// setting fs, fm and fe for current block
void block_LA_setKnownFeed(const block_t *b);

// setting si and sf
void block_LA_set_sisf(const block_t *b);

// ===================== calculating the 4 notable points ======================
void block_LA_forwardAcc_s1s2(const block_t *b, data_t MAX_acc, data_t vm, data_t si,
                     data_t sf);

void block_LA_backwardDec_s1s2(const block_t *b, data_t MAX_acc, data_t vm, data_t si,
                      data_t sf);

void block_LA_recompute_feed_ACC(const block_t *b, data_t MAX_acc, data_t si, data_t sf);
void block_LA_recompute_feed_DEC(const block_t *b, data_t MAX_acc, data_t si, data_t sf);

void block_LA_recompute_s1s2(const block_t *b, data_t MAX_acc, data_t vs, data_t vm,
                    data_t vf, data_t si, data_t s1, data_t s2, data_t sf);
// =============================================================================
void block_LA_path_name(const block_t *b, data_t vs, data_t vm, data_t vf, data_t si,
               data_t s1, data_t s2,
               data_t sf); // print a description of the block type

// Calculate intervals dt_1, dt_m, dt_2 for [si, s1], [s1, s2], [s2, sf]
void block_LA_timings(const block_t *b, data_t MAX_acc, data_t vs, data_t vm, data_t vf,
             data_t s1, data_t s2);

// Rescaling velocities and accelerations - decelerations
void block_LA_reshapeFeed(const block_t *b, data_t vs, data_t vm, data_t vf,
                 data_t total, int last);

void block_LA_reshapeAccDec(const block_t *b, data_t vs, data_t vm, data_t vf);

// Calculate lambda function to be used when doing Interpolation (in fsm.c)
data_t block_lambda_LA(const block_t *b, data_t vs, data_t vm, data_t vf,
                       data_t t, data_t *v);

#endif // BLOCK_LA_H