#include <stdio.h>
#include <string.h>

char *ho_strtok_r(char *str, const char *sep, char **lasts) {
    char *b, *e;

    b = str ? str : *lasts;
    b += strspn(b, sep);
    if (*b == '\0') {
        *lasts = NULL;
        return NULL;
    }

    e = b + strcspn(b, sep);
    if (*e != '\0') {
        *e++ = '\0';
    }

    *lasts = e;
    return b;
}

int main() {
    char test[400];
    char *sep = ":/.?&#=";
    char *word, *phrase, *brkt, *brkb;

    strcpy(test, ":/http://fanyi.baidu.com/translate?query=#&keyfrom=baidu&smartresult=dict&lang=auto2zh#auto/zh/");

#if 1
    for (word = ho_strtok_r(test, sep, &brkt);
            word;
            word = ho_strtok_r(NULL, sep, &brkt))
    {
        printf("::wrod = %s\n", word);
    }
#else
    for (word = strtok_r(test, sep, &brkt);
            word;
            word = strtok_r(NULL, sep, &brkt))
    {
        printf("::wrod = %s\n", word);
    }
#endif
    printf("%p\n", brkt);

}
