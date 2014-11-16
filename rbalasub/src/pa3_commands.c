#include "../include/pa3_commands.h"
#include "../include/global.h"
#include "../include/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void processCommandArray(int argc, char **argv);
/**
 * Handle User Commands to application
 * @param commandString raw string supplied to stdin
 */
void handle_commands(char * commandString){

	/******* Handle enter key press  *********/
        commandString[strlen(commandString)-1]='\0'; //remove new line

        /******* Tokenize input *********/
        char *arg;
        int argc=0;
        char **argv = (char **)calloc(4, sizeof (char*));
        arg = strtok(commandString, " ");
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

	if (strcasecmp("academic_integrity",argv[0])==0)
	{	
		printf("\n");
		cse4589_print_and_log((char *)"I have read and understood the course academic integrity policy \
located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
		printf("\n\n");
 

	}
}

