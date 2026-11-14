CC = gcc
CFLAGS = -Wall


all: tcpserver tcpclient udpserver udpclient




tcpserver: tcpserver.c
	$(CC) $(CFLAGS) -o tcpserver2 tcpserver.c


tcpclient: tcpclient.c
	$(CC) $(CFLAGS) -o tcpclient2 tcpclient.c

udpserver: udpserver.c
	$(CC) $(CFLAGS) -o udpserver2 udpserver.c

udpclient: udpclient.c
	$(CC) $(CFLAGS) -o udpclient2 udpclient.c


clean:
	rm -f tcpserver2 tcpclient2 udpserver2 udpclient2

.PHONY: all clean