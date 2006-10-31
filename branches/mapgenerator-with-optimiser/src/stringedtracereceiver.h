/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef STRINGEDTRACERECEIVER_H
#define STRINGEDTRACERECEIVER_H

namespace mapgeneration
{
	
	class StringedTraceReceiver
	{
		
		public:
			
			virtual
			~StringedTraceReceiver(){};
			
			virtual void
			new_trace(std::string& filtered_trace) = 0;
			
	};

} // namespace mapgeneration

#endif //STRINGEDTRACERECEIVER_H
