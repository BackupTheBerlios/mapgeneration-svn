/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

#include <iostream>
#include <vector>
#include "util/serializer.h"

using namespace mapgeneration_util;

#define _altitude _values[_ALTITUDE]
#define _latitude _values[_LATITUDE]
#define _longitude _values[_LONGITUDE]

namespace mapgeneration
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
			
			
			/**
			 * @brief Empty Constructor.
			 */
			inline GeoCoordinate() ;
			
			
			/**
			 * @brief Constructor for init the GeoCoordinate with specified values.
			 * 
			 * @param latitude a value for the latitude
			 * @param longitude a value for the longitude
			 * @param altitude a value for the altitude (default: 0)
			 */
			inline GeoCoordinate(double latitude, double longitude,
				double altitude = 0);
			
			
			double
			approximated_distance(GeoCoordinate geo_coordinate) const;
			
			
			/**
			 * @brief Calculates the direction of the GeoCoordinate.
			 */
			double
			calculate_direction(const GeoCoordinate& geo_coordinate) const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);
			
			
			/**
			 * @brief Calculates the distance between two geocoordinates.
			 * 
			 * @return the distance (in meter)
			 */
			double
			distance(GeoCoordinate geocoordinate) const;
			
			
			/**
			 * @brief Calculates the distances to the tile borders that attach at
			 * the specified heading.
			 * 
			 * @param heading the heading
			 * @return the distance (in meter)
			 */
			double
			distance_to_tile_border(Heading heading) const;
			
			
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
			 * @brief Calculates the needed tile IDs for the GeoCoordinate.
			 * 
			 * Threshold should be smaller than half of the height of a tile.
			 * 
			 * @param radius_threshold the threshold of the radius
			 * @return a vector of tile IDs that are within the radius_threshold
			 */
			std::vector<unsigned int>
			get_needed_tile_ids(const double radius_threshold) const;
			
			
			/**
			 * @brief Calculates the needed tile IDs for the line between two
			 * GeoCoordinates.
			 * 
			 * Threshold should be smaller than half of the height of a tile.
			 * 
			 * @param gc_1 first GeoCoordinate
			 * @param gc_2 second GeoCoordinate
			 * @param radius_threshold the threshold of the radius
			 * @return a vector of tile IDs that are within the radius_threshold
			 */
			static std::vector<unsigned int>
			get_needed_tile_ids(const GeoCoordinate& gc_1, const GeoCoordinate& gc_2,
				const double radius_threshold);
			
			
			/**
			 * @brief A wrapper to call static get_tile_id for the object 
			 * itself.
			 * 
			 * @return the tile ID
			 */
			inline unsigned int
			get_tile_id() const;
			
			
			/**
			 * @brief Returns the tile ID the given coordinate is on.
			 * 
			 * @return the tile ID
			 */
			inline static unsigned int
			get_tile_id(const double latitude, const double longitude);
			
			
			/**
			 * @brief Interpolates a new GeoCoordinate.
			 * 
			 * The new GeoCoordinate is located between the two specified
			 * GeoCoordinates according to the specified weight.
			 * 
			 * @param gc_1 a reference to the first GeoCoordinate
			 * @param gc_2 a reference to the second GeoCoordiante
			 * @param weight_on_first the weight on the first GeoCoordinate;
			 * 0 <= weight_on_first <= 1 (weight_on_second result in 1 -
			 * weight_on_first)
			 * @return the new GeoCoordinate
			 */
			inline static GeoCoordinate
			interpolate(const GeoCoordinate& gc_1, const GeoCoordinate& gc_2, const double weight_on_first);
			
			
			/**
			 * @brief Merges the northing part and the easting part to one
			 * unsigned int
			 * 
			 * @param northing the northing part of a tile ID
			 * @param easting the easting part of a tile ID
			 * @return the whole tile ID
			 * 
			 * @todo Explain the algorithm of tile ID generation.
			 */
			inline static unsigned int
			merge_tile_id_parts(const int northing, const int easting);
			
			
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
			set(double latitude, double longitude);
			
			
			/**
			 * @brief Sets the values for latitude, longitude and altitude.
			 * 
			 * @param latitude the value for the latitude
			 * @param longitude the value for the longitude
			 * @param altitude the value for the altitude
			 */
			inline void
			set(double latitude, double longitude, double altitude);
			
			
			/**
			 * @brief Sets the values for altitude.
			 * 
			 * @param value the value for the altitude
			 */
			inline void
			set_altitude(double value);
			
			
			/**
			 * @brief Sets the values for latitude.
			 * 
			 * @param value the value for the latitude
			 */
			inline void
			set_latitude(double value);
			
			
			/**
			 * @brief Sets the values for longitude.
			 * 
			 * @param value the value for the longitude
			 */
			inline void
			set_longitude(double value);
			
			
			/**
			 * @brief Splits the merged unsigned integer of the tile ID to the
			 * northing and easting part.
			 * 
			 * @param tile_id the tile ID
			 * @param northing_part a reference to the northing part
			 * @param easting_part a reference to the easting part
			 */
			static inline void
			split_tile_id(const unsigned int tile_id, int& northing_part, int& easting_part);
			
			
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
	
	
	inline void
	GeoCoordinate::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _latitude);
		Serializer::deserialize(i_stream, _longitude);
		Serializer::deserialize(i_stream, _altitude);
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
	
	
	inline unsigned int 
	GeoCoordinate::get_tile_id() const
	{
		return get_tile_id(_latitude, _longitude);
	}
	
	
	inline unsigned int
	GeoCoordinate::get_tile_id(const double latitude, const double longitude)
	{
		int northing = (int)((latitude + 90) * 100);
		int easting = (int)((longitude + 180) * 100);		
	   
		return merge_tile_id_parts(northing, easting);
	}
	
	
	inline GeoCoordinate
	GeoCoordinate::interpolate(const GeoCoordinate& gc_1,
		const GeoCoordinate& gc_2, const double weight_on_first)
	{
		double weight_on_second = 1.0 - weight_on_first;
		/*	interpolation of 2 geocoordinates by means of  the weight on 
		 *	the first  geocoordinate
		 */
		return GeoCoordinate(gc_1.get_latitude() * weight_on_first + gc_2.get_latitude() * weight_on_second, 
				gc_1.get_longitude() * weight_on_first + gc_2.get_longitude() * weight_on_second,
				gc_1.get_altitude() * weight_on_first + gc_2.get_altitude() * weight_on_second);
	}
	
	
	inline unsigned int
	GeoCoordinate::merge_tile_id_parts(int northing, int easting)
	{
		if (northing < 0 || northing > 18000) throw ("Pole regions are not supported!!! (merge_tile_id_parts)");
		
		easting = easting % 36000;
		if (easting < 0) easting += 36000;
		
		return ((northing << 16) + easting);
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
		
	
	inline void
	GeoCoordinate::split_tile_id(const unsigned int tile_id, int& northing_part, int& easting_part)
	{
		northing_part = tile_id >> 16;
		easting_part = tile_id % (1 << 16);
	}
	
}  //namespace mapgeneration

#endif //GEOCOORDINATE_H
