#include "RequestManager.h"
struct requestManager_t
{
    Queue run_queue;
    Queue wait_queue;
    int queue_size;
    int keep_running;
    OverLoadPolicy policy;
    int max_size;

    pthread_cond_t sleep_cond;
    pthread_mutex_t sleep_lock;

    pthread_cond_t block_main_cond;
    pthread_mutex_t block_main_lock;

    int number_of_workers;
    int current_thread_id;
    pthread_mutex_t get_id_lock;
    handlerThreadStats* threads_stats_array;
};

RequestManager RMInit(int queue_size, OverLoadPolicy policy, int max_size, int num_of_workers)
{
    RequestManager rm = malloc(sizeof(*rm));
    if(rm == NULL)
    {
        return NULL;
    }
    rm->queue_size = queue_size;
    rm->keep_running = 1;
    rm->policy = policy;
    rm->max_size = max_size;
    rm->run_queue = QueueInit();
    rm->wait_queue = QueueInit();
    pthread_cond_init(&(rm->sleep_cond),NULL);
    if(pthread_mutex_init(&(rm->sleep_lock), NULL) != 0)
    {
        printf("pthread_mutex_init failed\n");
        exit(1);
    }
    pthread_cond_init(&(rm->block_main_cond),NULL);
    if(pthread_mutex_init(&(rm->block_main_lock), NULL) != 0)
    {
        printf("pthread_mutex_init failed\n");
        pthread_mutex_destroy(&(rm->sleep_lock));
        exit(1);
    }

    rm->number_of_workers = num_of_workers;
    rm->current_thread_id = 0;
    if(pthread_mutex_init(&(rm->get_id_lock), NULL) != 0)
    {
        printf("pthread_mutex_init failed\n");
        pthread_mutex_destroy(&(rm->sleep_lock));
        pthread_mutex_destroy(&(rm->block_main_lock));
        exit(1);
    }
    rm->threads_stats_array = malloc(rm->number_of_workers * sizeof(*rm->threads_stats_array));
    if(rm->threads_stats_array == NULL)
    {
        //piazza epic #438 states that this won't be checked
        printf("Server is out of memory\n");
        pthread_mutex_destroy(&(rm->sleep_lock));
        pthread_mutex_destroy(&(rm->block_main_lock));
        pthread_mutex_destroy(&(rm->get_id_lock));
        exit(1);
    }
    for(int i=0; i < rm->number_of_workers; i++)
    {
        (rm->threads_stats_array)[i] = handlerThreadStatsInit(i);
        if((rm->threads_stats_array)[i]  == NULL)
        {
            //piazza epic #438 states that this won't be checked
            for(int j=0; j < i; j++)
            {
                handlerThreadStatsDestroy((rm->threads_stats_array)[j]);
            }
            free(rm->threads_stats_array);
            printf("Server is out of memory\n");
            pthread_mutex_destroy(&(rm->sleep_lock));
            pthread_mutex_destroy(&(rm->block_main_lock));
            pthread_mutex_destroy(&(rm->get_id_lock));
            exit(1);
        }
    }
    return rm;
}

void RMDestroy(RequestManager rm)
{
    QueueDestroy(rm->run_queue);
    QueueDestroy(rm->wait_queue);
    pthread_cond_destroy(&(rm->sleep_cond));
    pthread_mutex_destroy(&(rm->sleep_lock));
    pthread_cond_destroy(&(rm->block_main_cond));
    pthread_mutex_destroy(&(rm->block_main_lock));
    pthread_mutex_destroy(&(rm->get_id_lock));
    for(int j=0; j < rm->number_of_workers; j++)
    {
        handlerThreadStatsDestroy((rm->threads_stats_array)[j]);
    }
    free(rm->threads_stats_array);
    free(rm);
}

int RMCheckOverLoad(RequestManager rm)
{
    //pthread_mutex_lock(&(rm->sleep_lock));
    int number_of_requests = QueueGetSize(rm->run_queue) + QueueGetSize(rm->wait_queue);
    //printf("\n\nCheckOverLoad\n");
    //printf("requests: %d\n", number_of_requests);
    //printf("requests in run queue: %d\n", QueueGetSize(rm->run_queue));
    //printf("requests in wait queue: %d\n", QueueGetSize(rm->wait_queue));
    //QueuePrint(rm->run_queue);
    int queue_size = rm->queue_size;
    //pthread_mutex_unlock(&(rm->sleep_lock));
    return (number_of_requests >= queue_size);
}
void BlockMainThread(RequestManager rm)
{
    pthread_mutex_lock(&(rm->block_main_lock));
    pthread_mutex_unlock(&(rm->sleep_lock));
    pthread_cond_wait(&(rm->block_main_cond), &(rm->block_main_lock));
    pthread_mutex_lock(&(rm->sleep_lock));
    pthread_mutex_unlock(&(rm->block_main_lock));
}

void RMHandleRequest(RequestManager rm, int req_fd, struct timeval* arrival_time)
{
    Request req = RequestInit(req_fd, arrival_time);
    if(req == NULL)
    {
        //piazza epic #438 states that this won't be checked
        printf("Server is out of memory\n");
        exit(1);
    }
    //pthread_mutex_lock(&(rm->sleep_lock));
    enqueue(rm->wait_queue, req);
    pthread_mutex_unlock(&(rm->sleep_lock));
    pthread_cond_signal(&(rm->sleep_cond));
    //printf("\n\nHandle Request\n");
    //printf("requests in run queue: %d\n", QueueGetSize(rm->run_queue));
    //printf("requests in wait queue: %d\n", QueueGetSize(rm->wait_queue));
    //QueuePrint(rm->run_queue);
}

void* work(void* void_rm)
{
    RequestManager rm = (RequestManager)void_rm;
    pthread_mutex_lock(&(rm->get_id_lock));
    int thread_id = rm->current_thread_id++;
    pthread_mutex_unlock(&(rm->get_id_lock));
    while(rm->keep_running)
    {
        sleepWorker(rm);
        //printf("Thread number %lu woke up!!!\n", pthread_self());
        //pthread_mutex_lock(&(rm->sleep_lock));
        //printf("inside work before calling RMGetRequest\n");
        Request req = RMGetRequest(rm);
        //printf("inside work after calling RMGetRequest\n");
        // TODO: need to edit the start working time on the request here
        if (RequestSetDispatchInterval(req) == 0)
        {
            printf("Server is out of memory\n");
            exit(1);
        }
        pthread_mutex_unlock(&(rm->sleep_lock));

        int req_fd = RequestGetFD(req);
        if(req_fd == -1)
        {
            exit(1);
        }
        requestHandle(req, rm->threads_stats_array[thread_id]);
        Close(req_fd);
        RMFinishJob(rm,req);
    }
    return NULL;
}

void sleepWorker(RequestManager rm)
{
    pthread_mutex_lock(&(rm->sleep_lock));
    while(QueueGetSize(rm->wait_queue) == 0)
    {
        pthread_cond_wait(&(rm->sleep_cond), &(rm->sleep_lock));
    }
    //pthread_mutex_unlock(&(rm->sleep_lock));  //TODO: check if we should delete this line
}

Request RMGetRequest(RequestManager rm)
{
    //printf("inside RMGetRequest\n");
    Request req = dequeue(rm->wait_queue);
    //printf("\n\nGet Request\n");
    //printf("thread pid: %lu\n", pthread_self());
    //printf("request fd: %d\n", RequestGetFD(req));
    enqueue(rm->run_queue, req);
    return req;
}

void RMFinishJob(RequestManager rm, Request req)
{
    pthread_mutex_lock(&(rm->sleep_lock));
    //printf("\n\nFinish Job part 1\n");
    //printf("requests in run queue: %d\n", QueueGetSize(rm->run_queue));
    //printf("requests in wait queue: %d\n", QueueGetSize(rm->wait_queue));
    //QueuePrint(rm->run_queue);
    QueueRemoveRequest(rm->run_queue, req);
    //printf("\n\nFinish Job part 2\n");
    //printf("requests in run queue: %d\n", QueueGetSize(rm->run_queue));
    //printf("requests in wait queue: %d\n", QueueGetSize(rm->wait_queue));
    //QueuePrint(rm->run_queue);
    int number_of_requests = QueueGetSize(rm->run_queue) + QueueGetSize(rm->wait_queue);
    pthread_mutex_unlock(&(rm->sleep_lock));
    if((rm->policy == BlockFlush) && (number_of_requests > 0))
    {
        return;
    }
    pthread_cond_signal(&(rm->block_main_cond));
}

int DropOldestRequest(RequestManager rm)
{
    //pthread_mutex_lock(&(rm->sleep_lock));
    if (QueueGetSize(rm->wait_queue) == 0)
    {
        //pthread_mutex_unlock(&(rm->sleep_lock));
        return 1;
    }
    Request req = dequeue(rm->wait_queue);
    //pthread_mutex_unlock(&(rm->sleep_lock));
    int fd = RequestGetFD(req);
    if (fd != -1)
    {
        Close(fd);
    }
    RequestDestroy(req);
    return 0;
}

void IncQueueSize(RequestManager rm)
{
    if(rm->queue_size >= rm->max_size)
    {
        return;
    }
    rm->queue_size++;
}

int DropRandomRequests(RequestManager rm)
{
    //pthread_mutex_lock(&(rm->sleep_lock));
    int number_of_waiting_requests = QueueGetSize(rm->wait_queue);
    if (number_of_waiting_requests == 0)
    {
        //pthread_mutex_unlock(&(rm->sleep_lock));
        return 1;
    }
    int requests_to_drop = number_of_waiting_requests/2 + number_of_waiting_requests % 2;
    for(int i=0; i<requests_to_drop; i++)
    {
        int n = rand() % QueueGetSize(rm->wait_queue);
        int fd = QueueRemoveNthRequest(rm->wait_queue, n);
        if (fd != -1)
        {
            Close(fd);
        }
    }
    //pthread_mutex_unlock(&(rm->sleep_lock));
    return 0;
}

int RMEnforcePolicy(RequestManager rm)
{
    //printf("\n ~~~~~~~~~~~~~~~~~~~~~Inside RMEnforcePolicy~~~~~~~~~~~~~~~~~~\n");
    switch (rm->policy) {
        case Block:
            BlockMainThread(rm);
            break;
        case DropTail:
            return 1;
        case DropHead:
            return DropOldestRequest(rm);
        case BlockFlush:
            BlockMainThread(rm);
            return 1;   //TODO: make sure that we want to drop request
        case Dynamic:
            IncQueueSize(rm);
            return 1;
        case DropRandom:
            return DropRandomRequests(rm);
    }
    return 0;
}

pthread_mutex_t* RMGetSleepLock(RequestManager rm)
{
    if(rm == NULL)
    {
        printf("RequestManager is NULL\n");
        exit(1);
    }
    return &(rm->sleep_lock);
}

Queue RMGetRunQueue(RequestManager rm)
{
    if(rm == NULL)
    {
        printf("RequestManager is NULL\n");
        exit(1);
    }
    return rm->run_queue;
}
Queue RMGetWaitQueue(RequestManager rm)
{
    if(rm == NULL)
    {
        printf("RequestManager is NULL\n");
        exit(1);
    }
    return rm->wait_queue;
}
