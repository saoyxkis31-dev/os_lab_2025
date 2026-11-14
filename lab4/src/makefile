CC = gcc
CFLAGS = -Wall
PTHREAD_LIB = -pthread

all: parallel_min_max process_memory psum

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -c find_min_max.c -o find_min_max.o

parallel_min_max: utils.o find_min_max.o parallel_min_max.c
	$(CC) $(CFLAGS) parallel_min_max.c utils.o find_min_max.o -o parallel_min_max

process_memory: process_memory.c
	$(CC) $(CFLAGS) process_memory.c -o process_memory

# компиляция библиотеки lib1 и создание архива lib2.a
lib1.o: lib1.c lib1.h
	$(CC) $(CFLAGS) -c lib1.c -o lib1.o

lib2.a: lib1.o
	ar rcs lib2.a lib1.o

# компилируем parallel_sum в объектный файл
parallel_sum.o: parallel_sum.c lib1.h
	$(CC) $(CFLAGS) -c parallel_sum.c -o parallel_sum.o

# линкуем psum: библиотека -L. -l2 и флаг -pthread идут строго в конце строки
psum: parallel_sum.o utils.o lib2.a
	$(CC) $(CFLAGS) parallel_sum.o utils.o lib2.a -o psum $(PTHREAD_LIB)
clean:
	rm -f *.o *.a parallel_min_max process_memory psum