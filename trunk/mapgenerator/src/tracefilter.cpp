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
 

	TraceFilter::TraceFilter(pubsub::ServiceList* service_list,TileManager* tile_manager)
	: _service_list(service_list), _tile_manager(tile_manager)
	{
	}
	
	
	TraceFilter::TraceFilter(pubsub::ServiceList* service_list)
	: _service_list(service_list)
	{
	}
	
	
	TraceFilter::TraceFilter()
	{
	}
	
	
	void
	TraceFilter::add_trace_in_queue(FilteredTrace& filtered_trace)
	{
		_trace_filter_queue.push(filtered_trace);
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

			while(_trace_filter_queue.size()>0)
			{
				_tile_manager->new_trace(_trace_filter_queue.front());
				_trace_filter_queue.pop();
			}

			_should_stop_event.wait(500);
		}
	}
		
	
	void
	TraceFilter::thread_deinit()
	{
		mlog(MLog::info, "TraceFilter") << "Shutting down...\n";

		mlog(MLog::info, "TraceFilter") << "Stopped.\n";
	}

} // namespace mapgeneration
