#include <stdint.h>

#ifndef PA3_COMMANDS_
#define PA3_COMMANDS_ 
typedef enum {
    IP,
    SID,
} GET_TYPE;
void handle_commands(char * command_string);
uint16_t read_pkt_update(char *pkt);
int get_node(uint32_t sid_or_ip, GET_TYPE type);
#endif