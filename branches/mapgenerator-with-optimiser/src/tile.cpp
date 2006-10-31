/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tile.h"

#include <set>

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
		Id northing;
		Id easting;
		split_tile_id(northing, easting);
		
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
		
		rangereporting::AxisParallelRectangle<Node> span_rectangle;
		span_rectangle.set_point(0, llc);
		span_rectangle.set_point(1, urc);
		span_rectangle.validate_corners();
		
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
	
	
 	std::vector<Tile::Id>
	Tile::get_needed_tile_ids(const GeoCoordinate& geo_coordinate,
		const double radius_threshold)
	{
		std::vector<Id> vec_tile_ids;
		
		Id tile_id = get_tile_id_of(geo_coordinate);
		vec_tile_ids.push_back(tile_id);
		
		Id northing;
		Id easting;
		split_tile_id(tile_id, northing, easting);
		
		GeoCoordinate::Representation METER = GeoCoordinate::_METER;
		double distance = 0.0;
		
		// @todo Support north pole
		// puts  all tile_ids  into vec_tile_ids when the distance to the tile
		// border is less the treshold
		distance = distance_to_tile_border(geo_coordinate,
			GeoCoordinate::_NORTH, METER);
		if (distance < radius_threshold)
		{	    	
			vec_tile_ids.push_back(merge_tile_id_parts(northing + 1, easting));
			
			distance = distance_to_tile_border(geo_coordinate,
				GeoCoordinate::_NORTHWEST, METER);
			if (distance < radius_threshold)
				vec_tile_ids.push_back(merge_tile_id_parts(northing + 1, easting - 1));
			
			distance = distance_to_tile_border(geo_coordinate,
				GeoCoordinate::_NORTHEAST, METER);
			if (distance < radius_threshold)
				vec_tile_ids.push_back(merge_tile_id_parts(northing + 1, easting + 1));
		}
		
		// We need this extra if-clause! (NO "else if")
		// Think about (0/0) [or any other "smooth" coordinate]. Otherwise
		// we won't get every needed tile!
		// 
		// I changed it here, because it works. But you can also think about a
		// better concept.
		// 
		// (Another remark on "smooth" coordinates:
		// This methods gives us 9 tiles. Only 4 are needed.)
		distance = distance_to_tile_border(geo_coordinate,
			GeoCoordinate::_SOUTH, METER);
		if (distance < radius_threshold)
		{	
			vec_tile_ids.push_back(merge_tile_id_parts(northing - 1, easting));
			
			distance = distance_to_tile_border(geo_coordinate,
				GeoCoordinate::_SOUTHWEST, METER);
			if (distance < radius_threshold)
				vec_tile_ids.push_back(merge_tile_id_parts(northing - 1, easting - 1));
			
			distance = distance_to_tile_border(geo_coordinate,
				GeoCoordinate::_SOUTHEAST, METER);
			if (distance < radius_threshold)		
				vec_tile_ids.push_back(merge_tile_id_parts(northing - 1, easting + 1));
		}
		
		distance = distance_to_tile_border(geo_coordinate,
			GeoCoordinate::_WEST, METER);
		if (distance < radius_threshold)
			vec_tile_ids.push_back(merge_tile_id_parts(northing, easting - 1));
		
		distance = distance_to_tile_border(geo_coordinate,
			GeoCoordinate::_EAST, METER);
		if (distance < radius_threshold)
			vec_tile_ids.push_back(merge_tile_id_parts(northing, easting + 1));
		
		return vec_tile_ids;
	}
	
	
	std::vector<Tile::Id>
	Tile::get_needed_tile_ids(const GeoCoordinate& gc_1,
		const GeoCoordinate& gc_2,const double radius_threshold)
	{
		// Another big method is just ahead you *g*
		// Will you manage it?!

//		std::cout.setf(std::ios::boolalpha);
//		std::cout.setf(std::ios::fixed);
//		std::cout.precision(25);
		
//		std::cout << "GeoCoordinate 1: Lat = " << gc_1.get_latitude()
//			<< ", Lon= " << gc_1.get_longitude() << std::endl;
//		std::cout << "GeoCoordinate 2: Lat = " << gc_2.get_latitude()
//			<< ", Lon= " << gc_2.get_longitude() << std::endl;

		// First I define a set where I will insert the tile ids I get
		// from various parts of this method.
		// The vector is used whenever a method I call in between returns a vector
		std::set<Id> ids;
		std::vector<Id> container;
		
		// Definitely we need the tile ids the two GeoCoordinates are in.
		container = get_needed_tile_ids(gc_1, radius_threshold);
		ids.insert(container.begin(), container.end());
		
		container = get_needed_tile_ids(gc_2, radius_threshold);
		ids.insert(container.begin(), container.end());
		
		// If the GeoCoordinates are equal, copy the set into a vector and
		// go for a beer!
		if (gc_1 == gc_2)
		{
			std::vector<Id> for_return;
			for_return.insert(for_return.end(), ids.begin(), ids.end());
			return for_return;
		}
		
		// definition: bad constellations
		// 
		// We want to garantuee numeric stability, therefore we claim the
		// gradient m to be less then 36000 resp. greater than -36000. Above
		// resp. below these values I consider the longitude values of the given
		// GeoCoordinates to be too equal (means the difference is too small!).
		// The corresponding cases for the latitude values:
		// m > (1/72000) && m < -(1/72000).
		// These values are somehow arbitrary but they garantuee that the considered
		// line will not pass more than two tile rows/columns in the
		// critical direction (horizontal/vertical).
		// Too complicated, okay, an example:
		// 	- two GeoCoodinates given
		// 	- they have small difference in latitude value
		// 	- that means the gradient is somewhere around zero (and therefore critical)
		// 		and between -(1/72000) < m < (1/72000)
		// 	- we know we have at most 36000 tiles in that direction
		// 	- conclusion: the difference in "tile-height" is at most 0.5
		// 	- next conclusion: we can forget every "tile-row" except the ones the
		// 		two GeoCoordinates are in
		// 
		// If the gradient is critical we will approximate the line by two
		// parallels and calculate their needed-tiles.
		// p1 = a line parallel to the critical direction through gc_1
		// p2 = a line parallel to the critical direction through gc_2
		// 
		// An overhead of tiles but quite comfortable. Do you know a better idea?
		
		bool bad_latitude_constellation = false;
		bool bad_longitude_constellation = false;
		double latitude_difference = gc_2.get_latitude() - gc_1.get_latitude();
		double longitude_difference = gc_2.get_longitude() - gc_1.get_longitude();
		
//		std::cout << "Lat-Diff = " << latitude_difference
//			<< ", Lon-Diff = " << longitude_difference << std::endl;
		
		// tests for bad constellations:
		// I avoid division (bad for numeric stability???)
		// Trust my calculations :-))
		
//		std::cout << "Constellations: ";
		if (latitude_difference > 0.0)
		{
//			std::cout << "lat > 0; ";
			if (longitude_difference > 0.0)
			{
//				std::cout << "lon > 0; ";
				// latitude:
				if (longitude_difference > 72000.0 * latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				// longitude:
				if (latitude_difference > 36000.0 * longitude_difference)
				{
//					std::cout << "bad_lon_const; ";
					bad_longitude_constellation = true;
				}
			} else if (longitude_difference == 0.0)
			{
//				std::cout << "lon = 0; ";
//				std::cout << "bad_lon_const; ";
				bad_longitude_constellation = true;
			} else // explicit: if (longitude_difference < 0)
			{
//				std::cout << "lon < 0; ";
				// latitude:
				if (-longitude_difference > 72000.0 * latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				// longitude:
				if (latitude_difference > 36000.0 * -longitude_difference)
				{
//					std::cout << "bad_lon_const; ";
					bad_longitude_constellation = true;
				}
			}
		} else if (latitude_difference == 0.0)
		{
//			std::cout << "lat = 0; ";
			if (longitude_difference > 0.0)
			{
//				std::cout << "bad_lon_const; ";
				bad_latitude_constellation = true;
			} else if (longitude_difference == 0.0)
			{
//				std::cout << "MUST NOT HAPPEN; ";
				// MUST NOT happen!!!
			} else // explicit: if (longitude_difference < 0)
			{
//				std::cout << "bad_lat_const; ";
				bad_latitude_constellation = true;
			}
		} else // explicit: if (latitude_difference < 0.0)
		{
//			std::cout << "lat < 0; ";
			if (longitude_difference > 0.0)
			{
//				std::cout << "lon > 0; ";
				// latitude:
				if (longitude_difference > 72000.0 * -latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				// longitude:
				if (-latitude_difference > 36000.0 * longitude_difference)
				{
//					std::cout << "bad_lon_const; ";
					bad_longitude_constellation = true;
				}
			} else if (longitude_difference == 0.0)
			{
//				std::cout << "lon = 0; ";
//				std::cout << "bad_lon_const; ";
				bad_longitude_constellation = true;
			} else // explicit: if (longitude_difference < 0)
			{
	//			std::cout << "lon < 0; ";
				// latitude:
				if (-longitude_difference > 72000.0 * -latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				// longitude:
				if (-latitude_difference > 36000.0 * -longitude_difference)
				{
//					std::cout << "bad_lon_const; ";
					bad_longitude_constellation = true;
				}
			}
		}
//		std::cout << std::endl;
		
//		std::cout << "Bad constellations:" << std::endl
//			<< "\tlatitude:  " << bad_latitude_constellation << std::endl
//			<< "\tlongitude: " << bad_longitude_constellation << std::endl
//			<< std::endl;
		
		// calculate the tile borders with might be intersected.
		// Actually I set a bounding box around the two GeoCoordinates. Then
		// I will calculate the intersection points with the tile borders which
		// are within the bounding box.
		// 
		// Caution: That is an "inner" bounding box. What is that? Hard to explain,
		// try to understand by looking at the code.
		// And take in consideration that the tile borders are processed from
		// small to large latitude resp longitude (with a "cycle shift" for
		// certain longitude value combinations).
		double latitude_iter;
		double latitude_iter_end;
		if (gc_1.get_latitude() > gc_2.get_latitude())
		{
			latitude_iter = ceil(gc_2.get_latitude() * 100) / 100;
			latitude_iter_end = floor(gc_1.get_latitude() * 100) / 100;
		} else if (gc_1.get_latitude() == gc_2.get_latitude())
		{
			// Not nessecary! It would be enough to set latitude_iter > 
			// latitude_iter_end. In fact that construction does that!
			latitude_iter = ceil(gc_1.get_latitude() * 100) / 100;
			latitude_iter_end = floor(gc_1.get_latitude() * 100) / 100;
		} else // explicit: if (gc_1.get_latitude() < gc_2.get_latitude())
		{
			latitude_iter = ceil(gc_1.get_latitude() * 100) / 100;
			latitude_iter_end = floor(gc_2.get_latitude() * 100) / 100;
		}

		double longitude_iter;
		double longitude_iter_end;
		if (gc_1.get_longitude() > gc_2.get_longitude())
		{
			if (gc_1.get_longitude() - gc_2.get_longitude() <= 180.0)
			{
				longitude_iter = ceil(gc_2.get_longitude() * 100) / 100;
				longitude_iter_end = floor(gc_1.get_longitude() * 100) / 100;
			} else
			{
				longitude_iter = ceil(gc_1.get_longitude() * 100) / 100;

				// the "cycle shift" I spoke on/of (???)
				// Now longitude_iter_end has a value greater than 180 degrees.
				longitude_iter_end = floor((gc_2.get_longitude() + 360.0) * 100) / 100;
			}
		} else if (gc_1.get_longitude() == gc_2.get_longitude())
		{
			// Not nessecary! It would be enough to set longitude_iter > 
			// longitude_iter_end. In fact that construction does that!
			longitude_iter = ceil(gc_1.get_longitude() * 100) / 100;
			longitude_iter_end = floor(gc_1.get_longitude() * 100) / 100;
		} else // explicit: if (gc_1.get_longitude() < gc_2.get_longitude())
		{
			if (gc_2.get_longitude() - gc_1.get_longitude() <= 180.0)
			{
				longitude_iter = ceil(gc_1.get_longitude() * 100) / 100;
				longitude_iter_end = floor(gc_2.get_longitude() * 100) / 100;
			} else
			{
				longitude_iter = ceil(gc_2.get_longitude() * 100) / 100;
				longitude_iter_end = floor((gc_1.get_longitude() + 360.0) * 100) / 100;
			}
		}
		
		// First we deal with the bad constellations
		if (bad_latitude_constellation || bad_longitude_constellation)
		{
			double value_iter; // either the latitude_iter or longitude_iter
			double value_iter_end; // dito
			GeoCoordinate gc_1_iter; // an iterator for the first parallel line
			GeoCoordinate gc_2_iter; // an iterator for the second parallel line
			
			if (bad_latitude_constellation)
			{
				value_iter = longitude_iter;
				value_iter_end = longitude_iter_end;
			} else
			{
				value_iter = latitude_iter;
				value_iter_end = latitude_iter_end;
			}
		
			for (; value_iter < value_iter_end; value_iter += 0.01)
			{
				// if value_iter's value is greater then 180 degrees
				// "decycle the shift". Or type error message if we have
				// the wrong bad constellation.
				double correct_value = value_iter;
				if (value_iter > 180.0)
				{
					if (bad_latitude_constellation)
						correct_value = -360 + value_iter;
					else
						mlog(MLog::error, "GeoCoordinate") << "SHOULD NOT HAPPEN!!!\n";
				}
				
				// Change the GeoCoordinate iterator latitude/longitude
				// according the the given bad constellation
				if (bad_latitude_constellation)
				{
					gc_1_iter.set_longitude(correct_value);
					gc_2_iter.set_longitude(correct_value);
				} else
				{
					gc_1_iter.set_latitude(correct_value);
					gc_2_iter.set_latitude(correct_value);
				}
				
				// use the given method and insert the tile ids into the set
				container = get_needed_tile_ids(gc_1_iter, radius_threshold);
				ids.insert(container.begin(), container.end());
				
				container = get_needed_tile_ids(gc_2_iter, radius_threshold);
				ids.insert(container.begin(), container.end());
			} // end for
			
		} else
		{
			//"normal" situation:
			// Now it's save to calculate the gradient and y axis
			// intersection point
			double gradient = latitude_difference / longitude_difference;
			double y_intersection_point = gc_1.get_latitude()
				- (gradient * gc_1.get_longitude());
			
			// Control the values against a given EPSILON
			double control_y_intersection_point = gc_2.get_latitude()
				- (gradient * gc_2.get_longitude());
			if (y_intersection_point != control_y_intersection_point)
			{
//				mlog(MLog::debug, "GeoCoordinate") << "y intersections point NOT equal.\n";
				
				const double EPSILON = 0.00000001;
				double diff = y_intersection_point - control_y_intersection_point;
				if ((diff < -EPSILON) || (diff > EPSILON))
				{
					mlog(MLog::error, "GeoCoordinate") << "gc_1's and gc_2's y intersection "
						<< "points differ to much! (EPSILON = " << EPSILON << ")\n";
				}
			}
			
			double calculated_latitude;
			double calculated_longitude;
			GeoCoordinate calculated_gc;
			
			// the big loop over the tile borders:
			// calculate the missing argument, change the calculated_gc, obtain
			// the tile ids, insert them into the set, be happy...
			for (; latitude_iter < latitude_iter_end; latitude_iter += 0.01)
			{
				calculated_longitude = (latitude_iter - y_intersection_point) / gradient;
				
				calculated_gc.set_latitude(latitude_iter);
				calculated_gc.set_longitude(calculated_longitude);
				
				container = get_needed_tile_ids(calculated_gc, radius_threshold);
				ids.insert(container.begin(), container.end());
			}
			
			for (; longitude_iter < longitude_iter_end; longitude_iter += 0.01)
			{
				calculated_latitude = gradient * longitude_iter + y_intersection_point;
				
				calculated_gc.set_latitude(calculated_latitude);
				calculated_gc.set_longitude(longitude_iter);
				
				container = get_needed_tile_ids(calculated_gc, radius_threshold);
				ids.insert(container.begin(), container.end());
			}
		}
		
		// Now make a vector again from the set
		// (because it's the return value; but don't ask ME why? We have vectors
		// everywhere :-)
		std::vector<Id> for_return;
		for_return.insert(for_return.end(), ids.begin(), ids.end());
		
/*		std::vector<unsigned int>::iterator iter = for_return.begin();
		for (; iter != for_return.end(); ++iter)
		{
			std::cout << *iter << " ";
		}
		std::cout << std::endl;*/
		
		return for_return;
		
		// Did you forget the beer. ME NOT. Cheers!
	}
	
	
/*	void
	Tile::fast_cluster_nodes_search(const GeoCoordinate& in_geo_coordinate,
		const double in_search_radius, const double in_search_angle,
		std::vector<Node::Id>& out_query_results) const
	{
		double corner_dist = in_search_radius * sqrt(2.0);
		Node llc(in_geo_coordinate.compute_geo_coordinate(
			225.0, corner_dist, GeoCoordinate::_DEGREE, GeoCoordinate::_METER));
		Node urc(in_geo_coordinate.compute_geo_coordinate(
			45.0, corner_dist, GeoCoordinate::_DEGREE, GeoCoordinate::_METER));
		
		// set query rectangle, set up temporary result vector
		// and start the query...
		rangereporting::AxisParallelRectangle<Node> query_rectangle;
		query_rectangle.set_point(0, llc);
		query_rectangle.set_point(1, urc);
		query_rectangle.validate_corners();
		
		std::vector<D_IndexType> quadtree_results;
		_quadtree.range_query(query_rectangle, quadtree_results);
		// done.
		
		// compose Node::Ids, compare distance and angles...
		out_query_results.clear();
		
		std::vector<D_IndexType>::iterator iter = quadtree_results.begin();
		std::vector<D_IndexType>::iterator iter_end = quadtree_results.end();
		for (; iter != iter_end; ++iter)
		{
			// downcast: needed when D_IndexType is 64bit long, but
			// Node::LocalId only 32bit
			Node::LocalId the_node_local_id = static_cast<Node::LocalId>(*iter);
			const Node& the_node = node(the_node_local_id);
			
			bool use_this_iter = within_search_angle(in_geo_coordinate,
				the_node, in_search_angle);
			if (use_this_iter)
			{
				use_this_iter = within_search_radius(in_geo_coordinate,
					the_node, in_search_radius);
			}
			
			if (use_this_iter)
			{
				Node::Id node_id
					= Node::merge_id_parts(get_id(), the_node_local_id);
				out_query_results.push_back(node_id);
			}
		}
	}*/
	
	
	void
	Tile::fast_cluster_nodes_search(const GeoCoordinate& gc,
		const double in_search_distance,
		std::vector<Node::Id>& out_query_result) const
	{
		rangereporting::Quadrangle<GeoCoordinate> query_quadrangle;
		query_quadrangle.build_quadrangle(gc, in_search_distance);
		
		fast_cluster_nodes_search(query_quadrangle, in_search_distance, PI, 0.0,
			out_query_result);
	}
	
	void
	Tile::fast_cluster_nodes_search(const Segment<GeoCoordinate>& in_segment,
		const double in_search_distance, const double in_search_angle,
		std::vector<Node::Id>& out_query_results) const
	{
		rangereporting::Quadrangle<GeoCoordinate> query_quadrangle;
		query_quadrangle.build_quadrangle(in_segment,
			in_search_distance, 0.0, in_search_distance);
		
		double compare_to_angle = in_segment.get_point(0).bearing(
				in_segment.get_point(1));
		
		fast_cluster_nodes_search(query_quadrangle, in_search_distance,
			in_search_angle, compare_to_angle, out_query_results);
	}
	
	
	void
	Tile::fast_cluster_nodes_search(const Quadrangle<GeoCoordinate>& in_quadrangle,
	const double in_search_distance, const double in_max_search_angle,
	const double in_compare_to_angle,
	std::vector<Node::Id>& out_query_results) const
	{
		rangereporting::Quadrangle<Node> query_quadrangle;
		query_quadrangle.set_point(0, in_quadrangle.get_point(0));
		query_quadrangle.set_point(1, in_quadrangle.get_point(1));
		query_quadrangle.set_point(2, in_quadrangle.get_point(2));
		query_quadrangle.set_point(3, in_quadrangle.get_point(3));
		
		std::vector<D_IndexType> quadtree_results;
		_quadtree.range_query(query_quadrangle, quadtree_results);
		// done.
		
		// compose Node::Ids, compare angles...
		out_query_results.clear();

		std::vector<D_IndexType>::iterator iter = quadtree_results.begin();
		std::vector<D_IndexType>::iterator iter_end = quadtree_results.end();
		for (; iter != iter_end; ++iter)
		{
			// downcast: needed when D_IndexType is 64bit long, but
			// Node::LocalId only 32bit
			Node::LocalId the_node_local_id = static_cast<Node::LocalId>(*iter);
			const Node& the_node = node(the_node_local_id);
			
			mapgeneration_util::Direction compare_direction(in_compare_to_angle);
			double min_direction_difference
				= the_node.minimal_direction_difference_to(compare_direction);
			
			if (min_direction_difference <= in_max_search_angle)
			{
				Node::Id node_id
					= Node::merge_id_parts(get_id(), the_node_local_id);
				out_query_results.push_back(node_id);
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
	
	
	
	double
	Tile::distance_to_tile_border(const GeoCoordinate& geo_coordinate,
		const GeoCoordinate::Heading heading,
		const GeoCoordinate::Representation output_representation)
	{
		double latitude = geo_coordinate.get_latitude();
		double longitude = geo_coordinate.get_longitude();
		
		GeoCoordinate compare_point(geo_coordinate);
		
		// calculation of a GeoCoordinate on the north, east, south,
		// west, NE, NW, SE, SW  tile  border.
		switch (heading)
		{
			case GeoCoordinate::_NORTH:
				compare_point.set_latitude(ceil(latitude * 100.0) / 100.0);
				break;
			case GeoCoordinate::_EAST:
				compare_point.set_longitude(ceil(longitude * 100.0) / 100.0);
				break;
			case GeoCoordinate::_SOUTH:
				compare_point.set_latitude(floor(latitude * 100.0) / 100.0);
				break;
			case GeoCoordinate::_WEST:
				compare_point.set_longitude(floor(longitude * 100.0) / 100.0);
				break;
				
			case GeoCoordinate::_NORTHWEST:			
				compare_point.set_latitude(ceil(latitude * 100.0) / 100.0);
				compare_point.set_longitude(floor(longitude * 100.0) / 100.0);
				break;
			case GeoCoordinate::_NORTHEAST:
				compare_point.set_latitude(ceil(latitude * 100.0) / 100.0);
				compare_point.set_longitude(ceil(longitude * 100.0) / 100.0);
				break;			
			case GeoCoordinate::_SOUTHWEST:
				compare_point.set_latitude(floor(latitude * 100.0) / 100.0);			
				compare_point.set_longitude(floor(longitude * 100.0) / 100.0);
				break;
			case GeoCoordinate::_SOUTHEAST:
				compare_point.set_latitude(floor(latitude * 100.0) / 100.0);			
				compare_point.set_longitude(ceil(longitude * 100.0) / 100.0);
				break;
		}
		
		// calculation  of the distance between  the current GeoCoordinate 
		// and the calculated tile border coordinate
		return geo_coordinate.distance_approximated(
			compare_point, output_representation);
	}
	
	
	void
	Tile::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _id);
		Serializer::serialize(o_stream, _quadtree);
	}
		
} // namespace mapgeneration
