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

// Scalar Product between 2 vectors (P1P2 and P2P3) 
data_t dot_product(const point_t *p1, const point_t *p2, const point_t *p3);

// Compute the cosine of the angle between each pair of consecutive blocks 
data_t cosAlpha(const block_t *b);

// Initial velocity of current block
data_t initialVel(const block_t *b);

// Maintenance velocity of current block
data_t maintenanceVel(const block_t *b);

// Final velocity of current block
data_t finalVel(const block_t *b);

// setting fs, fm and fe for current block and s0 and sf
void setFeed_sisf(const block_t *b);

// ===================== calculating the 4 notable points ======================
void forwardAcc(const block_t *b, data_t MAX_acc, data_t vi, data_t vm, data_t vf, data_t si, data_t sf);

void backwardDec(const block_t *b, data_t MAX_acc, data_t vi, data_t vm, data_t vf, data_t si, data_t sf);

void recompute_s1s2(const block_t *b, data_t MAX_acc, data_t vi, data_t v_star,data_t vf, data_t si, data_t sf);

// =============================================================================

// calculate timings t1, t2, tf for ranges [si, s1], [s1, s2], [s2, sf]
void timings(const block_t *b, data_t MAX_acc, data_t vi, data_t v_star, data_t vf, data_t s1, data_t s2);

// rescaling velocities
void reshape(const block_t *b);

// calculate lambda function to be used when doing Interpolation
data_t block_lambda_LA(const block_t *b, data_t t, data_t *v);

#endif // BLOCK_LA_H