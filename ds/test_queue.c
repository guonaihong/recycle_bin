#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

struct queue_node2 {
    void *data;
    struct queue_node queue;
};

int main() {

    struct queue_head head = QUEUE_HEAD_INIT;
    struct queue_node *pos;
    struct queue_node2 *tmp;

    tmp = malloc(sizeof(struct queue_node2));
    tmp->data = strdup("aaaaaaaaaaaaaaa");
    queue_put(&tmp->queue, &head);

    tmp = malloc(sizeof(struct queue_node2));
    tmp->data = strdup("bbbbbbbbbbbbbbb");
    queue_put(&tmp->queue, &head);

    tmp = malloc(sizeof(struct queue_node2));
    tmp->data = strdup("ccccccccccccccc");
    queue_put(&tmp->queue, &head);

    tmp = malloc(sizeof(struct queue_node2));
    tmp->data = strdup("ddddddddddddddd");
    queue_put(&tmp->queue, &head);

#if 0
    queue_for_each(pos, &head) {
        tmp = queue_entry(pos, struct queue_node2, queue);
        printf("%s\n", (char *)tmp->data);
    }
#endif

    while (!queue_empty(&head)) {
        pos = queue_get(&head);
        tmp = queue_entry(pos, struct queue_node2, queue);
        printf("%s\n", (char *)tmp->data);
        free(tmp->data);
        free(tmp);
    }

    return 0;
}
