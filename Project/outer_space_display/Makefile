# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I. -I$(COMMON_PATH)
LDFLAGS = -lncurses -lzmq -lprotobuf-c
COMMON_OBJS ?=

# Source files for the project
SRC = outer-space-display.c
OBJ = $(SRC:.c=.o)
TARGET = outer_space_display

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
