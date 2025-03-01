
math: main.c
	cc -Wall -Wextra -ggdb -o math main.c

clean:
	rm -rf math
