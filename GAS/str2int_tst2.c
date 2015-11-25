#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
extern int str2int(const char *str, int base);

int main() {
    printf("35:%d\n", str2int("z", 35));
    return 0;
}
