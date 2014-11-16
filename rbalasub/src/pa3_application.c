#include "../include/pa3_application.h"
#include "../include/global.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

Environment environment;

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
		while(getline(&line,&size,topology_file) != -1){
			setupEvironment(index,line);
			index++;
		}
		free(line);
		fclose(topology_file);
	}else{

		perror(top_file_path);
	}
}


void setupEvironment(int index , char *line)
{	
	if(index == 0){
		environment.num_servers = atoi(line);
	}else if(index == 1){
		environment.num_neighbours = atoi(line);
	}else
	{
		if (index < 1+environment.num_servers)
		{
			char *split;
			int count=0;
			char **split_array = (char **)calloc(3, sizeof (char*));
			split = strtok(line, " ");
			while(split)
			{
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
			environment.nodes[index-2] = node;
		}else if (index < 1+environment.num_servers+environment.num_neighbours)
		{
			/* code */
		}{
			char *split;
			int count=0;
			int split_array[3];
			split = strtok(line, " ");
			while(split)
			{
				split_array[count] = atoi(split);
				count++;
				split = strtok(NULL," ");
			}

			for (int i = 0; i < environment.num_servers; ++i)
			{
				if (environment.nodes[i].server_id == split_array[1])
				{
					environment.nodes[i].cost = split_array[2];
					environment.nodes[i].neighbour = true;
				}
			}

		}
	}
}

