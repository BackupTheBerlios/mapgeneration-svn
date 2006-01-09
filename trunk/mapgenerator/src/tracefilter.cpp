/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tracefilter.h"

#include "util/mlog.h"

using namespace mapgeneration_util;

namespace mapgeneration
{
 

	TraceFilter::TraceFilter(pubsub::ServiceList* service_list,
		TileManager* tile_manager)
	: _service_list(service_list), _tile_manager(tile_manager),
		_queue_mutex()
	{
	}
	
	
	void
	TraceFilter::new_trace(std::string& nmea_string)
	{
		_queue_mutex.enterMutex();
		_queue.push(nmea_string);
		_queue_mutex.leaveMutex();
	}
	
	
	void
	TraceFilter::thread_init()
	{
		mlog(MLog::info, "TraceFilter") << "Initializing...\n";
		
		mlog(MLog::info, "TraceFilter") << "Initialized\n";		
	}


	void
	TraceFilter::thread_run()
	{		
		
		mlog(MLog::info, "TraceFilter") << "Waiting for FilteredTrace objects...\n";
		while (!should_stop())
		{				
			
			_queue_mutex.enterMutex();
			while(_queue.size() > 0)
			{
				FilteredTrace filtered_trace(_service_list);
				std::string queue_front = _queue.front();

				_queue_mutex.leaveMutex();
				if (filtered_trace.parse_nmea_string(queue_front))
				{
					_working_queue.push(filtered_trace);
					
					/*show_state("Inital state");*/
					
					int available_traces;
					int ready_traces;
	
					/* Test for time */
					available_traces = _working_queue.size();
					ready_traces = 0;
					while(ready_traces < available_traces)
					{
						apply_equal_time_filter(_working_queue.front());
						_working_queue.pop();
						++ready_traces;
						/*show_state("Applied time filter", ready_traces);*/
					}
					
					/* Test for location */
					available_traces = _working_queue.size();
					ready_traces = 0;
					while(ready_traces < available_traces)
					{
						apply_equal_location_filter(_working_queue.front());
						_working_queue.pop();
						++ready_traces;
						/*show_state("Applied location filter", ready_traces);*/
					}
	
					/* apply anti-cumulation filter */
					available_traces = _working_queue.size();
					ready_traces = 0;
					while(ready_traces < available_traces)
					{
						apply_anti_cumulation_filter(_working_queue.front());
						_working_queue.pop();
						++ready_traces;
						/*show_state("Applied anti-cumulation filter");*/
					}
					
					/* Test for gaps */
					available_traces = _working_queue.size();
					ready_traces = 0;
					while(ready_traces < available_traces)
					{
						apply_gap_filter(_working_queue.front());
						_working_queue.pop();
						++ready_traces;
						/*show_state("Applied speed filter", ready_traces);*/
					}
					
					/* Test for speed */
					available_traces = _working_queue.size();
					ready_traces = 0;
					while(ready_traces < available_traces)
					{
						apply_speed_filter(_working_queue.front());
						_working_queue.pop();
						++ready_traces;
						/*show_state("Applied speed filter", ready_traces);*/
					}
					
					/* Test for acceleration */
					available_traces = _working_queue.size();
					ready_traces = 0;
					while(ready_traces < available_traces)
					{
						apply_acceleration_filter(_working_queue.front());
						_working_queue.pop();
						++ready_traces;
						/*show_state("Applied acceleration filter", ready_traces);*/
					}
					
					/* Test for trace length and propagade it to the tile manager */
					int min_trace_length = 5;
					if (!_service_list->get_service_value("tracefilter.min_trace_length",
						min_trace_length))
					{
						mlog(MLog::info, "TraceFilter")
							<< "Configuration for min trace length not found,"
							<< " using default (" << min_trace_length << ").\n";
					}
	
					while(_working_queue.size() > 0)
					{
						FilteredTrace& trace = _working_queue.front();
						if (trace.size() < min_trace_length)
						{
//						mlog(MLog::debug, "TraceFilter")
//							<< "Trace too small. Discard it!\n";
						} else
						{
							_tile_manager->new_trace(trace);
						}
						
						_working_queue.pop();
						/*show_state("Propagation");*/
					}
				} else
				{
					mlog(MLog::warning, "TraceFilter")
							<< "Error parsing NMEA string!\n";
				}
				
				_queue_mutex.enterMutex();
				_queue.pop();
			}
			_queue_mutex.leaveMutex();
			// WARNING: The above enter leave combination is ok! Look at 
			// the beginning of the if/loop!

			_should_stop_event.wait(500);
		}
	}
		
	
	void
	TraceFilter::thread_deinit()
	{
		mlog(MLog::info, "TraceFilter") << "Shutting down...\n";

		mlog(MLog::info, "TraceFilter") << "Stopped.\n";
	}
	
	
	/**
	 * CAUTION:
	 * Two following points with the same timestamp are NOT allowed!
	 * Use apply_equal_time_filter before this filter to avoid flawed behaviour.
	 */
	void
	TraceFilter::apply_acceleration_filter(FilteredTrace& filtered_trace)
	{
		if (filtered_trace.size() < 3)
		{
			_working_queue.push(filtered_trace);
			return;
		}
		
		/* Now we know: the filtered_trace has a size of 3 at least! */
		
		/* Init the threshold */
		double acceleration_threshold = 20.0;
		if (!_service_list->get_service_value("tracefilter.max_acceleration",
			acceleration_threshold))
		{
			mlog(MLog::info, "TraceFilter")
				<< "Configuration for max. acceleration not found, using"
				<< " default ("	<< acceleration_threshold << ").\n";
		}
		
		/* Init the iterator for point testing */
		FilteredTrace::iterator first_test_point_iter
			= filtered_trace.begin();
		FilteredTrace::iterator second_test_point_iter
			= ++(filtered_trace.begin());
		FilteredTrace::iterator third_test_point_iter
			= ++(++(filtered_trace.begin()));
		
		int counter = 1;
		
		/* Loop over the trace */
		do
		{
			double distance_1_to_2 = second_test_point_iter
				->distance_default(*first_test_point_iter);
			double distance_2_to_3 = third_test_point_iter
				->distance_default(*second_test_point_iter);
				
			double time_1_to_2 = second_test_point_iter->get_time()
				- first_test_point_iter->get_time();
			double time_2_to_3 = third_test_point_iter->get_time()
				- second_test_point_iter->get_time();
				
			double speed_1_to_2 = distance_1_to_2 / time_1_to_2;
			double speed_2_to_3 = distance_2_to_3 / time_2_to_3;
			
			/* a first approximation */
			/*double acceleration = (speed_2_to_3 - speed_1_to_2) / time_2_to_3;*/
			
			/* a better approximation ?! */
			double acceleration = (speed_2_to_3 - speed_1_to_2)
				/ ((time_2_to_3  + time_1_to_2) / 2);
			
			if ((acceleration > acceleration_threshold)
				|| (acceleration < -acceleration_threshold))
			{
				/* Threshold exceeded */
				mlog(MLog::debug, "TraceFilter")
					<< "Acceleration threshold exceeded (|" << acceleration
					<< "| > " << acceleration_threshold << ") at point "
					<< counter << ".\n";
				
				/* Cut off first part of the trace */
				FilteredTrace cutoff_part(_service_list);
				cutoff_part.splice(cutoff_part.begin(), filtered_trace,
					filtered_trace.begin(), second_test_point_iter);
				_working_queue.push(cutoff_part);
				
				/* Increment iters */
				first_test_point_iter = second_test_point_iter;
				++second_test_point_iter;
				++third_test_point_iter;

				/*show_state("Applying acceleration filter");*/

			} else
			{
				++first_test_point_iter;
				++second_test_point_iter;
				++third_test_point_iter;
			}
			
			++counter;
			
		} while(third_test_point_iter != filtered_trace.end());
		
		_working_queue.push(filtered_trace);
	}
	
	
	void
	TraceFilter::apply_anti_cumulation_filter(FilteredTrace& filtered_trace)
	{
		FilteredTrace::iterator iter = filtered_trace.begin();
		
		while(iter != filtered_trace.end())
		/* ++iter is done inside loop by ++iter and erase! */
		{
			GPSPoint old_point = *iter;			
			FilteredTrace::iterator point_1 = iter;
			double merged_points = 1;
			++iter;
			
			double new_latitude = point_1->get_latitude();
			double new_longitude = point_1->get_longitude();
			double new_altitude = point_1->get_altitude();
			double new_time = point_1->get_time();
			
			/* while the  distance  between  points  is less 4.0 
			 * they are  all  merged into 1 point */
			while((iter != filtered_trace.end()) && (old_point.distance_default(*iter) < 4.0))
			{
				new_latitude += iter->get_latitude();
				new_longitude += iter->get_longitude();
				new_altitude += iter->get_altitude();
				new_time += iter->get_time();				
				merged_points += 1;
				
				iter = filtered_trace.erase(iter);
			}
			
			/*calculates  the new  merged time, longitude, latitude  and altitude */
			new_latitude /= merged_points;
			new_longitude /= merged_points;
			new_altitude /= merged_points;
			new_time /= merged_points;
			
			/* setting the attributes */
			point_1->set(new_latitude, new_longitude, new_altitude);
			point_1->set_time(new_time);
		}
		
		_working_queue.push(filtered_trace);
	}


	void
	TraceFilter::apply_equal_location_filter(FilteredTrace& filtered_trace)
	{
		if (filtered_trace.size() < 2)
		{
			_working_queue.push(filtered_trace);
			return;
		}
		
		/* Now we know: the filtered_trace has a size of 2 at least! */
		
		/* We test, if two points have the same coordinate. If true and
		 * the invalid flag is set, the point is deleted. */

		/* Init the iterator for point testing */
		FilteredTrace::iterator first_test_point_iter
			= filtered_trace.begin();
		FilteredTrace::iterator second_test_point_iter
			= ++(filtered_trace.begin());
			
		int counter = 1;
		
		/* Loop over the trace */
		do
		{
			if (second_test_point_iter->get_invalid()
				&& (*first_test_point_iter == *second_test_point_iter))
			{
				/* invalid flag is set and locations equal */
//				mlog(MLog::debug, "TraceFilter") << "Locations are equal "
//					<< "around point " << counter << ". Invalid flag is set.\n";
				
				/* Save iterator */
				FilteredTrace::iterator delete_iter = second_test_point_iter;
				++second_test_point_iter;
				
				/* Delete iterator */
				filtered_trace.erase(delete_iter);
				
				/*show_state("Applying location filter");*/

			} else
			{
				++first_test_point_iter;
				++second_test_point_iter;
			}
			
			++counter;
			
		} while(second_test_point_iter != filtered_trace.end());

		_working_queue.push(filtered_trace);
	}
	
	
	void
	TraceFilter::apply_equal_time_filter(FilteredTrace& filtered_trace)
	{
		if (filtered_trace.size() < 2)
		{
			_working_queue.push(filtered_trace);
			return;
		}

		/* Now we know: the filtered_trace has a size of 2 at least! */
			
		/* Init the iterator for point testing */
		FilteredTrace::iterator first_test_point_iter
			= filtered_trace.begin();
		FilteredTrace::iterator second_test_point_iter
			= ++(filtered_trace.begin());
			
		int counter = 1;
		
		do
		{
			if (first_test_point_iter->get_time()
				> second_test_point_iter->get_time())
			{
				/* Times invalid */
				mlog(MLog::debug, "TraceFilter") << "Timestamps are invalid "
				<< "around point " << counter << ".\n";
				
				/* Cut off first part of the trace */
				FilteredTrace cutoff_part(_service_list);
				cutoff_part.splice(cutoff_part.begin(), filtered_trace,
					filtered_trace.begin(), second_test_point_iter);
				_working_queue.push(cutoff_part);
				
				/* Increment iters */
				first_test_point_iter = second_test_point_iter;
				++second_test_point_iter;
				
				/*show_state("Applying time filter");*/

			} else if (first_test_point_iter->get_time()
				== second_test_point_iter->get_time())
			{
				/* Times equal */
				mlog(MLog::debug, "TraceFilter") << "Timestamps are equal "
				<< "around point " << counter << ".\n";
				
				/* Save iterator */
				FilteredTrace::iterator delete_iter = second_test_point_iter;
				++second_test_point_iter;
				
				/* Delete iterator */
				filtered_trace.erase(delete_iter);
				
				/*show_state("Applying time filter");*/

			} else
			{
				++first_test_point_iter;
				++second_test_point_iter;
			}
			
			++counter;
			
		} while (second_test_point_iter != filtered_trace.end());
		
		_working_queue.push(filtered_trace);	
	}
	
	
	void
	TraceFilter::apply_gap_filter(FilteredTrace& filtered_trace)
	{
		if (filtered_trace.size() < 2)
		{
			_working_queue.push(filtered_trace);
			return;
		}
		
		/* Now we know: the filtered_trace has a size of 2 at least! */
		
		/* Init the thresholds... */
		double longest_tunnel = 50000.0;
		if (!_service_list->get_service_value("tracefilter.longest_tunnel",
			longest_tunnel))
		{
			mlog(MLog::info, "TraceFilter")
				<< "Configuration for longest tunnel not found, using"
				<< " default ("	<< longest_tunnel << "m).\n";
		}
		
		double max_distance_gap = 15000.0;
		if (!_service_list->get_service_value("tracefilter.max_distance_gap",
			max_distance_gap))
		{
			mlog(MLog::info, "TraceFilter")
				<< "Configuration for max_distance_gap not found, using"
				<< " default ("	<< max_distance_gap << "m).\n";
		}
		
		if (max_distance_gap > longest_tunnel)
		{
			mlog(MLog::info, "TraceFilter")
				<< "A max_distance_gap greater than longest_tunnel does not make "
				<< "sense! Setting max_distance_gap to longest_tunnel.\n";
			max_distance_gap = longest_tunnel;
		}
		
		double max_time_gap = 3600.0;
		if (!_service_list->get_service_value("tracefilter.max_time_gap",
			max_time_gap))
		{
			mlog(MLog::info, "TraceFilter")
				<< "Configuration for max_time_gap not found, using"
				<< " default ("	<< max_time_gap << "s).\n";
		}
		/* done. */
		
		/* Init the iterator for point testing */
		FilteredTrace::iterator first_test_point_iter
			= filtered_trace.begin();
		FilteredTrace::iterator second_test_point_iter
			= ++(filtered_trace.begin());
			
		int counter = 1;
		
		/* Loop over the trace */
		do
		{
			double distance = second_test_point_iter
				->distance_default(*first_test_point_iter);
			double time = second_test_point_iter->get_time()
				- first_test_point_iter->get_time();
			
			if (distance > max_distance_gap)
			{
				/* Threshold exceeded */
				mlog(MLog::debug, "TraceFilter")
					<< "max_distance_gap exceeded (" << distance
					<< " > " << max_distance_gap << ") at point "
					<< counter << ".\n";
				
				/* Cut off first part of the trace */
				FilteredTrace cutoff_part(_service_list);
				cutoff_part.splice(cutoff_part.begin(), filtered_trace,
					filtered_trace.begin(), second_test_point_iter);
				_working_queue.push(cutoff_part);

				/* Increment iters */
				first_test_point_iter = second_test_point_iter;
				++second_test_point_iter;

				/*show_state("Applying speed filter");*/

			} else if (time > max_time_gap)
			{
				/* Threshold exceeded */
				mlog(MLog::debug, "TraceFilter")
					<< "max_time_gap exceeded (" << time
					<< " > " << max_time_gap << ") at point "
					<< counter << ".\n";
				
				/* Cut off first part of the trace */
				FilteredTrace cutoff_part(_service_list);
				cutoff_part.splice(cutoff_part.begin(), filtered_trace,
					filtered_trace.begin(), second_test_point_iter);
				_working_queue.push(cutoff_part);

				/* Increment iters */
				first_test_point_iter = second_test_point_iter;
				++second_test_point_iter;

				/*show_state("Applying speed filter");*/

			} else
			{
				++first_test_point_iter;
				++second_test_point_iter;
			}
			
			++counter;
			
		} while(second_test_point_iter != filtered_trace.end());

		_working_queue.push(filtered_trace);
	}
	
	
	/**
	 * CAUTION:
	 * Two following points with the same timestamp are NOT allowed!
	 * Use apply_equal_time_filter before this filter to avoid flawed behaviour.
	 */
	void
	TraceFilter::apply_speed_filter(FilteredTrace& filtered_trace)
	{
		if (filtered_trace.size() < 2)
		{
			_working_queue.push(filtered_trace);
			return;
		}
		
		/* Now we know: the filtered_trace has a size of 2 at least! */
		
		/* Init the threshold */
		double speed_threshold = 70.0;
		if (!_service_list->get_service_value("tracefilter.max_speed",
			speed_threshold))
		{
			mlog(MLog::info, "TraceFilter")
				<< "Configuration for max. speed not found, using"
				<< " default ("	<< speed_threshold << ").\n";
		}
		
		/* Init the iterator for point testing */
		FilteredTrace::iterator first_test_point_iter
			= filtered_trace.begin();
		FilteredTrace::iterator second_test_point_iter
			= ++(filtered_trace.begin());
			
		int counter = 1;
		
		/* Loop over the trace */
		do
		{
			double distance = second_test_point_iter
				->distance_default(*first_test_point_iter);
			double time = second_test_point_iter->get_time()
				- first_test_point_iter->get_time();
			
			double speed = distance / time;
			
			if (speed > speed_threshold || speed < 0.0)
			{
				/* Threshold exceeded */
				mlog(MLog::debug, "TraceFilter")
					<< "Speed threshold exceeded or below 0 (" << speed
					<< " > " << speed_threshold << ") at point "
					<< counter << ".\n";
				
				/* Cut off first part of the trace */
				FilteredTrace cutoff_part(_service_list);
				cutoff_part.splice(cutoff_part.begin(), filtered_trace,
					filtered_trace.begin(), second_test_point_iter);
				_working_queue.push(cutoff_part);

				/* Increment iters */
				first_test_point_iter = second_test_point_iter;
				++second_test_point_iter;

				/*show_state("Applying speed filter");*/

			} else
			{
				++first_test_point_iter;
				++second_test_point_iter;
			}
			
			++counter;
			
		} while(second_test_point_iter != filtered_trace.end());

		_working_queue.push(filtered_trace);
	}


/*	void
	TraceFilter::show_state(std::string step_name, int number)
	{
		std::queue<FilteredTrace> queue(_working_queue);
		std::vector<FilteredTrace> vec;
		
		int size = queue.size();
		for (int i = 0; i < size; ++i)
		{
			vec.push_back(queue.front());
			queue.pop();
		}
		
		std::cout << std::endl;
		std::cout << "--------------------------" << std::endl;
		std::cout << "|Status of _working_queue:" << std::endl;
		std::cout << "|\t(" << step_name;
		
		if (number > -1)
			std::cout << ": " << number;
		std::cout << ")" << std::endl;

		std::cout << "|=========================" << std::endl;
		std::cout << "|Size = " << vec.size() << std::endl;
		
		for (int i = 0; i < vec.size(); ++i)
		{
			std::cout << "|\tTrace " << i << ": Length = ";
			std::cout << vec[i].size() << std::endl;
		}

		std::cout << "-------------------------" << std::endl;
		std::cout << std::endl;
	}*/

} // namespace mapgeneration
