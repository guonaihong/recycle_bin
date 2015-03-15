#include <stdio.h>

void *ho_memchr(const void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    for (;n-- > 0; ++p) {
        if (*p == c)
            return (void *)p;
    }
    return NULL;
}

int main() {
    printf("%s\n", ho_memchr("12345", '2', 5));
    printf("%s\n", ho_memchr("12345", '3', 5));
    return 0;
}
