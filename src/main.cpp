#include "Node.h"
#include <iostream>
#include <vector>

using namespace std;

void add_link(Node* node1, Node* node2, int speed);

int main() {
    vector<Node*> nodeList;
    Node* n1 = new Node(MINER);
    nodeList.push_back(n1);
    Node* n2 = new Node(RELAY);
    nodeList.push_back(n2);
    for (vector<Node*>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
        cout << **it << endl;
    }
    add_link(n1, n2, 10);
    // free memory and exit
    for (vector<Node*>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
        delete *it;
    }
    return 0;
}

void add_link(Node* node1, Node* node2, int speed) {
    node1->add_link(node2, speed);
    node2->add_link(node1, speed);
}
