#include "../include/pa3_listen.h"
#include "../include/global.h"
#include "../include/pa3_commands.h"
#include "../include/pa3_network.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>

uint16_t self_port;
uint16_t self_id;
char *self_ip_str;
int listening_socket;
/**
 * Start listening for UDP packets and multiplex with
 * stdin using select()
 * @param timeout timeout value taken from user as a float
 */
void start_listening(float timeout){
	fd_set master; //master file desc set
	fd_set read_fds; //temp for select()
	int fd_max; //tracking maximum fd_set
    // socklen_t addrlen;  //length of address
    int yes = 1;
    int errorVar; //For reporting error with gai_strerror

    struct addrinfo hints, *receive_ai, *working_ai;

	FD_ZERO(&master);
    FD_ZERO(&read_fds);

    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    char port[5];
    sprintf(port, "%d",self_port);
    /******* Get addrinfo *********/
    if ((errorVar = getaddrinfo(self_ip_str, port, &hints, &receive_ai))!=0)
    {
        fprintf(stderr, "Get addr info: %s\n", gai_strerror(errorVar));
        exit(EXIT_FAILURE);
    }

    /******* Loop through addrinfos for binding a socket *********/
    for (working_ai = receive_ai; receive_ai!=NULL; receive_ai = receive_ai->ai_next)
    {

        /******* Create the socket if not continue the loop looking for other*********/
        listening_socket =  socket(working_ai->ai_family,working_ai->ai_socktype, 0);
        if (listening_socket<0)
        {
            continue;
        }

        /******* Make socket resuable *********/
        setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        /******* Bind else use another socket *********/
        if (bind(listening_socket, working_ai->ai_addr, working_ai->ai_addrlen)<0)
        {
            perror("bind");
            close(listening_socket);
            continue;
        }
        if (working_ai != NULL)
        {
           // char ip_str[INET_ADDRSTRLEN];
           // struct sockaddr_in *dest_sock_addr = (struct sockaddr_in *)working_ai->ai_addr;
           // void * temp_addr = &(dest_sock_addr->sin_addr);
           // inet_ntop(AF_INET, &temp_addr, ip_str, sizeof ip_str);
           // printf("Listening at : %s\n",ip_str);
        }
 
        break;

    }

    /******* If something went wrong  *********/
    if (working_ai == NULL)
    {
        fprintf(stderr, "Failed to bind\n");
        freeaddrinfo(receive_ai);

        exit(EXIT_FAILURE);
    }

    FD_SET(0,&master); // stdin for select()
    FD_SET(listening_socket, &master); //for listening for connections
    fd_max = listening_socket;
	

    /******* Free the linked list *********/
    freeaddrinfo(receive_ai);
    /******* Print prompt *********/
    printf("[PA3]> ");

    /******* Timeout Value *********/
    struct timeval tv;
    tv.tv_sec = (time_t)timeout;
    tv.tv_usec = (time_t)((timeout - tv.tv_sec)*1000000);
    /******* Main listening loop *********/
    while(true){

		read_fds = master; //going to select();
		fflush(stdout);
		int res = select(fd_max+1, &read_fds, NULL, NULL, &tv);
		if (res == -1)
		{
			perror("select");
			exit(EXIT_FAILURE);
		}else if(res == 0){
			printf("\nPeriodic update..\n\n");
            printf("[PA3]> ");

            tv.tv_sec = (time_t)timeout;
            tv.tv_usec = (time_t)((timeout - tv.tv_sec)*1000000);
            for (int i = 0; i < environment.num_servers; ++i)
            {
                if (environment.nodes[i].server_id == self_id 
                    || environment.nodes[i].neighbour == false)
                {
                    continue;
                }
                if (environment.nodes[i].reset_timeout == false)
                {
                    environment.nodes[i].timeout_counter++;
                    // debug()
                    if (environment.nodes[i].timeout_counter >=3)
                    {
                        printf("\nLink with %u disabled\n",environment.nodes[i].server_id);
                        char *error_str;
                        disable_link(environment.nodes[i].server_id, &error_str);
                    }
                }else{
                    if (environment.nodes[i].started == true)
                    {
                        environment.nodes[i].reset_timeout = false; // Reset all marking                       
                    }
                }
                
            }
            broadcast_packet();

            
		}

		for (int i = 0; i <= fd_max; i++){
			if (FD_ISSET(i, &read_fds)){
				if (i==0)
				{
					/******* Read from stdin *********/
					char commandString[512];
					fgets(commandString, sizeof(commandString), stdin);
					handle_commands(commandString);
					printf("[PA3]> ");
				}else{
                    /******* Read from socket *********/
                    size_t pkt_size = 8 + 12*environment.num_servers;
                    char pkt[pkt_size];
                    int recv_len = recvfrom(i, &pkt, pkt_size, 0, NULL, NULL);
                    if (recv_len<0)
                    {
                        perror("recvfrom");
                    }else{
                        uint16_t server_id = read_pkt_update(pkt);
                        packet_count++;//increment packet count for 'packets' comand
                        int index = get_node(server_id);
                        if (index != INT_MAX)
                        {
                            /******* Mark link as active *********/
                            environment.nodes[index].started = true;
                            environment.nodes[index].reset_timeout = true;
                            environment.nodes[index].timeout_counter = 0;
                        }

                    }

				}
			}
		}



	}
}

