#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
size_t ho_strlen1 (const char *s) {
    const char *sc;
    for (sc = s; *sc != '\0' ; ++sc)
        ;
    return (sc-s);
}

int main(int argc, char **argv) {
    int fd = -1;
    fd = open(argv[1], O_RDONLY, 0644);
    if (fd == -1) {
        printf("open error:%s\n", strerror(errno));
        return errno;
    }

    struct stat sb;
    if (stat(argv[1], &sb) == -1) {
        printf("stat error:%s\n", strerror(errno));
        return errno;
    }

    char *p = malloc(sb.st_size + 1);
    read(fd, p, sb.st_size);
    ho_strlen1(p);
    //printf("my strlen = %ld:\n", ho_strlen1(p));
    free(p);
    close(fd);
    return 0;
}
