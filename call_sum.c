#include <stdio.h>
#include <stdint.h>

int64_t sum(int64_t arr[], int64_t n);
int main() {

    int64_t test[] = {1,2,3,4,5,6,7,8,9,10};

    printf("%ld\n", sum(test, 1));
    printf("%ld\n", sum(test, 2));
    printf("%ld\n", sum(test, 3));
    printf("%ld\n", sum(test, 4));
    printf("%ld\n", sum(test, 5));
    printf("%ld\n", sum(test, -1));
    return 0;
}
