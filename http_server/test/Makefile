
CFLAGS = -g
CC = gcc


TEST_BASENAMES = $(basename $(wildcard tst-*.c))

all: $(addsuffix .out,$(TEST_BASENAMES))

.PRECIOUS : %.o

%.o: %.c $(patsubst tst-%.o, ../src/%.c,$@)
	$(CC) $(CFLAGS) -c $^ $(patsubst %.o,,$(patsubst tst-%.o, ../src/%.c,$@))

%.out: %.o main.o $(patsubst tst-%.out, %.c, $@)
	$(CC) -o $@ $^ $(patsubst tst-%.out, %.o, $@) -lm 

clean:
	-rm *.o *~ *.out
