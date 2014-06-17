#include "threadpool.h"


static void pool_launcher(pool_t *pool)
{
    pool_task* task = NULL;
    
    // Infinite loop.
    while (pool->alive)
    {
        pthread_mutex_lock(&pool->lock);
        // If there is something in the list.
        if (pool->head != NULL) {
            task = pool->head;
            // If has next.
            if (pool->head->next != NULL) {
                // The new head is the next.
                pool->head = pool->head->next;
            } else {
                // The queue is empty.
                pool->head = NULL;
            }
        }
        pthread_mutex_unlock(&pool->lock);
        
        // Execute the task in the head of the list.
        if (task) {
            (*(task->function))(task->args);
            task = NULL;
        } else {
            // or block the thread until there is something in the list.
            pthread_mutex_lock(&pool->stop);
        }
        
    }
    pthread_exit(NULL);
}

static void pool_manager(pool_t* pool)
{
    // Looping variable.
    pool_task* loop = NULL;
    while (pool->alive) {
        pthread_mutex_lock(&pool->lock);
        // Cleaning the head.
        if (pool->head != NULL)
        {
            // If the head passed the deadline and was launched.
            if (pool->head->deadline == -1) {
                // Destroy the head.
                pool->head = pool->head->next;
            }
            
            loop =  pool->head;
            while (loop != NULL) {
                // If a task has passed its deadline.
                if (loop->deadline > pool->deadline) {
                    // Set to destroy and launch a thread for it.
                    loop->deadline = -1;
                    pthread_t thread;
                    int rc = pthread_create(&thread, NULL, loop->function, (void*)loop->args);
                    if (rc)
                    {
                        fprintf(stderr, "pool_manager: Error creating an extra thread.\n");
                    }
                } else {
                    // If is not set to destroy, increase the counter.
                    if (loop->deadline != -1) {
                        loop->deadline++;
                    }
                }
                
                // Cleaning.
                if (loop->next != NULL) {
                    // If the next task is set to destroy
                    if(loop->next->deadline == -1) {
                        // Skip it and destroy.
                        pool_task* next = loop->next;
                        loop->next = loop->next->next;
                        free(next);
                    }
                }
                loop = loop->next;
            }
        }
        pthread_mutex_unlock(&pool->lock);
    }

}

pool_t* pool_init(int deadline)
{
    pool_t* pool;
    int max_threads;

    pool = (pool_t *)malloc(sizeof(pool_t));
    pool->head = NULL;
    pool->tail = NULL;
    pool->alive = 1;
    
    // Only positive numbers
    if (deadline > 0) {
        pool->deadline = deadline;

    } else {
        pool->deadline = 100;
    }
    
    pthread_mutex_init(&pool->lock, NULL);
    pthread_mutex_init(&pool->stop, NULL);
    
    max_threads =  1 + 2 * (int)sysconf(_SC_NPROCESSORS_ONLN);
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_threads + 1);
    
    for(int i = 0; i < max_threads; i++)
    {
        int rc = pthread_create(&pool->threads[i], NULL, &pool_launcher, (void *)pool);
        if (rc)
        {
            fprintf(stderr, "pool_init: Error creating a pool launcher.\n");
        }
    }
    
    int rc = pthread_create(&pool->threads[max_threads], NULL, &pool_manager, (void *)pool);
    if (rc)
    {
        fprintf(stderr, "pool_init: Error creating the pool manager.\n");
    }
    
    return pool;
}

void pool_add_task(pool_t *pool, void* (*function)(void *), void* args)
{
    pool_task *task;
    task = (pool_task *)malloc(sizeof(pool_task));
    task->function = function;
    task->args = args;
    task->deadline = 0;

    // Update the queue of tasks.
    task->next = NULL;
    pthread_mutex_lock(&pool->lock);
    if (pool->head == NULL) {
        pool->head = task;
    }
    
    if(pool->tail != NULL)
    {
        pool->tail->next = task;
    }
    pool->tail = task;
    pthread_mutex_unlock(&pool->lock);
    // Added a new task. Unlock possible waiting threads.
    pthread_mutex_unlock(&pool->stop);
}

void pool_destroy(pool_t *pool)
{
    int max_threads;
    
    // Let the threads die.
    pool->alive = 0;
    
    // Clean the list.
    pool_task* loop = NULL;

    pthread_mutex_lock(&pool->lock);
    if (pool->head != NULL)
    {
        loop =  pool->head;
        while (loop != NULL)
        {
            if (loop->next != NULL)
            {
                free(loop);
                loop = loop->next;
            } else {
                loop = NULL;
            }
        }
    }
    pthread_mutex_unlock(&pool->lock);
    
    // Kill remaining threads.
    max_threads =  1 + 2 * (int)sysconf(_SC_NPROCESSORS_ONLN);
    for(int i = 0; i < max_threads; i++)
    {
        int rc = pthread_kill(pool->threads[i], SIGTERM);
        if (rc == EINVAL)
        {
            fprintf(stderr, "pool_destroy: Invalid signal.\n");
        }
    }
    
    // Destroy the mutex.
    pthread_mutex_unlock(&pool->lock);
    pthread_mutex_destroy(&pool->lock);
    pthread_mutex_unlock(&pool->stop);
    pthread_mutex_destroy(&pool->stop);
    
    // Destroy the struct.
    free(pool);
}