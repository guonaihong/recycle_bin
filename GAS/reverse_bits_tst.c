#include <stdio.h>
#include <assert.h>

extern int reverse_bits(unsigned int ui);

int main() {
    unsigned u; 
    u = reverse_bits(0x7fffffff);
    assert(u == 0xfffffffe);
    printf("u(%x)\n", u);

    u = reverse_bits(0x00ffffff);
    assert(u == 0xffffff00);
    printf("u(%x)\n", u);

    u = reverse_bits(0xffff00ff);
    assert(u == 0xff00ffff);
    printf("u(%x)\n", u);

    u = reverse_bits(0xffffff00);
    assert(u == 0x00ffffff);
    printf("u(%x)\n", u);

    u = reverse_bits(0xff00ffff);
    assert(u == 0xffff00ff);
    printf("u(%x)\n", u);
    return 0;
}
