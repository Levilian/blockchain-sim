CC=g++
CFLAGS=--std=c++11
OBJ=blockchain-sim.o Node.o simlib.o

all: executable

debug: CFLAGS += -DDEBUG -g
debug: executable

executable: $(OBJ)
	$(CC) -o blockchain-sim $(OBJ)

blockchain-sim.o: blockchain-sim.cpp simlib.o
	$(CC) $(CFLAGS) -c blockchain-sim.cpp simlib.c

Node.o: Node.cpp
	$(CC) $(CFLAGS) -c Node.cpp

simlib.o: simlib.c
	$(CC) -x c -c simlib.c

clean:
	-rm blockchain-sim *.o

.PHONY: clean debug executable
