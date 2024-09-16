#ifndef WEBSERVER_FILES_NODE_H
#define WEBSERVER_FILES_NODE_H
#include "request.h"

struct node_t;
typedef struct node_t* Node;
Node NodeInit(Request req);
void NodeDestroy(Node node);
Node NodeGetNext(Node node);
void NodeSetNext(Node node, Node next_node);
Request NodeGetRequest(Node node);
#endif //WEBSERVER_FILES_NODE_H
