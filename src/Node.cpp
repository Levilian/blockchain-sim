
#include "Node.h"
#include "simlib.h"
#include "blockchain-sim-defs.h"

Node::Node(Type type, unsigned int node_no) {
    this->_type = type;
    this->_node_no = node_no;
    this->_adjList = new vector<Link*>;
    this->_known_transactions = new vector<Transaction*>;
    this->_known_blocks = new vector<Block*>;
    this->_in_transit_tx_nos = new vector<unsigned int>;
}

Node::~Node() {
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        delete *it;
    }
    delete this->_adjList;
    for (vector<Transaction*>::iterator it = this->_known_transactions->begin(); it != this->_known_transactions->end(); ++it) {
        delete *it;
    }
    delete this->_known_transactions;
    for (vector<Block*>::iterator it = this->_known_blocks->begin(); it != this->_known_blocks->end(); ++it) {
        delete *it;
    }
    delete this->_known_blocks;
    delete this->_in_transit_tx_nos;
}

void Node::add_link(Node* other_node, float speed) {
    Link* new_link = new Link(other_node, speed);
    this->_adjList->push_back(new_link);
}

bool Node::aware_of(Transaction* tx) {
    bool already_known = false;
    for (vector<Transaction*>::iterator it = this->_known_transactions->begin(); it != this->_known_transactions->end(); ++it) {
        if ((*it)->get_tx_no() == tx->get_tx_no()) already_known = true;
    }
    for (vector<unsigned int>::iterator it = this->_in_transit_tx_nos->begin(); it != this->_in_transit_tx_nos->end(); ++it) {
        if (*it == tx->get_tx_no()) already_known = true;
    }
    return already_known;
}

bool Node::aware_of(Block* b) {
    bool already_known = false;
    for (vector<Block*>::iterator it = this->_known_blocks->begin(); it != this->_known_blocks->end(); ++it) {
        if ((*it)->get_block_no() == b->get_block_no()) already_known = true;
    }
    return already_known;
}

bool Node::linked_to(unsigned int node_no) {
    bool linked = false;
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        if ((*it)->get_other_node()->get_node_no() == node_no) linked = true;
    }
    return linked;
}

void Node::in_transit_tx(unsigned int tx_no) {
    this->_in_transit_tx_nos->push_back(tx_no);
}

void Node::broadcast_transaction(Transaction* tx) {
    // add it to our list of transactions
    this->_known_transactions->push_back(tx);

    // schedule events for neighboring nodes to be aware of it
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        if (!((*it)->get_other_node()->aware_of(tx))) {
            printf("broadcasting tx %d from node %d to node %d\n", tx->get_tx_no(), this->get_node_no(), (*it)->get_other_node()->get_node_no());
            transfer[3] = tx->get_tx_no();
            transfer[4] = tx->get_tx_fee();
            transfer[5] = (*it)->get_other_node()->get_node_no();
            event_schedule(sim_time + (*it)->get_speed(), EVENT_TX_RELAY);
            // record that it's in transit so it isn't broadcast again before it arrives
            (*it)->get_other_node()->in_transit_tx(tx->get_tx_no());
        }
    }
}

void Node::broadcast_block(Block* b) {
    //TODO check if we already know about the block
    //TODO remove tx from known_transactions
    //TODO let other nodes know about it
}

ostream& operator<<(ostream& os, const Node& n) {
    os << "Node type: ";
    if (n.GetType() == RELAY) {
        os << "RELAY";
    } else if (n.GetType() == MINER) {
        os << "MINER";
    }
    return os;
}
