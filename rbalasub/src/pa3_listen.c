#include "../include/pa3_listen.h"
#include "../include/global.h"
#include "../include/pa3_commands.h"

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

int self_port;
int self_id;

/**
 * Start listening for UDP packets and multiplex with
 * stdin using select()
 * @param timeout timeout value taken from user as a float
 */
void start_listening(float timeout){
	fd_set master; //master file desc set
	fd_set read_fds; //temp for select()
	int fd_max; //tracking maximum fd_set
	int listening_socket;
    // socklen_t addrlen;  //length of address
    int yes = 1;
    int errorVar; //For reporting error with gai_strerror

    struct addrinfo hints, *receive_ai, *working_ai;

	FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /******* Get i/p for socket creation *********/
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    char port[5];
    sprintf(port, "%d",self_port);
    /******* Get addrinfo *********/
    if ((errorVar = getaddrinfo(NULL, port, &hints, &receive_ai))!=0)
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
            close(listening_socket);
            continue;
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
    /******* Main listening loop *********/
    while(true){
		read_fds = master; //going to select();
		fflush(stdout);
		struct timeval tv;
		tv.tv_sec = (time_t)timeout;
		tv.tv_usec = (time_t)((timeout - tv.tv_sec)*1000000);
		int res = select(fd_max+1, &read_fds, NULL, NULL, &tv);
		if (res == -1)
		{
			perror("select");
			exit(EXIT_FAILURE);
		}else if(res == 0){

			printf("Timeout\n");
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

				}
			}
		}
	}
}