#include <stdio.h>

char *ho_strchr(const char *s, int c) {
    for (;*s; ++s) {
        if (*s == c)
            return (char *)s;
    }
    return NULL;
}

int main() {
    printf("%s", ho_strchr("12345", '2'));
    return 0;
}
