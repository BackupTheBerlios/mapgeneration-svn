/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FILTEREDTRACE_H
#define FILTEREDTRACE_H

#include <list>
#include <set>
#include <string>
#include "gpspoint.h"
#include "util/pubsub/servicelist.h"

namespace mapgeneration
{

	/**
	 * @brief FilteredTrace contains a list of GPSPoints.
	 * 
	 * This class provides methods for modifying the FilteredTrace used
	 * in the clustering process.
	 * 
	 * @see TraceProcessor
	 * @see TraceProcessor::run() the cluster algorithm
	 */
	class FilteredTrace : protected std::list<GPSPoint> {

		/* Proxy methods from std::list<GPSPoint> 
		 * 
		 * start.proxy */
		public:
		
			typedef std::list<GPSPoint>::const_iterator const_iterator;
			typedef std::list<GPSPoint>::iterator iterator;
			typedef std::list<GPSPoint>::size_type size_type;
			
			
			iterator
			begin()
			{
				_fast_access_is_valid = false;
				_length_meters_is_valid = false;
				
				std::cout << "********************************" << std::endl;
				std::cout << "* INVALID FAST ACCESS & LENGTH *" << std::endl;
				std::cout << "********************************" << std::endl;
				
				return std::list<GPSPoint>::begin();
			}


			const_iterator
			begin() const
			{
				return std::list<GPSPoint>::begin();
			}


			iterator
			end()
			{
				_fast_access_is_valid = false;
				_length_meters_is_valid = false;
				
				std::cout << "********************************" << std::endl;
				std::cout << "* INVALID FAST ACCESS & LENGTH *" << std::endl;
				std::cout << "********************************" << std::endl;
				
				return std::list<GPSPoint>::end();
			}


			const_iterator
			end() const
			{
				return std::list<GPSPoint>::end();
			}
			

			iterator
			erase(iterator item)
			{
				_fast_access_is_valid = false;
				_length_meters_is_valid = false;
				
				std::cout << "********************************" << std::endl;
				std::cout << "* INVALID FAST ACCESS & LENGTH *" << std::endl;
				std::cout << "********************************" << std::endl;
				
				return std::list<GPSPoint>::erase(item);
			}


			iterator
			erase(iterator from, iterator to)
			{
				_fast_access_is_valid = false;
				_length_meters_is_valid = false;
				
				std::cout << "********************************" << std::endl;
				std::cout << "* INVALID FAST ACCESS & LENGTH *" << std::endl;
				std::cout << "********************************" << std::endl;
				
				return std::list<GPSPoint>::erase(from, to);
			}


			const GPSPoint&
			front() const
			{
				return std::list<GPSPoint>::front();
			}
			
			size_type
			size() const
			{
				return std::list<GPSPoint>::size();
			}
			
			
			void
			splice(iterator insert_at,	FilteredTrace& from_list,
				iterator begin_incl, iterator end_excl)
			{
				_fast_access_is_valid = false;
				_length_meters_is_valid = false;
				
				std::cout << "********************************" << std::endl;
				std::cout << "* INVALID FAST ACCESS & LENGTH *" << std::endl;
				std::cout << "********************************" << std::endl;
				
				return std::list<GPSPoint>::splice(insert_at, from_list,
					begin_incl, end_excl);
			}
		/* end.proxy */


		public:

			/**
			 * @brief Default Constructor.
			 */
			FilteredTrace(pubsub::ServiceList* service_list);
			
			
			/**
			 * @brief Copy Constructor.
			 * 
			 * @param filtered_trace the FilteredTrace that will be copied
			 */
			FilteredTrace(const FilteredTrace& filtered_trace);

			
			/**
			 * @brief Calculates the directions of the GPSPoints.
			 */
			void
			calculate_directions();

			
			/**
			 * @brief Calculates the IDs of the tiles that are needed for the
			 * FilteredTrace.
			 */
			void
			calculate_needed_tile_ids(double radius);

			
			/**
			 * @see mapgeneration_util::Serailizer
			 */
			void
			deserialize(std::istream &i_stream);

			
			/**
			 * @brief Filters the trace by merging GPSPoints.
			 * 
			 * This method will merge points if the distance between them
			 * is less x meters.
			 * 
			 * @todo x should be defined elsewhere
			 */
//			void
//			filter();


			/**
			 * @brief Returns a vector containing the tiles which this FilteredTrace
			 * travers.
			 * 
			 * @return the vector of tiles
			 */
			std::vector<unsigned int>
			get_needed_tile_ids();

			
			/**
			 * @brief Returns _time of the last element in the FilteredTrace.
			 * 
			 * @return the time
			 */ 
			double
			get_last_time();
			
			
			/**
			 * @brief Returns the time of the previous starting point.
			 * 
			 * @return the time
			 */
			double
			get_time_from_previous_start();
			
			
			/**
			 * @brief Returns the interpolated GPSPoint at the given position.
			 * 
			 * @param meters The position to interpolate.
			 */
			GPSPoint
			gps_point_at(double meters);
			
			
			/**
			 * @brief Returns corresponding member attribute.
			 * 
			 * @return true, if the GPSPoints in this list has valid altitude
			 * information
			 */
			inline bool
			gps_points_have_valid_altitudes();
			
			
			bool
			last_gps_point_before(
				double input_meters,
				FilteredTrace::const_iterator* output_before_iter = 0,
				FilteredTrace::const_iterator* output_after_iter = 0,
				double* output_before_iter_meters = 0,
				double* output_after_iter_meters = 0);
			
			
			/**
			 * @brief Returns the length of the trace.
			 * 
			 * At the moment this method calculates the length each time it is
			 * called. Should we name it calculate_length therefore, or just
			 * hope that we find a better way?
			 * 
			 * @return Length of the trace in meters.
			 */
			inline double
			length_meters();
			
			
			
			/**
			 * @brief Moves the starting point position to the specified
			 * GeoCoordinate.
			 *
			 * @param new_start_point new starting point position
			 */
			void
			move_start_point(GeoCoordinate& new_start_point);
			
			
			/**
			 * @brief Returns a reference to the vector of needed tile ids.
			 * 
			 * @return Needed
			 */
			inline const std::vector<unsigned int>&
			needed_tile_ids() const;


			/**
			 * @brief Inserts a new GPSPoint at "meter" meter from the beginning
			 * of the FilteredTrace.
			 * 
			 * @param meter distance from beginning
			 * @return iterator pointing to the new GPSPoint
			 */
			std::list<GPSPoint>::iterator
			new_gps_point_at(double meter);
			            
			
			/**
			 * @brief Sets new start at specified point.
			 * 
			 * This point is given as iterator.
			 * 
			 * @param new_start_point iterator pointing to the new starting point
			 * @param removed_gps_points after invokation it contains the number of
			 * GPSPoints which was removed until the new start is reached
			 * @return iterator pointing at the beginning of the FilteredTrace
			 * (that is now the same like new_start_point)
			 */
			std::list<GPSPoint>::iterator
			new_start(std::list<GPSPoint>::iterator new_start_point,
				int* const removed_gps_points = 0);


			/**
			 * @brief Parses a NMEA string and fill the FilteredTrace with
			 * GPSPoints.
			 * 
			 * Initializes objects from type GPSPoint and evokes the method
			 * parse_nmea_string from the GPSPoint class, and parses the NMEA
			 * string into a string which contains the substrings "GGA" in the
			 * first line and "RMC" in the second line.
			 * The initialized objects are put into the list FilteredTrace.
			 * 
			 * @param nmea_string the NMEA string
			 * 
			 * @return true, if parsing was successful and no error occured
			 */
			bool
			parse_nmea_string (const std::string& nmea_string);
			
			
			/**
			 * @see mapgeneration_util::Serailizer
			 */
			void
			serialize(std::ostream& o_stream) const;
			
			
			/**
			 * @brief Sets a vector containing the tiles which this FilteredTrace
			 * travers.
			 * 
			 * @param needed_tile_ids the vector of tiles
			 */
			void
			set_needed_tile_ids(std::vector<unsigned int> needed_tile_ids);
			
			
		private:
		

			std::vector<const_iterator> _fast_access;
			
			
			bool _fast_access_is_valid;
			
			
			/**
			 * @brief Flag indicating that the GPSPoints has valid altitudes.
			 */
			bool _gps_points_have_valid_altitudes;
			
			
			double _length_meters;
			
			
			bool _length_meters_is_valid;
			
			
			/**
			 * @brief a vector of the tile IDs which are needed by the FilteredTrace
			 */
			std::vector<unsigned int> _needed_tile_ids;
			
		
			/**
			 * @brief a vector containing every GPSPoint from the previous starting
			 * point until the actual starting point exclusive.
			 */
			std::vector<GPSPoint> _points_from_previous_start;
			
			
			pubsub::ServiceList* _service_list;
			
			
			void
			build_fast_access();


			double
			length_meters(const const_iterator& begin_incl,
				const const_iterator& end_incl) const;
	};

	
	inline bool
	FilteredTrace::gps_points_have_valid_altitudes()
	{
		return _gps_points_have_valid_altitudes;
	}
	
	
	inline double
	FilteredTrace::length_meters()
	{
		if ( !_length_meters_is_valid )
		{
			_length_meters = length_meters(begin(), --(end()));
			_length_meters_is_valid = true;
		}
		
		return _length_meters;
	}
	
	
	inline const std::vector<unsigned int>&
	FilteredTrace::needed_tile_ids() const
	{
		return _needed_tile_ids;
	}
	

} // namespace mapgeneration

#endif //FILTEREDTRACE_H
