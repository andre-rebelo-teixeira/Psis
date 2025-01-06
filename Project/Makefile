CC = gcc
NCURSES = -lncurses
ZMQ = -lzmq
PROTOBUF = -lprotobuf-c

all: protobuf game-server astronaut-display-client outer-space-display astronaut-client  

protobuf: score_message.proto
	protoc --c_out=. score_message.proto
	protoc --python_out=. score_message.proto

game-server: game-server.c utils.c utils.h messages.h list.h game-server.h score_message.pb-c.c
	$(CC) game-server.c utils.c score_message.pb-c.c -o game-server $(NCURSES) $(ZMQ) $(PROTOBUF)
        
astronaut-client: astronaut-client.c utils.c utils.h messages.h 
	$(CC) astronaut-client.c utils.c -o astronaut-client $(NCURSES) $(ZMQ)

outer-space-display: outer-space-display.c utils.c utils.h messages.h
	$(CC) outer-space-display.c utils.c -o outer-space-display $(NCURSES) $(ZMQ)

astronaut-display-client: astronaut-display-client.c astronaut-display-client.h utils.c utils.h messages.h
	$(CC) astronaut-display-client.c utils.c -o astronaut-display-client $(NCURSES) $(ZMQ)

clean:
	rm -f game-server astronaut-client outer-space-display astronaut-display-client score_message.pb-c.c score_message.pb-c.h score_message_pb2.py

