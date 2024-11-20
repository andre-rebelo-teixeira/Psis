CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-L. -lfuncs

all: funcs.so execution-server client

execution-server: execution-server.c funcs.h funcs.so
	$(CC) $(CFLAGS) execution-server.c -o execution-server -ldl $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client

funcs.so: funcs.c funcs.h
	$(CC) -fPIC -shared funcs.c -o funcs.so -o libfuncs.so

clean:
	rm -f execution-server client funcs.so libfuncs.so
