/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceserver.h"

#include <cc++/socket.h>
#include <cc++/network.h>

#include "util/mlog.h"

using namespace mapgeneration_util;

namespace mapgeneration
{


	TraceServer::TraceServer(pubsub::ServiceList* service_list, 
		TraceFilter* trace_filter)
	: _service_list(service_list), _trace_filter(trace_filter), 
		_trace_connections()
	{
	}
	
	
	TraceServer::~TraceServer()
	{
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
		
/*		ost::InetAddress address("127.0.0.1");
		ost::TCPSocket server(address, 9000);*/
		
		mlog(MLog::info, "TraceServer") << "Waiting for connections...\n";
		while (!should_stop())
		{
			std::list<TraceConnection>::iterator i = _trace_connections.begin();
			while (i != _trace_connections.end() && !should_stop())
			{
				(*i).receive();
				if (!i->active())
				{
					std::string input_string = (*i).input_string();
					/*mlog(MLog::debug, "TraceServer") << "Received:\n";
					mlog(MLog::debug, "TraceServer") << input_string << "\n";*/
					i = _trace_connections.erase(i);
					
					mlog(MLog::debug, "TraceServer") << "Parsing input...\n";
					FilteredTrace filtered_trace(_service_list);
					if (filtered_trace.parse_nmea_string(input_string))
					{
						mlog(MLog::debug, "TraceServer") << "" << filtered_trace.size() << " Points\n";
						mlog(MLog::debug, "TraceServer") << "Sending new trace to TileManager.\n";
						_trace_filter->new_trace(filtered_trace);
					}
				}
				else
					++i;
			}
			
			int timeout = (_trace_connections.size() ? 50 : 200);
			_should_stop_event.wait(timeout);
			
			std::list<ost::TCPSocket>::iterator tcp_socket_iter = 
				_tcp_sockets.begin();
			std::list<ost::TCPSocket>::iterator tcp_socket_iter_end = 
				_tcp_sockets.end();
			for (; tcp_socket_iter != tcp_socket_iter_end; ++tcp_socket_iter)
			{
				while (tcp_socket_iter->isPendingConnection(10) && !should_stop())
				{
					mlog(MLog::debug, "TraceServer") 
						<< "Accepting new connection on address "
						<< tcp_socket_iter->getLocal().getHostname() << ".\n";
					_trace_connections.push_back(*(
						new TraceConnection(*tcp_socket_iter)
					));
				}
			}
		}
	
		mlog(MLog::info, "TraceServer") << "Shutting down...\n";
		
		std::list<TraceConnection>::iterator i = _trace_connections.begin();
		while (i != _trace_connections.end())
		{
			
			
			++i;
		}

		mlog(MLog::info, "TraceServer") << "Stopped.\n";
	}


} // namespace mapgeneration
