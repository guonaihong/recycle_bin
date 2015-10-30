#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>
#include "ant_array.h"
int iszero(void *s, size_t n) {
    char *p;
    int cnt;

    p   = (char *)s;
    cnt = n;

    for (; cnt >= 4; cnt -= 4, p += 4) {
        if (*(uint32_t*)p != 0x0)
            return 0;
    }

    for (;cnt > 0; cnt--, p++) {
        if (*p != 0x0)
            return 0;
    }
    return 1;
}


int ant_array_init(ant_array_t *array, int length, size_t size) {
    if ((array->array  = calloc(length, size)) == NULL)
        return -1;

    array->size   = size;
    array->ncurr = 0;
    array->length = length;
    array->flag   = 0;

    return 0;
}

ant_array_t *ant_array_new(int length, size_t size) {

    ant_array_t *array = malloc(sizeof(ant_array_t));
    if (array == NULL)
        return NULL;

    if (ant_array_init(array, length, size) == -1) {
        perror("ant_array_init");
        free(array);
        return NULL;
    }
    array->flag = 1;
    return array;
}

void *ant_array_put(ant_array_t *array, int i, void *elem) {
    int n;

    /*
     * array expansion
     * */
    if (i >= array->length) {

        n = (int)ceil(i / (double)array->length);
        
        if (n == 1)
            n = 2;

        void *new = calloc(array->length * n, array->size);
        if (new == NULL)
            return NULL;

        memcpy(new, array->array, array->length * array->size);
        free(array->array);
        array->array  = new;
        array->length = array->length * n;

    }

    if (iszero(array->array + i * array->size, array->size))
        array->ncurr++;
    memcpy(array->array + i * array->size, elem, array->size);
    
    return elem;
}

void *ant_array_get(ant_array_t *array, int i) {
    assert(i >= 0 && i < array->length);

    return array->array + i * array->size;
}

void *ant_array_clear(ant_array_t *array, int i) {
    assert(i >= 0 && i < array->length);

    array->ncurr--;
    memset(array->array + i *array->size, '\0', array->size);
    return array->array + i * array->size;
}

void ant_array_free(ant_array_t *array, void (*user_free)(void *)) {
    if (user_free) {
        ;
    }
    free(array->array);
    if (array->flag)
        free(array);
}

