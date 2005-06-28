/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceconnection.h"

#include <cc++/socket.h>
#include <iostream>
#include <string>

#include "tilemanager.h"
#include "util/mlog.h"


namespace mapgeneration
{
	
	TraceConnection::TraceConnection(ost::TCPSocket &server, 
		TraceFilter* trace_filter)
	: ost::TCPSession(server), _input_string(""), _trace_filter(trace_filter)
	{
		std::cout << "Creating TraceConnection!" << std::endl;
	}


	TraceConnection::~TraceConnection()
	{
	}
	
	
	void
	TraceConnection::run()
	{
		setCompletion(true);

		int buffer_size = 1024;
		char* buffer = new char[buffer_size + 1];
		
		bool active = true;
		
		while (isPending(ost::Socket::pendingInput) && active)
		{
			int read_bytes = readData(buffer, buffer_size, 0);
			if (read_bytes > 0)
				_input_string.append(buffer, read_bytes);
			else
				active = false;
		}
		
		delete [] buffer;
		
		mlog(MLog::debug, "TraceConnection") << 
			"Sending input to TraceFilter\n";
		_trace_filter->new_trace(_input_string);
	}


/*void TraceConnection::run()
{
	std::cout << "TraceConnection accepted and waiting for data..." << std::endl;	
	std::string input;
	
	int buffer_size = getBufferSize();
	char* buffer = new char[buffer_size];
	mlog(MLog::debug, "TraceConnection") << "Using " << buffer_size << "Byte as buffer.\n";
		
	while (peek(buffer, 1))
	{
		try {
			
			*tcp() >> input_buffer;
			input += input_buffer;
		} catch (...)
		{
			std::cout << "Connection problem in TraceConnection!" << std::endl;
		}

		sleep(10);
	}	
	delete [] buffer;
	std::cout << "Received input: " << input << std::endl;
	std::cout << "Connection closed!" << std::endl;
}*/

} // namespace mapgeneration
