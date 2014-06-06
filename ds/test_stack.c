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

	tmp = calloc(1, sizeof(struct stack_node2));
	tmp->data = strdup("123456789");
	stack_push(&(tmp->stack), &head);

	tmp = calloc(1, sizeof(struct stack_node2));
	tmp->data = strdup("aaaaaaaaaa");
	stack_push(&(tmp->stack), &head);

	tmp = calloc(1, sizeof(struct stack_node2));
	tmp->data = strdup("bbbbbbbbbb");
	stack_push(&(tmp->stack), &head);

#if 0
	stack_for_each(pos, &head) {
		pos2 = stack_entry(pos, struct stack_node2, stack);
		printf("==========%p:%s\n", pos2, (char *)pos2->data);
	}
#endif
	while (!stack_empty(&head)) {
		pos = stack_pop(&head);
		pos2 = stack_entry(pos, struct stack_node2, stack);
		printf("%s\n", (char *)pos2->data);
		free(pos2->data);
		free(pos2);
	}
	return 0;
}
