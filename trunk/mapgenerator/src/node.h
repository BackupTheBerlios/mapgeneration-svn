/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include "geocoordinate.h"
#include "gpspoint.h"
#include "util/direction.h"
#include "util/serializer.h"

using namespace mapgeneration_util;


namespace mapgeneration
{

	/**
	 * @brief Node implements a node (a wonder!).
	 * 
	 * This class provides methods to
	 * <ul>
	 * <li>merge a GPSPoint with a Node</li>
	 * <li>get the edge IDs the Node is located on</li>
	 * <li>test if the Node is located on a specified edge</li>
	 * <li>...</li>
	 * </ul>
	 * 
	 * The class extends Direction and GeoCoordiante
	 * 
	 * @see Direction
	 * @see GeoCoordiante
	 */
	class Node : public Direction, public GeoCoordinate {

		public:
		
		
			typedef std::pair<unsigned int, unsigned int> Id;
		

			/**
			 * @brief Empty constructor.
			 */
			Node();
			
			
			/**
			 * @brief Copy constructor dealing a Node.
			 * 
			 * @param node the node that will be copied
			 */
			Node(const Node& node);
			
			
			/**
			 * @brief Copy constructor dealing a GPSPoint.
			 * 
			 * @param gps_point the gps point that will be copied
			 */
			Node(const GPSPoint& gps_point);
			
			
			void
			add_next_node_id(
				std::pair<unsigned int, unsigned int> next_node_id);
				
				
			int
			connected_nodes() const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);			
			
			
			/**
			 * @see multi_purpose_integer
			 */
			inline int
			get_mpi() const;
			
			
			/**
			 * @return the weight
			 */
			inline int
			get_weight();
			
			
			/**
			 * @brief Return true if the node id is reachable from this node.
			 * 
			 * @return True if node id is reachable, false otherwise.
			 */
			bool
			is_reachable(Id node_id) const;
			

			/**
			 * @brief Merges two gpspoints.
			 * 
			 * Tis method calculates the mean values of the
			 * latitudes, longitudes, altitudes and directions
			 * and gives the merged point a new weight (old weight + 1).
			 * 
			 * @param gps_point the point which will be merged with this node
			 */
			void
			merge(const GPSPoint& gps_point);
			
			
			/**
			 * @brief Returns a reference to the vector of next node ids.
			 * 
			 * @return Reference to vector of next node ids.
			 */
			inline std::vector< std::pair<unsigned int, unsigned int> >&
			next_node_ids();
			
			
			/**
			 * @brief Returns a constant 
			 * reference to the vector of next node ids.
			 * 
			 * @return Constant reference to vector of next node ids.
			 */
			inline const std::vector< std::pair<unsigned int, unsigned int> >&
			next_node_ids() const;
			
			
			/**
			 * @brief Assignment operator.
			 * 
			 * Assigns a Node to this.
			 * 
			 * @param node a reference to a Node
			 * @return (new) this
			 */
			Node&
			operator=(const Node& node);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			serialize (std::ostream& o_stream) const;
			
			
			/**
			 * @see multi_purpose_integer
			 */
			inline void
			set_mpi(int mpi);
			
			
		private:
		
			/**
			 * @brief This is used for different temporary storage operations
			 * in some parts of the program. This value is not serialized
			 * and may behave oddly. It is accessible via get_mpi and set_mpi.
			 * Just don't use it!
			 * 
			 * @see get_mpi
			 * @see set_mpi
			 */
			int
			_multi_purpose_integer;

			/**
			 * @brief A vector of node ids that reachable from this node.
			 */
			std::vector< std::pair<unsigned int, unsigned int> >
			_next_node_ids;


			/**
			 * @brief A value for the weight of a node.
			 */
			int
			_weight;

	};


	inline void
	Node::deserialize(std::istream& i_stream)
	{
		Direction::deserialize(i_stream);
		GeoCoordinate::deserialize(i_stream);
		Serializer::deserialize(i_stream, _next_node_ids);
		Serializer::deserialize(i_stream, _weight);
	}
	
	
	inline int
	Node::get_mpi() const
	{
		return _multi_purpose_integer;
	}

	
	inline int
	Node::get_weight()
	{
		return _weight;
	}
	
	
	inline std::vector< std::pair<unsigned int, unsigned int> >&
	Node::next_node_ids()
	{
		return _next_node_ids;
	}
			
			
	inline const std::vector< std::pair<unsigned int, unsigned int> >&
	Node::next_node_ids() const
	{
		return _next_node_ids;
	}
	
	
	inline void
	Node::serialize(std::ostream& o_stream) const
	{
		Direction::serialize(o_stream);
		GeoCoordinate::serialize(o_stream);
		Serializer::serialize(o_stream, _next_node_ids);
		Serializer::serialize(o_stream, _weight);
	}
	
	
	inline void
	Node::set_mpi(int mpi)
	{
		_multi_purpose_integer = mpi;
	}
	
} // namespace mapgeneration

#endif //NODE_H
