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
			double distance = iter->second.approximated_distance(in_gps_point);
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
		
		/* 13 trigonometrical computations */
		
		double lat1 = (in_gps_point.get_latitude() / 180.0) * PI;
		double lon1 = (in_gps_point.get_longitude() / 180.0) * PI;
		
		double cos_lat1 = cos(lat1);
		double sin_lat1 = sin(lat1);
		double cos_lon1 = cos(lon1);
		double sin_lon1 = sin(lon1);

		double dist = (sqrt(2.0) * (in_search_radius + 2.0)) / EARTH_RADIUS_M;
		double cos_dist = cos(dist);
		double sin_dist = sin(dist);
		
		double angle;
		double acos_xyz;
		
		/* compute upper right corner */
		angle = PI / 4.0; // 45 degrees
		double lat2_ur = asin( cos(angle) * cos_lat1 * sin_dist + sin_lat1 * cos_dist );
		acos_xyz = acos( (cos_dist - sin_lat1 * sin(lat2_ur)) / (cos_lat1 * cos(lat2_ur)) );
		double lon2_ur = lon1 + acos_xyz;
		
		GeoCoordinate urc;
		urc.set_latitude( (lat2_ur / PI) * 180.0 );
		urc.set_longitude( (lon2_ur / PI) * 180.0 );
		/* done */
		
		/*compute lower right corner */
		angle += PI; // 135 degrees
		double lat2_ll = asin( cos(angle) * cos_lat1 * sin_dist + sin_lat1 * cos_dist );
		double lon2_ll = lon1 - acos_xyz;
		
		GeoCoordinate llc;
		llc.set_latitude( (lat2_ll / PI) * 180.0 );
		llc.set_longitude( (lon2_ll / PI) * 180.0 );
		/* done */
		
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
			
			if (the_node.approximated_distance(in_gps_point) > in_search_radius)
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
