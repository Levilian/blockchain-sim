
// The code below simulates a P2P network similar to Bitcoin.

#include "Node.h"
#include "simlib.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

#define EVENT_NEW_TRANSACTION 1 // event type for a transaction (a transfer between wallets)
#define EVENT_NEW_BLOCK 2 // event type for a new block being mined (a set of transactions)
#define MAX_BLOCKS 100 // the simulation will be stopped after this many blocks are mined
#define NUMBER_NODES 100 // total number of nodes on the network
#define MINER_FRACTION 0.1 // fraction of the nodes that are miners (rather than relay nodes)

using namespace std;

int num_blocks, num_transactions;

void init_model(vector<Node*>* nodeList); // initialize the model
void add_link(Node* node1, Node* node2, int speed); // add a communication link between nodes
void new_transaction(); // run for every new transaction event
void new_block(); // run for every new block event
void report(); // print statistics from the simulation run

int main() {
    // initialize simlib
    init_simlib();

    // set number of attributes per record
    maxatr = 4;

    // initialize model
    vector<Node*>* nodeList = new vector<Node*>;
    init_model(nodeList);

    // run the simulation until enough blocks are mined
    while (num_blocks < MAX_BLOCKS) {
        // determine the next event
        timing();
        
        // invoke the appropriate event function
        switch(next_event_type) {
            case EVENT_NEW_TRANSACTION:
                new_transaction();
                break;
            case EVENT_NEW_BLOCK:
                new_block();
                break;
        }
    }

    // write out a report
    report();

    // free memory and exit
    for (vector<Node*>::iterator it = nodeList->begin(); it != nodeList->end(); ++it) {
        delete *it;
    }
    return 0;
}

void init_model(vector<Node*>* nodeList) {
    // initialize statistical variables
    num_blocks = 0;
    num_transactions = 0;

    // add nodes to the nodeList
    unsigned int num_miners = MINER_FRACTION * NUMBER_NODES;
    unsigned int num_relays = NUMBER_NODES - num_miners;
    for (unsigned int i = 0; i < num_miners; ++i) {
        Node* n = new Node(MINER);
        nodeList->push_back(n);
    }
    for (unsigned int i = 0; i < num_relays; ++i) {
        Node* n = new Node(RELAY);
        nodeList->push_back(n);
    }

    // debug
    for (vector<Node*>::iterator it = nodeList->begin(); it != nodeList->end(); ++it) {
        cout << **it << endl;
    }

    // TODO add links between nodes

    // schedule the first block to be mined
    event_schedule(sim_time + 5, EVENT_NEW_BLOCK);
}

void new_transaction() {
    ++num_transactions;
    //TODO schedule event
}

void new_block() {
    ++num_blocks;

    // schedule the next block
    event_schedule(sim_time + 5, EVENT_NEW_BLOCK);
}

void report() {
    printf("Number of blocks: %d\n", num_blocks);
    printf("Number of transactions: %d\n", num_transactions);
    //TODO print rest of report
}

void add_link(Node* node1, Node* node2, int speed) {
    node1->add_link(node2, speed);
    node2->add_link(node1, speed);
}
