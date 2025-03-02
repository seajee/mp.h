CC=gcc
CFLAGS=-Wall -Wextra -ggdb

all: math

math: main.o arena.o interpreter.o parser.o vm.o
	$(CC) -o math main.o arena.o interpreter.o parser.o vm.o

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

arena.o: arena.c
	$(CC) $(CFLAGS) -c -o arena.o arena.c

interpreter.o: interpreter.c
	$(CC) $(CFLAGS) -c -o interpreter.o interpreter.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c -o parser.o parser.c

vm.o: vm.c
	$(CC) $(CFLAGS) -c -o vm.o vm.c


clean:
	rm -rf math
	rm -rf main.o
	rm -rf arena.o
	rm -rf interpreter.o
	rm -rf parser.o
	rm -rf vm.o
