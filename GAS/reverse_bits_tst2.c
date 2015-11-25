#include <stdio.h>

extern int reverse_bits(unsigned int ui);

int main() {
    unsigned u; 
    u = reverse_bits(0x00ffffff);
    printf("u(%x)\n", u);
    return 0;
}
