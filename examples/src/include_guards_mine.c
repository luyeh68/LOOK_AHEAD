#include <stdio.h>
#include <stdlib.h>

void print_args(int argc, const char *argv[])
{
    printf("Number of args: #%i\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("Args #%i is: %s\n", i, argv[i]); // s = string placeholder
    }
    
}

int main(int argc, const char *argv[]) // n° of arguments - the content of my arguments
{
    print_args(argc, argv);
    return 0;
}