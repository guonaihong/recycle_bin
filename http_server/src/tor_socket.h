#ifndef ANT_SOCKET_H
#define ANT_SOCKET_H
#include <fcntl.h>
#include <netinet/in.h>

#define IS_NONBLOCK(sock) \
        (fcntl(sock, F_GETFL, 0) & O_NONBLOCK)

extern int create_and_bind(unsigned int ip, unsigned short port);

extern int set_fl(int fd, int flags);

extern ssize_t readn_nonblock(int fd, void *buf, size_t len, int flags);

extern ssize_t writen_nonblock(int fd, const void *bp, size_t len, int flags);

extern ssize_t sock_read(int fd, void *buf, size_t len, int flags);

#endif
