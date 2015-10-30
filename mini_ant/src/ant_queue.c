#include <stdlib.h>
#include <assert.h>
#include "ant_queue.h"

struct queue_head *queue_new() {
	struct queue_head *queue;

	queue = malloc(sizeof(struct queue_head));
	if (queue == NULL) {
		return NULL;
	}
	queue_init(queue);
    queue->flag = 1;
	return queue;
}

void queue_init(struct queue_head *qp) {
	qp->head = NULL;
	qp->tail = NULL;
	qp->size = 0;
    qp->flag = 0;
}

int queue_empty(struct queue_head *qp) {
	return qp->size == 0;
}
struct queue_elem * 
Q_NEW(void *d, struct queue_elem *qe) {
	struct queue_elem *e = malloc(sizeof(*e));
	e->data  = d;
	e->next  = qe;
	return e;
}
void queue_put(void *d, struct queue_head *qp) {
	if(qp->head == NULL) {
		qp->head = qp->tail = Q_NEW(d, qp->head);
		qp->size++;
		return ;
	}
    
	qp->tail->next = Q_NEW(d, qp->tail->next);
	qp->tail = qp->tail->next;
	qp->size++;
}

void *queue_get(struct queue_head *qp) {
	struct queue_elem *t = qp->head->next;
	void *d = qp->head->data;
	free(qp->head);
	qp->head = t;
	qp->size--;
	return d;
}

#include <stdio.h>
void queue_join_init(struct queue_head *q1, struct queue_head *q2) {
    assert(q1!= NULL);
    assert(q2!= NULL);
    if (q2->head == NULL)
        return;

    if (q1->head == NULL) {
        q1->head = q2->head;
        q1->tail = q2->tail;
        q1->size = q2->size;

    } else {
        q1->tail->next = q2->head;
        q1->tail = q2->tail;
        q1->size = q1->size + q2->size;
    }
    q2->tail = q2->head = NULL;
    q2->size = 0;
}

void queue_free(struct queue_head *q) {
    if (q->flag)
        free(q);
}
