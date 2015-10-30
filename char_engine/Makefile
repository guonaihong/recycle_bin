cc=gcc

main:main.c str_match.o
	gcc -o main main.c str_match.o -g -Wall
str_match.o:str_match.c
	gcc -c str_match.c -Wall -g

clean:
	-rm *~ *.o main
