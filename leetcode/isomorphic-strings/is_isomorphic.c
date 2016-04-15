#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

bool is_isomorphic(char* s, char* t) {
    char *ps = s;
    char *pt = t;

    char tab_left[128] = "";
    char tab_right[128] = "";

    while (*ps && *pt) {
        if (tab_right[(int)*pt] == 0) {
            tab_right[(int)*pt] = *ps;
        }
        else if (tab_right[(int)*pt] != *ps) {
            return 0;
        }

        if (tab_left[(int)*ps] == 0) {
            tab_left[(int)*ps] = *pt;
        }
        else if (tab_left[(int)*ps] != *pt) {
            return 0;
        }
        ps++, pt++;
    }

    if (*ps - *pt != 0)
        return 0;

    return 1;
}

int main() {
    int b = 0;
    b = is_isomorphic("ab", "aa");
    //assert(b == 0);
    printf("...b = %d\n", b);

    b = is_isomorphic("egg", "add");
    assert(b == 1);
    printf("b = %d\n", b);

    b = is_isomorphic("foo", "bar");
    assert(b == 0);
    printf("b = %d\n", b);

    b = is_isomorphic("paper", "title");
    assert(b == 1);
    printf("b = %d\n", b);

    return 0;
}
