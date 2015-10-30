#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../../src/ant_page.h"

void print(char *url, void *arg) {
    fprintf(stderr, "%s\n", (char *)url);
}

int main(int argc, char **argv) {
    int  fd;
    char *mem;
    int  size;
    struct stat sb;

    if (argc != 2) {
        printf("<a.out> <html page>\n");
        return 0;
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    fstat(fd, &sb);
    size = ceil((double)sb.st_size / sysconf(_SC_PAGE_SIZE)) * sysconf(_SC_PAGE_SIZE);
    mem = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mem == (void *)-1) {
        goto fail;
    }

    parse_page(mem, NULL, print);
    /*write(1, mem, sb.st_size);*/
fail:
    close(fd);
    munmap(mem, size);

    return 0;
}
