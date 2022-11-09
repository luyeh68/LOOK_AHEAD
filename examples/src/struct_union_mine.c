#include "struct_union_mine.h"

void point_print(point_t p);

void point_print_withPointers(point_t *a);

void array_print(data_t a[], int len);

void point_print_withUnions(my_union *a);

int main(){
   printf("Hello world\n");
   int myArray[] = {1,2,3,4,12,23,2111};
   // 2 possible initializers for structs:
   // point_t pt = {x:0, y:0, z:0}; ------- old version
   // or just: point_t pt;
   // or:
   point_t pt = {.x = 0, .y = 0, .z = 0 /*, {.x = 1, .y = 2, .z = 3}*/ };
   point_print(pt); //print points
   printf("Modify pt.x\n");
   pt.x = 123.456; // modify one value
   point_print(pt);
   printf("New pt.x = %f\n", pt.x);

   point_t *pt_ptr = &pt;
   pt.y = 20;
   pt_ptr->x = 10;

   point_print_withPointers(pt_ptr);
   // ---------------- Array of structs ---------------- //
   // 1. Exceed predefined array size. This does not work.
   // data_t ary1[ARY_LEN] = {1, 2, 3, 4, 5, 6, 7}; // cannot exceed array size


   my_union another_pt; // union saves space: it allocates just the space for the biggest data_type not do the allocation as for struct that allocate space for each data (double x, y and z)

   another_pt.x = 10;
   another_pt.y = 20;

   printf("Another_pt union size: %lu bytes\n", sizeof(another_pt));
   printf("Pt union size:         %lu bytes\n", sizeof(pt));

   point_print_withUnions(&another_pt);

   // or

   /*
   printf("another_pt.x %f\n", another_pt.x);
   printf("another_pt.y %f\n", another_pt.y);
   */

   // UNION: continuos chunk of memory
   // STRUCT: NOT '' 

   // array of struct
   data_t ary[ARY_LEN] = {10, 2};
   data_t ary_[] = {0, 0, 0, 0, 0}; // to create automatically an array of struct of size 5

   array_print(ary, ARY_LEN);

   /*
   {
      int sum = 10000;
   }

   printf("Sum is: %i\n", sum);*/

   return 0;  
}

void point_print(point_t p){
   printf("( %8.3f %f %f )\n", p.x, p.y, p.z); //%.3f tells how many digits to put after and before comma 
   // p.x = 2;  has only local scope (visiblity)
}

void point_print_withPointers(point_t *a){
   printf("( %8.3f %f %f )\n", a->x, a->y, a->z); //%.3f tells how many digits to put after and before comma 
   // a->x = 2;  has only local scope (visiblity)
}

void array_print(data_t a[], int len)
{

   printf("Array print: {");
   for (int i = 0; i < len - 1; i++)
   {
      printf("a[%d] = %f, ", i, a[i]);
      // printf("a[%d] = %f\n, ", i, a[i]); without the starting and final{} 
   }
   printf("a[%d] = %f", len - 1, a[len - 1]);
   printf("}\n");
}

void point_print_withUnions(my_union *a){
   printf("( %8.3f %f %f )\n", a->x, a->y, a->z); //%.3f tells how many digits to put after and before comma 
   // a->x = 2;  has only local scope (visibility)
    
}