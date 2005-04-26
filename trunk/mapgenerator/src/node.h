/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef NODE_H
#define NODE_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef HAVE_STDINT_H
	#include "stdint.h"
#endif

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
	class Node : public GeoCoordinate {

		public:
		
			typedef uint64_t Id;
			
			typedef uint32_t LocalId;


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
			add_direction(double direction);
			
			
			void
			add_next_node(Id node_id, double direction);

				
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
			
			
			static inline Node::LocalId
			local_id(Id id);
			
			
			double
			minimal_direction_difference_to(const Direction& direction) const;
			
			
			double
			minimal_direction_difference_to(const Node& node) const;
			

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
			
			
			static inline Id
			merge_id_parts(uint32_t tile_id, LocalId local_node_id);
			
			
			/**
			 * @brief Returns a reference to the vector of next node ids.
			 * 
			 * @return Reference to vector of next node ids.
			 */
			inline std::vector<Id>&
			next_node_ids();
			
			
			/**
			 * @brief Returns a constant 
			 * reference to the vector of next node ids.
			 * 
			 * @return Constant reference to vector of next node ids.
			 */
			inline const std::vector<Id>&
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
			
			
			static inline void
			split_id(Id id, uint32_t& tile_id, LocalId& local_id);
			
			
			static inline uint32_t
			tile_id(Id id);
			
			
		private:
		
			/**
			 * @brief A vector of the directions that are associated with this
			 * node. This are not the directions to the _next_node_ids!
			 * 
			 */
			std::vector<Direction>
			_directions;
		
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
			std::vector<Id>
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
		GeoCoordinate::deserialize(i_stream);
		Serializer::deserialize(i_stream, _directions);
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
	
	
	inline Node::LocalId
	Node::local_id(Id id)
	{
		return (id & 0x00000000FFFFFFFFULL);
	}
	
	
	inline Node::Id
	Node::merge_id_parts(uint32_t tile_id, LocalId local_id)
	{
		return ( (static_cast<Node::Id>(tile_id) << 32) + local_id );
	}
	
	
	inline std::vector<Node::Id>&
	Node::next_node_ids()
	{
		return _next_node_ids;
	}
			
			
	inline const std::vector<Node::Id>&
	Node::next_node_ids() const
	{
		return _next_node_ids;
	}
	
	
	inline void
	Node::serialize(std::ostream& o_stream) const
	{
		GeoCoordinate::serialize(o_stream);
		Serializer::serialize(o_stream, _directions);
		Serializer::serialize(o_stream, _next_node_ids);
		Serializer::serialize(o_stream, _weight);
	}
	
	
	inline void
	Node::set_mpi(int mpi)
	{
		_multi_purpose_integer = mpi;
	}
	
	
	inline void
	Node::split_id(Id id, uint32_t& tile_id, LocalId& local_id)
	{
		tile_id = id >> 32;
		local_id = id & 0x00000000FFFFFFFFULL;
	}
	
	
	inline uint32_t
	Node::tile_id(Id id)
	{
		return (id >> 32);
	}
	
	
} // namespace mapgeneration

#endif //NODE_H
