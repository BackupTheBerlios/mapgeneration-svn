/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceserver.h"

#include <cc++/network.h>

#include "stringedtracereceiver.h"
#include "traceconnection.h"
#include "util/mlog.h"

using namespace mapgeneration_util;

namespace mapgeneration
{


	TraceServer::TraceServer(pubsub::ServiceList* service_list, 
		StringedTraceReceiver* stringed_trace_receiver)
	: _service_list(service_list),
		_stringed_trace_receiver(stringed_trace_receiver)
	{
	}
	
	
	void
	TraceServer::thread_deinit()
	{
		mlog(MLog::info, "TraceServer") << "Shutting down... Stopped\n";
	}
	
	
	void
	TraceServer::thread_init()
	{
		mlog(MLog::info, "TraceServer") << "Initializing...\n";
		
		int port = 9000;
		if (!_service_list->get_service_value("traceserver.port", port))
		{
			mlog(MLog::info, "TraceServer")
				<< "Configuration for port number not found, using default (" 
				<< port << ").\n";
		}

		std::vector<ost::NetworkDeviceInfo> devices;
		enumNetworkDevices (devices);
		
		if (devices.size() == 0)
		{
			mlog(MLog::warning, "TraceServer") << 
				"No devices found, trying to bind to 127.0.0.1.\n";
			ost::TCPSocket new_tcp_socket(ost::InetAddress("127.0.0.1"), port);
			_tcp_sockets.push_back(new_tcp_socket);
		} else
		{
			std::vector<ost::NetworkDeviceInfo>::iterator iter = devices.begin();
			std::vector<ost::NetworkDeviceInfo>::iterator iter_end = devices.end();
			for(; iter != iter_end; ++iter)
			{
				mlog(MLog::debug, "TraceServer") << "Binding to interface "
					<< iter->name() << " address " << iter->address().getHostname()
					<< " port " << port << ".\n";
				ost::TCPSocket new_tcp_socket(iter->address(), port);
				_tcp_sockets.push_back(new_tcp_socket);
			}
		}
		
		mlog(MLog::info, "TraceServer") << "Initialized\n";		
	}


	void
	TraceServer::thread_run()
	{		
		mlog(MLog::info, "TraceServer") << "Waiting for connections...\n";
		while (!should_stop())
		{			
			std::list<ost::TCPSocket>::iterator tcp_socket_iter = 
				_tcp_sockets.begin();
			std::list<ost::TCPSocket>::iterator tcp_socket_iter_end = 
				_tcp_sockets.end();
			for (; tcp_socket_iter != tcp_socket_iter_end; ++tcp_socket_iter)
			{
				if (tcp_socket_iter->isPendingConnection(0))
				{
					mlog(MLog::debug, "TraceServer") 
						<< "Accepting new connection on address "
						<< tcp_socket_iter->getLocal().getHostname() << ".\n";
					TraceConnection* new_connection = new TraceConnection(
						*tcp_socket_iter, _stringed_trace_receiver);
					new_connection->start();
				}
			}
			
			_should_stop_event.wait(100);
		}
	}


} // namespace mapgeneration
