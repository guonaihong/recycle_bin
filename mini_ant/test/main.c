#include <stdio.h>

extern char msg[];
extern void test(void);

int main() {
    printf("%s\n", msg);
    test();
    return 0;
}
