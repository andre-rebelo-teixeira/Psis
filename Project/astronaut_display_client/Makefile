# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I. -I$(COMMON_PATH)
LDFLAGS = -lncurses -lzmq

# Source files for the child project
SRC = astronaut-display-client.c 
OBJ = $(SRC:.c=.o)
TARGET = astronaut-display-client

# Build rules
all: $(TARGET)

# Link step includes the common object file
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(COMMON_OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)