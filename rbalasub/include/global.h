#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdbool.h>

#define HOSTNAME_LEN 128

typedef struct node{
	unsigned int cost;
	int server_id;
	int port;
	bool neighbour;
	char *ip_addr;
}Node;

typedef struct env_{
	int num_servers;
	int num_neighbours;
	Node nodes[5];
} Environment;




extern Environment environment;

#endif
