#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int tcp_listen(char *ip, char *port);

#define MAX_THREADS 256
struct thread_pool {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;

	pthread_t       tids[MAX_THREADS];
	int			    connfd;
	int				ready;

	void (*process)(int s);
};

/*server code*/
static int   thread_pool_create(struct thread_pool **pool, int nthreads, void (*process)(int s));
static void  thread_main(struct thread_pool *pool, int s);
static void *thread_worker(void *arg);
/*server code*/

static void  process_msg(int s);

void process_msg(int s) {
	char buf[128];
	int r;
	r = read(s, buf, sizeof(buf));
	write(1, buf, r);
	close(s);
}

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
