CC=gcc
CFLAGS=-Wall -Wextra -ggdb

all: math test example

math: repl.c
	$(CC) $(CFLAGS) -o math repl.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c

example: examples/example.c
	$(CC) $(CFLAGS) -I. -o example examples/example.c

clean:
	rm -rf math
	rm -rf test
	rm -rf example
