CC=gcc
NCURSES=-lncurses

all: server human-control-client machine-control-client

server: server.c remote-char.h
	$(CC) server.c -o server $(NCURSES)

human-control-client: human-control-client.c remote-char.h
	$(CC) human-control-client.c -o human-control-client $(NCURSES)

machine-control-client: machine-control-client.c remote-char.h
	$(CC) machine-control-client.c -o machine-control-client $(NCURSES)

clean:
	rm -f server human-control-client machine-control-client /tmp/fifo_lab3