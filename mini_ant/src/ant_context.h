#ifndef _ANT_CONTEXT_H
#define _ANT_CONTEXT_H
#include "ant_hash.h"
#include "ant_queue.h"
#include "ant_http.h"
#include "ant_macro.h"

#ifndef IP_LEN
#    define IP_LEN 4
#endif

#if 0
#define close(fd) \
    fprintf(stderr, "close(fd)::%d\n", (fd)); \
    close(fd);
#endif

struct cache_val{
    int flags; /* 1 is parse ok*/
    time_t ctime; /* create time */
    time_t mtime; /* modify time */
    unsigned ip[IP_LEN];
    struct queue_head *down;
};

#define ANT_CACHE_URL 0
#define ANT_CACHE_DNS 1
#define ANT_QUEUE_DNS 2
#define ANT_DOWN 3
#define ANT_ITEM_DOWN 4
typedef struct total_context_s{
    ant_hash_t *cache_url;
    ant_hash_t *cache_dns;

    struct queue_head *queue_dns;
    struct queue_head *down;

    pthread_mutex_t mutex[5];
}total_context_t;

extern void ant_total_context_init(total_context_t *total);

struct status_info{
    int queue_num;
    int should_down;
    int total_down; /*Total number of downloads*/
    time_t btime;      /*Running time of the program*/
    time_t etime;      /*End of program running time*/
    int speed;
};

extern void cache_url_init(total_context_t *total, char *fname);


extern void uniq_init();

/* 1 exists
 * 0 does not exists */
extern int uniq_exists(char *url);

extern void uniq_put(char *url);
#endif
