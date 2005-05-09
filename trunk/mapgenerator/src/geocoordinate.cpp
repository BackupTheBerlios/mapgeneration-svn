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

#include "util/mlog.h"

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
		const Representation representation) const
	{
		double lat_diff = geo_coordinate._latitude - _latitude;
		double lon_diff = geo_coordinate._longitude - _longitude;
		
		/* calculation of the direction with the following formula */
		double bearing = atan2(lat_diff, lon_diff);
		
		if (bearing < 0)
			bearing += 2 * PI;
		
		return convert(bearing, representation);
	}
	
	
	double
	GeoCoordinate::bearing_on_great_circle(const GeoCoordinate& geo_coordinate,
		const Representation representation, const double at_point) const
	{
		/* everything in radians! */
		
		GeoCoordinate start_gc = interpolate(*this, geo_coordinate, 1.0 - at_point);
		
		double drf = PI / 180; // degree to radian factor
		double lat1 = start_gc._latitude * drf;
		double lat2 = geo_coordinate._latitude * drf;
		double lon_diff = (geo_coordinate._longitude - start_gc._longitude) * drf;
		
		/** @todo does that work? Is the start_gc on the same great circle like
		 * the two other gcs??? */
		double dist = start_gc.distance_on_great_circle(geo_coordinate, _RADIAN);
		
		double bearing = acos(
			(sin(lat2) - sin(lat1) * cos(dist)) / (cos(lat1) * sin(dist))
		);
		
		if (sin(lon_diff) < 0.0)
			bearing = 2.0 * PI - bearing;
		
		/* make a mathematical angle */
		bearing = PI / 2.0 - bearing;
		
		if (bearing < 0.0)
			bearing += 2.0 * PI;
		
		return convert(bearing, representation);
	}
	
	
	double
	GeoCoordinate::bearing_on_rhumb_line(const GeoCoordinate& geo_coordinate,
		const Representation representation) const
	{
		/* everything in radians! */
		
		double drf = PI / 180; // degree to radian factor
		double lat1 = _latitude * drf;
		double lat2 = geo_coordinate._latitude * drf;
		double lat_diff = lat2 - lat1;
		
		double lon_diff_W = (_longitude - geo_coordinate._longitude) * drf;
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
		
		while ( (bearing < 0.0) || (bearing >= 2.0 * PI) )
		{
			if (bearing < 0.0)
				bearing += (2.0 * PI);
			else if (bearing > 0.0)
				bearing -= (2.0 * PI);
		}
		
		return convert(bearing, representation);
	}
	
	
	GeoCoordinate
	GeoCoordinate::compute_geo_coordinate(double bearing_on_rhumb_line,
		double distance_on_rhumb_line,
		const GeoCoordinate::Representation bearing_representation,
		const GeoCoordinate::Representation distance_representation) const
	{
		/* test representations and convert to radian... */
		double drf = PI / 180.0; // degree to radian factor
		double mrf = 1.0 / EARTH_RADIUS_M; // meter to radian factor
		
		double bearing = bearing_on_rhumb_line;
		switch (bearing_representation)
		{
			case _DEGREE:
				bearing *= drf;
				break;
				
			case _METER:
				throw "FALSE REPRESENTATION: bearing is given in meters!";
				break;
		}
		
		while ( !((0 <= bearing) && (bearing < 2.0 * PI)) )
		{
			if (bearing < 0)
				bearing += (2.0 * PI);
			else
				bearing -= (2.0 * PI);
		}
		
		double distance = distance_on_rhumb_line;
		switch (distance_representation)
		{
			case _DEGREE:
				distance *= drf;
				break;
				
			case _METER:
				distance *= mrf;
				break;
		}
		/* done. */
		
		/* convert mathematical bearing to navigation bearing... */
		bearing = (PI / 2.0) - bearing;
		/* done */
		
		/* convert my coordinates to radians... */
		double lat1 = _latitude * drf;
		double lon1 = _longitude * drf;
		/* done. */
		
		/* calculate values...*/
		double lat2 = lat1 + distance * cos(bearing);
		double lat_diff = lat1 - lat2;
		
		double dphi = log( tan(lat2/2.0 + PI/4.0) / tan(lat1/2.0 + PI/4.0) );
		
		double EPSILON = 0.00000001; // arbitrary
		double q;
		if ( fabs(lat_diff) < EPSILON )
			q = cos(lat1);
		else
			q = lat_diff / dphi;
		
		double dlon = -distance * sin(bearing) / q;
		double lon2 = lon1 + dlon;
		
		while ( (lon2 < -PI) || (PI <= lon2) )
		{
			if (lon2 < 0.0)
				lon2 += (2.0 * PI);
			else if (lon2 > 0.0)
				lon2 -= (2.0 * PI);
		}
		/* done. */
		
		/* convert to degree and return... */
		double rdf = 180.0 / PI; // radian to degree factor;
		
		GeoCoordinate gc;
		gc.set_latitude(lat2 * rdf);
		gc.set_longitude(lon2 * rdf);
		
		return gc;
		/* done. */
	}
	
	
	double
	GeoCoordinate::distance_approximated(const GeoCoordinate& geo_coordinate,
		const Representation representation) const
	{
		double lat_diff = _latitude - geo_coordinate._latitude;
		double lon_diff = _longitude - geo_coordinate._longitude;
		
		double distance = sqrt(lat_diff * lat_diff + lon_diff * lon_diff);
		
		return convert(distance * PI / 180.0, representation);
	}
	
	
	double
	GeoCoordinate::distance_on_great_circle(const GeoCoordinate& geo_coordinate,
		const Representation representation) const
	{
		/* calculates the distance between two GeoCoordinates with the
		 * Great circle distance formula */

		double drf = PI / 180.0; // degree to radian factor
		
		double lat1 = _latitude * drf;
		double lat2 = geo_coordinate._latitude * drf;
		
		double lon1 = _longitude * drf;
		double lon2 = geo_coordinate._longitude * drf;
		
		double sin_average_lat = sin( (lat1 - lat2) / 2.0 );
		double sin_average_lon = sin( (lon1 - lon2) / 2.0 );
			
		double distance = 2.0 * asin( sqrt(sin_average_lat * sin_average_lat
			+ cos(lat1) * cos(lat2) * sin_average_lon * sin_average_lon) );
		
		return convert(distance, representation);
	}
	
	
	double
	GeoCoordinate::distance_on_rhumb_line(const GeoCoordinate& geo_coordinate,
		const Representation representation) const
	{
		/* everything in radians! */
		
		double drf = PI / 180; // degree to radian factor
		double lat1 = _latitude * drf;
		double lat2 = geo_coordinate._latitude * drf;
		double lat_diff = lat2 - lat1;
		
		double lon_diff_W = (geo_coordinate._longitude - _longitude) * drf;
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
		
		return convert(distance, representation);
	}
	
	
	double
	GeoCoordinate::distance_to_tile_border(const Heading heading,
		const Representation representation) const
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
		return distance_approximated(compare_point, representation);
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
	
	
} // namespace mapgeneration
