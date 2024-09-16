#ifndef __REQUEST_H__
#include "segel.h"
#include "HandlerThreadStats.h"

struct request_t;
typedef struct request_t* Request;
Request RequestInit(int fd, struct timeval* arrival_time);
void RequestDestroy(Request req);
int RequestGetFD(Request req);
struct timeval* RequestGetArrivalTime(Request req);
struct timeval* RequestGetDispatchInterval(Request req);
int RequestSetArrivalTime(Request req, struct timeval* arrival_time);
int RequestSetDispatchInterval(Request req);
Request RequestCopy(Request src);
int RequestCompare(Request req1, Request req2);
void requestHandle(Request req, handlerThreadStats handler_stats);
#endif
