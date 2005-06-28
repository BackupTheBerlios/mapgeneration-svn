/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "geocoordinate.h"

#include <ctime>
#include <cmath>
#include <set>

namespace mapgeneration
{
	
	std::ostream&
	operator<<(std::ostream& out, const GeoCoordinate& geo_coordinate)
	{
		std::ios::fmtflags original_flags = out.flags();
		std::streamsize original_precision = out.precision();
		
		out.setf(std::ios::fixed);
		out.precision(10);
		
		out << "GeoCoordinate: (" << geo_coordinate[0] << ", "
			<< geo_coordinate[1] << ", "
			<< geo_coordinate[2] << ") [lat, lon, alt]";
		
		out.flags(original_flags);
		out.precision(original_precision);
		
		return out;
	}
	
	
	double
	GeoCoordinate::bearing_approximated(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		double cos_lat1 = cos(_latitude * d2r);
		
		double lat_diff = (geo_coordinate._latitude - _latitude) * d2r;
		double lon_diff = (geo_coordinate._longitude - _longitude) * d2r;
		
		double bearing = atan2(cos_lat1 * lon_diff, lat_diff);
		
		/* make a mathematical angle */
		bearing = PI / 2.0 - bearing;
		
		normalise_arc(bearing, _RADIAN);
		convert(bearing, _RADIAN, output_representation);
		
		return bearing;
	}
	
	
	double
	GeoCoordinate::bearing_on_great_circle(const GeoCoordinate& geo_coordinate,
		const Representation output_representation, const double at_point) const
	{
		/* Do not call interpolate_on_great_circle for the trivial value... */
		GeoCoordinate start_gc;
		if (at_point == 0.0)
			start_gc = *this;
		else 
			/* I do not test for at_point==1.0 because if start_gc==end_gc this
			 * method cannot return a reasonable bearing! */
			start_gc = interpolate_on_great_circle(*this, geo_coordinate,
				1.0 - at_point);
		/* done. */
		
		double lat1 = start_gc._latitude * d2r;
		double lat2 = geo_coordinate._latitude * d2r;
		double lon_diff = (geo_coordinate._longitude - start_gc._longitude) * d2r;
		
		double dist = start_gc.distance_on_great_circle(geo_coordinate, _RADIAN);
		
		double bearing = acos(
			(sin(lat2) - sin(lat1) * cos(dist)) / (cos(lat1) * sin(dist))
		);
		
		if (sin(lon_diff) < 0.0)
			bearing = 2.0 * PI - bearing;
		
		/* make a mathematical angle */
		bearing = PI / 2.0 - bearing;
		
		normalise_arc(bearing, _RADIAN);
		convert(bearing, _RADIAN, output_representation);
		
		return bearing;
	}
	
	
	double
	GeoCoordinate::bearing_on_rhumb_line(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		/* everything in radians! */
		
		double lat1 = _latitude * d2r;
		double lat2 = geo_coordinate._latitude * d2r;
		double lat_diff = lat2 - lat1;
		
		double lon_diff_W = (_longitude - geo_coordinate._longitude) * d2r;
		double lon_diff_E = -lon_diff_W;
		
		if (lon_diff_W < 0.0)
			lon_diff_W += (2.0 * PI);
		if (lon_diff_E < 0.0)
			lon_diff_E += (2.0 * PI);
		
		double dphi = log( tan(lat2/2.0 + PI/4.0) / tan(lat1/2.0 + PI/4.0) );
		
		/** @todo outsource EPSILON! I used it here and there. */
		double EPSILON = 0.00000001; // arbitrary
		double q;
		if ( fabs(lat_diff) < EPSILON )
			q = cos(lat1);
		else
			q = lat_diff / dphi;
		
		double bearing;
		if (lon_diff_W < lon_diff_E)
			bearing = atan2(-lon_diff_W, dphi);
		else
			bearing = atan2(lon_diff_E, dphi);
		
		if (bearing < 0.0)
			bearing += 2.0 * PI;
		
		/* make a mathematical angle */
		bearing = PI / 2.0 - bearing;
		
		normalise_arc(bearing, _RADIAN);
		convert(bearing, _RADIAN, output_representation);
		
		return bearing;
	}
	
	
	GeoCoordinate
	GeoCoordinate::compute_geo_coordinate_approximated(double bearing,
		double distance,
		const GeoCoordinate::Representation bearing_representation,
		const GeoCoordinate::Representation distance_representation) const
	{
		/* test representations and convert to radian... */
		convert(bearing, bearing_representation, _RADIAN);
		convert(distance, distance_representation, _RADIAN);
		/* done. */
		
		/* convert mathematical bearing to navigation bearing... */
		bearing = (PI / 2.0) - bearing;
		/* done */
		
		/* convert my coordinates to radians... */
		double lat0 = _latitude * d2r;
		double lon0 = _longitude * d2r;
		/* done. */
		
		/* calculate new values... */
		double lat_new = lat0 + distance * cos(bearing);
		double lon_new = lon0 + distance * sin(bearing) / cos(lat0);
		/* done. */
		
		/* convert to degree and return... */
		GeoCoordinate gc_new;
		gc_new.set_latitude(lat_new * r2d);
		gc_new.set_longitude(lon_new * r2d);
		
		return gc_new;
		/* done. */
	}
	
	
	GeoCoordinate
	GeoCoordinate::compute_geo_coordinate_on_great_circle(double starting_bearing,
		double distance,
		const GeoCoordinate::Representation bearing_representation,
		const GeoCoordinate::Representation distance_representation) const
	{
		/* test representations and convert to radian... */
		convert(starting_bearing, bearing_representation, _RADIAN);
		convert(distance, distance_representation, _RADIAN);
		
		double bearing = starting_bearing;
		/* done. */
		
		/* convert mathematical bearing to navigation bearing... */
		bearing = (PI / 2.0) - bearing;
		/* done */
		
		/* convert my coordinates to radians and precompute some cos ans sins... */
		double lat0 = _latitude * d2r;
		double lon0 = _longitude * d2r;
		
		double cos_lat0 = cos(lat0);
		double sin_lat0 = sin(lat0);
		
		double cos_dist = cos(distance);
		double sin_dist = sin(distance);
		/* done. */
		
		/* calculate new values... */
		double lat_new = asin(
			sin_lat0 * cos_dist + cos_lat0 * sin_dist * cos(bearing) );
		double dlon = atan2(
			sin(bearing) * sin_dist * cos_lat0, cos_dist - sin_lat0 * sin(lat_new) );
		double lon_new = lon0 + dlon;
		/* done. */
		
		/* convert to degree and return... */
		GeoCoordinate gc_new;
		gc_new.set_latitude(lat_new * r2d);
		gc_new.set_longitude(lon_new * r2d);
		
		return gc_new;
		/* done. */
	}
	
	
	GeoCoordinate
	GeoCoordinate::compute_geo_coordinate_on_rhumb_line(double bearing,
		double distance,
		const GeoCoordinate::Representation bearing_representation,
		const GeoCoordinate::Representation distance_representation) const
	{
		/* test representations and convert to radian... */
		convert(bearing, bearing_representation, _RADIAN);
		convert(distance, distance_representation, _RADIAN);
		/* done. */
		
		/* convert mathematical bearing to navigation bearing... */
		bearing = (PI / 2.0) - bearing;
		/* done */
		
		/* convert my coordinates to radians... */
		double lat0 = _latitude * d2r;
		double lon0 = _longitude * d2r;
		/* done. */
		
		/* calculate values...*/
		double lat_new = lat0 + distance * cos(bearing);
		double lat_diff = lat0 - lat_new;
		
		double dphi = log( tan(lat_new/2.0 + PI/4.0) / tan(lat0/2.0 + PI/4.0) );
		
		double EPSILON = 0.00000001; // arbitrary
		double q;
		if ( fabs(lat_diff) < EPSILON )
			q = cos(lat0);
		else
			q = lat_diff / dphi;
		
		double dlon = -distance * sin(bearing) / q;
		double lon_new = lon0 + dlon;
		/* done. */
		
		/* convert to degree and return... */
		GeoCoordinate gc_new;
		gc_new.set_latitude(lat_new * r2d);
		gc_new.set_longitude(lon_new * r2d);
		
		return gc_new;
		/* done. */
	}
	
	
	double
	GeoCoordinate::distance_approximated(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		double cos_lat1 = cos(_latitude * d2r);
		
		double dist_N = (geo_coordinate._latitude - _latitude);
		double dist_E = cos_lat1 * (geo_coordinate._longitude - _longitude);
		
		double distance = sqrt(dist_N * dist_N + dist_E * dist_E);
		convert(distance, _DEGREE, output_representation);
		
		return distance;
	}
	
	
	double
	GeoCoordinate::distance_on_great_circle(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		/* calculates the distance between two GeoCoordinates with the
		 * Great circle distance formula */

		double lat1 = _latitude * d2r;
		double lat2 = geo_coordinate._latitude * d2r;
		
		double lon1 = _longitude * d2r;
		double lon2 = geo_coordinate._longitude * d2r;
		
		double sin_average_lat = sin( (lat1 - lat2) / 2.0 );
		double sin_average_lon = sin( (lon1 - lon2) / 2.0 );
			
		double distance = 2.0 * asin( sqrt(sin_average_lat * sin_average_lat
			+ cos(lat1) * cos(lat2) * sin_average_lon * sin_average_lon) );
		
		convert(distance, _RADIAN, output_representation);
		
		return distance;
	}
	
	
	double
	GeoCoordinate::distance_on_rhumb_line(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		/* everything in radians! */
		
		double lat1 = _latitude * d2r;
		double lat2 = geo_coordinate._latitude * d2r;
		double lat_diff = lat2 - lat1;
		
		double lon_diff_W = (geo_coordinate._longitude - _longitude) * d2r;
		double lon_diff_E = -lon_diff_W;

		if (lon_diff_W < 0.0)
			lon_diff_W += (2.0 * PI);
		if (lon_diff_E < 0.0)
			lon_diff_E += (2.0 * PI);
		
		double dphi = log( tan(lat2/2.0 + PI/4.0) / tan(lat1/2.0 + PI/4.0) );
		
		double EPSILON = 0.00000001; // arbitrary
		double q;
		if ( fabs(lat_diff) < EPSILON )
			q = cos(lat1);
		else
			q = (lat_diff) / dphi;
		
		double distance;
		if (lon_diff_W < lon_diff_E)
			distance = sqrt(q * q * lon_diff_W * lon_diff_W + lat_diff * lat_diff);
		else
			distance = sqrt(q * q * lon_diff_E * lon_diff_E + lat_diff * lat_diff);
		
		convert(distance, _RADIAN, output_representation);
		
		return distance;
	}
	
	
	double
	GeoCoordinate::distance_to_tile_border(const Heading heading,
		const Representation output_representation) const
	{
		GeoCoordinate compare_point(*this);
		
		/* calculation of a GeoCoordinate on the north, east, south,
		 * west, NE, NW, SE, SW  tile  border. */
		switch (heading)
		{
			case _NORTH:
				compare_point.set_latitude(ceil(get_latitude() * 100) / 100);
				break;
			case _EAST:
				compare_point.set_longitude(ceil(get_longitude() * 100) / 100);
				break;
			case _SOUTH:
				compare_point.set_latitude(floor(get_latitude() * 100) / 100);
				break;
			case _WEST:
				compare_point.set_longitude(floor(get_longitude() * 100) / 100);
				break;
				
			case _NORTHWEST:			
				compare_point.set_latitude(ceil(get_latitude() * 100) / 100);
				compare_point.set_longitude(floor(get_longitude() * 100) / 100);
				break;
			case _NORTHEAST:
				compare_point.set_latitude(ceil(get_latitude() * 100) / 100);
				compare_point.set_longitude(ceil(get_longitude() * 100) / 100);
				break;			
			case _SOUTHWEST:
				compare_point.set_latitude(floor(get_latitude() * 100) / 100);			
				compare_point.set_longitude(floor(get_longitude() * 100) / 100);
				break;
			case _SOUTHEAST:
				compare_point.set_latitude(floor(get_latitude() * 100) / 100);			
				compare_point.set_longitude(ceil(get_longitude() * 100) / 100);
				break;
		}
		
		/* calculation  of the distance between  the current GeoCoordinate 
		 * and the calculated tile border coordinate */
		return distance_approximated(compare_point, output_representation);
	}
	
	
 	std::vector<unsigned int>
	GeoCoordinate::get_needed_tile_ids(const double radius_threshold) const
	{				
		std::vector<unsigned int> vec_tile_ids;
		
		unsigned int tile_id = get_tile_id();
		vec_tile_ids.push_back(tile_id);
		
		int northing;
		int easting;
		split_tile_id(tile_id, northing, easting);
		
		/** @todo Support north pole */
		/* puts  all tile_ids  into vec_tile_ids when the distance to the tile
		 * border is less the treshold */
		if (distance_to_tile_border(_NORTH, _METER) < radius_threshold)
		{	    	
			vec_tile_ids.push_back(merge_tile_id_parts(northing + 1, easting));
		
			if (distance_to_tile_border(_NORTHWEST, _METER) < radius_threshold)
				vec_tile_ids.push_back(merge_tile_id_parts(northing + 1, easting - 1));
		  
			if (distance_to_tile_border(_NORTHEAST, _METER) < radius_threshold)
				vec_tile_ids.push_back(merge_tile_id_parts(northing + 1, easting + 1));
		}
		
		/* We need this extra if-clause! (NO "else if")
		 * Think about (0/0) [or any other "smooth" coordinate]. Otherwise
		 * we won't get every needed tile!
		 * 
		 * I changed it here, because it works. But you can also think about a
		 * better concept.
		 * 
		 * (Another remark on "smooth" coordinates:
		 * This methods gives us 9 tiles. Only 4 are needed.) */
		if (distance_to_tile_border(_SOUTH, _METER) < radius_threshold)
		{	
			vec_tile_ids.push_back(merge_tile_id_parts(northing - 1, easting));
			
			if (distance_to_tile_border(_SOUTHWEST, _METER) < radius_threshold)
				vec_tile_ids.push_back(merge_tile_id_parts(northing - 1, easting - 1));
			
			if (distance_to_tile_border(_SOUTHEAST, _METER) < radius_threshold)		
				vec_tile_ids.push_back(merge_tile_id_parts(northing - 1, easting + 1));
		}
		
		if (distance_to_tile_border(_WEST, _METER) < radius_threshold)
			vec_tile_ids.push_back(merge_tile_id_parts(northing, easting - 1));
		
		if (distance_to_tile_border(_EAST, _METER) < radius_threshold)
			vec_tile_ids.push_back(merge_tile_id_parts(northing, easting + 1));
		
		return vec_tile_ids;
	}
	
	
	std::vector<unsigned int>
	GeoCoordinate::get_needed_tile_ids(
		const GeoCoordinate& gc_1,
		const GeoCoordinate& gc_2,
		const double radius_threshold
	)
	{
		/* Another big method is just ahead you *g*
		 * Will you manage it?! */

//		std::cout.setf(std::ios::boolalpha);
//		std::cout.setf(std::ios::fixed);
//		std::cout.precision(25);
		
//		std::cout << "GeoCoordinate 1: Lat = " << gc_1.get_latitude()
//			<< ", Lon= " << gc_1.get_longitude() << std::endl;
//		std::cout << "GeoCoordinate 2: Lat = " << gc_2.get_latitude()
//			<< ", Lon= " << gc_2.get_longitude() << std::endl;

		/* First I define a set where I will insert the tile ids I get from various
		 * parts of this method.
		 * The vector is used whenever a method I call in between returns a vector */
		std::set<unsigned int> ids;
		std::vector<unsigned int> container;
		
		/* Definitely we need the tile ids the two GeoCoordinates are in. */
		container = gc_1.get_needed_tile_ids(radius_threshold);
		ids.insert(container.begin(), container.end());
		
		container = gc_2.get_needed_tile_ids(radius_threshold);
		ids.insert(container.begin(), container.end());
		
		/* If the GeoCoordinates are equal, copy the set into a vector and go for
		 * a beer! */
		if (gc_1 == gc_2)
		{
			std::vector<unsigned int> for_return;
			for_return.insert(for_return.end(), ids.begin(), ids.end());
			return for_return;
		}
		
		/* definition: bad constellations
		 * 
		 * We want to garantuee numeric stability, therefore we claim the
		 * gradient m to be less then 36000 resp. greater than -36000. Above
		 * resp. below these values I consider the longitude values of the given
		 * GeoCoordinates to be too equal (means the difference is too small!).
		 * The corresponding cases for the latitude values:
		 * m > (1/72000) && m < -(1/72000).
		 * These values are somehow arbitrary but they garantuee that the considered
		 * line will not pass more than two tile rows/columns in the
		 * critical direction (horizontal/vertical).
		 * Too complicated, okay, an example:
		 * 	- two GeoCoodinates given
		 * 	- they have small difference in latitude value
		 * 	- that means the gradient is somewhere around zero (and therefore critical)
		 * 		and between -(1/72000) < m < (1/72000)
		 * 	- we know we have at most 36000 tiles in that direction
		 * 	- conclusion: the difference in "tile-height" is at most 0.5
		 * 	- next conclusion: we can forget every "tile-row" except the ones the
		 * 		two GeoCoordinates are in
		 * 
		 * If the gradient is critical we will approximate the line by two
		 * parallels and calculate their needed-tiles.
		 * p1 = a line parallel to the critical direction through gc_1
		 * p2 = a line parallel to the critical direction through gc_2
		 * 
		 * An overhead of tiles but quite comfortable. Do you know a better idea? */
		
		bool bad_latitude_constellation = false;
		bool bad_longitude_constellation = false;
		double latitude_difference = gc_2.get_latitude() - gc_1.get_latitude();
		double longitude_difference = gc_2.get_longitude() - gc_1.get_longitude();
		
//		std::cout << "Lat-Diff = " << latitude_difference
//			<< ", Lon-Diff = " << longitude_difference << std::endl;
		
		/* tests for bad constellations:
		 * I avoid division (bad for numeric stability???)
		 * Trust my calculations :-)) */
		
//		std::cout << "Constellations: ";
		if (latitude_difference > 0.0)
		{
//			std::cout << "lat > 0; ";
			if (longitude_difference > 0.0)
			{
//				std::cout << "lon > 0; ";
				/* latitude: */
				if (longitude_difference > 72000.0 * latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				/* longitude: */
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
				/* latitude: */
				if (-longitude_difference > 72000.0 * latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				/* longitude: */
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
				/* latitude: */
				if (longitude_difference > 72000.0 * -latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				/* longitude: */
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
				/* latitude: */
				if (-longitude_difference > 72000.0 * -latitude_difference)
				{
//					std::cout << "bad_lat_const; ";
					bad_latitude_constellation = true;
				}
				
				/* longitude: */
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
		
		/* calculate the tile borders with might be intersected.
		 * Actually I set a bounding box around the two GeoCoordinates. Then
		 * I will calculate the intersection points with the tile borders which
		 * are within the bounding box.
		 * 
		 * Caution: That is an "inner" bounding box. What is that? Hard to explain,
		 * try to understand by looking at the code.
		 * And take in consideration that the tile borders are processed from
		 * small to large latitude resp longitude (with a "cycle shift" for
		 * certain longitude value combinations). */
		double latitude_iter;
		double latitude_iter_end;
		if (gc_1.get_latitude() > gc_2.get_latitude())
		{
			latitude_iter = ceil(gc_2.get_latitude() * 100) / 100;
			latitude_iter_end = floor(gc_1.get_latitude() * 100) / 100;
		} else if (gc_1.get_latitude() == gc_2.get_latitude())
		{
			/* Not nessecary! It would be enough to set latitude_iter > 
			 * latitude_iter_end. In fact that construction does that! */
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

				/* the "cycle shift" I spoke on/of (???)
				 * Now longitude_iter_end has a value greater than 180 degrees. */
				longitude_iter_end = floor((gc_2.get_longitude() + 360.0) * 100) / 100;
			}
		} else if (gc_1.get_longitude() == gc_2.get_longitude())
		{
			/* Not nessecary! It would be enough to set longitude_iter > 
			 * longitude_iter_end. In fact that construction does that! */
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
		
		/* First we deal with the bad constellations */
		if (bad_latitude_constellation || bad_longitude_constellation)
		{
			double value_iter; /* either the latitude_iter or longitude_iter */
			double value_iter_end; /* dito */
			GeoCoordinate gc_1_iter; /* an iterator for the first parallel line */
			GeoCoordinate gc_2_iter; /* an iterator for the second parallel line */
			
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
				/* if value_iter's value is greater then 180 degrees
				 * "decycle the shift". Or type error message if we have the wrong
				 * bad constellation. */
				double correct_value = value_iter;
				if (value_iter > 180.0)
				{
					if (bad_latitude_constellation)
						correct_value = -360 + value_iter;
					else
						mlog(MLog::error, "GeoCoordinate") << "SHOULD NOT HAPPEN!!!\n";
				}
				
				/* Change the GeoCoordinate iterator latitude/longitude according the
				 * the given bad constellation */
				if (bad_latitude_constellation)
				{
					gc_1_iter.set_longitude(correct_value);
					gc_2_iter.set_longitude(correct_value);
				} else
				{
					gc_1_iter.set_latitude(correct_value);
					gc_2_iter.set_latitude(correct_value);
				}
				
				/* use the given method and insert the tile ids into the set */
				container = gc_1_iter.get_needed_tile_ids(radius_threshold);
				ids.insert(container.begin(), container.end());
				
				container = gc_2_iter.get_needed_tile_ids(radius_threshold);
				ids.insert(container.begin(), container.end());
			} // end for
			
		} else
		{
			/* "normal" situation:
			 * Now it's save to calculate the gradient and y axis intersection point */
			double gradient = latitude_difference / longitude_difference;
			double y_intersection_point = gc_1.get_latitude()
				- (gradient * gc_1.get_longitude());
			
			/* Control the values against a given EPSILON */
			double control_y_intersection_point = gc_2.get_latitude()
				- (gradient * gc_2.get_longitude());
			if (y_intersection_point != control_y_intersection_point)
			{
				mlog(MLog::debug, "GeoCoordinate") << "y intersections point NOT equal.\n";
				
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
			
			/* the big loop over the tile borders:
			 * calculate the missing argument, change the calculated_gc, obtain
			 * the tile ids, insert them into the set, be happy... */
			for (; latitude_iter < latitude_iter_end; latitude_iter += 0.01)
			{
				calculated_longitude = (latitude_iter - y_intersection_point) / gradient;
				
				calculated_gc.set_latitude(latitude_iter);
				calculated_gc.set_longitude(calculated_longitude);
				
				container = calculated_gc.get_needed_tile_ids(radius_threshold);
				ids.insert(container.begin(), container.end());
			}
			
			for (; longitude_iter < longitude_iter_end; longitude_iter += 0.01)
			{
				calculated_latitude = gradient * longitude_iter + y_intersection_point;
				
				calculated_gc.set_latitude(calculated_latitude);
				calculated_gc.set_longitude(longitude_iter);
				
				container = calculated_gc.get_needed_tile_ids(radius_threshold);
				ids.insert(container.begin(), container.end());
			}
		}
		
		/* Now make a vector again from the set
		 * (because it's the return value; but don't ask ME why? We have vectors
		 * everywhere :-) */
		std::vector<unsigned int> for_return;
		for_return.insert(for_return.end(), ids.begin(), ids.end());
		
/*		std::vector<unsigned int>::iterator iter = for_return.begin();
		for (; iter != for_return.end(); ++iter)
		{
			std::cout << *iter << " ";
		}
		std::cout << std::endl;*/
		
		return for_return;
		
		/* Did you forget the beer. ME NOT. Cheers! */
	}
	
	
	GeoCoordinate
	GeoCoordinate::interpolate_approximated(const GeoCoordinate& gc_1,
	 const GeoCoordinate& gc_2, const double weight_on_first)
	{
		double distance = gc_1.distance_approximated(gc_2, _RADIAN);
		double bearing = gc_1.bearing_approximated(gc_2, _RADIAN);
		
		return gc_1.compute_geo_coordinate_approximated(bearing,
			distance * (1.0 - weight_on_first), _RADIAN, _RADIAN);
	}
	
	
	GeoCoordinate
	GeoCoordinate::interpolate_on_great_circle(const GeoCoordinate& gc_1,
	 const GeoCoordinate& gc_2, const double weight_on_first)
	{
		/* return, if weight_on_first has trivial values... */
		if (weight_on_first == 0.0)
			return gc_2;
		if (weight_on_first == 1.0)
			return gc_1;
		/* done. */
		
		double dist = gc_1.distance_on_great_circle(gc_2, _RADIAN);
		double sin_dist = sin(dist);
		
		double lat1 = gc_1._latitude * d2r;
		double lon1 = gc_1._longitude * d2r;
		double cos_lat1 = cos(lat1);
		double sin_lat1 = sin(lat1);
		double cos_lon1 = cos(lon1);
		double sin_lon1 = sin(lon1);
		
		double lat2 = gc_2._latitude * d2r;
		double lon2 = gc_2._longitude * d2r;
		double cos_lat2 = cos(lat2);
		double sin_lat2 = sin(lat2);
		double cos_lon2 = cos(lon2);
		double sin_lon2 = sin(lon2);
		
		double A = sin(weight_on_first * dist) / sin_dist;
		double B = sin( (1.0 - weight_on_first) * dist ) / sin_dist;
		
		double x = (A * cos_lat1 * cos_lon1) + (B * cos_lat2 * cos_lon2);
		double y = (A * cos_lat1 * sin_lon1) + (B * cos_lat2 * sin_lon2);
		double z = (A * sin_lat1) + (B * sin_lat2);
		
		double lat_intermediate = atan2(z, sqrt(x * x + y * y));
		double lon_intermediate = atan2(y, x);
		
		GeoCoordinate gc;
		gc.set_latitude(lat_intermediate * r2d);
		gc.set_longitude(lon_intermediate * r2d);
		return gc;
	}
	
	
	GeoCoordinate
	GeoCoordinate::interpolate_on_rhumb_line(const GeoCoordinate& gc_1,
	 const GeoCoordinate& gc_2, const double weight_on_first)
	{
		double distance = gc_1.distance_on_rhumb_line(gc_2, _RADIAN);
		double bearing = gc_1.bearing_on_rhumb_line(gc_2, _RADIAN);
		
		return gc_1.compute_geo_coordinate_on_rhumb_line(bearing,
			distance * (1.0 - weight_on_first), _RADIAN, _RADIAN);
	}
	
	
	GeoCoordinate
	GeoCoordinate::nearest_geo_coordinate_on_segment_on_great_circle(
		const GeoCoordinate& start_gc, const GeoCoordinate& end_gc) const
	{
		/* This method automatically choose the shorter segment from start_gc
		 * to end_gc! Just have that in mind. */
		 
		/* A = start_gc
		 * B = end_gc
		 * D = *this */
		
		/** @todo think about fast_... methods.
		 * e.g. fast_nearest_geo_coordinate... where we will not call distance
		 * and bearing methods but compute everything in here.
		 * At the moment many infomation are compute twice or more often
		 * (e.g. distances) */
		
		double distance_AB = start_gc.distance_on_great_circle(end_gc, _RADIAN);
		double distance_AD = start_gc.distance_on_great_circle(*this, _RADIAN);
		
		double bearing_AB = start_gc.bearing_on_great_circle(end_gc);
		double bearing_AD = start_gc.bearing_on_great_circle(*this);
		
		/* alpha is the arc(AB, AD) */
		double abs_alpha = fabs(bearing_AB - bearing_AD);
		
		if (abs_alpha > PI / 2.0)
			return start_gc;
		
		double bearing_BA = end_gc.bearing_on_great_circle(start_gc);
		double bearing_BD = end_gc.bearing_on_great_circle(*this);
		double abs_beta = fabs(bearing_BA - bearing_BD);
		
		if (abs_beta > PI / 2.0)
			return end_gc;
				
		double sin_distance_AD = sin(distance_AD);
		
		double sin_XTD = sin_distance_AD * sin(bearing_AD - bearing_AB);
		double XTD = asin( sin_XTD );
		
		double ATD = asin(
			sqrt(sin_distance_AD * sin_distance_AD - sin_XTD * sin_XTD) / cos(XTD)
		);
		
		return start_gc.compute_geo_coordinate_on_great_circle(bearing_AB, ATD,
			_RADIAN, _RADIAN);
	}
	
} // namespace mapgeneration
