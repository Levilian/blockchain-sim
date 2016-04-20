CC=g++
CFLAGS=-g -Wall --std=c++11
OBJ=main.o Node.o simlib.o

all: $(OBJ)
	$(CC) -o blockchain-sim $(OBJ)

main.o: main.cpp simlib.o
	$(CC) $(CFLAGS) -c main.cpp simlib.c

Node.o: Node.cpp
	$(CC) $(CFLAGS) -c Node.cpp

simlib.o: simlib.c
	$(CC) -Wno-macro-redefined -Wno-return-type -x c -c simlib.c

clean:
	-rm blockchain-sim *.o

.PHONY: clean
