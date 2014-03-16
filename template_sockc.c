#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>

#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {

    int s;
    int r;

    if (argc != 3) {
        fprintf(stderr, "%s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client;
    s = socket(AF_INET, SOCK_STREAM, 0);
    assert(s != -1);

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    if (inet_aton(argv[1], &client.sin_addr) == 0) {
        perror("inet_aton");
        exit(EXIT_FAILURE);

    }

    client.sin_port = htons(atoi(argv[2]));
    r = connect(s, (struct sockaddr*)&client, sizeof(client));

    if (r != 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    write(s, "hello world\n", 12);
    close(s);
    return 0;
}
