/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FILTEREDTRACE_H
#define FILTEREDTRACE_H

#include <list>

#include "gpspoint.h"
#include "tile.h"

/* Forward declarations ... */
namespace pubsub
{
	
	class ServiceList;
	
} // namespace pubsub
/* Forward declarations done! */

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
	class FilteredTrace : public std::list<GPSPoint> {

		public:

			/**
			 * @brief Default Constructor.
			 */
			FilteredTrace(pubsub::ServiceList* service_list = 0);
			
			
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
			calculate_needed_tile_ids(double radius,
				bool use_cached_size = true);
			
			
			/**
			 * @brief Returns the curvature at the given position.
			 */
			double
			curvature_at(double meters) const;
			
			
			/**
			 * @brief Returns the curvature at the given position.
			 */
			double
			curvature_at(const_iterator point_iter) const;
			
			
			/**
			 * @see mapgeneration_util::Serailizer
			 */
			void
			deserialize(std::istream &i_stream);

			
			double
			distance_to_next_point(double meters) const;
			
			
			double
			distance_to_nth_point(double meters, int n, int* reached = 0) const;
			
			
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
//			std::vector<unsigned int>
//			get_needed_tile_ids();
			
			
			pubsub::ServiceList*
			get_service_list();

			
			/**
			 * @brief Returns _time of the last element in the FilteredTrace.
			 * 
			 * @return the time
			 */ 
//			double
//			get_last_time();
			
			
			/**
			 * @brief Returns the time of the previous starting point.
			 * 
			 * @return the time
			 */
//			double
//			get_time_from_previous_start();
			
			
			/**
			 * @brief Returns the interpolated GPSPoint at the given position.
			 * 
			 * @param meters The position to interpolate.
			 */
			 
//			GPSPoint
//			old_gps_point_at(double meters);
			
			GPSPoint
			gps_point_at(double meters) const;
			
			
			void
			gps_points_before_and_after(
				double input_meters,
				FilteredTrace::const_iterator* output_before_iter = 0,
				FilteredTrace::const_iterator* output_after_iter = 0,
				double* output_before_iter_meters = 0,
				double* output_after_iter_meters = 0) const;
			
			
			/**
			 * @brief Returns corresponding member attribute.
			 * 
			 * @return true, if the GPSPoints in this list has valid altitude
			 * information
			 */
			inline bool
			gps_points_have_valid_altitudes() const;
			
			
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
			length_m() const;
			
			
			
			/**
			 * @brief Moves the starting point position to the specified
			 * GeoCoordinate.
			 *
			 * @param new_start_point new starting point position
			 */
//			void
//			move_start_point(GeoCoordinate& new_start_point);
			
			
			/**
			 * @brief Returns a reference to the vector of needed tile ids.
			 * 
			 * @return Needed
			 */
			inline const std::vector<Tile::Id>&
			needed_tile_ids() const;


			/**
			 * @brief Inserts a new GPSPoint at "meter" meter from the beginning
			 * of the FilteredTrace.
			 * 
			 * @param meter distance from beginning
			 * @return iterator pointing to the new GPSPoint
			 */
//			std::list<GPSPoint>::iterator
//			new_gps_point_at(double meter);
			            
			
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
//			std::list<GPSPoint>::iterator
//			new_start(std::list<GPSPoint>::iterator new_start_point,
//				int* const removed_gps_points = 0);


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
			parse_nmea_string(const std::string& nmea_string);
			
			
			void
			precompute_data();


			/**
			 * @see mapgeneration_util::Serializer
			 */
			void
			serialize(std::ostream& o_stream) const;
			
			
			/**
			 * @brief Sets a vector containing the tiles which this FilteredTrace
			 * travers.
			 * 
			 * @param needed_tile_ids the vector of tiles
			 */
//			void
//			set_needed_tile_ids(std::vector<Tile::Id> needed_tile_ids);
			
			
			void
			set_service_list(pubsub::ServiceList* service_list);
			
			
			double
			velocity_at(double meters) const;
			
			
		private:
		

			double _cached_length_m;
			
			
			std::list<GPSPoint>::size_type _cached_size;
			
			
			std::vector< std::pair<const_iterator, double> > _fast_access;
			
			
			/**
			 * @brief Flag indicating that the GPSPoints has valid altitudes.
			 */
			bool _gps_points_have_valid_altitudes;
			
			
			/**
			 * @brief a vector of the tile IDs which are needed by the FilteredTrace
			 */
			std::vector<Tile::Id> _needed_tile_ids;
			
		
			/**
			 * @brief a vector containing every GPSPoint from the previous starting
			 * point until the actual starting point exclusive.
			 */
			//std::vector<GPSPoint> _points_from_previous_start;
			
			
			pubsub::ServiceList* _service_list;
			
			
			double
			compute_distance_iteratively(const_iterator iter,
				const_iterator iter_end, bool use_iter_end = false) const;
	};
	
	
	std::ostream&
	operator<<(std::ostream& out, const FilteredTrace& trace);

	
	inline bool
	FilteredTrace::gps_points_have_valid_altitudes() const
	{
		return _gps_points_have_valid_altitudes;
	}
	
	
	inline double
	FilteredTrace::length_m() const
	{
		return _cached_length_m;
	}
	
	
	inline const std::vector<Tile::Id>&
	FilteredTrace::needed_tile_ids() const
	{
		return _needed_tile_ids;
	}
	

} // namespace mapgeneration

#endif //FILTEREDTRACE_H
