//
// Created by student on 6/15/23.
//

#ifndef WEBSERVER_FILES_HANDLERTHREADSTATS_H
#define WEBSERVER_FILES_HANDLERTHREADSTATS_H
#include "segel.h"

struct handler_thread_stats_t;
typedef struct handler_thread_stats_t* handlerThreadStats;
handlerThreadStats handlerThreadStatsInit(int id);
void handlerThreadStatsDestroy(handlerThreadStats stats);
int handlerThreadStatsGetID(handlerThreadStats stats);
int handlerThreadStatsGetTotalReqCount(handlerThreadStats stats);
int handlerThreadStatsGetStaticReqCount(handlerThreadStats stats);
int handlerThreadStatsGetDynamicReqCount(handlerThreadStats stats);
int handlerThreadStatsSetID(handlerThreadStats stats, int id);
int handlerThreadStatsIncTotalReqCount(handlerThreadStats stats);
int handlerThreadStatsIncStaticReqCount(handlerThreadStats stats);
int handlerThreadStatsIncDynamicReqCount(handlerThreadStats stats);
#endif //WEBSERVER_FILES_HANDLERTHREADSTATS_H
