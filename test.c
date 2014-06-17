#include "threadpool.h"
#include <stdio.h>

void sampleFunction(int n)
{
    printf("Sample function: %d\n", n);
}

void infiniteLoop()
{
    while (1);
}

int main ()
{
    pool_t* pool;
    int max_threads;
    
    pool = pool_init(100);
    printf("Adding 100 sample functions.\n");
    for (int i = 0; i < 100; i++) {
        pool_add_task(pool, &sampleFunction, i);
    }
    max_threads =  1 + 2 * (int)sysconf(_SC_NPROCESSORS_ONLN);

    printf("Adding %d infinite loop threads to create starvation.\n",max_threads);
    for (int i = 0; i < max_threads; i++) {
        pool_add_task(pool, &infiniteLoop, NULL);
    }
    
    printf("Adding 100 sample function that will starve.\n");
    for (int i = 0; i < 100; i++) {
        pool_add_task(pool, &sampleFunction, i);
    }

    printf("Set to infinite loop to see it starve. Press Ctrl+C to stop.\n");
    while (1);
    return 0;
}