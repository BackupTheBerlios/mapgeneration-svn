/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tile.h"

#include <cmath>
#include <set>
#include <vector>

#include "util/constants.h"


namespace mapgeneration
{
	
	Tile::Tile()
	: _id(), _quadtree()
	{
	}

	
	Tile::Tile(Tile::Id tile_id)
	: _id(tile_id), _quadtree()
	{
		// compute span rectangle:
		int northing;
		int easting;
		GeoCoordinate::split_tile_id(get_id(), northing, easting);
		
		double lower_left_latitude
			= static_cast<double>(northing - 9000) / 100.0;
		double lower_left_longitude
			= static_cast<double>(easting - 18000) / 100.0;
		
		Node llc;
		llc.set_latitude(lower_left_latitude);
		llc.set_longitude(lower_left_longitude);
		
		Node urc;
		urc.set_latitude(lower_left_latitude + 0.01);
		urc.set_longitude(lower_left_longitude + 0.01);
		
		rangereporting::Rectangle<Node> span_rectangle;
		span_rectangle.set_corners(llc, urc);
		_quadtree.set_span_rectangle(span_rectangle);
		// done.
		
		_quadtree.init_ready();
	}
	
	
/*	void
	Tile::cluster_nodes_search(const GPSPoint& in_gps_point,
		const double in_search_radius, const double in_search_angle,
		std::vector<D_RangeReporting::Id>& out_query_result) const
	{
		FixpointVector<Node>::const_iterator iter = _nodes.begin();
		FixpointVector<Node>::const_iterator iter_end = _nodes.end();
		for(; iter != iter_end; ++iter)
		{
			double distance = iter->second.distance_approximated(in_gps_point);
			double direction_difference = iter->
				second.minimal_direction_difference_to(in_gps_point);
			if ((distance <= in_search_radius) && (direction_difference <= in_search_angle))
			{
				D_RangeReporting::Id id(
					Node::merge_id_parts(get_id(), iter.position_number()), 0);
				
				out_query_result.push_back(id);
			}
		}
	}*/
	
	
	void
	Tile::fast_cluster_nodes_search(const GPSPoint& in_gps_point,
		const double in_search_radius, const double in_search_angle,
		std::vector<Node::Id>& out_query_results) const
	{
		// compute corner distance.
		// + 2.0 is needed to provide 100% compatibility to cluster_node_search!
		double corner_dist = (in_search_radius + 2.0) * sqrt(2.0);
		Node llc(
			in_gps_point.compute_geo_coordinate_default(
				225.0,corner_dist, GeoCoordinate::_DEGREE, GeoCoordinate::_METER
			)
		);
		Node urc(
			in_gps_point.compute_geo_coordinate_default(
				45.0, corner_dist, GeoCoordinate::_DEGREE, GeoCoordinate::_METER
			)
		);
		
		// set query rectangle, set up temporary result vector
		// and start the query...
		rangereporting::Rectangle<Node> query_rectangle;
		query_rectangle.set_corners(llc, urc);
		
		std::vector<Node::LocalId> temp_results;
		_quadtree.range_query(query_rectangle, temp_results);
//		std::cout << "\t\t" << temp_results.size() << " TEMP CLUSTER NODES FOUND!" << std::endl;
		// done.
		
		// compose Node::Ids, compare distance and angles...
		out_query_results.clear();
		
		std::vector<Node::LocalId>::iterator iter = temp_results.begin();
		while (iter != temp_results.end())
		{
			bool erased = false;
			const Node& the_node = node(*iter);
//			std::cout << "\t\t\t" << *iter << ": "
//				<< the_node.distance_approximated(in_gps_point) << "m, "
//				<< the_node.minimal_direction_difference_to(in_gps_point)
//				<< "rad." << std::endl;
			
			if ( !within_search_distance(in_gps_point, the_node, in_search_radius) )
			{
				iter = temp_results.erase(iter);
				erased = true;
			} else
			{
				if ( !within_search_angle(in_gps_point, the_node, in_search_angle) )
				{
					iter = temp_results.erase(iter);
					erased = true;
				}
			}
			
			if ( !erased )
			{
				Node::Id node_id = Node::merge_id_parts(get_id(), *iter);
				out_query_results.push_back(node_id);
				
				++iter;
			}
		}
		
//		std::vector<Node::Id>::iterator iter2 = out_query_results.begin();
//		for(; iter2 != out_query_results.end(); ++iter2)
//		{
//			const Node& the_node = node(*iter2);
//			std::cout << "\t\t\t\t" << *iter2 << ": "
//				<< the_node.distance_approximated(in_gps_point) << std::endl;
//		}
		
	}
	
	
	void
	Tile::fast_cluster_nodes_search(const Segment<GeoCoordinate>& in_segment,
		const double in_search_distance, const double in_search_angle,
		std::vector<Node::Id>& out_query_results) const
	{
		// everything in radian!
		
		// get points...
		GeoCoordinate point_1 = in_segment.get_points().first;
		GeoCoordinate point_2 = in_segment.get_points().second;
		// done.
		
		// compute distance and bearing...
		double p2p_bearing = point_1.bearing_default(point_2);
		double corner_dist = (sqrt(2.0) * in_search_distance) / EARTH_RADIUS_M;
		// done.
		
		// compute the four corners of the trapezoid...
		double current_bearing;
		
		current_bearing = p2p_bearing - (0.75 * PI); // bearing - 135 degree
		Node corner_1(
			point_1.compute_geo_coordinate_default(current_bearing, corner_dist)
		);
		
		current_bearing = p2p_bearing + (0.75 * PI);
		Node corner_2(
			point_1.compute_geo_coordinate_default(current_bearing, corner_dist)
		);
		
		current_bearing = p2p_bearing + (0.25 * PI);
		Node corner_3(
			point_2.compute_geo_coordinate_default(current_bearing, corner_dist)
		);
		
		current_bearing = p2p_bearing - (0.25 * PI);
		Node corner_4(
			point_2.compute_geo_coordinate_default(current_bearing, corner_dist)
		);
		// done.
		
		// set query trapezoid, set up temporary result vector
		// and start the query...
		rangereporting::Trapezoid<Node> query_trapezoid(
			corner_1, corner_2, corner_3, corner_4);
		
		std::vector<Node::LocalId> temp_results;
		_quadtree.range_query(query_trapezoid, temp_results);
		// done.
		
		// compose Node::Ids, compare distance and angles...
		out_query_results.clear();

		std::vector<Node::LocalId>::iterator iter = temp_results.begin();
		while (iter != temp_results.end())
		{
			const Node& the_node = node(*iter);
			mapgeneration_util::Direction segment_direction(p2p_bearing);
			double min_direction_difference
				= the_node.minimal_direction_difference_to(segment_direction);
			
			if (min_direction_difference > in_search_angle)
			{
				iter = temp_results.erase(iter);
			} else
			{
				Node::Id node_id = Node::merge_id_parts(get_id(), *iter);
				out_query_results.push_back(node_id);
				
				++iter;
			}
		}
		// done.
	}
	
	
	void
	Tile::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _id);
		Serializer::deserialize(i_stream, _quadtree);
	}
	
	
	
	void
	Tile::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _id);
		Serializer::serialize(o_stream, _quadtree);
	}
		
} // namespace mapgeneration
