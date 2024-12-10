CC = gcc
NCURSES = -lncurses
ZMQ = -lzmq

all: game-server astronaut-client 

game-server: game-server.c game-server.h messages.h list.h
	$(CC) game-server.c -o game-server $(NCURSES) $(ZMQ)
	
astronaut-client: astronaut-client.c
	$(CC) astronaut-client.c -o astronaut-client $(NCURSES) $(ZMQ)

clean:
	rm -f game-server astronaut-client