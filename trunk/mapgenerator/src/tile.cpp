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
	: _id(), _nodes(), _range_reporting()
	{
	}

	
	Tile::Tile(unsigned int tile_id)
	: _id(tile_id), _nodes(), _range_reporting()
	{
		build_range_reporting_system();
	}
	
	
	void
	Tile::build_range_reporting_system()
	{
		/* compute span rectangle... */
		int northing;
		int easting;
		GeoCoordinate::split_tile_id(_id, northing, easting);
		
		double lower_left_latitude = static_cast<double>(northing - 9000) / 100.0;
		double lower_left_longitude = static_cast<double>(easting - 18000) / 100.0;
		
		GeoCoordinate llc;
		llc.set_latitude(lower_left_latitude);
		llc.set_longitude(lower_left_longitude);
		
		GeoCoordinate urc;
		urc.set_latitude(lower_left_latitude + 0.01);
		urc.set_longitude(lower_left_longitude + 0.01);
		
		rangereporting::Rectangle<GeoCoordinate> rectangle;
		rectangle.set_corners(llc, urc);
		/* done. */
		
		/* compute max depth... */
		int max_depth = static_cast<int>(ceil(log10(_nodes.size()) / log10(4)));
		if (max_depth < 5)
			max_depth = 5;
		/* done. */
		
		/* set the values for _range_reporting... */
		_range_reporting.set_max_depth(max_depth);
		_range_reporting.set_points(this);
		_range_reporting.set_span_rectangle(rectangle);
		_range_reporting.init_ready();
		/* done. */
		
		FixpointVector<Node>::iterator iter = _nodes.begin();
		FixpointVector<Node>::iterator iter_end = _nodes.end();
		for(; iter != iter_end; ++iter)
		{
			_range_reporting.add_point( Node::merge_id_parts(_id,
				static_cast<Node::LocalId>(iter.position_number())) );
		}
	}
	
	
	void
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
	}
	
	
	void
	Tile::fast_cluster_nodes_search(const GPSPoint& in_gps_point,
		const double in_search_radius, const double in_search_angle,
		std::vector<D_RangeReporting::Id>& out_query_result) const
	{
		/* everything in radian! */
		
		/* compute corner distance.
		 * + 2.0 is needed to provide 100% compatibility to cluster_node_search! */
		double corner_dist = (in_search_radius + 2.0) * sqrt(2.0);
		GeoCoordinate llc = in_gps_point.compute_geo_coordinate_default(225.0,
			corner_dist, GeoCoordinate::_DEGREE, GeoCoordinate::_METER);
		GeoCoordinate urc = in_gps_point.compute_geo_coordinate_default(45.0,
			corner_dist, GeoCoordinate::_DEGREE, GeoCoordinate::_METER);
		
		/* set query rectangle and start the query... */
		rangereporting::Rectangle<GeoCoordinate> query_rectangle;
		query_rectangle.set_corners(llc, urc);
		
		out_query_result.clear();
		_range_reporting.range_query(query_rectangle, out_query_result);
		/* done. */
		
		/* compare distance and angles... */
		std::vector<D_RangeReporting::Id>::iterator iter
			= out_query_result.begin();
		while (iter != out_query_result.end())
		{
			bool erased = false;
			const Node& the_node = node(**iter);
			
			if (the_node.distance_approximated(in_gps_point) > in_search_radius)
			{
				iter = out_query_result.erase(iter);
				erased = true;
			} else
			{
				double min_angle = the_node.minimal_direction_difference_to(in_gps_point);
				if (min_angle > in_search_angle)
				{
					iter = out_query_result.erase(iter);
					erased = true;
				}
			}
				
			if ( !erased )
				++iter;
		}
		/* done. */
	}
	
	
	void
	Tile::fast_cluster_nodes_search(const Segment<GeoCoordinate>& in_segment,
		const double in_search_distance, const double in_search_angle,
		std::vector<D_RangeReporting::Id>& out_query_result) const
	{
		/* everything in radian! */
		
		/* get points... */
		GeoCoordinate point_1 = in_segment.get_points().first;
		GeoCoordinate point_2 = in_segment.get_points().second;
		/* done. */
		
		/* compute distance and bearing... */
		double p2p_bearing = point_1.bearing_default(point_2);

		double corner_dist = (sqrt(2.0) * in_search_distance) / EARTH_RADIUS_M;
		/* done. */
		
		/* compute the four corners of the trapezoid... */
		double current_bearing;
		
		current_bearing = p2p_bearing - (0.75 * PI); // bearing - 135 degree
		GeoCoordinate corner_1 = point_1.compute_geo_coordinate_default(
			current_bearing, corner_dist);
		
		current_bearing = p2p_bearing + (0.75 * PI);
		GeoCoordinate corner_2 = point_1.compute_geo_coordinate_default(
			current_bearing, corner_dist);
		
		current_bearing = p2p_bearing + (0.25 * PI);
		GeoCoordinate corner_3 = point_2.compute_geo_coordinate_default(
			current_bearing, corner_dist);
		
		current_bearing = p2p_bearing - (0.25 * PI);
		GeoCoordinate corner_4 = point_2.compute_geo_coordinate_default(
			current_bearing, corner_dist);
		/* done. */
		
		/* start the query... */
		rangereporting::Trapezoid<GeoCoordinate> query_trapezoid(
			corner_1, corner_2, corner_3, corner_4);
		
		out_query_result.clear();
		_range_reporting.range_query(query_trapezoid, out_query_result);
		/* done. */
		
		/* compare angles... */
/*		std::vector<D_RangeReporting::Id>::iterator iter
			= out_query_result.begin();
		while (iter != out_query_result.end())
		{
			bool erased = false;
			
			double min_angle = node(**iter).minimal_direction_difference_to(in_gps_point);
			if (min_angle > in_search_angle)
			{
				iter = out_query_result.erase(iter);
				erased = true;
			}
			
			if ( !erased )
				++iter;
		}*/
		/* done. */
	}
	
	
	void
	Tile::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _id);
		Serializer::deserialize(i_stream, _nodes);
		
		build_range_reporting_system();
	}
	
	
	
	void
	Tile::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _id);
		Serializer::serialize(o_stream, _nodes);
	}


} // namespace mapgeneration
