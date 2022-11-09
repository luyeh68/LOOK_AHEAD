#include "inout_p_mine.h"

// -------------- entry point -------------

/*
int main() {
    int n = 10; // array size

    //------ First example ----------- for declaring arrays
    float *a;
    printf("A address is: %p\n", a);
    a = f1(n);
    array_print(a, n);
    printf("A address is: %p\n", a);
    printf("\n");

    // ------- Second example ------- 
    float *b = NULL; //pointer value = 0x00
    f2(b, n);
    printf("B address is: %p\n", b);
    //array_print(b, n);
    printf("\n");

    // ------- Third Example (BEST ONE) ---------   
    float *c = NULL;
    f3(&c, n);
    printf("C address is: %p\n", c);
    array_print(c, n);
    printf("\n");

    // free memory
    free(a);
    free(b);
    free(c);

    return 0;
} // end of MAIN{}
*/

// --------------------------- DEFINITIONS ---------------------------

void array_print(float *array, int length)
{
    printf("[");
    for (int i = 0; i < length; i++)
    {
        printf(" %f ", array[i]);
    }
    printf("]\n");
    
}

float *f1(int size)
{
    float *pt = malloc(size * sizeof(float)); // to allocate space to pt
    printf("f1(%i) - pt address is: %p\n", size, pt); // p is the pointer placeholder
    for (int i = 0; i < size; i++)
    {
        pt[i] = i;
    }
    return pt;
}

void f2(float *pt, int size)
{
    pt = malloc(size * sizeof(float));
    printf("f2(%i) - pt address is: %p\n", size, pt);

    for (int i = 0; i < size; i++)
        pt[i] = i;

    array_print(pt, size);
}

void f3(float **pt, int size)
{
    *pt = malloc(size * sizeof(float));
    printf("f3(%i) - pt address is: %p\n", size, *pt);

    for (int i = 0; i < size; i++)
        (*pt)[i] = i;

}