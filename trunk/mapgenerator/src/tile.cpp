/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tile.h"

#include <set>
#include <vector>


namespace mapgeneration
{
	
	Tile::Tile()
	: _id(), _nodes()
	{
	}

	
	Tile::Tile(unsigned int tile_id)
	: _id(tile_id), _nodes()
	{
	}
	
	
	std::vector< std::pair<unsigned int, unsigned int> >
	Tile::cluster_nodes_search(const GPSPoint& gps_point, const double search_radius, const double search_angle) const
	{	
		std::vector< std::pair<unsigned int, unsigned int> > cluster_nodes;
		
		FixpointVector<Node>::const_iterator iter = nodes().begin();
		for(; iter != nodes().end(); ++iter)
		{
			double distance = iter->second.distance(gps_point);
			double angle_difference = iter->second.angle_difference(gps_point);
			if ((distance <= search_radius) && (angle_difference <= search_angle))
			{
				cluster_nodes.push_back(std::make_pair(get_id(), iter.position_number()));
			}
		}
		
		std::cout << "Cluster nodes search returns " << cluster_nodes.size() << " nodes.\n";
		
		return cluster_nodes;
	}
	
	
	void
	Tile::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _id);
		Serializer::deserialize(i_stream, _nodes);
	}
	
	
	
	std::vector<unsigned int>
	Tile::nearest_neighbours_search(const GeoCoordinate& geo_coordinate, const double search_radius)
	{	
		std::vector<unsigned int> nearest_nodes_vector;
		
		FixpointVector<Node>::iterator iter = nodes().begin();
		for(; iter != nodes().end(); ++iter)
		{
			double distance = iter->second.distance(geo_coordinate);
			if (distance <= search_radius)
			{
				nearest_nodes_vector.push_back(iter.position_number());
			}
		}
		
		return nearest_nodes_vector;	
	}


	void
	Tile::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _id);
		Serializer::serialize(o_stream, _nodes);
	}


} // namespace mapgeneration
