#include "ant_page.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

static int filter_type_url(char *str, size_t n) {
    static char *type[] = {
        ".css",
        ".ico",
        ".xml",
        ".gif",
        ".jpg",
        ".png",
        ".exe",
        ".chm",
        NULL
    };

    if (strncasecmp(str, "http://", 7)
        || (strcasecmp(str, "http://") == 0)){
        return -1;
    }

    char **p = type;
    if (n >= 3) {
        if (!strncasecmp(str + n - 3, ".js", 3))
            return -1;
    }
    if (n >= 4) {
        for (;*p; p++) {
            if (!strncasecmp(str + n - 4, *p, 4))
                return -1;
        }
    } 

    return 0;
}

void parse_page(char *mem, void *total, void (*cb_func)(char *url, void *)) {
    char *p, *fmt;
    int  len, n;
    char buf[4096];

    p   = mem;
    fmt = "%*[\" \t\r\n=']%4096[^ \t'\"<>)\r]";
    len = -1;
    for (; *p; p++) {

        if(!strncasecmp(p, "href", 4)) {
            len = 4;
        }

        if (len != -1) {
            p += len;
            len = -1;

            /* %*[ \"'] --> skip space and " and ' */
            if (sscanf(p, fmt, buf) == 1) {

                n = strlen(buf);

                if (filter_type_url(buf, n) == 0) {
                    filter_type_url(buf, n);
                    cb_func(buf,total);

                    p += n;
                }
            }
        }

    }
}

int extract_fromcode(char *from_code, const char *src) {
    char *p = (char *)src;

    char *fmt = "%*[=\" ]%36[^\" ]";

    for (; *p; p++) {

        if (!strncasecmp(p, "charset", 7)) {
            sscanf(p + 7, fmt, from_code);

            return 0;
        }
        if (!strncasecmp(p, "</head>", 7))
            return -1;
    }

    return -1;
}
#ifdef _ANT_TEST
int main(int argc, char **argv) {
    char *p = "<!doctype html> \n"
            "<html>\n"
            "<head>\n"
            "<meta charset=\"gb2312\">\n";
    char buf[128] = "";
    extract_fromcode(buf, 128, p);
    printf("%s\n", buf);
    return 0;
}
#endif
