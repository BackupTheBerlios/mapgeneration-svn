/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CONNECTION_H
#define CONNECTION_H

#include <cc++/socket.h>
#include <iostream.h>

#include "tilemanager.h"
#include "traceconnection.h"


namespace mapgeneration
{

	/**
	 * @brief TraceConnection is created in TraceServer to handle connections.
	 * 
	 * At the moment TraceConnection is not working as it should, so there
	 * is no complete documentation at the moment.
	 */
	class TraceConnection : public ost::SimpleTCPStream {
	
		public:
		
			TraceConnection(ost::TCPSocket &server);
			
			~TraceConnection();
			
			void
			receive();
			
			bool
			active();
			
			std::string
			input_string();
		
		//	void run();
		
		private:
		
			char* _buffer;
			int _buffer_size;
			
			bool _active;
			
			std::string _input_string;
			
	};

} // namespace mapgeneration

#endif //CONNECTION_H
