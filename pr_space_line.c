#include <stdio.h>

void pr_space_line(int n) {
    int i;
    for (i = 0; i < n; i++) {
        puts("\n");
    }
}

int main() {

    char buf[128] = "";
    fgets(buf, sizeof(buf), stdin);
    pr_space_line(atoi(buf));

    return 0;
}
