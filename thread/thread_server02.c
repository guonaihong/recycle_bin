#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "proto.h"
#include "user.h"
static int tcp_listen(char *ip, char *port);

#define MAX_THREADS 128
struct thread_pool {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;

    int             s;                /*server listen socket*/
    int             fds[MAX_THREADS]; /*current accept socket*/
    int             iget;
    int             iput;

    pthread_t       tids[MAX_THREADS];

    void (*process)(int s);
};

void process_msg(int s) {
    printf("s = %d\n", s);
    close(s);
}
/*server code*/
static int   thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s), int s);
static int   thread_main(struct thread_pool *p);
static void *thread_worker(void *arg);
/*server code*/

static void usage(void) {
    fprintf(
            stderr, 
            "Usage: \r\n"
            "-p <port> <must exist>\r\n"
            "-i <ip>   \r\n"
            "-n <number of threads:default %d>\r\n"
            "-h help \r\n"
            ,MAX_THREADS
           );
}

#if SERVER
int main(int argc, char **argv) {

    int s;
    int nthread = -1;
    char *ip, *port;
    char c;

    ip = port = NULL;

    while ((c = getopt(argc, argv, "p:i:n:h")) != -1) {
        switch(c) {
            case 'p':
                port = optarg;
                break;
            case 'i':
                ip   = optarg;
                break;
            case 'n':
                nthread = atoi(optarg);
                break;
            case 'h':
                usage();
                break;
        }
    }
    if (port == NULL) {
        usage();
        return 1;
    }

    s = tcp_listen(ip, port);
    if (s == -1)
        return 1;

    struct thread_pool *pool;

    int r = thread_pool_create(&pool, nthread, process_msg, s);
    printf("thread pool create = %d\n", r);
    if (thread_main(pool) == -1)
        exit(1);
    close(s);

    return 0;
}
#endif

static int tcp_listen(char *ip, char *port) {
    struct sockaddr_in server;
    int r, s;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("socket");
        return -1;
    }

    if (!port) {
        fprintf(stderr, "port is null\n");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port   = htons(atoi(port));
    server.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) {
        perror("setsockopt");
        return -1;
    }

    r = bind(s, (struct sockaddr*)&server, sizeof(server));
    if (r == -1) {
        perror("bind");
        return -1;
    }

    r = listen(s, 10);
    if (r == -1) {
        perror("listen");
        return -1;
    }
    return s;
}

static int   thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s), int s) {
    int i, r;

    struct thread_pool *p = NULL;
    r = 0;
    p = calloc(1, sizeof(struct thread_pool));
    if (!p)
        goto nomem;

    if ((r = pthread_mutex_init(&p->mutex, NULL)) != 0)
        goto failed;

    if ((r = pthread_cond_init(&p->cond, NULL)) != 0)
        goto failed;

    if (nthreads <= 0 || nthreads > MAX_THREADS)
        nthreads = MAX_THREADS;

    p->process = process;
    p->s       = s;
    for (i = 0; i < nthreads; i++) {
        r = pthread_create(&p->tids[i], NULL, thread_worker, p);
    }
    *pool = p;

    return 0;

failed:

    pthread_mutex_destroy(&p->mutex);
    pthread_cond_destroy(&p->cond);
    free(p);

nomem:
    fprintf(stderr, "error:%s:%s\n", strerror(r), strerror(errno));
    return -1;
}

static void *thread_worker(void *arg) {

    struct thread_pool *p = (struct thread_pool *)arg;

    int connfd = -1;
    int r;
    for (;;) {

        /* lock */
        if ((r = pthread_mutex_lock(&p->mutex)) != 0)
            goto quit;

        /* wait */
        while (p->iput == p->iget) {
            if((r = pthread_cond_wait(&p->cond, &p->mutex)) != 0)
                goto quit;
        }

        /* get sock */
        if (p->iget == MAX_THREADS)
            p->iget = 0;
        connfd = p->fds[p->iget++];

        /* unlock */
        if ((r = pthread_mutex_unlock(&p->mutex)) != 0)
            goto quit;

        p->process(connfd);
    }

quit:
    fprintf(stderr, "error:%s\n", strerror(r));
    exit(1);
    return (void *)0;
}

static int   thread_main(struct thread_pool *p) {

    int connfd;
    int s = p->s;
    struct sockaddr_in addr;
    socklen_t size;
    int r = 0;
    char str[INET6_ADDRSTRLEN];

    for (;;) {

        size   = sizeof(addr);
        connfd = accept(s, (struct sockaddr *)&addr, &size);

        if (connfd == -1) {
            goto quit;
        }
        /*print ip address*/
        if (inet_ntop(AF_INET, &addr.sin_addr, str, INET6_ADDRSTRLEN) == NULL) {
            goto quit;
        }
        printf("current client ip = %s\n", str);

        /* lock */
        if ((r = pthread_mutex_lock(&p->mutex)) != 0)
            goto quit;

        if (p->iput == MAX_THREADS)
            p->iput = 0;

        /* put sock */
        p->fds[p->iput++] = connfd;

        if ((r = pthread_cond_signal(&p->cond)) != 0)
            goto quit;

        /* unlock */
        if ((r = pthread_mutex_unlock(&p->mutex)) != 0)
            goto quit;
    }

quit:
    fprintf(stderr, "error:%s:%s\n", strerror(r), strerror(errno));
    return -1;
}
