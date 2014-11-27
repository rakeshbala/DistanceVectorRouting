#include "../include/pa3_bf.h"
#include "../include/global.h"
#include "../include/pa3_commands.h"	

#include <stdlib.h>
#include <limits.h>

uint16_t self_id;


/**
 * The bellman ford algorithm
 */
void run_BF(){

	for (int i = 0; i < environment.num_servers; ++i)
	{
		//Cost to node i
		Node node = environment.nodes[i];
		node.cost = USHRT_MAX;
		node.next_hop_server_id = -1;
		if (node.server_id == self_id)
		{
			node.cost = 0; //cost to self
			node.next_hop_server_id = node.server_id;
			continue;
		}


		/******* Loop through all neighbours and calculate cost to node i 
		through each and select the minimum *********/
		for (int j = 0; j < environment.num_servers; ++j)
		{

			//Costs with jth node which is a neighbour
			Node compare_node = environment.nodes[j];
			if (compare_node.neighbour == true)
			{
				// self -----> j
				uint16_t cost_to_node = compare_node.real_cost;

				//j -------- > i
				uint16_t dv_index = get_dv_idx(node.server_id,compare_node.dv);
				uint16_t cost_from_node = compare_node.dv[dv_index].cost;

				//self---->j---->i
				//handle USHRT_MAX wrap around
				uint16_t total_cost = /*if*/(cost_from_node == USHRT_MAX || cost_to_node == USHRT_MAX)?
									  /*then*/USHRT_MAX :/*else*/cost_to_node+cost_from_node;

				//compare with minimum
				if (node.cost>total_cost)
				{
					node.cost = total_cost;
					node.next_hop_server_id = compare_node.server_id;
				}
			}
		}
		//min after all iteration
		environment.nodes[i] = node;
	}

}

/**
 * Get index of the distance vector entry for a server id
 * @param  server_id server_id being queried
 * @param  dv        the distance vector
 * @return           the index
 */
uint16_t get_dv_idx(uint16_t server_id, Pkt_node *dv)
{

	for (int i = 0; i < environment.num_servers; ++i)
	{
		if (dv[i].server_id == server_id)
		{
			return i;
		}
	}
	return USHRT_MAX;
}

