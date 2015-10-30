#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ant_socket.h"

int sock_nonblock_create(struct sockaddr_in *peer, unsigned int ip, unsigned short port) {
    int connfd;

    if ((connfd = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
        perror( "socket" );
        return -1;
    }

    memset( peer, '\0', sizeof(*peer) );
    peer->sin_family = AF_INET;
    peer->sin_port = htons(port);
    peer->sin_addr.s_addr= ip;

    set_fl(connfd, O_NONBLOCK);

    return connfd;
}

int sock_create(struct sockaddr_in *peer, unsigned int ip, unsigned short port) {
    int connfd;

    if ((connfd = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
        perror( "socket" );
        return -1;
    }

    memset( peer, '\0', sizeof(*peer) );
    peer->sin_family = AF_INET;
    peer->sin_port = htons(port);
    peer->sin_addr.s_addr= ip;

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

int sock_read(int fd, void *buf, size_t len, int flags) {
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    if (select(fd + 1, &rfds, NULL, NULL, NULL) < 0) {
        fprintf(stderr, "in %s select %m\n", __FUNCTION__);
        return -1;
    }
    return recv(fd, buf, len, flags);
}

int sock_read_isready(int fd, long tv_sec, long tv_usec) {
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec = tv_sec;
    tv.tv_usec = tv_usec;

    return select(fd + 1, &rfds, NULL, NULL, &tv);

}

int readn_block(int fd, void *buf, size_t len) {
    int    cnt;
    int    rc;
    char   *bp;
    fd_set rfds;

    cnt = len;
    bp = buf;

    while (cnt > 0) {
        rc = recv(fd, bp, cnt, 0);
        if (rc < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                FD_ZERO(&rfds);
                FD_SET(fd, &rfds);
                if (select(fd + 1, &rfds, NULL, NULL, NULL) < 0) {
                    fprintf(stderr, "readn_nonblock select error:%m\n");
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

int readn_nonblock(int fd, void *buf, size_t len, int *my_errno) {
    int    cnt;
    int    rc;
    char   *bp;
    fd_set rfds;

    cnt = len;
    bp = buf;

    if (my_errno)
        *my_errno = 0;
    while (cnt > 0) {
        rc = recv(fd, bp, cnt, 0);
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
                else if (rc == 0) {
                    if (my_errno)
                        *my_errno = errno;
                    return len - cnt;
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

int writen_nonblock(int fd, char *bp, size_t len) {
    int    cnt;
    int    rc;
    fd_set wfds;

    cnt = len;

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
