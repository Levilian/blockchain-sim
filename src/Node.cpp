
#include <algorithm>
#include "Node.h"
#include "simlib.h"
#include "blockchain-sim-defs.h"

Node::Node(Type type, unsigned int node_no) {
    this->_type = type;
    this->_node_no = node_no;
    this->_adj_list = new vector<Link*>;
    this->_known_transactions = new vector<Transaction>;
    this->_known_blocks = new vector<Block*>;
    this->_in_transit_tx_nos = new vector<unsigned int>;
    this->_in_transit_block_nos = new vector<unsigned int>;
}

Node::~Node() {
    for (vector<Link*>::iterator it = this->_adj_list->begin(); it != this->_adj_list->end(); ++it) {
        delete *it;
    }
    delete this->_adj_list;
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
    this->_adj_list->push_back(new_link);
}

bool Node::aware_of(Transaction tx) {
    bool already_known = false;
    for (vector<Transaction>::iterator it = this->_known_transactions->begin(); it != this->_known_transactions->end(); ++it) {
        if (it->get_tx_no() == tx.get_tx_no()) already_known = true;
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
    for (vector<Link*>::iterator it = this->_adj_list->begin(); it != this->_adj_list->end(); ++it) {
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
    for (vector<Link*>::iterator it = this->_adj_list->begin(); it != this->_adj_list->end(); ++it) {
        if (!((*it)->get_other_node()->aware_of(tx))) {
            #ifdef DEBUG
            printf("broadcasting tx %d from node %d to node %d\n",
                   tx.get_tx_no(), this->get_node_no(), (*it)->get_other_node()->get_node_no());
            #endif
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
    #ifdef DEBUG
    printf("number of known transactions before block propagation: %d\n", this->_known_transactions->size());
    #endif
    for (vector<Transaction>::iterator it = b->get_transactions()->begin(); it != b->get_transactions()->end(); ++it) {
        auto new_end = remove_if(this->_known_transactions->begin(), this->_known_transactions->end(),
                                 [&](Transaction  t) { return t.get_tx_no() == it->get_tx_no(); });
        this->_known_transactions->erase(new_end, this->_known_transactions->end());
    }
    #ifdef DEBUG
    printf("number of known transactions after block propagation: %d\n", this->_known_transactions->size());
    #endif

    // schedule events for neighboring nodes to be aware of it
    for (vector<Link*>::iterator it = this->_adj_list->begin(); it != this->_adj_list->end(); ++it) {
        if (!((*it)->get_other_node()->aware_of(b))) {
            #ifdef DEBUG
            printf("broadcasting block %d from node %d to node %d\n",
                           b->get_block_no(), this->get_node_no(), (*it)->get_other_node()->get_node_no());
            #endif
            transfer[3] = b->get_block_no();
            transfer[4] = this->get_node_no();
            transfer[5] = (*it)->get_other_node()->get_node_no();
            transfer[6] = b->get_block_time();
            transfer[7] = b->get_block_reward();
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

float Node::decide_tx_fee() {
    // get the avg time to confirmation over the course of the simulation
    sampst(0.0, -SAMPST_TTC);
    float overall_avg_ttc = transfer[1];

    // calculate avg time to confirmation and avg fee in the most recent block
    float avg_confirmation_time = 0;
    float avg_tx_fee = DEFAULT_FEE;
    if (this->_known_blocks->size() > 0) {
        float total_time_to_confirmation = 0;
        float total_tx_fees = 0;
        Block* b = this->_known_blocks->back();
        if (b->get_transactions()->size() == 0) {
            // if no transactions were confirmed, that's like an infinite time-to-confirmation
            avg_confirmation_time = overall_avg_ttc * 10;
        } else {
            for (vector<Transaction>::iterator it = b->get_transactions()->begin(); it != b->get_transactions()->end(); ++it) {
                total_time_to_confirmation += (it->get_confirmation_time() - it->get_broadcast_time());
                total_tx_fees += it->get_tx_fee();
            }
            avg_confirmation_time = total_time_to_confirmation / b->get_transactions()->size();
            avg_tx_fee = total_tx_fees / b->get_transactions()->size();
        }
    }
    #ifdef DEBUG
    printf("(fee) overall_avg_ttc: %f\n", overall_avg_ttc);
    printf("(fee) avg_confirmation_time: %f\n", avg_confirmation_time);
    printf("avg_tx_fee: %f\n", avg_tx_fee);
    #endif

    // the fee should be proportional to the amount of network congestion
    float tx_fee;
    if (overall_avg_ttc == 0 || avg_confirmation_time == 0) {
        tx_fee = DEFAULT_FEE;
    } else {
        tx_fee = avg_tx_fee * (avg_confirmation_time / overall_avg_ttc);
    }
    sampst(tx_fee, SAMPST_TX_FEE);
    return tx_fee;
}

vector<Transaction>* Node::decide_included_tx_list(float block_reward, float block_time) {
    // decide which transactions to include based on fees and block reward and greediness

    if (this->_greediness == 0 || this->_known_transactions->size() == 0) {
        #ifdef DEBUG
        printf("Included 0 transactions\n");
        #endif
        return new vector<Transaction>;
    }

    // sort transactions by fee
    sort(this->_known_transactions->begin(), this->_known_transactions->end(),
         [](Transaction t1, Transaction t2) { return t1.get_tx_fee() < t2.get_tx_fee(); });

    // we should be greedier with tx fees if the block reward is low
    float reward_factor = 1 - (block_reward / DEFAULT_BLOCK_REWARD);
    float greediness_delta = (100 - this->_greediness) * reward_factor;
    float real_greediness = this->_greediness + greediness_delta;

    // include high-fee transactions based on greediness
    int last_tx_index = (int)(((float)real_greediness / 100.0) * this->_known_transactions->size());
    vector<Transaction>* tx_list = new vector<Transaction>;
    for (int i = 0; i < last_tx_index; ++i) {
        Transaction t = this->_known_transactions->at(i);
        t.set_confirmation_time(block_time);
        float time_to_conf = block_time - t.get_broadcast_time();
        sampst(time_to_conf, SAMPST_TTC);
        tx_list->push_back(t);
    }
    #ifdef DEBUG
    printf("Included %d transactions of %d\n", tx_list->size(), this->_known_transactions->size());
    #endif
    return tx_list;
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
