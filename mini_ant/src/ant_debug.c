#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ant_debug.h"
#include "ant_macro.h"

/*print port and addresses based on sock*/
void ant_debug_sock(int sock) {
    struct sockaddr_in addr;
    int olderrno;
    socklen_t  len = sizeof(addr);
    
    olderrno = errno;
    memset(&addr, '\0', sizeof(addr));
    if (getpeername(sock, (struct sockaddr*)&addr, &len) < 0) {
        errno = olderrno;
    }
    PRINT_IP(addr.sin_addr.s_addr);
    fprintf(stderr, "port = %hd\n", ntohs(addr.sin_port));
}

void ant_debug_sock_url(int sock, char *url) {
    ant_debug_sock(sock);
    fprintf(stderr, "url = %s\n", url);
}
