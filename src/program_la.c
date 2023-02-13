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
#include "block.h"
#include "block_la.h"
#include "program.h"

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

  while (b) {
    setFeed_sisf(b);
    b = block_next(b);
  }
}

void program_LA_execution(const program_t *p, machine_t *cfg) {
  assert(p && cfg);
  block_t *b = program_first(p);
  data_t MAX_acc = machine_A(cfg);
  data_t tq = machine_tq(cfg);

  // setting the known velocities
  guards_G00(p);

  while (b) {
    data_t vi = block_FS(b);
    data_t vm = block_FM(b);
    data_t vf = block_FE(b);
    data_t si = block_si(b);
    data_t sf = block_sf(b);
    data_t v_star = block_v_star(b);

    // computation of s1 and s2 using Forward and Backward step
    forwardAcc(b, MAX_acc, vi, vm, vf, si, sf);
    backwardDec(b, MAX_acc, vi, vm, vf, si, sf);

    // so Now I need to recalculate s1, s2 for each block because can happen
    // that s2 < s1 (WRONG)
    recompute_s1s2(b, MAX_acc, vi, v_star, vf, si, sf);

    // change variable from curvilinear abscissa (s) to time (t)
    timings(b, MAX_acc, vi, v_star, vf, block_s1(b), block_s2(b));

    // reshaping for D.T.
    reshape(b);

    b = block_next(b);
  }
}
