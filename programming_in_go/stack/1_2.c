#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct my_stack_t my_stack_t;
struct my_stack_t {
    void **s;  /* 元素为void *的数组 */
    int use;   /* 已存入的元素个数 */
    int size;  /* 在内存中已分配的个数 */
    void (*do_free)(void *);
};

#define DEFAULT_N 100
int stack_init(my_stack_t *s, void (*do_free)()) {
    assert(s != NULL);
    s->s = (void **)malloc(sizeof(void *) * DEFAULT_N);
    s->use = 0;
    s->size = DEFAULT_N;
    s->do_free = do_free;
    return 0;
}

int stack_push(my_stack_t *s, void *v) {
    assert(s != NULL);
    void **p;
    if (s->use == s->size) {
        p = (void **)realloc(s->s, s->size * 2);
        if (p == NULL)
            return -1;
        s->s = p;
        s->size *= 2;
    }
    s->s[s->use++] = v;
    return 0;
}

void *stack_pop(my_stack_t *s) {
    assert(s != NULL);
    if (s->use == 0) {
        return NULL;
    }

    return s->s[--s->use];
}

void *stack_top(my_stack_t *s) {
    assert(s != NULL);
    if (s->use == 0) {
        return NULL;
    }
    return s->s[s->use - 1];
}

int stack_len(my_stack_t *s) {
    return s->use;
}

int stack_free(my_stack_t *s) {
    void *v;
    if (s->do_free == NULL) {
        goto done;
    }

    while ((v = stack_pop(s)) != NULL) {
        s->do_free(s);
    }
done:
    free(s->s);
    return 0;
}

int main() {
    my_stack_t s;
    stack_init(&s, NULL);

    stack_push(&s, "12345");
    stack_push(&s, "45678");
    stack_push(&s, "99999");
    stack_push(&s, "bbbbbb");
    stack_push(&s, "cccccc");

    char *p;
    for (;;) {
        p = stack_pop(&s);
        if (p == NULL) break;
        printf("%s\n", p);
    }
}
