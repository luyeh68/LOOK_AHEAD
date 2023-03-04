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

#include "program_la.h"

//  _   _                 _____          _
// | \ | | _____      __ |  ___|__  __ _| |_ _   _ _ __ ___  ___
// |  \| |/ _ \ \ /\ / / | |_ / _ \/ _` | __| | | | '__/ _ \/ __|
// | |\  |  __/\ V  V /  |  _|  __/ (_| | |_| |_| | | |  __/\__ \
// |_| \_|\___| \_/\_/   |_|  \___|\__,_|\__|\__,_|_|  \___||___/

// =============================================================================
// * int program_look_ahead(program_t *p);

void program_LA_guards_G00(const program_t *p) {
  assert(p);
  block_t *b = program_first(p);

  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_set_sisf(b);

    block_LA_setKnownFeed(b);
    b = block_next(b);
  }
}

void program_LA_s1s2_ACC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  // calculating s1 and s2 with the constraint of only ACCELERATION
  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_forwardAcc_s1s2(b, MAX_acc, block_F(b), si, block_sf(b));
    b = block_next(b);
  }
}

void program_LA_s1s2_DEC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_last(p);

  // calculating s1 and s2 with the constraint of only DECELERATION
  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_backwardDec_s1s2(b, MAX_acc, block_F(b), si, block_sf(b));
    b = block_prev(b);
  }
}

void program_LA_s1s2_ordering(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_recompute_s1s2(b, MAX_acc, block_FS(b), block_F(b), block_FE(b),
                              si, block_s1(b), block_s2(b), block_sf(b));
    b = block_next(b);
  }
}

void program_LA_feed_ACC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  // calculating admissible velocities with the constraint of only ACCELERATION
  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_recompute_feed_ACC(b, MAX_acc, si, block_sf(b));
    b = block_next(b);
  }
}

void program_LA_feed_DEC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_last(p);

  // calculating admissible velocities with the constraint of only DECELERATION
  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_recompute_feed_DEC(b, MAX_acc, si, block_sf(b));
    b = block_prev(b);
  }
}

void program_LA_timer(const program_t *p, data_t MAX_acc) {
  assert(p);

  block_t *b = program_first(p);

  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_timings(b, MAX_acc, block_FS(b), block_F(b), block_FE(b),
                       block_s1(b), block_s2(b));
    b = block_next(b);
  }
}

void program_LA_set_path_name(const program_t *p, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_path_name(b, block_FS(b), block_F(b), block_FE(b), si,
                         block_s1(b), block_s2(b), block_sf(b));
    b = block_next(b);
  }
}

void program_LA_correct_ACC_DEC(const program_t *p) {
  assert(p);

  block_t *b = program_first(p);

  while (b) {
    if (block_type(b) > RAPID && block_type(b) < NO_MOTION)
      block_LA_reshapeAccDec(b, block_FS(b), block_F(b), block_FE(b));
    b = block_next(b);
  }
}

void program_LA_totalTime_G00_G00(const program_t *p) {
  assert(p);
  block_t *b = program_first(p);
  int idx = 1; // for looping over the whole program
  int Nb = 0;  // number of blocks between 2 G00 blocks
  data_t TOT = 0.0;

  while (idx < program_length(p)) {
    block_t *current = b;
    if (block_type(b) != RAPID && block_FE(b) == 0.0) {
      TOT += block_dt(b);
      Nb++;

      // Loop for reshaping the velocities of blocks between 2 G00 blocks
      for (int j = 1; j < Nb; j++) {
        block_LA_reshapeFeed(b, block_FS(b), block_F(b), block_FE(b), TOT, 0);
        b = block_prev(b);
      }
      block_LA_reshapeFeed(b, block_FS(b), block_F(b), block_FE(b), TOT, 1);

      b = block_next(current); // we go on for the other G00 blocks if existing
      idx++;
      Nb = 0;
      TOT = 0.0;
    } else if (block_type(b) == RAPID && block_FE(b) == 0.0) {
      b = block_next(b);
      idx++;
    } else if (block_type(b) != RAPID && block_FE(b) != 0.0) {
      TOT += block_dt(b);
      b = block_next(b);
      Nb++;
      idx++;
    }
  }
}

void program_LA_execution(const program_t *p, machine_t *cfg) {
  assert(p && cfg);
  // Constant Parameters for every block in program p
  data_t si = block_si(program_first(p));
  data_t MAX_acc = machine_A(cfg) * 3600; // [mm/min^2]

  // ============================== 1° Step ==================================
  // ***          Setting the known velocities and spaces                  ***
  program_LA_guards_G00(p);

  // ========================== 2° and 3° Step =============================
  // ***     Forward and Backward calculations (4 notable points)        ***

  // *** Recomputation of admissible velocities (unfeasible final or initial
  // velocities) and then of s1, s2 for each block ***
  program_LA_feed_ACC(p, MAX_acc, si);

  //                  ||
  //                  ||
  //                  \/
  // ----- Possible change of fs, fe -----
  program_LA_feed_DEC(p, MAX_acc, si);

  //                  ||
  //                  ||
  //                  \/
  // ----- Possible change of fs, fe -----
  program_LA_s1s2_ACC(p, MAX_acc, si);

  //                  ||
  //                  ||
  //                  \/
  // -----      Change of s1, s2     -----

  program_LA_s1s2_DEC(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // -----      Change of s1, s2     -----

  // ------------------    Can happen that s2 <= s1 (WRONG)   ----------------
  program_LA_s1s2_ordering(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // ----- Possible change of s1 and s2 and it is set a value for v_star -----
  //                         (if it's A-D or  D-A)                       -----

  // *** Set the path name for each and every block ***
  program_LA_set_path_name(p, si);

  // ========================== 4° Step ====================================
  // *** Change variable from curvilinear abscissa (s) to time (t) ***
  program_LA_timer(p, MAX_acc);

  // ========================== 5° Step ====================================
  // ***                      Reshaping for D.T.                         ***
  program_LA_totalTime_G00_G00(p);

  //                                   ||
  //                                   ||
  //                                   \/
  // ----- Change of fs, fm, fe and of v_star (if it's A-D or D-A) -----

  // set rescaled acceleration and deceleration
  program_LA_correct_ACC_DEC(p);
}