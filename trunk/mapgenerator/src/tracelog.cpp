/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tracelog.h"

#include "util/mlog.h"


using namespace mapgeneration_util;


namespace mapgeneration
{

	TraceLog::TraceLog()	
	: _command_positions(),
		_command_stream(std::stringstream::in | std::stringstream::out |
			std::stringstream::binary),
		_current_position(-1),
		_tile_cache(0, 0, TileCache::_FIFO,
			TileCache::_NON_PERSISTENT | TileCache::_NO_MEMORY_LIMIT)
	{
	}
	
	
	void
	TraceLog::do_current_command()
	{
		/** @todo This code s**ks. Find another workaround for the problems
		 * with seekg! */
		std::stringstream _command_stream;
		_command_stream.str(_command_string);
		_command_stream.seekg(0);
		
		std::cout << "I am at " << _command_stream.tellg() << "\n";
		std::cout << _command_stream.str();
		_command_stream.seekg(-_command_stream.tellg());
		_command_stream.ignore(_command_positions[_current_position]);
//		_command_stream.seekg(_command_positions[_current_position]);
		
		std::cout << "Tried to seek " << _command_positions[_current_position]
			<< " arrived at " << _command_stream.tellg() << "\n";
		
		int command = Serializer::deserialize<int>(_command_stream);
		
		std::cout << "Found command " << command << "\n";
		
		switch (command)
		{
			case _NEXT_TRACE_NODE:
				std::cout << "next_trace_node\n";
			break;
			case _CHANGED_TRACE:
				std::cout << "changed_trace\n";
			break;
			case _END_OF_TRACELOG:
				std::cout << "end_of_tracelog\n";
			break;
			case _NEW_NODE:
				std::cout << "new_node\n";
			break;
			case _MERGE_NODE:
				std::cout << "merge_node\n";
			break;
			default:
				std::cout << "unknown command\n";
			break;
		}
	}
	
	
	bool
	TraceLog::load(std::istream& i_stream)
	{
		// *** Header ***
		
		int version_high = Serializer::deserialize<int>(i_stream);
		int version_low = Serializer::deserialize<int>(i_stream);
		
		
		
		int tile_number = Serializer::deserialize<int>(i_stream);
		mlog(MLog::debug, "TraceLog::load") << "Reading "
			<< tile_number << " tiles.\n";
			
		for (int i = 0; i<tile_number; ++i)
		{
			Tile* tile = new Tile(Serializer::deserialize<Tile>(i_stream));
			_tile_cache.insert(tile->get_id(), tile);
		}		
		
		// *** Commands	***
		// Read data
		char buffer[4096];
		std::streamsize read_bytes;
		_command_string = "";
		
		while(read_bytes = i_stream.readsome(buffer, 4096))
			_command_string.append(buffer, read_bytes);
			
		_command_stream.str(_command_string);
		_command_stream.seekg(0);
		
		// Parse for command positions
		while(_command_stream.peek() != EOF)
		{
			int command_length = Serializer::deserialize<int>(_command_stream);
			_command_positions.push_back((int)_command_stream.tellg());
			_command_stream.ignore(command_length);
		}
		
		_command_stream.flush();
		
		mlog(MLog::debug, "TraceLog::load") << "Read "
			<< _command_positions.size() << " commands.\n";
		
		if (_command_positions.size())		
			_current_position = 0;
		else
			_current_position = -1;
	}
	
	
	int
	TraceLog::step_backward(const unsigned int steps)
	{
		
		return get_current_position();
	}
	
	
	int
	TraceLog::step_forward(const unsigned int steps)
	{
		int i=0;
		if (i<steps && _current_position<get_number_of_commands())
		{
			do_current_command();
			
			++i; ++_current_position;
		}
	
		return get_current_position();
	}
	

} // namespace mapgeneration
