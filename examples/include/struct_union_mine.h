#include <stdio.h>
#include <stdlib.h>

// Macro example for constant definition. 
#define ARY_LEN 6

/* struct name {
   double x, y, z;
}*/

// is equal to the following

// typedef struct {
//   double x, y, z;
// } point_t;

//typedef is used to create type aliases.
typedef double data_t;

// type - struct NO_NAME {...} - point_t
typedef struct
{
   data_t x, y, z;
   /* 
   struct my_struct {
      float x, y, z;
   };
   */
} point_t; 

//we are substituting, when using point_t struct, only the struct (point_t) --> simple struct

// Alternatively 

// type - struct point_t{...} - point_t
// typedef struct point_t
// {
//     data_t x, y, z;
// } point_t;

// here we are substituting point_t with a struct named point_t --> struct which name is point_t

typedef union another_union {
   double x, y, z;
} my_union;

// everytime I use my_union I am allocating another union (another_union) which
// has a different name

typedef union 
{
   float x, y, z;
   char str[20];
} my_union_;


void point_print(point_t p);

void point_print_withPointers(point_t *a);

void array_print(data_t a[], int len);

void point_print_withUnions(my_union *a);