
// This class represents a server running blockchain software 
// that relays transactions or mines blocks.

#include <iostream>
#include <vector>

using namespace std;

class Node;

enum Type { RELAY, MINER };

typedef struct Link {
    public:
        Link(Node* other_node, float speed) {
            _other_node = other_node;
            _speed = speed;
        }
        Node* get_other_node() { return _other_node; }
        float get_speed() { return _speed; }
    private:
        Node* _other_node;
        float _speed;
} Link;

typedef struct Transaction {
    public:
        Transaction(unsigned int tx_no, float tx_fee, float broadcast_time) {
            _tx_no = tx_no;
            _tx_fee = tx_fee;
            _broadcast_time = broadcast_time;
        }
        unsigned int get_tx_no() { return _tx_no; }
        float get_tx_fee() { return _tx_fee; }
        float get_broadcast_time() { return _broadcast_time; }
        float get_confirmation_time() { return _confirmation_time; }
        void set_confirmation_time(float conf_time) { _confirmation_time = conf_time; }
    private:
        unsigned int _tx_no;
        float _tx_fee;
        float _broadcast_time;
        float _confirmation_time;
} Transaction;

typedef struct Block {
    public:
        Block(unsigned int block_no, vector<Transaction>* transactions, float block_time, float block_reward) {
            _block_no = block_no;
            _transactions = transactions;
            _block_time = block_time;
            _block_reward = block_reward;
        }
        unsigned int get_block_no() { return _block_no; }
        vector<Transaction>* get_transactions() { return _transactions; }
        float get_block_time() { return _block_time; }
        float get_block_reward() { return _block_reward; }
    private:
        unsigned int _block_no;
        vector<Transaction>* _transactions;
        float _block_time;
        float _block_reward;
} Block;

class Node {
    public:
        Node(Type type, unsigned int node_no);
        ~Node();
        void set_greediness(int greediness) { _greediness = greediness; }
        int get_greediness() { return _greediness; }
        Type get_type() const { return _type; }
        void add_link(Node* otherNode, float speed);
        unsigned int get_num_links() { return _adj_list->size(); }
        void in_transit_tx(unsigned int tx_no);
        void in_transit_block(unsigned int block_no);
        void broadcast_transaction(Transaction tx);
        void broadcast_block(Block* b);
        unsigned int get_node_no() { return _node_no; }
        vector<Transaction>* get_known_transactions() { return new vector<Transaction>(*_known_transactions); }
        vector<Block*>* get_known_blocks() { return new vector<Block*>(*_known_blocks); }
        bool aware_of(Transaction tx);
        bool aware_of(Block* b);
        bool linked_to(unsigned int node_no);
        vector<Transaction>* get_block_transactions(unsigned int block_no);
        float decide_tx_fee();
        vector<Transaction>* decide_included_tx_list(float block_reward, float block_time);
    private:
        friend ostream& operator<<(ostream& os, const Node& n);
        Type _type;
        vector<Link*>* _adj_list;
        vector<Transaction>* _known_transactions;
        vector<Block*>* _known_blocks;
        vector<unsigned int>* _in_transit_tx_nos;
        vector<unsigned int>* _in_transit_block_nos;
        unsigned int _node_no;
        int _greediness;
};

