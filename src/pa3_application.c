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

uint16_t self_id;

void parseLine(int index, char * line);
void setup_environment(FILE *topology_file);

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
		setup_environment(topology_file);
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
 * Setup the global environment variable from topology file
 * @param topology_file Topology file
 */
void setup_environment(FILE *topology_file)
{
	size_t size = 30;
	char *line= (char *)malloc(size);
	int index = 0;
	/******* Parse each line of topology file*********/
	while(getline(&line,&size,topology_file) != -1){
		/******* Parse each line *********/
		parseLine(index,line);
		index++;
	}
	free(line);
	/******* Initialize distance vectors *********/
	for (int i = 0; i < environment.num_servers; ++i)
	{
		for (int j = 0; j < environment.num_servers; ++j)
		{
			environment.nodes[i].dv[j].server_id = environment.nodes[j].server_id;
			if (environment.nodes[i].server_id == self_id)
			{
				environment.nodes[i].dv[j].cost = environment.nodes[j].cost;
			}else{
				//Cost to self
				if (environment.nodes[i].dv[j].server_id == environment.nodes[i].server_id)
				{
					environment.nodes[i].dv[j].cost = 0;
				}else{
					environment.nodes[i].dv[j].cost = USHRT_MAX;					
				}
				if(environment.nodes[i].neighbour == true){
					//Cost to neighbour
					if (environment.nodes[j].server_id==self_id)
					{
						environment.nodes[i].dv[j].cost = environment.nodes[i].cost;
					}
				}
			} 		
		}
	}
}

/**
 * Setup the environment variables for each line in topology file
 * @param index index of line being read
 * @param line  line being read
 */
void parseLine(int index , char *line)
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
			//cost and real_cost kind of redundant 
			node.cost = USHRT_MAX;
			node.real_cost = USHRT_MAX;
			node.next_hop_server_id = -1;
			node.enabled = true;
			node.socket = INT_MAX;
			node.timeout_counter = 0;
			node.reset_timeout = true;
			node.started = false;
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
			
			self_id = split_array[0];
			for (int i = 0; i < environment.num_servers; ++i)
			{
				/******* Set self node global variable *********/
				if (environment.nodes[i].server_id == self_id){
					environment.nodes[i].cost = 0;
					environment.nodes[i].next_hop_server_id = self_id;
					self_ip_str = environment.nodes[i].ip_addr;
					self_ip = environment.nodes[i].ip_addr_bin;
					self_port = environment.nodes[i].port;
				}else if(environment.nodes[i].server_id == split_array[1]){ 
					/******* Set state of neighbours *********/
					environment.nodes[i].cost = split_array[2];
					environment.nodes[i].real_cost = split_array[2];
					environment.nodes[i].neighbour = true;
					environment.nodes[i].next_hop_server_id = environment.nodes[i].server_id;
				}
			}

		}
	}
}

