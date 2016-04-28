#include <stdio.h>
#include <stdint.h>

unsigned hash(const char *s) {
    unsigned hash_val;
    char    *p;

    p        = (char *)s;
    hash_val = 0;

    while (*p) {
        hash_val = hash_val * 31 + *p++;
    }

    return hash_val;
}

int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets) {
    int64_t b = -1, j = 0;
    while (j < num_buckets) {
        b = j;
        key = key * 2862933555777941757ULL + 1;
        j = (b + 1) * ((double)(1LL << 31) / (double)((key >> 33) + 1));
    }
    return b;
}

int32_t tst_consistent_hash(const char *s, int32_t num_buckets) {
    return JumpConsistentHash(hash(s), num_buckets);
}

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("<Usage:> <%s> <word> <num_buckets>\n", argv[0]);
        return 1;
    }

    printf("%s:%d\n", argv[1], tst_consistent_hash(argv[1], atoi(argv[2])));
    return 0;
}
