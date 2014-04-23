#include <stdio.h>
#include <stdint.h>

int ho_strlen(char *str);
int ho_strcmp(const char *s1, const char *s2);

int main(int argc, char **argv) {

	int i = 0;
	/*test ho_strlen*/
	for (; i < argc; i++) {
		printf("%d\n", ho_strlen(argv[i]));
	}

	/*test ho_strcmp*/
	printf("%d\n", ho_strcmp("12", "123"));
	printf("%d\n", ho_strcmp("12", "12"));
	printf("%d\n", ho_strcmp("123", "12"));
	return 0;
}
