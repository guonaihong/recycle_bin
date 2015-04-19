#include <stdio.h>
#include <string.h>

int ho_strcspn(const char *s1, const char *s2) {
    int n;
    const char *s;
    for ( n = 0; *s1; s1++, n++) {
        for (s = s2; *s && (*s != *s1); s++) {
        }

        if ((*s != '\0'))
            return n;
    }
    return n;
}

int main() {

    char *p = "world\t\r\n\v";
    printf(":%s:\n", p + ho_strcspn(p, " \t\r\n"));
    return 0;
}
