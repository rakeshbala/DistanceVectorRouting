#include "../include/pa3_application.h"
#include "../include/global.h"
#include "../include/pa3_listen.h"
#include "../include/pa3_bf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <arpa/inet.h>

/******* Globals *********/
Environment environment;
Node *self_node;


void setupEvironment(int index, char * line);
/**
 * The main entry point of the application
 * @param top_file_path Path to topology file
 * @param timeout       Update interval
 */	
void start_run_loop(char *top_file_path, float timeout)
{

	/******* Look for topology file *********/
	FILE *topology_file = fopen(top_file_path,"r");
	if (topology_file != NULL )
	{
		size_t size = 30;
		char *line= (char *)malloc(size);
		int index = 0;
		/******* Parse each line of topology file*********/
		while(getline(&line,&size,topology_file) != -1){
			/******* Setup environment *********/
			setupEvironment(index,line);
			index++;
		}

		free(line);
		fclose(topology_file);
	}else{

		perror(top_file_path);
		exit(EXIT_FAILURE);
	}

	//Run BF for the first time
	run_BF();
	//Start listening to updates
	start_listening(timeout);

}



/**
 * Setup the environment variables for each line in topology file
 * @param index index of line being read
 * @param line  line being read
 */
void setupEvironment(int index , char *line)
{	
	if (strlen(line)<=1)
	{
		return;
	}
	if(index == 0){
		environment.num_servers = atoi(line);
	}else if(index == 1){
		environment.num_neighbours = atoi(line);
	}else{
		/******* Set default state of all nodes in the system *********/
		if (index <= 1+environment.num_servers){
			char *split;
			int count=0;
			char **split_array = (char **)calloc(3, sizeof (char*));
			split = strtok(line, " ");
			while(split){
				split_array[count] = strdup(split);
				count++;
				split = strtok(NULL," ");
			}
			Node node;
			node.server_id = atoi(split_array[0]);
			struct in_addr ipaddress;
			node.ip_addr = strdup(split_array[1]);
			inet_pton(AF_INET, split_array[1], &ipaddress);
			node.ip_addr_bin = ipaddress.s_addr;
			node.port = (uint16_t)strtoul(split_array[2],NULL,0);
			node.neighbour = false;
			node.cost = USHRT_MAX;
			node.next_hop_server_id = -1;
			node.enabled = true;
			node.socket = INT_MAX;
			node.timeout_counter = 0;
			node.reset_timeout = true;
			node.started = false;
			for (int i = 0; i < environment.num_servers; ++i)
			{
				node.dv[i].server_id = 0;
				node.dv[i].cost = USHRT_MAX;
			}
			environment.nodes[index-2] = node;
			free(split_array);

		}else if (index <= 1+environment.num_servers+environment.num_neighbours){

			/******* Tokenize line *********/
			char *split;
			int count=0;
			int split_array[3];
			split = strtok(line, " ");
			while(split){
				split_array[count] = atoi(split);
				count++;
				split = strtok(NULL," ");
			}
			/******* Set self id *********/
			uint16_t self_id = split_array[0];
			for (int i = 0; i < environment.num_servers; ++i)
			{
				/******* Set self node global variable *********/
				if (environment.nodes[i].server_id == self_id){
					self_node = &environment.nodes[i];
				}else if(environment.nodes[i].server_id == split_array[1]){ 
					/******* Set state of neighbours *********/
					environment.nodes[i].cost = split_array[2];
					environment.nodes[i].neighbour = true;
					environment.nodes[i].next_hop_server_id = environment.nodes[i].server_id;
				}
			}

		}
	}
}

