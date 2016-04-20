
// This class represents a server running blockchain software 
// that relays transactions or mines blocks.

#pragma once

#include <iostream>
#include <vector>

using namespace std;

class Node;

enum Type { RELAY, MINER };

typedef struct Link {
    public:
        Link(Node* other_node, int speed) {
            _other_node = other_node;
            _speed = speed;
        }
    private:
        Node* _other_node;
        int _speed;
} Link;

class Node {
    public:
        Node(Type type);
        ~Node();
        Type GetType() const { return _type; }
        void add_link(Node* otherNode, int speed);
    private:
        Type _type;
        vector<Link*>* _adjList;
        friend ostream& operator<<(ostream& os, const Node& n);
};

