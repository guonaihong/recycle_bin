#ifndef _HO_READ_CONF_H
#define _HO_READ_CONF_H

/*Copyright (C) guonaihong*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ho_rc_t {
    void (*cb_get_val)(struct ho_rc_t *rc, char *in_val);
    const char *key;
    void *out_val;
    int  len;
    unsigned char not_empty;
};

int ho_read_conf(const char *fname, struct ho_rc_t *rc);

static inline void ho_get_int(struct ho_rc_t *rc, char *val) {
    *(int *)(rc->out_val) = atoi(val);
}

static inline void ho_get_double(struct ho_rc_t *rc, char *val) {
    *(double *)rc->out_val = atof(val);
}

static inline void ho_get_str_alloc(struct ho_rc_t *rc, char *val) {
    *(char **)rc->out_val = strdup(val);
}

static inline void ho_get_str(struct ho_rc_t *rc, char *val) {
    int n = rc->len;
    strncpy((char *)rc->out_val, val, n);
    if (n > 0)
        ((char *)rc->out_val)[n - 1]= '\0';
}

static inline void ho_get_ip(struct ho_rc_t *rc, char *val) {
    struct in_addr addr;
    if (inet_aton(val, &addr) == 0) {
        perror("inet_aton");
    }
    *(int *)rc->out_val = addr.s_addr;
}

static inline void ho_get_port(struct ho_rc_t *rc, char *val) {
    *(unsigned short *)rc->out_val = htons(atoi(val));
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>
static inline void ho_get_str_cxx(struct ho_rc_t *rc, char *val) {
    ((std::string *)rc->out_val)->clear();
    ((std::string *)rc->out_val)->append(val);
}
#endif
#endif

