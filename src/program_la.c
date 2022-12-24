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
  block_t *b, *iterator = program_first(p);
  if (block_type(iterator) == RAPID) {
    b = block_next(iterator);
    while (b) 
    {
      if (block_type(b) == RAPID) {
        iterator->prof->initialVel = 0.0;
        b->prof->finalVel = 0.0;
        iterator = b;
      }
      if (maintenanceVel(b) == 0.0)
        b->prof->finalVel = 0.0;
      b = block_next(b);
    } // b = NULL, iterator = last block of program
  }
}

void program_LA_execution(const program_t *p, machine_t *cfg) 
{
  assert(p && cfg);
  block_t *b = program_first(p);
  data_t MAX_acc = machine_A(cfg) * 3600; // to get [mm/min^2]
  data_t tq = machine_tq(cfg);
  
  // setting initial and final velocities for the whole program p
  guards_G00(p);

  while(b)
  {
    // setting intermediate velocities and initial and final s coordinate for each block b
    settingVel_sisf(b);

    data_t vi = b->prof->initialVel;
    data_t vf = b->prof->finalVel;
    data_t vm = b->prof->maintenanceVel;
    data_t si = b->prof->s[0];
    data_t sf = b->prof->s[S_LENGTH-1];
  
    // computation of s1 and s2
    if(vi <= vm)
      s1s2_Acc(b, MAX_acc, vi, vm, vf, si, sf);
    else
      s1s2_Dec(b, MAX_acc, vi, vm, vf, si, sf);

    // reordering the s coordinates for each and every block
    ascending(b->prof->s, S_LENGTH);

    // change variable from curvilinear abscissa (s) to time (t)
    timings(b, MAX_acc, vi, vm, vf, si, b->prof->s[1], b->prof->s[2], sf);

    reshaping(b, b->prof->tf, tq);

    // so Now I recalculate the accelerations and decelerations for each block
    // until now we use the nominal max acceleration (also equal in magnitude to the max deceleration)
    if(vi <= vm)
      s1s2_Acc(b, MAX_acc, vi, vm, vf, si, sf);
    else
      s1s2_Dec(b, MAX_acc, vi, vm, vf, si, sf);

    b = block_next(b);
  }
  
}

//   _____ _____ ____ _____   __  __       _
//  |_   _| ____/ ___|_   _| |  \/  | __ _(_)_ __
//    | | |  _| \___ \ | |   | |\/| |/ _` | | '_ \
//    | | | |___ ___) || |   | |  | | (_| | | | | |
//    |_| |_____|____/ |_|   |_|  |_|\__,_|_|_| |_|
//
#ifdef PROGRAM_LA_MAIN
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