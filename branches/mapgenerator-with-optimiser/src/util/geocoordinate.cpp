/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "geocoordinate.h"

#include <ctime>
#include <cmath>

namespace mapgeneration_util
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
		
		out.precision(original_precision);
		out.flags(original_flags);
		
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
	GeoCoordinate::bearing_approximated2(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		double cos_lat1 = cos(_latitude * d2r);
		double cos_lat2 = cos(geo_coordinate._latitude * d2r);
		
		double lat_diff = (geo_coordinate._latitude - _latitude) * d2r;
		double lon_diff = (geo_coordinate._longitude - _longitude) * d2r;
		
		double bearing = atan2(
			((cos_lat1 + cos_lat2) / 2.0) * lon_diff, lat_diff );
		
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
	GeoCoordinate::distance_approximated2(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		double cos_lat1 = cos(_latitude * d2r);
		double cos_lat2 = cos(geo_coordinate._latitude * d2r);
		
		double dist_N = (geo_coordinate._latitude - _latitude);
		double dist_E = ((cos_lat1 + cos_lat2) / 2.0)
			* (geo_coordinate._longitude - _longitude);
		
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
	
/*	
*/	
	
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
	
	
	bool
	GeoCoordinate::has_perpendicular_on_segment_approximated(
		const GeoCoordinate& start_gc, const GeoCoordinate& end_gc) const
	{
		/* A = start_gc
		 * B = end_gc
		 * D = *this */
		
		// for further comments,
		// see nearest_geo_coordinate_to_segment_on_great_circle.
		
		double bearing_AB = start_gc.bearing_approximated2(end_gc);
		double bearing_AD = start_gc.bearing_approximated2(*this);
		double abs_alpha = fabs(bearing_AB - bearing_AD);
		
		if (abs_alpha > PI / 2.0)
			return false;
		
		double bearing_BA = bearing_AB + PI; // bearing_approximated2 is invariant!
		normalise_arc(bearing_BA);
		double bearing_BD = end_gc.bearing_approximated2(*this);
		double abs_beta = fabs(bearing_BA - bearing_BD);
		
		if (abs_beta > PI / 2.0)
			return false;
		
		return true;
	}
	
	
	bool
	GeoCoordinate::has_perpendicular_on_segment_on_great_circle(
		const GeoCoordinate& start_gc, const GeoCoordinate& end_gc) const
	{
		/* A = start_gc
		 * B = end_gc
		 * D = *this */
		
		// for further comments,
		// see nearest_geo_coordinate_to_segment_on_great_circle.
		
		double bearing_AB = start_gc.bearing_on_great_circle(end_gc);
		double bearing_AD = start_gc.bearing_on_great_circle(*this);
		double abs_alpha = fabs(bearing_AB - bearing_AD);
		
		if (abs_alpha > PI / 2.0)
			return false;
		
		double bearing_BA = end_gc.bearing_on_great_circle(start_gc);
		double bearing_BD = end_gc.bearing_on_great_circle(*this);
		double abs_beta = fabs(bearing_BA - bearing_BD);
		
		if (abs_beta > PI / 2.0)
			return false;
		
		return true;
	}
	
	
	GeoCoordinate
	GeoCoordinate::nearest_geo_coordinate_to_segment_on_great_circle(
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
