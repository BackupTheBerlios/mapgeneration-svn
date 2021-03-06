/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tracelogwriter.h"
#include "util/serializer.h"

namespace mapgeneration
{

	TraceLogWriter::TraceLogWriter(TileManager* tile_manager, 
		const std::string file_name, const FilteredTrace& filtered_trace,
		pubsub::ServiceList* service_list)
	: _log_stream(0), _write_to_file(false)
	{
		service_list->get_service_value("tracelogwriter.write_to_file",
			_write_to_file);
		
		_tile_cache = tile_manager->get_tile_cache();
		
		if (_write_to_file)
		{
			_log_stream = new std::ofstream(file_name.c_str(), 
				std::ios::binary | std::ios::trunc);
			
			if (!(*_log_stream))
			{
				_log_stream = 0;
				return;
			}
			
			write_header(filtered_trace);
		}
	}
	
	
	TraceLogWriter::~TraceLogWriter()
	{
		end_of_tracelog();
		
		if (_write_to_file)
		{
			_log_stream->close();
			delete _log_stream;
		}
	}
	
	
	void
	TraceLogWriter::next_step()
	{
	}
	
	
	void
	TraceLogWriter::changed_trace(const GPSPoint& new_start, 
		const int removed_gps_points)
	{
		std::stringstream command_stream;
		Serializer::serialize(command_stream, TraceLog::_CHANGED_TRACE);
		Serializer::serialize(command_stream, new_start);
		Serializer::serialize(command_stream, removed_gps_points);
		write_command(command_stream.str());
	}	
	
	
	void
	TraceLogWriter::new_node(const std::pair<unsigned int, unsigned int>& node_id, 
		const Node& node)
	{
		std::stringstream command_stream;
		Serializer::serialize(command_stream, TraceLog::_NEW_NODE);
		Serializer::serialize(command_stream, node_id);
		Serializer::serialize(command_stream, node);
		write_command(command_stream.str());
	}
	
	
	void
	TraceLogWriter::merge_node(const std::pair<unsigned int, unsigned int>& node_id, 
		const GPSPoint& gps_point, const Node& new_node)
	{
		std::stringstream command_stream;
		Serializer::serialize(command_stream, TraceLog::_MERGE_NODE);
		Serializer::serialize(command_stream, node_id);
		Serializer::serialize(command_stream, gps_point);
		Serializer::serialize(command_stream, new_node);
		write_command(command_stream.str());
	}
	
	
/*	void
	TraceLogWriter::connect_edges(
		const unsigned int  edge_id_1, 
		const unsigned int edge_id_2,  const Edge& new_edge)
	{
		std::stringstream command_stream;
		Serializer::serialize(command_stream, TraceLog::_CONNECT_EDGES);
		Serializer::serialize(command_stream, edge_id_1);
		Serializer::serialize(command_stream, edge_id_2);
		Serializer::serialize(command_stream, new_edge);
		write_command(command_stream.str());
	}*/



	void
	TraceLogWriter::end_of_tracelog()
	{
		std::stringstream command_stream;
		Serializer::serialize(command_stream, TraceLog::_END_OF_TRACELOG);
		write_command(command_stream.str());
	}
	
	
	void
	TraceLogWriter::write_command(const std::string& command_string)
	{
		if (_write_to_file)
		{
			Serializer::serialize(*_log_stream, (int)command_string.size());
			(*_log_stream)<<command_string;
		}
	}
	
	
	void
	TraceLogWriter::write_header(const FilteredTrace& filtered_trace)
	{
		if (_write_to_file)
		{
			// Version
			Serializer::serialize(*_log_stream, (int)1);
			Serializer::serialize(*_log_stream, (int)0);
			
			// Tiles
			std::set<unsigned int> needed_edges;
	
			//mlog(MLog::debug, "TraceLogWriter::write_header") << "Writing "
				//<< filtered_trace.needed_tile_ids().size() << " tiles.\n";
			Serializer::serialize(*_log_stream, (int)filtered_trace.needed_tile_ids().size());
			std::vector<unsigned int>::const_iterator tile_ids_iter = 
				filtered_trace.needed_tile_ids().begin();
			std::vector<unsigned int>::const_iterator tile_ids_iter_end = 
				filtered_trace.needed_tile_ids().end();
			for (; tile_ids_iter != tile_ids_iter_end; ++tile_ids_iter)
			{
				TileCache::Pointer tile_pointer = _tile_cache->get(*tile_ids_iter);
				if (tile_pointer != 0)
				{
					Serializer::serialize(*_log_stream, *tile_pointer);
				} else
				{
					/** @todo what to do? Insert dummy 0-tile, create the tile and insert it
					 * then */
					/* I will realize the second version, just to get a working programm
					 * again. (this is an old docu!) */
//					_tile_cache->insert(*tile_ids_iter, new Tile(*tile_ids_iter));
//					tile_pointer = _tile_cache->get(*tile_ids_iter);
					throw(std::string("Arg"));
				}
			}
					
//			changed_trace(filtered_trace);
		}
	}


} // namespace mapgeneration
