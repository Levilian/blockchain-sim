
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
        Transaction(unsigned int tx_no, float tx_fee) {
            _tx_no = tx_no;
            _tx_fee = tx_fee;
        }
        unsigned int get_tx_no() { return _tx_no; }
        float get_tx_fee() { return _tx_fee; }
    private:
        unsigned int _tx_no;
        float _tx_fee;
} Transaction;

typedef struct Block {
    public:
        Block(unsigned int block_no, vector<Transaction*>* transactions) {
            _block_no = block_no;
            _transactions = transactions;
        }
        unsigned int get_block_no() { return _block_no; }
        vector<Transaction*>* get_transactions() { return _transactions; }
    private:
        unsigned int _block_no;
        vector<Transaction*>* _transactions;
} Block;

class Node {
    public:
        Node(Type type, unsigned int node_no);
        ~Node();
        Type GetType() const { return _type; }
        void add_link(Node* otherNode, float speed);
        unsigned int get_num_links() { return _adjList->size(); }
        void in_transit_tx(unsigned int tx_no);
        void broadcast_transaction(Transaction* tx);
        void broadcast_block(Block* b);
        unsigned int get_node_no() { return _node_no; }
        vector<Transaction*>* get_known_transactions() { return _known_transactions; }
        vector<Block*>* get_known_blocks() { return _known_blocks; }
        bool aware_of(Transaction* tx);
        bool aware_of(Block* b);
        bool linked_to(unsigned int node_no);
    private:
        friend ostream& operator<<(ostream& os, const Node& n);
        Type _type;
        vector<Link*>* _adjList;
        vector<Transaction*>* _known_transactions;
        vector<Block*>* _known_blocks;
        vector<unsigned int>* _in_transit_tx_nos;
        unsigned int _node_no;
};

