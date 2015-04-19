#include <string.h>
#include <stdio.h>

char *ho_strpbrk(const char *s1, const char *s2) {
    const char *s;
    for (; *s1; s1++) {
        for (s = s2; *s; s++) {
            if (*s == *s1)
                return (char *)s1;
        }
    }
    return NULL;
}

int main() {

    printf("%s\n", strpbrk("12345678", "8"));
    printf("%s\n", strpbrk("12345678", "1"));
    printf("%s\n", strpbrk("12345678", "654"));
    printf("%s\n", strpbrk("12345678", "#"));

    printf("========\n");
    printf("%s\n", ho_strpbrk("12345678", "8"));
    printf("%s\n", ho_strpbrk("12345678", "1"));
    printf("%s\n", ho_strpbrk("12345678", "654"));
    printf("%s\n", ho_strpbrk("12345678", "#"));
    return 0;
}
