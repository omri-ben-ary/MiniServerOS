#include "Queue.h"
struct queue_t
{
    int size;
    Node head;
    Node tail;
};

Queue QueueInit()
{
    Queue que = malloc(sizeof(*que));
    if(que == NULL)
    {
        printf("Server is out of memory\n");
        exit(1);
    }
    que->size = 0;
    que->head = NULL;
    que->tail = NULL;
    return que;
}

void QueueDestroy(Queue que)
{
    Node node_to_destroy = que->head;
    while(node_to_destroy != NULL)
    {
        Node next_node = NodeGetNext(node_to_destroy);
        NodeDestroy(node_to_destroy);
        node_to_destroy = next_node;
    }
}

void enqueue(Queue que, Request req)
{
    Node node = NodeInit(req);
    if(node == NULL)
    {
        //piazza epic #438 states that this won't be checked
        printf("Server is out of memory\n");
        exit(1);
    }
    if(que->head == NULL)
    {
        que->head = node;
        que->tail = node;
    }
    else
    {
        NodeSetNext(que->tail, node);
        que->tail = node;
    }
    que->size++;
}

Request dequeue(Queue que)
{
    //printf("inside dequeue\n");
    //printf("printing the wait queue: \n");
    //QueuePrint(que);
    if(que->head == NULL)
    {
        printf("Try to get next for NULL ptr\n");
        exit(1);
    }
    Node node_to_return = que->head;

    que->head = NodeGetNext(que->head);
    if(que->head == NULL)
    {
        que->tail = NULL;
    }
    que->size--;

    Request req = RequestCopy(NodeGetRequest(node_to_return));
    if(req == NULL)
    {
        //piazza epic #438 states that this won't be checked

        printf("Server is out of memory\n");
        exit(1);
    }
    //printf("reached here\n");
    NodeDestroy(node_to_return);
    return req;
}

int QueueGetSize(Queue que)
{
    if (que == NULL)
    {
        printf("Try to read size of Null queue!\n");
        exit(1);
    }
    return que->size;
}

void QueueRemoveRequest(Queue que, Request req)
{
    if(que->head == NULL)
    {
        printf("Try to remove element from Null queue!\n");
        exit(1);
    }
    if(req == NULL)
    {
        printf("Try to remove Null element from queue!\n");
        exit(1);
    }
    Node itr = que->head;
    Request itr_req = NodeGetRequest(itr);
    //printf("\n\nRemove Request\n");
    //printf("current request fd: %d\n", RequestGetFD(itr_req));
    //printf("wanted request fd: %d\n", RequestGetFD(req));
    if(RequestCompare(itr_req,req) == 0)
    {
        que->size--;
        if(NodeGetNext(itr) == NULL)
        {
            NodeDestroy(itr);
            que->head = NULL;
            que->tail = NULL;
            return;
        }
        que->head = NodeGetNext(itr);
        NodeDestroy(itr);
        return;
    }
    while(NodeGetNext(itr) != NULL)
    {
        itr_req = NodeGetRequest(NodeGetNext(itr));
        //printf("\n\nRemove Request\n");
        //printf("current request fd: %d\n", RequestGetFD(itr_req));
        //printf("wanted request fd: %d\n", RequestGetFD(req));
        if(RequestCompare(itr_req,req) == 0)
        {
            if(RequestCompare(NodeGetRequest(que->tail),req) == 0)
            {
                que->tail = itr;
                //printf("\nTail fd is %d\n", RequestGetFD(NodeGetRequest(que->tail)));
            }
            que->size--;
            Node node_to_destroy = NodeGetNext(itr);
            NodeSetNext(itr, NodeGetNext(NodeGetNext(itr)));
            NodeDestroy(node_to_destroy);
            return;
        }
        itr = NodeGetNext(itr);
    }
    //printf("\n\nI got here!!!\n");
}

int QueueRemoveNthRequest(Queue que, int n)
{
    if(que->head == NULL)
    {
        printf("Try to remove element from Null queue!\n");
        exit(1);
    }
    if(n > que->size)
    {
        printf("Try to read size of Null queue!\n");
        exit(1);
    }
    Node itr = que->head;
    for(int i=1; i<n; i++)
    {
        itr = NodeGetNext(itr);
    }
    int return_fd = RequestGetFD(NodeGetRequest(itr));
    QueueRemoveRequest(que, NodeGetRequest(itr));
    return return_fd;
}

void QueuePrint(Queue que)
{
    printf("\n\nPrint Que:\n");
    if(que->head != NULL)
    {
        printf("head: %d\n", RequestGetFD(NodeGetRequest(que->head)));
    }
    else
    {
        printf("head: NULL\n");
    }
    Node itr = que->head;
    while(itr != NULL)
    {
        printf("%d\n", RequestGetFD(NodeGetRequest(itr)));
        itr = NodeGetNext(itr);
    }
    if(que->tail != NULL)
    {
        printf("tail: %d\n", RequestGetFD(NodeGetRequest(que->tail)));
    }
    else
    {
        printf("tail: NULL\n");
    }
}