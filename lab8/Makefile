CC = gcc
CFLAGS = -Wall
LDFLAGS = -ldl

all: libdll.so server client

libdll.so: dll.c dll.h
	$(CC) -shared -fPIC -o $@ dll.c

server: server.c
	$(CC) $(CFLAGS) -o $@ server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o $@ client.c

clean:
	rm -f server client libdll.so

run: all
	./server &
	./client file.txt file_1.txt 15	

.PHONY: all clean
