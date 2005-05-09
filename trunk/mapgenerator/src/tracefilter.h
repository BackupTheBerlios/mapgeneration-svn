/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACEFILTER_H
#define TRACEFILTER_H

#include <cc++/thread.h>
#include <queue>
#include <list>

#include "gpspoint.h"
#include "tilemanager.h"
#include "util/pubsub/servicesystem.h"

namespace mapgeneration
{
	
	class TraceFilter : public mapgeneration_util::ControlledThread {
		
		public:
			
			TraceFilter(pubsub::ServiceList* service_list, TileManager* tile_manager);
			
						
			void
			new_trace(FilteredTrace& filtered_trace);
			
				
		protected:
			
			void
			thread_deinit();
			
			
			void
			thread_init();
			
			
			void
			thread_run();
			
			
		private:
			
			pubsub::ServiceList* _service_list;
			
			
			TileManager* _tile_manager;
			
			
			std::queue<FilteredTrace> _queue;
			
			
			std::queue<FilteredTrace> _working_queue;
			
			
			void
			apply_acceleration_filter(FilteredTrace& filtered_trace);
			
			
			void
			apply_anti_cumulation_filter(FilteredTrace& filtered_trace);
			
			
			void
			apply_equal_location_filter(FilteredTrace& filtered_trace);
			
			
			void
			apply_equal_time_filter(FilteredTrace& filtered_trace);
			
			
			void
			apply_gap_filter(FilteredTrace& filtered_trace);
			
			
			void
			apply_speed_filter(FilteredTrace& filtered_trace);
			
			
			/*void
			show_state(std::string step_name, int number = -1);*/
			
	};
	
} // namespace mapgeneration

#endif //TRACEFILTER_H


