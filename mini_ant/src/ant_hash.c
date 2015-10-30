#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "ant_hash.h"

unsigned ant_hash(void *s, int size, int m) {
    unsigned val;
    unsigned char *p;

    val = 0;
    p   = (unsigned char *)s;
    if (size) {
        while (size-- > 0) {
            val = *p++ * 31 + val;
        }
    } else {
        while (*p) {
            val = *p++ * 31 + val;
        }
    }
    return val % m;
}

int ant_hash_init(ant_hash_t *hash, int ksize, int m, unsigned (*hash_func)(void *, int, int), int (*cmp)(const void *, const void *)) {
    if ((hash->hash = calloc(m, sizeof(void *))) == NULL)
        return -1;
    hash->count    = 0; 
    hash->m        = m;
    hash->ksize    = ksize;
    hash->length   = 0;
    hash->ant_hash = hash_func ? hash_func : ant_hash;
    hash->cmp      = cmp;
    return 0;
}

ant_hash_t *ant_hash_new(int m, int ksize, unsigned (*hash_func)(void *, int, int), int (*cmp)(const void *, const void*)) {
    ant_hash_t *hash = (ant_hash_t *)malloc(sizeof(ant_hash_t));
    if (hash == NULL)
        return NULL;
    if (ant_hash_init(hash, ksize, m, hash_func, cmp) == -1) {
        free(hash);
        return NULL;
    }
    return hash;
}

void *ant_hash_put(ant_hash_t *hash, const void *key, void *val) {
    ant_hash_elem_t *p;
    void *prev;
    void *head;

    p = ant_lookup(hash, key, NULL);
    if (!p) {
        head = hash->hash + hash->ant_hash((void *)key, hash->ksize, hash->m) * sizeof(ant_hash_elem_t *);
        p = malloc(sizeof(ant_hash_elem_t));
        p->key = key;
        p->val = val;
        p->next = *(ant_hash_elem_t **)head;
        *(ant_hash_elem_t**)head = p;

        prev = NULL;
        hash->length++;
        /*memcpy(head, &p, sizeof(ant_hash_elem_t *));*/
    } else {
        prev = p->val;
    }

    p->val = val;
    return prev;
}

void *ant_hash_get(ant_hash_t *hash, const void *key) {
    ant_hash_elem_t *p;

    p = ant_lookup(hash, key, NULL);
    return p ? p->val : NULL;
}

void *ant_hash_del(ant_hash_t *hash, const void *key) {
    ant_hash_elem_t *p;
    ant_hash_elem_t *prev;
    void *head;
    void *val;

    prev = NULL;
    val  = NULL;
    p = ant_lookup(hash, key,(void **) &prev);

    if (p) {
        head = hash->hash + hash->ant_hash((void *)key, hash->ksize, hash->m) * sizeof(ant_hash_elem_t *);
        /*printf("-----prev = %x::%x\n", prev, ant_hash((void *)key, hash->ksize, hash->m));*/
        if (prev == NULL)
            *(ant_hash_elem_t**)head = p->next;
        else 
            prev->next = p->next;

        val = p->val;
        free((void *)p->key);
        free(p);
        hash->length--;
    }
    return val;
}

void *ant_lookup(ant_hash_t *hash, const void *key, void **prev) {
    ant_hash_elem_t *p;
    void *mem;
    void *pr;

    pr = NULL;
    mem = hash->hash + hash->ant_hash((void *)key, hash->ksize, hash->m) * sizeof(ant_hash_elem_t *);
    p = *(ant_hash_elem_t**)mem;

    /* test count */
    int count = 0;

    if (hash->ksize) {
        for (; p; p = p->next) {

            count++;
            if (p->key && !memcmp(p->key, key, hash->ksize)) {
                /*fprintf(stderr, "find = %d\n",count);*/

                if (prev)
                    *prev = pr;
                return p;
            }
            if (prev)
                pr = p;
        }
    }
    else if (hash->cmp) {
        for (; p; p = p->next) {

            count++;
            if (p->key && !hash->cmp(p->key, key)) {
                /*fprintf(stderr, "find = %d\n",count);*/

                if (prev)
                    *prev = pr;
                return p;
            }

            if (prev)
                pr = p;
        }
    }
    else {
        for (; p; p = p->next) {

            count++;
            if (p->key && !strcmp(p->key, key)) {

                /*fprintf(stderr, "find = %d\n",count);*/
                if (prev)
                    *prev = pr;
                return p;
            }
            if (prev)
                pr = p;
        }
    }
    if (prev)
        *prev = NULL;
    return NULL;
}

