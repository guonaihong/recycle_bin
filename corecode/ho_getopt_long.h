#ifndef _HO_GETOPT_LONG_H
#define _HO_GETOPT_LONG_H

#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __cplusplus
extern "C" {
#endif

struct ho_option {
    void (*get_val)(const struct ho_option *opt, const char *in_val);
    char *name;
    int has_arg;
    void *flag;
    int val;
};

static inline void ho_opt_int(const struct ho_option *opt, const char *in_val) {
    *(int *)opt->flag = atoi(in_val);
}

static inline void ho_opt_double(const struct ho_option *opt, const char *in_val) {
    *(double *)opt->flag = atof(in_val);
}

static inline void ho_opt_ip(const struct ho_option *opt, const char *in_val) {
    struct in_addr addr;
    if (inet_aton(in_val, &addr) == 0) {
        perror("inet_aton");
    }
    *(int *)opt->flag = addr.s_addr;
}

static inline void ho_opt_port(const struct ho_option *opt, const char *in_val) {
    *(unsigned short *)opt->flag = htons(atoi(in_val));
}

#ifdef __cplusplus
}
#endif
#endif
