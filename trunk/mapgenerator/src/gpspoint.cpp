/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "gpspoint.h"

#include <ctime>
#include <cmath>
#include <vector>
#include "util/constants.h"


namespace mapgeneration
{

	GPSPoint::GPSPoint()
	: Direction(), GeoCoordinate(), _invalid(false), _time(0)
	{
	}
	  
	
	GPSPoint::GPSPoint(double latitude, double longitude, double altitude, double time)
	: Direction(), GeoCoordinate(latitude, longitude, altitude),
		_invalid(false), _time(time)
	{
	}
	
	
	GPSPoint::GPSPoint(const GeoCoordinate& geo_coordinate)
	: Direction(), GeoCoordinate(geo_coordinate), _invalid(false), _time(0)
	{
	}
  

	bool
	GPSPoint::parse_nmea_string (const std::string& gpgga_string, const std::string& gprmc_string) 
	{
		const char DELIMITER = ',';
		const int GPGGA_DELIMITERS[8] = {9, -1, 2, 3, 4, 5, -1, 1};
		const int GPRMC_DELIMITERS[8] = {-1, 2, 3, 4, 5, 6, 9, 1};
		
		enum DelimiterIndex
		{
			ALTITUDE = 0,
			INVALID,
			LATITUDE,
			LATITUDE_HEADING,
			LONGITUDE,
			LONGITUDE_HEADING,
			DATE,
			TIME
		};
		
		std::string altitude;
		std::string latitude;
		Heading latitude_heading;
		std::string longitude;
		Heading longitude_heading;
		std::string date;
		std::string time;
		bool found_altitude = false;
		bool found_latitude = false;
		bool found_latitude_heading = false;
		bool found_longitude = false;
		bool found_longitude_heading = false;
		bool found_date = false;
		bool found_time = false;
		
		/* First searches GPGGA string... */
		bool exit_loop = false;
		int run_index = 0;
		std::string::size_type delimiter_in_string_index = 0;
		std::string::size_type next_delimiter_in_string_index
			= gpgga_string.find(DELIMITER, delimiter_in_string_index);
		
		if (next_delimiter_in_string_index == std::string::npos)
			exit_loop = true;
			
		while (!exit_loop)
		{
//			std::cout << "run_index=" << run_index << " & substr="
//				<< gpgga_string.substr(delimiter_in_string_index,
//							next_delimiter_in_string_index - delimiter_in_string_index)
//				<< "; ";
				
			if (run_index == GPGGA_DELIMITERS[ALTITUDE])
			{
				altitude.append(gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1));
				found_altitude = true;
			}

			if (run_index == GPGGA_DELIMITERS[INVALID])
			{
				std::string invalid_string = gpgga_string.substr(
					delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1
				);
				
				if (invalid_string == "V")
					_invalid = true;
			}

			if (run_index == GPGGA_DELIMITERS[LATITUDE])
			{
				latitude.append(gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1));
				found_latitude = true;
			}

			if (run_index == GPGGA_DELIMITERS[LATITUDE_HEADING])
			{
				std::string heading = gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1);
				if (heading == "N")
					latitude_heading = _NORTH;
				else if (heading == "S")
					latitude_heading = _SOUTH;
				else
				{
//					std::cout << "latitude_heading=" << heading << "; ";
					return false;
				}
				found_latitude_heading = true;
			}

			if (run_index == GPGGA_DELIMITERS[LONGITUDE])
			{
				longitude.append(gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1));
				found_longitude = true;
			}

			if (run_index == GPGGA_DELIMITERS[LONGITUDE_HEADING])
			{
				std::string heading = gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1);
				if (heading == "E")
					longitude_heading = _EAST;
				else if (heading == "W")
					longitude_heading = _WEST;
				else
				{
//					std::cout << "longitude_heading=" << heading << "; ";
					return false;
				}
				found_longitude_heading = true;
			}

			if (run_index == GPGGA_DELIMITERS[DATE])
			{
				date.append(gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1));
				found_date = true;
			}

			if (run_index == GPGGA_DELIMITERS[TIME])
			{
				time.append(gpgga_string.substr(delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1));
				found_time = true;
			}
			
			delimiter_in_string_index = next_delimiter_in_string_index;
			next_delimiter_in_string_index = gpgga_string.find(DELIMITER, delimiter_in_string_index + 1);
			if (next_delimiter_in_string_index == std::string::npos)
				exit_loop = true;
			
			++run_index;
		}
		
		/* Now searches GPRMC string... */
		exit_loop = false;
		run_index = 0;
		delimiter_in_string_index = 0;
		next_delimiter_in_string_index = gprmc_string.find(DELIMITER, delimiter_in_string_index);
		
		if (next_delimiter_in_string_index == std::string::npos)
			exit_loop = true;;
			
		while (!exit_loop)
		{
//			std::cout << "run_index=" << run_index << " & substr="
//				<< gprmc_string.substr(delimiter_in_string_index,
//							next_delimiter_in_string_index - delimiter_in_string_index)
//				<< "; ";
				
			if (run_index == GPRMC_DELIMITERS[ALTITUDE])
			{
				if (!found_altitude)
				{
					altitude.append(gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1));
					found_altitude = true;
				}
			}

			if (run_index == GPRMC_DELIMITERS[INVALID])
			{
				std::string invalid_string = gprmc_string.substr(
					delimiter_in_string_index + 1,
					next_delimiter_in_string_index - delimiter_in_string_index - 1
				);
				
				if (invalid_string == "V")
					_invalid = true;
			}

			if (run_index == GPRMC_DELIMITERS[LATITUDE])
			{
				if (!found_latitude)
				{
					latitude.append(gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1));
					found_latitude = true;
				}
			}

			if (run_index == GPRMC_DELIMITERS[LATITUDE_HEADING])
			{
				if (!found_latitude_heading)
				{
					std::string heading = gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1);
					if (heading == "N")
						latitude_heading = _NORTH;
					else if (heading == "S")
						latitude_heading = _SOUTH;
					else
					{
//						std::cout << "latitude_heading=" << heading << "; ";
						return false;
					}
					found_latitude_heading = true;
				}
			}

			if (run_index == GPRMC_DELIMITERS[LONGITUDE])
			{
				if (!found_longitude)
				{
					longitude.append(gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1));
					found_longitude = true;
				}
			}

			if (run_index == GPRMC_DELIMITERS[LONGITUDE_HEADING])
			{
				if (!found_longitude_heading)
				{
					std::string heading = gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1);
					if (heading == "E")
						longitude_heading = _EAST;
					else if (heading == "W")
						longitude_heading = _WEST;
					else
					{
//						std::cout << "longitude_heading=" << heading << "; ";
						return false;
					}
					found_longitude_heading = true;
				}
			}

			if (run_index == GPRMC_DELIMITERS[DATE])
			{
				if (!found_date)
				{
					date.append(gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1));
					found_date = true;
				}
			}

			if (run_index == GPRMC_DELIMITERS[TIME])
			{
				if (!found_time)
				{
					time.append(gprmc_string.substr(delimiter_in_string_index + 1,
						next_delimiter_in_string_index - delimiter_in_string_index - 1));
					found_time = true;
				}
			}
			
			delimiter_in_string_index = next_delimiter_in_string_index;
			next_delimiter_in_string_index = gprmc_string.find(DELIMITER, delimiter_in_string_index + 1);
			if (next_delimiter_in_string_index == std::string::npos)
				break;
			
			++run_index;
		}

//		std::cout << "\t";
//		std::cout << "altitude=" << altitude << "; "
//			<< "latitude=" << latitude << "; "
//			<< "longitude=" << longitude << "; "
//			<< "date=" << date << "; "
//			<< "time=" << time << "; "
//			<< "\t";
		
		/* Tries to parse the altitude string. On failure _altitude is set to -1.0.
		 * No return statement here, because we consider _altitude less importent! */
		/** @todo Define a value for a invalid altitude (perhaps -1000000, because
		 * this value is never reached in real world!) */
		if (!found_altitude || !parse_altitude(altitude))
			_altitude = -1.0;
//		std::cout << "after_altitude_parse; ";
		
		if (!found_latitude || !parse_latitude(latitude, latitude_heading))
			return false;
//		std::cout << "after_latitude_parse; ";
			
		if (!found_longitude || !parse_longitude(longitude, longitude_heading))
			return false;
//		std::cout << "after_longitude_parse; ";
		
		if (!found_date || !found_time || !parse_date_time(date, time))
			return false;
//		std::cout << "after_date_time_parse; ";
		
		return true;
	}
	
	
	GPSPoint
	GPSPoint::interpolate(const GPSPoint& gpsp_1, const GPSPoint& gpsp_2, const double weight_on_first)
	{
		/* interpolation of two GPSPoints with the aid of the weight on
		 * the first GPSPoint */
		double weight_on_second = 1 - weight_on_first;
		GPSPoint interpolated_gps_point(GeoCoordinate::interpolate_default(gpsp_1, gpsp_2, weight_on_first));
		interpolated_gps_point.set_time(gpsp_1.get_time() * weight_on_first + 
			gpsp_2.get_time() * weight_on_second);
		interpolated_gps_point.set_direction(
			gpsp_1.get_direction() * weight_on_first +
			gpsp_2.get_direction() * weight_on_second
		);
		
		return interpolated_gps_point;
	}
	
	
	inline bool
	GPSPoint::is_numeric(const std::string& test_string)
	{
		bool found_dot = false;
		for (int i = 0; i < test_string.size(); ++i)
		{
			if (!isdigit(test_string[i]))
			{
				if ( (!found_dot) && (test_string[i] == '.'))
					found_dot = true;
				else
					return false;
			}
		}
		
		return true;
	}
	

	inline bool
	GPSPoint::parse_altitude(const std::string& altitude_string)
	{
		if (!is_numeric(altitude_string))
			return false;
		
		_altitude = atof(altitude_string.c_str());
		return true;
	}
	
	
	inline bool
	GPSPoint::parse_latitude(const std::string& latitude_string, const Heading heading)
	{
		if (!is_numeric(latitude_string))
			return false;
			
		if ((heading != _NORTH) && (heading != _SOUTH))
			return false;
			
		std::string degrees = latitude_string.substr(0, 2);
		std::string minutes = latitude_string.substr(2);
		
		_latitude = atof(degrees.c_str()) + atof(minutes.c_str()) / 60.0;
		
		if (heading == _SOUTH)
			_latitude = -_latitude;
		
		return true;
	}
	
	
	inline bool
	GPSPoint::parse_longitude(const std::string& longitude_string, const Heading heading)
	{
		if (!is_numeric(longitude_string))
			return false;
			
		if ((heading != _EAST) && (heading != _WEST))
			return false;
			
		std::string degrees = longitude_string.substr(0, 3);
		std::string minutes = longitude_string.substr(3);
		
		_longitude = atof(degrees.c_str()) + atof(minutes.c_str()) / 60.0;
		
		if (heading == _WEST)
			_longitude = -_longitude;
			
		return true;
	}
	
	
	inline bool
	GPSPoint::parse_date_time(const std::string& date_string, const std::string& time_string)
	{
//		std::cout << "parse_date_time; ";
//		std::cout << "date=" << date_string << "; time=" << time_string << "; ";
		if (!is_numeric(date_string) || !is_numeric(time_string))
			return false;

		/* Date string format: DDMMYY or DDMMYYYY(YYYY...) */		
		if ((date_string.length() != 6) && (date_string.length() < 8))
			return false;
		
		/* Time string format: HHMMSS(.xxxxxxxxxxx...). But at least six digits! */
		/* First test: */
		if (time_string.length() < 6)
			return false;
		
		/* Second test: */
		std::string::size_type dot_index = time_string.find('.');
		if ((dot_index != std::string::npos) && (dot_index < 6))
			return false;
		
//		std::cout << "after_tests; ";
		

		std::string year_string = date_string.substr(4);
		int year = atoi(year_string.c_str());
		
		struct tm date_time;
		date_time.tm_isdst = 0; /* no summertime */
		
		if (year_string.length() >= 4)
		{
			/* year given in full length */
			date_time.tm_year = year - 1900;
		} else if (year_string.length() == 2)
		{
		 /* year given only with last two digits.
		  * assume that the year is max 100 year before actual year! */
		  time_t temp_time = time(0);
			struct tm actual_system_time = *gmtime(&temp_time);
			if (actual_system_time.tm_year % 100 < year)
				date_time.tm_year = (((actual_system_time.tm_year / 100) - 1) * 100) + year;
			else
				date_time.tm_year = ((actual_system_time.tm_year / 100) * 100) + year ;
		}
		
		date_time.tm_mon = atoi(date_string.substr(2, 2).c_str()) - 1;
		date_time.tm_mday = atoi(date_string.substr(0, 2).c_str());
		
		/* time parsing: Will ignore milliseconds or smaller! */
		date_time.tm_hour = atoi(time_string.substr(0 ,2).c_str());
		date_time.tm_min = atoi(time_string.substr(2, 2).c_str());
		date_time.tm_sec = atoi(time_string.substr(4, 2).c_str());
		
		time_t mktime_result = mktime(&date_time);
		
		if (mktime_result == -1)
			return false;
			
		
		
		_time = (double)mktime_result; /** @todo Use correct cast here! */
		return true;
	}
	
} // namespace mapgeneration
