//
// Created by istoffa on 11/23/16.
//

#include <map>
#include <string>

#ifndef DISCRETE_SIMS_NETWORK_H
#define DISCRETE_SIMS_NETWORK_H

using namespace std;

class Network {
	//Network will hold all the nodes and strings
	vector<class Node> nodes;
	multimap<string, class Edge> edges;

public:
	Network();
};

class Node;

class Edge {
	string street;
	class Node* v[2];
	void* data;  //Facilities

public:
	Edge(class node* n1, class node n2, void *data = NULL);
};

class Node {
	void* data; //Facilities
	Node(void *data = NULL);
};


#endif //DISCRETE_SIMS_NETWORK_H
