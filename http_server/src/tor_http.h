#ifndef _TOR_HTTP_H
#define _TOR_HTTP_H

#include <stdint.h>

#define MAX_URL_SIZE 4096
#define TOR_BUFSIZE 8192

typedef struct http_context {
    int  fd;
    char path[MAX_URL_SIZE];
    char fname[MAX_URL_SIZE];

    uint32_t debug:1;

    /* 0 --> connection: close 
     * 1 --> connection: keep-alive */
    uint32_t conn :2; 

    /* bufsize max 2 ^ 13 = 8192
     * read buf len  */
    uint32_t rblen : 14;
    
    uint32_t mm:1; /*mmanp(1) or malloc(0) */

    /* head */
    char *head;
    int huse;
    int hsize;

    /* page */
    char *page;
    int puse;
    int psize;

    int (*list_dir)(struct http_context *http);

    ssize_t (*read)(int fd, void *buf, size_t n, int flags);

    ssize_t (*write)(int fd, const void *buf, size_t n, int flags);

    void (*record_err)(char *fmt, ...);
} http_context;

extern http_context * http_new(
        int fd, 
        int (*list_dir)(struct http_context *http),
        ssize_t (*tor_read )(int fd, void *buf, size_t len, int flags),
        ssize_t (*tor_write)(int fd, const void *buf, size_t len, int flags)
        );

extern void http_free(http_context *http);

extern int http_read_head(http_context *http);

extern int http_join_head(http_context *http);
#endif
