#include <stdio.h>
#include <string.h>
#include <limits.h>

/*
 *找到满足条件的数
 *给定函数d(n)=n+n的各位之和，n为正整数，如d(78)=78+7+8=93。这样这个函数可以看成一个生成器，如93可以看成由78生成。
 *定义数A：数A找不到一个数B可以由d(B)=A，即A不能由其他数生成。现在要写程序，找出1至10000里的所有符合数A定义的数。
 */
#define NUM_MAX 10000
#define NUM NUM_MAX / CHAR_BIT

#define ELT(d) ((d) / CHAR_BIT)
#define MASK(d) ((d) % CHAR_BIT)

typedef struct {
    unsigned char bit_data[NUM];
} gen_bit_t;

static void bit_zero(gen_bit_t *bit) {
    memset(bit, '\0', sizeof(gen_bit_t));
}

static void bit_set(gen_bit_t *bit, unsigned int n) {
    bit->bit_data[ELT(n)] |= 1 << MASK(n);
}

static int bit_isset(gen_bit_t *bit, unsigned int n) {
    return !!(bit->bit_data[ELT(n)] & 1 << MASK(n));
}

int gen(int n) {

    int r = 0;

    if (n <= 0)
        return -1;

    do {
        r += n % 10;
    } while (n /= 10);

    return r;
}

int main() {

    int i, t;
    gen_bit_t gen_bit;
    bit_zero(&gen_bit);
#if 0
    /* test bit */
    for (i = 0; i < NUM_MAX; i++) {
        bit_set(&gen_bit, i);
    }
    for (i = 0; i < NUM_MAX; i++) {
        if (bit_isset(&gen_bit, i))
            printf("%d\n", i);
    }
#endif

    for (i = 1; i < NUM_MAX; i++) {

        t = i + gen(i);
        if (t < NUM_MAX) {
            /*remove duplicate data*/
            if (!bit_isset(&gen_bit, t))
                printf("%d:%d\n", i , t);
            bit_set(&gen_bit, t);
        }
    }


    /* how to test
     * gcc file.c
     * ./a.out |awk -F ":" '{print $2 }' |sort |uniq -c  |grep -v '^ *1' |wc -l
     * if it is empty on the right*/
    return 0;
}
