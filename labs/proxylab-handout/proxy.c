#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include "csapp.h"
#include <netdb.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* sbuf size */
#define SBUF_SIZE 20
/* n threads */
#define NTHREADS 4
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
}sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

void *doit();
int read_requesthdrs_save(rio_t *rp, char *headers_buf, int maxlen); 
int parse_uri_proxy(const char *uri, char *hostname, char *path, char *port);
void build_http_request(char *http_request, const char *method,
                        const char *path, const char *host, 
                        const char *headers);

void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
         char *shortmsg, char *longmsg);

sbuf_t sbuf;

int main(int argc, char **argv)  
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    sbuf_init(&sbuf, SBUF_SIZE);
    for (int i = 0; i < NTHREADS; i++) {
        pthread_create(&tid, NULL, doit, NULL);
    }
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
            Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                        port, MAXLINE, NI_NUMERICHOST | NI_NUMERICSERV);
            printf("Accepted connection from (%s, %s)\n", hostname, port);  
        sbuf_insert(&sbuf, connfd);                                //line:netp:tiny:doit                                            //line:netp:tiny:close
    }
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void *doit() 
{
    Pthread_detach(pthread_self());
    int fd;
    while (1) {
        fd = sbuf_remove(&sbuf);
        int clientfd;
        char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
        rio_t rio;
        int hdr_len;
        char http_request[MAXLINE];
        char host[MAXLINE], path[MAXLINE], port[MAXLINE];
        char request_headers[MAXLINE];

        /* server response variable */
        rio_t server_rio;
        char response_buf[MAXBUF];
        ssize_t n;


        /* Read request line and headers */
        Rio_readinitb(&rio, fd);
        if (!Rio_readlineb(&rio, buf, MAXLINE)) {
            Close(fd);
            continue;
        }

        printf("%s", buf);
        sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
        if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
            clienterror(fd, method, "501", "Not Implemented",
                        "Tiny does not implement this method");
            Close(fd);
            continue;
        }        
        /* parse http request to header information */                                            //line:netp:doit:endrequesterr
        hdr_len = read_requesthdrs_save(&rio, request_headers, sizeof(request_headers));     //line:netp:doit:readrequesthdrs
        
        /* parse uri to get host, path and port information */
        parse_uri_proxy(uri, host, path, port);
        
        /* build http request */
        build_http_request(http_request, method, path, host, request_headers);

        clientfd = Open_clientfd(host, port);
        Rio_writen(clientfd, http_request, strlen(http_request));

        Rio_readinitb(&server_rio, clientfd);
        while ((n = Rio_readnb(&server_rio, response_buf, MAXBUF)) > 0) {
            Rio_writen(fd, response_buf, n);
        }

        /* file send over */
        Close(fd);
        Close(clientfd);
    }

}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
int read_requesthdrs_save(rio_t *rp, char *headers_buf, int maxlen) {
    char buf[MAXLINE];
    int total_len = 0;

    while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
        if (!strcmp(buf, "\r\n")) break;  // 读到空行
        int len = strlen(buf);
        if (total_len + len < maxlen - 1) {
            memcpy(headers_buf + total_len, buf, len);
            total_len += len;
        }
    }

    headers_buf[total_len] = '\0';
    return total_len;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri_proxy(const char *uri, char *hostname, char *path, char *port) 
{
    char *hostbegin, *hostend, *pathbegin;
    int len;
    if (strncasecmp(uri, "http://", 7) == 0) {
        hostbegin = uri + 7;
    }
    else hostbegin = uri;

    pathbegin = strchr(hostbegin, '/');
    if (pathbegin) {
        strcpy(path, pathbegin);
        len = pathbegin - hostbegin;
    }
    else {
        strcpy(path, "/");
        len = strlen(hostbegin);
    }
    
    /* host[:port] */
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* search port number */
    hostend = strchr(hostname, ':');
    if (hostend) {
        *hostend = '\0';
        strcpy(port, hostend + 1);
    }
    else {
        strcpy(port, "80");
    }
    return 0;
}


/**
 * build a standord http request from proxy to server by using client 
 * sending information
 */
void build_http_request(char *http_request, const char *method,
                        const char *path, const char *host, 
                        const char *headers) {
    /* build http request line */
    sprintf(http_request, "%s %s HTTP/1.0\r\n", method, path);

    /* rebuild request headers */
    sprintf(http_request + strlen(http_request), "%s", headers);
    /* host name */
    sprintf(http_request + strlen(http_request), "Host: %s\r\n", host);
    /* user-agent model */
    sprintf(http_request + strlen(http_request), "%s", user_agent_hdr);

    /* connection and proxy-connection headers */
    sprintf(http_request + strlen(http_request), "Connection: close\r\n");
    sprintf(http_request + strlen(http_request), "Proxy-Connection: close\r\n");


    
    /* end header */
    sprintf(http_request + strlen(http_request), "\r\n");
    
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);    //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); //line:netp:servestatic:beginserve
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    Rio_writen(fd, buf, strlen(buf));    //line:netp:servestatic:endserve

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0); //line:netp:servestatic:open
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //line:netp:servestatic:mmap
    Close(srcfd);                       //line:netp:servestatic:close
    Rio_writen(fd, srcp, filesize);     //line:netp:servestatic:write
    Munmap(srcp, filesize);             //line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
    else
    strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) { /* Child */ //line:netp:servedynamic:fork
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
    Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2
    Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
    }
    Wait(NULL); /* Parent waits for and reaps child */ //line:netp:servedynamic:wait
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
         char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */


/* init a sbuf which shared by multiply threads */
void sbuf_init(sbuf_t *sp, int n) {
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1);
    Sem_init(&sp->slots, 0, n);
    Sem_init(&sp->items, 0, 0);
}

/* free sbuf */
void sbuf_deinit(sbuf_t *sp) {
    Free(sp->buf);
}

/* insert one item in sbuf followed by fifo policy */
void sbuf_insert(sbuf_t *sp, int item) {
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[sp->rear] = item;
    sp->rear = (sp->rear + 1) % sp->n;
    V(&sp->mutex);
    V(&sp->items);
}

/* remvoe and return the first item in sbuf */
int sbuf_remove(sbuf_t *sp) {
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item = sp->buf[sp->front];
    sp->front = (sp->front + 1) % sp->n;
    V(&sp->mutex);
    V(&sp->slots);
    
    return item;
}

