#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>

#include "processpool.h"

#if 0
sem_t *sem;
#endif
static void hello(void *arg) {
    /*sem_wait(sem);*/
    printf("hi.hello:%s\n", arg == NULL ? "" : (char *)arg);
    /*sem_post(sem);*/
}

static void world(void *arg) {
    printf("hi.world:%s\n", arg == NULL ? "" : (char *)arg);
}

int main() {

    pp_pool_t *pool = NULL;
    int        i;

#if 0
    sem_unlink("lock");
    sem = sem_open("lock", O_CREAT | O_EXCL, 0644, 1);
    if (sem == NULL) {
        printf("sem_open:%s\n", strerror(errno));
        return 1;
    }
#endif

    pool = pp_pool_new(5, PP_NULL);
    printf("process pool create ok\n");

    for (i = 0; i < 1000; i++) {
        if (pp_pool_add(pool, hello, "aaaa") != 0) {
            printf("add %d fail\n", i);
        }

        if (pp_pool_add(pool, world, "bbbb") != 0) {
            printf("add %d fail\n", i);
        }
        printf("%d\n", i);
    }

    pp_pool_wait(pool);

    pp_pool_free(pool);
    return 0;
}
