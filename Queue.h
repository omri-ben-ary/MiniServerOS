#ifndef WEBSERVER_FILES_QUEUE_H
#define WEBSERVER_FILES_QUEUE_H
#include "Node.h"
struct queue_t;
typedef struct queue_t* Queue;

Queue QueueInit();
void QueueDestroy(Queue que);
void enqueue(Queue que,Request req);
Request dequeue(Queue que);
int QueueGetSize(Queue que);
void QueueRemoveRequest(Queue que, Request req);
int QueueRemoveNthRequest(Queue que, int n);
void QueuePrint(Queue que);
#endif //WEBSERVER_FILES_QUEUE_H
