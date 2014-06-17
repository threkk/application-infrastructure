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
    int deadline;
    pool_task* next;
};

typedef struct pool_t {
    pthread_t *threads;
    pthread_mutex_t lock;
    pthread_mutex_t stop;
    int alive;
    int deadline;
    pool_task *head;
    pool_task *tail;
} pool_t;

pool_t* pool_init(int deadline);
void pool_add_task(pool_t *pool, void* (*function)(void *), void* args);
void pool_destroy(pool_t *pool);
