#include <stdio.h>
#include <unistd.h>

void *ho_memcpy(void *dst, const void *src, size_t n) {
    char *d = dst;
    char *s = (char *)src;

    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

int main() {

    char buf[128] = "";
    write(1, ho_memcpy(buf, "12345", 5), 5);
    printf("\n");
    write(1, ho_memcpy(buf, "12", 2), 2);
    printf("\n");
    return 0;
}
