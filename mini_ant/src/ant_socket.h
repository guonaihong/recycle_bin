#ifndef ANT_SOCKET_H
#define ANT_SOCKET_H
#include <fcntl.h>
#include <netinet/in.h>

#define IS_NONBLOCK(sock) \
        (fcntl(sock, F_GETFL, 0) & O_NONBLOCK)

extern int sock_read(int fd, void *buf, size_t len, int flags);

extern int sock_read_isready(int fd, long tv_sec, long tv_usec);

extern int sock_create(struct sockaddr_in *peer, unsigned int ip, unsigned short port);

extern int sock_nonblock_create(struct sockaddr_in *peer, unsigned int ip, unsigned short port);

extern int set_fl(int fd, int flags);

extern int readn(int fd, void *buf, size_t len);

extern int writen( int fd, char *bp, size_t len );

extern int readn_block(int fd, void *buf, size_t len);

extern int readn_nonblock(int fd, void *buf, size_t len, int *my_errno);

extern int writen_nonblock(int fd, char *bp, size_t len);
#endif
