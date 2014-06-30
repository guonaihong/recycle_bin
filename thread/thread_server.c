#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int tcp_listen(char *ip, char *port);

#define MAX_THREADS 512
struct thread_pool {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;

	pthread_t       tids[MAX_THREADS];
	int			    connfd;
	int				ready;

	void (*process)(int s);
};

static int   thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s));
static void  thread_main(struct thread_pool *pool, int s);
static void *thread_worker(void *arg);

static void  process_msg(int s);

void process_msg(int s) {
	char buf[128];
	int r;
	r = read(s, buf, sizeof(buf));
	write(1, buf, r);
	close(s);
}

int main(int argc, char **argv) {

    int s;
	int nthread = -1;

    if (argc == 2) {
		s = tcp_listen(NULL, argv[1]);
	} else if (argc == 3 || argc == 4) {
		s = tcp_listen(argv[1], argv[2]);
		nthread = atoi(argv[3]);
	} else {
		fprintf(
				stderr, 
				"Usage: \r\n"
				"%s <port> \r\n"
				"%s <ip> <port> \r\n"
				"%s <ip> <port> <number of threads:default %d>\r\n"
				,argv[0]
				,argv[0]
				,argv[0], MAX_THREADS
			   );
		return 1;
	}

	struct thread_pool *pool;

	int r = thread_pool_create(&pool, nthread, process_msg);
	printf("thread pool create = %d\n", r);
	thread_main(pool, s);
	close(s);

    return 0;
}

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
    server.sin_addr.s_addr = ip ? htonl(atoi(ip)) : htonl(INADDR_ANY);

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

static int thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s)) {
	int i, r;

	struct thread_pool *p = NULL;
	p = calloc(1, sizeof(struct thread_pool));
	if (!p)
		goto failed;

	if ((r = pthread_mutex_init(&p->mutex, NULL)) != 0)
		goto failed;

	if ((r = pthread_cond_init(&p->cond, NULL)) != 0)
		goto failed;

	if (nthreads <= 0)
		nthreads = MAX_THREADS;

	for (i = 0; i < nthreads; i++) {
		r = pthread_create(&p->tids[i], NULL, thread_worker, p);
	}

	p->process = process;
	*pool = p;
	return 0;

failed:
	fprintf(stderr, "error:%s:%s\n", strerror(r), strerror(errno));

	pthread_mutex_destroy(&p->mutex);
	pthread_cond_destroy(&p->cond);
	free(p);
	return -1;
}

static void *thread_worker(void *arg) {

	struct thread_pool *p = (struct thread_pool *)arg;
	
	int connfd = -1;
	for (;;) {
		pthread_mutex_lock(&p->mutex);

		while (p->ready == 0) {
			pthread_cond_wait(&p->cond, &p->mutex);
		}
		connfd   = p->connfd;
		p->ready = 0;

		pthread_mutex_unlock(&p->mutex);

		p->process(connfd);
	}
	return (void *)0;
}

static void thread_main(struct thread_pool *pool, int s) {
	int connfd;

	for (;;) {
		/*get client sock*/
		connfd = accept(s, NULL, NULL);
		fprintf(stderr, "current sock = %d\n", connfd);
		if (connfd == -1) {
			perror("accept");
			break;
		}

		pthread_mutex_lock(&pool->mutex);
		pool->connfd = connfd;
		pool->ready  = 1;
		pthread_cond_signal(&pool->cond);
		pthread_mutex_unlock(&pool->mutex);
	}
}
