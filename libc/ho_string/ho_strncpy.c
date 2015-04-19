#include <stdio.h>
#include <string.h>

char *ho_strncpy(char *s1, const char *s2, size_t n) {
    char *s = s1;
    for (;n && (*s1 = *s2) != '\0'; n--) {
        s1++, s2++;
    }

    while (n--) {
        *s1++ = '\0';
    }
    return s;
}

int main() {
    char buf[128] = "12345";
    ho_strncpy(buf, "aaaa", 3);
    printf("%s\n", buf);
    return 0;
}
