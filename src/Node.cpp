
#include "Node.h"

Node::Node(Type type) {
    this->_type = type;
    this->_adjList = new vector<Link*>;
}

Node::~Node() {
    for (vector<Link*>::iterator it = this->_adjList->begin(); it != this->_adjList->end(); ++it) {
        delete *it;
    }
    delete this->_adjList;
}

void Node::add_link(Node* other_node, int speed) {
    Link* new_link = new Link(other_node, speed);
    this->_adjList->push_back(new_link);
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
