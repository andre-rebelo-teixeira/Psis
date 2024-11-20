CC = gcc

CFLAGS = -fPIC -Wall

LDFLAGS = -ldl

TARGET = main
LIB1 = liblib1.so
LIB2 = liblib2.so

all: $(TARGET)

$(LIB1): lib1.c lib.h
	$(CC) $(CFLAGS) -shared -o $(LIB1) lib1.c

$(LIB2): lib2.c lib.h
	$(CC) $(CFLAGS) -shared -o $(LIB2) lib2.c

$(TARGET): main.c $(LIB1) $(LIB2)
	$(CC) -o $(TARGET) main.c $(LDFLAGS)

error:
	$(CC) main.c lib1.c lib2.c

clean:
	rm -f $(TARGET) $(LIB1) $(LIB2)
