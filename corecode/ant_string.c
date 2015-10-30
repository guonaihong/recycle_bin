#include "ant_string.h"
#include <ctype.h>

int ether_atoe(const char *a, unsigned char *e) {
    char *c = (char *) a;
    int i = 0;

    memset(e, 0, ETHER_ADDR_LEN);
    for (;;) {
        e[i++] = (unsigned char) strtoul(c, &c, 16);
        if (!*c++ || i == ETHER_ADDR_LEN)
            break;
    }
    return (i == ETHER_ADDR_LEN);
}

char *ant_dump_num(char *dst, unsigned int ui, int base) {
    char *d = dst;
    int n = sizeof(unsigned int) * 8;
    static const char hex[] = "0123456789abcdef";

    if (base == 2) {
        while (n--)
            *dst++ = hex[(ui >> n) & 0x1];

    }else if (base == 16) {
        do {
            n -= 4;
            *dst++ = hex[(ui >> n) & 0xf];
        }while (n);

    }
    *dst = '\0';
    return d;
}


int ant_iszero(void *s, size_t n) {
    char *p;
    int cnt;

    p   = (char *)s;
    cnt = n;

    for (; cnt >= 4; cnt -= 4, p += 4) {
        if (*(uint32_t*)p != 0x0)
            return 0;
    }

    for (;cnt > 0; cnt--, p++) {
        if (*p != 0x0)
            return 0;
    }
    return 1;
}

int isallc(const void *s, int c, size_t n) {
    char *p;
    int cnt;
    uint32_t t;

    p = (char *)s;
    cnt = n;

    memset(&t, c, 4);
    for (;cnt >= 4; cnt -= 4, p += 4) {
        if (*(uint32_t*)p != t)
            return 0;
    }

    for (;cnt > 0; cnt--, p++) {
        if (*p != (char)(c & 0xff))
            return 0;
    }

    return 1;
}

char *ant_itoa(int n, char *dst, int base) {
    char     *d, *p;
    char     str[33];
    unsigned m;

    p = str + sizeof(str) - 1;
    d = dst;

    *p = '\0';

    /* n == INT_MIN*/
    if (n == (int)(((unsigned int)~0 >> 1) + 1))
        m = (unsigned)n;
    else if (n < 0 && base == 10)
        m = -n;
    else 
        m = n;

    do {
        *--p = "0123456789abcdef"[m % base];
    }while (m /= base);

    if (n < 0 && base == 10)
        *--p = '-';

    while ((*dst++ = *p++) != '\0')
        ;
    return d;
}

double ant_atod(char *s, char **endptr) {
    char sign;
    double d;
    int n;

    d = 0;
    n = 1;

    /* skip space */
    for (; isspace(*s) && *s; s++);

    sign = *s == '+' || *s == '-' ? *s++ : '+';

    for (; *s && isdigit(*s) && *s != '.'; s++)
        d = d * 10 + *s - '0';

    if ( *s == '.') s++;

    for (; *s && isdigit(*s); s++) {
        d = d * 10 + *s - '0';
        n *= 10;
    }

    if (sign == '-')
        d = -d;

    /* error pos*/
    if (endptr)
        *endptr = s;
    return d / n;
}

int add_buf(char **buf, int *cur_len, int *bufsize, char *str, int len) {
    int needed;
    char *newbuf;

    needed = 0;
    if (*buf == NULL || *bufsize == 0) {
        *bufsize = len + 5;
        *buf = calloc(*bufsize, 1);
        if (*buf == NULL)
            return -1;
    } else {
        needed = *cur_len + len + 1;
        if (needed > *bufsize) {
            if (needed < 2 * *bufsize) {
                needed = 2 * *bufsize;
            }
            newbuf = (char *)realloc(*buf, needed);
            if (newbuf == NULL)
                return -1;

            *buf = newbuf;
            *bufsize = needed;
        }
    }

    memcpy(*buf + *cur_len, str, len);
    *cur_len += len;

    return 0;
}
