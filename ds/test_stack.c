#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

struct stack_node2 {
    void *data;
    struct stack_node stack;
};

int main() {

    struct stack_head head = STACK_HEAD_INIT;
    struct stack_node2 *tmp, *pos2;
    struct stack_node *pos;

    tmp = malloc(sizeof(struct stack_node2));
    tmp->data = strdup("11111111");
    stack_push(&(tmp->stack), &head);

    tmp = malloc(sizeof(struct stack_node2));
    tmp->data = strdup("22222222");
    stack_push(&(tmp->stack), &head);

    tmp = malloc(sizeof(struct stack_node2));
    tmp->data = strdup("33333333");
    stack_push(&(tmp->stack), &head);

#if 0
    stack_for_each(pos, &head) {
        pos2 = stack_entry(pos, struct stack_node2, stack);
        printf("==========%p:%s\n", pos2, (char *)pos2->data);
    }
#endif

    /*test stack_top*/
    pos = stack_top(&head);
    pos2 = stack_entry(pos, struct stack_node2, stack);
    printf("%s\n", (char *)pos2->data);

    while (!stack_empty(&head)) {
        pos = stack_pop(&head);
        pos2 = stack_entry(pos, struct stack_node2, stack);
        printf("%s\n", (char *)pos2->data);
        free(pos2->data);
        free(pos2);
    }
    return 0;
}
