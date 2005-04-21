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


using namespace mapgeneration_util;

namespace mapgeneration
{

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
			Tile (unsigned int tile_id);
			
			
			inline std::pair<bool, Node::Id>
			add_node(const Node& node);
			
			
			inline Tile::const_iterator
			begin() const;
			
			
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
			std::vector<Node::Id>
			cluster_nodes_search(const GPSPoint& gps_point, const double search_radius, const double search_angle) const;
			
		
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			deserialize(std::istream& i_stream);
			
			
			inline Tile::const_iterator
			end() const;
			
			
			inline bool
			exists_node(Node::Id node_id) const;

			
			/**
			 * @return the id
			 */
			inline unsigned int
			get_id() const;
			
			
//			inline bool
//			move_node(const Node& from_node, const Node& to_node);


			/**
			 * @brief Searches for all nodes which are inside the search_radius
			 * of the specified GeoCoordinate.
			 * 
			 * @param geo_coordinate the GeoCoordinate
			 * @param search_radius the search radius
			 * @return the vector of found nodes
			 */
//			std::vector<unsigned int>
//			nearest_neighbours_search(const GeoCoordinate& geo_coordinate, const double search_radius);

			
			inline Node&
			node(Node::Id);
			
			
			inline const Node&
			node(Node::Id) const;
			
			
//			inline bool
//			remove_node(const Node& node);
			
			
			/**
			 * @brief Return a reference to _nodes.
			 * 
			 * @return nodes
			 */
//			inline FixpointVector<Node>&
//			nodes();
			
			
			/**
			 * @brief Return a const reference to _nodes.
			 * 
			 * @return const nodes
			 */
//			inline const FixpointVector<Node>&
//			nodes() const;

			
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
			
			
//			rangereporting::Quadtree<Node*> _range_reporting;
	
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
		
		return std::make_pair(true, node_id);
		
/*		if ( _range_reporting.add_node(&(_nodes[node_local_id])) )
		{
			return std::make_pair(true, node_id);
		} else
		{
			_nodes.erase(node_local_id);
			
			return std::make_pair(false, 0);
		}*/
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
		return _nodes[Node::local_id(node_id)].first;
	}
	
	
	inline unsigned int
	Tile::get_id() const
	{
		return _id;
	}
	
	
/*	inline bool
	Tile::move_node(Node::Id from_node_id, const Node& to_node)
	{
		Node& from_node = _nodes[from_node_id].second;
		Node saved_node = from_node;
		
		from_node = to_node;
		
		if ( _range_reporting.move_node(
	}*/
	
	
	inline Node&
	Tile::node(Node::Id node_id)
	{
		return _nodes[Node::local_id(node_id)].second;
	}
	
	
	inline const Node&
	Tile::node(Node::Id node_id) const
	{
		return _nodes[Node::local_id(node_id)].second;
	}
	
	
/*	inline FixpointVector<Node>&
	Tile::nodes()
	{
		return _nodes;
	}*/
	
	
/*	inline const FixpointVector<Node>&
	Tile::nodes() const
	{
		return _nodes;
	}*/
	
	
	inline int
	Tile::size_of() const
	{
		return _nodes.size_of();
	}
	
} // namespace mapgeneration

#endif //TILE_H
