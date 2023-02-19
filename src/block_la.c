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

#include "block_la.h"
#include <ctype.h>

//  _   _                 _____          _
// | \ | | _____      __ |  ___|__  __ _| |_ _   _ _ __ ___  ___
// |  \| |/ _ \ \ /\ / / | |_ / _ \/ _` | __| | | | '__/ _ \/ __|
// | |\  |  __/\ V  V /  |  _|  __/ (_| | |_| |_| | | |  __/\__ \
// |_| \_|\___| \_/\_/   |_|  \___|\__,_|\__|\__,_|_|  \___||___/
// ============================================================================

// STATIC FUNCTIONS (for internal use only)
static data_t quantize_LA(data_t time, data_t tq, data_t *dq);
static point_t *point_zero(const block_t *b);
static data_t dot_product(const point_t *p1, const point_t *p2,
                          const point_t *p3);
static data_t cosAlpha(const block_t *b);
//  ____ _____ _____ ____    _
// / ___|_   _| ____|  _ \  / |
// \___ \ | | |  _| | |_) | | |
//  ___) || | | |___|  __/  | |
// |____/ |_| |_____|_|     |_|
// Intermediate velocities

data_t block_LA_maintenanceFeed(block_t *b) {
  assert(b);
  return block_actFeed(b); // [mm/min] -- calculated during PARSING
}

data_t block_LA_finalFeed(block_t *b) {
  assert(b);
  // angle > 45° OR vm,i = 0 ==> 0 speed OR if last segment ==> 0 speed
  if (cosAlpha(b) < sqrt(2) / 2 || block_actFeed(b) == 0.0 ||
      block_type(b) == RAPID || !block_next(b))
    return 0.0;

  return (block_LA_maintenanceFeed(b) +
          block_LA_maintenanceFeed(block_next(b))) /
         2.0 * cosAlpha(b); //[mm/min]
}

data_t block_LA_initialFeed(block_t *b) {
  assert(b);
  return block_prev(b) ? block_LA_finalFeed(block_prev(b))
                       : 0.0; // fs,i = fe,i-1
}

void block_LA_setKnownFeed(block_t *b) {
  assert(b);
  block_set_FS(b, block_LA_initialFeed(b));
  block_set_F(b, block_LA_maintenanceFeed(b));
  block_set_FE(b, block_LA_finalFeed(b));
}

void block_LA_set_sisf(block_t *b) {
  assert(b);
  // Any block can be considered indipendent in its own Reference frame:
  // si
  block_set_si(b, 0.0);
  // sf
  block_set_sf(b, block_length(b));
}

//  ____ _____ _____ ____    ____      _____
// / ___|_   _| ____|  _ \  |___ \    |___ /
// \___ \ | | |  _| | |_) |   __) |____ |_ \
//  ___) || | | |___|  __/   / __/_____|__) |
// |____/ |_| |_____|_|     |_____|   |____/

// ============================== 9 possible cases =============================

// ============================= Accelerations =================================
void block_LA_forwardAcc_s1s2(block_t *b, data_t MAX_acc, data_t vm, data_t si,
                              data_t sf) {
  assert(b);
  if (block_FS(b) <= vm) // Equality for dealing with pure Maintenance
    block_set_s1(b, si + (pow(vm, 2) - pow(block_FS(b), 2)) / (2.0 * MAX_acc));

  if (vm <= block_FE(b)) // Equality for dealing with pure Maintenance
    block_set_s2(b, sf + (pow(vm, 2) - pow(block_FE(b), 2)) / (2.0 * MAX_acc));
}

// ============================= Decelerations =================================
void block_LA_backwardDec_s1s2(block_t *fromLast, data_t MAX_acc, data_t vm,
                               data_t si, data_t sf) {
  assert(fromLast);
  if (block_FS(fromLast) > vm)
    block_set_s1(fromLast, si + (pow(block_FS(fromLast), 2) - pow(vm, 2)) /
                                    (2.0 * MAX_acc));

  if (vm > block_FE(fromLast))
    block_set_s2(fromLast, sf + (pow(block_FE(fromLast), 2) - pow(vm, 2)) /
                                    (2.0 * MAX_acc));
}

void block_LA_recompute_feed_ACC(block_t *b, data_t MAX_acc, data_t si,
                                 data_t sf) {
  assert(b);
  data_t vf_max = sqrt(2 * MAX_acc * (sf - si) + pow(block_FS(b), 2));

  // A -- only acceleration (case of vf_max < vf_nom)
  if (vf_max < block_FE(b)) {
    // Reset new admissible speed to vf_max
    block_set_FE(b, vf_max);

    block_set_FS(block_next(b), vf_max);
  }
}

void block_LA_recompute_feed_DEC(block_t *fromLast, data_t MAX_acc, data_t si,
                                 data_t sf) {
  assert(fromLast);
  data_t vs_max = sqrt(2 * MAX_acc * (sf - si) + pow(block_FE(fromLast), 2));

  // D -- only deceleration (case of vs_max < vs_nom)
  if (vs_max < block_FS(fromLast)) {
    // Reset new admissible speed to vs_max
    block_set_FS(fromLast, vs_max);

    block_set_FE(block_prev(fromLast), vs_max);
  }
}

void block_LA_recompute_s1s2(block_t *b, data_t MAX_acc, data_t vs, data_t vm,
                             data_t vf, data_t si, data_t s1, data_t s2,
                             data_t sf) {
  assert(b);
  data_t vs_max = sqrt(2 * MAX_acc * (sf - si) + pow(vf, 2));
  data_t vf_max = sqrt(2 * MAX_acc * (sf - si) + pow(vs, 2));
  data_t s_star_AD =
      (pow(vf, 2) - pow(vs, 2) + 2 * MAX_acc * (si + sf)) / (4 * MAX_acc);
  data_t s_star_DA =
      (pow(vs, 2) - pow(vf, 2) + 2 * MAX_acc * (si + sf)) / (4 * MAX_acc);

  // deal also with the case of not being able to reach fm when Accelerating !
  if (vf_max == vf) {
    if (vs >= vm || (vs < vm && vf_max <= vm)) {
      block_set_s1(b, si);
      block_set_s2(b, sf);
      block_set_path(b, "A-!M");
    } else
      block_set_path(b, "A");
  }

  // A-D
  else if (vs < vm && vf < vm && s2 <= s1) {
    block_set_s1(b, s_star_AD);
    block_set_s2(b, s_star_AD);
    block_set_v_star(b, sqrt(pow(vs, 2) + 2 * MAX_acc * (block_s1(b) - si)));
    block_set_path(b, "A-D");
  }

  // deal also with the case of not being able to reach fm when Decelerating !
  else if (vs_max == vs) {
    if ((vs > vm && vf >= vm) || (vs <= vm && vf < vm)) {
      block_set_s1(b, si);
      block_set_s2(b, sf);
      block_set_path(b, "D-!M");
    } else
      block_set_path(b, "D");
  }

  // D-A
  else if (vs > vm && vm < vf && s2 <= s1) {
    block_set_s1(b, s_star_DA);
    block_set_s2(b, s_star_DA);
    block_set_v_star(b, sqrt(pow(vs, 2) - 2 * MAX_acc * (block_s1(b)) - si));
    block_set_path(b, "D-A");
  }
}

// useful later on to distinguish among the 9 different cases
void block_LA_path_name(block_t *b, data_t vs, data_t vm, data_t vf, data_t si,
                        data_t s1, data_t s2, data_t sf) {
  assert(b);
  if (vs < vm && vm < vf && s1 != si && s2 != sf && s1 != s2)
    block_set_path(b, "A-M-A");
  else if (vs > vm && vm > vf && s1 != si && s2 != sf && s1 != s2)
    block_set_path(b, "D-M-D");
  else if (vs == vm && vm == vf)
    block_set_path(b, "M");
  else if (vs > vm && vm < vf && s1 != si && s2 != sf && s1 != s2)
    block_set_path(b, "D-M-A");
  else if (vs < vm && vm > vf && s1 != si && s2 != sf && s1 != s2)
    block_set_path(b, "A-M-D");
}

//  ____ _____ _____ ____    _  _
// / ___|_   _| ____|  _ \  | || |
// \___ \ | | |  _| | |_) | | || |_
//  ___) || | | |___|  __/  |__   _|
// |____/ |_| |_____|_|        |_|
// Calculate the timings [sec]

void block_LA_timings(block_t *b, data_t MAX_acc, data_t vs, data_t vm,
                      data_t vf, data_t s1, data_t s2) {
  assert(b);
  char *block_path = block_path_name(b);
  data_t dt_1 = 0.0;
  data_t dt_m = 0.0;
  data_t dt_2 = 0.0;

  if (strcmp(block_path, "A-M-A") == 0 || strcmp(block_path, "A-M-D") == 0 ||
      strcmp(block_path, "D-M-A") == 0 || strcmp(block_path, "D-M-D") == 0) {
    dt_1 = fabs(vm - vs) / MAX_acc;
    dt_m = (s2 - s1) / vm;
    dt_2 = fabs(vf - vm) / MAX_acc;
  } else if (strcmp(block_path, "A-D") == 0 || strcmp(block_path, "D-A") == 0) {
    dt_1 = fabs(block_v_star(b) - vs) / MAX_acc;
    dt_m = 0.0;
    dt_2 = fabs(vf - block_v_star(b)) / MAX_acc;
  } else if (strcmp(block_path, "A") == 0 || strcmp(block_path, "D") == 0 ||
             strcmp(block_path, "A-!M") == 0 ||
             strcmp(block_path, "D-!M") == 0) {
    dt_1 = fabs(vf - vs) / MAX_acc;
    dt_m = dt_2 = 0.0;
  } else if (strcmp(block_path, "M") == 0) {
    dt_1 = 0.0;
    dt_m = (s2 - s1) / vm;
    dt_2 = 0.0;
  }

  // from [min] to [sec]
  // set calculated values in block velocity profile object
  dt_1 *= 60;
  dt_m *= 60;
  dt_2 *= 60;

  block_set_dt_1(b, dt_1);
  block_set_dt_m(b, dt_m);
  block_set_dt_2(b, dt_2);
  block_set_dt(b, block_dt_1(b) + block_dt_m(b) + block_dt_2(b));
}

//  ____ _____ _____ ____    ____
// / ___|_   _| ____|  _ \  | ___|
// \___ \ | | |  _| | |_) | |___ \
//  ___) || | | |___|  __/   ___) |
// |____/ |_| |_____|_|     |____/
// Reshaping Velocities

void block_LA_reshapeFeed(block_t *b, data_t vs, data_t vm, data_t vf,
                          data_t total, int last) {
  assert(b);
  data_t dq; // amount of time for rounding up to the next multiple of tq
  char *block_path = block_path_name(b);

  data_t t_star = quantize_LA(total, machine_tq(block_machine(b)), &dq);
  data_t k = t_star / total;

  // rescaling velocities (actual velocities)
  block_set_FS(b, vs / k);
  block_set_F(b, vm / k);
  block_set_FE(b, vf / k);

  if (strcmp(block_path, "A-D") == 0 || strcmp(block_path, "D-A") == 0)
    block_set_v_star(b, block_v_star(b) / k);

  // adding dq to the time of the portion of the LAST block between 2 G00 blocks
  if (last) {
    if (strcmp(block_path, "A") == 0 || strcmp(block_path, "D") == 0 ||
        strcmp(block_path, "A-!M") == 0 || strcmp(block_path, "D-!M") == 0)
      block_set_dt_1(b, block_dt_1(b) + dq);
    else if (strcmp(block_path, "M") == 0)
      block_set_dt_m(b, block_dt_m(b) + dq);
    else if (strcmp(block_path, "A-D") == 0 || strcmp(block_path, "D-A") == 0 ||
             strcmp(block_path, "A-M-A") == 0 ||
             strcmp(block_path, "A-M-D") == 0 ||
             strcmp(block_path, "D-M-A") == 0 ||
             strcmp(block_path, "D-M-D") == 0)
      block_set_dt_2(b, block_dt_2(b) + dq);

    block_set_dt(b, block_dt_1(b) + block_dt_m(b) + block_dt_2(b));
  }

  block_set_k(b, k); // for plotting purposes
  block_set_length(b, block_length(b));
}

void block_LA_reshapeAccDec(block_t *b, data_t vs, data_t vm, data_t vf) {
  assert(b);
  char *block_path = block_path_name(b);
  data_t dt_1 = block_dt_1(b);
  data_t dt_2 = block_dt_2(b);

  // Accelerations, Decelerations (pure M as default values)
  data_t a = 0.0;
  data_t d = 0.0;
  if (strcmp(block_path, "A-D") == 0 || strcmp(block_path, "D-A") == 0) {
    a = (block_v_star(b) - vs) / dt_1;
    d = (vf - block_v_star(b)) / dt_2;
  } else if (strcmp(block_path, "A-M-A") == 0 ||
             strcmp(block_path, "A-M-D") == 0 ||
             strcmp(block_path, "D-M-A") == 0 ||
             strcmp(block_path, "D-M-D") == 0) {
    a = (vm - vs) / dt_1;
    d = (vf - vm) / dt_2;
  } else if (strcmp(block_path, "A") == 0 || strcmp(block_path, "D") == 0 ||
             strcmp(block_path, "A-!M") == 0 || strcmp(block_path, "D-!M") == 0)
    a = (vf - vs) / dt_1;

  // actual acceleration and deceleration ([mm/s^2])
  block_set_acc(b, a / 60);
  block_set_acc(b, d / 60);
}

//  ____ _____ _____ ____     __
// / ___|_   _| ____|  _ \   / /_
// \___ \ | | |  _| | |_) | | '_ \
//  ___) || | | |___|  __/  | (_) |
// |____/ |_| |_____|_|      \___/
// Calculate lambda as usual (used in fsm.c)

data_t block_lambda_LA(const block_t *b, data_t fs, data_t f, data_t fe,
                       data_t t, data_t *v) {
  assert(b);
  data_t res = 0.0;
  char *block_path = block_path_name(b);
  data_t a = block_acc(b);
  data_t d = block_dec(b);

  data_t dt_1 = block_dt_1(b);
  data_t dt_m = block_dt_m(b);
  data_t dt_2 = block_dt_2(b);

  // deal also with the cases for whick fm is not reached
  if (strcmp(block_path, "A-D") == 0 || strcmp(block_path, "D-A") == 0)
    f = block_v_star(b) / 60;

  if (t < 0.0)
    *v = 0.0;
  else if (t < dt_1) {
    res = a * pow(t, 2) / 2.0 + fs * t;
    *v = a * t + fs;
  } else if (t < (dt_1 + dt_m)) {
    res = a * pow(dt_1, 2) / 2.0 + fs * dt_1 + f * (t - dt_1);
    *v = f;
  } else if (t < (dt_1 + dt_m + dt_2)) {
    data_t t2 = dt_1 + dt_m;
    res = a * pow(dt_1, 2) / 2.0 + fs * dt_1 + f * (dt_m + t - t2) +
          d / 2.0 * (pow(t, 2) + pow(t2, 2)) - d * t * t2;
    *v = f + d * (t - t2);
  } else {
    res = block_len(b);
    *v = fe;
  }

  res /= block_len(b); // normalization
  *v *= 60;            // convert to [mm/min]
  return res;
}

//   ____  _        _   _         __
//  / ___|| |_ __ _| |_(_) ___   / _|_   _ _ __   ___
//  \___ \| __/ _` | __| |/ __| | |_| | | | '_ \ / __|
//   ___) | || (_| | |_| | (__  |  _| |_| | | | | (__
//  |____/ \__\__,_|\__|_|\___| |_|  \__,_|_| |_|\___|
// Definitions for the static functions declared above

static data_t quantize_LA(data_t time, data_t tq, data_t *dq) {
  data_t q = ((size_t)(time / tq) + 1) * tq;
  *dq = q - time;
  return q;
}

static point_t *point_zero(const block_t *b) {
  assert(b);
  return block_prev(b) ? block_target(block_prev(b))
                       : machine_zero(block_machine(b));
}

//                                   ____     ____
// Scalar Product between 2 vectors (P1P2 and P2P3)
static data_t dot_product(const point_t *p1, const point_t *p2,
                          const point_t *p3) {
  assert(p1 && p2 && p3);
  return (point_x(p2) - point_x(p1)) * (point_x(p3) - point_x(p2)) +
         (point_y(p2) - point_y(p1)) * (point_y(p3) - point_y(p2)) +
         (point_z(p2) - point_z(p1)) * (point_z(p3) - point_z(p2));
}

// Compute the cosine of the angle between each pair of consecutive blocks
static data_t cosAlpha(const block_t *b) {
  assert(b);
  data_t cos_alpha = 0.0;
  point_t *p1 = point_zero(b);
  point_t *p2 = block_target(b);
  point_t *p3 = block_target(block_next(b));
  point_t *center = block_center(b);
  point_t *center_next = block_center(block_next(b));

  data_t v1_lin = point_dist(p1, p2);
  data_t v2_lin = point_dist(p2, p3);
  data_t v1_arc = block_r(b);
  data_t v2_arc = block_r(block_next(b));

  // LINEAR BLOCK -- LINEAR BLOCK
  if (block_type(b) == LINE && block_type(block_next(b)) == LINE)
    cos_alpha = dot_product(p1, p2, p3) / (fabs(v1_lin) * fabs(v2_lin));

  // ARC_CW or ARC_CCW BLOCK -- LINEAR BLOCK
  else if ((block_type(b) == ARC_CW || block_type(b) == ARC_CCW) &&
           block_type(block_next(b)) == LINE) {
    data_t dot = dot_product(center, p2, p3);
    data_t beta = acos(dot / (fabs(v1_arc) * fabs(v2_lin))); // [rad]
    cos_alpha = cos(M_PI / 2.0 - beta); // r is always orthogonal to a
                                        // trajectory
  }

  // LINEAR BLOCK -- ARC_CW or ARC_CCW BLOCK
  else if (block_type(b) == LINE && (block_type(block_next(b)) == ARC_CW ||
                                     block_type(block_next(b)) == ARC_CCW)) {
    data_t dot = dot_product(p1, p2, center_next);
    data_t beta = acos(dot / (fabs(v1_lin) * fabs(v2_arc))); // [rad]
    cos_alpha = cos(M_PI / 2.0 - beta); // r is always orthogonal to a
                                        // trajectory
  }

  // all 4 combinations of ARC_CW and ARC_CCW blocks
  else if ((block_type(b) == ARC_CW && block_type(block_next(b)) == ARC_CCW) ||
           (block_type(b) == ARC_CCW && block_type(block_next(b)) == ARC_CW) ||
           (block_type(b) == ARC_CW && block_type(block_next(b)) == ARC_CW) ||
           (block_type(b) == ARC_CCW && block_type(block_next(b)) == ARC_CCW)) {
    cos_alpha =
        dot_product(center, p2, center_next) / (fabs(v1_arc) * fabs(v2_arc));
  }
  return cos_alpha;
}