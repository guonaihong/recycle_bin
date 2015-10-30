cc = gcc

main: parse_page.o  ant_page.o
	${cc} -o parse_page ant_page.o parse_page.c -g -Wall -lm
ant_page.o: ../../src/ant_page.c
	${cc} -c ../../src/ant_page.c -Wall

clean:
	rm *~
	rm *.o
