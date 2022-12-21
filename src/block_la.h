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

#include "defines.h"
#include "block.h"

// ============================= ALGORITHMS ====================================

// Scalar Product between 2 CONSECUTIVE vectors given the respective initial and final point for each vector
data_t dot_product(const point_t *p1, const point_t *p2, const point_t *p3);

// Compute the cosine of the angle between each pair of consecutive blocks 
data_t cosAlpha(const block_t *b);

// Initial velocity of current block
data_t initialVel(const block_t *b);

// Maintenance velocity of current block
data_t maintenanceVel(const block_t *b);

// Final velocity of current block
data_t finalVel(const block_t *b);


#endif // BLOCK_LA_H