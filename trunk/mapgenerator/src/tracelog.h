/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACELOG_H
#define TRACELOG_H

#include <iostream>
#include <map>
#include <sstream>

#include "tilecache.h"
#include "util/serializer.h"


namespace mapgeneration
{

	/**
	 * Class TraceLog
	 */
	class TraceLog
	{

		public:
		
			static const int _NEXT_TRACE_NODE = 1;
			static const int _CHANGED_TRACE = 2;
			static const int _END_OF_TRACELOG = 9;
			static const int _NEW_NODE = 10;
			static const int _MERGE_NODE = 12;
			

			/**
			 * Standard constructor.
			 */
			TraceLog();
			
			
			void
			do_current_command();
			
			
			inline int
			get_current_position();
			
			
			inline int
			get_number_of_commands();
			
			
			bool
			load(std::istream& i_stream);
			
		
			int
			step_backward(const unsigned int steps);
	

			int
			step_forward(const unsigned int steps);
			
			
			inline TileCache*
			tile_cache();


		private:
		
			std::vector<std::stringstream::pos_type> _command_positions;
			
			std::stringstream _command_stream;
			
			std::string _command_string;
			
			int _current_position;
			
			TileCache _tile_cache;

	};
	
	
	inline int
	TraceLog::get_current_position()
	{
		return _current_position;
	}
	
	
	inline int
	TraceLog::get_number_of_commands()
	{
		return _command_positions.size();
	}


	inline TileCache*
	TraceLog::tile_cache()
	{
		return &_tile_cache;
	}
	


} // namespace mapgeneration

#endif //TRACELOG_H
