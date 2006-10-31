/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FILTEREDTRACERECEIVER_H
#define FILTEREDTRACERECEIVER_H

namespace mapgeneration
{
	
	class FilteredTrace;
	
	
	class FilteredTraceReceiver
	{
		
		public:
			
			virtual
			~FilteredTraceReceiver(){};
			
			virtual void
			new_trace(FilteredTrace& filtered_trace) = 0;
			
	};

} // namespace mapgeneration

#endif //FILTEREDTRACERECEIVER_H
