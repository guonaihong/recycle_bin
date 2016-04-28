#include <stdio.h>
#include <stdlib.h>

unsigned hash(const char *s, int num_buckets) {
    unsigned hash_val;
    char    *p;

    p        = (char *)s;
    hash_val = 0;

    while (*p) {
        hash_val = hash_val * 33 + *p++;
    }

    return hash_val % num_buckets;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("<Usage:> <%s> <word> <num_buckets>\n", argv[0]);
        return 1;
    }

    printf("%s:%d\n", argv[1], hash(argv[1], atoi(argv[2])));
    return 0;
}
