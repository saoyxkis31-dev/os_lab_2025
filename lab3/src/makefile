CC=gcc
CFLAGS=-I.

all : N1 N2 N4

N1 : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o N1 find_min_max.o utils.o sequential_min_max.c $(CFLAGS)

N2 : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o N2 utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

N4 : N4.c
	$(CC) -o N4 N4.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm -f utils.o find_min_max.o N1 N2 N4