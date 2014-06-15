#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct pool_task pool_task;

struct pool_task {
    void* (*function)(void *);
    void* args;
    // Here goes the timer for the deadline.
    pool_task* prev;
    pool_task* next;
};

typedef struct pool_t {
    pthread_t *threads;
    pool_task *head;
    pool_task *tail;
} pool_t;

pool_t* pool_init();
int pool_add_task(pool_t *pool, void* (*function)(void *), void* args);
