#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 1234
static int listen_new(unsigned ip, unsigned short port) {
    struct sockaddr_in ser;
    int listen_fd, rv;

    ser.sin_family = AF_INET;
    ser.sin_addr.s_addr = ip; 
    ser.sin_port   = port;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
        return -1;

    rv = bind(listen_fd, (struct sockaddr *)&ser, sizeof(struct sockaddr_in));
    if (rv == -1)
        return -1;

    if (listen(listen_fd, 0) == -1) {
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

int file2mem(const char *fname, char **p, int *size) {
    char *mem = NULL;
    struct stat sb; 
    int fd = -1; 

    if(stat(fname, &sb) == -1) {
        perror("stat");
        return -1; 
    }   

    mem = (char *)malloc(sb.st_size);
    if (!mem) {
        perror("malloc");
        return -1; 
    }   

    fd = open(fname, O_RDONLY, 0644);
    if (fd == -1) {
        perror("open");
        free(mem);
        return -1; 
    }   

    read(fd, mem, sb.st_size);
    *p = mem;
    *size = sb.st_size;
    close(fd);
    return 0;
}

#define RSP_BODY_MEDIA "HTTP/1.0 200 OK\r\n" \
"Content-Type: audio/mpeg\r\n" \
"Content-Length: %d\r\n" \
"Connection: close\r\n\r\n"

#define RSP_BODY_FILE "HTTP/1.0 200 OK\r\n" \
"Content-Length: %d\r\n" \
"Connection: close\r\n\r\n"

void *do_work(void *argv) {
    char  header[4096] = "";
    char  rsp[4096]    = "";
    char  path[1024]   = ".";
    int   connfd       = -1;
    int   rv           = -1;
    char *pos, *p;
    int   size = 0;

    connfd = (intptr_t)argv;
    rv = recv(connfd, header, sizeof(header) - 1, MSG_PEEK);
    if (rv == -1) {
        printf("recv peek fail %s\n", strerror(errno));
        goto done;
    }

    printf("===%s===\n", header);
    if (strncmp("GET", header, 3)) {
        printf("header is %s\n", header);
        goto done;
    }

    pos = strstr(header, "\r\n\r\n");
    if (pos == NULL) {
        printf("not found \\r\\n\\r\\n \n");
        goto done;
    }

    /*read */
    rv = recv(connfd, header, pos - header + 4, 0);
    sscanf(header, "GET %[^ ]", path + 1);
    printf("path(%s)\n", path);

    rv = file2mem(path, &p, &size);
    if (rv == -1) {
        printf("file2mem fail\n");
        return 0;
    }

    if (strncmp(path + strlen(path) - 4, ".mp3", 4) == 0) {
        printf("mp3 type");
        rv = snprintf(rsp, sizeof(rsp) - 1, RSP_BODY_MEDIA, size);
    } else {
        rv = snprintf(rsp, sizeof(rsp) - 1, RSP_BODY_FILE, size);
    }

    /*write rsp*/
    write(connfd, rsp, rv);
    /*write body*/
    write(connfd, p, size);
done:
    close(connfd);

    return 0;
}

void main_loop() {
    int listen_fd = listen_new(0, htons(PORT));
    int connfd    = -1;

    if (listen_fd == -1) {
        printf("listen error\n");
        return ;
    }
    
    printf("listen port = %d\n", PORT);
    for (;;) {
        connfd = accept(listen_fd, NULL, NULL);
        printf("connfd = %d\n", connfd);
        do_work((void *)(intptr_t)connfd);
    }
}

int main(int argc, char **argv) {

    main_loop();
    return 0;
}
