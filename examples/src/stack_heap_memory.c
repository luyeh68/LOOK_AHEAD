// Program to calculate the sum of n numbers entered by the user

#include <stdio.h>
#include <stdlib.h> //malloc(), free(), calloc() ...

int main() 
{
    //  ______ _             _
    // /  ____| |           | |
    // \ '___.| |_ __ _   __| | __  _ __ ___   ___ _ __ ___   ___  _ __ _   _
    //  \ __. \ __/ _ '|/ __| |/ / | '_ ' _ \ / _ \ '_ ' _ \ / _ \| '__| | | |
    // /\__/  / || (_| | (__|   <  | | | | | |  __/ | | | | | ( ) | |  | |_| |
    // \_____/\__\___,_|\___|_|\_\ |_| |_| |_|\___|_| |_| |_|\___/|_|  \__,| |
    //                                                                   __/ |
    //                                                                  |___/
    
    //stored in the stack memory program

    int myArray[10];
    printf("Size of my array: %li bytes\n", sizeof(myArray));  //stack memory

    //Trick ""segmentation fault 11 error"
    // If I stack memory in ~8Mb then I need at least:
    // 8192 bytes / 4 bytes = 2048000 elements in order to fill all the stack memory
    //with an array of integers.

    //Uncomment the following lines in order to trick "segmentation fault 11 error"
    //int array__too__big[3000000];
    //printf("Size of array__too__big: %li bytes\n", sizeof(array__too__big)); //stack overflow memory

    //  _   _ 
    // | | | |
    // | |_| | ___  __ _ _ __     _ __ ___   ___ _ __ ___   ___  _ __ _   _
    // |  _  |/ _ \/ _' | '_ \   | '_ ' _ \ / _ \ '_ ' _ \ /   \| '__| | | |
    // | | | |  __/ (_| | |_) |  | | | | | |  __/ | | | | | ( ) | |  | |_| |
    // \_| |_/\___|\__,_| .__/   |_| |_| |_|\___|_| |_| |_|\___/|_|  \___, |
    //                  | |                                           __/  |
    //                  |_|                                          |____/ 

    int n = 1000; // number of elements
    // int n = 3000000 --> more than available stack size
    printf("Number of elements: %i\n", n);

    int *ptr = (int*) malloc(n * sizeof(int)); //(int*) -->casting operator 

    printf("Size of int:  %li bytes\n", sizeof(int));
    printf("Size of *ptr:  %li bytes\n", sizeof(ptr));

    //if memory cannot be allocated
    if(ptr == NULL){
      printf("Error! Memory not allocated. \n");
      exit(0);
    }

    printf("Fill array with some elements\n");
    for (int i = 0; i < n; ++i)
    {
      // 1. Method
      //ptr[i] = i * 2;

      // 2. Method
      *(ptr + i) = i * 2;
    }

    // Deallocating the memory
    free(ptr);

    return 0;
}