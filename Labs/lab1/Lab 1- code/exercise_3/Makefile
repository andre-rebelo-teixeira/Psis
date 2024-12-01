CC = gcc

CFLAGS = -g -Wall

TARGET = prog1

OBJ = lib1.o prog1.o

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

lib1.o: lib1.c lib.h
	$(CC) $(CFLAGS) -c lib1.c
prog1.o: prog1.c lib.h
	$(CC) $(CFLAGS) -c prog1.c

clean:
	rm -f $(OBJ) $(TARGET)
