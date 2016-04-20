CC=clang++
CFLAGS=-g -Wall --std=c++11
OBJ=main.o Node.o

all: $(OBJ)
	$(CC) -o blockchain-sim $(OBJ)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

Node.o: Node.cpp
	$(CC) $(CFLAGS) -c Node.cpp

clean:
	-rm blockchain-sim *.o

.PHONY: clean
