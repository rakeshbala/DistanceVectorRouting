#include <stdint.h>
#include <stdbool.h>

#ifndef PA3_COMMANDS_
#define PA3_COMMANDS_ 

void handle_commands(char * command_string);
uint16_t read_pkt_update(char *pkt);
int get_node(uint16_t sid);
int get_node_from_ip_port(uint32_t ip, uint16_t port);
bool disable_link(uint16_t server_id, char **error_string, bool set_disable);

#endif