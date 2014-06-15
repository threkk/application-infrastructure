#include "threadpool.h"


static void pool_launcher(pool_t *pool)
{
    printf("Executing manager\n");
    
    pool_task* task = NULL;
    while (1)
    {
        pthread_mutex_lock(&pool->lock);
        if (pool->head != NULL) {
            task = pool->head;
            if (pool->head->next != NULL) {
                pool->head = pool->head->next;
                //pool->head->prev = NULL;
            } else {
                pool->head = NULL;
            }
        }
        pthread_mutex_unlock(&pool->lock);
        
        if (task) {
            (*(task->function))(task->args);
            task = NULL;
        } else {
            pthread_mutex_lock(&pool->stop);
        }
        
    }
    pthread_exit(NULL);
}

static void pool_manager(pool_t* pool)
{
    pool_task* loop = NULL;
    while (1) {
        pthread_mutex_lock(&pool->lock);
        if (pool->head != NULL)
        {
            if (pool->head->deadline == -1) {
                pool->head = pool->head->next;
            }
            loop =  pool->head;
            while (loop != NULL) {
                if (loop->deadline > POOL_DEADLINE) {
                    loop->deadline = -1;
                    pthread_t thread;
                    int rc = pthread_create(&thread, NULL, loop->function, (void*)loop->args);
                    if (rc)
                    {
                        printf("Error creating the  extra thread.\n");
                    }
                } else {
                    if (loop->deadline != -1) {
                        loop->deadline++;
                    }
                }
                
                if (loop->next != NULL) {
                    if(loop->next->deadline == -1) {
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

pool_t* pool_init()
{
    pool_t* pool;
    int max_threads;

    pool = (pool_t *)malloc(sizeof(pool_t));
    pool->head = NULL;
    pool->tail = NULL;
    
    pthread_mutex_init(&pool->lock, NULL);
    pthread_mutex_init(&pool->stop, NULL);
    
    max_threads =  1 + 2 * 1; //(int)sysconf(_SC_NPROCESSORS_ONLN);
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_threads + 1);

    for(int i = 0; i < max_threads; i++)
    {
        int rc = pthread_create(&pool->threads[i], NULL, &pool_launcher, (void *)pool);
        if (rc)
        {
            printf("Error creating the pool launcher.\n");
        }
    }
    
    int rc = pthread_create(&pool->threads[max_threads], NULL, &pool_manager, (void *)pool);
    if (rc)
    {
        printf("Error creating the pool manager.\n");
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
    task->deadline = 0;

    // Update the queue of tasks.
    //task->prev = NULL;
    task->next = NULL;
    pthread_mutex_lock(&pool->lock);
    if (pool->head == NULL) {
        pool->head = task;
    }
    
    if(pool->tail != NULL)
    {
        //task->prev = pool->head;
        pool->tail->next = task;
    }
    pool->tail = task;
    pthread_mutex_unlock(&pool->lock);
    pthread_mutex_unlock(&pool->stop);

    printf("Task added to the queue\n");
    return 0;
}

void sample(int n)
{
    printf("Funcion example: %d\n", n);
}

void infiniteLoop()
{
    while (1);
}

int main ()
{
    pool_t* pool;
    pool = pool_init();
    pool_add_task(pool, &infiniteLoop, NULL);
    pool_add_task(pool, &infiniteLoop, NULL);
    pool_add_task(pool, &infiniteLoop, NULL);
    for (int i = 0; i < 100; i++) {
        pool_add_task(pool, &sample, i);
    }
    sleep(1);
    pool_add_task(pool, &sample, 100);
    while (1);
    return 0; 
}
