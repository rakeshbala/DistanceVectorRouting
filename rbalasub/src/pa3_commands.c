#include "../include/pa3_commands.h"
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/pa3_network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

uint16_t self_id;
uint16_t self_port;
uint32_t self_ip;
char *self_ip_str;
int packet_count=0;

void processCommands(int argc, char **argv);
bool update_cost(uint16_t my_id, uint16_t server_id, char *cost, char **error_string);
void display_rt();
int node_cmp(const void * n1, const void * n2);
bool dump_packet(char **error_string);
bool disable_link(int server_id, char **error_string);
bool is_number ( char * string) ;

// #define NDEBUG
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, " " M "", ##__VA_ARGS__)
// #define debug(M, ...) fprintf(stderr, "DEBUG %s:%s:%d: " M "\n", __FILE__,__func__, __LINE__, ##__VA_ARGS__)
#endif

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
            processCommands(argc, argv);
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
void processCommands(int argc, char **argv){

    bool error_flag=false;
    char *command_string = argv[0];
    char *error_string = (char *)"";

	if (strcasecmp("academic_integrity",argv[0])==0)
	{	

        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);

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
            error_flag = !update_cost(
                (uint16_t)strtoul(argv[1],NULL,0), 
                (uint16_t)strtoul(argv[2],NULL,0), 
                argv[3],
                &error_string);
            if (error_flag == false)
            {
                cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);          
            } 
        }
             
    }else if (strcasecmp("step",argv[0])==0)
    {
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
        broadcast_packet();        
    }else if (strcasecmp("packets",argv[0])==0)
    {
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
        cse4589_print_and_log((char *)"%d\n", packet_count);
        packet_count = 0; 
    }else if (strcasecmp("display",argv[0])==0)
    {
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
        display_rt(); 
        
    }else if (strcasecmp("disable",argv[0])==0)
    {
        if (argc!=2)
        {
            error_string = (char *)"Invalid number of arguments";
            error_flag = true;
        }else{

            char *endPtr;
            uint16_t server_id = strtoul(argv[1],&endPtr,0);
            if (strcmp(endPtr,"")){
                error_string = (char *)"Invalid server id";
                error_flag = true;
            }else{
                error_flag = !disable_link(server_id, &error_string); 
                if (error_flag == false)
                {
                    cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);          
                }
            }

        }

    }else if (strcasecmp("crash",argv[0])==0)
    {
        cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);
        close_all();
        while(1){};
        
    }else if (strcasecmp("dump",argv[0])==0)
    {
        error_flag = !dump_packet(&error_string);
        if (error_flag == false)
        {
            cse4589_print_and_log((char *)"%s:SUCCESS\n", command_string);          
        } 
    }else if (strcasecmp("exit",argv[0])==0)
    {
        close_all();
        exit(EXIT_SUCCESS);
    }else if(strcasecmp("myip", argv[0])==0){
        printf("%s\n",self_ip_str);
        printf("%d\n",self_port); 
    }else
    {
        error_flag = true;
        error_string = (char *)"Unknown command";
    }


    if (error_flag)
    {
        cse4589_print_and_log((char *)"%s:%s\n",command_string,error_string);        
    }

}

/**
 * Disable the link to ther server_id
 * @param server_id [description]
 */
bool disable_link(int server_id, char **error_string)
{   
    int index = get_node(server_id);
    if (index > environment.num_servers)
    {
        *error_string = (char *)"Invalid server id";
        return false;
    }
    Node node = environment.nodes[index];
    if (node.neighbour == false)
    {
        *error_string = (char *)"Server not a neighbour";
        return false;
    }

    node.enabled = false;
    node.cost = USHRT_MAX;
    node.next_hop_server_id = -1;
    environment.nodes[index] = node;
    return true;

}

 /**
  * Read packet and update routing table
  * @param pkt update packet
  */
uint16_t read_pkt_update(char *pkt)
{
    
    uint16_t s_port;
    uint32_t s_ip;

    /******* Get source id and port *********/
    memcpy(&s_port, pkt+2, 2);
    s_port = ntohs(s_port);
    memcpy(&s_ip, pkt+4, 4);
    Node source_node = environment.nodes[get_node_from_ip_port(s_ip,s_port)];

    /******* Don't respond to disabled links *********/
    if (source_node.enabled == false){
        return USHRT_MAX;
    }

    printf("\n");
    cse4589_print_and_log((char *)"RECEIVED A MESSAGE FROM SERVER %d\n",source_node.server_id);
    pkt = pkt+8;//move to the entries
    for (int i = 0; i < environment.num_servers; ++i)
    {
        /******* Server Id *********/
        uint16_t server_id;
        memcpy(&server_id, pkt+(i*12)+8, 2);
        server_id = ntohs(server_id);

        /******* Cost to node *********/
        uint16_t serv_cost;
        memcpy(&serv_cost, pkt+(i*12)+10, 2);
        serv_cost = ntohs(serv_cost);
        cse4589_print_and_log((char *)"%-15d%-15d\n", server_id, serv_cost);
        int compare_index = get_node(server_id);
        Node compare_node = environment.nodes[compare_index];
        if (compare_node.server_id != self_id)
        {
            uint16_t new_cost = source_node.cost+ serv_cost;
            /******* Bellman - Ford *********/
            compare_node.cost = new_cost < compare_node.cost? new_cost : compare_node.cost;
            environment.nodes[compare_index] = compare_node;
        }

        
    }

    return source_node.server_id;
}


/**
 * Get node from server_id or ip address
 * @param  server_id server_id of the node
 * @return the index of node in enviornment.nodes[]
 */
int get_node (uint16_t sid)
{

    for (int j = 0; j < environment.num_servers; ++j)
    {
        if (sid == environment.nodes[j].server_id)    
        {

            return j;
        }
    }
    return INT_MAX;
}

int get_node_from_ip_port(uint32_t ip, uint16_t port){
    for (int j = 0; j < environment.num_servers; ++j)
    {
        if (ip == environment.nodes[j].ip_addr_bin && port==environment.nodes[j].port)    
        {

            return j;
        }
    }
    return INT_MAX;
}
/**
 * Create and dump the packet to binary
 */
bool dump_packet(char **error_string){

    void *packet = make_pkt();
    size_t pkt_size = 8 + 12*environment.num_servers;

   int status =  cse4589_dump_packet(packet, pkt_size); 
   if (status < 0)
   {
       *error_string = (char *)"Failed to write file";
       return false;
   }

   // read_pkt_update(packet); //For testing
   return true;

}



/**
 * Display the routing table
 */
void display_rt(){

    /******* Sort the nodes *********/
    qsort( environment.nodes, environment.num_servers, sizeof(Node), node_cmp);

    for (int i = 0; i < environment.num_servers ; ++i)
    {
        Node node= environment.nodes[i];
        cse4589_print_and_log((char *)"%-15d%-15d%-15d\n", 
            node.server_id,node.next_hop_server_id,node.cost); 
    }

}

/**
 * Comparator function for sorting array
 * @param  n1 node 1
 * @param  n2 node 2
 * @return    result
 */
int node_cmp(const void * n1, const void * n2){
    Node *n1_s = (Node *)n1;
    Node *n2_s = (Node *)n2;
    return n1_s->server_id - n2_s->server_id;
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
                    environment.nodes[i].next_hop_server_id = -1;
                }else{

                    if(is_number(cost) == true){
                        uint16_t cost_int = (uint16_t)strtoul(cost, NULL, 0);
                        environment.nodes[i].cost = cost_int;
                    }else{
                        *error_string = (char *)"Invalid cost";
                        return false;
                    }
                } 
                return true;
            }
        }
        *error_string = (char *)"Invalid server_id 2";
        return false;
    }
}


/**
 * Checks if string is Number
 * @param  string [description]
 * @return        [description]
 */
bool is_number ( char * string) 
{
    char *endPtr;
    strtoul(string,&endPtr,0);
    if (strcmp(endPtr,"")){
        return  true;
    }else{
        return false;
    }
}

