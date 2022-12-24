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

#define S_LENGTH 4

#include "defines.h"
#include "block.h"

// ============================= ALGORITHMS ====================================

// Scalar Product between 2 CONSECUTIVE vectors given the respective initial and final point for each vector
data_t dot_product(const point_t *p1, const point_t *p2, const point_t *p3);

//ordering a vector of numbers
void ascending(data_t *vector, data_t size);

// Compute the cosine of the angle between each pair of consecutive blocks 
data_t cosAlpha(const block_t *b);

// Initial velocity of current block
data_t initialVel(const block_t *b);

// Maintenance velocity of current block
data_t maintenanceVel(const block_t *b);

// Final velocity of current block
data_t finalVel(const block_t *b);

// setting vi, vm and vf for current block and s0 and sf
void settingVel_sisf(const block_t *b);

// ===================== calculating the 4 notable points ======================
void s1s2_Acc(const block_t *b, data_t MAX_acc, data_t vi, data_t vm, data_t vf, data_t si, data_t sf);

void s1s2_Dec(const block_t *b, data_t MAX_acc, data_t vi, data_t vm, data_t vf, data_t si, data_t sf);
// =============================================================================

// calculate timings t1, t2, tf for ranges [si, s1], [s1, s2], [s2, sf]
void timings(const block_t *b, data_t MAX_acc, data_t vi, data_t vm, data_t vf,
              data_t si, data_t s1, data_t s2, data_t sf);

// compute k and the rescale velocities
void reshaping(const block_t *b, data_t tf, data_t tq);

#endif // BLOCK_LA_H