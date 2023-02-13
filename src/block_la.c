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

//  _   _                 _____          _
// | \ | | _____      __ |  ___|__  __ _| |_ _   _ _ __ ___  ___
// |  \| |/ _ \ \ /\ / / | |_ / _ \/ _` | __| | | | '__/ _ \/ __|
// | |\  |  __/\ V  V /  |  _|  __/ (_| | |_| |_| | | |  __/\__ \
// |_| \_|\___| \_/\_/   |_|  \___|\__,_|\__|\__,_|_|  \___||___/
// ============================================================================

// STATIC FUNCTIONS (for internal use only)
static data_t quantize_LA(data_t time, data_t tq);

//  ____ _____ _____ ____    _
// / ___|_   _| ____|  _ \  / |
// \___ \ | | |  _| | |_) | | |
//  ___) || | | |___|  __/  | |
// |____/ |_| |_____|_|     |_|
// Intermediate velocities

data_t dot_product(const point_t *p1, const point_t *p2, const point_t *p3) {
  assert(p1 && p2 && p3);
  return (point_x(p2) - point_x(p1)) * (point_x(p3) - point_x(p2)) +
         (point_y(p2) - point_y(p1)) * (point_y(p3) - point_y(p2)) +
         (point_z(p2) - point_z(p1)) * (point_z(p3) - point_z(p2));
}

data_t cosAlpha(const block_t *b) {
  assert(b);
  data_t cos_alpha;
  point_t *p1 = block_prev(b) ? block_target(block_prev(b))
                              : machine_zero(block_machine(b));
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
  else {
    cos_alpha =
        dot_product(center, p2, center_next) / (fabs(v1_arc) * fabs(v2_arc));
  }
  return cos_alpha;
}

data_t maintenanceVel(const block_t *b) {
  assert(b);

  // LINEAR block
  if (block_type(b) == LINE)
    return block_nomFeed(b); // [mm/min]

  // ARC_CW or ARC_CCW block
  else if (block_type(b) == ARC_CW || block_type(b) == ARC_CCW)
    return sqrt(machine_A(block_machine(b)) * block_r(b)) * 60; // [mm/min]

  return 0.0; // RAPID block
}

data_t finalVel(const block_t *b) {
  assert(b);

  // angle > 45° OR vm,i = 0 ==> 0 speed OR if last segment ==> 0 speed
  if (cosAlpha(b) > (sqrt(2) / 2) || block_FM(b) == 0.0 ||
      block_type(block_next(b)) == RAPID || !block_next(b))
    return 0.0;

  return (maintenanceVel(b) + maintenanceVel(block_next(b))) / 2.0 *
         cosAlpha(b); //[mm/min]
}

data_t initialVel(const block_t *b) {
  assert(b);
  return block_prev(b) ? finalVel(block_prev(b)) : 0.0; // fs,i = fe,i-1
}

void setFeed_sisf(const block_t *b) {
  assert(b);
  block_set_FS(b, initialVel(b));
  block_set_FM(b, maintenanceVel(b));
  block_set_FE(b, finalVel(b));

  // Any block can be considered indipendent in its own Reference frame:

  // si
  block_set_si(b, 0.0);
  // sf
  block_set_sf(b, block_length(b));

  block_set_v_star(b, block_FM(b));
}

//  ____ _____ _____ ____    ____      _____
// / ___|_   _| ____|  _ \  |___ \    |___ /
// \___ \ | | |  _| | |_) |   __) |____ |_ \
//  ___) || | | |___|  __/   / __/_____|__) |
// |____/ |_| |_____|_|     |_____|   |____/

// ============================== 9 possible cases =============================

// ============================= Accelerations =================================
void forwardAcc(const block_t *b, data_t MAX_acc, data_t vi, data_t vm,
                data_t vf, data_t si, data_t sf) {
  assert(b);
  MAX_acc = MAX_acc * 3600;
  if (vi <= vm) // Equality for dealing with pure Maintenance
    block_set_s1(b, si + (pow(vm, 2) - pow(vi, 2)) / (2.0 * MAX_acc));

  if (vm <= vf) // Equality for dealing with pure Maintenance
    block_set_s2(b, sf + (pow(vm, 2) - pow(vf, 2)) / (2.0 * MAX_acc));
}

// ============================= Decelerations =================================
void backwardDec(const block_t *b, data_t MAX_acc, data_t vi, data_t vm,
                 data_t vf, data_t si, data_t sf) {
  assert(b);
  MAX_acc = MAX_acc * 3600;
  if (vi > vm)
    block_set_s1(b, si + (pow(vi, 2) - pow(vm, 2)) / (2.0 * MAX_acc));

  if (vm > vf)
    block_set_s2(b, sf + (pow(vf, 2) - pow(vm, 2)) / (2.0 * MAX_acc));
}

void recompute_s1s2(const block_t *b, data_t MAX_acc, data_t vi, data_t v_star,
                    data_t vf, data_t si, data_t sf) {
  assert(b);

  if (block_s2(b) < block_s1(b)) {

    MAX_acc = MAX_acc * 3600; // [mm/min^2]

    // A: only acceleration starting with vi (vf_max < vf_nom)
    if (sqrt(2 * MAX_acc * (sf - si) + pow(vi, 2)) < vf) {
      block_set_FE(b, sqrt(2 * MAX_acc * (sf - si) + pow(vi, 2)));
      block_set_s_star(b, (pow(v_star, 2) - pow(vi, 2)) / (2 * MAX_acc) + si);
      // OR s_star = (pow(v_star, 2) - pow(b->prof->fe, 2)) / (2 * MAX_acc) +
      // sf;
    }

    // D: only deceleration (vi_max < vi_nom)
    else if (sqrt(2 * MAX_acc * (sf - si) + pow(vf, 2)) < vi) {
      block_set_FS(b, sqrt(2 * MAX_acc * (sf - si) + pow(vf, 2)));
      block_set_s_star(
          b, (pow(block_FS(b), 2) - pow(v_star, 2)) / (2 * MAX_acc) + si);
      // OR s_star = (pow(vf, 2) - pow(v_star, 2)) / (2 * MAX_acc) + sf;
    }

    // A-D
    else if (vi < v_star && vf < v_star) {
      block_set_s_star(b, (pow(vf, 2) - pow(vi, 2) + 2 * MAX_acc * (si + sf)) /
                              (4 * MAX_acc));
      block_set_v_star(b,
                       sqrt(pow(vi, 2) + 2 * MAX_acc * (block_s_star(b) - si)));
    }

    // D-A
    else if (vi > v_star && vf > v_star) {
      block_set_s_star(b, (pow(vi, 2) - pow(vf, 2) + 2 * MAX_acc * (si + sf)) /
                              (4 * MAX_acc));
      block_set_v_star(b,
                       sqrt(pow(vi, 2) - 2 * MAX_acc * (block_s_star(b) - si)));
    }

    // intersection (s1 = s2 = s_star)
    block_set_s1(b, block_s_star(b));
    block_set_s2(b, block_s_star(b));
  }
}

//  ____ _____ _____ ____    _  _
// / ___|_   _| ____|  _ \  | || |
// \___ \ | | |  _| | |_) | | || |_
//  ___) || | | |___|  __/  |__   _|
// |____/ |_| |_____|_|        |_|
// Calculate the timings [sec]
void timings(const block_t *b, data_t MAX_acc, data_t vi, data_t v_star,
             data_t vf, data_t s1, data_t s2) {
  assert(b);
  block_set_dt_1(b, fabs((v_star - vi) / (60 * MAX_acc)));
  block_set_dt_m(b, (s2 - s1) / v_star * 60);
  block_set_dt_2(b, fabs((vf - v_star) / (60 * MAX_acc)));
  block_set_dt(b, block_dt_1(b) + block_dt_m(b) + block_dt_2(b));
}

//  ____ _____ _____ ____    ____
// / ___|_   _| ____|  _ \  | ___|
// \___ \ | | |  _| | |_) | |___ \
//  ___) || | | |___|  __/   ___) |
// |____/ |_| |_____|_|     |____/
// Reshaping

void reshape(const block_t *b) {
  assert(b);

  data_t dt_1 = block_dt_1(b);
  data_t dt_m = block_dt_m(b);
  data_t dt_2 = block_dt_2(b);

  data_t t_tot = block_dt(b);

  data_t t_star = quantize_LA(t_tot, machine_tq(block_machine(b)));
  data_t k = t_star / t_tot;

  // rescaling velocities (actual velocities)
  block_set_FS(b, block_FS(b) / k);
  // block_set_FM(b, block_FM(b) / k);
  block_set_v_star(b, block_v_star(b) / k);
  block_set_FE(b, block_FE(b) / k);

  data_t fs = block_FS(b);
  data_t f_star = block_v_star(b);
  data_t fe = block_FE(b);
  // data_t fm = block_FM(b); fm is equal to f_star or different (D-A or A-D)

  // (default values) Pure Maintenance
  data_t a = 0.0;
  data_t d = 0.0;

  // A-D or A-M-A or A-M-D or D-A or D-M-D or D-M-A
  if ((fs < f_star && fe < f_star) ||
      (fs < f_star && f_star < fe && dt_m != 0.0) ||
      (fs < f_star && f_star > fe && dt_m != 0.0) ||
      (fs > f_star && fe > f_star) ||
      (fs > f_star && f_star > fe && dt_m != 0.0) ||
      (fs > f_star && f_star < fe && dt_m != 0.0)) {
    a = (f_star - fs) / dt_1;
    d = (fe - f_star) / dt_2;
  }
  //  pure A or pure D or pure M (takes into account also the possibility of not
  //  being able to reach fm if we have only A or only D)
  else if ((fs <= f_star && f_star <= fe && dt_m == 0.0) ||
           (fs > f_star && f_star > fe && dt_m == 0.0)) {
    if (t_tot != 0.0)
      a = (fe - fs) / t_tot;
  }

  a /= 60; // [mm/s^2]
  d /= 60; // [mm/s^2]

  // set calculated values in block velocity profile object
  // actual acc and decel
  block_set_acc(b, a);
  block_set_dec(b, d);
  // actual f and block duration
  block_set_F(b, block_v_star(b));
  block_set_dt(b, t_tot);
  block_set_length(b, block_length(b));
}

//  ____ _____ _____ ____     __
// / ___|_   _| ____|  _ \   / /_
// \___ \ | | |  _| | |_) | | '_ \
//  ___) || | | |___|  __/  | (_) |
// |____/ |_| |_____|_|      \___/
// Calculate lambda as usual

data_t block_lambda_LA(const block_t *b, data_t t, data_t *v) {
  assert(b);
  data_t res = 0.0;
  data_t dt_1 = block_dt_1(b);
  data_t dt_m = block_dt_m(b);
  data_t dt_2 = block_dt_2(b);
  data_t a = block_acc(b);
  data_t d = block_dec(b);
  data_t fs = block_FS(b) / 60; // [mm/s]
  data_t f = block_F(b) / 60;

  // A-M-A, A-M-D, D-M-A, D-M-D, A-D, D-A, A, D, M
  if (t < 0.0)
    *v = 0.0;
  else if (t < dt_1 || (t < (dt_1 + dt_m + dt_2) && d == 0.0)) {
    res = a * pow(t, 2) / 2.0 + fs * t;
    *v = a * t + fs;
  } else if (t < (dt_1 + dt_m) && dt_m != 0.0) {
    res = a * pow(dt_1, 2) / 2.0 + fs * dt_1 + f * (t - dt_1);
    *v = f;
  } else if (t < (dt_1 + dt_m + dt_2) && a != 0.0 && d != 0.0) {
    data_t t2 = dt_1 + dt_m;
    res = a * pow(dt_1, 2) / 2.0 + fs * dt_1 + f * (dt_m + t - t2) +
          d / 2.0 * (pow(t, 2) + pow(t2, 2)) - d * t * t2;
    *v = f + d * (t - t2);
  } else {
    res = block_len(b);
    *v = finalVel(b);
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

static data_t quantize_LA(data_t time, data_t tq) {
  return ((size_t)(time / tq) + 1) * tq;
}
