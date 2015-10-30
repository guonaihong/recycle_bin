#ifndef _ANT_URL_H
#define _ANT_URL_H
#include <stdio.h>
extern char * url_to_domain(char *dst, size_t n, char *url);

extern int domain_cmp(const char *d1, const char *d2);

extern int url_modify(char *buf, size_t n, char *url, size_t n2);
#endif
