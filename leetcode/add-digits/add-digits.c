#include <stdio.h>

int add_digits(int num) {
    if (num == 0)
        return 0;
    int v = num % 9;
    return (v == 0) ? 9 : v;
}

int main(int argc, char **argv) {
    return 0;
}
