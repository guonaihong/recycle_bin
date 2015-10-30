#ifndef _ANT_HTTP_H
#define _ANT_HTTP_H

#define MAX_URL_SIZE 4096
#define MAX_FILE_SIZE 128
#include <stdint.h>
#include "ant_buf.h"
struct file_info{
    char filename[MAX_FILE_SIZE];
};

typedef struct down_s{
    char      *url;      /* get or pos url*/
    char      *fname;    /* file name     */
    unsigned  *ip;       /* host ip       */
    unsigned  ip2;       /* host ip       */
    unsigned short port; /* host port */
}down_t;

#define ANT_WRITE 0x1
#define ANT_READ_HEAD 0x2
#define ANT_READ_BODY 0x4
#define ANT_DUMP 0x7

struct http_info {
    int fd;

    int64_t  filesize;
    int64_t chunksize;
    int     linecount;
    int     http_code;
    int    end_header; /* 1 */

    int32_t chunkhead;

    int           my_errno;
    uint32_t      isgzip:1;  /* 1 is gzip */
    uint32_t      nfree:1;  /*Don't free http_info equal to 1*/
    uint32_t      close:1; /*if Connection: close then 1 else 0*/
    uint32_t      try_num:4;
    uint32_t      status:3; /* read_body| read_head | send_head = ANT_DUMP */

    char    location[MAX_URL_SIZE];
    char   *cookies;
    struct file_info fi;

    ant_buf_t *page; /* html */

    down_t *down;
};

extern char *setfilename(struct http_info *http);

extern int http_read_head(struct http_info *http);

extern int http_read_body(struct http_info *http);

extern int http_send_head(struct http_info *http);

#ifndef ant_error
#define    ant_error(msg ) \
           do{ perror(msg); return -1;}while(0)
#endif

#endif
