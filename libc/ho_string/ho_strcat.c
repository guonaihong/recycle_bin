#include <stdio.h>

char *ho_strcat(char *s1, const char *s2) {
    char *p = s1;

    for (; *p; p++)
        ;
    while ((*p++ = *s2++) != '\0')
        ;
    return s1;
}

int main() {
    char buf[128] = "";
    printf(":%s:\n", ho_strcat(buf, "1234"));
    printf(":%s:\n", ho_strcat(buf, "5678"));
    printf(":%s:\n", ho_strcat(buf, "9abc"));
    return 0;
}
