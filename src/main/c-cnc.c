#include "../defines.h"
#include "../machine.h"
#include "../program.h"
#include "../block.h"
#include "../point.h"
#include "../fsm.h"

// =============================================================================
//  _____         _   _              
// |_   _|__  ___| |_(_)_ __   __ _  
//   | |/ _ \/ __| __| | '_ \ / _` | 
//   | |  __/\__ \ |_| | | | | (_| | 
//   |_|\___||___/\__|_|_| |_|\__, | 
//                            |___/  
//all the developed classes!!!

// =============================================================================


/* 
x HW2 completion:
 load a file one line at a time and print out the result of the interpolation for G00 and possibly for G01, G02 and G03 commands: in particular it creates some blocks of G-code (loading the G-code file and running it) and for a line or arc move make a loop stepping in time with the given sampling time and print out a table of time, lambda, feedrate, x, y, z.
 We parse the block, for every block we calculate the velocity profile then 
 at every time step we calculate the lambda (t = currentTime = time elapsed since the beginning of each block). THen we interpolate for calculating the desired position of 3 axes which will be sent via MQTT to the machine (machine_sync) */


/*Our code is going to work as a publisher publishing the setPoints (our machine acts a subscriber): we are going to publish on one single topic the setPoint holding the values for x, y, z separated by commas*/  

#define eprintf(...) fprintf(stderr, __VA_ARGS__)


#if 1
int main(int argc, char const *argv[]) {
  ccnc_state_data_t state_data = {
    .ini_file = "settings.ini",
    .prog_file = argv[1],
    .machine = NULL,
    .prog = NULL
  };
  ccnc_state_t cur_state = CCNC_STATE_INIT;
  do {
    cur_state = ccnc_run_state(cur_state, &state_data);
    wait_next(machine_tq(state_data.machine) * 1E9 / machine_rt_pacing(state_data.machine)); //waiting for the next occurence of a time step multiple in order to go to the run the next machine state
  } while (cur_state != CCNC_STATE_STOP);
  ccnc_run_state(cur_state, &state_data);
  return 0;
}

// AS EXERCISE WE CAN try to:
// *make a chart / 3D chart of run.csv file to plot the nominal trajectory and see if it is actually what we expected
// * get/save data from Simulink to the workspace
// * plot the differences between the setPoint (nominal positions) sent by the CNC exe (CNC controller) to Simulink and the actual positions reached by the simulator in Simulink


#else // NAIF APPROACH
int main(int argc, char const *argv[]) {
  point_t *sp = NULL;
  block_t *b = NULL;
  program_t *p = NULL;
  data_t t, tt, tq, lambda, f;
  machine_t *machine = machine_new("settings.ini");
  if (!machine) {
    eprintf("Error creating machine instance\n");
    exit(EXIT_FAILURE);
  }
  tq = machine_tq(machine);

  p = program_new(argv[1]);
  if (!p) {
    eprintf("Could not create program, exiting.\n");
    exit(EXIT_FAILURE);
  }
  if (program_parse(p, machine) == EXIT_FAILURE) {
    eprintf("Could not parse program in %s, exiting.\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  program_print(p, stderr);
  
  machine_connect(machine, NULL);

  // Main loop
  // loading one block at a time and for each block we do the interpolation

  printf("n,t,tt,lambda,s,f,x,y,z\n");
  tt = 0;
  while ((b = program_next(p))) {
    if (block_type(b) == RAPID || block_type(b) > ARC_CCW) {
      continue;
    }
    eprintf("Interpolating the block %s\n", block_line(b));
    // interpolation loop
    // careful: we check t <= block_dt(b) + tq/2.0 for double values are
    // never exact, and we may have that adding many tq carries over a small
    // error that accumulates and may result in n*tb being greater than Dt
    // (if so, we would miss the last step)
    for (t = 0; t <= block_dt(b) + tq/2.0; t += tq, tt += tq) {
      lambda = block_lambda(b, t, &f);
      sp = block_interpolate(b, lambda);
      if (!sp) continue;
      printf("%lu,%f,%f,%f,%f,%f,%f,%f,%f\n", block_n(b), t, tt,
        lambda, lambda * block_length(b), f,
        point_x(sp), point_y(sp), point_z(sp));
      machine_sync(machine); // takes the current setPoint set by blockInterpolate and sends it via MQTT to the machine itself
      wait_next(5e6);
    }
  }


  machine_free(machine);
  program_free(p);
  return 0;
}
#endif