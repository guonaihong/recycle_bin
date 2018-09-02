#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/wait.h>
#include <sys/socket.h>

#include "processpool.h"

#define PARENT 0
#define CHILD  1

/*
 * @module util
 */

/*
 * @param fd     socket fd
 * @param flags  man fcntl the second parameter
 *
 * @return a value on error -1, on success 0
 */
static int set_fl(int fd, int flags) {
    int val;
    val = fcntl(fd, F_GETFL, 0);
    if (val == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    val |= flags;
    if (fcntl(fd, F_SETFL, val) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}

/*
 * @param fd    socket fd
 * @param msec  millisecond
 *
 * @return value timedout 0, error -1, on success other number
 */
static int read_wait(int fd, int msec) {
    struct timeval tv;
    fd_set         rfds;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    tv.tv_sec  = 0;
    tv.tv_usec = msec;
    return select(fd + 1, &rfds, NULL, NULL, (msec != -1) ? &tv : NULL);
}

/*
 * @param fd    socket fd
 * @param buf   memory addr
 * @param len   buf length
 * @param msec  millisecond
 *
 * @param return a value, on error, -1 if timeout error == ETIMEDOUT
 * On success, the number of bytes read is returned (zero indicates end of file)
 */
static int readn_timedwait(int fd, void *buf, size_t len, int msec) {
    int   cnt, rv;
    char *bp;

    cnt = len;
    bp  = buf;

    while (cnt > 0) {
        rv = read(fd, bp, cnt);
        if (rv < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if ((rv = read_wait(fd, msec)) < 0) {
                    printf("%s select error:%s:%d\n",
                            __func__, strerror(errno), fd);
                    return -1;
                }

                if (rv == 0) {
                    printf("%s select timeout:%d\n", __func__, fd);
                    errno = ETIMEDOUT;
                    return -1;
                }
                continue;
            }
            return -1;
        }
        else if (rv == 0)
            return len - cnt;
        cnt -= rv;
        bp += rv;
    }
    return len;
}

/*
 * @param fd    socket fd
 * @param msec  millisecond
 *
 * @return value timedout 0, error -1, on success other number
 */
static int write_wait(int fd, int msec) {
    struct timeval tv;
    fd_set         wfds;

    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    tv.tv_sec  = 0;
    tv.tv_usec = msec;
    return select(fd + 1, NULL, &wfds, NULL, (msec != -1) ? &tv : NULL);
}

/*
 * @param fd    socket fd
 * @param buf   memory addr
 * @param len   buf length
 * @param msec  millisecond
 *
 * @param return a value, on error, -1 if timeout error == ETIMEDOUT
 * On success, the number of bytes written is returned (zero indicates nothing was written)
 */
static int writen_timedwait(int fd, void *buf, size_t len, int msec) {
    int   cnt, rv;
    char *bp;

    cnt = len;
    bp  = buf;

    while (cnt > 0) {
        rv = write(fd, bp, cnt);
        if (rv < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if ((rv = write_wait(fd, msec)) < 0) {
                    printf("%s select error:%s:%d\n",
                            __func__, strerror(errno), fd);
                    return -1;
                }

                if (rv == 0) {
                    printf("%s select timeout:%d\n", __func__, fd);
                    errno = ETIMEDOUT;
                    return -1;
                }
                continue;
            }
            return -1;
        }
        else if (rv == 0)
            return len - cnt;
        cnt -= rv;
        bp += rv;
    }
    return len;
}

/*
 * @module process pool
 */
struct pp_pool_t {
    int fd[2];
    int max_process;
    int min_process;
    int run_process;
    int flags;
    int ms;
};

static int pp_pool_need_nprocess_gen(pp_pool_t *pool, int *nprocess) {
    if (pool->max_process < pool->min_process) {
        return -1;
    }

    if (pool->min_process < 0) {
        return -1;
    }

    if ((pool->flags & PP_MIN_CREATE) &&
            (pool->flags & PP_MAX_CREATE)) {
        return -1;
    }

    /* pool->flags & PP_MAX_CREATE */
    *nprocess = pool->max_process;
    if (pool->flags & PP_MIN_CREATE) {
        *nprocess = pool->min_process;
    }

    if (*nprocess < 0) {
        return -1;
    }

    return 0;
}

static void pp_pool_args_set(pp_pool_t *pool, va_list ap) {
    int i, val;

    for (i = 0; (val = va_arg(ap, int)) != PP_NULL; i++) {
        if (i == 0) {
            pool->min_process = val;
        } else if (i == 1) {
            pool->flags = val;
        } else if (i == 2) {
            pool->ms = val;
        }
    }
}

/*
 * @param np            the number of process
 * @param min_processa  (opt)
 * @param flags         (opt)
 * @param ms            (opt)
 *
 * @return a value fail NULL
 */
#define PP_DEFAULT 500
pp_pool_t *pp_pool_new(int np, ...) {
    va_list    ap;
    int        nprocess;
    pp_pool_t *pool = NULL;

    if (np <= 0) {
        return NULL;
    }

    pool              = malloc(sizeof(*pool));
    pool->max_process = np;
    pool->ms          = 500;

    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, pool->fd) == -1) {
        goto fail;
    }

    if (set_fl(pool->fd[PARENT], O_NONBLOCK) == -1) {
        goto fail1;
    }

    if (set_fl(pool->fd[CHILD],  O_NONBLOCK) == -1) {
        goto fail1;
    }

    va_start(ap, np);
    pp_pool_args_set(pool, ap);
    va_end(ap);

    if (pp_pool_need_nprocess_gen(pool, &nprocess) == -1) {
        goto fail1;
    }

    if (pp_pool_process_addn(pool, nprocess) == -1) {
        goto fail2;
    }

    return pool;

fail2:
    exit(1);
fail1:
    close(pool->fd[CHILD]);
    close(pool->fd[PARENT]);
fail:
    free(pool);
    return NULL;
}

typedef struct process_arg_t process_arg_t;
struct process_arg_t {
    int                type;
    void (*func)(void *arg);
    void              *arg;
};

/*
 * @param pool  process pool
 *
 * @return the child does not return directly to end
 */
static int pp_pool_process_loop(pp_pool_t *pool) {

    process_arg_t arg;
    int           rv;

    printf("loop start\n");
    for (;;) {
        rv = readn_timedwait(pool->fd[CHILD], &arg, sizeof(process_arg_t), pool->ms * 2);
        if (rv <= 0) {
            if (errno == ETIMEDOUT) {
                printf("%u bye bye\n", getpid());
            } else {
                printf("reading data from the parent process:%s\n",
                        strerror(errno));
            }
            pp_pool_free(pool);
            exit(0);
        }

        arg.func(arg.arg);
    }

    return 0;
}

/*
 * @param pool  process pool
 *
 * @return child process pid or -1
 */
static int pp_pool_process_add_core(pp_pool_t *pool) {
    pid_t pid;

    if (pool->run_process > pool->max_process) {
        return 0;
    }

    switch(pid = fork()) {
        case 0:/* child */
            pp_pool_process_loop(pool);
            break;
        case -1:/* error */
            return -1;
        default:/* parent */
            pool->run_process++;
            break;
    }

    return pid;
}

/*
 * @param pool   process pool
 * @param func   pointer to the function that will perform the task.
 *
 * @return value -1 error, 0 success, 1 timedout
 */
int pp_pool_add(pp_pool_t *pool, void (*func)(void *), void *arg) {
    process_arg_t a;
    int           ntry, rv;

    a.type = 0;
    a.func = func;
    a.arg  = arg;

    for (ntry = 2; ntry > 0; ntry--) {
        rv = writen_timedwait(pool->fd[PARENT], &a, sizeof(a), pool->ms);
        if (rv == -1) {
            if (errno != ETIMEDOUT) {
                return PP_ERROR;
            }

            if (pool->flags & PP_AUTO_ADD) {
                if (pp_pool_process_addn(pool, 1) == PP_ERROR) {
                    return PP_ERROR;
                }
                continue;
            }
            return PP_TIMEOUT;
        }
    }
    return 0;
}

/*
 * @param pool  process pool address
 * @param n     the number of process
 *
 * @return a value on success pid, or on error -1
 */
int pp_pool_process_addn(pp_pool_t *pool, int n) {
    int i, pid;
    for (i = 0; i < n; i++) {
        pid = pp_pool_process_add_core(pool);
        if (pid == -1) {
            printf("create process:%s\n", strerror(errno));
            return pid;
        }
    }
    return 0;
}

int pp_pool_process_deln(pp_pool_t *pool, int n) {
    return 0;
}

/*
 * @param pool process pool address
 *
 * @return a value 0
 */
int pp_pool_wait(pp_pool_t *pool) {
    int status;

    for (;;) {
        if (wait(&status) == -1) {
            printf("wait:%s\n", strerror(errno));
            break;
        }
    }
    return 0;
}

/*
 * @param pool process pool address
 *
 * @return a value 0
 */
int pp_pool_free(pp_pool_t *pool) {
    close(pool->fd[PARENT]);
    close(pool->fd[CHILD]);
    free(pool);
    return 0;
}

