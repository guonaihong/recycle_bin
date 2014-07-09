#include "proto.h"
#include "libusc_mrcp.h"
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
    int  use, rv;
    int  len;

    int n = 0;

    p = NULL;

    uint32_t narg1, narg2, narg3;
    uint32_t offset;
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

        printf(" offset = %d\n", offset);
        if (!CMP(p + 4, "usc_mrcp_asr_init")) {
            unsigned int arg = 0;
            POP_BUF(p, offset, n, argbuf, narg1);
            POP_INT(p, offset, n, arg, sizeof(unsigned int));

            printf("%s:%ld\n", argbuf, arg);

            free(argbuf);
            argbuf = NULL;
        } else if (!CMP(p + 4, "usc_mrcp_asr_destroy")) {
            printf("usc_mrcp_asr_destroy");
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
    
    printf("Bye Bye\n");
}
