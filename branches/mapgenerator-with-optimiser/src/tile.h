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
#include "util/rangereporting/quadrangle.h"
#include "util/rangereporting/quadtree.h"
#include "util/rangereporting/segment.h"

using rangereporting::Quadrangle;
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
			typedef FixpointVector<Node> D_Nodes;
			
			
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
			fast_cluster_nodes_search(const GeoCoordinate& in_geo_coordinate,
				const double in_search_radius, /*const double in_search_angle,*/
				std::vector<Node::Id>& out_query_results) const;
			
			
			inline void
			fast_cluster_nodes_search(const GeoCoordinate& in_start_gc,
				const GeoCoordinate& end_gc, const double in_search_distance,
				const double in_search_angle,
				std::vector<Node::Id>& out_query_results) const;
			
			
			void
			fast_cluster_nodes_search(const Segment<GeoCoordinate>& in_segment,
				const double in_search_distance, const double in_search_angle,
				std::vector<Node::Id>& out_query_results) const;
			
			
			void
			fast_cluster_nodes_search(
				const Quadrangle<GeoCoordinate>& in_quadrangle,
				const double in_search_distance, const double in_search_angle,
				const double in_compare_to_angle,
				std::vector<Node::Id>& out_query_results) const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			deserialize(std::istream& i_stream);
			
			
			/**
			 * @brief Calculates the distances to the tile borders that attach at
			 * the specified heading.
			 * 
			 * @param heading the heading
			 * @return the distance (in meter)
			 */
			static double
			distance_to_tile_border(const GeoCoordinate& geo_coordinate,
				const GeoCoordinate::Heading heading,
				const GeoCoordinate::Representation output_representation);
			
			
			inline bool
			exists_node(Node::Id node_id) const;

			
			inline bool
			exists_node(Node::LocalId node_local_id) const;

			
			/**
			 * @return the id
			 */
			inline Id
			get_id() const;
			
			
			/**
			 * @brief Calculates the needed tile IDs for the GeoCoordinate.
			 * 
			 * Threshold should be smaller than half of the height of a tile.
			 * 
			 * @param geo_coordinate the GeoCoordinate
			 * @param radius_threshold the threshold of the radius
			 * 
			 * @return a vector of tile IDs that are within the radius_threshold
			 */
			static std::vector<Tile::Id>
			get_needed_tile_ids(const GeoCoordinate& geo_coordinate,
				const double radius_threshold);
			
			
			/**
			 * @brief Calculates the needed tile IDs for the line between two
			 * GeoCoordinates.
			 * 
			 * Threshold should be smaller than half of the height of a tile.
			 * 
			 * @param gc_1 first GeoCoordinate
			 * @param gc_2 second GeoCoordinate
			 * @param radius_threshold the threshold of the radius
			 * 
			 * @return a vector of tile IDs that are within the radius_threshold
			 */
			static std::vector<Tile::Id>
			get_needed_tile_ids(const GeoCoordinate& gc_1,
				const GeoCoordinate& gc_2, const double radius_threshold);
			
			
			/**
			 * @brief Calculates the Tile::Id of the given coordinates.
			 * 
			 * @return the Tile::Id
			 */
			inline static Tile::Id
			get_tile_id_for(double latitude, double longitude);
			
			
			/**
			 * @brief Calculates the Tile::Id of the given GeoCoordinate.
			 * 
			 * @return the Tile::Id
			 */
			inline static Tile::Id
			get_tile_id_of(const GeoCoordinate& geo_coordiante);
			
			
			/**
			 * @brief Merges the northing part and the easting part to one
			 * Tile::Id
			 * 
			 * @param northing the northing part of a tile ID
			 * @param easting the easting part of a tile ID
			 * @return the whole tile ID
			 * 
			 * @todo Explain the algorithm of tile ID generation.
			 */
			inline static Tile::Id
			merge_tile_id_parts(Tile::Id northing, Tile::Id easting);
			
			
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
			
			
			/** @todo This is needed for moving nodes. DO NOT ALTER THE NODES'
			 * COORDINATES. THIS WILL DEFINITALY DESTROY THE QUADTREE!!! */
//			inline FixpointVector<Node>&
//			nodes();
			
			
			/** @todo This is needed for the gui!!! */
			inline const D_Nodes&
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
			
			
			/**
			 * @brief Splits tile_id to the northing and easting part.
			 * 
			 * @param tile_id the Tile::Id
			 * @param northing_part a reference to the northing part
			 * @param easting_part a reference to the easting part
			 */
			inline static void
			split_tile_id(Id tile_id, Id& northing_part, Id& easting_part);
			
		private:
			
			/**
			 * @brief the ID of the tile
			 */
			Id _id;
			
			Quadtree<Node> _quadtree;
			
			
			Tile(const Tile& tile) {}
			
		
			void
			init_quadtree() const;
			
			
			/**
			 * @brief Splits my Tile::Id to the northing and easting part.
			 * 
			 * @param northing_part a reference to the northing part
			 * @param easting_part a reference to the easting part
			 */
			inline void
			split_tile_id(Id& northing_part, Id& easting_part) const;
			
			
			inline bool
			within_search_radius(const GeoCoordinate& geo_coordinate,
				const Node& node, const double search_radius) const;
			
			
/*			inline bool
			within_search_angle(const GeoCoordinate& geo_coordinate,
				const Node& node, const double search_angle) const;*/
			
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
	
	
	inline void
	Tile::fast_cluster_nodes_search(const GeoCoordinate& in_start_gc,
		const GeoCoordinate& in_end_gc, const double in_search_distance,
		const double in_search_angle, std::vector<Node::Id>& out_query_results)
		const
	{
		Segment<GeoCoordinate> segment;
		segment.set_point(0, in_start_gc);
		segment.set_point(1, in_end_gc);
		fast_cluster_nodes_search(segment, in_search_distance, in_search_angle,
			out_query_results);
	}
	
	
	inline Tile::Id
	Tile::get_id() const
	{
		return _id;
	}
	
	
	inline Tile::Id
	Tile::get_tile_id_for(double latitude, double longitude)
	{
		Tile::Id northing = static_cast<Tile::Id>((latitude + 90.0) * 100.0);
		Tile::Id easting = static_cast<Tile::Id>((longitude + 180.0) * 100.0);		
	   
		return merge_tile_id_parts(northing, easting);
	}
	
	
	inline Tile::Id
	Tile::get_tile_id_of(const GeoCoordinate& geo_coordinate)
	{
		return get_tile_id_for(
			geo_coordinate.get_latitude(), geo_coordinate.get_longitude());
	}
	
	
	inline Tile::Id
	Tile::merge_tile_id_parts(Tile::Id northing, Tile::Id easting)
	{
		if (northing < 0 || northing > 18000)
			throw ("Pole regions are not supported!!! (merge_tile_id_parts)");
		
		easting = easting % 36000;
		if (easting < 0) easting += 36000;
		
		return ((northing << 16) + easting);
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
	
	
/*	inline FixpointVector<Node>&
	Tile::nodes()
	{
		return _quadtree.points();
	}*/
	
	
	inline const Tile::D_Nodes&
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
		
		assert(tile_id == _id);
		
		return operator[](node_local_id);
	}
	
	
	inline const Node&
	Tile::operator[](Node::Id node_id) const
	{
		Tile::Id tile_id;
		Node::LocalId node_local_id;
		Node::split_id(node_id, tile_id, node_local_id);
		
		assert(tile_id == _id);
		
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
		
		assert(tile_id == _id);
		
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
	
	
	inline void
	Tile::split_tile_id(Tile::Id& northing_part, Tile::Id& easting_part) const
	{
		split_tile_id(get_id(), northing_part, easting_part);
	}
	
	
	inline void
	Tile::split_tile_id(Tile::Id tile_id, Tile::Id& northing_part,
		Tile::Id& easting_part)
	{
		northing_part =	tile_id >> 16;
		easting_part = tile_id % (1 << 16);
	}
	
	
	inline bool
	Tile::within_search_radius(const GeoCoordinate& geo_coordinate,
		const Node& node, const double search_radius) const
	{
		return (node.distance(geo_coordinate) <= search_radius);
	}
	
	
/*	inline bool
	Tile::within_search_angle(const GeoCoordinate& geo_coordinate,
	const Node& node, const double search_angle) const
	{
		double min_angle = node.minimal_direction_difference_to(geo_coordinate);
		
		return (min_angle <= search_angle);
	}*/
	
} // namespace mapgeneration

#endif //TILE_H
