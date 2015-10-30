#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ho_getopt_long.h"

static void parse_long_opt(const struct ho_option *opt, char *name, char *val, char *argv0) {
    const struct ho_option *p;

    for (p = opt; p->get_val || p->name || p->has_arg ||
            p->flag || p->val
            ; p++) {

        if (!strcmp(p->name, name)) {
            if (p->has_arg == 1 && val == NULL) {
                printf("%s: option `--%s' requires an argument\n", 
                        argv0, name);
                return ;
            }

            if (p->get_val && (p->has_arg == 1 || p->has_arg == 2) && val != NULL)
                p->get_val(p, val);

            if (p->val != 0)
                *(int *)p->flag = p->val;

            return ;
        }
    }
    printf("%s: unrecognized option `--%s'\n", 
            argv0, name);
}

int ho_getopt_long(int argc, char **argv, const struct ho_option *opt) {
    int i;

    for (i = 0; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == '-') {
            parse_long_opt(opt, argv[i] + 2, argv[i+1], argv[0]);
        }
    }
    return 0;
}

#if 0
int main(int argc, char **argv) {
    struct out_opt {
        int max_nthreads;
        int max_nqueues;
        unsigned ip;
        int debug;
    } option = {0};

    struct ho_option longopts[] = {
        {ho_opt_int, "nt", 1, &option.max_nthreads, 0},
        {ho_opt_int, "nq", 1, &option.max_nqueues, 0},
        {ho_opt_ip, "ip", 1, &option.ip, 0},
        {NULL, "debug", 0,    &option.debug, 1},
        {NULL, NULL, 0, NULL, 0}
    };

    ho_getopt_long(argc, argv, longopts);

    printf("max_nthreads = %d:max_nthreads = %d"
            "debug = %d\n",
            option.max_nthreads, option.max_nqueues, option.debug);

    unsigned ip = option.ip;
    printf("ip = %u.%u.%u.%u\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    return 0;
}
#endif
