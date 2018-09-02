#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "processpool.h"

#define NPR_WC "ps aux |grep 'auto_add_del$'" \
               "|grep -v grep |wc -l"

static void wc_processnum() {
    FILE *fp       = NULL;
    char  buf[128] = "";
    int   rv;

    fp = popen(NPR_WC, "r");
    if (fp == NULL) {
        return;
    }

    rv = snprintf(buf, sizeof(buf), "current process number:");
    fgets(buf + rv, sizeof(buf) - rv, fp);
    pclose(fp);

    printf("%s\n", buf);
}

static void hello(void *arg) {
    printf("hi.hello:%s\n", arg == NULL ? "" : (char *)arg);
}

int main() {

    pp_pool_t *pool = NULL;
    int        i;

    pool = pp_pool_new(15/* max process */, 1 /* min process*/,
           PP_MIN_CREATE | PP_AUTO_ADD |PP_AUTO_DEL /* flags */, PP_NULL);
    assert(pool != NULL);

    sleep(1);
    wc_processnum();

    for (i = 0; i < 10000; i++) {
        if (pp_pool_add(pool, hello, "aaaa") != 0) {
            printf("add %d fail\n", i);
        }
    }

    wc_processnum();
    pp_pool_wait(pool);

    pp_pool_free(pool);
    return 0;
}
