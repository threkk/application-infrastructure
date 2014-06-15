#include "threadpool.h"


void pool_launcher(pool_t *pool)
{
    printf("Executing launcher\n");
 
    // Pick tasks from the queue
    // TODO: Update the queue.
    while (1) {
        if (pool->head != NULL) {
            (*(pool->head->function))(pool->head->args);
        }
    }
    pthread_exit(NULL);
}



pool_t* pool_init()
{
    pool_t* pool;
    pool = (pool_t *)malloc(sizeof(pool_t));
    pool->head = NULL;
    pool->tail = NULL;
    
    int max_threads =  1 + 2 * 1; //(int)sysconf(_SC_NPROCESSORS_ONLN);
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_threads);


    for(int i = 0; i < max_threads; i++)
    {
        int rc = pthread_create(&pool->threads[i], NULL, &pool_launcher, (void *)pool);
        if (rc)
        {
            printf("Error creating the pool launcher.\n");
        }
        
    }
    
    printf("Pool_init executed\n");
    return pool; //Future error handling
}

int pool_add_task(pool_t *pool, void* (*function)(void *), void* args)
{
    pool_task *task;
    task = (pool_task *)malloc(sizeof(pool_task));
    task->function = function;
    task->args = args;

    // Update the queue of tasks.
    task->prev = NULL;
    task->next = NULL;
    
    if (pool->head == NULL) {
        pool->head = task;
    }
    
    if(pool->tail != NULL)
    {
        task->prev = pool->head;
        pool->tail->next = task;
    }
    pool->tail = task;
    
    printf("Task added to the queue\n");
    return 0;
}

int sum2(int a)
{
    return a + 2;
}

void simple()
{
    printf("Ejemplo de function\n");
}

int main ()
{
    pool_t* pool;
    pool = pool_init();
    pool_add_task(pool, &simple, NULL);
    pool_add_task(pool, &sum2, 4);
    pool_add_task(pool, &simple, NULL);
    //(*(pool->head->function))(pool->head->args);
    while(1);
    return 0; 
}
