#ifndef ANT_SOCKET_H
#define ANT_SOCKET_H
#include <fcntl.h>

#define IS_NONBLOCK(sock) \
        (fcntl(sock, F_GETFL, 0) & O_NONBLOCK)

int readn(int fd, char *bp, size_t len);

int writen( int fd, char *bp, size_t len );

int readn_nonblock(int fd, char *bp, size_t len);

int writen_nonblock(int fd, char *bp, size_t len);
#endif
