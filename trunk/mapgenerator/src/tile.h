/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TILE_H
#define TILE_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef HAVE_STDINT_H
	#include "stdint.h"
#endif

#include "node.h"
#include "util/serializer.h"
#include "util/rangereporting/quadtree.h"
#include "util/rangereporting/segment.h"

using namespace mapgeneration_util;
using rangereporting::Quadtree;
using rangereporting::Segment;

namespace mapgeneration
{

	/**
	 * @brief Tile implements a tile containing Nodes.
	 * 
	 * This class provides mainly the method to calculate the nearest neighbour.
	 */
	class Tile {
		
		public:
		
			typedef uint32_t Id;
			typedef Quadtree<Node>::D_IndexType D_IndexType;
			
			
			/**
			 * @brief Empty constructor.
			 * 
			 * @todo DO NOT USE DIRECTLY. Otherwise no Id is assigned!
			 */
			Tile();
			
			
			/**
			 * @brief Constructor that inits the Tile with specified ID.
			 * 
			 * @param tile_id the ID
			 */
			Tile(Id tile_id);
			
			
			inline Node::Id
			add_node(const Node& node);
			
			
			/**
			 * @brief Returns a vector of nodes which fulfill the cluster conditions
			 * for the specified point.
			 * 
			 * @param gps_point point of interest
			 * @param search_radius the search radius
			 * @param search_angle the search angle
			 * 
			 * @return the vector
			 */
			void
			fast_cluster_nodes_search(const Segment<GeoCoordinate>& in_segment,
				const double in_search_distance, const double in_search_angle,
				std::vector<Node::Id>& out_query_results) const;
			
		
			void
			fast_cluster_nodes_search(const GPSPoint& in_gps_point,
				const double in_search_radius, const double in_search_angle,
				std::vector<Node::Id>& out_query_results) const;
			
		
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			deserialize(std::istream& i_stream);
			
			
			inline bool
			exists_node(Node::Id node_id) const;

			
			inline bool
			exists_node(Node::LocalId node_local_id) const;

			
			/**
			 * @return the id
			 */
			inline Id
			get_id() const;
			
			
			inline bool
			move_node(Node::Id from_node_id,
				const GeoCoordinate& to_geo_coordinate);


			inline bool
			move_node(Node::LocalId from_node_id,
				const GeoCoordinate& to_geo_coordinate);


			inline Node&
			node(Node::Id node_id);
			
			
			inline const Node&
			node(Node::Id node_id) const;
			
			
			inline Node&
			node(Node::LocalId node_local_id);
			
			
			inline const Node&
			node(Node::LocalId node_local_id) const;
			
			
			/** @todo This is only a hack for the gui!!! */
			inline const FixpointVector<Node>&
			nodes() const;
			
			
			inline Node&
			operator[](Node::Id node_id);
			
			
			inline const Node&
			operator[](Node::Id node_id) const;
			
			
			inline Node&
			operator[](Node::LocalId node_local_id);
			
			
			inline const Node&
			operator[](Node::LocalId node_local_id) const;
			
			
			inline void
			remove_node(Node::Id node_id);
			
			
			inline void
			remove_node(Node::LocalId node_id);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			serialize (std::ostream& o_stream) const;
			
			
			inline size_t
			size_of() const;
		

		private:
			
			/**
			 * @brief the ID of the tile
			 */
			Id _id;
			
			Quadtree<Node> _quadtree;
			
			
			void
			init_quadtree() const;
			
			
			inline bool
			within_search_distance(const GPSPoint& gps_point, const Node& node,
				const double search_radius) const;
			
			
			inline bool
			within_search_angle(const GPSPoint& gps_point, const Node& node,
				const double search_angle) const;
			
	};
	
	
	//---------------------------------------------------//
	//--- Iterator --------------------------------------//
	//---------------------------------------------------//
/*	inline
	Tile::const_iterator::const_iterator(
		FixpointVector<Node>::const_iterator start,
		FixpointVector<Node>::const_iterator end,
		FixpointVector<Node>::const_iterator position)
	: _start(start), _end(end), _position(position)
	{
	}
	
	
	inline bool
	Tile::const_iterator::operator==(const const_iterator& iter) const
	{
		return (_position == iter._position);
	}
	
					
	inline bool
	Tile::const_iterator::operator!=(const const_iterator& iter) const
	{
		return ( !operator==(iter) );
	}
	
	
	inline Tile::const_iterator
	Tile::const_iterator::operator++()
	{
		if (_position != _end)
		{
			++_position;
		}
		
		return *this;
	}
	
	
	inline Tile::const_iterator
	Tile::const_iterator::operator++(int dummy)
	{
		return operator++();
	}
	
	
	inline const std::pair<bool, Node>&
	Tile::const_iterator::operator*() const
	{
		return _position.operator*();
	}
	
	
	inline const std::pair<bool, Node>*
	Tile::const_iterator::operator->() const
	{
		return _position.operator->();
	}*/
	
	
	//---------------------------------------------------//
	//--- Main class: Tile ------------------------------//
	//---------------------------------------------------//
	inline Node::Id
	Tile::add_node(const Node& node)
	{
		D_IndexType index = _quadtree.add_point(node);
		Node::Id id = Node::merge_id_parts(_id, index);
		
		return id;
		
/*		Node::LocalId node_local_id
			= static_cast<Node::LocalId>(_nodes.insert(node));
		Node::Id node_id = Node::merge_id_parts(_id, node_local_id);
		
		if ( _range_reporting.add_point(node_id) )
		{
			return std::make_pair(true, node_id);
		} else
		{
			_nodes.erase(node_local_id);
			
			return std::make_pair(false, 0);
		}*/
	}
	
	
/*	inline Tile::const_iterator
	Tile::begin() const
	{
		FixpointVector<Node>::const_iterator start = _nodes.begin();
		FixpointVector<Node>::const_iterator end = _nodes.end();
		return Tile::const_iterator(start, end, start);
	}
	
	
	inline Tile::const_iterator
	Tile::end() const
	{
		FixpointVector<Node>::const_iterator start = _nodes.begin();
		FixpointVector<Node>::const_iterator end = _nodes.end();
		return Tile::const_iterator(start, end, end);
	}*/
	
	
	inline bool
	Tile::exists_node(Node::Id node_id) const
	{
		Tile::Id tile_id;
		Node::LocalId node_local_id;
		Node::split_id(node_id, tile_id, node_local_id);
		
		return exists_node(node_local_id);
	}
	
	
	inline bool
	Tile::exists_node(Node::LocalId node_local_id) const
	{
		return _quadtree.exists_point(node_local_id);
	}
	
	
	inline unsigned int
	Tile::get_id() const
	{
		return _id;
	}
	
	
	inline bool
	Tile::move_node(Node::Id from_node_id,
		const GeoCoordinate& to_geo_coordinate)
	{
		Tile::Id tile_id;
		Node::LocalId node_local_id;
		Node::split_id(from_node_id, tile_id, node_local_id);
		
		return move_node(node_local_id, to_geo_coordinate);
	}
	
	
	inline bool
	Tile::move_node(Node::LocalId from_node_local_id,
		const GeoCoordinate& to_geo_coordinate)
	{
		Node to_node(to_geo_coordinate);
		
		return _quadtree.move_point(from_node_local_id, to_node);
	}
	
	
	inline Node&
	Tile::node(Node::Id node_id)
	{
		return operator[](node_id);
	}
	
	
	inline const Node&
	Tile::node(Node::Id node_id) const
	{
		return operator[](node_id);
	}
	
	
	inline Node&
	Tile::node(Node::LocalId node_local_id)
	{
		return operator[](node_local_id);
	}
	
	
	inline const Node&
	Tile::node(Node::LocalId node_local_id) const
	{
		return operator[](node_local_id);
	}
	
	
	inline const FixpointVector<Node>&
	Tile::nodes() const
	{
		return _quadtree.points();
	}
	
	
	inline Node&
	Tile::operator[](Node::Id node_id)
	{
		Tile::Id tile_id;
		Node::LocalId node_local_id;
		Node::split_id(node_id, tile_id, node_local_id);
		
		return operator[](node_local_id);
	}
	
	
	inline const Node&
	Tile::operator[](Node::Id node_id) const
	{
		Tile::Id tile_id;
		Node::LocalId node_local_id;
		Node::split_id(node_id, tile_id, node_local_id);
		
		return operator[](node_local_id);
	}
	
	
	/** @todo NEVER change the coordinates of the returned node.
	 * That will definitely DESTROY the quadtree!!! */
	inline Node&
	Tile::operator[](Node::LocalId node_local_id)
	{
		return _quadtree.point(node_local_id);
	}
	
	
	inline const Node&
	Tile::operator[](Node::LocalId node_local_id) const
	{
		return _quadtree.point(node_local_id);
	}
	
	
	inline void
	Tile::remove_node(Node::Id node_id)
	{
		Tile::Id tile_id;
		Node::LocalId node_local_id;
		Node::split_id(node_id, tile_id, node_local_id);
		
		remove_node(node_local_id);
	}
	
	
	inline void
	Tile::remove_node(Node::LocalId node_local_id)
	{
		_quadtree.remove_point(node_local_id);
	}
	
	
	inline size_t
	Tile::size_of() const
	{
		return sizeof(Tile) + _quadtree.size_of();
	}
	
	
	inline bool
	Tile::within_search_distance(const GPSPoint& gps_point, const Node& node,
		const double search_radius) const
	{
		return (node.distance_approximated(gps_point) <= search_radius);
	}
	
	
	inline bool
	Tile::within_search_angle(const GPSPoint& gps_point, const Node& node,
		const double search_angle) const
	{
		double min_angle = node.minimal_direction_difference_to(gps_point);
		
		return (min_angle <= search_angle);
	}
	
} // namespace mapgeneration

#endif //TILE_H
