#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/tor_string.h"

char msg[] = "test tor_string.c";

void test(void) {

    char *p;
    int use = 0;
    int size = 0;

    sprintfcat(&p, &use, &size, "12345");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("12345"));
    sprintfcat(&p, &use, &size, "abcdef");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("abcdef"));
    sprintfcat(&p, &use, &size, "6789a");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("6789a"));
    sprintfcat(&p, &use, &size, "6789a");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("6789a"));
    sprintfcat(&p, &use, &size, "6789a");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("6789a"));

    sprintfcat(&p, &use, &size, "6789a");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("6789a"));
    sprintfcat(&p, &use, &size, "6789a");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("6789a"));
    sprintfcat(&p, &use, &size, "6789a");
    printf("%s:%d:%d:%ld\n", p, use, size, strlen("6789a"));
    free(p);

}
