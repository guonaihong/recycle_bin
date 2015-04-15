#include <stdio.h>
#include <unistd.h>

void *ho_memset(void *b, int c, size_t n) {
    char *p = (char *)b;
    while (n--) {
        *p++ = c;
    }
    return b;
}

int main() {
    char buf[] = "12345";
    printf("%s\n", buf);
    write(1, ho_memset(buf, 0, 5), 5);
    printf("::\n");
    return 0;
}
