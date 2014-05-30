CC = gcc
main: ant_hash.o ant_queue.o parse_smtp_clent.o
	${CC} ant_queue.o ant_hash.o parse_smtp_clent.o -lnids -Wall
parse_smtp_clent.o:
	${CC} -c parse_smtp_clent.c -Wall
ant_hash.o: ant_hash.c
	${CC} -c ant_hash.c -Wall
ant_queue.o: ant_queue.c
	${CC} -c ant_queue.c -Wall

clean:
	rm *.o a.out
