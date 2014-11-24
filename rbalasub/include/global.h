#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HOSTNAME_LEN 128

//[PA3] Update Packet Start
//[PA3] Routing Table Start
typedef struct node_{
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
	bool started;
} Node;
//[PA3] Routing Table End

typedef struct env_{
	int num_servers;
	int num_neighbours;
	Node nodes[5];
} Environment;
//[PA3] Update Packet End

/******* For printing *********/
typedef struct pkt_node_
{
    uint16_t server_id;
    uint16_t cost;
}Pkt_node;

extern Environment environment;
extern uint16_t self_port;
extern uint16_t self_id;
extern uint32_t self_ip;
extern char * self_ip_str;
extern int listening_socket;
extern int packet_count;

#endif
