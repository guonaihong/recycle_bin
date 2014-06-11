#ifndef HO_QUEUE_H
#define HO_QUEUE_H

#include <stdlib.h>
#undef offsetof
struct queue_head {
    struct queue_node *head;
    struct queue_node *tail;
};

struct queue_node {
    struct queue_node *next;
};

#define QUEUE_HEAD_INIT {.head = NULL, .tail = NULL}
#define INIT_QUEUE_HEAD(ptr) do {\
    (ptr)->head = NULL;(ptr)->tail = NULL; \
} while (0)

#define offsetof(type, member) ((size_t)&((type *)0)->member)
#define queue_entry(ptr, type, member) (type *)((char *)ptr - offsetof(type, member))

static inline void queue_put(struct queue_node *n, struct queue_head *h) {
    n->next = NULL;
    if (!h->head) {
        h->head = h->tail = n;
    } else {
        h->tail->next = n;
        h->tail = n;
    }
}

static inline struct queue_node *queue_get(struct queue_head *h) {
    struct queue_node *t = h->head;
    h->head = h->head ? h->head->next: NULL;
    return t;
}

static inline int queue_empty(struct queue_head *h) {
    return !h->head;
}

#define queue_for_each(pos, h) \
    for(pos = (h)->head; pos; pos = pos->next)
#endif
