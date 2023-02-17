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

void guards_G00(const program_t *p) {
  assert(p);
  block_t *b = program_first(p);

  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    setKnownFeed(b);
    set_sisf(b);
    b = block_next(b);
  }
}

void s1s2_ACC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  // calculating s1 and s2 with the constraint of only ACCELERATION
  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    forwardAcc_s1s2(b, MAX_acc, block_F(b), si, block_sf(b));
    b = block_next(b);
  }
}

void s1s2_DEC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_last(p);

  // calculating s1 and s2 with the constraint of only DECELERATION
  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    backwardDec_s1s2(b, MAX_acc, block_F(b), si, block_sf(b));
    b = block_prev(b);
  }
}

void s1s2_ordering(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    recompute_s1s2(b, MAX_acc, block_FS(b), block_F(b), block_FE(b), si,
                   block_s1(b), block_s2(b), block_sf(b));
    b = block_next(b);
  }
}

void feed_ACC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  // calculating admissible velocities with the constraint of only ACCELERATION
  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    recompute_feed_ACC(b, MAX_acc, si, block_sf(b));
    b = block_next(b);
  }
}

void feed_DEC(const program_t *p, data_t MAX_acc, data_t si) {
  assert(p);

  block_t *b = program_last(p);

  // calculating admissible velocities with the constraint of only DECELERATION
  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    recompute_feed_DEC(b, MAX_acc, si, block_sf(b));
    b = block_prev(b);
  }
}

void timer_LA(const program_t *p, data_t MAX_acc) {
  assert(p);

  block_t *b = program_first(p);

  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    timings(b, MAX_acc, block_FS(b), block_F(b), block_FE(b), block_s1(b),
            block_s2(b));
    b = block_next(b);
  }
}

void set_path_name(const program_t *p, data_t si) {
  assert(p);

  block_t *b = program_first(p);

  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    path_name(b, block_FS(b), block_F(b), block_FE(b), si, block_s1(b),
              block_s2(b), block_sf(b));
    b = block_next(b);
  }
}

void correct_ACC_DEC(const program_t *p) {
  assert(p);

  block_t *b = program_first(p);

  while (b && (block_type(b) > RAPID && block_type(b) < NO_MOTION)) {
    reshapeAccDec(b, block_FS(b), block_F(b), block_FE(b));
    b = block_next(b);
  }
}

void totalTime_G00_G00(const program_t *p) {
  assert(p);
  // start from second block: the first one will be RAPID in the gcode provided
  block_t *b = program_first(p);
  data_t TOT = 0.0;
  int nG00 = 0; // takes into account the number of blocks between 2 G00 blocks
  int idx = 0;

  while (idx < program_length(p) - 1) {
    // next block is a RAPID or block_actFeed(b) = 0 or block_next(b) = NULL
    // or Alpha > 45°
    if (block_FE(b) == 0.0) {
      TOT += block_dt(b);
      nG00++;
      // Loop for reshaping the velocities of blocks between 2 G00 blocks
      for (int j = 1; j < nG00; j++) {
        reshapeFeed(b, block_FS(b), block_F(b), block_FE(b), TOT, 0);
        b = block_prev(b);
      }
      reshapeFeed(b, block_FS(b), block_F(b), block_FE(b), TOT, 1);
      idx++;
    } else if (block_type(b) == RAPID) {
      b = block_next(b);
      idx++;
    }
    // neither RAPID nor the next block is a RAPID
    else {
      TOT += block_dt(b);
      b = block_next(b);
      nG00++;
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
  guards_G00(p);

  // ========================== 2° and 3° Step =============================
  // ***     Forward and Backward calculations (4 notable points)        ***

  // *** Recomputation of admissible velocities (unfeasible final or initial
  // velocities) and then of s1, s2 for each block ***
  feed_ACC(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // ----- Possible change of fs, fe -----
  feed_DEC(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // ----- Possible change of fs, fe -----
  s1s2_ACC(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // -----      Change of s1, s2     -----

  s1s2_DEC(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // -----      Change of s1, s2     -----

  // ------------------    Can happen that s2 <= s1 (WRONG)   ----------------
  s1s2_ordering(p, MAX_acc, si);
  //                  ||
  //                  ||
  //                  \/
  // ----- Possible change of s1 and s2 and it is set a value for v_star -----
  //                         (if it's A-D or  D-A)                       -----

  // *** Set the path name for each and every block ***
  set_path_name(p, si);

  // ========================== 4° Step ====================================
  // *** Change variable from curvilinear abscissa (s) to time (t) ***
  timer_LA(p, MAX_acc);

  // ========================== 5° Step ====================================
  // ***                      Reshaping for D.T.                         ***
  totalTime_G00_G00(p);
  //                                   ||
  //                                   ||
  //                                   \/
  // ----- Change of fs, fm, fe and of v_star (if it's A-D or D-A) -----

  // set rescaled acceleration and decelaration
  correct_ACC_DEC(p);
}