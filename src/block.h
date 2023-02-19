//   ____  _            _
//  | __ )| | ___   ___| | __
//  |  _ \| |/ _ \ / __| |/ /
//  | |_) | | (_) | (__|   <
//  |____/|_|\___/ \___|_|\_\
//  Block class

#ifndef BLOCK_H
#define BLOCK_H

#include "defines.h"
#include "machine.h"
#include "point.h"

//   _____
//  |_   _|   _ _ __   ___  ___
//    | || | | | '_ \ / _ \/ __|
//    | || |_| | |_) |  __/\__ \
//    |_| \__, | .__/ \___||___/
//        |___/|_|

// Opaque structure representing a G-code block
typedef struct block block_t;

// Block types
typedef enum { RAPID = 0, LINE, ARC_CW, ARC_CCW, NO_MOTION } block_type_t;

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE ===================================================================

block_t *block_new(const char *line, block_t *prev, machine_t *cfg);
void block_free(block_t *b);
void block_print(const block_t *b, FILE *out);

// ALGORITHMS ==================================================================

// Parsing the G-code string. Returns an integer for success/failure
int block_parse(block_t *b);

// Evaluate the value of lambda at a certaint time
// also return speed in the parameter v (current velocity)
data_t block_lambda(const block_t *b, data_t time, data_t *v);

// Interpolate lambda over three axes: returns x, y, z at time t
point_t *block_interpolate(const block_t *b,
                           data_t lambda); // linear and arc interpolation

// ACCESSORS ===================================================================

// GETTERS

data_t block_length(const block_t *b);
data_t block_dtheta(const block_t *b); // for Non linear trajectories
data_t block_dt(const block_t *b);
data_t block_r(const block_t *b);
block_type_t block_type(const block_t *b);
char *block_line(const block_t *b);
size_t block_n(const block_t *b);
point_t *block_center(const block_t *b);
block_t *block_next(const block_t *b);
point_t *block_target(const block_t *b);

// FOR LOOK AHEAD ==============================================================
block_t *block_prev(const block_t *b);
machine_t *block_machine(const block_t *b);
data_t block_actFeed(const block_t *b);
data_t block_FS(const block_t *b);
data_t block_F(const block_t *b);
data_t block_FE(const block_t *b);
data_t block_si(const block_t *b);
data_t block_s1(const block_t *b);
data_t block_s2(const block_t *b);
data_t block_sf(const block_t *b);
data_t block_len(const block_t *b);
data_t block_v_star(const block_t *b);
data_t block_dt_1(const block_t *b);
data_t block_dt_m(const block_t *b);
data_t block_dt_2(const block_t *b);
data_t block_acc(const block_t *b);
data_t block_dec(const block_t *b);
data_t block_k(const block_t *b);
char *block_path_name(const block_t *b);

// SETTERS
void block_set_FS(block_t *b, data_t value);
void block_set_F(block_t *b, data_t value);
void block_set_FE(block_t *b, data_t value);
void block_set_si(block_t *b, data_t value);
void block_set_s1(block_t *b, data_t value);
void block_set_s2(block_t *b, data_t value);
void block_set_sf(block_t *b, data_t value);
void block_set_v_star(block_t *b, data_t value);
void block_set_dt_1(block_t *b, data_t value);
void block_set_dt_m(block_t *b, data_t value);
void block_set_dt_2(block_t *b, data_t value);
void block_set_dt(block_t *b, data_t value);
void block_set_length(block_t *b, data_t value);
void block_set_acc(block_t *b, data_t value);
void block_set_dec(block_t *b, data_t value);
void block_set_k(block_t *b, data_t value);
void block_set_path(const block_t *b, char *desc);

#endif // BLOCK_H