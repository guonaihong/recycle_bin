#ifndef _ANT_ARRAY_H
#define _ANT_ARRAY_H

typedef struct ant_array_s {
    void   *array;
    int    ncurr;
    size_t size;  /* element size */
    int    length;
    char   flag; /* 1 is heap 0 is stack */
} ant_array_t;

extern int iszero(void *s, size_t n);

extern ant_array_t *ant_array_new(int length, size_t size);

extern int ant_array_init(ant_array_t *array, int length, size_t size);

extern void *ant_array_put(ant_array_t *array, int i, void *elem);

extern void *ant_array_get(ant_array_t *array, int i);

extern void *ant_array_clear(ant_array_t *array, int i);

extern void ant_array_free(ant_array_t *array, void (*user_free)(void *));

#endif
