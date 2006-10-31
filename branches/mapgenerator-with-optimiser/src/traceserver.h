/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACESERVER_H
#define TRACESERVER_H

#include <cc++/socket.h>
#include <list>

#include "util/controlledthread.h"
#include "util/pubsub/servicesystem.h"

namespace mapgeneration
{
	
	class StringedTraceReceiver;
	

	/**
	 * @brief TraceServer accepts connections on a specific port and creates
	 * TraceConnections to handle them.
	 * 
	 * At the moment this class does not work as it should, so the documentation
	 * is also not complete and correct.....
	 * 
	 * @todo Insert TraceFilter!
	 */
	class TraceServer : public mapgeneration_util::ControlledThread
	{

		public:

			TraceServer (pubsub::ServiceList* service_list,
				StringedTraceReceiver* stringed_trace_receiver);
			
			
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
			
			
			std::list<ost::TCPSocket> _tcp_sockets;
			
			
			StringedTraceReceiver* _stringed_trace_receiver;

	};
	

} // namespace mapgeneration

#endif //TRACESERVER_H
