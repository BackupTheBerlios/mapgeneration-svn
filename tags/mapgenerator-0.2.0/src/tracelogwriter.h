/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACELOGWRITER_H
#define TRACELOGWRITER_H


#include <fstream>
#include <ostream>


namespace mapgeneration
{
	class TraceLogWriter;
}


#include "tilemanager.h"

#include "tracelog.h"


namespace mapgeneration
{

	/**
	 * Class TraceLogWriter
	 */	      
	class TraceLogWriter
	{

		public:

			/**
			 * Standard constructor.
			 */
			TraceLogWriter (TileManager* tile_manager, const std::string file_name,
				const FilteredTrace& filtered_trace);
			
			
			~TraceLogWriter();
			
			
			void
			add_tile(const Tile& tile, bool created = false);
			
			
			void
			changed_trace(const GPSPoint& new_start, const int removed_gps_points);
			

			void
			merge_node(const std::pair<unsigned int, unsigned int>& node_id, 
				const GPSPoint& gps_point, const Node& new_node);

			
			void
			new_node(const std::pair<unsigned int, unsigned int>& node_id, 
				const Node& node);
				
			
			void
			next_step();
			
			
		private:

			std::ofstream* _log_stream;


			TileCache* _tile_cache;
			
			
			void
			end_of_tracelog();
			
			
			void
			write_command(const std::string& command_string);
			
			
			void
			write_header(const FilteredTrace& filtered_trace);
			
	};


} // namespace mapgeneration

#endif //TRACELOGWRITER_H
