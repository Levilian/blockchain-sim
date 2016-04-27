CC=g++
CFLAGS=-g --std=c++11
OBJ=blockchain-sim.o Node.o simlib.o


#Stuff taken from stackoverflow

all: executable

debug: CFLAGS += -DDEBUG
debug: executable


# Regular stuff

executable: $(OBJ)
	$(CC) -o blockchain-sim $(OBJ)

blockchain-sim.o: blockchain-sim.cpp simlib.o
	$(CC) $(CFLAGS) -c blockchain-sim.cpp simlib.c

Node.o: Node.cpp
	$(CC) $(CFLAGS) -c Node.cpp

simlib.o: simlib.c
	$(CC) -Wno-macro-redefined -Wno-return-type -x c -c simlib.c

clean:
	-rm blockchain-sim *.o

.PHONY: clean
