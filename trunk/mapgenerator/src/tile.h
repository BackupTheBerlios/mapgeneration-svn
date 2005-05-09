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

#include "gpspoint.h"
#include "node.h"
#include "util/fixpointvector.h"
#include "util/serializer.h"
#include "util/rangereporting/quadtree.h"
#include "util/rangereporting/segment.h"

using namespace mapgeneration_util;
using rangereporting::Quadtree;
using rangereporting::Segment;

namespace mapgeneration
{

	class Tile;
	
	typedef Quadtree<Node::Id, GeoCoordinate, Tile> D_RangeReporting;
	
	
	/**
	 * @brief Tile implements a tile containing Nodes.
	 * 
	 * This class provides mainly the method to calculate the nearest neighbour.
	 * At the moment this is done brute force.
	 * 
	 * @todo Speed up nearest neighbour search.
	 */
	class Tile {

		public:
		
			typedef uint32_t Id;
			
			
			class const_iterator
			{
				
				public:
					
					inline
					const_iterator(FixpointVector<Node>::const_iterator start,
						FixpointVector<Node>::const_iterator end,
						FixpointVector<Node>::const_iterator position);
					
					inline bool
					operator==(const const_iterator& iter) const;
					
					inline bool
					operator!=(const const_iterator& iter) const;
					
					inline const_iterator
					operator++();
					
					inline const_iterator
					operator++(int dummy);
					
					inline const std::pair<bool, Node>&
					operator*() const;
					
					inline const std::pair<bool, Node>*
					operator->() const;
					
					
				private:
					
					FixpointVector<Node>::const_iterator _start;

					FixpointVector<Node>::const_iterator _end;

					FixpointVector<Node>::const_iterator _position;
					
			};
			
			
			/**
			 * @brief Empty constructor.
			 */
			Tile();
			
			
			/**
			 * @brief Constructor that inits the Tile with specified ID.
			 * 
			 * @param tile_id the ID
			 */
			Tile(unsigned int tile_id);
			
			
			inline std::pair<bool, Node::Id>
			add_node(const Node& node);
			
			
			inline Tile::const_iterator
			begin() const;
			
			
			void
			build_range_reporting_system();
			
			
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
			cluster_nodes_search(const GPSPoint& in_gps_point,
				const double in_search_radius, const double in_search_angle,
				std::vector<D_RangeReporting::Id>& query_result) const;
			
			
			void
			fast_cluster_nodes_search(const Segment<GeoCoordinate>& in_segment,
				const double in_search_distance, const double in_search_angle,
				std::vector<D_RangeReporting::Id>& query_result) const;
			
		
			void
			fast_cluster_nodes_search(const GPSPoint& in_gps_point,
				const double in_search_radius, const double in_search_angle,
				std::vector<D_RangeReporting::Id>& query_result) const;
			
		
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			deserialize(std::istream& i_stream);
			
			
			inline Tile::const_iterator
			end() const;
			
			
			inline bool
			exists_node(Node::Id node_id) const;

			
			inline bool
			exists_node(Node::LocalId node_local_id) const;

			
			/**
			 * @return the id
			 */
			inline unsigned int
			get_id() const;
			
			
			inline bool
			move_node(D_RangeReporting::Id& from_node_id, const Node& to_node);


			inline Node&
			node(Node::Id node_id);
			
			
			inline const Node&
			node(Node::Id node_id) const;
			
			
			inline Node&
			node(Node::LocalId node_local_id);
			
			
			inline const Node&
			node(Node::LocalId node_local_id) const;
			
			
			inline Node&
			operator[](Node::Id node_id);
			
			
			inline const Node&
			operator[](Node::Id node_id) const;
			
			
			inline Node&
			operator[](Node::LocalId node_local_id);
			
			
			inline const Node&
			operator[](Node::LocalId node_local_id) const;
			
			
			inline bool
			remove_node(D_RangeReporting::Id& node_id);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			serialize (std::ostream& o_stream) const;
			
			
			inline int
			size_of() const;
		

		private:
			
			/**
			 * @brief the ID of the tile
			 */
			unsigned int _id;
			
			/**
			 * @brief a Fixpointvector of all nodes in the tile
			 */
			FixpointVector<Node> _nodes;
			
			
			D_RangeReporting _range_reporting;
			
	};
	
	
	//---------------------------------------------------//
	//--- Iterator --------------------------------------//
	//---------------------------------------------------//
	inline
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
	}
	
	
	//---------------------------------------------------//
	//--- Main class: Tile ------------------------------//
	//---------------------------------------------------//
	inline std::pair<bool, Node::Id>
	Tile::add_node(const Node& node)
	{
		Node::LocalId node_local_id
			= static_cast<Node::LocalId>(_nodes.insert(node));
		Node::Id node_id = Node::merge_id_parts(_id, node_local_id);
		
		if ( _range_reporting.add_point(node_id) )
		{
			return std::make_pair(true, node_id);
		} else
		{
			_nodes.erase(node_local_id);
			
			return std::make_pair(false, 0);
		}
	}
	
	
	inline Tile::const_iterator
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
	}
	
	
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
		return _nodes[static_cast<int>(node_local_id)].first;
	}
	
	
	inline unsigned int
	Tile::get_id() const
	{
		return _id;
	}
	
	
/*	inline bool
	Tile::move_node(Node::Id from_node_id, const Node& to_node)
	{
	}*/
	
	
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
	
	
	inline Node&
	Tile::operator[](Node::LocalId node_local_id)
	{
		return _nodes[static_cast<int>(node_local_id)].second;
	}
	
	
	inline const Node&
	Tile::operator[](Node::LocalId node_local_id) const
	{
		return _nodes[static_cast<int>(node_local_id)].second;
	}
	
	
	inline int
	Tile::size_of() const
	{
		return _nodes.size_of();
	}
	
} // namespace mapgeneration

#endif //TILE_H
