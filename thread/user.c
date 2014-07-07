#include "proto.h"
static write_err(int s, char *msg) {
    ;
}

static void debug_print(FILE *fp, ...) {
    //#ifdef DEUBG
    ;
}

#define CMP(p, s) strncmp(p, s, sizeof(s) - 1)
#define SKIP_FUN(p) (4 + ntohl(*(uint32_t *)(p)))

#if 0
typedef struct buf_s {
    char *str;
    int  use;
    int   n;
} buf_t;
#endif

void process_msg(int s) {
    char *p;
    int  use, n, rv;
    int  len;

    p = NULL;
    n = 0;

    uint32_t narg1, narg2, narg3;
    int offset;
    char *argbuf = NULL;

    for (;;) {

        /*
         * function name
         * arg1
         * arg2
         * arg3
         * ....
         * data type(function name,arg1, arg2, arg3 ...)
         * |--data len---|--data---|
         */
        rv = readvrec(s, &p, &n);
        if (rv == 0) {
            goto failed;
        }
        printf(" readvrec rv = %d---\n", rv);

        offset = SKIP_FUN(p);
        if (!CMP(p + 4, "usc_mrcp_asr_init")) {
            POP_BUF(p, offset, n, argbuf, narg1);

#if 1
            printf(":::::::::$%s$narg1 = %d\n", argbuf, narg1);
            write(1, argbuf, narg1);
#endif

            free(argbuf);
            argbuf = NULL;
        } else if (!CMP(p + 4, "usc_mrcp_asr_destroy")) {
            ;
        } else if (!CMP(p + 4, "usc_mrcp_service_init")) {
            ;
        } else if (!CMP(p + 4, "usc_mrcp_service_start")) {
            ;
        } else if (!CMP(p + 4, "usc_mrcp_service_recognize")) {
            ;
        } else if (!CMP(p + 4, "usc_mrcp_service_get_recg_result")) {
            ;
        } else if (!CMP(p + 4, "usc_mrcp_service_release")) {
            ;
        } else if (!CMP(p + 4, "usc_mrcp_service_set_option")) {
            ;
        }
    }

failed:
    free(p);
    free(argbuf);
    close(s);
}
