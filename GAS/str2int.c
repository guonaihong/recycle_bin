#include <stdio.h>
#include <ctype.h>

/*
 * @param base 可以是0或者2-36之间的任意整数
 * 如果是0,函数会自己猜测要转的进制,如果str是以0x开头,等同base为16
 * 如果str是0b开头,等同base为2
 * 如果str是0开头,等同base为8
 */ 
int str2int(char *str, int base) {
    char sign;
    int rv = 0;
    char c;
    int newbase = -1;

    if ((base < 2 && base != 0) || base > 36)
        return 0;

    //跳过str开头的空白字符
    while(*str && isspace(*str))
        str++;

    //取得符号位
    sign = (*str == '-' || *str == '+') ? *str++ : '+';

    //猜测需要转换的类型
    if (*str == '0' && ++str) {
        if ((*str == 'x' || *str == 'X') && ++str) {
            newbase = 16;
        } else if ((*str == 'b' || *str == 'B') && ++str) {
            newbase = 2;
        } else {
            newbase = 8;
        }
    }

    //默认10进制
    if (base == 0) {
        base = newbase == -1 ? 10 : newbase;
    }

    //printf("base = %d:%s\n", base, str);
    for (;*str; str++) {
        //大写转小写
        c = *str | 0x20;
        if (c >= 'a' && c <= 'z') {
            rv = rv * base + c - 'a' + 10;
        } else if (c >= '0' && c <= '9') {
            rv = rv * base + c - '0';
        } else {
            break;
        }
        //printf("rv = %d\n", rv);
    }
    
    return sign == '-' ? -rv : rv;
}
