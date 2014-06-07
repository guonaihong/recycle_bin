#ifndef HO_STACK_H
#define HO_STACK_H

#include <stdlib.h>
struct stack_head {
	struct stack_node *first;
};

struct stack_node {
	struct stack_node *next;
};

#define STACK_HEAD_INIT {.first = NULL}
#define INIT_STACK_HEAD(ptr) ((ptr)->first = NULL)

#ifndef offsetof
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

#define stack_entry(ptr, type, member) (type *)((char *)ptr - offsetof(type, member))

static inline void stack_push(struct stack_node *n, struct stack_head *h) {
	n->next  = h->first;
	h->first = n;
}

static inline struct stack_node *stack_pop(struct stack_head *h) {
	struct stack_node *t;
	t = h->first;
	h->first = h->first ? h->first->next : NULL;
	return t;
}

static inline int stack_empty(struct stack_head *h) {
	return !h->first;
}

#define stack_for_each(pos, head) \
	for(pos = (head)->first; pos; pos = pos->next)
#endif
