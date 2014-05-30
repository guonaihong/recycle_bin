#ifndef _ANT_HASH_H
#define _ANT_HASH_H


typedef struct ant_hash_elem_s {
    const void *key;
    void *val;
    struct ant_hash_elem_s *next;
}ant_hash_elem_t;

typedef struct ant_hash_s{
    void *hash; 
    int ksize;   /* key size -> if the key size is equal to 0, key musb be a char * */
    int count;  /* with a few buckets */
    int m;      /* bucket size */
    int length; /* number of elements */
    unsigned (*ant_hash)(void *, int , int);
}ant_hash_t;


unsigned ant_hash(void *s, int size, int m);

void *ant_lookup(ant_hash_t *hash, const void *key, void **prev);

int ant_hash_init(ant_hash_t *hash, int ksize, int m, unsigned (*hash_func)(void *, int, int));

ant_hash_t *ant_hash_new(int m, int size, unsigned (*hash_func)(void *, int, int));

void *ant_hash_put(ant_hash_t *hash, const void *key, void *val);

void *ant_hash_get(ant_hash_t *hash, const void *key);

void *ant_hash_del(ant_hash_t *hash, const void *key);

void *ant_lookup(ant_hash_t *hash, const void *key, void **prev);

#endif
