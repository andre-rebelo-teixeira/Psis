CC = gcc
NCURSES = -lncurses
ZMQ = -lzmq

all: game-server astronaut-client outer-space-display astronaut-display-client

game-server: game-server.c utils.c utils.h messages.h list.h game-server.h
	$(CC) game-server.c utils.c -o game-server $(NCURSES) $(ZMQ)
	
astronaut-client: astronaut-client.c utils.c utils.h messages.h 
	$(CC) astronaut-client.c -o astronaut-client $(NCURSES) $(ZMQ)

outer-space-display: outer-space-display.c utils.c utils.h messages.h
	$(CC) outer-space-display.c utils.c -o outer-space-display $(NCURSES) $(ZMQ)
astronaut-display-client: astronaut-display-client.c astronaut-display-client.h utils.c utils.h messages.h
	$(CC) astronaut-display-client.c utils.c -o astronaut-display-client $(NCURSES) $(ZMQ)

clean:
	rm -f game-server astronaut-client outer-space-display