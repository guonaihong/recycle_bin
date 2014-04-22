#include <stdio.h>
#include <stdint.h>

int ho_strlen(char *str);

int main(int argc, char **argv) {

	while (*++argv) {
		printf("%d\n", ho_strlen(*argv));
	}
	return 0;
}
