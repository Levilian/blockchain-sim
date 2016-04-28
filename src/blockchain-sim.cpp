
// The code below simulates a P2P network similar to Bitcoin.

#include "Node.h"
#include "simlib.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <unordered_set>
#include "blockchain-sim-defs.h"

using namespace std;

int num_blocks, num_transactions, min_links_per_node;
float mean_tx_interarrival, mean_block_interarrival, mean_link_speed;
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
    fscanf(infile, "%d %f %f %f", &min_links_per_node, &mean_tx_interarrival,
                                  &mean_block_interarrival, &mean_link_speed);
    fclose(infile);

    // Write report heading with input parameters.
    fprintf(outfile, "Mean interarrival time for transactions: %.3f\n", mean_tx_interarrival);
    fprintf(outfile, "Mean interarrival time for blocks: %.3f\n", mean_block_interarrival);
    fprintf(outfile, "Mean link speed: %.3f\n", mean_link_speed);
    fprintf(outfile, "Min links per node: %d\n", min_links_per_node);

    // initialize simlib
    init_simlib();

    // set number of attributes per record
    maxatr = 7;

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

    //modified code with get random data from /dev/urandom instead of time
    int from_urandom;
    FILE* fp;
    fp = fopen("/dev/urandom", "r");
    if (fp != NULL) {
      fread(&from_urandom, 1, sizeof(int), fp);
      srand(from_urandom);
      fread(&from_urandom, 1, sizeof(int), fp);
      lcgrandst(from_urandom, STREAM_TX_INTERARRIVAL);
      fread(&from_urandom, 1, sizeof(int), fp);
      lcgrandst(from_urandom, STREAM_BLOCK_INTERARRIVAL);
      fread(&from_urandom, 1, sizeof(int), fp);
      lcgrandst(from_urandom, STREAM_LINK_SPEED);
      
      fclose(fp);
    } else { //fall back on time if /dev/urandom fails for some reason
      srand(time(NULL) / 2);
      lcgrandst((time(NULL) + getpid()), STREAM_TX_INTERARRIVAL);
      lcgrandst((time(NULL) / 4), STREAM_BLOCK_INTERARRIVAL);
      lcgrandst((time(NULL) - getpid()), STREAM_LINK_SPEED);
    }

    // add nodes to the nodeList
    unsigned int num_miners = MINER_FRACTION * NUMBER_NODES;
    unsigned int num_relays = NUMBER_NODES - num_miners;
    for (unsigned int i = 0; i < num_miners; ++i) {
        Node* n = new Node(MINER, i);
        n->set_greediness(rand() % 100 + 1);
        nodeList->push_back(n);
        #ifdef DEBUG
        printf("created MINER node %d\n", i);
        #endif
    }
    for (unsigned int i = 0; i < num_relays; ++i) {
        Node* n = new Node(RELAY, num_miners + i);
        nodeList->push_back(n);
        #ifdef DEBUG
        printf("created RELAY node %d\n", num_miners + i);
        #endif
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
            #ifdef DEBUG
            printf("linking node %d to node %d\n", node1, node2);
            #endif
            add_link(*it, nodeList->at(node2), expon(mean_link_speed, STREAM_LINK_SPEED));
        }
    }

    // schedule the first transaction and first block to occur
    event_schedule(sim_time + expon(mean_tx_interarrival, STREAM_TX_INTERARRIVAL), EVENT_NEW_TRANSACTION);
    event_schedule(sim_time + expon(mean_block_interarrival, STREAM_BLOCK_INTERARRIVAL), EVENT_NEW_BLOCK);
}

void new_transaction() {
    ++num_transactions;

    unsigned int random_index = rand() % NUMBER_NODES;

    // the node should decide the tx fee
    float tx_fee = nodeList->at(random_index)->decide_tx_fee();

    #ifdef DEBUG
    printf("new_transaction() %d from %d with fee %f at t=%f\n", num_transactions, random_index, tx_fee, sim_time);
    #endif

    Transaction tx = Transaction(num_transactions, tx_fee, sim_time);

    // let the network know about the transaction
    nodeList->at(random_index)->broadcast_transaction(tx);

    // schedule the next transaction
    event_schedule(sim_time + expon(mean_tx_interarrival, STREAM_TX_INTERARRIVAL), EVENT_NEW_TRANSACTION);
}

void new_block() {
    ++num_blocks;

    unsigned int random_index = rand() % NUMBER_NODES;
    while (!(nodeList->at(random_index)->get_type() == MINER)) {
        random_index = rand() % NUMBER_NODES;
    }

    #ifdef DEBUG
    printf("new_block() from %d\n", random_index);
    #endif

    float block_time = sim_time;

    int number_of_reward_changes = num_blocks / BLOCKS_BETWEEN_REWARD_CHANGES;
    float block_reward = DEFAULT_BLOCK_REWARD / pow(2, number_of_reward_changes);

    vector<Transaction>* tx_list = nodeList->at(random_index)->decide_included_tx_list(block_reward, block_time);

    Block* b = new Block(num_blocks, tx_list, block_time, block_reward);

    // let the network know about the block
    nodeList->at(random_index)->broadcast_block(b);

    // schedule the next block
    event_schedule(sim_time + expon(mean_block_interarrival, STREAM_BLOCK_INTERARRIVAL), EVENT_NEW_BLOCK);
}

void tx_relay() {
    unsigned int tx_no = transfer[3];
    float tx_fee = transfer[4];
    unsigned int node_no = transfer[5];
    float broadcast_time = transfer[6];
    #ifdef DEBUG
    printf("tx_relay() of tx %d to node %d\n", tx_no, node_no);
    #endif
    Transaction tx = Transaction(tx_no, tx_fee, broadcast_time);
    nodeList->at(node_no)->broadcast_transaction(tx);
}

void block_relay() {
    unsigned int block_no = transfer[3];
    unsigned int from_node = transfer[4];
    unsigned int to_node = transfer[5];
    float block_time = transfer[6];
    float block_reward = transfer[7];
    #ifdef DEBUG
    printf("block_relay() of block %d from node %d to node %d\n", block_no, from_node, to_node);
    #endif
    vector<Transaction>* transactions = nodeList->at(from_node)->get_block_transactions(block_no);
    Block* b = new Block(block_no, transactions, block_time, block_reward);
    nodeList->at(to_node)->broadcast_block(b);
}

void report() {
    printf("Number of blocks: %d\n", num_blocks);
    printf("Number of transactions: %d\n", num_transactions);
    sampst(0.0, -SAMPST_TTC);
    printf("Avg time-to-confirmation: %f\n", transfer[1]);
    sampst(0.0, -SAMPST_TX_FEE);
    printf("Avg tx fee: %f\n", transfer[1]);
    // find number of confirmed and uncomfirmed transactions
    unordered_set<unsigned int> confirmed_tx_nos, known_tx_nos;
    for (vector<Node*>::iterator it = nodeList->begin(); it != nodeList->end(); ++it) {
        vector<Transaction>* tx_list = (*it)->get_known_transactions();
        for (vector<Transaction>::iterator it2 = tx_list->begin(); it2 != tx_list->end(); ++it2) {
            known_tx_nos.insert(it2->get_tx_no());
        }
        vector<Block*>* block_list = (*it)->get_known_blocks();
        for (vector<Block*>::iterator it3 = block_list->begin(); it3 != block_list->end(); ++it3) {
            vector<Transaction>* block_tx_list = (*it3)->get_transactions();
            for (vector<Transaction>::iterator it4 = block_tx_list->begin(); it4 != block_tx_list->end(); ++it4) {
                confirmed_tx_nos.insert(it4->get_tx_no());
                known_tx_nos.insert(it4->get_tx_no());
            }
        }
    }
    printf("%% confirmed transactions: %f\n", ((float)confirmed_tx_nos.size() / (float)known_tx_nos.size()));
    //TODO print rest of report
}

void add_link(Node* node1, Node* node2, float speed) {
    node1->add_link(node2, speed);
    node2->add_link(node1, speed);
}
