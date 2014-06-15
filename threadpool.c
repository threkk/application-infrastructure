#include "threadpool.h"


static void pool_launcher(pool_t *pool)
{
    printf("Executing launcher\n");
 
    // Pick tasks from the queue
    while (1) {
        pthread_mutex_lock(&pool->lock);
        if (pool->head != NULL) {
            pool_task task = *pool->head;
            if (pool->head->next != NULL) {
                pool->head = pool->head->next;
                pool->head->prev = NULL;
            } else {
                pool->head = NULL;
            }
            (*(task.function))(task.args);
        }
        pthread_mutex_unlock(&pool->lock);
    }
    pthread_exit(NULL);
}



pool_t* pool_init()
{
    pool_t* pool;
    pool = (pool_t *)malloc(sizeof(pool_t));
    pool->head = NULL;
    pool->tail = NULL;
    
    pthread_mutex_init(&pool->lock, NULL);
    
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
    pthread_mutex_lock(&pool->lock);
    if (pool->head == NULL) {
        pool->head = task;
    }
    
    if(pool->tail != NULL)
    {
        task->prev = pool->head;
        pool->tail->next = task;
    }
    pool->tail = task;
    pthread_mutex_unlock(&pool->lock);
    printf("Task added to the queue\n");
    return 0;
}

void simple(int n)
{
    printf("Ejemplo de function %d\n", n);
}

int main ()
{
    pool_t* pool;
    pool = pool_init();
    pool_add_task(pool, &simple, 1);
    pool_add_task(pool, &simple, 2);
    pool_add_task(pool, &simple, 3);
    while(1);
    return 0; 
}
