#include <stdio.h>

char *ho_strcpy(char *dst, const char *src) {
    char *p = dst;
    while ((*dst++ = *src++) != '\0')
        ;
    return p;
}

int main() {

    char buf[128] = "";
    printf("%s\n", ho_strcpy(buf, "12345678"));
    printf("%s\n", ho_strcpy(buf, "aaaaaa"));
    printf("%s\n", ho_strcpy(buf, "bbbbbb"));
    return 0;
}
