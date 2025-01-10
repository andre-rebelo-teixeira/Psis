# Variables
PROTOC = protoc
CFLAGS = -Wall -g
PYTHON_OUT = --python_out=.
C_OUT = --c_out=.
PROTO_FILE = messages.proto
PROTO_C = messages.pb-c.c
PROTO_H = messages.pb-c.h
PROTO_PY = messages_pb2.py
OBJECTS = messages.pb-c.o utils.o

# Targets
all: $(OBJECTS) $(PROTO_PY)

# Generate .c and .h files from .proto
$(PROTO_C) $(PROTO_H): $(PROTO_FILE)
	$(PROTOC) $(C_OUT) $(PROTO_FILE)

# Compile the generated .c file into an object file
score_message.pb-c.o: $(PROTO_C)
	gcc $(CFLAGS) -c $(PROTO_C) -o $@

# Generate .py file from .proto
$(PROTO_PY): $(PROTO_FILE)
	$(PROTOC) $(PYTHON_OUT) $(PROTO_FILE)

# Compile utils.c into an object file
utils.o: utils.c utils.h
	gcc $(CFLAGS) -c utils.c -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(PROTO_C) $(PROTO_H) $(PROTO_PY)
	rm -rf __pycache__

.PHONY: all clean
