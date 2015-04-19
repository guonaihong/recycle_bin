#include <stdio.h>
#include <string.h>

char *ho_strrchr(const char *s, int c) {
    const char *p;
    unsigned char cc = c;

    for (p = s; *p; p++);

    while (s <= p) {
        if (*p == cc)
            return (char *)p;
        p--;
    }
    return NULL;
}

char *ho_strrchr2(const char *s, int c) {
    const char *p;

    p = NULL;

    for (; ;s++) {
        if (*s == '\0')
            return (char *)p;

        if (*s == c)
            p = s;
    }
}

int main() {
    printf("%s\n", ho_strrchr("123456789", '1'));
    printf("%s\n", ho_strrchr("123456789", '2'));
    printf("%s\n", ho_strrchr("123456789", '3'));
    printf("%s\n", ho_strrchr("123456789", '9'));
    printf("%s\n", ho_strrchr("123456789", 'a'));
    printf("%s\n", ho_strrchr("123123", '1'));

    printf("===============\n");
    printf("%s\n", ho_strrchr2("123456789", '1'));
    printf("%s\n", ho_strrchr2("123456789", '2'));
    printf("%s\n", ho_strrchr2("123456789", '3'));
    printf("%s\n", ho_strrchr2("123456789", '9'));
    printf("%s\n", ho_strrchr2("123456789", 'a'));
    printf("%s\n", ho_strrchr2("123123", '1'));
    return 0;
}
