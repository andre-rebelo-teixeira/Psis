# Compiler and flags
CC = gcc
CFLAGS ?= -Wall -Wextra
LDFLAGS = -lncurses -lzmq -lprotobuf-c
COMMON_OBJS ?=

# Source files for the project
SRC = astronaut-display-client.c
OBJ = $(SRC:.c=.o)
TARGET = astronaut-display-client

# Build rules
all: $(TARGET)

# Link step includes the common object files
$(TARGET): $(OBJ) $(COMMON_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(TARGET)
