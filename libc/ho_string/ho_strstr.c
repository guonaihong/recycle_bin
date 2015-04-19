#include <stdio.h>

char *ho_strstr(const char *s1, const char *s2) {
    const char *p1, *p2;

    if (*s2 == '\0')
        return (char *)s1;
    for (; *s1; s1++) {
        p1 = s1;
        p2 = s2;
        for (;*p1 && (*p1 == *p2); p1++, p2++) {
            ;
        }
        if (*p1 == '\0')
            return (char *)s1;
    }
    return NULL;
}

int main() {

    printf("%s\n", ho_strstr("1234678abc", "abc"));
    printf("%s\n", ho_strstr("1234678abc", "sss"));
    printf("%s\n", ho_strstr("1234678abc", "bc"));
    printf("%s\n", ho_strstr("1234678abc", ""));
    return 0;
}
