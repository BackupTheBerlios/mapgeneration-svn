/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CONNECTION_H
#define CONNECTION_H

#include <cc++/socket.h>
#include <iostream>

#include "tracefilter.h"
#include "traceconnection.h"


namespace mapgeneration
{

	/**
	 * @brief TraceConnection is created in TraceServer to handle connections.
	 * 
	 * At the moment TraceConnection is not working as it should, so there
	 * is no complete documentation at the moment.
	 */
	class TraceConnection : public ost::TCPSession {
	
		public:
		
			TraceConnection(ost::TCPSocket &server, TraceFilter* trace_filter);
			
			~TraceConnection();
			
			void
			run();
		
		//	void run();
		
		private:
			
			std::string _input_string;
			
			TraceFilter* _trace_filter;			
			
	};

} // namespace mapgeneration

#endif //CONNECTION_H
