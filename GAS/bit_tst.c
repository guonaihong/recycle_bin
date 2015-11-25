#include <stdio.h>
#include <assert.h>

extern void set_bit(char arr[], unsigned int bit_number);
extern void clear_bit(char arr[], unsigned int bit_number);
extern int  test_bit(char arr[], unsigned int bit_number);
extern void assign_bit(char arr[], unsigned int bit_number, int value);

void test_one_bit() {
    printf("test one bit start...\n");
    unsigned char c = 0;
    set_bit(&c, 1);
    assert(test_bit(&c, 1) == 1);

    clear_bit(&c, 1);
    assert(test_bit(&c, 1) == 0);

    assign_bit(&c, 1, 1);
    assert(test_bit(&c, 1) == 1);
    assert(c == 2);
    assign_bit(&c, 1, 0);
    assert(test_bit(&c, 1) == 0);
    assert(c == 0);
}

void test_byte() {
    printf("test one byte start...\n");
    unsigned char c = 0;
    int i;
    for (i = 0; i < sizeof(c) * 8; i++) {
        set_bit(&c, i);
    }
    assert(c == 0xff);
    for (i = 0; i < sizeof(c) * 8; i++) {
        assert(test_bit(&c, i) == 1);
    }
    for (i = 0; i < sizeof(c) * 8; i++) {
        clear_bit(&c, i);
    }
    assert(c == 0);

    for (i = 0; i < sizeof(c) * 8; i++) {
        assign_bit(&c, i, 1);
    }
    assert(c == 0xff);
    for (i = 0; i < sizeof(c) * 8; i++) {
        assign_bit(&c, i, 0);
    }
    assert(c == 0);
}

void test_two_byte() {
    printf("test two byte start...\n");
    unsigned short s = 0;
    int i;
    for (i = 0; i < sizeof(s) * 8; i++) {
        set_bit((char *)&s, i);
    }
    assert(s == 0xffff);
    for (i = 0; i < sizeof(s) * 8; i++) {
        assert(test_bit((char *)&s, i) == 1);
    }
    for (i = 0; i < sizeof(s) * 8; i++) {
        clear_bit((char *)&s, i);
    }
    assert(s == 0);

    for (i = 0; i < sizeof(s) * 8; i++) {
        assign_bit((char *)&s, i, 1);
    }
    assert(s == 0xffff);
    for (i = 0; i < sizeof(s) * 8; i++) {
        assign_bit((char *)&s, i, 0);
    }
    assert(s == 0);
}

int main() {
    test_one_bit();
    test_byte();
    test_two_byte();
    //test_xxoo();
    return 0;
}
