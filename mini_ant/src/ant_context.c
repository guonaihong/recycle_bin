#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/stat.h>
#include <math.h>
#include "ant_context.h"

void ant_total_context_init(total_context_t *total) {
    int i, len;

    total->cache_url = ant_hash_new(4096, 0, NULL, NULL);
    total->cache_dns = ant_hash_new(4096, 0, NULL, NULL);
    total->queue_dns = queue_new();
    total->down      = queue_new();
    for (i = 0, len = ARRAY_LEN(total->mutex); i < len; i++)
        pthread_mutex_init(&total->mutex[i], NULL);
}

void cache_url_init(total_context_t *total, char *fname) {
    int    fd;
    int    size;
    struct stat sb;
    char   *p, *begin, *end;

    fd = open(fname, O_RDWR);
    if (fd < 0) {
        perror("");
        return ;
    }
    fstat(fd, &sb);
    size = (int)ceil((double)sb.st_size / sysconf(_SC_PAGE_SIZE)) * sysconf(_SC_PAGE_SIZE);

    p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (p == (void *)-1) {
        /*perror("mmap");*/
        goto fail;
    }

    begin = end = p;

    for (; *end; end++) {
        if (*end != '\n')
            continue;

        *end = '\0';

        if (!ant_lookup(total->cache_url, begin, NULL)) {
            ant_hash_put(total->cache_url, strdup(begin), NULL);
        }

        begin = ++end;

    }

fail:
    /*printf("%d\n", total->cache_url->count);*/
    close(fd);
    munmap(p, size);
}

static ant_hash_t *uniq_hash;

void uniq_init() {
    uniq_hash = ant_hash_new(4096, 0, NULL, NULL);
}

/* 1 exists
 * 0 does not exists */
int uniq_exists(char *url) {
    return ant_lookup(uniq_hash, url, NULL) ? 1: 0;
}

void uniq_put(char *url) {
    ant_hash_put(uniq_hash, url, NULL);
}
