CC=gcc
CFLAGS=-Wall -Wextra -ggdb -lm

all: math example benchmark

math: repl.c
	$(CC) $(CFLAGS) -o math repl.c

example: examples/example.c
	$(CC) $(CFLAGS) -I. -o example examples/example.c

benchmark: benchmark.c
	$(CC) $(CFLAGS) -I. -o benchmark benchmark.c

clean:
	rm -rf math
	rm -rf example
	rm -rf benchmark
