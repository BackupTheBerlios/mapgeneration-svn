/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "filteredtrace.h"

#include <iostream>
#include <list>
#include <string>

#include "gpspoint.h"
#include "traceprocessor.h"
#include "util/mlog.h"

namespace mapgeneration
{	

	FilteredTrace::FilteredTrace(pubsub::ServiceList* service_list)
	: _fast_access(),
		_gps_points_have_valid_altitudes(true),
		_length_m(-1.0),
		_needed_tile_ids(),
		_points_from_previous_start(),
		_service_list(service_list)
	{
	}

	
	FilteredTrace::FilteredTrace(const FilteredTrace& filtered_trace)
	: std::list<GPSPoint>::list(filtered_trace),
		_fast_access(filtered_trace._fast_access),
		_gps_points_have_valid_altitudes(filtered_trace._gps_points_have_valid_altitudes),
		_length_m(filtered_trace._length_m),
		_needed_tile_ids(filtered_trace._needed_tile_ids),
		_points_from_previous_start(filtered_trace._points_from_previous_start),
		_service_list(filtered_trace._service_list)
	{
		/** @todo Check if this copy constructor works. */
	}


	void
	FilteredTrace::calculate_directions()
	{
		if (size() < 2) return;
		
		iterator iter = begin();
		iterator iter_end = end();
		
		iterator previous_point = iter;
		++iter;
				
		double previous_direction = 0;
		/* selects two successive GPSPoints and calculates the direction */
		for (; iter != iter_end; ++iter)
		{
			previous_point->calculate_direction(*iter);
			previous_direction = previous_point->get_direction();
			previous_point = iter;
		}
		
		previous_point->set_direction(previous_direction);
	}


	void
	FilteredTrace::calculate_needed_tile_ids(double radius)
	{
		std::set<unsigned int> temp_set;
		
		if (size() == 1)
		{
			std::vector<unsigned int> temp_vector
				= front().get_needed_tile_ids(radius);
			temp_set.insert(temp_vector.begin(), temp_vector.end());
			
			_needed_tile_ids.clear();
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
			std::vector<unsigned int> temp_vector
				= GeoCoordinate::get_needed_tile_ids(*previous_iter, *iter,
					radius);
			temp_set.insert(temp_vector.begin(), temp_vector.end());
		}
		
		_needed_tile_ids.clear();
		_needed_tile_ids.insert(_needed_tile_ids.end(), temp_set.begin(), 
			temp_set.end());
	}
	
	
	void
	FilteredTrace::deserialize(std::istream& i_stream)
	{
		/** @todo Serialize and deserialize _needed_tiles */
		Serializer::deserialize(i_stream, *static_cast< std::list<GPSPoint>* >(this));
		Serializer::deserialize(i_stream, _gps_points_have_valid_altitudes);
		Serializer::deserialize(i_stream, _needed_tile_ids);
		Serializer::deserialize(i_stream, _points_from_previous_start);
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
	
	
	GPSPoint
	FilteredTrace::gps_point_at(double meters)
	{
		if (size() <= 1)
		{
			mlog(MLog::error, "FilteredTrace") << "Empty FilteredTrace: "
				<< "Cannot calculate new gps point at " << meters << "m!\n";
			GPSPoint gps_point;
			return gps_point;
		}
		
		if (meters<0 || meters>length_m())
		{
			mlog(MLog::error, "FilteredTrace") << "Position does not exist: "
				<< "Cannot calculate new gps point at " << meters << "m!\n";
			GPSPoint gps_point;
			return gps_point;
		}

		const_iterator point_before;
		const_iterator point_after;
		double point_before_meters;
		
		if (gps_points_before_and_after(meters, &point_before, &point_after,
				&point_before_meters))
		{
			double left_distance = meters - point_before_meters;
			double weight = 1.0 - left_distance / point_before->distance(*point_after);
			GPSPoint new_point = GPSPoint::interpolate(*point_before , *point_after, weight);
			new_point.set_direction(point_before->get_direction());
			return new_point;
			
		} else
		{
			GPSPoint gps_point;
			return gps_point;
		}
	}
	
	
	bool
	FilteredTrace::gps_points_before_and_after(
		double input_meters,
		FilteredTrace::const_iterator* output_before_iter,
		FilteredTrace::const_iterator* output_after_iter,
		double* output_before_iter_meters,
		double* output_after_iter_meters)
	{
		if (size() <= 0)
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
		}

		double meters_per_entry = length_m() / static_cast<double>(_fast_access.size());
		int entry = static_cast<int>(floor(input_meters / meters_per_entry));
		double current_meters = entry * meters_per_entry;

		const_iterator iter = _fast_access[entry];
		const_iterator previous_iter = iter;
		++iter;

		while ( (current_meters < input_meters) && (iter != end()) )
		{
			current_meters += length_m(previous_iter, iter);
			++previous_iter;
			++iter;
		}
		
		if (output_before_iter != 0)
			*output_before_iter = previous_iter;
			
		if (output_after_iter != 0)
			*output_after_iter = iter;
		
		if (output_before_iter_meters != 0)
			*output_before_iter_meters
				= current_meters - length_m(previous_iter, iter);
		
		if (output_after_iter_meters != 0)
			*output_after_iter_meters = current_meters;
		
		return true;
	}


	double
	FilteredTrace::length_m(const const_iterator& begin_incl,
		const const_iterator& end_incl) const
	{
		if (begin_incl == end_incl)
			return 0.0;
		
		const_iterator iter = begin_incl;		
		const_iterator previous_iter = begin_incl;
		const_iterator iter_end = end_incl;
		
		++iter;
		double length_m = 0.0;

		do
		{
			length_m += previous_iter->distance(*iter);
			++previous_iter;
			++iter;
		} while (previous_iter != iter_end);

		return length_m;
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
		std::cout << std::endl << "Precomputing data structures...";
		
		_length_m = length_m(begin(), --(end()));
		
		if (size() <= 0)
		{
			_fast_access.clear();
			std::cout << "ready (size <= 0)" << std::endl;
			return;
		}

		double size_factor;
		_service_list->get_service_value("filteredtrace.size_factor", size_factor);
		
		std::cout << std::endl;
		std::cout << "\tsize_factor = " << size_factor << std::endl;
		std::cout << "\tfiltered_trace.size = " << size() << std::endl;

		_fast_access.resize(static_cast<int>(ceil(static_cast<double>(size()) * size_factor)));
		std::cout << "\tfast_access.size = " << _fast_access.size() << std::endl;
		_fast_access[0] = begin();

		const_iterator iter = begin();
		const_iterator iter_end = end();
		++iter;

		if (iter == iter_end)
		{
			return;
		}

		const_iterator previous_iter = begin();

		int index = 1;
		int iter_index = 1;
		double current_meters = 0.0;
		double meters_per_entry = length_m() / static_cast<double>(_fast_access.size());
		
		std::cout << "\tlength = " << length_m() << std::endl;
		std::cout << "\tmeters_per_entry = " << meters_per_entry << std::endl;

		for (; iter != iter_end; ++iter, ++previous_iter, ++iter_index)
		{
			current_meters += length_m(previous_iter, iter);
			std::cout << "\t\tcurrent_meters = " << current_meters << std::endl;
			for (; current_meters > meters_per_entry * index; ++index)
			{
				_fast_access[index] = previous_iter;
				std::cout << "\t_fast_access[" << index << "] at meter "
					<< meters_per_entry * index << " = " << iter_index - 1 << std::endl;
			}
		}

		std::cout << std::endl;
	}
	
	
	
	std::vector<unsigned int>
	FilteredTrace::get_needed_tile_ids()
	{
		return _needed_tile_ids;
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
		Serializer::serialize(o_stream, _points_from_previous_start);
	}


	void
	FilteredTrace::set_needed_tile_ids(std::vector<unsigned int> needed_tile_ids)
	{
		_needed_tile_ids = needed_tile_ids;
	}

} // namespace mapgeneration
