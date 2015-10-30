#ifndef _HO_MEM_H
#define _HO_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef __cplusplus
extern "C" {
#endif

int mem_read(int *muse, int msize, char *m, void *buf, size_t count);

int mem_write(int *muse, int *msize, char **m, void *buf, size_t count);

int file2mem(const char *fname, char **p, int *size);

#ifdef __cplusplus
}
#endif
#endif
