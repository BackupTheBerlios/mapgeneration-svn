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
#include "util/pubsub/servicelist.h"

namespace mapgeneration
{

	/**
	 * @brief TraceServer accepts connections on a specific port and creates
	 * TraceConnections to handle them.
	 * 
	 * At the moment this class does not work as it should, so the documentation
	 * is also not complete and correct.....
	 */
	class TraceFilter : public mapgeneration_util::ControlledThread {

		public:

			TraceFilter(pubsub::ServiceList* service_list, TileManager* tile_manager);
			
						
			TraceFilter(pubsub::ServiceList* service_list);
						
			
			TraceFilter();
			
			/**
			 * Method to add trace into the _trace_filter_queue
			 */
			void
			add_trace_in_queue(FilteredTrace& filtered_trace);

				
		protected:
		
			void
			thread_deinit();
			
		
			void
			thread_init();
	
			/**
			 * "The thread itself" and also the main method of the TraceServer.
			 */
			void
			thread_run();
		
		private:
		
			pubsub::ServiceList* _service_list;
		
		
			TileManager* _tile_manager;


			std::queue<FilteredTrace> _trace_filter_queue;
						
	};
	

} // namespace mapgeneration

#endif //TRACEFILTER_H


