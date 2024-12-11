CC = gcc
NCURSES = -lncurses
ZMQ = -lzmq

all: game-server astronaut-client outer-space-display

game-server: game-server.c utils.c utils.h messages.h list.h
	$(CC) game-server.c utils.c -o game-server $(NCURSES) $(ZMQ)
	
astronaut-client: astronaut-client.c utils.c utils.h messages.h 
	$(CC) astronaut-client.c -o astronaut-client $(NCURSES) $(ZMQ)

outer-space-display: outer-space-display.c utils.c utils.h messages.h
	$(CC) outer-space-display.c utils.c -o outer-space-display $(NCURSES) $(ZMQ)

clean:
	rm -f game-server astronaut-client outer-space-display