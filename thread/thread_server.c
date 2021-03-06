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

    int             ssize;/*stack size*/
    int             nthreads; /*thread number*/
    pthread_t       tids[MAX_THREADS];
    int             s;
    void (*process)(int s);
};

/*server code*/
static int   thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s), int s, int ssize);
static void *thread_worker(void *arg);
static int   thread_pool_destroy(struct thread_pool *pool);
/*server code*/

static void usage(void) {
    fprintf(
            stderr, 
            "Usage: \r\n"
            "-p <port> <must exist>\r\n"
            "-i <ip>   \r\n"
            "-n <number of threads:default %d>\r\n"
            "-s <thread stack size <1..20>MB>\r\n"
            "-h help \r\n"
            ,MAX_THREADS
           );
}

#if TEST
void process_msg(int s) {
    printf("s = %d\n", s);
    sleep(2);
    close(s);
}
#endif
#if SERVER

#define STACKSIZE(s) ((s) >=1 && (s) <= 20)
int main(int argc, char **argv) {

    int s;
    int nthread = -1;
    int ssize   = -1;
    char *ip, *port;
    char c;

    ip = port = NULL;

    while ((c = getopt(argc, argv, "p:i:n:s:h")) != -1) {
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
            case 's':
                ssize   = atoi(optarg);
                if (!STACKSIZE(ssize))
                    ssize = 10;
                break;
            case 'h':
                usage();
                exit(0);
            default:
                usage();
                exit(1);
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

    int r = thread_pool_create(&pool, nthread, process_msg, s, ssize);
    printf("thread pool create = %d\n", r);
    thread_pool_destroy(pool);
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

static int   thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s), int s, int ssize) {
    int i, r;
    int rc = -1;

    struct thread_pool *p = NULL;
    pthread_attr_t *attr  = NULL;;
    if ((p = calloc(1, sizeof(struct thread_pool))) == NULL)
        goto nomem;

    if ((r = pthread_mutex_init(&p->mutex, NULL)) != 0)
        goto failed;

    if (nthreads <= 0 || nthreads > MAX_THREADS)
        nthreads = MAX_THREADS;

    if (STACKSIZE(ssize)) {
        if ((attr = calloc(1, sizeof(pthread_attr_t))) == NULL)
            goto failed;

        if ((r = pthread_attr_init(attr)) != 0)
            goto failed;

        if ((r = pthread_attr_setstacksize(attr, ssize * 1000000)) != 0)
            goto failed;
    }

    p->s       = s;
    p->nthreads= nthreads;
    p->ssize   = ssize;
    p->process = process;

    for (i = 0; i < nthreads; i++) {
        /*if ssize == -1 attr == NULL*/
        r = pthread_create(&p->tids[i], attr, thread_worker, p);
    }

    *pool = p;
    rc = 0;

failed:
    if (attr) {
        pthread_attr_destroy(attr);
        free(attr);
    }
    if (rc == 0)
        return 0;

    pthread_mutex_destroy(&p->mutex);
    free(p);

nomem:
    fprintf(stderr, "error:%s:%s\n", strerror(r), strerror(errno));
    return -1;
}

static void *thread_worker(void *arg) {

    struct thread_pool *p = (struct thread_pool *)arg;
    struct sockaddr_in addr;
    char str[INET6_ADDRSTRLEN];
    socklen_t size;

    int connfd = -1;
    for (;;) {
        size = sizeof(addr);

        pthread_mutex_lock(&p->mutex);
        /*get client sock*/
        connfd = accept(p->s, (struct sockaddr *)&addr, &size);
        pthread_mutex_unlock(&p->mutex);

        if (inet_ntop(AF_INET, &addr.sin_addr, str, INET6_ADDRSTRLEN) == NULL)
            break;
        fprintf(stderr, "current socket %d:ip = %s\n", connfd, str);

        p->process(connfd);
    }

    fprintf(stderr, "error:%s\n", strerror(errno));
    return (void *)0;
}

static int   thread_pool_destroy(struct thread_pool *pool) {
    int i, n;

    n = pool->nthreads;
    for (i = 0; i < n; i++) {
        pthread_join(pool->tids[i], NULL);
    }

    pthread_mutex_destroy(&pool->mutex);
    free(pool);
    return 0;
}
