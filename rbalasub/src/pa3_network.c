#include "../include/pa3_network.h"
#include "../include/global.h"
#include "../include/pa3_application.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

uint16_t self_port;
uint32_t self_ip;


void broadcast_packet(){
	for (int i = 0; i < environment.num_servers; ++i)
	{
		Node node = environment.nodes[i];
		if (node.neighbour == true && node.enabled == true)
		{
			 /******* Get addrinfo of destination - Begin *********/
			struct addrinfo hints, *res, *dest_addr_info ;
			int sockfd;
			int status;
			memset (&hints, 0,sizeof hints);
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_DGRAM;
			char port_str[5];
			sprintf(port_str,"%d",node.port);
			if ((status = getaddrinfo(node.ip_addr, port_str, &hints, &res)) != 0)
			{
				fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(status));
				return ;
			}


			for (dest_addr_info = res; dest_addr_info != NULL; dest_addr_info = dest_addr_info->ai_next)
			{
		        if (dest_addr_info->ai_family==AF_INET) //break at the first found IPv4 address
		        {
		        	if (node.socket > environment.num_servers) //If socket not made
		        	{
		        		/******* sockaddr in dest_addr_info *********/
		        		if ((sockfd = socket(dest_addr_info->ai_family, dest_addr_info->ai_socktype,
		        			dest_addr_info->ai_protocol)) == -1) {
		        			perror("socket");
		        			continue;
		        		}
		        		printf("socket opened: %d",node.socket);
			        	node.socket = sockfd;
		        	}
		        	break;
			    }
			}
			
			void *pkt = make_pkt();
			size_t pkt_size = 8 + 12*environment.num_servers;

			int numbytes;
			if ((numbytes = sendto(node.socket, pkt, pkt_size, 0,
            	dest_addr_info->ai_addr, dest_addr_info->ai_addrlen)) == -1) {
		        perror("send: sendto");
		    }

		    if(close(node.socket) < 0){
		    	perror("close socket");
		    };
		    
		}//if
	}//main for
}

void * make_pkt(){
	/******* Calculate size of the packet *********/
    size_t pkt_size = 8 + 12*environment.num_servers;
    char *packet = (char *)calloc(1,pkt_size);

    /******* Covert to network byte order  *********/

    /******* Copy number of servers *********/
    uint16_t pkt_num_serv = htons(environment.num_servers);
    memcpy(packet, &(pkt_num_serv), 2);

    /******* Copy own server id *********/
    uint16_t pkt_self_id = htons(self_port);
    memcpy(packet+2, &(pkt_self_id), 2);

    /******* Copy each entry *********/
    memcpy(packet+4, &(self_ip), 4);
    for (int i = 0; i < environment.num_servers; ++i)
    {

        /******* No need to convert to network byte order. Already done by inet_pton *********/
        /******* Copy node ip address *********/
        memcpy(packet+8+(i*12), &(environment.nodes[i].ip_addr_bin), 4);
        
        /******* Copy node port *********/
        uint16_t pkt_node_port = htons(environment.nodes[i].port);
        memcpy(packet+12+(i*12), &(pkt_node_port), 2);

        /******* Copy node server id *********/
        uint16_t pkt_node_sid = htons(environment.nodes[i].server_id);
        memcpy(packet+16+(i*12), &(pkt_node_sid), 2);

        /******* Copy link cost *********/
        uint16_t pkt_node_cost = htons(environment.nodes[i].cost);
        memcpy(packet+18+(i*12), &(pkt_node_cost), 2);
    }
    return packet;
}

/**
 * Close all sockets
 */
void close_all(){

	for (int i = 0; i < environment.num_servers; ++i)
	{
		close(environment.nodes[i].socket);
	}
	close(listening_socket);
}