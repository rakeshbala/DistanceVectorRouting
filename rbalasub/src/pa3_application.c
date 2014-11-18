#include "../include/pa3_application.h"
#include "../include/global.h"
#include "../include/pa3_listen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <arpa/inet.h>

/******* Globals *********/
Environment environment;
uint16_t self_port=0;
uint16_t self_id=0;
uint32_t self_ip=0;
char * self_ip_str;

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
			if (self_id == 0){
				self_id = split_array[0];
			}
			for (int i = 0; i < environment.num_servers; ++i)
			{
				Node node = environment.nodes[i];
				/******* Set self cost and self_port *********/
				if (node.server_id == self_id){
					self_port = node.port;
					self_ip = node.ip_addr_bin;
					self_ip_str = node.ip_addr;
					node.cost = 0;
					node.next_hop_server_id = self_id;
					environment.nodes[i] = node;
				}else if(node.server_id == split_array[1]){ 
					/******* Set state of neighbours *********/
					node.cost = split_array[2];
					node.neighbour = true;
					node.next_hop_server_id = node.server_id;
					environment.nodes[i] = node;

				}
			}

		}
	}
}

