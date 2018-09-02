CFLAGS = -D_REENTRANT -Wall -pedantic -Isrc

CC = gcc
OSNAME = $(shell uname)
ifeq ($(OSNAME), Linux)
	LDLIBS = -lpthread -lm -lrt
else
	LDFLAGS = -lpthread -lm
endif

CFLAGS  += -g
LDFLAGS += -g

TARGETS = tests/fixed_size tests/auto_add_del

all: $(TARGETS)

tests/fixed_size: tests/fixed_size.o src/processpool.o
tests/auto_add_del: tests/auto_add_del.o src/processpool.o

src/processpool.o: src/processpool.c src/processpool.h
		    $(CC) -c ${CFLAGS} -o $@ $<
clean:
		rm -f $(TARGETS) *~ */*~ */*.o

test:
	./tests/fixed_size
