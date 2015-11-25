#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
extern int str2int(const char *str, int base);

int main() {

    //test
    //==== max=====
    //2进制
    assert(str2int("0b11111111111111111111111111111111", 0) == 0xFFFFFFFF);
    assert(str2int("0b11111111111111111111111111111111", 2) == 0xFFFFFFFF);
    printf("2 max:%x\n", str2int("0b11111111111111111111111111111111", 0));
    printf("2 max:%x\n", str2int("0b11111111111111111111111111111111", 2));

    //8进制
    assert(str2int("0777777777777", 0) == 0xFFFFFFFF);
    assert(str2int("0777777777777", 8) == 0xFFFFFFFF);

    printf("8 max:%x\n", str2int("0777777777777", 0));
    printf("8 max:%u\n", str2int("0777777777777", 0));
    printf("8 max:%x\n", str2int("0777777777777", 8));
    printf("8 max:%u\n", str2int("0777777777777", 8));

    //10进制
    assert(str2int("4294967295", 0) == 0xFFFFFFFF);
    assert(str2int("   \t\n\r 4294967295", 0) == 0xFFFFFFFF);

    printf("10 max:%u\n", str2int("4294967295	", 0));
    printf("10 max:%u\n", str2int("4294967295	", 10));

    //16进制
    assert(str2int("0xffffffff", 0) == 0xFFFFFFFF);
    assert(str2int("0xFFFFFFFF", 0) == 0xFFFFFFFF);
    assert(str2int("0xffffffff", 16) == 0xFFFFFFFF);
    assert(str2int("0XFFFFFFFF", 16) == 0xFFFFFFFF);

    printf("16 max:%x\n", str2int("0xffffffff", 0));
    printf("16 max:%x\n", str2int("0XFFFFFFFf", 0));
    printf("16 max:%x\n", str2int("0xffffffff", 16));
    printf("16 max:%x\n", str2int("0XFFFFFFFf", 16));

    //=====min=====
    //2进制
    assert(str2int("0b10000000000000000000000000000000", 0) == 0x80000000);
    assert(str2int("0b10000000000000000000000000000000", 0) == 0x80000000);
    assert(str2int("0b10000000000000000000000000000000", 2) == 0x80000000);
    assert(str2int("0b10000000000000000000000000000000", 0) == 0x80000000);
    assert(str2int("0b10000000000000000000000000000000", 2) == 0x80000000);

    printf("2 min:%x\n", str2int("0b10000000000000000000000000000000", 0));
    printf("2 min:%x\n", str2int("0b10000000000000000000000000000000", 2));
    printf("2 min:%d\n", str2int("0b10000000000000000000000000000000", 0));
    printf("2 min:%d\n", str2int("0b10000000000000000000000000000000", 2));

    //8进制
    //
    assert(str2int("020000000000", 0) == 0x80000000);
    assert(str2int("020000000000", 8) == 0x80000000);
    assert(str2int("020000000000", 0) == 0x80000000);
    assert(str2int("020000000000", 8) == 0x80000000);
    assert(str2int("020000000000", 8) == 0x80000000);

    printf("8 min:%d\n", str2int("020000000000", 8));
    printf("8 min:%d\n", str2int("020000000000", 0));
    printf("8 min:%x\n", str2int("020000000000", 8));
    printf("8 min:%x\n", str2int("020000000000", 8));

    //10进制
    assert(str2int("-2147483648", 10) == -2147483648);
    assert(str2int("-2147483648", 0)  == -2147483648);

    printf("10 min:%d\n", str2int("-2147483648", 10));
    printf("10 min:%x\n", str2int("-2147483648", 10));

    //16进制
    assert(str2int("0x80000000", 16) == 0x80000000);
    assert(str2int("0x80000000", 16) == 0x80000000);
    printf("16 min:%d\n", str2int("0x80000000", 16));
    printf("16 min:%x\n", str2int("0x80000000", 16));

    //printf("%x", strtoul("-2147483648", NULL, ));
    //35进制
    assert(str2int("z", 35) == 35);
    printf("35:%d\n", str2int("z", 35));
    return 0;
}
