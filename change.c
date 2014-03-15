#include <stdio.h>
#include <stdlib.h>

struct time_s {
    int y;
    int m;
    int d;
};

#define ARRAYR_SIZE(a) (sizeof(a)/sizeof((a)[0]))
int cal(unsigned num, struct time_s *t) {

    int i, week, day;
    int sum_day = 0; // 2014-1-1 is four day
                         /*1  2    3   4   5   6   7  8   9   10  11  12*/
    static int month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* if num is 4123 week is 12
     * day is 3 */
    week = num % 1000 /10;
    day = num % 10;

    /*check*/
    if (day > 7) {
        printf("day must is <= 7");
        return -1;
    }

    /*debug*/
    printf("%d:%d\n", week, day);

    sum_day = (week - 1) * 7 - 3;

    for (i = 0; i < ARRAYR_SIZE(month); i++) {

        if (sum_day <= month[i])
            break;

        sum_day -= month[i];
    }


    if (sum_day + day > month[i]) {
        sum_day -= month[i];
        i++;
    }

    printf(":::%d:%d:%d\n", i + 1, sum_day + day, month[i]);

    t->y = 2014;
    t->m = i + 1;
    t->d = sum_day + day;
    return 0;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("<a.out> number\n");
        return -1;
    }

    struct time_s t = {0};
    cal(atoi(argv[1]), &t);

    printf("year = %d, month = %d, day = %d\n", 
            t.y, t.m, t.d);

    return 0;
}
