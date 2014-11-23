#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HOSTNAME_LEN 128

typedef struct node{
	uint16_t cost;
	uint16_t server_id;
	uint16_t port;
	bool neighbour;
	int16_t next_hop_server_id; 
	uint32_t ip_addr_bin;
	char *ip_addr;
	bool enabled;
	int socket;
	int timeout_counter;
	int reset_timeout;
} Node;

typedef struct env_{
	int num_servers;
	int num_neighbours;
	Node nodes[5];
} Environment;




extern Environment environment;
extern uint16_t self_port;
extern uint16_t self_id;
extern uint32_t self_ip;
extern char * self_ip_str;
extern int listening_socket;

#endif
