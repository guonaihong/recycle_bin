#include "proto.h"
static write_err(int s, char *msg) {
	;
}

void process_msg(int s) {
	uint32_t total = 0;
	char buf[512] = "";
	char 	   *p, *p2;
	int            err;

	p2 = NULL;
	p  = buf;

	for (;;) {

		/*get data header*/
		readn(s, &total, 4);
		total = ntohl(total);

		fprintf(stderr, "::%d\n", total);

		if (total > sizeof(buf)) {
			p2 = p = malloc(total);

			if (p == NULL) {
				err = errno;
				write_err(s, strerrno(err));
				goto failed;
			}
		}

		/*reset buf*/
		free(p2);
		p  = buf;
		p2 = NULL;
	}

failed:
	close(s);
}
