/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EDGE_H
#define EDGE_H

#include <iostream>
#include <vector>
#include <set>
#include <list>
#include "util/serializer.h"

using namespace mapgeneration_util;


namespace mapgeneration
{
	/**
	 * @brief This class represent an edge.
	 */
	class Edge {
		
		public:
				
			/**
			 * @brief Empty Constructor.
			 */
			Edge () {}
			
			/**
			 * @brief Constructor which inits this edge with specified ID.
			 * 
			 * @param edgeID the id
			 */
			Edge (int edgeID);
			
			/**
			 * @brief Changes time at current_time_iter to new_time_after.
			 * 
			 * @todo this name is a little bit confusing, we should change it!
			 * 
			 * @param current_time_iter iterator pointing at the current time
			 * @param new_time_after value of the new time
			 */
			void
			change_time_after(std::list<double>::iterator& current_time_iter, double new_time_after);
			
			
			/**
			 * @brief Changes time before current_time_iter to new_time_before.
			 * 
			 * @param current_time_iter iterator pointing at the current time
			 * @param new_time_before value of the new time
			 */
			void
			change_time_before(std::list<double>::iterator& current_time_iter, double new_time_before);
			

			/**
			 * @brief Changes time before and at current_time_iter to new_time
			 * resp. next_new_time.
			 * 
			 * @param current_time_iter iterator pointing at the current time
			 * @param new_time value of the new time
			 * @param new_next_time value of next new time
			 */
			void
			change_times(std::list<double>::iterator& current_time_iter, double new_time, double new_next_time);
			

			/**
			 * @see mapgeneration_util::Serializer
			 */			
			void
			deserialize(std::istream& i_stream);
			

			/**
			 * @brief Initializes a edge, and inserts the first two node IDs and
			 * the time into the lists.
			 * 
			 * @param first_node_id id of first node which will be inserted
			 * @param second_node_id id of second node which will be inserted
			 * @param time time between first and second node
			 */
			void
			init(std::pair<unsigned int, unsigned int> first_node_id, std::pair<unsigned int, unsigned int> second_node_id, double time);


			/**
			 * @brief Extends the edge by adding the node ID next to a specified
			 * node id.
			 * 
			 * Handles senseless inputs like next_to_node_id is not at end of edge.
			 * 
			 * @param next_to_node_id neighbor of the new node id
			 * @param new_node_id id of new node which will extend this edge
			 * @param time time between new node and "next to node"
			 */
			void
			extend(std::pair<unsigned int, unsigned int> next_to_node_id, std::pair<unsigned int, unsigned int> new_node_id, double time);
	

			/**
			 * @return edge id
			 */		
			inline unsigned int
			get_id() const;


			/**
			 * @brief Returns a set of tile IDs.
			 * 
			 * This method travers the list of node IDs and
			 * inserts the first item of node IDs pairs into a set.
			 * 
			 * @return the set
			 */
			std::set<unsigned int>
			get_tile_ids();
			
						
			/**
			 * @todo const?
			 * 
			 * @return a reference to node_ids
			 */
			std::list< std::pair<unsigned int, unsigned int> >&
			node_ids();
			

			/**
			 * @return a const reference to node_ids
			 */
			const std::list< std::pair<unsigned int, unsigned int> >&
			node_ids() const;


			/**
			 * @brief Checks if the node is at the end of the edge.
			 * 
			 * @param node_id the node which will be tested
			 * @return true, if check passed successfully
			 */
			bool
			node_is_at_end(const std::pair<unsigned int, unsigned int> node_id) const;
			
			
			/**
			 * @brief Checks if the node is the first element of the edge.
			 * 
			 * @param node_id the node which will be tested.
			 * @return true, if check passed successfully
			 */
			bool
			node_is_at_start(const std::pair<unsigned int, unsigned int> node_id) const;


			/**
			 * @brief Equality operator
			 * 
			 * @param compare_to the edge for comparison
			 * @return true, if both edge are equal (whatever equal mean!)
			 */
			bool
			operator==(Edge& compare_to) const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */			
			void
			serialize (std::ostream& o_stream) const;
			
			
			inline void
			set_id(const unsigned int id);
			
			
			/**
			 * @brief Returns the approx. size of this object.
			 * 
			 * @return the approx. size
			 */			
			int
			size_of();
			
			
			/**
			 * @return a reference to times
			 */
			inline std::list<double>&
			times();


		private:
			
			/**
			 * @brief the ID of the edge
			 */
			unsigned int _id;
			 
			 /**
			  * @brief a list of the node IDs of the edge
			  */
			std::list< std::pair<unsigned int, unsigned int> > _node_ids;
			
			/**
			 * @brief a list of time differences of the nodes of the edge
			 */ 
			std::list<double> _times;

		
	};


	inline unsigned int
	Edge::get_id() const
	{
		return _id;
	}
	
	
	inline void
	Edge::set_id(const unsigned int id)
	{
		_id = id;
	}
	
	
	
	inline std::list<double>&
	Edge::times()
	{
		return _times;
	}

} // namespace mapgeneration

#endif //EDGE_H
