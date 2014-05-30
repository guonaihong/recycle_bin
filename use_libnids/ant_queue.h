#ifndef _ANT_QUEUE_H
#define _ANT_QUEUE_H
struct queue_elem {
    void *data;
    struct queue_elem *next;
};

struct queue_head {
    struct queue_elem *head;
    struct queue_elem *tail;
    int size;
};

struct queue_head * queue_new();

void queue_init(struct queue_head *qp);

int queue_empty(struct queue_head *qp);

struct queue_elem * 
Q_NEW(void *d, struct queue_elem *qe);

void queue_put(void *d, struct queue_head *qp);

void *queue_get(struct queue_head *qp);
#endif
