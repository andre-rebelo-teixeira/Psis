CC = gcc
NCURSES = -lncurses
ZMQ = -lzmq

all: game-server outer-space-display astronaut-client 

game-server: game-server.c
	$(CC) game-server.c -o game-server $(NCURSES) $(ZMQ)

outer-space-display: outer-space-display.c
	$(CC) outer-space-display.c -o outer-space-display $(NCURSES) $(ZMQ)

astronaut-client: astronaut-client.c
	$(CC) astronaut-client.c -o astronaut-client $(NCURSES) $(ZMQ)

clean:
	rm -f game-server outer-space-display astronaut-client