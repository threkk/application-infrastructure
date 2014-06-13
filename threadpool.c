#include "threadpool.h"

void pool_manager(pool_t* pool)
{

}

int pool_init(pool_t *pool)
{
    pool->max_threads = sysconf(_SC_NPROCESSORS_ONLN);
    pool->head = 0;
    pool->tail = 0;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * pool->max_threads);

    // More things to do.

    return 0;
}

int pool_add_task(pool_t *pool, void* (*function)(void *), void* args)
{
    pool_task *task;
    task->function = function;
    task->args = args;

    // Update the queue of tasks.
    if(pool->tail == 0)
    {
        pool->tail = task;
    }

    if(pool->head != 0)
    {
        task->next = pool->head;
        pool->head->prev = task;
    }

    task->prev = 0;
    pool->head = task;
    return 0;
}

int sum(int a, int b)
{
    return a + b;
}

void simple()
{
    printf("Ejemplo de function\n");
}

int main ()
{
    return 0; 
}
