#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    const char *(a[][11]) = {
        {"  000 ",
        " 0     0",
        " 0     0",
        " 0     0",
        "   000  ",
        },
        {
        "   1    ",
        " 1 1    ",
        "   1    ",
        "   1    ",
        "1111111 "
        },
        {
        "222222 ",
        "     2 ",
        "222222 ",
        "2      ",
        "222222 "
        },
        {
        "333333 ",
        "     3 ",
        "333333 ",
        "     3 ",
        "333333 "
        },
        {
        "4    4 ",
        "4    4 ",
        "444444 ",
        "     4 ",
        "     4 "
        },
        {
        "555555 ",
        "5      ",
        "555555 ",
        "     5 ",
        "555555 "
        },
        {
        "666666 ",
        "6      ",
        "666666 ",
        "6    6 ",
        "666666 "
        },
        {
        "777777 ",
        "     7 ",
        "     7 ",
        "     7 ",
        "     7 "
        },
        {
        "888888 ",
        "8    8 ",
        "888888 ",
        "8    8 ",
        "888888 "
        },
        {
        "999999 ",
        "9    9 ",
        "999999 ",
        "     9 ",
        "999999 "
        }
    };

    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 0;
    }

    char *p;
    int d, r, row;
    row = (int)(sizeof(a[0]) / sizeof(a[0][0]));

    for (r = 0; r < row; r++) {

        for (p = argv[1]; *p; p++) {
            d = *p - '0';
            if (d < 0 || d > 9) {
                printf("error number \n");
                return EXIT_FAILURE;
            }
            if (a[d][r] == NULL)
                goto done;

            printf("%s", a[d][r]);
        }
        printf("\n");
    }
done:
    return 0;
}
