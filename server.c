#include "segel.h"
#include "request.h"
#include "RequestManager.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


// HW3: Parse the new arguments too
void getargs(int *port, int* number_of_workers, int* queue_size, OverLoadPolicy* policy, int* max_size, int argc, char *argv[])
{
    if (argc < 5 || argc > 6)
    {
	    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	    exit(1);
    }
    *port = atoi(argv[1]);
    *number_of_workers = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    if(argc > 5)
    {
        *max_size =atoi(argv[5]);
    }
    else
    {
        *max_size = 0;
    }
    if(strcmp(argv[4], "block") == 0)
    {
        *policy = Block;
    }
    else if(strcmp(argv[4], "dt") == 0)
    {
        *policy = DropTail;
    }
    else if(strcmp(argv[4], "dh") == 0)
    {
        *policy = DropHead;
    }
    else if(strcmp(argv[4], "bf") == 0)
    {
        *policy = BlockFlush;
    }
    else if(strcmp(argv[4], "dynamic") == 0)
    {
        *policy = Dynamic;
    }
    else if(strcmp(argv[4], "random") == 0)
    {
        *policy = DropRandom;
    }
    else
    {
        //TODO: Check what to do in this scenario
        printf("Illegal policy given\n");
        exit(1);
    }
}

/*TODO: validate input argument? (and overLoadPolicy)
 */
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, number_of_workers, queue_size, max_size;
    OverLoadPolicy policy;
    struct sockaddr_in clientaddr;
    getargs(&port, &number_of_workers, &queue_size, &policy, &max_size, argc, argv);
    if(number_of_workers <= 0 || queue_size <= 0)
    {
        //piazza epic #411 states that this won't be checked
        printf("You Entered a non-positive number in threads or queue_size command line arguments\n");
        exit(1);
    }
    RequestManager rm = RMInit(queue_size, policy, max_size, number_of_workers);
    if(rm == NULL)
    {
        //piazza epic #438 states that this won't be checked
        printf("Program is out of memory\n");
        exit(1);
    }
    /*pthread_mutex_lock(RMGetSleepLock(rm));
    int number_of_requests = QueueGetSize(RMGetRunQueue(rm)) + QueueGetSize(RMGetWaitQueue(rm));
    printf("\n\nIntialize\n");
    printf("requests: %d\n", number_of_requests);
    printf("requests in run queue: %d\n", QueueGetSize(RMGetRunQueue(rm)));
    printf("requests in wait queue: %d\n", QueueGetSize(RMGetWaitQueue(rm)));
    printf("front request in run queue fd is: %d\n", RequestGetFD(Front(RMGetRunQueue(rm))));
    pthread_mutex_unlock(RMGetSleepLock(rm));*/
    pthread_t** workers = malloc(number_of_workers * sizeof(*workers));
    if(workers == NULL)
    {
        //piazza epic #438 states that this won't be checked
        printf("Server is out of memory\n");
        exit(1);
    }
    for(int i=0; i < number_of_workers; i++)
    {
        workers[i] = malloc(sizeof (*workers[0]));
        if(workers[i] == NULL)
        {
            //piazza epic #438 states that this won't be checked
            for(int j=0; j < i; j++)
            {
                free(workers[j]);
            }
            free(workers);
            printf("Server is out of memory\n");
            exit(1);
        }

        if(pthread_create(workers[i], NULL, work, (void*) rm) != 0)
        {
            //TODO: figure out what to do in this case
            exit(1);
        }

    }
   /* pthread_mutex_lock(RMGetSleepLock(rm));
    number_of_requests = QueueGetSize(RMGetRunQueue(rm)) + QueueGetSize(RMGetWaitQueue(rm));
    printf("\n\nAfter creating threadss\n");
    printf("requests: %d\n", number_of_requests);
    printf("requests in run queue: %d\n", QueueGetSize(RMGetRunQueue(rm)));
    printf("requests in wait queue: %d\n", QueueGetSize(RMGetWaitQueue(rm)));
    printf("front request in run queue fd is: %d\n", RequestGetFD(Front(RMGetRunQueue(rm))));
    pthread_mutex_unlock(RMGetSleepLock(rm));*/
    listenfd = Open_listenfd(port);
    while (1)
    {
        int drop_request = 0;
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        pthread_mutex_lock(RMGetSleepLock(rm));
        struct timeval* arrival_time = malloc(sizeof(*arrival_time));
        if(arrival_time == NULL)
        {
            for(int i=0; i < number_of_workers; i++)
            {
                free(workers[i]);
            }
            free(workers);
            printf("Server is out of memory\n");
            exit(1);
        }
        gettimeofday(arrival_time, NULL);

        if(RMCheckOverLoad(rm) != 0)
        {
            drop_request = RMEnforcePolicy(rm);
        }
        //pthread_mutex_unlock(RMGetSleepLock(rm));
        if(!drop_request)
        {
            RMHandleRequest(rm, connfd, arrival_time);
        }
        else
        {
            //printf("Closing!!!\n");
            Close(connfd);
            pthread_mutex_unlock(RMGetSleepLock(rm));
        }
        free(arrival_time);
    }

}

//
// HW3: In general, don't handle the request in the main thread.
// Save the relevant info in a buffer and have one of the worker threads
// do the work.
//

//original
//requestHandle(connfd);

//Close(connfd);
    


 
