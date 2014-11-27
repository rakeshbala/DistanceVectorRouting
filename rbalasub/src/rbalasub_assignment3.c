/**
 * @rbalasub_assignment3
 * @author  Rakesh Balasubramanian <rbalasub@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. 
 * Main validates the command line arguments and starts the application
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/pa3_application.h"

/**
 * main function
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */


int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));

	/*Start Here*/

	char *top_file_path;
	float timeout;
	
	char opt;
	/******* Handle inputs *********/
	while ((opt = getopt(argc, argv, "ti")) != -1) {
        switch (opt) {
        case 't': top_file_path = argv[optind]; break;
        case 'i': {
        	char *endPtr;
        	float timeval = strtof(argv[optind],&endPtr);
        	if (strcmp(endPtr,"") !=0 )
        	{
        		fprintf(stderr, "Usage: %s -t <Path to topology file> -i <Routing Update Interval>\
            	\n", argv[0]);
        		exit(EXIT_FAILURE);
        	}else{
        		timeout = timeval;
        	}
        	break;
        }
        default:
            fprintf(stderr, "Usage: %s -t <Path to topology file> -i <Routing Update Interval>\
            	\n", argv[0]);
            exit(EXIT_FAILURE);
        }

    }

    /** Ref:
     * http://stackoverflow.com/questions/18079340/using-getopt-in-c-with-non-option-arguments
     */
    if (argv[optind] == NULL || argv[optind + 1] == NULL) {
    	fprintf(stderr, "Usage: %s -t <Path to topology file> -i <Routing Update Interval>\
    		\n", argv[0]);
    	exit(EXIT_FAILURE);
    }
	
    /******* Start Run Loop *********/
    start_run_loop(top_file_path, timeout);

	return 0;	
}
