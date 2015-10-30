cc = gcc

src_file = mini_ant.c ant_queue.c ant_socket.c ant_array.c  \
	ant_dns.c ant_buf.c ant_http.c ant_log.c ant_hash.c \
	ant_context.c ant_page.c ant_url.c ant_look.c \
	ant_debug.c

obj_file = $(src_file:.c=.o)

CFLAGS = -c -g

VPATH = src

mini_ant: ${obj_file}
	${cc} -o $@ ${obj_file} -lpthread -lm

%.o:%.c
	${cc} ${CFLAGS} -o $@ $< 

clean:
	-rm ${obj_file} mini_ant *~ 2>/dev/null
cleanall:
	-rm ${src}/*~ 2>/dev/null
	-rm dump/* 2>/dev/null
	-rm log/* 2>/dev/null
	-rm *~ 2>/dev/null
