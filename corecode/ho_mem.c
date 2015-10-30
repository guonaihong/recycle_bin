#include "ho_mem.h"

#define HO_MIN(x, y) ((x) > (y) ? (y) : (x))
int mem_read(int *muse, int msize, char *m, void *buf, size_t count) {
    int cur_len;

    if (msize == 0)
        return -1;

    if (*muse >= msize)
        return -1;

    cur_len = HO_MIN(msize - *muse, count);

    memcpy(buf, m + *muse, cur_len);

    *muse += cur_len;
    return cur_len;
}

int mem_write(int *muse, int *msize, char **m, void *buf, size_t count) {
    int needed;
    char *newbuf;
    needed = 0;
    if (*m == NULL || *msize == 0) {
        *msize = count + 500;
        *m = calloc(*msize, 1);
        if (*m == NULL)
            return -1;
    } else {
        needed = *muse + count + 1;
        if (needed > *msize) {
            if (needed < 2 * *msize) {
                needed = 2 * *msize;
            }
            newbuf = (char *)realloc(*m, needed);
            if (newbuf == NULL)
                return -1;
            *m = newbuf;
            *msize = needed;
        }
    }
    memcpy(*m + *muse, buf, count);
    *muse += count;
    return 0;
}

int file2mem(const char *fname, char **p, int *size) {
    char *mem = NULL;
    struct stat sb; 
    int fd = -1; 

    if(stat(fname, &sb) == -1) {
        perror("stat");
        return -1; 
    }   

    mem = malloc(sb.st_size);
    if (!mem) {
        perror("malloc");
        return -1; 
    }   

    fd = open(fname, O_RDONLY, 0644);
    if (fd == -1) {
        perror("open");
        free(mem);
        return -1; 
    }   

    read(fd, mem, sb.st_size);
    *p = mem;
    *size = sb.st_size;
    close(fd);
    return 0;
}

#if 0
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "<Usage> <%s> <filename>\n" ,argv[0]);
        return 1;
    }

    char *p = NULL;
    int size = 0;
    file2mem(argv[1], &p, &size);

    int in_use = 0;
    int in_size = size;
    char buf[3] = "";
    int rv = 0;

    int out_use = 0;
    int out_size = 0;
    char *out = NULL;
    while ((rv = mem_read(&in_use, in_size, p, buf, 2)) != -1) {
        //printf("rv = %d\n", rv);
        //write(1, buf, strlen(buf));
        mem_write(&out_use, &out_size, &out, buf, rv);
    }

    write(1, p, out_use);

    return 0;
}
#endif
