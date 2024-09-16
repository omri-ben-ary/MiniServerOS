#include "Node.h"
#include "segel.h"
struct node_t
{
    Request req;
    Node next;
};

Node NodeInit(Request req)
{
    Node node = malloc(sizeof(*node));
    if(node == NULL)
    {
        return NULL;
    }
    node->req = req;
    node->next = NULL;
    return node;
}

void NodeDestroy(Node node)
{
    RequestDestroy(node->req);
    free(node);
}

Node NodeGetNext(Node node)
{
    if(node == NULL)
    {
        printf("Try to get next for NULL ptr\n");
        exit(1);
    }
    return node->next;
}

void NodeSetNext(Node node, Node next_node)
{
    if(node == NULL)
    {
        printf("Try to set next for NULL ptr\n");
        exit(1);
    }
    node->next = next_node;
}
Request NodeGetRequest(Node node)
{
    if(node == NULL)
    {
        printf("Try to get request for NULL ptr\n");
        exit(1);
    }
    return node->req;
}