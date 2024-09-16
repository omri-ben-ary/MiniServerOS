#ifndef WEBSERVER_FILES_REQUESTMANAGER_H
#define WEBSERVER_FILES_REQUESTMANAGER_H
#include "Queue.h"
#include "HandlerThreadStats.h"
struct requestManager_t;
typedef struct requestManager_t* RequestManager;
typedef enum
{
    Block,
    DropTail,
    DropHead,
    BlockFlush,
    Dynamic,
    DropRandom
}OverLoadPolicy;
RequestManager RMInit(int queue_size, OverLoadPolicy policy, int max_size, int num_of_workers);
void RMDestroy(RequestManager rm);
int RMCheckOverLoad(RequestManager rm);
void RMHandleRequest(RequestManager rm, int req_fd, struct timeval* arrival_time);
void* work(void* rm);
void sleepWorker(RequestManager rm);
Request RMGetRequest(RequestManager rm);
void RMFinishJob(RequestManager rm, Request req);
int RMEnforcePolicy(RequestManager rm);
pthread_mutex_t* RMGetSleepLock(RequestManager rm);
Queue RMGetRunQueue(RequestManager rm);
Queue RMGetWaitQueue(RequestManager rm);
#endif //WEBSERVER_FILES_REQUESTMANAGER_H