#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ssize_t ho_getline(char **buf, size_t *n, FILE *fp) {
    char c;
    int needed = 0;
    int maxlen = *n;
    char *buf_ptr = *buf;

    if (buf_ptr == NULL || maxlen == 0) {
        maxlen = 128;
        if ((buf_ptr = malloc(maxlen)) == NULL)
            return -1;
    }

    do {
        c = fgetc(fp);
        buf_ptr[needed++] = c;

        if (needed >= maxlen) {

            *buf = buf_ptr;
            buf_ptr = realloc(buf_ptr, maxlen *= 2);

            if (buf_ptr == NULL) {
                (*buf)[needed - 1] = '\0';
                return -1;
            }
        }

        if (c == EOF)
            return -1;

    } while (c != '\n');

    buf_ptr[needed] = '\0';
    *buf = buf_ptr;
    *n = maxlen;
    return needed;
}


