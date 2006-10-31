/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACECONNECTION_H
#define TRACECONNECTION_H

#include <cc++/socket.h>

/* Forward declarations... */
namespace mapgeneration
{
	
	class StringedTraceReceiver;
	
} // namespace mapgeneration
/* Forward declarations done! */


namespace mapgeneration
{
	
	/**
	 * @brief TraceConnection is created in TraceServer to handle connections.
	 * 
	 * At the moment TraceConnection is not working as it should, so there
	 * is no complete documentation at the moment.
	 */
	class TraceConnection : public ost::TCPSession
	{
	
		public:
		
			TraceConnection(ost::TCPSocket& server,
				StringedTraceReceiver* stringed_trace_receiver);
			
			
			void
			run();
			
			
		private:
			
			std::string _input_string;
			
			
			StringedTraceReceiver* _stringed_trace_receiver;			
			
	};

} // namespace mapgeneration

#endif //TRACECONNECTION_H
