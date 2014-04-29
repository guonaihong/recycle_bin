#include <stdio.h>
#include <string.h>

//add by gnh
static char *str_cpy(char *dst, const char *src) {
	char *d = dst;
	while ((*dst++ = *src++) != '\0')
		;
	return d;
}

static char *str_find_right(const char *str, const char *last, const char *substr, int sublen) {
	if (str > last || sublen <= 0)
		return NULL;

	sublen--;
	const char *sub  = substr + sublen;
	const char *bsub = NULL , *blast = NULL;

	for (; *last && (last - str) >= sublen; last--) {

		if (last[-sublen] == sub[-sublen])  {
			for (   bsub = sub, blast = last; 
				bsub > substr && *bsub == *blast;
				bsub--, blast--)
				/*space line*/;

			if (sub - bsub == sublen)
				return (char *)blast;
		}
	}

	return NULL;
}

char *str_del_all(char *str, const char *substr) {

	int sublen = strlen(substr);
	char *pos = NULL, *s = str;
	char *last = str + strlen(str) - 1;

	while ((pos = str_find_right(s, last, substr, sublen))) {
		str_cpy(pos, pos + sublen);
		last -= sublen;
	}

	return str;
}

int main(int argc, char **argv) {

	char buf[] = "hello </s>world </s> ni hao </s>";
	printf("%s\n", str_del_all(buf, "</s>"));

	char buf2[] = "1212123456";
	printf("%s\n", str_del_all(buf2, "12"));

	char buf3[] = "1a31231b3";
	printf("%s\n", str_del_all(buf3, "1b3"));
	return 0;
}
