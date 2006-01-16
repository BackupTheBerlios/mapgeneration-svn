/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef GPSPOINT_H
#define GPSPOINT_H

#include <string>
#include "geocoordinate.h"
#include "util/direction.h"
#include "util/serializer.h"

using namespace mapgeneration_util;

namespace mapgeneration
{

	/**
	 * @brief GPSPoint extends GeoCoordinate by a time attribute.
	 * 
	 * This class provides a method to parse a NMEA string and generate
	 * the appropriate values of the attribute.
	 * 
	 * @see Direction
	 * @see GeoCoordinate
	 */
	class GPSPoint : public Direction, public GeoCoordinate {

		public:
		
			/**
			 * @brief Empty constructor.
			 */
			GPSPoint() ;
			
			
			/**
			 * @brief Constructor for init the GPSPoint with specified values.
			 * 
			 * @param latitude a value for the latitude
			 * @param longitude a value for the longitude
			 * @param altitude a value for the altitude (default: 0)
			 * @param time a value for the time (default: 0)
			 */
			GPSPoint(double latitude, double longitude, double altitude = 0,
				double time = 0);
			
			
			/**
			 * @brief Constructor for init the GPSPoint with a GeoCoordinate.
			 * 
			 * @param geo_coordinate the GeoCoordinate
			 */
			GPSPoint(const GeoCoordinate& geo_coordinate);
			

			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);
			
			
			/**
			 * @brief Parses the specified string and sets the attributes.
			 * 
			 * This method receives a string of RMC and GGA NMEA strings and
			 * converts them into the attributes of GPSPoint.
			 * 
			 * @param gpgga_string the NMEA GPGGA string
			 * @param gprmc_string the NMEA GPRMC string
			 * 
			 * @return true, if parsing was successful and no error occured
			 */ 
			bool
			parse_nmea_string (const std::string& gpgga_string,
				const std::string& gprmc_string);
	

			/**
			 * @return the invalid flag
			 */
			inline bool
			get_invalid() const;



			/**
			 * @return the time
			 */
			inline double
			get_time() const;
			
			
			/**
			 * @brief Interpolates a new GPSPoint.
			 * 
			 * The new GPSPoint is located between the two specified
			 * GPSPoints according to the specified weight.
			 * 
			 * @param gpsp_1 a reference to the first GPSPoint
			 * @param gpsp_2 a reference to the second GPSPoint
			 * @param weight_on_first the weight on the first GPSPoint;
			 * 0 <= weight_on_first <= 1 (weight_on_second result in 1 -
			 * weight_on_first)
			 * @return the new GPSPoint
			 * 
			 * @see GeoCoordinate::interpolate
			 */
			static GPSPoint
			interpolate(const GPSPoint& gpsp_1, const GPSPoint& gpsp_2,
				const double weight_on_first);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			serialize (std::ostream& o_stream) const;


			/**
			 * @param value the time
			 */
			void
			set_time (double value);
			
			
		private:

			/**
			 * @brief invalid flag as given from the gps reveicer if provided
			 */
			bool _invalid;
			
			
			/**
			 * @brief the time in seconds since 0.00 o'clock 1. January 1900
			 */
			double _time;
			
			
			/**
			 * @brief Tests if the specified string is numeric.
			 * 
			 * @return true, if string is numeric
			 */
			static bool
			is_numeric(const std::string& test_string);
			
			
			/**
			 * @brief Parses the specified string into the _altitude attribute.
			 * 
			 * @return true, if parsing was successful and no error occured
			 */
			inline bool
			parse_altitude(const std::string& altitude_string);
			
			
			/**
			 * @brief Parses the specified string into the _latitude attribute.
			 * 
			 * @return true, if parsing was successful and no error occured
			 */
			bool
			parse_latitude(const std::string& latitude_string,
				const Heading heading);
			
			
			/**
			 * @brief Parses the specified string into the _longitude attribute.
			 * 
			 * @return true, if parsing was successful and no error occured
			 */
			bool
			parse_longitude(const std::string& longitude_string,
				const Heading heading);
			
			
			/**
			 * @brief Parses the specified string into the _time attribute.
			 * 
			 * @return true, if parsing was successful and no error occured
			 */
			bool
			parse_date_time(const std::string& date_string,
				const std::string& time_string);

	};
	
	
	inline void
	GPSPoint::deserialize(std::istream& i_stream)
	{
		Direction::deserialize(i_stream);
		GeoCoordinate::deserialize(i_stream);
		Serializer::deserialize(i_stream, _time);
	}
	

	inline bool
	GPSPoint::get_invalid() const
	{
		return _invalid;
	}


	inline double	
	GPSPoint::get_time () const
	{
		return _time;
	}
	
	
	inline void
	GPSPoint::serialize(std::ostream& o_stream) const
	{
		Direction::serialize(o_stream);
		GeoCoordinate::serialize(o_stream);
		Serializer::serialize(o_stream, _time);
	}


	inline void
	GPSPoint::set_time (double value ) 
	{
		_time = value;
	}
	
	
	inline bool
	GPSPoint::parse_altitude(const std::string& altitude_string)
	{
		if (!is_numeric(altitude_string))
			return false;
		
		_altitude = atof(altitude_string.c_str());
		return true;
	}
	
}  //namespace mapgeneration

#endif //GPSPOINT_H
