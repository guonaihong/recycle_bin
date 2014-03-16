#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {

    int s;
    int r;

    struct sockaddr_in server;
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port   = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    r = bind(s, (struct sockaddr*)&server, sizeof(server));
    if (r == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(s, 10);

    int connfd;
    connfd = accept(s, NULL, NULL);
    if (connfd == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char line[128];
    for(;;) {
        r = read(connfd, line, sizeof(line) - 1);
        if (r != -1)
            write(1, line, r);
    }

    close(connfd);
    close(s);
    return 0;
}
