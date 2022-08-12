CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=LogServer.cpp
LIBS=-lpthread

logserver: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o logserver
	
all: logserver
