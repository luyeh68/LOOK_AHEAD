//   ____  _            _
//  | __ )| | ___   ___| | __
//  |  _ \| |/ _ \ / __| |/ /
//  | |_) | | (_) | (__|   <
//  |____/|_|\___/ \___|_|\_\

#include "block.h"
#include <ctype.h>

//   ____            _                 _   _
//  |  _ \  ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __  ___
//  | | | |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | \__ \
//  |____/ \___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|___/

// Trapezoidal velocity profile
typedef struct {
  data_t a, d;   // acceleration, deceleration
  data_t f, l;   // nominal feedrate and length
  data_t fs, fe; // initial and final feedrate
  data_t dt_1, dt_m,
      dt_2;  // trapezoidal acceleration, maintenance and deceleration times
  data_t dt; // total time

  // ADDITIONS FOR LOOK AHEAD
  data_t s[4];           // 4 notable points [si, s1, s2, sf]
  char *path_desc;       // profile description (9 possible cases)
  data_t k;              // feed correcting factor
  data_t v_star, s_star; // v_star and s_star for A-D and D-A case
} block_profile_t;

// Block object structure
typedef struct block {
  char *line;            // G-code line
  block_type_t type;     // type of block
  size_t n;              // block number
  size_t tool;           // tool number
  data_t feedrate;       // nominal feedrate
  data_t act_feedrate;   // actual feedrate (possibly reduced along arcs)
  data_t spindle;        // spindle rate
  point_t *target;       // destination point
  point_t *delta;        // distance vector w.r.t. previous point
  point_t *center;       // arc center (if it is an arc)
  data_t length;         // total length
  data_t i, j, r;        // center coordinates and radius (if it is an arc)
  data_t theta0, dtheta; // arc initial angle and arc angle
  data_t acc;            // actual acceleration
  machine_t *machine;    // machine configuration
  block_profile_t *prof; // velocity profile
  struct block *prev;    // next block (linked list fashion)
  struct block *next;    // previous block
} block_t;

// STATIC FUNCTIONS (for internal use only) ====================================
static int block_set_fields(block_t *b, char cmd, char *arg);
static point_t *point_zero(const block_t *b);
static void block_compute(const block_t *b);
static int block_arc(block_t *b);
static data_t quantize(data_t t, data_t tq, data_t *dq);

//   _____                 _   _
//  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
//  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
//  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
//  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/

// LIFECYCLE ===================================================================

block_t *block_new(const char *line, block_t *prev, machine_t *cfg) {
  assert(line && cfg); // prev is NULL if this is the first block
  block_t *b = (block_t *)calloc(1, sizeof(block_t));
  if (!b) {
    perror("Could not allocate block object");
    return NULL;
  }

  if (prev) { // copy the memory from the previous block
    memcpy(b, prev, sizeof(block_t));
    // linked list behaviour
    b->prev = prev;
    prev->next = b;
  } else { // this is the first block
    // nothing to do
  }

  // non-modal g-code parameters: I, J, R
  b->i = b->j = b->r = 0.0;

  // fields to be calculated
  b->length = 0.0;
  b->target = point_new();
  b->delta = point_new();
  b->center = point_new();

  // allocate memory for profile struct
  b->prof = (block_profile_t *)calloc(1, sizeof(block_profile_t));
  if (!b->prof) {
    perror("Could not allocate velocity profile structure");
    return NULL;
  }

  // assigning sensitive defaults
  b->machine = cfg;
  b->type = NO_MOTION;
  b->acc = machine_A(b->machine);
  b->line = strdup(line);
  if (!b->line) {
    perror("Could not allocate G-code line/block");
    return NULL;
  }

  return b;
}

void block_free(block_t *b) {
  assert(b);
  if (b->line)
    free(b->line);
  if (b->prof)
    free(b->prof);
  point_free(b->target);
  point_free(b->center);
  point_free(b->delta);
  free(b);
  b = NULL;
}

void block_print(const block_t *b, FILE *out) {
  assert(b && out);
  char *start, *end;
  // if this is the first block, p0 is the origin
  // otherwise is the target of the previous block
  point_t *p0 = point_zero(b);
  // inspect origin and target points
  point_inspect(p0, &start);
  point_inspect(b->target, &end);
  // print out block description
  fprintf(out, "%03lu %s->%s F%7.1f S%7.1f T%2lu (G%02d)\n", b->n, start, end,
          b->feedrate, b->spindle, b->tool, b->type);
  free(end);
  free(start);
}

void block_set_path(const block_t *b, char *desc) // ADDED FOR LOOK AHEAD ***
{
  assert(b && desc);
  b->prof->path_desc = desc;
}

// ALGORITHMS ==================================================================

// Parsing the G-code string. Returns an integer for success/failure
int block_parse(block_t *b) {
  assert(b);
  char *word, *line, *tofree;
  point_t *p0;
  int rv = 0; // parsing errors Counter

  tofree = line = strdup(b->line);
  if (!line) {
    perror("Could not allocate memory for tokenizing G-code line");
    return 1;
  }
  // Tokenizing loop for PARSING G-code line
  while ((word = strsep(&line, " ")) != NULL) {
    // word[0] is the command
    // word+1 is the pointer to the argument as a string
    rv += block_set_fields(b, toupper(word[0]), word + 1);
  }
  free(tofree);

  // inherit modal fields from the previous block
  p0 = point_zero(b);
  point_modal(p0, b->target);
  point_delta(p0, b->target, b->delta);
  b->length = point_dist(p0, b->target);

  // deal with motion blocks or types of blocks
  switch (b->type) {
  case LINE:
    // calculate feed profile
    b->acc = machine_A(b->machine);
    b->act_feedrate = b->feedrate;
    block_compute(b);
    break;
  case ARC_CW:
  case ARC_CCW:
    /* calculate arc coordinates: arc radius OR arc center depending on the
       INPUT: calculate the center if R is given OR we calculate R if center is
       given */
    if (block_arc(b)) {
      rv++;
      break;
    }
    // set corrected feedrate and acceleration
    // centripetal acc = f^2/r, must be <= A
    // INI file gives A in mm/s^2, feedrate is given in mm/min
    // We divide by two because, in the critical condition where we have
    // the maximum feedrate, in the following equation for calculating the
    // acceleration, it goes to 0. In fact, if we accept the centripetal
    // acceleration to reach the maximum acceleration, then the tangential
    // acceleration would go to 0.
    // A more elegant solution would be to calculate a minimum time solution for
    // the whole arc, but it is outside the scope.
    b->act_feedrate =
        MIN(b->feedrate, sqrt(machine_A(b->machine) / 2.0 * b->r) * 60);
    // tangential acceleration: when composed with centripetal one, total
    // acceleration must be <= A
    // a^2 <= A^2 - v^4/r^2
    b->acc = sqrt(pow(machine_A(b->machine), 2) -
                  pow(b->act_feedrate / 60, 4) / pow(b->r, 2));
    // deal with complex result
    if (isnan(b->acc)) {
      eprintf("Cannot compute arc: insufficient acceleration");
      rv++;
    }
    // calculate feed profile
    block_compute(b);
    break;
  default:
    break;
  }
  // return number of parsing errors
  return rv;
}

/* Evaluate the value of lambda at a certaint time and also return the current
 * velocity v (Lambda = it's the integral of block velocity profile normalized
 * to 1 (divided by the total block length)) */
data_t block_lambda(const block_t *b, data_t t, data_t *v) {
  assert(b);
  data_t res;
  data_t dt_1 = b->prof->dt_1;
  data_t dt_2 = b->prof->dt_2;
  data_t dt_m = b->prof->dt_m;
  data_t a = b->prof->a;
  data_t d = b->prof->d;
  data_t f = b->prof->f;

  if (t < 0) // never happen
  {
    res = 0.0;
    *v = 0.0;
  } else if (t < dt_1) { // acceleration
    res = a * pow(t, 2) / 2.0;
    *v = a * t;
  } else if (t < (dt_1 + dt_m)) { // maintenance
    res = f * (dt_1 / 2.0 + (t - dt_1));
    *v = f;
  } else if (t < (dt_1 + dt_m + dt_2)) { // deceleration
    data_t t_2 = dt_1 + dt_m;
    res = f * dt_1 / 2.0 + f * (dt_m + t - t_2) +
          d / 2.0 * (pow(t, 2) + pow(t_2, 2)) - d * t * t_2;
    *v = f + d * (t - t_2);
  } else {
    res = b->prof->l;
    *v = 0.0;
  }
  res /= b->prof->l; // normalization
  *v *= 60;          // convert to mm/min
  return res;
}

point_t *block_interpolate(const block_t *b, data_t lambda) {
  assert(b);
  point_t *result = machine_setpoint(b->machine);
  point_t *p0 = point_zero(b); // NOT to be deallocated

  if (b->type == LINE) {
    point_set_x(result, point_x(p0) + point_x(b->delta) * lambda);
    point_set_y(result, point_y(p0) + point_y(b->delta) * lambda);
  } else if (b->type == ARC_CW || b->type == ARC_CCW) {
    point_set_x(result, point_x(b->center) +
                            b->r * cos(b->theta0 + b->dtheta * lambda));
    point_set_y(result, point_y(b->center) +
                            b->r * sin(b->theta0 + b->dtheta * lambda));
  } else {
    fprintf(stderr, "Unexpected block type!\n");
    return NULL;
  }
  point_set_z(result, point_z(p0) + point_z(b->delta) * lambda);

  return result;
}

// GETTERS =====================================================================

#define block_getter(typ, par, name)                                           \
  typ block_##name(const block_t *b) {                                         \
    assert(b);                                                                 \
    return b->par;                                                             \
  }

// SETTERS =====================================================================

#define block_setter(typ, par, name)                                           \
  typ block_set_##name(block_t *b, data_t value) {                             \
    assert(b);                                                                 \
    b->par = value;                                                            \
  }

block_getter(data_t, length, length);
block_getter(data_t, dtheta, dtheta);
block_getter(data_t, prof->dt, dt);
block_getter(block_type_t, type, type);
block_getter(char *, line, line);
block_getter(size_t, n, n);
block_getter(data_t, r, r);
block_getter(point_t *, center, center);
block_getter(block_t *, next, next);
block_getter(point_t *, target, target);

// ADDITIONS FOR LOOK AHEAD ====================================================
block_getter(block_t *, prev, prev);
block_getter(machine_t *, machine, machine);
block_getter(data_t, act_feedrate, actFeed);
block_getter(data_t, prof->fs, FS);
block_getter(data_t, prof->f, F);
block_getter(data_t, prof->fe, FE);
block_getter(data_t, prof->s[0], si);
block_getter(data_t, prof->s[1], s1);
block_getter(data_t, prof->s[2], s2);
block_getter(data_t, prof->s[3], sf);
block_getter(data_t, prof->v_star, v_star);
block_getter(data_t, prof->dt_1, dt_1);
block_getter(data_t, prof->dt_m, dt_m);
block_getter(data_t, prof->dt_2, dt_2);
block_getter(data_t, prof->l, len);
block_getter(data_t, prof->a, acc);
block_getter(data_t, prof->d, dec);
block_getter(data_t, prof->k, k);
block_getter(char *, prof->path_desc, path_name);

block_setter(void, prof->fs, FS);
block_setter(void, prof->f, F);
block_setter(void, prof->fe, FE);
block_setter(void, prof->s[0], si);
block_setter(void, prof->s[1], s1);
block_setter(void, prof->s[2], s2);
block_setter(void, prof->s[3], sf);
block_setter(void, prof->v_star, v_star);
block_setter(void, prof->dt_1, dt_1);
block_setter(void, prof->dt_m, dt_m);
block_setter(void, prof->dt_2, dt_2);
block_setter(void, prof->dt, dt);
block_setter(void, prof->l, length);
block_setter(void, prof->a, acc);
block_setter(void, prof->d, dec);
block_setter(void, prof->k, k);

// =============================================================================

//   ____  _        _   _         __
//  / ___|| |_ __ _| |_(_) ___   / _|_   _ _ __   ___
//  \___ \| __/ _` | __| |/ __| | |_| | | | '_ \ / __|
//   ___) | || (_| | |_| | (__  |  _| |_| | | | | (__
//  |____/ \__\__,_|\__|_|\___| |_|  \__,_|_| |_|\___|
// Definitions for the static functions declared above

// Calculate the integer multiple of sampling time; also provide the rounding
// amount in dq
static data_t quantize(data_t t, data_t tq, data_t *dq) {
  data_t q = ((size_t)(t / tq) + 1) * tq;
  *dq = q - t; // amount of time for rounding up to the next multiple of tq
  return q;
}

// Calculate the velocity profile
static void block_compute(const block_t *b) {
  assert(b);
  data_t dt, dq; // dq = dt - nextTick => dt = n * dq

  data_t A = b->acc;
  data_t f_m = b->act_feedrate / 60.0; // [mm/s]
  data_t l = b->length;
  data_t dt_1 = f_m / A;
  data_t dt_2 = dt_1;
  data_t dt_m = l / f_m - (dt_1 + dt_2) / 2.0;
  if (dt_m > 0) { // trapezoidal profile
    dt = quantize(dt_1 + dt_m + dt_2, machine_tq(b->machine), &dq);
    dt_m += dq;
    f_m = (2 * l) / (dt_1 + dt_2 + 2 * dt_m);
  } else { // triangular profile (short block): too short to reach the nominal f
    dt_1 = sqrt(l / A);
    dt_2 = dt_1;
    dt = quantize(dt_1 + dt_2, machine_tq(b->machine), &dq);
    dt_m = 0.0;
    dt_2 += dq;
    f_m = 2 * l / (dt_1 + dt_2);
  }
  data_t a = f_m / dt_1;
  data_t d = -(f_m / dt_2);
  // set calculated values in block velocity profile object
  // timings
  b->prof->dt_1 = dt_1;
  b->prof->dt_2 = dt_2;
  b->prof->dt_m = dt_m;
  // actual acc and decel
  b->prof->a = a;
  b->prof->d = d;
  // actual f and block duration
  b->prof->f = f_m;
  b->prof->dt = dt;
  b->prof->l = l;
}

// Calculate the arc coordinates
static int block_arc(block_t *b) {
  assert(b);
  data_t x0, y0, z0, xc, yc, xf, yf, zf, r;
  point_t *p0 = point_zero(b);
  x0 = point_x(p0);
  y0 = point_y(p0);
  z0 = point_z(p0);
  xf = point_x(b->target);
  yf = point_y(b->target);
  zf = point_z(b->target);

  if (b->r) { // if the radius is given
    data_t dx = point_x(b->delta);
    data_t dy = point_y(b->delta);
    r = b->r;
    data_t dxy2 = pow(dx, 2) + pow(dy, 2);
    data_t sq = sqrt(-pow(dy, 2) * dxy2 * (dxy2 - 4 * r * r));
    // Signs table
    // ===========================
    // sign(r) | CW(-1) | CCW(+1)
    // ===========================
    //   -1     |   +    |   -
    //   +1     |   -    |   +
    // ===========================
    int s = (r > 0) - (r < 0); // radius sign
    s *= (b->type == ARC_CCW ? 1 : -1);
    xc = x0 + (dx - s * sq / dxy2) / 2.0;
    yc = y0 + dy / 2.0 + s * (dx * sq) / (2 * dy * dxy2);
  } else { // if I,J are given
    data_t r2;
    r = hypot(b->i, b->j);
    xc = x0 + b->i;
    yc = y0 + b->j;
    r2 = hypot(xf - xc, yf - yc);
    if (fabs(r - r2) > machine_error(b->machine)) {
      fprintf(stderr, "Arc endpoints mismatch error (%f)\n", r - r2);
      return 1;
    }
    b->r = r;
  }
  point_set_x(b->center, xc);
  point_set_y(b->center, yc);
  b->theta0 = atan2(y0 - yc, x0 - xc);
  b->dtheta = atan2(yf - yc, xf - xc) - b->theta0;
  // we need the net angle so we take the 2PI complement if negative
  if (b->dtheta < 0)
    b->dtheta = 2 * M_PI + b->dtheta;
  // if CW, takes the negative complement
  if (b->type == ARC_CW)
    b->dtheta = -(2 * M_PI - b->dtheta);
  //
  b->length = hypot(zf - z0, b->dtheta * b->r);
  // from now on, it's safer to drop radius angle (only better positive values
  // for r)
  b->r = fabs(b->r);
  return 0;
}

// Return a reliable previous point, i.e. machine zero if this is the first
// block (no memory leaks)
static point_t *point_zero(const block_t *b) {
  assert(b);
  return b->prev ? b->prev->target : machine_zero(b->machine);
}

// Parse a single G-code word (cmd+arg)
static int block_set_fields(block_t *b, char cmd, char *arg) {
  assert(b && arg);
  switch (cmd) {
  case 'N':
    b->n = atol(arg);
    break;
  case 'G':
    b->type = (block_type_t)atoi(arg);
    break;
  case 'X':
    point_set_x(b->target, atof(arg));
    break;
  case 'Y':
    point_set_y(b->target, atof(arg));
    break;
  case 'Z':
    point_set_z(b->target, atof(arg));
    break;
  case 'I':
    b->i = atof(arg);
    break;
  case 'J':
    b->j = atof(arg);
    break;
  case 'R':
    b->r = atof(arg);
    break;
  case 'F':
    b->feedrate = atof(arg);
    break;
  case 'S':
    b->spindle = atof(arg);
    break;
  case 'T':
    b->tool = atol(arg);
    break;
  default: // we either specify an arc with its R and end Point or by its end
           // point and center
    fprintf(stderr, "PARSING ERROR: Usupported G-code command %c%s\n", cmd,
            arg);
    return 1;
    break;
  }
  // cannot have R and IJ on the same block
  if (b->r && (b->i || b->j)) {
    fprintf(stderr, "ERROR: Cannot mix R and IJ\n");
    return 1;
  }
  return 0;
}

//   _____ _____ ____ _____   __  __       _
//  |_   _| ____/ ___|_   _| |  \/  | __ _(_)_ __
//    | | |  _| \___ \ | |   | |\/| |/ _` | | '_ \
//    | | | |___ ___) || |   | |  | | (_| | | | | |
//    |_| |_____|____/ |_|   |_|  |_|\__,_|_|_| |_|
//
#ifdef BLOCK_MAIN
int main() {
  block_t *b1 = NULL, *b2 = NULL, *b3 = NULL;
  machine_t *cfg = machine_new(NULL);

  b1 = block_new("N10 G00 X90 Y90 Z100 t3", NULL, cfg);
  block_parse(b1);
  b2 = block_new("N20 G01 Y100 X100 F1000 S2000", b1, cfg);
  block_parse(b2);
  b3 = block_new("N30 G01 Y200", b2, cfg);
  block_parse(b3);

  block_print(b1, stdout);
  block_print(b2, stdout);
  block_print(b3, stdout);

  block_free(b1);
  block_free(b2);
  block_free(b3);
  return 0;
}
#endif