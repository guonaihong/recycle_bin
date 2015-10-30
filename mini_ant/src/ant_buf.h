#ifndef _ANT_BUF_H
#define _ANT_BUF_H
#include <stdint.h>
typedef struct ant_buf {
    void *buf;
    int  length;
    int  ncurr;             /* ncurr record the current number of bytes*/

    uint32_t  flag:1;       /* 1 is heap, 0 is stack*/
    uint32_t  nm_alloc:31;  /* The number of memory allocation */
} ant_buf_t;

void ant_buf_init(ant_buf_t *buf, int length);

ant_buf_t * ant_buf_new(int length);

int ant_buf_append(ant_buf_t *buf, void *elem, int size);

void ant_buf_free(ant_buf_t *buf);

void ant_buf_clear(ant_buf_t *buf);
#endif
