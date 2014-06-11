#include <stdio.h>

void test_main(const char *fname) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(fname, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = ho_getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);
    }

    fclose(fp);
    free(line);
}

int main(void) {

    test_main("/etc/motd");
    return 0;
}
