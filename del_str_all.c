#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * del_str_all(char *str, const char *del) {
    int len;
    char *pos;

    len = strlen(del);
    while ((pos = strstr(str, del)) != NULL) {
        strcpy(pos, pos + len);
    }

    return str;
}

int main(int argc, char **argv) {

    char buf[] = "aabbaaccbbddeeffgghhiijjkkllmmnn";

    if (argc != 2) {
        fprintf(stderr, "%s <string>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("old->%s\n", buf);
    printf("new->%s\n", del_str_all(buf, argv[1]));
    return 0;
}
