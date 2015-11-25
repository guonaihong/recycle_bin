#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

char *int2str(int val, char *str, int base) {
    static char tab[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buf[sizeof(int) * 8 + 1] = "";
    char *p = buf;
    char *p2 = str;
    unsigned uval = val;

    //检查base的范围
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    //负数
    uval = val;
    if (val < 0 && base == 10) {
        uval = -val;
    }

    do {
        *p++ = tab[uval % base];
        //printf("%d:%d:%d\n", uval, uval % base, base);
    } while (uval /= base);

    if (val < 0 && base == 10) {
        *p++ = '-';
    }

    p--;
    while (p >= buf && (*p2++ = *p) != '\0')
        p--;
    *p2 = '\0';

    return str;
}

void tst_max_2() {
    char buf_int2str[34] = "";
    printf("max 2#%s\n", int2str(CHAR_MAX, buf_int2str, 2));

    printf("max 2#%s\n", int2str(SHRT_MAX, buf_int2str, 2));

    printf("max 2#%s\n", int2str(INT_MAX, buf_int2str, 2));
}

void tst_max_8() {
    char buf_int2str[34] = "";
    printf("max 8#%s\n", int2str(CHAR_MAX, buf_int2str, 8));

    printf("max 8#%s\n", int2str(SHRT_MAX, buf_int2str, 8));

    printf("max 8#%s\n", int2str(INT_MAX, buf_int2str, 8));

}

void tst_max_10() {
    char buf_int2str[34] = "";
    char buf_snprintf[34] = "";

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%d", CHAR_MAX);
    assert(!strcmp(int2str(CHAR_MAX, buf_int2str, 10), buf_snprintf));

    printf("max 10#%d:%s:%s\n", CHAR_MAX, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%d", SHRT_MAX);
    assert(!strcmp(int2str(SHRT_MAX, buf_int2str, 10), buf_snprintf));
    printf("max 10#%d:%s:%s\n", SHRT_MAX, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%d", INT_MAX);
    assert(!strcmp(int2str(INT_MAX, buf_int2str, 10), buf_snprintf));
    printf("max 10#%d:%s:%s\n", INT_MAX, buf_int2str, buf_snprintf);
}

void tst_max_16() {
    char buf_int2str[34] = "";
    char buf_snprintf[34] = "";
    snprintf(buf_snprintf, sizeof(buf_snprintf), "%x", CHAR_MAX);
    assert(!strcmp(int2str(CHAR_MAX, buf_int2str, 16), buf_snprintf));
    printf("max 16#%d:%s:%s\n", CHAR_MAX, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%x", SHRT_MAX);
    assert(!strcmp(int2str(SHRT_MAX, buf_int2str, 16), buf_snprintf));
    printf("max 16#%d:%s:%s\n", SHRT_MAX, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%x", INT_MAX);
    assert(!strcmp(int2str(INT_MAX, buf_int2str, 16), buf_snprintf));
    printf("max 16#%d:%s:%s\n", INT_MAX, buf_int2str, buf_snprintf);
}

void tst_min_2() {
    char buf_int2str[34] = "";
    printf("min 2#%s\n", int2str(CHAR_MIN, buf_int2str, 2));

    printf("min 2#%s\n", int2str(SHRT_MIN, buf_int2str, 2));

    printf("min 2#%s\n", int2str(INT_MIN, buf_int2str, 2));
}

void tst_min_8() {
    char buf_int2str[34] = "";
    printf("min 8#%s\n", int2str(CHAR_MIN, buf_int2str, 8));

    printf("min 8#%s\n", int2str(SHRT_MIN, buf_int2str, 8));

    printf("min 8#%s\n", int2str(INT_MIN, buf_int2str, 8));
}

void tst_min_10() {
    char buf_int2str[34] = "";
    char buf_snprintf[34] = "";
    snprintf(buf_snprintf, sizeof(buf_snprintf), "%d", CHAR_MIN);
    assert(!strcmp(int2str(CHAR_MIN, buf_int2str, 10), buf_snprintf));
    printf("min 10#%d:%s:%s\n", CHAR_MIN, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%d", SHRT_MIN);
    assert(!strcmp(int2str(SHRT_MIN, buf_int2str, 10), buf_snprintf));
    printf("min 10#%d:%s:%s\n", SHRT_MIN, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%d", INT_MIN);
    assert(!strcmp(int2str(INT_MIN, buf_int2str, 10), buf_snprintf));
    printf("min 10#%d:%s:%s\n", INT_MIN, buf_int2str, buf_snprintf);
}

void tst_min_16() {
    char buf_int2str[34] = "";
    char buf_snprintf[34] = "";

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%x", CHAR_MIN);
    assert(!strcmp(int2str(CHAR_MIN, buf_int2str, 16), buf_snprintf));
    printf("min 16#%d:%s:%s\n", CHAR_MIN, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%x", SHRT_MIN);
    assert(!strcmp(int2str(SHRT_MIN, buf_int2str, 16), buf_snprintf));
    printf("min 16#%d:%s:%s\n", SHRT_MIN, buf_int2str, buf_snprintf);

    snprintf(buf_snprintf, sizeof(buf_snprintf), "%x", INT_MIN);
    assert(!strcmp(int2str(INT_MIN, buf_int2str, 16), buf_snprintf));
    printf("min 16#%d:%s:%s\n", INT_MIN, buf_int2str, buf_snprintf);
}

void tst_0() {
    char buf_int2str[34] = "";
    int2str(0, buf_int2str, 16);
    printf("0 #%s\n", buf_int2str);
}

int main() {
#if 0
    tst_max_2();
    tst_max_8();
    tst_max_10();
    tst_max_16();

    tst_min_2();
    tst_min_8();
    tst_min_10();
    tst_min_16();

    tst_0();
#endif
#if 0
    int val = INT_MIN;
    printf("##%d:%d:%u\n", val, -val, -val);
    //printf("%d:%d:%u\n", INT_MIN, -INT_MIN, -INT_MIN);
#endif

    char buf_int2str[34] = "";
    char c = -1;
    printf("%s\n", int2str(c, buf_int2str, 2));
    printf("%s\n", int2str(-1, buf_int2str, 2));
    return 0;
}
