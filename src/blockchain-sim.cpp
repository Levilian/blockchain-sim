
// The code below simulates a P2P network similar to Bitcoin.

#include "Node.h"
#include "simlib.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "blockchain-sim-defs.h"

using namespace std;

int num_blocks, num_transactions, block_reward, min_links_per_node;
float mean_tx_interarrival, mean_link_speed; // tx is short for transaction
FILE *infile, *outfile;
vector<Node*>* nodeList;

void init_model(); // initialize the model
void add_link(Node* node1, Node* node2, float speed); // add a communication link between nodes
void new_transaction(); // run for every new transaction event
void new_block(); // run for every new block event
void tx_relay(); // run when transactions are relayed to nodes
void block_relay(); // run when blocks are relayed to nodes
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
    fscanf(infile, "%d %d %f %f", &block_reward, &min_links_per_node,
           &mean_tx_interarrival, &mean_link_speed);
    fclose(infile);

    // Write report heading with input parameters.
    fprintf(outfile, "Mean interarrival time for transactions: %.3f\n", mean_tx_interarrival);
    fprintf(outfile, "Mean link speed: %.3f\n", mean_link_speed);
    fprintf(outfile, "Block reward: %d\n", block_reward);
    fprintf(outfile, "Min links per node: %d\n", min_links_per_node);

    // initialize simlib
    init_simlib();

    // set number of attributes per record
    maxatr = 4;

    // initialize model
    init_model();

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
            case EVENT_TX_RELAY:
                tx_relay();
                break;
            case EVENT_BLOCK_RELAY:
                block_relay();
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

void init_model() {
    // allocate memory for a vector of nodes
    nodeList = new vector<Node*>;

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
        Node* n = new Node(MINER, i);
        nodeList->push_back(n);
        printf("created MINER node %d\n", i);
    }
    for (unsigned int i = 0; i < num_relays; ++i) {
        Node* n = new Node(RELAY, num_miners + i);
        nodeList->push_back(n);
        printf("created RELAY node %d\n", num_miners + i);
    }

    // add links between nodes based on min_links_per_node and mean_link_speed
    for (vector<Node*>::iterator it = nodeList->begin(); it != nodeList->end(); ++it) {
        while ((*it)->get_num_links() < min_links_per_node) { // if more links are needed
            unsigned int node1 = (*it)->get_node_no();
            // find a node to link with
            unsigned int node2 = rand() % NUMBER_NODES;
            while (node1 == node2 || (*it)->linked_to(node2)) {
                node2 = rand() % NUMBER_NODES;
            }
            printf("linking node %d to node %d\n", node1, node2);
            add_link(*it, nodeList->at(node2), expon(mean_link_speed, STREAM_LINK_SPEED));
        }
    }

    // schedule the first transaction and first block to occur
    event_schedule(sim_time + expon(mean_tx_interarrival, STREAM_INTERARRIVAL_TIME), EVENT_NEW_TRANSACTION);
    event_schedule(sim_time + expon((mean_tx_interarrival * 10), STREAM_INTERARRIVAL_TIME), EVENT_NEW_BLOCK);
}

void new_transaction() {
    ++num_transactions;

    unsigned int random_index = rand() % NUMBER_NODES;

    printf("new_transaction() from %d\n", random_index);

    //TODO use a smart algorithm to determine fee
    Transaction* tx = new Transaction(num_transactions, 0.01);

    // let the network know about the transaction
    nodeList->at(random_index)->broadcast_transaction(tx);

    // schedule the next transaction
    event_schedule(sim_time + expon(mean_tx_interarrival, STREAM_INTERARRIVAL_TIME), EVENT_NEW_TRANSACTION);
}

void new_block() {
    printf("new_block()\n");

    ++num_blocks;

    unsigned int random_index = rand() % NUMBER_NODES;

    Block* b = new Block(num_blocks, nodeList->at(random_index)->get_known_transactions());

    // let the network know about the block
    nodeList->at(random_index)->broadcast_block(b);

    // schedule the next block
    event_schedule(sim_time + expon((mean_tx_interarrival * 10), STREAM_INTERARRIVAL_TIME), EVENT_NEW_BLOCK);
}

void tx_relay() {
    int tx_no = transfer[3];
    float tx_fee = transfer[4];
    int node_no = transfer[5];
    printf("tx_relay() of tx %d to node %d\n", tx_no, node_no);
    Transaction* tx = new Transaction(tx_no, tx_fee);
    nodeList->at(node_no)->broadcast_transaction(tx);
}

void block_relay() {
    printf("block_relay()\n");
    //TODO
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
