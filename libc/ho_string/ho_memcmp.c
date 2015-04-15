#include <stdio.h>
#include <string.h>

int ho_memcmp(const void *s1, const void *s2, size_t n) {
    unsigned char *p1, *p2;
    p1 = (unsigned char *)s1;
    p2 = (unsigned char *)s2;

    char c;
    while (!(c = *p1 - *p2) && n--) {
        p1++;
        p2++;
    }

    return c;
}

int main() {
    printf(":::%d\n", ho_memcmp("1", "12", 2));
    printf(":::%d\n", memcmp("1", "12", 2));
    
    printf(":::%d\n", ho_memcmp("12", "1", 2));
    printf(":::%d\n", memcmp("12", "1", 2));

    printf(":::%d\n", ho_memcmp("12", "12", 2));
    printf(":::%d\n", memcmp("12", "12", 2));
    return 0;
}
