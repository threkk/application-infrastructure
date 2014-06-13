#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct pool_task pool_task;

struct pool_task {
    void* (*function)(void *);
    void* args;
    // Here goes the timer.
    pool_task* prev;
    pool_task* next;
};

typedef struct pool_t {
    int max_threads;
    pthread_t *threads;
    pool_task *head;
    pool_task *tail;
} pool_t;

int pool_init(pool_t *pool);
int pool_add_task(pool_t *pool, void* (*function)(void *), void* args);
