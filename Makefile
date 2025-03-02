CC=gcc
CFLAGS=-Wall -Wextra -ggdb

all: math

math: main.o arena.o interpreter.o parser.o
	$(CC) -o math main.o arena.o interpreter.o parser.o

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

arena.o: arena.c
	$(CC) $(CFLAGS) -c -o arena.o arena.c

interpreter.o: interpreter.c
	$(CC) $(CFLAGS) -c -o interpreter.o interpreter.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c -o parser.o parser.c


clean:
	rm -rf math
	rm -rf main.o
	rm -rf arena.o
	rm -rf interpreter.o
	rm -rf parser.o
