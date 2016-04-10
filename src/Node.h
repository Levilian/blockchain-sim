
// This class represents a server running blockchain software 
// that relays transactions or mines blocks.

#include <iostream>

using namespace std;

enum Type { RELAY, MINER };

class Node {
    public:
        Node(Type type) { _type = type; }
        ~Node() {}
        Type GetType() const { return _type; }
   private:
        Type _type;
};

ostream& operator<<(ostream& os, const Node& n) {
    os << "Node type: ";
    if (n.GetType() == RELAY) {
        os << "RELAY";
    } else if (n.GetType() == MINER) {
        os << "MINER";
    }
    return os;
}
