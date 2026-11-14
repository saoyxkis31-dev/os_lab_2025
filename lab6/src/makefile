CC = gcc
CFLAGS = -Wall -pthread
AR = ar
ARFLAGS = rcs

all: server client

# стат библа
libmodulo.a: modulo.o
	$(AR) $(ARFLAGS) $@ $^

modulo.o: modulo.c modulo.h
	$(CC) $(CFLAGS) -c modulo.c

# сервер с библой
server: server.o libmodulo.a
	$(CC) $(CFLAGS) -o $@ server.o -L. -lmodulo

server.o: server.c modulo.h
	$(CC) $(CFLAGS) -c server.c

# клиента с библой
client: client.o libmodulo.a
	$(CC) $(CFLAGS) -o $@ client.o -L. -lmodulo

client.o: client.c modulo.h
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o *.a server client

.PHONY: all clean