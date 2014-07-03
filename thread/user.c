#include "proto.h"
static write_err(int s, char *msg) {
	;
}

void process_msg(int s) {
    char *p;
    int  use, n, rv;

	for (;;) {
        rv = readrecv(&p, &use, &n);
        write(1, p, rv);
	}

failed:
	close(s);
}
