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
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);
			
			
			/**
			 * @brief Returns a reference to edge_ids.
			 * 
			 * @return edge_ids
			 */
			inline std::vector<unsigned int>&
			edge_ids();
			
			
			/**
			 * @brief Returns a const reference to edge_ids.
			 * 
			 * @return const edge_ids
			 */
			inline const std::vector<unsigned int>&
			edge_ids() const;
			
			
			/**
			 * @return the weight
			 */
			inline int
			get_weight();
			
			
			/**
			 * @brief Checks if this node is on specified edge.
			 * 
			 * @param edge_id the edge which is tested
			 * @return true, if check passed successfully
			 */
			bool
			is_on_edge_id(unsigned int edge_id) const;


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
			merge(GPSPoint& gps_point);
			
			
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
			 * @brief Renumbers items in edge_ids from form_id to to_id.
			 * 
			 * @param from_id the old value
			 * @param to_id the new value
			 */
			void
			renumber(unsigned int from_id, unsigned int to_id);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			serialize (std::ostream& o_stream) const;
			
			
		private:
	
			/**
			 * @brief vector of edge ids.
			 * 
			 * This node belongs the every edge whose id is in this vector.
			 */
			std::vector<unsigned int> _edge_ids;
			
			/**
			 * @brief a value for the weight of a node
			 */
			int _weight;

	};


	inline void
	Node::deserialize(std::istream& i_stream)
	{
		Direction::deserialize(i_stream);
		GeoCoordinate::deserialize(i_stream);
		Serializer::deserialize(i_stream, _edge_ids);
		Serializer::deserialize(i_stream, _weight);
	}
	
	
	inline std::vector<unsigned int>&
	Node::edge_ids()
	{
		return _edge_ids;
	}
	
	
	inline const std::vector<unsigned int>&
	Node::edge_ids() const
	{
		return _edge_ids;
	}
	
	
	
	inline int
	Node::get_weight()
	{
		return _weight;
	}
	
	
	inline void
	Node::serialize(std::ostream& o_stream) const
	{
		Direction::serialize(o_stream);
		GeoCoordinate::serialize(o_stream);
		Serializer::serialize(o_stream, _edge_ids);
		Serializer::serialize(o_stream, _weight);
	}
	
} // namespace mapgeneration

#endif //NODE_H
