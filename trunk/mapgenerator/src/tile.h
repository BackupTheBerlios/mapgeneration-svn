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

			
			/**
			 * @return the id
			 */
			inline unsigned int
			get_id() const;


			/**
			 * @brief Searches for all nodes which are inside the search_radius
			 * of the specified GeoCoordinate.
			 * 
			 * @param geo_coordinate the GeoCoordinate
			 * @param search_radius the sear radius
			 * @return the vector of found nodes
			 */
			std::vector<unsigned int>
			nearest_neighbours_search(const GeoCoordinate& geo_coordinate, const double search_radius);

			
			/**
			 * @brief Return a reference to _nodes.
			 * 
			 * @return nodes
			 */
			inline FixpointVector<Node>&
			nodes();
			
			
			/**
			 * @brief Return a const reference to _nodes.
			 * 
			 * @return const nodes
			 */
			inline const FixpointVector<Node>&
			nodes() const;

			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			serialize (std::ostream& o_stream) const;
		

		private:
			
			/**
			 * @brief the ID of the tile
			 */
			unsigned int _id;
			
			/**
			 * @brief a Fixpointvector of all nodes in the tile
			 */
			FixpointVector<Node> _nodes;
	
	};
	
	
	inline unsigned int
	Tile::get_id() const
	{
		return _id;
	}
	
	
	inline FixpointVector<Node>&
	Tile::nodes()
	{
		return _nodes;
	}
	
	
	inline const FixpointVector<Node>&
	Tile::nodes() const
	{
		return _nodes;
	}


} // namespace mapgeneration

#endif //TILE_H
