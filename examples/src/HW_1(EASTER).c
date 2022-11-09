
// HW_1 possible implementations for void block_print(const block *b, FILE* out)

  // 1) --------------------------------------------
  /*if (!out)
    out = stdout; // printing output on the main output device for the session
                  // (whatever)
    char *point_descr = NULL;
  fprintf(out,
          "Line %zu, motion type: ", block->n); // printing size_t variables
  switch (block->type) {
  case RAPID:
    fprintf(out, "rapid\n");
    break;
  case LINE:
    fprintf(out, "linear\n");
    break;
  case ARC_CW:
    fprintf(out, "arc - clockwise\n");
    break;
  case ARC_CCW:
    fprintf(out, "arc - counter-clockwise\n");
    break;
  case NO_MOTION:
    fprintf(out, "no motion\n");
    break;
  }

  fprintf(out, "\tTool n°:     %zu\n", block->tool);
  fprintf(out, "\tFeedrate:    %8.3f\n", block->feedRateNom);
  fprintf(out, "\tSpindlerate:     %8.3f\n", block->spindleRate);

  pointInspection(block->target, &point_descr);
  fprintf(out, "\tTarget:      %s\n", point_descr);

  if (block->type == ARC_CW || block->type == ARC_CCW) {
    pointInspection(block->center, &point_descr);
    fprintf(out, "\tArc center: %s\n", point_descr);
    free(point_descr);
  }

  free(point_descr); // free the memory internally allocated
  */

  /* 2) ----------------------------------
  // writing the file
  char *filename = "block_file"; // char filename[] = "block_file";
  char *point_descr;
  out = fopen(filename, "w"); // open file in writing mode

  block_parse(block);
  printf("Block number: %lu\n", block->n);
  printf("Block type:   %u\n", block->type);

  pointInspection(block->target, &point_descr);
  printf("Target point: %s\n", point_descr);
  free(point_descr);

  pointInspection(block->delta, &point_descr);
  printf("Delta:        %s\n", point_descr);
  free(point_descr);

  printf("Tool n°:        %li\n", block->tool);
  printf("Feed rate:        %f\n", block->feedRate);
  printf("Spindle rate n°:        %li\n", block->spindleRate);

  if (block->type == ARC_CW && block->type == ARC_CCW) {
    pointInspection(block->center, &point_descr);
    printf("Arc center:        %s\n", point_descr);
    printf("I:                 %f\n", block->I);
    printf("J:                 %f\n", block->J);
    printf("Arc radius:        %f\n", block->radius);
    free(point_descr);
  }

  printf("Theta_0:           %f\n", block->theta_0);
  printf("Arc length:        %f\n", block_arcLength(block));
  printf("Actual acceleration:        %f\n", block->actualAcceleration);

  printf("------------------------------------\n");

*/



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

  char l1[] = "N01 G00 X100 Y20 Z100 T1 S5000";
  char l2[] = "N02 G01 Z20 F1000";
  char l3[] = "N03 G03 Y40 J10";

  b1 = block_new(l1, NULL, cfg);
  block_parse(b1);
  b2 = block_new(l2, b1, cfg);
  block_parse(b2);
  b3 = block_new(l3, b2, cfg);
  block_parse(b3);


  block_print(b1, NULL);
  block_print(b2, NULL);
  block_print(b3, NULL);
  
  block_free(b1);
  block_free(b2);
  block_free(b3);
  return 0;
}
#endif