#include "request.h"

struct request_t
{
    int fd;
    struct timeval* arrival_time;
    struct timeval* dispatch_interval;
};

Request RequestInit(int fd, struct timeval* arrival_time)
{
    Request req = malloc(sizeof(*req));
    if(req == NULL)
    {
        free(arrival_time);
        return NULL;
    }
    req->fd = fd;
    if (RequestSetArrivalTime(req, arrival_time) == 0)
    {
        return NULL;
    }
    req->dispatch_interval = NULL;
    return req;
}

void RequestDestroy(Request req)
{
    free(req->dispatch_interval);
    free(req->arrival_time);
    free(req);
}

int RequestGetFD(Request req)
{
    if(req == NULL)
    {
        return -1;
    }
    return req->fd;
}

struct timeval* RequestGetArrivalTime(Request req)
{
    if(req == NULL)
    {
        printf("request is NULL !\n");
        exit(1);
    }
    return req->arrival_time;
}
struct timeval* RequestGetDispatchInterval(Request req)
{
    if(req == NULL)
    {
        printf("request is NULL !\n");
        exit(1);
    }
    return req->dispatch_interval;
}
int RequestSetArrivalTime(Request req, struct timeval* arrival_time)
{
    req->arrival_time = malloc(sizeof(*req->arrival_time));
    if(req->arrival_time == NULL)
    {
        free(arrival_time);
        free(req);
        return 0;
    }
    req->arrival_time->tv_sec = arrival_time->tv_sec;
    req->arrival_time->tv_usec = arrival_time->tv_usec;
    return 1;
}

int RequestSetDispatchInterval(Request req)
{
    req->dispatch_interval = malloc(sizeof(*req->dispatch_interval));
    if(req->dispatch_interval == NULL)
    {
        free(req->arrival_time);
        free(req);
        return 0;
    }
    gettimeofday(req->dispatch_interval, NULL);
    if (req->dispatch_interval->tv_usec < req->arrival_time->tv_usec)
    {
        req->dispatch_interval->tv_usec += 1000000;
        req->dispatch_interval->tv_sec -= 1;
    }
    req->dispatch_interval->tv_usec -= req->arrival_time->tv_usec;
    req->dispatch_interval->tv_sec -= req->arrival_time->tv_sec;
    return 1;
}

Request RequestCopy(Request src)
{
    //printf("Inside RequestCopy\n");
    if(src == NULL)
    {
        printf("Try to create a copy of a NULL ptr!\n");
        exit(1);
    }
    Request dst = malloc(sizeof(*dst));
    if(dst == NULL)
    {
        return NULL;
    }

    dst->arrival_time = malloc(sizeof(*dst->arrival_time));
    if(dst->arrival_time == NULL)
    {
        free(dst);
        return NULL;
    }
    dst->dispatch_interval = malloc(sizeof(*dst->dispatch_interval));
    if(dst->dispatch_interval == NULL)
    {
        free(dst->arrival_time);
        free(dst);
        return NULL;
    }

    dst->fd = src->fd;
    //printf("Stat-Req-Arrival:: %lu.%06lu\r\n", RequestGetArrivalTime(src)->tv_sec, RequestGetArrivalTime(src)->tv_usec);
    //printf("Stat-Req-Dispatch:: %lu.%06lu\r\n", RequestGetDispatchInterval(src)->tv_sec, RequestGetDispatchInterval(src)->tv_usec);
    //printf("Stat-Req-Arrival:: %lu.%06lu\r\n", RequestGetArrivalTime(dst)->tv_sec, RequestGetArrivalTime(dst)->tv_usec);
    //printf("Stat-Req-Dispatch:: %lu.%06lu\r\n", RequestGetDispatchInterval(dst)->tv_sec, RequestGetDispatchInterval(dst)->tv_usec);
    dst->arrival_time->tv_sec = src->arrival_time->tv_sec;
    dst->arrival_time->tv_usec = src->arrival_time->tv_usec;
    if (src->dispatch_interval == NULL)
    {
        dst->dispatch_interval = NULL;
    }
    else
    {
        dst->dispatch_interval->tv_sec = src->dispatch_interval->tv_sec;
        dst->dispatch_interval->tv_usec = src->dispatch_interval->tv_usec;
    }
    return dst;
}

int RequestCompare(Request req1, Request req2)
{
    if(req1 == NULL)
    {
        printf("Try to cmp NULL ptr\n");
        exit(1);
    }
    if(req2 == NULL)
    {
        printf("Try to cmp NULL ptr\n");
        exit(1);
    }
    return (req1->fd != req2->fd);
}

// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(Request req, char *cause, char *errnum, char *shortmsg, char *longmsg, handlerThreadStats handler_stats)
{
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(RequestGetFD(req), buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(RequestGetFD(req), buf, strlen(buf));
   printf("%s", buf);

    sprintf(buf, "Content-Length: %lu\r\n", strlen(body));
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, RequestGetArrivalTime(req)->tv_sec, RequestGetArrivalTime(req)->tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, RequestGetDispatchInterval(req)->tv_sec, RequestGetDispatchInterval(req)->tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, handlerThreadStatsGetID(handler_stats));
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, handlerThreadStatsGetTotalReqCount(handler_stats));
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, handlerThreadStatsGetStaticReqCount(handler_stats));
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, handlerThreadStatsGetDynamicReqCount(handler_stats));
    Rio_writen(RequestGetFD(req), buf, strlen(buf));
    printf("%s", buf);
   //TODO: add sprintf to the stats

    // Write out the content
   Rio_writen(RequestGetFD(req), body, strlen(body));
   printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n")) {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) 
{
   char *ptr;

   if (strstr(uri, "..")) {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi")) {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri)-1] == '/') {
         strcat(filename, "home.html");
      }
      return 1;
   } else {
      // dynamic
      ptr = index(uri, '?');
      if (ptr) {
         strcpy(cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html")) 
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif")) 
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg")) 
      strcpy(filetype, "image/jpeg");
   else 
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(Request req, char *filename, char *cgiargs, handlerThreadStats handler_stats)
{
   char buf[MAXLINE], *emptylist[] = {NULL};

   // The server does only a little bit of the header.  
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   //TODO: add sprintf to the stats
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, RequestGetArrivalTime(req)->tv_sec, RequestGetArrivalTime(req)->tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, RequestGetDispatchInterval(req)->tv_sec, RequestGetDispatchInterval(req)->tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, handlerThreadStatsGetID(handler_stats));
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, handlerThreadStatsGetTotalReqCount(handler_stats));
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, handlerThreadStatsGetStaticReqCount(handler_stats));
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, handlerThreadStatsGetDynamicReqCount(handler_stats));
   Rio_writen(RequestGetFD(req), buf, strlen(buf));
    pid_t pid = Fork();
   if (pid == 0) {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(RequestGetFD(req), STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   WaitPid(pid, NULL, 0);
}


void requestServeStatic(Request req, char *filename, int filesize, handlerThreadStats handler_stats)
{
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory, 
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
   //TODO: add sprintf to the stats
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, RequestGetArrivalTime(req)->tv_sec, RequestGetArrivalTime(req)->tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, RequestGetDispatchInterval(req)->tv_sec, RequestGetDispatchInterval(req)->tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, handlerThreadStatsGetID(handler_stats));
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, handlerThreadStatsGetTotalReqCount(handler_stats));
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, handlerThreadStatsGetStaticReqCount(handler_stats));
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, handlerThreadStatsGetDynamicReqCount(handler_stats));

    Rio_writen(RequestGetFD(req), buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file 
   Rio_writen(RequestGetFD(req), srcp, filesize);
   Munmap(srcp, filesize);

}

// handle a request
void requestHandle(Request req, handlerThreadStats handler_stats)
{
    //printf("inside requestHandle\n");
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    //TODO: change total request thread counter
    handlerThreadStatsIncTotalReqCount(handler_stats);

    Rio_readinitb(&rio, RequestGetFD(req));
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    printf("%s %s %s\n", method, uri, version);

    if (strcasecmp(method, "GET")) {
      requestError(req, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method", handler_stats);
      return;
    }
    requestReadhdrs(&rio);

    is_static = requestParseURI(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
      requestError(req, filename, "404", "Not found", "OS-HW3 Server could not find this file", handler_stats);
      return;
    }
    if (is_static)
    {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
      {
         requestError(req, filename, "403", "Forbidden", "OS-HW3 Server could not read this file", handler_stats);
         return;
      }
      //TODO: change static thread counter
      handlerThreadStatsIncStaticReqCount(handler_stats);

      requestServeStatic(req, filename, sbuf.st_size, handler_stats);
    }
    else
    {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
      {
         requestError(req, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program", handler_stats);
         return;
      }
      //TODO: change dynamic thread counter
      handlerThreadStatsIncDynamicReqCount(handler_stats);

      requestServeDynamic(req, filename, cgiargs, handler_stats);
    }
}


