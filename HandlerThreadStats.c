//
// Created by student on 6/15/23.
//

#include "HandlerThreadStats.h"
struct handler_thread_stats_t
{
    int handler_thread_id;
    int handler_thread_req_count;
    int handler_thread_static_req_count;
    int handler_thread_dynamic_req_count;
};

handlerThreadStats handlerThreadStatsInit(int id)
{
    handlerThreadStats stats = malloc(sizeof(*stats));
    if (stats == NULL)
    {
        return NULL;
    }
    stats->handler_thread_id = id;
    stats->handler_thread_req_count = 0;
    stats->handler_thread_static_req_count = 0;
    stats->handler_thread_dynamic_req_count = 0;
    return stats;
}

void handlerThreadStatsDestroy(handlerThreadStats stats)
{
    free(stats);
}

int handlerThreadStatsGetID(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    return stats->handler_thread_id;
}

int handlerThreadStatsGetTotalReqCount(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    return stats->handler_thread_req_count;
}

int handlerThreadStatsGetStaticReqCount(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    return stats->handler_thread_static_req_count;
}

int handlerThreadStatsGetDynamicReqCount(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    return stats->handler_thread_dynamic_req_count;
}

int handlerThreadStatsSetID(handlerThreadStats stats, int id)
{
    if (stats == NULL)
    {
        return -1;
    }
    stats->handler_thread_id = id;
    return 1;
}

int handlerThreadStatsIncTotalReqCount(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    stats->handler_thread_req_count++;
    return 1;
}

int handlerThreadStatsIncStaticReqCount(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    stats->handler_thread_static_req_count++;
    return 1;
}

int handlerThreadStatsIncDynamicReqCount(handlerThreadStats stats)
{
    if (stats == NULL)
    {
        return -1;
    }
    stats->handler_thread_dynamic_req_count++;
    return 1;
}

