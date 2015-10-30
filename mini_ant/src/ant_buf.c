#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ant_buf.h"

void ant_buf_init(ant_buf_t *buf, int length) {
    assert(buf != NULL);
    assert(length >= 0);

    buf->buf = (length == 0) ? NULL : malloc(length);
    buf->length = length;
    buf->ncurr = 0; /*ncurr record the current number of bytes*/
    buf->nm_alloc = 0; /* The number of memory allocation */
    buf->flag = 0; /* 1 is heap, 0 is stack*/
}

ant_buf_t * ant_buf_new(int length) {
    ant_buf_t *buf = malloc(sizeof(ant_buf_t));
    assert(buf != NULL);

    ant_buf_init(buf, length);
    buf->flag = 1;
    return buf;
}

int ant_buf_append(ant_buf_t *buf, void *elem, int size) {
    int needed;
    char *newp;

    assert(buf != NULL);
    assert(elem != NULL);
    assert(size >= 0);

    if (buf->buf == NULL || buf->length == 0) {
        buf->length = size + 500;
        buf->buf = malloc(buf->length);
        if (buf->buf == NULL)
            return -1;
        buf->nm_alloc = 1;
    }
    else {
        needed = buf->ncurr + size + 1;
        if (needed > buf->length) {
            if (needed < 2 * buf->length) {
                needed = 2 * buf->length;
            }
            newp = calloc(needed, 1);
            if (newp == NULL)
                return -1;
            buf->nm_alloc++;
            memcpy(newp, buf->buf, buf->ncurr);

            /* free old */
            free(buf->buf);
            buf->buf = newp;
            buf->length = needed;
        }

    }

    memcpy(buf->buf + buf->ncurr, elem, size);
    buf->ncurr += size;
    return 0;
}

void ant_buf_free(ant_buf_t *buf) {
    assert(buf != NULL);
    free(buf->buf);
    if (buf->flag)
        free(buf);
}

void ant_buf_clear(ant_buf_t *buf) {
    assert(buf != NULL);
    memset(buf->buf, '\0', buf->ncurr);
    buf->ncurr = 0;
}
