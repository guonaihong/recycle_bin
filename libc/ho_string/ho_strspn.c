#include <stdio.h>
#include <string.h>

int ho_strspn(const char *s1, const char *s2) {
    int n = 0;
    const char *s;
    for (; *s1; s1++, n++) {
        for (s = s2; *s && (*s1 != *s); s++) {
        }
        if (*s == '\0')
            return n;
    }

    return n;
}

int main() {

    char *p = " \t\r\nhello";

    printf("%s\n", p + ho_strspn(p, " \t\r\n"));
    return 0;
}
