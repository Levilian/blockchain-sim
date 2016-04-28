
// definitions for blockchain-sim.cpp

#define EVENT_NEW_TRANSACTION 1 // event type for a transaction (a transfer between wallets)
#define EVENT_NEW_BLOCK 2 // event type for a new block being mined (a set of transactions)
#define EVENT_TX_RELAY 3 // event type for a transaction being relayed to a node
#define EVENT_BLOCK_RELAY 4 // event type for a block being relayed to a node
#define SAMPST_TTC 1 // variable for time-to-confirmation sampling
#define SAMPST_TX_FEE 2 // variable for transaction fee sampling
#define STREAM_TX_INTERARRIVAL 1 // random number stream for transaction interarrival times
#define STREAM_BLOCK_INTERARRIVAL 2 // random number stream for block interarrival times
#define STREAM_LINK_SPEED 3 // random number stream for link speeds between nodes
#define LIST_TRANSACTIONS 1 // list to hold all transactions
#define MAX_BLOCKS 100 // the simulation will be stopped after this many blocks are mined
#define NUMBER_NODES 10 // total number of nodes on the network
#define MINER_FRACTION 0.1 // fraction of the nodes that are miners (rather than relay nodes)
#define DEFAULT_FEE 0.01 // default value for transaction fees
#define DEFAULT_BLOCK_REWARD 25.0 // default reward for miners when they mine a block
#define BLOCKS_BETWEEN_REWARD_CHANGES 10 // number of blocks between changes in block reward amount
#define IN_FILENAME "blockchain-sim.in"
#define OUT_FILENAME "blockchain-sim.out"
