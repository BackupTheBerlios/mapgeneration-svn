/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "filteredtrace.h"

#include <cmath>
#include <set>

#include "tile.h"
#include "util/mlog.h"
#include "util/pubsub/servicesystem.h"

using mapgeneration_util::MLog;

namespace mapgeneration
{	

	FilteredTrace::FilteredTrace(pubsub::ServiceList* service_list)
	: _fast_access(),
		_gps_points_have_valid_altitudes(true),
		_cached_length_m(0.0),
		_needed_tile_ids(),
//		_points_from_previous_start(),
		_service_list(service_list),
		_cached_size(0)
	{
	}

	
	FilteredTrace::FilteredTrace(const FilteredTrace& filtered_trace)
	: std::list<GPSPoint>::list(filtered_trace),
		_fast_access(filtered_trace._fast_access),
		_gps_points_have_valid_altitudes(filtered_trace._gps_points_have_valid_altitudes),
		_cached_length_m(filtered_trace._cached_length_m),
		_needed_tile_ids(filtered_trace._needed_tile_ids),
//		_points_from_previous_start(filtered_trace._points_from_previous_start),
		_service_list(filtered_trace._service_list),
		_cached_size(filtered_trace._cached_size)
	{
		/** @todo Check if this copy constructor works. */
	}


	void
	FilteredTrace::calculate_directions()
	{
		if (_cached_size < 1)
			return;
		
		iterator iter = begin();
		iterator previous_iter = iter;
		++iter;
		
		iterator iter_end = end();
		while (iter != iter_end)
		{
			previous_iter->set_direction( previous_iter->bearing(*iter) );
			
			previous_iter = iter;
			++iter;
		}
		
		// Initialize iterators
/*		if (size() < 2)
			return;
		
		iterator iter = begin();
		iterator iter_end = end();

		iterator previous_iter = iter;
		++iter;
		
		// Initialize direction variables
		double direction = 0;
		double previous_direction = 0;
		
		// Calculate direction for first point
		direction = previous_iter->bearing(*iter);
		previous_iter->set_direction(direction);
		previous_direction = direction;
		previous_iter = iter;
		++iter;
		for (; iter != iter_end; ++iter)
		{
			direction = previous_iter->bearing(*iter);
/*			previous_point->set_direction(Direction::interpolate(
				previous_direction, direction, 0.5));
			previous_direction = direction;********
			previous_iter->set_direction(direction); // NEW NEW NEW NEW NEW NEW NEW NEW
			previous_iter = iter;
		}
		
		previous_iter->set_direction(direction);*/
	}


	void
	FilteredTrace::calculate_needed_tile_ids(double radius,
		bool use_cached_size)
	{
		_needed_tile_ids.clear();
		
		int my_size = _cached_size;
		if (!use_cached_size)
			my_size = size();
		
		if (my_size < 1)
			return;
		
		std::set<unsigned int> temp_set;
		
		if (my_size == 1)
		{
			std::vector<Tile::Id> temp_vector
				= Tile::get_needed_tile_ids(front(), radius);
			temp_set.insert(temp_vector.begin(), temp_vector.end());
			
			_needed_tile_ids.insert(_needed_tile_ids.end(), temp_set.begin(), 
				temp_set.end());
			
			return;
		}
		
		const_iterator iter = begin();
		const_iterator iter_end = end();

		const_iterator previous_iter = iter;
		++iter;
		for (; iter != iter_end; ++iter, ++previous_iter)
		{
			/** @todo Get the treshold from another position. */
			/* the iterator invokes the method get_needed_tile_ids with the 
			 * threshold "30" and puts values into the vector needed_tile_ids */
			std::vector<Tile::Id> temp_vector
				= Tile::get_needed_tile_ids(*previous_iter, *iter, radius);
			temp_set.insert(temp_vector.begin(), temp_vector.end());
		}
		
		_needed_tile_ids.insert(_needed_tile_ids.end(), temp_set.begin(), 
			temp_set.end());
	}
	
	
	double
	FilteredTrace::compute_distance_iteratively(const_iterator iter,
		const_iterator iter_end, bool use_iter_end) const
	{
		// computes the length of iter, iter_end
		// use_iter_end decides if [) or []!
		
		if ( use_iter_end && (iter == iter_end) )
			return 0.0;
		
		assert( !((!use_iter_end) && (iter == iter_end)) );
		
		const_iterator previous_iter = iter;
		++iter;
		
		double length_m = 0.0;
		while (iter != iter_end)
		{
			length_m += previous_iter->distance(*iter);
			
			previous_iter = iter;
			++iter;
		}
		
		if (use_iter_end)
			length_m += previous_iter->distance(*iter);

		return length_m;
	}
	
	
	double
	FilteredTrace::curvature_at(double meters) const
	{
		assert( (meters >= 0.0) && (meters < length_m()) );
		
/*		if (_cached_size <= 1)
		{
			mlog(MLog::error, "FilteredTrace") << "Empty FilteredTrace: "
				<< "Cannot calculate new gps point at " << meters << "m!\n";
			return 0;
		}
		
		if (meters<0 || meters>=length_m())
		{
			mlog(MLog::error, "FilteredTrace") << "Position does not exist: "
				<< "Cannot calculate new gps point at " << meters << "m!\n";
			return 0;
		}*/

		const_iterator before_iter;
		const_iterator after_iter;
		double before_iter_meters;
		double after_iter_meter;
		gps_points_before_and_after(meters, &before_iter, &after_iter,
			&before_iter_meters, &after_iter_meter);
		
		assert(before_iter_meters < after_iter_meter);
		assert(after_iter != end());
		
		double curvature = 
			(before_iter->angle_difference_signed(*after_iter))
			/ (after_iter_meter - before_iter_meters);
		
		return curvature;
	}
	
	
	double
	FilteredTrace::curvature_at(FilteredTrace::const_iterator point_iter) const
	{
		assert(point_iter != end());
		
		const_iterator next_point_iter = point_iter;
		++next_point_iter;
		
		assert(next_point_iter != end());
		
		double curvature = 
			(point_iter->angle_difference_signed(*next_point_iter))
			/ (compute_distance_iteratively(point_iter, next_point_iter, true));
		
		return curvature;
	}
	
	
	void
	FilteredTrace::deserialize(std::istream& i_stream)
	{
		/** @todo Serialize and deserialize _needed_tiles */
		Serializer::deserialize(i_stream, *static_cast< std::list<GPSPoint>* >(this));
		Serializer::deserialize(i_stream, _gps_points_have_valid_altitudes);
		Serializer::deserialize(i_stream, _needed_tile_ids);
//		Serializer::deserialize(i_stream, _points_from_previous_start);
	}
	
	
	double
	FilteredTrace::distance_to_next_point(double meters) const
	{
		assert( (meters >= 0.0) && (meters <= length_m() ) );
		
		return distance_to_nth_point(meters, 1);
	}
	
	
	double
	FilteredTrace::distance_to_nth_point(double meters, int n,
		int* reached) const
	{
		assert( (meters >= 0.0) && (meters <= length_m()) );
		assert(n >= 1);
		
		if (meters == length_m())
		{
			if (reached != 0)
				*reached = 1;
			
			return 0.0;
		}
		
		const_iterator after_meters_iter;
		double position_m;
		gps_points_before_and_after(meters, 0, &after_meters_iter,
			0, &position_m);
		
		assert(after_meters_iter != end());
		
		int i = 1;
		const_iterator iter = after_meters_iter;
		const_iterator iter_end = end();
		while ( (i < n) && (iter != iter_end) )
		{
			++iter;
			++i;
		}
		
		if (iter != iter_end)
		{
			position_m += compute_distance_iteratively(
				after_meters_iter, iter, true);
		} else
		{
			position_m += compute_distance_iteratively(
				after_meters_iter, iter, false);
			--i;
		}
		
		if (reached != 0)
			*reached = i;
		
		return position_m - meters;
	}
		
	
/*	void
	FilteredTrace::filter()
	{
		iterator iter = begin();
		while(iter != end())	/* ++iter is done inside loop by ++iter and erase! *
		{
			GPSPoint old_point = *iter;			
			iterator point_1 = iter;
			double merged_points = 1;
			++iter;
			double new_latitude = point_1->get_latitude();
			double new_longitude = point_1->get_longitude();
			double new_altitude = point_1->get_altitude();
			double new_time = point_1->get_time();
			
			/* while the  distance  between  points  is less 4.0 
			 * they are  all  merged into 1 point *
			while((iter != end()) && (old_point.distance(*iter) < 4.0))
			{
				new_latitude += iter->get_latitude();
				new_longitude += iter->get_longitude();
				new_altitude += iter->get_altitude();
				new_time += iter->get_time();				
				merged_points += 1;
				
				iter = erase(iter);
			}
			
			/*calculates  the new  merged time, longitude, latitude  and altitude *
			new_latitude /= merged_points;
			new_longitude /= merged_points;
			new_altitude /= merged_points;
			new_time /= merged_points;
			
			/* setting the attributes *
			point_1->set(new_latitude, new_longitude, new_altitude);
			point_1->set_time(new_time);
		}
	}*/
	
	
/*	GPSPoint
	FilteredTrace::old_gps_point_at(double meters)
	{
			iterator iter = begin();
			iterator iter_end = end();
				
			if (iter == iter_end)
			{
				mlog(MLog::error, "FilteredTrace") << "Empty FilteredTrace: Cannot calculate new gps point at " << meters << " m!\n";
				GPSPoint gps_point;
				return gps_point;
			}
		
			std::list<GPSPoint>::iterator previous_point = iter;
			++iter;
		
			double left_distance = meters;
			double distance_from_previous_point;
			for(; iter != iter_end; ++iter)
			{
				distance_from_previous_point = previous_point->distance_m(*iter); // calculating the distance from the previous point
				if(left_distance < distance_from_previous_point)
				{
					double weight = 1.0 - left_distance / previous_point->distance_m(*iter);
					GPSPoint new_point = iter->interpolate(*previous_point , *iter, weight);
					--iter;
					new_point.set_direction(iter->get_direction());
					++iter;
					return new_point;
				} else 
				{
					left_distance -= distance_from_previous_point;
					previous_point = iter;
				}
			}
		}*/
	
	
	GPSPoint
	FilteredTrace::gps_point_at(double meters) const
	{
		assert( (meters >= 0.0) && (meters <= length_m()) );
		
		if (meters == length_m())
			return back();
		
		//GPSPoint old_result = old_gps_point_at(meters);
		
/*		if (_cached_size <= 1)
		{
			mlog(MLog::error, "FilteredTrace") << "Empty FilteredTrace: "
				<< "Cannot calculate new gps point at " << meters << "m!\n";
			GPSPoint gps_point;
			return gps_point;
		}
		
		if (meters<0 || meters>=length_m())
		{
			mlog(MLog::error, "FilteredTrace") << "Position does not exist: "
				<< "Cannot calculate new gps point at " << meters << "m!\n";
			GPSPoint gps_point;
			return gps_point;
		}*/

		const_iterator before_iter;
		const_iterator after_iter;
		double before_iter_meters;
		double after_iter_meter;
		gps_points_before_and_after(meters, &before_iter, &after_iter,
				&before_iter_meters, &after_iter_meter);
		
		assert(before_iter_meters < after_iter_meter);
		assert(after_iter != end());
		assert(before_iter_meters <= meters);
		
		double left_distance = meters - before_iter_meters;
		assert(left_distance < before_iter->distance(*after_iter));
		
		double weight
			= 1.0 - (left_distance / before_iter->distance(*after_iter));
		
		GPSPoint new_point
			= GPSPoint::interpolate(*before_iter , *after_iter, weight);
		return new_point;
	}
	
	
	void
	FilteredTrace::gps_points_before_and_after(
		double input_meters,
		FilteredTrace::const_iterator* output_before_iter,
		FilteredTrace::const_iterator* output_after_iter,
		double* output_before_iter_meters,
		double* output_after_iter_meters) const
	{
		assert(_cached_size >= 1);
		assert( (input_meters >= 0.0) && (input_meters <= length_m()) );
		
		if (input_meters == length_m())
		{
			assert(_fast_access.back().second == length_m());
			
			const_iterator iter = _fast_access.back().first;
			
			if (output_before_iter != 0)
				*output_before_iter = iter;
				
			if (output_after_iter != 0)
				*output_after_iter = end();
			
			if (output_before_iter_meters != 0)
				*output_before_iter_meters = length_m();
			
			if (output_after_iter_meters != 0)
				*output_after_iter_meters = length_m();
			
			return;
		}
		
/*		/** @todo exception?! *********
		if (_cached_size <= 0 || input_meters < 0.0 || input_meters > length_m())
		{
			if (output_before_iter != 0)
				*output_before_iter = end();
				
			if (output_after_iter != 0)
				*output_after_iter = end();
			
			if (output_before_iter_meters != 0)
				*output_before_iter_meters = -1.0;
			
			if (output_after_iter_meters != 0)
				*output_after_iter_meters = -1.0;

			return false;
		}*/
		
		// outsource meters_per_entry!!! this calc twice is critical.
		double meters_per_entry = length_m() / static_cast<double>(_fast_access.size() - 1);
		int entry = static_cast<int>(floor(input_meters / meters_per_entry));
		
		assert(entry < _fast_access.size() - 1);
		assert(_fast_access[entry].first != end());
		assert(_fast_access[entry].second <= input_meters);
		
		const_iterator previous_iter = end();
		const_iterator iter = _fast_access[entry].first;
		const_iterator iter_end = end();
		
		double previous_meters = -1.0;
		double current_meters = _fast_access[entry].second;
		
		// second condition is fulfilled iff first one is.
		// So actually we don't need it.
		// I'm paranoid ... so DON'T touch it :-)
		while ( (current_meters <= input_meters) && (iter != iter_end) )
		{
			previous_iter = iter;
			++iter;
			
			assert(iter != iter_end);
			
			previous_meters = current_meters;
			current_meters += compute_distance_iteratively(
				previous_iter, iter, true);
		}
		
		assert(previous_iter != end());
		assert(previous_meters != -1.0);
		
		if (output_before_iter != 0)
			*output_before_iter = previous_iter;
			
		if (output_after_iter != 0)
			*output_after_iter = iter;
		
		if (output_before_iter_meters != 0)
			*output_before_iter_meters = previous_meters;
		
		if (output_after_iter_meters != 0)
			*output_after_iter_meters = current_meters;
	}


	bool
	FilteredTrace::parse_nmea_string(const std::string& nmea_string)
	{
		/* Guarantees that no spaces are inside nmea_string */
		/* ... */
		/** @todo Think about that later (is it necessary???) */
		
		const std::string ENDLINE = "\n";
		const std::string GPGGA_PREFIX("$GPGGA");
		const std::string GPRMC_PREFIX("$GPRMC");
		const int PREFIX_BEGIN_INDEX = 0;
		const int PREFIX_LENGTH = 6;
		const int TIME_BEGIN_INDEX = 7;
		const int TIME_LENGTH = 6;

		bool use_gpgga_string = true;
		bool found_gprmc_string = false;
		bool found_gpgga_string = false;
		std::string gpgga_string;
		std::string gprmc_string;
		std::string time_string;

		GPSPoint gps_point;
		int found_gps_points = 0;	
			
		std::string::size_type begin_index = nmea_string.find_first_not_of(ENDLINE);
		if (begin_index == std::string::npos)	return false;
		std::string::size_type end_index = nmea_string.find_first_of(ENDLINE, begin_index);
		
		while(end_index != std::string::npos)
		{
			std::string nmea_prefix(
				nmea_string.substr(begin_index + PREFIX_BEGIN_INDEX, PREFIX_LENGTH)
			);
			
//			std::cout << "nmea_prefix=" << nmea_prefix << "; ";
			if (nmea_prefix == GPRMC_PREFIX)
			{
//				std::cout << "GPRMC; ";
				if (use_gpgga_string)
				{
//					std::cout << "!found_gprmc_string; ";
					if (!found_gprmc_string)
					{
						if (!found_gpgga_string)
						{
//							std::cout << "!found_gpgga_string; ";
							time_string.append(nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH));
							gprmc_string.append(nmea_string.substr(begin_index, end_index - begin_index));
							found_gprmc_string = true;
						} else // explicit: if (found_gpgga_string)
						{
//							std::cout << "found_gpgga_string; ";
							gprmc_string.append(nmea_string.substr(begin_index, end_index - begin_index));
							
							if (nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH) == time_string)
							{
//								std::cout << "same_time; ";
								if (gps_point.parse_nmea_string(gpgga_string, gprmc_string))
								{
//									std::cout << "parse_gps_point ok; ";
									push_back(gps_point);
									++found_gps_points;
								} else
								{
									mlog(MLog::debug, "FilteredTrace::parse_from_nmea")
										<< "Parsing of NMEA strings fails! (GPGGA and GPRMC strings follows) "
										<< gpgga_string << " " << gprmc_string << "\n";
								}
								
								found_gprmc_string = false;
								found_gpgga_string = false;
								gpgga_string.erase();
								gprmc_string.erase();
								time_string.erase();
							} else // explicit: if (nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH) != time_string)
							{
//								std::cout << "!same_time; ";
								gpgga_string.erase();
								found_gpgga_string = false;
								found_gprmc_string = true;
								time_string.erase();
								time_string.append(
									nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH)
								);
							} // end: if (nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH) == time_string)
						} // end: if (!found_gpgga_string)
						
					} else // explicit: if(found_gprmc_string)
					{
						mlog(MLog::info, "FilteredTrace") << "Switching to \"No-GPGGA-Modus\".\n";
						use_gpgga_string = false;
						_gps_points_have_valid_altitudes = false;
						
						if (size() > 0)
						{
							mlog(MLog::debug, "FilteredTrace") << "Must review GPSPoints.\n";
							iterator iter = begin();
							iterator iter_end = end();
							for (; iter != iter_end; ++iter)
							{
								iter->set_altitude(-1.0);
								/** @todo Define a value for a invalid altitude
								 * (perhaps -1000000, because this value is never reached in
								 * real world!) */
							}
						}
						
						if (gps_point.parse_nmea_string("", gprmc_string))
						{
							push_back(gps_point);
							++found_gps_points;
						}
						
						if (gps_point.parse_nmea_string("", nmea_string.substr(begin_index, end_index - begin_index)))
						{
							push_back(gps_point);
							++found_gps_points;
						}
					}
					
				} else // explicit: if (!use_gpgga_string)
				{
//					std::cout << "found_gprmc_string; ";
					if (gps_point.parse_nmea_string("", nmea_string.substr(begin_index, end_index - begin_index)))
					{
						push_back(gps_point);
						++found_gps_points;
					}
				} // end: if (use_gpgga_string)
				
			} else if (use_gpgga_string && (nmea_prefix == GPGGA_PREFIX))
			{
//				std::cout << "GPGGA; ";
				if (!found_gpgga_string)
				{
//					std::cout << "!found_gpgga_string; ";
					if (!found_gprmc_string)
					{
//						std::cout << "!found_gprmc_string; ";
						time_string.append(nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH));
						gpgga_string.append(nmea_string.substr(begin_index, end_index- begin_index));
						found_gpgga_string = true;
					} else // explicit: if (found_gprmc_string)
					{
//						std::cout << "found_gprmc_string; ";
						gpgga_string.append(nmea_string.substr(begin_index, end_index - begin_index));
						
						if (nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH) == time_string)
						{
//							std::cout << "same_time; ";
							if (gps_point.parse_nmea_string(gpgga_string, gprmc_string))
							{
//								std::cout << "parse_gps_point ok; ";
								push_back(gps_point);
								++found_gps_points;
							} else
							{
								mlog(MLog::debug, "FilteredTrace::parse_from_nmea")
									<< "Parsing of NMEA strings fails! (GPGGA and GPRMC strings follows) "
									<< gpgga_string << " " << gprmc_string << "\n";
							}
							
							found_gprmc_string = false;
							found_gpgga_string = false;
							gpgga_string.erase();
							gprmc_string.erase();
							time_string.erase();
						} else // explicit: if (nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH) != time_string)
						{
//							std::cout << "!same_time; ";
							gprmc_string.erase();
							found_gprmc_string = false;
							found_gpgga_string = true;
							time_string.erase();
							time_string.append(
								nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH)
							);
						} // end: if (nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH) == time_string)
					} // end: if (!found_gprmc_string)
				} else // explicit: if (found_gpgga_string)
				{
//					std::cout << "found_gpgga_string; ";
					gpgga_string.erase();
					gpgga_string.append(nmea_string.substr(begin_index, end_index - begin_index));
					time_string.erase();
					time_string.append(nmea_string.substr(begin_index + TIME_BEGIN_INDEX, TIME_LENGTH));
				} // end: if (!found_gpgga_string)
			} // end: if (nmea_prefix == *)
			
			begin_index = nmea_string.find_first_not_of(ENDLINE, end_index);
			if (begin_index == std::string::npos)	break;
			end_index = nmea_string.find_first_of(ENDLINE, begin_index);
			
		} // end: while
		
//		std::cout << std::endl;
		
		return (found_gps_points > 0);
	}
	
	
/*	double
	FilteredTrace::get_last_time()
	{
		return back().get_time();
	}*/

	
	void
	FilteredTrace::precompute_data()
	{
//		std::cout << std::endl << "Precomputing data structures...";

		_cached_size = size();
		
		if (_cached_size <= 0)
		{
			_fast_access.clear();
			_cached_length_m = 0.0;
			
			return;
		}
		
		if (_cached_size == 1)
		{
			_fast_access.resize(1);
			_fast_access[0].first = begin();
			_fast_access[0].second = 0.0;
			_cached_length_m = 0.0;
			
			return;
		}
		
		_cached_length_m = compute_distance_iteratively(begin(), end());
		
		#warning
		#warning FilteredTrace::precompute_data: Little hack to circumvent \
		the use of the _service_list. We should think about a secure method \
		to set the ServiceList after deserialization. Change this!
		#warning
		
		double size_factor = 1.0;
		if (_service_list != 0)
		{
			_service_list->get_service_value("filteredtrace.size_factor",
				size_factor);
		}
		
		assert(size_factor > 0.0);
		
		int new_fast_access_size = static_cast<int>(
			1 + ceil(static_cast<double>(size()) * size_factor) );
		
		assert(new_fast_access_size >= 2);
		
		_fast_access.resize(new_fast_access_size);
		_fast_access[0].first = begin();
		_fast_access[0].second = 0.0;

		const_iterator iter = begin();
		const_iterator previous_iter = iter;
		++iter;
		const_iterator iter_end = end();
		
		assert(iter != iter_end);

		double meters_per_entry = length_m() / static_cast<double>(new_fast_access_size - 1);
		double previous_meters = -1.0;
		double current_meters = 0.0;
		
		int fast_access_index = 1;
		while (iter != iter_end)
		{
			previous_meters = current_meters;
			current_meters += compute_distance_iteratively(
				previous_iter, iter, true);
			
			// Problem. Sometimes current_meters is (better: should be) equal
			// to fast_access_index * meters_per_entry, e.g. when
			// current_meters == length_m(). But unfortunately floating point
			// operations seems to be unprecise. We need a fix, see below.
			while (current_meters > fast_access_index * meters_per_entry)
			{
				// IT ALWAYS HOLDS:               <----------------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// _fast_access[index] <= index * meters_per_entry;
				_fast_access[fast_access_index].first = previous_iter;
				_fast_access[fast_access_index].second = previous_meters;
				
				assert(_fast_access[fast_access_index].second <= fast_access_index * meters_per_entry);
				
				++fast_access_index;
			}
			
			previous_iter = iter;
			++iter;
		}
		
		assert(iter == end());
		assert(current_meters == length_m());
		
		// This assertion does not hold in the mentiones case above.
		// Solution: just set fast_access_index to the correct value.
//		assert(fast_access_index == _fast_access.size() - 1);
		fast_access_index = _fast_access.size() - 1;
		
		_fast_access[fast_access_index].first = previous_iter;
		_fast_access[fast_access_index].second = current_meters; //unschoen, aber is nunmal so.
		
		// Here we have to mirror to the above described fp ops problem.
		// Will test against a slight bigger value instead.
//		assert(_fast_access[fast_access_index].second <= fast_access_index * meters_per_entry);
		assert(_fast_access[fast_access_index].second <= fast_access_index * meters_per_entry + 0.000000001);
		
		
/*		for (; iter != iter_end; ++iter, ++previous_iter, ++iter_index)
		{
			previous_meters = current_meters;
			current_meters += length_m(previous_iter, iter);
//			std::cout << "\t\tcurrent_meters = " << current_meters << std::endl;
			for (; current_meters > meters_per_entry * index && index < _fast_access.size(); ++index)
			{
				_fast_access[index].first = previous_iter;
				_fast_access[index].second = previous_meters;
//				std::cout << "\t_fast_access[" << index << "] at meter "
//					<< meters_per_entry * index << " = " << iter_index - 1 << std::endl;
			}
		}*/

//		std::cout << std::endl;
	}
	
	
	
/*	std::vector<unsigned int>
	FilteredTrace::get_needed_tile_ids()
	{
		return _needed_tile_ids;
	}*/
	
	
	pubsub::ServiceList*
	FilteredTrace::get_service_list()
	{
		return _service_list;
	}
	
	
/*	double
	FilteredTrace::get_time_from_previous_start()
	{
		if (_points_from_previous_start.empty())
		{
			mlog(MLog::info, "FilteredTrace") << "No points from previous start available.\n";
			return 0;
		}
		
		std::vector<GPSPoint>::iterator iter = _points_from_previous_start.begin();
		std::vector<GPSPoint>::iterator iter_end = _points_from_previous_start.end();
		std::vector<GPSPoint>::iterator previous_iter = iter;
		++iter;
		
		/* adds all times from the previous starting point until the actual
		 * starting point *
		double time_from_previous_start = 0.0;
		for(; iter != iter_end; ++iter, ++previous_iter)
		{
			time_from_previous_start += (iter->get_time() - previous_iter->get_time());
		}
		time_from_previous_start += (front().get_time() - previous_iter->get_time());
		
		return time_from_previous_start;
	}*/


/*	void
	FilteredTrace::move_start_point(GeoCoordinate& new_start_point)
	{
		iterator iter_begin = begin();
		iterator iter_end = end();
		
		if (iter_begin == iter_end)
		{
			mlog(MLog::warning, "FilteredTrace") << "Empty FilteredTrace: Cannot move start point!\n";
			return;
		}
		
		++iter_begin;
		iterator iter_second = iter_begin;
		--iter_begin;
		if (iter_second != iter_end)
		{
			/* adjusts the new attributs of the moved starting point *
			double old_distance = iter_begin->distance(*iter_second);
			double new_distance = new_start_point.distance(*iter_second);
			double old_time_difference = iter_second->get_time() - iter_begin->get_time();
			double velocity = old_distance / old_time_difference;
			double new_time = iter_second->get_time() - (new_distance / velocity);
			iter_begin->set_time(new_time);
		} else
			mlog(MLog::warning, "FilteredTrace") << "Only one entry: Cannot modify time!\n";

		iter_begin->set_latitude(new_start_point.get_latitude());
		iter_begin->set_longitude(new_start_point.get_longitude());
		iter_begin->set_altitude(new_start_point.get_altitude());
	}*/
	
	
/*	FilteredTrace::iterator
	FilteredTrace::new_gps_point_at(double meter)
	{
		iterator iter = begin();
		iterator iter_end = end();
			
		if (iter == iter_end)
		{
			mlog(MLog::error, "FilteredTrace") << "Empty FilteredTrace: Cannot calculate new gps point at " << meter << " m!\n";
			return 0;
		}

		iterator previous_point = iter;
		++iter;

		double left_distance = meter;
		double distance_from_previous_point;
		for(; iter != iter_end; ++iter)
		{
			distance_from_previous_point = previous_point->distance(*iter); // calculating the distance from the previous point
			if(left_distance < distance_from_previous_point)
			{
				double weight = 1.0 - left_distance / previous_point->distance(*iter);
				GPSPoint new_point = iter->interpolate(*previous_point , *iter, weight);
				--iter;
				new_point.set_direction(iter->get_direction());
				++iter;
				/* inserts the new calculated Point into  the list *
				return insert(iter, new_point);
			} else 
			{
				left_distance -= distance_from_previous_point;
				previous_point = iter;
			}
		}
		
		return 0;
	}*/
   	
   	
/*	FilteredTrace::iterator
	FilteredTrace::new_start(FilteredTrace::iterator new_start_point,
		int* const removed_gps_points)
	{
		_points_from_previous_start.clear(); 
		for (iterator iter = begin(); iter != new_start_point; ++iter)
		{
			_points_from_previous_start.push_back(*iter);
		}
		
		erase(begin(), new_start_point); 	/* clearing unsused points *
		
		if (removed_gps_points != 0)
			*removed_gps_points = _points_from_previous_start.size();
		
		return begin();
	}*/
   	
  	
	void
	FilteredTrace::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, *static_cast<const std::list<GPSPoint>*>(this));
		Serializer::serialize(o_stream, _gps_points_have_valid_altitudes);
		Serializer::serialize(o_stream, _needed_tile_ids);
//		Serializer::serialize(o_stream, _points_from_previous_start);
	}


/*	void
	FilteredTrace::set_needed_tile_ids(std::vector<unsigned int> needed_tile_ids)
	{
		_needed_tile_ids = needed_tile_ids;
	}*/
	
	
	void
	FilteredTrace::set_service_list(pubsub::ServiceList* service_list)
	{
		_service_list = service_list;
	}
	
	
	double
	FilteredTrace::velocity_at(double meters) const
	{
		assert( (meters >= 0.0) && (meters < length_m()) );
		
		const_iterator before_iter;
		const_iterator after_iter;
		gps_points_before_and_after(meters, &before_iter, &after_iter);
		
		double distance = after_iter->distance(*before_iter);
		double time_diff = after_iter->get_time() - before_iter->get_time();
		
		assert(time_diff > 0.0);
		
		return distance / time_diff;
	}
	
	
	std::ostream&
	operator<<(std::ostream& out, const FilteredTrace& trace)
	{
		std::list<GPSPoint>::const_iterator iter = trace.begin();
		std::list<GPSPoint>::const_iterator iter_end = trace.end();
		
		for (; iter != iter_end; ++iter)
		{
			out << *iter << std::endl;
		}
		
		return out;
	}

} // namespace mapgeneration
