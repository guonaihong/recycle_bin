#include "ant_url.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char * url_to_domain(char *dst, size_t n, char *url) {
    char *end, *start;

    start = url;
    n--;

    /*skip spaces*/
    while (isspace(*start)) start++;

    /*skip http:// or https:// */
    end = start;
    if(!strncasecmp(start, "http://", 7) 
            || !strncasecmp(start, "https://", 8)) {
        start += (start[4] == 's' ? 8 : 7);
        end = start;
    }

    /* if *end is '\0' '/' ':' '?' '#' ';' stop */
    while (*end && !strchr("/:?#;", *end)) end++;

    if (end - start < n)
        n = end - start;

    strncpy(dst, start, n);
    if (n > 0)
        dst[n] = '\0';
    return dst;
}

int url_modify(char *buf, size_t n, char *url, size_t n2) {
    int len = 0;

    if (!strncasecmp(url, "http://", 7) ||
            !strncasecmp(url, "https://", 8)) {
        len = url[4] == 's' ? 8 : 7;
    }

    /*
     * url is www.baidu.com
     * modified to http://www.baidu.com
     */
    if (len == 0) {
        snprintf(buf, n, "http://%s",url);

        return 0;
    }

    return -1;
}

int domain_cmp(const char *d1, const char *d2) {
    char c;
    char *p = (char *)d1;
    char *p2= (char *)d2;

    /*skip spaces*/
    while (isspace(*p)) p++;
    while (isspace(*p2)) p2++;

    /*skip http:// or https:// */
    if (!strncmp(p, "http", 4))
        while(*p && 
            (p[-1] != '/' || p[-2] != '/' || p[-3] != ':')) 
            p++;

    if (!strncmp(p2, "http", 4))
        while(*p2 &&
            (p2[-1] != '/' || p2[-2] != '/' || p2[-3] != ':'))
            p2++;

    /* cmp */
    for(;;) {
        if ((c = (*p | 0x20) - (*p2 | 0x20)) && 
                !(*p == ':' || *p == '/' || *p2 == ':' || *p2 == '/'))
            break;

        if (*p == '\0' ||
                (*p == ':' || *p == '/' || *p2 == ':' || *p2 == '/')) {
            return 0;
        }
        p++;
        p2++;
    }

    return c < 0 ? -1 : 1;
}

#if defined _ANT_URL_TO_DOMAIN || defined _ANT_URL_MODIFY
#define _ANT_TEST
#endif

#ifdef _ANT_TEST
/* gcc ant_url.c -D_ANT_TEST*/
int main(int argc, char **argv) {
#if _ANT_DOMAIN_CMP
    fprintf(stderr, "%d\n", domain_cmp("1.com", ""));
    fprintf(stderr, "%d\n", domain_cmp("","3.com"));
    fprintf(stderr, "%d\n", domain_cmp("q.com:80","q.com"));
    fprintf(stderr, "%d\n", domain_cmp("q.com","q.com:80"));
    fprintf(stderr, "%d\n", domain_cmp("Q.com", "q.com/1.html"));
    fprintf(stderr, "%d\n", domain_cmp("q.com", "q.com/1.html"));
    fprintf(stderr, "%d\n", domain_cmp("q.com/1.html", "q.com"));
    fprintf(stderr, "%d\n", domain_cmp("q.com/", "q.com/1.html"));
    fprintf(stderr, "%d\n", domain_cmp("q.com", "q.com"));
#endif

#if _ANT_URL_TO_DOMAIN
    char domain[128] = "";
    while (*++argv) {
        fprintf(stderr, "%s\n", url_to_domain(domain, sizeof(domain), *argv));
    }
#endif

#if _ANT_URL_MODIFY
    char url[4096] = "";
    while (*++argv) {
        fprintf(stderr, "%d\n", url_modify(url, sizeof(url), *argv, strlen(*argv)));
        fprintf(stderr, "#%s#%s\n", url, *argv);
    }
#endif
    return 0;
}
#endif
