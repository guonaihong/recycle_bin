#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tor_socket.h"

int create_and_bind(unsigned int ip, unsigned short port) {
    struct sockaddr_in local;
    int connfd;
    int val;
    int opt = 1;

    if ((connfd = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
        perror("socket");
        return -1;
    }

    memset( &local, '\0', sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr= ip;

    if (setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("");
        exit(EXIT_FAILURE);
    }
        
    if (bind(connfd, (struct sockaddr*)&local, sizeof(local)) < 0) {
        perror("");
        exit(EXIT_FAILURE);
    }
    return connfd;
}

int set_fl(int fd, int flags) {
    int val;
    val = fcntl(fd, F_GETFL, 0);
    if (val == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }

    val |= flags;
    if (fcntl(fd, F_SETFL, val) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

ssize_t sock_read(int fd, void *buf, size_t len, int flags) {
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    tv.tv_sec  = 0;
    tv.tv_usec = 100;

    select(fd + 1, &rfds, NULL, NULL, &tv);

    return recv(fd, buf, len, flags);
}

ssize_t readn_nonblock(int fd, void *buf, size_t len, int flags) {
    int    cnt;
    int    rc;
    char   *bp;
    fd_set rfds;

    cnt = len;
    bp = buf;
    
    while (cnt > 0) {
        rc = recv(fd, bp, cnt, flags);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);
                struct timeval tv = {
                    .tv_sec = 0,
                    .tv_usec = 100
                };
                rc = select(fd + 1, &rfds, NULL, NULL, &tv);
                if (rc < 0)
                    return -1;
                else if (rc == 0)
                    return len - cnt;
                continue;
            }
            return -1;
        }
        else if (rc == 0)
            return len - cnt;

        cnt -= rc;
        bp  += rc;
    }
    return len;
}

ssize_t writen_nonblock(int fd, const void *buf, size_t len, int flags) {
    int    cnt;
    int    rc;
    char  *bp;
    fd_set wfds;

    cnt = len;
    bp = (char *)buf;

    while (cnt > 0) {
        rc = send(fd, bp, cnt, 0);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                FD_ZERO(&wfds);
                FD_SET(fd, &wfds);
                if (select(fd + 1, NULL, &wfds, NULL, NULL) < 0) {
                    fprintf(stderr, "writen_nonblock select error:%m\n");
                    return -1;
                }
                continue;
            }
            return -1;
        }
        else if (rc == 0)
            return len - cnt;

        cnt -= rc;
        bp  += rc;
    }
    return len;
}
