#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *ho_trim(char *s) {
    char *start, *last, *bk;
    int len;

    start = s;
    while (isspace(*start))
        start++;

    bk = last = s + strlen(s) - 1;
    while (last > start && isspace(*last))
        last--;

    if ((s != start) || (bk != last)) {
        len = last - start + 1;
        strncpy(s, start, len);
        s[len] = '\0';
    }
    return s;
}

int main() {

    char buf[128];
    strcpy(buf, "    1234    ");
    printf("$%s:\n", ho_trim(buf));

    strcpy(buf, "1234    ");
    printf("$%s:\n", ho_trim(buf));

    strcpy(buf, "    1234");
    printf("$%s:\n", ho_trim(buf));

    strcpy(buf, "    1234  abcd   ");
    printf("$%s:\n", ho_trim(buf));

    strcpy(buf, "    ");
    printf("$%s:\n", ho_trim(buf));

    strcpy(buf, " ");
    printf("$%s:\n", ho_trim(buf));
    return 0;
}
