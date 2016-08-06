#include <iostream>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    int n = 10;
    int *stack = (int *) malloc(n * sizeof(int));
    int *heap = new int[n];
    int *leak = new int[n];

    /* uninitialized memory */
    if (stack[0] == 42) {
        std::cout << "42";
    }

    /* out of bound access */
    leak[n] = 42;

    /* access after free */
    delete[] heap;
    heap[0] = 42;

    return 0;
}

