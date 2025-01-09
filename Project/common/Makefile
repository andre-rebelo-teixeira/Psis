# Variables
PROTOC = protoc
CFLAGS = -Wall -g
PYTHON_OUT = --python_out=.
C_OUT = --c_out=.
PROTO_FILE = score_message.proto
PROTO_C = score_message.pb-c.c
PROTO_H = score_message.pb-c.h
OBJECTS = score_message.pb-c.o utils.o

# Targets
all: $(OBJECTS)

# Generate .c and .h files from .proto
$(PROTO_C) $(PROTO_H): $(PROTO_FILE)
	$(PROTOC) $(C_OUT) $(PROTO_FILE)

# Compile the generated .c file into an object file
score_message.pb-c.o: $(PROTO_C)
	gcc $(CFLAGS) -c $(PROTO_C) -o $@

# Compile utils.c into an object file
utils.o: utils.c utils.h
	gcc $(CFLAGS) -c utils.c -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(PROTO_C) $(PROTO_H)
	rm -rf __pycache__

.PHONY: all clean
