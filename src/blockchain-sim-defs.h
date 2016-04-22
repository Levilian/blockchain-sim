
// definitions for blockchain-sim.cpp

#define EVENT_NEW_TRANSACTION 1 // event type for a transaction (a transfer between wallets)
#define EVENT_NEW_BLOCK 2 // event type for a new block being mined (a set of transactions)
#define EVENT_TX_RELAY 3 // event type for a transaction being relayed to a node
#define EVENT_BLOCK_RELAY 4 // event type for a block being relayed to a node
#define STREAM_INTERARRIVAL_TIME 1 // random number stream for transaction interarrival times
#define STREAM_LINK_SPEED 2 // random number stream for link speeds between nodes
#define LIST_TRANSACTIONS 1 // list to hold all transactions
#define MAX_BLOCKS 100 // the simulation will be stopped after this many blocks are mined
#define NUMBER_NODES 10 // total number of nodes on the network
#define MINER_FRACTION 0.1 // fraction of the nodes that are miners (rather than relay nodes)
#define IN_FILENAME "blockchain-sim.in"
#define OUT_FILENAME "blockchain-sim.out"
