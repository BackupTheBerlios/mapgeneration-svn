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
	
	TraceConnection::TraceConnection(ost::TCPSocket &server)
	: ost::SimpleTCPStream(server), _input_string(""), _active(true)
	{
		std::cout << "Creating TraceConnection!" << std::endl;
		_buffer_size = 1024;
		_buffer = new char[_buffer_size + 1];
	}


	TraceConnection::~TraceConnection()
	{
		delete [] _buffer;
	}
	
	
	void
	TraceConnection::receive()
	{
		bool active = false;
		int read_bytes=1;				
		while (read_bytes)
		{
			read_bytes = read(_buffer, _buffer_size, 0);
			_input_string.append(_buffer, read_bytes);
			if (read_bytes) active = true;
		}
		_active = active;
	}
	
	
	bool
	TraceConnection::active()
	{
		return _active;
	}


	std::string
	TraceConnection::input_string()
	{
		return _input_string;
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
