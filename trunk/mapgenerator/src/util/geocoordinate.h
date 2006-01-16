/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

#include "util/constants.h"
#include "util/mlog.h"
#include "util/serializer.h"

#define _altitude _values[_ALTITUDE]
#define _latitude _values[_LATITUDE]
#define _longitude _values[_LONGITUDE]

#define bearing_default bearing_approximated
#define compute_geo_coordinate_default compute_geo_coordinate_approximated
#define distance_default distance_approximated
#define interpolate_default interpolate_approximated

/* factors for conversion from x to y
 * x = {d|m|r}, y = {d|m|r}
 * d = degrees
 * m = meters
 * r = radians */
#define d2d (1.0)
#define d2m ((EARTH_RADIUS_M * PI) / 180.0)
#define d2r (PI / 180.0)
#define m2d (180.0 / (PI * EARTH_RADIUS_M))
#define m2m (1.0)
#define m2r (1.0 / EARTH_RADIUS_M)
#define r2d (180.0 / PI)
#define r2m (EARTH_RADIUS_M)
#define r2r (1.0)

namespace mapgeneration_util
{
	
	class GeoCoordinate;
	
	
	/**
	 * @brief Output operator (for a GeoCoodinate object)
	 * 
	 * @param out the ostream
	 * @param geo_coordinate the GeoCoordinate
	 * @param the ostream
	 */
	std::ostream&
	operator<<(std::ostream& out, const GeoCoordinate& geo_coordinate);
	
	
	/**
	 * @brief GeoCoordinate implements a 3D coordinate based on geodatic data.
	 * 
	 * A GeoCoordinate consists of the attributes
	 * <ul>
	 * <li>altitude</li>
	 * <li>latitude</li>
	 * <li>longitude</li>
	 * </ul>
	 * and provides methods to calculate corresponding tile ID, etc.
	 */
	class GeoCoordinate
	{
		
		friend std::ostream& operator<<(std::ostream& out,
			const GeoCoordinate& geo_goordinate);
		
		
		public:
			
			static const int _LATITUDE = 0;
			static const int _LONGITUDE = 1;
			static const int _ALTITUDE = 2;
			
			
			/**
			 * @brief Simple enumeration to code the heading.
			 */
			enum Heading
			{
				_NORTH = 0,
				_EAST,
				_SOUTH,
				_WEST,
				_NORTHWEST,
				_NORTHEAST,
				_SOUTHWEST,
				_SOUTHEAST
			};
			
			
			enum Representation
			{
				_DEGREE = 0,
				_RADIAN,
				_METER
			};
			
			
			/**
			 * @brief Empty Constructor.
			 */
			GeoCoordinate() ;
			
			
			/**
			 * @brief Constructor for init the GeoCoordinate with specified values.
			 * 
			 * @param latitude a value for the latitude
			 * @param longitude a value for the longitude
			 * @param altitude a value for the altitude (default: 0)
			 */
			GeoCoordinate(const double latitude, const double longitude,
				const double altitude = 0);
				
			
			/**
			 * @brief Copy constructor.
			 * 
			 * @param geo_coordinate the GeoCoordinate that is copied
			 */
			GeoCoordinate(const GeoCoordinate& geo_coordinate);
			
			
			/**
			 * @brief Calculates an approximated value for the bearing
			 * from this to the given GeoCoordinate.
			 * 
			 * @param geo_coordinate the given GeoCoordinate
			 * @param output_representation a switch to control the
			 * representation of the return value. Default is radian. You can
			 * choose between radian, degree, meter (only where appropriate!).
			 * 
			 * @return the approximated value for bearing
			 */
			double
			bearing_approximated(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _RADIAN) const;
			
			
			/**
			 * @brief Calculates the exact value for the bearing
			 * from this to the given GeoCoordinate on a Great Circle.
			 * As this value changes with every infitesimal movement on the
			 * Great Circle, you can state a proportional value
			 * (within [0, 1]) that specify the position where you are on the
			 * Great Circle "at the moment".
			 * 
			 * @param geo_coordinate the given GeoCoordinate
			 * @param output_representation a switch to control the
			 * representation of the return value. Default is radian. You can
			 * choose between radian, degree, meter (only where appropriate!).
			 * @param at_point the proportional value
			 * 
			 * @return the exact value for bearing on a Great Circle
			 * 
			 * @see bearing_on_rhumb_line
			 */
			double
			bearing_on_great_circle(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _RADIAN,
				const double at_point = 0.0) const;
			
			
			/**
			 * @brief Loxodrom = Rhumbline.
			 * @see bearing_on_rhumb_line
			 */
			inline double
			bearing_on_loxodrom(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _RADIAN) const;
			
			
			/**
			 * @brief Orthodrom = Great Circle.
			 * @see bearing_on_great_circle
			 */
			inline double
			bearing_on_orthodrom(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _RADIAN,
				const double at_point = 0.0) const;
			
			
			/**
			 * @brief Calculates the exact value for the bearing
			 * from this to the given GeoCoordinate on a Rhumbline.
			 * As navigation on a Rhumbline is easier then on a Great Circle,
			 * you calculate a bearing value in advance and follow it until you
			 * reach your destination, a proportional value need not to be
			 * specified. But as you know every advantage has a drawback: the
			 * travel distance on a rhumbline is longer then on a Great Circle.
			 * 
			 * @param geo_coordinate the given GeoCoordinate
			 * @param output_representation a switch to control the
			 * representation of the return value. Default is radian. You can
			 * choose between radian and degree.
			 * 
			 * @return the exact value for bearing on a Rhumbline
			 * 
			 * @see bearing_on_great_circle
			 */
			double
			bearing_on_rhumb_line(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _RADIAN) const;
			
			
			/**
			 * @brief Computes an approximated GeoCoordinate given the polar
			 * coordinates (bearing and distance).
			 * You can specify in which representation these values are given.
			 * 
			 * @param bearing the bearing
			 * @param distance the distance
			 * @param bearing_representation the representation of the bearing
			 * value. You can choose between radian and degree. Default is
			 * radian.
			 * @param distance_representation the representation of the distance
			 * value. You can choose between radian, degree and meter. Default
			 * is meter.
			 * 
			 * @return the approximated GeoCoodinate
			 */
			GeoCoordinate
			compute_geo_coordinate_approximated(double bearing, double distance,
				const Representation bearing_representation = _RADIAN,
				const Representation distance_representation = _METER) const;
			
			
			/**
			 * @brief Computes an exact GeoCoordinate following the Great
			 * Circle which yield from the starting value for the bearing. On
			 * this Great Circle the given distance is travelled.
			 * You can specify in which representation these values are given.
			 * 
			 * @param starting_bearing the starting value for the bearing
			 * @param distance the distance
			 * @param bearing_representation the representation of the bearing
			 * value. You can choose between radian and degree. Default is
			 * radian.
			 * @param distance_representation the representation of the distance
			 * value. You can choose between radian, degree and meter. Default
			 * is meter.
			 * 
			 * @return the exact GeoCoodinate
			 * 
			 * @see compute_geo_coordinate_on_rhumb_line
			 */
			GeoCoordinate
			compute_geo_coordinate_on_great_circle(double starting_bearing,
				double distance,
				const Representation bearing_representation = _RADIAN,
				const Representation distance_representation = _METER) const;
			
			
			/**
			 * @brief Computes an exact GeoCoordinate given the polar
			 * coordinates (bearing and distance) on the resulting Rhumbline.
			 * You can specify in which representation these values are given.
			 * 
			 * @param bearing the bearing
			 * @param distance the distance
			 * @param bearing_representation the representation of the bearing
			 * value. You can choose between radian and degree. Default is
			 * radian.
			 * @param distance_representation the representation of the distance
			 * value. You can choose between radian, degree and meter. Default
			 * is meter.
			 * 
			 * @return the exact GeoCoodinate
			 * 
			 * @see compute_geo_coordinate_on_great_circle
			 */
			GeoCoordinate
			compute_geo_coordinate_on_rhumb_line(double bearing, double distance,
				const Representation bearing_representation = _RADIAN,
				const Representation distance_representation = _METER) const;
			
			
			/**
			 * @brief Converts the given value from one representation to
			 * another one.
			 * 
			 * @param value the given value
			 * @param from_representation the initial representation of the
			 * given value
			 * @param to_representation the representation the given value
			 * is converted to
			 */
			inline static void
			convert(double& value, const Representation from_representation,
				const Representation to_representation);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);
			
			
			/**
			 * @brief Calculates the approximated distance between this and
			 * the given GeoCoordinate.
			 * You can specify in which representation the output value
			 * should be: radian, degree, meter (default is meter).
			 * 
			 * @param geo_coordinate the given GeoCoordinate
			 * 
			 * @return the approximated distance in the chosen representation.
			 */
			double
			distance_approximated(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _METER) const;
			
			
			/**
			 * @brief Calculates the exact distance between this and
			 * the given GeoCoordinate on a Great Circle.
			 * You can specify in which representation the output value
			 * should be: radian, degree, meter (default is meter).
			 * 
			 * @param geo_coordinate the given GeoCoordinate
			 * 
			 * @return the exact distance on the Great Circle in the chosen
			 * representation
			 * 
			 * @see distance_on_rhumb_line
			 */
			double
			distance_on_great_circle(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _METER) const;
			
			
			/**
			 * @brief Loxodrom = Rhumbline.
			 * @see distance_on_rhumb_line
			 */
			inline double
			distance_on_loxodrom(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _METER) const;
			
			
			/**
			 * @brief Orthodrom = Great Circle.
			 * @see distance_on_rhumb_line
			 */
			inline double
			distance_on_orthodrom(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _METER) const;
			
			
			/**
			 * @brief Calculates the exact distance between this and
			 * the given GeoCoordinate on a Rhumbline.
			 * You can specify in which representation the output value
			 * should be: radian, degree, meter (default is meter).
			 * 
			 * @param geo_coordinate the given GeoCoordinate
			 * 
			 * @return the exact distance on the Rhumbline in the chosen
			 * representation
			 * 
			 * @see distance_on_great_circle
			 */
			double
			distance_on_rhumb_line(const GeoCoordinate& geo_coordinate,
				const Representation output_representation = _METER) const;
			
			
			/**
			 * @return the value of the altitude
			 */
			inline double
			get_altitude() const;
			
			
			/**
			 * @return the value of the latitude
			 */
			inline double
			get_latitude() const;
			
			
			/**
			 * @return the value of the longitude
			 */
			inline double
			get_longitude() const;
			
			
			/**
			 * @brief Interpolates a new approximated GeoCoordinate.
			 * 
			 * The new GeoCoordinate is located between the two specified
			 * GeoCoordinates according to the specified weight.
			 * 
			 * @param gc_1 a reference to the first GeoCoordinate
			 * @param gc_2 a reference to the second GeoCoordiante
			 * @param weight_on_first the weight on the first GeoCoordinate;
			 * 0 <= weight_on_first <= 1 (weight_on_second result in 1 -
			 * weight_on_first)
			 * @return the new approximated GeoCoordinate
			 */
			static GeoCoordinate
			interpolate_approximated(const GeoCoordinate& gc_1,
				const GeoCoordinate& gc_2, const double weight_on_first);
			
			
			/**
			 * @brief Interpolates a new exact GeoCoordinate. The interpolated
			 * CeoCoordinate is situated on the initiated Great Circle.
			 * 
			 * The new GeoCoordinate is located between the two specified
			 * GeoCoordinates according to the specified weight.
			 * 
			 * @param gc_1 a reference to the first GeoCoordinate
			 * @param gc_2 a reference to the second GeoCoordiante
			 * @param weight_on_first the weight on the first GeoCoordinate;
			 * 0 <= weight_on_first <= 1 (weight_on_second result in 1 -
			 * weight_on_first)
			 * @return the new exact GeoCoordinate
			 */
			static GeoCoordinate
			interpolate_on_great_circle(const GeoCoordinate& gc_1,
				const GeoCoordinate& gc_2, const double weight_on_first);
			
			
			/**
			 * @brief Interpolates a new exact GeoCoordinate. The interpolated
			 * CeoCoordinate is situated on the initiated Rhumbline.
			 * 
			 * The new GeoCoordinate is located between the two specified
			 * GeoCoordinates according to the specified weight.
			 * 
			 * @param gc_1 a reference to the first GeoCoordinate
			 * @param gc_2 a reference to the second GeoCoordiante
			 * @param weight_on_first the weight on the first GeoCoordinate;
			 * 0 <= weight_on_first <= 1 (weight_on_second result in 1 -
			 * weight_on_first)
			 * @return the new exact GeoCoordinate
			 */
			static GeoCoordinate
			interpolate_on_rhumb_line(const GeoCoordinate& gc_1,
				const GeoCoordinate& gc_2, const double weight_on_first);
			
			
			/**
			 * @brief Normalise the given arc to be within [0, 2PI)
			 * 
			 * @param arc the arc which is normalised
			 * @param the representation of the arc
			 */
			inline static void
			normalise_arc(double& arc, const Representation representation);
			
			
			/**
			 * @brief Assignment operator.
			 * 
			 * Assigns a GeoCoordinate to this.
			 * 
			 * @param geo_coordinate a reference to a GeoCoordinate
			 * @return (new) this
			 */
			inline GeoCoordinate&
			operator=(const GeoCoordinate& geo_coordinate);
			
			
			/**
			 * @brief Equality operator.
			 * 
			 * @param geo_coordinate a reference to a GeoCoordinate
			 * @return true if *this == geo_coordinate
			 */
			inline bool
			operator==(const GeoCoordinate& geo_coordinate) const;
			
			
			/**
			 * @brief Inequality operator.
			 * 
			 * @param geo_coordinate a reference to a GeoCoordinate
			 * @return true if *this != geo_coordinate
			 */
			inline bool
			operator!=(const GeoCoordinate& geo_coordinate) const;
			
			
			/**
			 * @brief Index operator (const version).
			 * 
			 * @param dimension the dimension (use Dimension enum values!)
			 * @return the corresponding value
			 */
			inline double
			operator[](int dimension) const;
			
			
			/**
			 * @brief Index operator.
			 * 
			 * @param dimension the dimension (use Dimension enum values!)
			 * @return the corresponding value
			 */
			inline double&
			operator[](int dimension);
			
			
			/**
			 * @brief Computed the nearest GeoCoordinate to the segment which
			 * results from the two given GeoCoordinates.
			 * 
			 * @param start_gc the first GeoCoordinate
			 * @param end_gc the second GeoCoordinate
			 * 
			 * @return the nearest GeoCoordinate
			 */
			GeoCoordinate
			nearest_geo_coordinate_to_segment_on_great_circle(
				const GeoCoordinate& start_gc, const GeoCoordinate& end_gc) const;
			
			
			/**
			 * @see mapgeneration_util::Serailizer
			 */
			inline void
			serialize(std::ostream& o_stream) const;
			
			
			/**
			 * @brief Sets the values for latitude and longitude.
			 * 
			 * @param latitude the value for the latitude
			 * @param longitude the value for the longitude
			 */
			inline void
			set(const double latitude, const double longitude);
			
			
			/**
			 * @brief Sets the values for latitude, longitude and altitude.
			 * 
			 * @param latitude the value for the latitude
			 * @param longitude the value for the longitude
			 * @param altitude the value for the altitude
			 */
			inline void
			set(const double latitude, const double longitude,
				const double altitude);
			
			
			/**
			 * @brief Sets the values for altitude.
			 * 
			 * @param value the value for the altitude
			 */
			inline void
			set_altitude(const double value);
			
			
			/**
			 * @brief Sets the values for latitude.
			 * 
			 * @param value the value for the latitude
			 */
			inline void
			set_latitude(const double value);
			
			
			/**
			 * @brief Sets the values for longitude.
			 * 
			 * @param value the value for the longitude
			 */
			inline void
			set_longitude(const double value);
			
			
		protected:
			
			double _values[3];
			
			
	};
	
	
	inline
	GeoCoordinate::GeoCoordinate()
	{
		_values[_LATITUDE] = 0.0;
		_values[_LONGITUDE] = 0.0;
		_values[_ALTITUDE] = 0.0;
	}
	
	
	inline
	GeoCoordinate::GeoCoordinate(double latitude, double longitude,
		double altitude)
	{
		_values[_LATITUDE] = latitude;
		_values[_LONGITUDE] = longitude;
		_values[_ALTITUDE] = altitude;
	}
	
	
	inline
	GeoCoordinate::GeoCoordinate(const GeoCoordinate& geo_coordinate)	
	{
		_latitude = geo_coordinate.get_latitude();
		_longitude = geo_coordinate.get_longitude();
		_altitude = geo_coordinate.get_altitude();
	}
	
	
	inline double
	GeoCoordinate::bearing_on_loxodrom(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		return bearing_on_rhumb_line(geo_coordinate, output_representation);
	}
	
	
	inline double
	GeoCoordinate::bearing_on_orthodrom(const GeoCoordinate& geo_coordinate,
		const Representation output_representation, const double at_point) const
	{
		return bearing_on_great_circle(geo_coordinate, output_representation, at_point);
	}
		
		
	inline void
	GeoCoordinate::convert(double& value,
		const Representation from_representation,
		const Representation to_representation)
	{
		switch (from_representation)
		{
			case _DEGREE:
				value *= d2r;
				break;
			
			case _METER:
				value *= m2r;
				break;
		}
		
		switch (to_representation)
		{
			case _DEGREE:
				value *= r2d;
				break;
			
			case _METER:
				value *= r2m;
				break;
		}
	}
	
	
	inline void
	GeoCoordinate::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _latitude);
		Serializer::deserialize(i_stream, _longitude);
		Serializer::deserialize(i_stream, _altitude);
	}
	
	
	inline double
	GeoCoordinate::distance_on_loxodrom(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		return distance_on_rhumb_line(geo_coordinate, output_representation);
	}
	
	
	inline double
	GeoCoordinate::distance_on_orthodrom(const GeoCoordinate& geo_coordinate,
		const Representation output_representation) const
	{
		return distance_on_great_circle(geo_coordinate, output_representation);
	}
	
	
	inline double
	GeoCoordinate::get_altitude() const
	{
		return _altitude;
	}
	
	
	inline double
	GeoCoordinate::get_latitude() const
	{
		return _latitude;
	}
	
	
	inline double
	GeoCoordinate::get_longitude()const
	{
		return _longitude;
	}
	
	
	inline void
	GeoCoordinate::normalise_arc(double& arc, const Representation representation)
	{
		convert(arc, representation, _RADIAN);
		
		while (arc < 0)
			arc += (2.0 * PI);
		
		while (2.0 * PI <= arc)
			arc -= (2.0 * PI);
		
		convert(arc, _RADIAN, representation);
	}
	
	
	inline GeoCoordinate&
	GeoCoordinate::operator=(const GeoCoordinate& geo_coordinate)
	{
		_altitude = geo_coordinate._altitude;
		_latitude = geo_coordinate._latitude;
		_longitude = geo_coordinate._longitude;
		
		return *this;
	}
	
	
	inline bool
	GeoCoordinate::operator==(const GeoCoordinate& geo_coordinate) const
	{
		return (
			(_altitude == geo_coordinate._altitude) &&
			(_latitude == geo_coordinate._latitude) &&
			(_longitude == geo_coordinate._longitude)
		);
	}
	
	
	inline bool
	GeoCoordinate::operator!=(const GeoCoordinate& geo_coordinate) const
	{
		return ( !operator==(geo_coordinate) );
	}
	
	
	inline double
	GeoCoordinate::operator[](int index) const
	{
		return _values[index];
	}
	
	
	inline double&
	GeoCoordinate::operator[](int index)
	{
		return _values[index];
	}
	
	
	inline void
	GeoCoordinate::set(double latitude, double longitude)
	{
		set_latitude(latitude);
		set_longitude(longitude);
	}
	
			
	inline void
	GeoCoordinate::set(double latitude, double longitude, double altitude)
	{
		set_altitude(altitude);
		set_latitude(latitude);
		set_longitude(longitude);
	}
	
	
	inline void
	GeoCoordinate::set_altitude (double value)
	{
		_altitude = value;
	}
	
	
	inline void
	GeoCoordinate::set_latitude (double value)
	{
		_latitude = value;
	}
	
	
 	inline void 
	GeoCoordinate::set_longitude (double value)
	{
		_longitude = value;
	}
	
	
	inline void
	GeoCoordinate::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _latitude);
		Serializer::serialize(o_stream, _longitude);
		Serializer::serialize(o_stream, _altitude);
	}
	
}  //namespace mapgeneration_util

#endif //GEOCOORDINATE_H
