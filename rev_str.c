#include <stdio.h>

void rev_str(char *str) {
    char *last;
    char tmp;

    for (last = str; *last; last++);
    last--;

    while (str < last) {
        tmp = *last;
        *last-- = *str;
        *str++ = tmp;
    }
}

int main(int argc, char **argv) {

    while (*++argv) {
        rev_str(*argv);
        printf("%s\n", *argv);
    }
    return 0;
}
