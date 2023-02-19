CC = gcc
CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic -Werror

all : smallsh

smallsh : main.o smallsh.o utilities.o signals.o
				$(CC) $(CFLAGS) -o $@ $^

main.o : smallsh.h main.c

smallsh.o : smallsh.h smallsh.c

utilities.o : smallsh.h utilities.c

signals.o : smallsh.h signals.c

clean :
				-rm *.o
				-rm smallsh
