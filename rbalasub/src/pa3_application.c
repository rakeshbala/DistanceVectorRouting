#include "../include/pa3_application.h"
#include "../include/global.h"
#include "../include/pa3_listen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/******* Globals *********/
Environment environment;
int self_port=0;
int self_id=0;

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
			node.ip_addr = split_array[1];
			node.port = atoi(split_array[2]);
			node.neighbour = false;
			node.cost = UINT_MAX;
			node.next_hop_server_id = -1;
			environment.nodes[index-2] = node;
		}else if (index < 1+environment.num_servers+environment.num_neighbours){

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
			if (self_id == 0){
				self_id = split_array[0];
			}
			for (int i = 0; i < environment.num_servers; ++i)
			{
				/******* Set self cost and self_port *********/
				if (environment.nodes[i].server_id == self_id){
					self_port = environment.nodes[i].port;
					environment.nodes[i].cost = 0;
					environment.nodes[i].next_hop_server_id = self_id;
				}else if(environment.nodes[i].server_id == split_array[1]){ 
					/******* Set state of neighbours *********/
					environment.nodes[i].cost = split_array[2];
					environment.nodes[i].neighbour = true;
					environment.nodes[i].next_hop_server_id = self_id;
				}
			}

		}
	}
}

