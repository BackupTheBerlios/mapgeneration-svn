/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef NODE_H
#define NODE_H

#include <cassert>
#include <iostream>

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef HAVE_STDINT_H
	#include "stdint.h"
#endif

#include "util/geocoordinate.h"
#include "util/serializer.h"

/* Forward declarations... */
namespace mapgeneration_util
{
	
	class Direction;
	
} // namespace mapgeneration_util

namespace mapgeneration
{
	
	class Edge;
	class Node;
	
} // namespace mapgeneration
/* Forward declarations done! */


using mapgeneration_util::Direction;
using mapgeneration_util::GeoCoordinate;
using mapgeneration_util::Serializer;

namespace mapgeneration
{
	
	std::ostream&
	operator<<(std::ostream& o_stream, const Node& node);
	
	
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
			
			typedef std::list<Edge> D_Edges;
//			typedef std::vector<Edge> D_Edges;
			
			
			static const int _MAX_WEIGHT = 15;
			
			
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
			 * @brief Adds a new predecessor to this Node.
			 * 
			 * @param node the designated predecessor
			 */
			#warning Convention for the direction?! From node to this, or otherwise.
			void
			add_predecessor(Id node, double direction);
			
			
			/**
			 * @brief Adds a new successor to this Node.
			 * 
			 * @param node the designated successor
			 */
			void
			add_successor(Id node, double direction);
//			void
//			add_next_node(Id node, double direction);

			
			/**
			 * @brief Calculates the number of connected nodes to this Node.
			 * 
			 * @return the number of connected nodes
			 */
//			inline int
//			connected_nodes() const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);			
			
			
			inline const D_Edges&
			edges() const;
			
			
			/**
			 * @see multi_purpose_integer
			 */
//			inline int
//			get_mpi() const;
			
			
			/**
			 * @return the weight
			 */
			inline int
			get_weight();
			
			
			bool
			has_predecessor(Id node_id) const;
			
			
			bool
			has_successor(Id node_id) const;
			
			
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
			
			
			// NOT POSSIBLE!
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
//			double
//			minimal_direction_difference_to(const Node& node) const;
			
			
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
			
			
			inline const Edge&
			predecessor(Id id) const;
			
			
			/**
			 * @brief Returns a constant 
			 * reference to the vector of next node ids.
			 * 
			 * @return Constant reference to vector of next node ids.
			 */
			inline const D_Edges&
			predecessors() const;
			
			
			inline bool
			remove_predecessor(Id id);
			
			
			inline bool
			remove_successor(Id id);
			
			
			inline void
			set_weight(int weight);
			
			
			inline const Edge&
			successor(Id id) const;
			
			
			inline const D_Edges&
			successors() const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			serialize (std::ostream& o_stream) const;
			
			
			/**
			 * @see multi_purpose_integer
			 */
//			inline void
//			set_mpi(int mpi);
			
			
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
			
			enum EdgeKind
			{
				_NONE = 0,
				_PREDECESSOR,
				_SUCCESSOR
			};
			
			
			/**
			 * @brief This is used for different temporary storage operations
			 * in some parts of the program. This value is not serialized
			 * and may behave oddly. It is accessible via get_mpi and set_mpi.
			 * Just don't use it!
			 * 
			 * @see get_mpi
			 * @see set_mpi
			 */
//			int
//			_multi_purpose_integer;

			/**
			 * @brief A vector of node ids that reachable from this node.
			 */
			D_Edges
			_predecessors;
			
			
			D_Edges
			_successors;


			/**
			 * @brief A value for the weight of a node.
			 */
			int
			_weight;
			
			
			D_Edges::const_iterator
			edge_iterator(Id node_id, EdgeKind search_in_kind) const;
			
			
			D_Edges::iterator
			edge_iterator(Id node_id, EdgeKind search_in_kind);
			
	};
	
} // namespace mapgeneration

#include "edge.h"

namespace mapgeneration
{
	
	inline void
	Node::deserialize(std::istream& i_stream)
	{
		GeoCoordinate::deserialize(i_stream);
		Serializer::deserialize(i_stream, _predecessors);
		Serializer::deserialize(i_stream, _successors);
		Serializer::deserialize(i_stream, _weight);
	}
	
	
//	inline int
//	Node::connected_nodes() const
//	{
//		return _edges.size();
//	}
	
	
/*	inline int
	Node::get_mpi() const
	{
		return _multi_purpose_integer;
	}*/

	
	inline const Node::D_Edges&
	Node::edges() const
	{
		#warning This method is deprecated!
		
		return successors();
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

		++_weight;
		if (_weight > _MAX_WEIGHT)
			_weight = _MAX_WEIGHT;
	}
	
	
	inline Node::Id
	Node::merge_id_parts(uint32_t tile_id, LocalId local_id)
	{
		return ( (static_cast<Node::Id>(tile_id) << 32) + local_id );
	}
	
	
	inline Node&
	Node::operator=(const Node& node)
	{
		if (this != &node)
		{
			GeoCoordinate::operator=(node);
			
			_predecessors.clear();
			_predecessors.insert(_predecessors.end(),
				node._predecessors.begin(), node._predecessors.end());
			
			_successors.clear();
			_successors.insert(_successors.end(),
				node._successors.begin(), node._successors.end());
			
			_weight = node._weight;
		}
		
		return *this;
	}
	
	
	inline const Edge&
	Node::predecessor(Id id) const
	{
		assert(has_predecessor(id));
		return *edge_iterator(id, _PREDECESSOR);
	}
	
	
	inline const Node::D_Edges&
	Node::predecessors() const
	{
		return _predecessors;
	}
	
	
	inline bool
	Node::remove_predecessor(Id id)
	{
		D_Edges::iterator iter = edge_iterator(id, _PREDECESSOR);
		if (iter == _predecessors.end())
			return false;
		
		_predecessors.erase(iter);
		
		assert(!has_predecessor(id));
		
		return true;
	}
	
	
	inline bool
	Node::remove_successor(Id id)
	{
		D_Edges::iterator iter = edge_iterator(id, _SUCCESSOR);
		if (iter == _successors.end())
			return false;
		
		_successors.erase(iter);
		
		assert(!has_successor(id));
		
		return true;
	}
	
	
	inline void
	Node::set_weight(int weight)
	{
		_weight = weight;
	}
	
	
	inline const Edge&
	Node::successor(Id id) const
	{
		assert(has_successor(id));
		return *edge_iterator(id, _SUCCESSOR);
	}
	
	
	inline const Node::D_Edges&
	Node::successors() const
	{
		return _successors;
	}
	
	
	inline void
	Node::serialize(std::ostream& o_stream) const
	{
		GeoCoordinate::serialize(o_stream);
		Serializer::serialize(o_stream, _predecessors);
		Serializer::serialize(o_stream, _successors);
		Serializer::serialize(o_stream, _weight);
	}
	
	
/*	inline void
	Node::set_mpi(int mpi)
	{
		_multi_purpose_integer = mpi;
	}*/
	
	
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
