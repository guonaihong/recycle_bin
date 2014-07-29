#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

void *conn(void *arg) {
    struct timeval b, e;
    struct sockaddr *client = arg;
    int    r;
    int    s;

    gettimeofday(&b, NULL);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    r = connect(s, client, sizeof(*client));
    if (r != 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&e, NULL);

    printf("%ld\n", (e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec));
    write(s, "hello world\n", 12);
    close(s);
    return (void *)0;
}

#define MAX_THREAD 128
int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "%s <ip> <port> <thread num>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client;

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    if (inet_aton(argv[1], &client.sin_addr) == 0) {
        perror("inet_aton");
        exit(EXIT_FAILURE);
    }

    client.sin_port = htons(atoi(argv[2]));

    int i;
    int n;
    pthread_t tids[MAX_THREAD];

    n = atoi(argv[3]);
    if (n > MAX_THREAD)
        n = MAX_THREAD;

    for (i = 0; i < n; i++ ) {
        pthread_create(&tids[i], NULL, conn, &client);
    }

    for (i = 0; i < n; i++ ) {
        pthread_join(tids[i], NULL);
    }

    return 0;
}
