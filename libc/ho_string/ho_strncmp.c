#include <string.h>
#include <stdio.h>

int ho_strncmp(const char *s1, const char *s2, size_t n) {
    char *s = (char *)s1;

    int c;
    while (n-- && !(c = *s - *s2) && *s)
        s++, s2++;

    return c;
}

int main() {

    printf("%d\n", ho_strncmp("https", "http", 5));
    printf("%d\n", strncmp("https", "http", 5));

    printf("%d\n", ho_strncmp("http", "https", 5));
    printf("%d\n", strncmp("http", "https", 5));

    printf("%d\n", ho_strncmp("http", "https", 4));
    printf("%d\n", strncmp("http", "https", 4));
    return 0;
}
