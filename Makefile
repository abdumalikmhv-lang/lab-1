CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: lab1_malik

lab1_malik: lab1_malik.c
	$(CC) $(CFLAGS) -o lab1_malik lab1_malik.c

clean:
	rm -rf lab1_malik a

.PHONY: all clean
