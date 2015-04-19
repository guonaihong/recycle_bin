#include <stdio.h>
#include <string.h>

int ho_strcmp(const char *s1, const char *s2) {
    char *s = (char *)s1;
    int c;

    while (!(c = *s - *s2) && *s)
        s++, s2++;
    return c;
}

int main() {
    printf("strcmp:%d\n", strcmp("1234", "123"));
    printf("%d\n", ho_strcmp("1234", "123"));
    
    printf("strcmp:%d\n", strcmp("123", "1234"));
    printf("%d\n", ho_strcmp("123", "1234"));

    printf("strcmp:%d\n", strcmp("123", "123"));
    printf("%d\n", ho_strcmp("123", "123"));
    return 0;
}
