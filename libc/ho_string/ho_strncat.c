#include <stdio.h>
#include <string.h>

char *ho_strncat(char *s1, char *s2, int n) {
    char *s = s1;

    while (*s1)
        s1++;

    while (n-- && (*s1++ = *s2++) != '\0') {
    }
    *s1 = '\0';
    return s;
}

int main() {

    char buf[128] = "0123";
    buf[0] = 0;
    printf("%s\n", ho_strncat(buf, "aaa", 2));

    strcpy(buf, "0123");
    buf[0] = 0;
    printf("%s\n", strncat(buf, "aaa", 2));
    return 0;
}
