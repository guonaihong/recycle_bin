#include <stdio.h>
#include <assert.h>

//If version1 > version2 return 1, if version1 < version2 return -1, otherwise return 0
int compare_version(char *ver1, char *ver2) {
    char *p1 = ver1;
    char *p2 = ver2;

    int v1 = 0;
    int v2 = 0;

    while (*p1 || *p2) {
        while (*p1 && *p1 != '.') {
            v1 = v1 * 10 + *p1 - '0';
            p1++;
        }
        if (*p1 == '.') p1++;

        while (*p2 && *p2 != '.') {
            v2 = v2 * 10 + *p2 - '0';
            p2++;
        }
        if (*p2 == '.') p2++;

        if (v1 > v2) {
            return 1;
        } else if (v1 < v2) {
            return -1;
        }
        v1 = v2 = 0;
    }

    return 0;
}

int compareVersion(char* version1, char* version2) {
    return compare_version(version1, version2);
}

int main() {
#if 0
    printf("%d\n", ver2num("1.1"));
    printf("%d\n", ver2num("1.1.2"));
    printf("%d\n", ver2num("13.37"));
#endif
#if 1
    assert(compare_version("01", "1") == 0);
    printf("test1 ok\n");
    assert(compare_version("123", "123") == 0);
    printf("test2 ok\n");
    assert(compare_version("0.1", "0.0.1") == 1);
    printf("test3 ok\n");
    assert(compare_version("1.0", "1") == 0);
    printf("test4 ok\n");
#endif
    return 0;
}
