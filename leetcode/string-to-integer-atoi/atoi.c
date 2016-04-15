#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>

int myatoi(char *str) {
    char     *p    = str;
    int64_t   v    = 0;
    int       sign = '+';

    while(*p && isspace(*p)) {
        p++;
    }

    sign = (*p == '-' || *p == '+') ? *p++ : '+';

    while(*p) {
        if (*p < '0' || *p > '9')
            goto done;

        v = v * 10 +  (*p - '0');

        //printf("str(%s) v(%ld) v >= INT_MAX(%d)\n", str, v, );
        if (sign == '+' && v >= INT_MAX) {
            return INT_MAX;
        }

        if (sign == '-' && v >= -((int64_t)INT_MIN)) {
            return INT_MIN;
        }

        p++;
    }

done:
    return (sign == '-') ? -v : v;
}

int main() {

    int i;

    i = myatoi("-2147483649");
    assert(i == INT_MIN);
    //printf("i = %d\n", i);

    i = myatoi("2147483648");
    assert(i == INT_MAX);

    i = myatoi("-2147483648");
    assert(i == INT_MIN);

    i = myatoi("      -11919730356x");
    assert(i == INT_MIN);

    return 0;
}
