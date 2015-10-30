#include <stdio.h>
#include "../src/ant_buf.h"

char msg[] = "test ant_buf.c";

void test(void) {
    ant_buf_t *page;
    page = ant_buf_new(0);
    printf("1.---------\n");
    ant_buf_append(page, "1234", 4);
    printf("ncurr = %d: len = %d: %s\n", page->ncurr, page->length, (char *)page->buf);
    ant_buf_append(page, "5678", 4);
    printf("ncurr = %d: len = %d: %s\n", page->ncurr, page->length, (char *)page->buf);
    ant_buf_append(page, "9abc", 4);
    printf("ncurr = %d: len = %d: %s\n", page->ncurr, page->length, (char *)page->buf);
    printf("nm_alloc = %d\n", page->nm_alloc);
    ant_buf_free(page);
}
