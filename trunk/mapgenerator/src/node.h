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
	 * <li>add successor nodes (resp. their ids)</li>
	 * <li>calculate the minimal direction difference between nodes</li>
	 * <li>merge and split Node::Ids to Node::LocalId and Tile::Id</li>
	 * <li>...</li>
	 * </ul>
	 * 
	 * The class extends a GeoCoordinate
	 * 
	 * @see Direction
	 * @see GeoCoordiante
	 * @see Tile
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
			 * @brief Copy constructor dealing a GeoCoordinate.
			 * 
			 * @param geo_coordinate the node that will be copied
			 */
			Node(const GeoCoordinate& geo_coordinate);
			
			
			/**
			 * @brief Copy constructor dealing a GPSPoint.
			 * 
			 * @param gps_point the gps point that will be copied
			 */
			Node(const GPSPoint& gps_point);
			
			
			/**
			 * @brief Adds a direction to this Node.
			 * 
			 * @param direction the direction value
			 */
//			inline void
//			add_direction(double direction);
			
			
			/**
			 * @brief Adds a new successor to this Node.
			 * 
			 * @param node_id the node_id of the designated successor
			 * @param direction the direction value
			 */
			void
			add_next_node(Id node_id, double direction);

			
			/**
			 * @brief Calculates the number of connected nodes to this Node.
			 * 
			 * @return the number of connected nodes
			 */
			inline int
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
			 * @brief Calculates the local node id.
			 * 
			 * @param id the id from which the local node id is calculated
			 * 
			 * @return the local node id
			 */
			static inline Node::LocalId
			local_id(Id id);
			
			
			/**
			 * @brief Merges a GeoCoordinate to this Node.
			 * 
			 * This method adds the GeoCoordinate's values (latitude,
			 * longitude and altitude) with respect to the weight to this Node.
			 * Afterwards a new weight (old weight + 1) is assigned.
			 * 
			 * @param geo_coordinate the point which will be merged with this node
			 */
			inline void
			merge(const GeoCoordinate& geo_coordinate);
			
			
			/**
			 * @brief Merges the Node::LocalId and Tile::Id to the Node::Id.
			 * This Node::Id is globally valid and unique.
			 * 
			 * @param tile_id the Tile::Id
			 * @param local_node_id the Node::LocalId
			 * 
			 * @return the Node::Id
			 */
			static inline Id
			merge_id_parts(uint32_t tile_id, LocalId local_node_id);
			
			
			/**
			 * @brief Calculates the minimal direction difference to the given
			 * Direction object.
			 * As a node may have several successors, each of them will be
			 * tested and the minimal difference is returned.
			 * 
			 * @param direction the Direction object
			 * 
			 * @return the minimal direction difference value
			 */
			double
			minimal_direction_difference_to(const Direction& direction) const;
			
			
			/**
			 * @brief Calculates the minimal direction difference to the given
			 * Node object.
			 * As a node may have several successors, each of them will be
			 * tested (which result in an exhausting search) and the
			 * minimal difference is returned.
			 * 
			 * @param node the Node object
			 * 
			 * @return the minimal direction difference value
			 */
			double
			minimal_direction_difference_to(const Node& node) const;
			

			/**
			 * @brief Returns a reference to the vector of next node ids.
			 * 
			 * @return Reference to vector of next node ids.
			 */
//			inline std::vector<Id>&
//			next_node_ids();
			
			
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
			inline Node&
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
			
			
			/**
			 * @brief Set the weight of this Node.
			 */
//			inline void
//			set_weight(int weight);
			
			
			/**
			 * @brief Splits the Node::Id into Tile::Id and Node::LocalId.
			 * 
			 * @param id the Node::Id
			 * @param tile_id the (returned) Tile::Id
			 * @param local_id the (returned) Node::LocalId
			 */
			static inline void
			split_id(Id id, uint32_t& tile_id, LocalId& local_id);
			
			
			/**
			 * @brief Calculates the Tile::Id from a Node::Id.
			 * 
			 * @param id the Node::Id
			 * 
			 * @return the Tile::Id
			 */
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
	
	
/*	inline void
	Node::add_direction(double direction)
	{
		_directions.push_back(Direction(direction));
	}*/
	
	
	inline int
	Node::connected_nodes() const
	{
		return _next_node_ids.size();
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
	
	
	inline void
	Node::merge(const GeoCoordinate& geo_coordinate)
	{
		double current_weight = static_cast<double>(_weight);
		double incremented_weight = static_cast<double>(_weight + 1);
		set_latitude(
			(get_latitude() * current_weight + geo_coordinate.get_latitude())
			/ (incremented_weight));
		set_longitude(
			(get_longitude() * current_weight + geo_coordinate.get_longitude())
			/ (incremented_weight));
		set_altitude(
			(get_altitude() * current_weight + geo_coordinate.get_altitude())
			/ (incremented_weight));

/*		double old_direction = get_direction();
		double merge_direction = gps_point.get_direction();
		double difference = merge_direction - old_direction;			

		if (difference > PI) merge_direction -= 2 * PI;
		else if (difference < -PI) old_direction -= 2 * PI;

		if ((old_direction < 0) || (merge_direction < 0))
		{
			old_direction += 2 * PI;
			merge_direction += 2 * PI;
		}

		double new_direction = (old_direction * (double)_weight + merge_direction) / (_weight + 1);
		if (new_direction >= 2 * PI) new_direction -= 2 * PI;
		else if (new_direction < 0) new_direction += 2 * PI;
		set_direction(new_direction);*/

		++_weight;
	}
	
	
	inline Node::Id
	Node::merge_id_parts(uint32_t tile_id, LocalId local_id)
	{
		return ( (static_cast<Node::Id>(tile_id) << 32) + local_id );
	}
	
	
/*	inline std::vector<Node::Id>&
	Node::next_node_ids()
	{
		return _next_node_ids;
	}*/
			
			
	inline const std::vector<Node::Id>&
	Node::next_node_ids() const
	{
		return _next_node_ids;
	}
	
	
	inline Node&
	Node::operator=(const Node& node)
	{
		GeoCoordinate::operator=(node);
		_directions = node._directions;
		_next_node_ids = node._next_node_ids;
		_weight = node._weight;
		
		return *this;
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
	
	
/*	inline void
	Node::set_weight(int weight)
	{
		_weight = weight;
	}*/
	
	
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
