#include "common.h"

int sprintfcat(char **strp, int *use, int *size, char *fmt, ...) {
    int n, needed;

    char*np;
    va_list ap;

    if (*strp == NULL || *size == 0) {
        *size = 500;
        *use  = 0;
        *strp = calloc(*size, 1);
        if (!*strp)
            return -1;
    }

    while (1) {

        /* Try to print in the allocated space. */

        va_start(ap, fmt);
        n = vsnprintf(*strp + *use, *size - *use, fmt, ap);
        va_end(ap);

        /* If that worked, return 0. */

        if (n > -1 && n < *size - *use) {
            *use += n;
            return 0;
        }

        /* Else try again with more space. */

        
        if (n > -1) { /* glibc 2.1 */
            needed = *use + n + 1; /* precisely what is needed */
            if (needed > *size) {
                if (needed < 2 * *size) {
                    needed = 2 * *size;
                }
                *size = needed;
            }
        } else
            *size *= 2; /* glibc 2.0 */

        /*printf("==========\n");*/
        if ((np = realloc (*strp, *size)) == NULL) {
            free(*strp);
            return -1;
        } else {
            *strp = np;
        }
    }

}

