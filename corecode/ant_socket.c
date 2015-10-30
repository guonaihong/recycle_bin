#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>

int readn( int fd, char *bp, size_t len ) {
    int cnt;
    int rc;

    cnt = len;
    while( cnt > 0 ) {
        if ((rc = recv( fd, bp, cnt, 0 )) < 0 ){
            if ( errno == EINTR )
                continue;
            return -1;
        }

        if ( rc == 0 )
            return len - cnt;
        bp  += rc;
        cnt -= rc;
    }
    return len;
}

int writen( int fd, char *bp, size_t len ) {
    int cnt;
    int rc;

    cnt = len;
    while( cnt > 0 ) {
        if ((rc = send( fd, bp, cnt, 0 )) < 0 ){
            if ( errno == EINTR )
                continue;
            return -1;
        }

        if ( rc == 0 )
            return len - cnt;
        bp  += rc;
        cnt -= rc;
    }
    return len;
}

int readn_nonblock(int fd, char *bp, size_t len) {
    int    cnt;
    int    rc;
    fd_set rfds;

    cnt = len;

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
