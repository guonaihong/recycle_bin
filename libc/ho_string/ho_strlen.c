#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int ho_strlen(const char *str) {
    char *p = (char *)str;
    while (*p++)
        ;
    return p - str - 1;
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
    ho_strlen(p);
    //printf("my strlen = %d:\n", ho_strlen(p));
    //printf("my strlen = %d:\n", ho_strlen(p));
    free(p);
    close(fd);
    return 0;
}
