
// The code below simulates a P2P network similar to Bitcoin.

#include "Node.h"
#include "simlib.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define EVENT_NEW_TRANSACTION 1 // event type for a transaction (a transfer between wallets)
#define EVENT_NEW_BLOCK 2 // event type for a new block being mined (a set of transactions)
#define STREAM_INTERARRIVAL_TIME 1 // random number stream for transaction interarrival times
#define STREAM_LINK_SPEED 2 // random number stream for link speeds between nodes
#define MAX_BLOCKS 100 // the simulation will be stopped after this many blocks are mined
#define NUMBER_NODES 100 // total number of nodes on the network
#define MINER_FRACTION 0.1 // fraction of the nodes that are miners (rather than relay nodes)
#define IN_FILENAME "blockchain-sim.in"
#define OUT_FILENAME "blockchain-sim.out"

using namespace std;

int num_blocks, num_transactions, block_reward, links_per_node;
float mean_tx_interarrival, mean_link_speed; // tx is short for transaction
FILE *infile, *outfile;

void init_model(vector<Node*>* nodeList); // initialize the model
void add_link(Node* node1, Node* node2, float speed); // add a communication link between nodes
void new_transaction(); // run for every new transaction event
void new_block(); // run for every new block event
void report(); // print statistics from the simulation run

int main() {
    // Open input and output files.
    infile  = fopen(IN_FILENAME,  "r");
    if (infile == NULL) {
        perror(IN_FILENAME);
        exit(1);
    }
    outfile = fopen(OUT_FILENAME, "w");
    if (outfile == NULL) {
        perror(OUT_FILENAME);
        fclose(infile);
        exit(1);
    }

    // Read input parameters.
    fscanf(infile, "%d %d %f %f", &block_reward, &links_per_node,
           &mean_tx_interarrival, &mean_link_speed);
    fclose(infile);

    // Write report heading with input parameters.
    fprintf(outfile, "Mean interarrival time for transactions: %.3f\n", mean_tx_interarrival);
    fprintf(outfile, "Mean link speed: %.3f\n", mean_link_speed);
    fprintf(outfile, "Block reward: %d\n", block_reward);
    fprintf(outfile, "Links per node: %d\n", links_per_node);

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
    fclose(outfile);

    // free memory and exit
    for (vector<Node*>::iterator it = nodeList->begin(); it != nodeList->end(); ++it) {
        delete *it;
    }
    return 0;
}

void init_model(vector<Node*>* nodeList) {
    // initialize statistical variables and random number streams
    num_blocks = 0;
    num_transactions = 0;
    srand(time(NULL) / 2);
    lcgrandst((time(NULL) + getpid()), STREAM_INTERARRIVAL_TIME);
    lcgrandst((time(NULL) - getpid()), STREAM_LINK_SPEED);

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

    // add links between nodes based on links_per_node and mean_link_speed
    for (vector<Node*>::iterator it = nodeList->begin(); it != nodeList->end(); ++it) {
        while ((*it)->get_num_links() < links_per_node) { // if more links are needed
            // find a node to link with
            unsigned int random_index = rand() % NUMBER_NODES;
            while (nodeList->at(random_index)->get_num_links() >= links_per_node) {
                random_index = rand() % NUMBER_NODES;
            }
            add_link(*it, nodeList->at(random_index), expon(mean_link_speed, STREAM_LINK_SPEED));
        }
    }

    // schedule the first transaction and first block to occur
    event_schedule(sim_time + expon(mean_tx_interarrival, STREAM_INTERARRIVAL_TIME), EVENT_NEW_TRANSACTION);
    event_schedule(sim_time + expon((mean_tx_interarrival * 10), STREAM_INTERARRIVAL_TIME), EVENT_NEW_BLOCK);
}

void new_transaction() {
    ++num_transactions;

    //TODO let the network know about the transaction

    // schedule the next transaction
    event_schedule(sim_time + expon(mean_tx_interarrival, STREAM_INTERARRIVAL_TIME), EVENT_NEW_TRANSACTION);
}

void new_block() {
    ++num_blocks;

    //TODO let the network know about the block

    // schedule the next block
    event_schedule(sim_time + expon((mean_tx_interarrival * 10), STREAM_INTERARRIVAL_TIME), EVENT_NEW_BLOCK);
}

void report() {
    printf("Number of blocks: %d\n", num_blocks);
    printf("Number of transactions: %d\n", num_transactions);
    //TODO print rest of report
}

void add_link(Node* node1, Node* node2, float speed) {
    node1->add_link(node2, speed);
    node2->add_link(node1, speed);
}
