#include "../include/pa3_commands.h"
#include "../include/global.h"
#include "../include/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

uint16_t self_id;
uint16_t self_port;

void processCommandArray(int argc, char **argv);
bool update_cost(uint16_t my_id, uint16_t server_id, char *cost, char **error_string);
/**
 * Handle User Commands to application
 * @param command_string raw string supplied to stdin
 */
void handle_commands(char * command_string){

	/******* Handle enter key press  *********/
        command_string[strlen(command_string)-1]='\0'; //remove new line

        /******* Tokenize input *********/
        char *arg;
        int argc=0;
        char **argv = (char **)calloc(4, sizeof (char*));
        arg = strtok(command_string, " ");
        while(arg)
        {
            argv[argc] = strdup(arg);
            argc++;
            if (argc>4)
            {
                fprintf(stderr, "Too many arguments\n");
                free(argv);
                return;
            }
            arg = strtok(NULL," ");
        }

        /******* Send tokenized commands for processing *********/
        if (!(argc==0))
        {
            processCommandArray(argc, argv);
            /******* Free the commands array *********/
            int ii =0;
            for (ii = 0; ii < argc; ++ii)
            {
                free(argv[ii]);
            }
        }
        free (argv);
}

/**
 * Process tokenized commands
 * @param argc Number of arguments
 * @param argv The argument array
 */
void processCommandArray(int argc, char **argv){

    bool error_flag=false;
    char *command_string = argv[0];
    char *error_string = (char *)"";

	if (strcasecmp("academic_integrity",argv[0])==0)
	{	
		printf("\n");
		cse4589_print_and_log((char *)"I have read and understood the course academic integrity policy \
located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
		printf("\n\n");
 

	}else if (strcasecmp("update",argv[0])==0)
    {
        if (argc != 4)
        {
            error_flag = true;
            error_string = (char *)"Invalid number of arguments";
        }else{
            error_flag = update_cost(
                (uint16_t)strtoul(argv[1],NULL,0), 
                (uint16_t)strtoul(argv[2],NULL,0), 
                argv[3],
                &error_string);
        }
             
    }else if (strcasecmp("step",argv[0])==0)
    {
        
    }else if (strcasecmp("packets",argv[0])==0)
    {
        
    }else if (strcasecmp("display",argv[0])==0)
    {
        
    }else if (strcasecmp("disable",argv[0])==0)
    {
        
    }else if (strcasecmp("crash",argv[0])==0)
    {
        
    }else if (strcasecmp("dump",argv[0])==0)
    {
        
    }else if (strcasecmp("exit",argv[0])==0)
    {
        exit(EXIT_SUCCESS);
    }else{
        error_flag = true;
        error_string = (char *)"Unknown command";

    }

    if (error_flag == false)
    {
        cse4589_print_and_log((char *)"%s:%s\n",command_string,error_string);        
    }else{
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
    }

}

/**
 * Update cost between two links (will be executed on both machines)
 * @param  my_id        self server_id
 * @param  server_id    server_id of the node
 * @param  cost         cost to be updated
 * @param  error_string error string out parameter if any
 * @return              update was successful or not
 */
bool update_cost (uint16_t my_id, uint16_t server_id, char *cost, char **error_string) {

    if (my_id != self_id)
    {
      *error_string = (char *)"Invalid server_id 1";
      return false;
    }else{
        for (int i = 0; i < environment.num_servers; ++i)
        {   
            if (server_id == environment.nodes[i].server_id)
            {
                if ( strcasecmp("inf",cost) == 0 )
                {
                    environment.nodes[i].cost = USHRT_MAX;
                }else if(strtol(cost, NULL, 0) > 0 ){
                    uint16_t cost_int = (uint16_t)strtoul(cost, NULL, 0);
                    environment.nodes[i].cost = cost_int;
                }else{
                    *error_string = (char *)"Invalid cost";
                    return false;
                }
                return true;
            }
        }
        *error_string = (char *)"Invalid server_id 2";
        return false;
    }
}