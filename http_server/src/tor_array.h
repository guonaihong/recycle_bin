#ifndef _ANT_ARRAY_H
#define _ANT_ARRAY_H

typedef struct tor_array_s {
    void   *array;
    int    ncurr;
    size_t size;  /* element size */
    int    length;
    char   flag; /* 1 is heap 0 is stack */
} tor_array_t;

extern int iszero(void *s, size_t n);

extern tor_array_t *tor_array_new(int length, size_t size);

extern int tor_array_init(tor_array_t *array, int length, size_t size);

extern void *tor_array_put(tor_array_t *array, int i, void *elem);

extern void *tor_array_get(tor_array_t *array, int i);

extern void *tor_array_clear(tor_array_t *array, int i);

extern void tor_array_free(tor_array_t *array, void (*user_free)(void *));

#endif
