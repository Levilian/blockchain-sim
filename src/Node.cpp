
#include <algorithm>
#include "Node.h"
#include "simlib.h"
#include "blockchain-sim-defs.h"

Node::Node(Type type, unsigned int node_no) {
    this->_type = type;
    this->_node_no = node_no;
    this->_adjList = new vector<Link*>;
    this->_known_transactions = new vector<Transaction>;
    this->_known_blocks = new vector<Block*>;
    this->_in_transit_tx_nos = new vector<unsigned int>;
    this->_in_transit_block_nos = new vector<unsigned int>;
}

Node::~Node() {
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        delete *it;
    }
    delete this->_adjList;
    delete this->_known_transactions;
    for (vector<Block*>::iterator it = this->_known_blocks->begin(); it != this->_known_blocks->end(); ++it) {
        delete (*it)->get_transactions();
        delete *it;
    }
    delete this->_known_blocks;
    delete this->_in_transit_tx_nos;
    delete this->_in_transit_block_nos;
}

void Node::add_link(Node* other_node, float speed) {
    Link* new_link = new Link(other_node, speed);
    this->_adjList->push_back(new_link);
}

bool Node::aware_of(Transaction tx) {
    bool already_known = false;
    for (vector<Transaction>::iterator it = this->_known_transactions->begin(); it != this->_known_transactions->end(); ++it) {
        if ((*it).get_tx_no() == tx.get_tx_no()) already_known = true;
    }
    for (vector<unsigned int>::iterator it = this->_in_transit_tx_nos->begin(); it != this->_in_transit_tx_nos->end(); ++it) {
        if (*it == tx.get_tx_no()) already_known = true;
    }
    return already_known;
}

bool Node::aware_of(Block* b) {
    bool already_known = false;
    for (vector<Block*>::iterator it = this->_known_blocks->begin(); it != this->_known_blocks->end(); ++it) {
        if ((*it)->get_block_no() == b->get_block_no()) already_known = true;
    }
    for (vector<unsigned int>::iterator it = this->_in_transit_block_nos->begin(); it != this->_in_transit_block_nos->end(); ++it) {
        if (*it == b->get_block_no()) already_known = true;
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

void Node::in_transit_block(unsigned int block_no) {
    this->_in_transit_block_nos->push_back(block_no);
}

void Node::broadcast_transaction(Transaction tx) {
    // add it to our list of transactions
    this->_known_transactions->push_back(tx);

    // remove it from the list of in transit transactions
    auto new_end = remove_if(this->_in_transit_tx_nos->begin(), this->_in_transit_tx_nos->end(),
                             [&](unsigned int tx_no) { return tx_no == tx.get_tx_no(); });
    this->_in_transit_tx_nos->erase(new_end, this->_in_transit_tx_nos->end());

    // schedule events for neighboring nodes to be aware of it
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        if (!((*it)->get_other_node()->aware_of(tx))) {
            printf("broadcasting tx %d from node %d to node %d\n",
                   tx.get_tx_no(), this->get_node_no(), (*it)->get_other_node()->get_node_no());
            transfer[3] = tx.get_tx_no();
            transfer[4] = tx.get_tx_fee();
            transfer[5] = (*it)->get_other_node()->get_node_no();
            transfer[6] = tx.get_broadcast_time();
            event_schedule(sim_time + (*it)->get_speed(), EVENT_TX_RELAY);
            // record that it's in transit so it isn't broadcast again before it arrives
            (*it)->get_other_node()->in_transit_tx(tx.get_tx_no());
        }
    }
}

void Node::broadcast_block(Block* b) {
    // add it to our list of blocks
    this->_known_blocks->push_back(b);

    // remove it from the list of in transit blocks
    auto new_end = remove_if(this->_in_transit_block_nos->begin(), this->_in_transit_block_nos->end(),
                             [&](unsigned int block_no) { return block_no == b->get_block_no(); });
    this->_in_transit_block_nos->erase(new_end, this->_in_transit_block_nos->end());

    // remove transactions from _known_transactions that were included in the block
    printf("number of known transactions before block propagation: %d\n", this->_known_transactions->size());
    for (vector<Transaction>::iterator it = b->get_transactions()->begin(); it != b->get_transactions()->end(); ++it) {
        auto new_end = remove_if(this->_known_transactions->begin(), this->_known_transactions->end(),
                                 [&](Transaction  t) { return t.get_tx_no() == (*it).get_tx_no(); });
        this->_known_transactions->erase(new_end, this->_known_transactions->end());
    }
    printf("number of known transactions after block propagation: %d\n", this->_known_transactions->size());

    // schedule events for neighboring nodes to be aware of it
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        if (!((*it)->get_other_node()->aware_of(b))) {
            printf("broadcasting block %d from node %d to node %d\n",
                   b->get_block_no(), this->get_node_no(), (*it)->get_other_node()->get_node_no());
            transfer[3] = b->get_block_no();
            transfer[4] = this->get_node_no();
            transfer[5] = (*it)->get_other_node()->get_node_no();
            transfer[6] = b->get_block_time();
            event_schedule(sim_time + (2 * (*it)->get_speed()), EVENT_BLOCK_RELAY);
            // record that it's in transit so it isn't broadcast again before it arrives
            (*it)->get_other_node()->in_transit_block(b->get_block_no());
        }
    }
}

vector<Transaction>* Node::get_block_transactions(unsigned int block_no) {
    for (vector<Block*>::iterator it = this->_known_blocks->begin(); it != this->_known_blocks->end(); ++it) {
        if ((*it)->get_block_no() == block_no) return new vector<Transaction>(*(*it)->get_transactions());
    }
    return NULL; // control should never reach here
}

ostream& operator<<(ostream& os, const Node& n) {
    os << "Node type: ";
    if (n.get_type() == RELAY) {
        os << "RELAY";
    } else if (n.get_type() == MINER) {
        os << "MINER";
    }
    return os;
}
