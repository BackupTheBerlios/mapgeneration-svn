/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceprocessor.h"

namespace mapgeneration
{
	#warning serialize Tiles! Otherwise consistency cannot be garanteed!
	
	TraceProcessor::Protocol::Protocol(bool auto_delete)
	: _auto_delete(auto_delete), _trace(), _path(), _used_tiles(),
		_used_tile_ids()
	{
	}
	
	
	TraceProcessor::Protocol::~Protocol()
	{
		if (_auto_delete)
			delete_all_path_entries();
		
		for (int i = 0; i < _used_tiles.size(); ++i)
			delete _used_tiles[i];
	}
	
	
	void
	TraceProcessor::Protocol::delete_all_path_entries()
	{
		int path_size = _path.size();
		if (path_size == 0)
		{
			return;
		} else if (path_size == 1)
		{
			delete _path.begin()->second;
			return;
		}
		
		// Path has a size of at most 2.
		// PathEntries may occur twice BUT only directly following!
		D_Path::iterator previous_iter = _path.begin();
		D_Path::iterator current_iter = _path.begin();
		++current_iter;
		
		D_Path::iterator iter_end = _path.end();
		while (current_iter != iter_end)
		{
			if (previous_iter->second == current_iter->second)
				++current_iter;
			
			delete previous_iter->second;
			
			previous_iter = current_iter;
			++current_iter;
		}
	}
	
	
	void
	TraceProcessor::Protocol::deserialize(std::istream& i_stream)
	{
		// deserialize the tiles
		if (true)
		{
			for (int i = 0; i < _used_tiles.size(); ++i)
				delete _used_tiles[i];
			
			_used_tiles.clear();
			
			uint64_t used_tiles_size;
			Serializer::deserialize(i_stream, used_tiles_size);
			for (uint64_t i = 0; i < used_tiles_size; ++i)
			{
				Tile* tile = new Tile;
				Serializer::deserialize(i_stream, *tile);
				
				_used_tiles.push_back(tile);
				_used_tile_ids.insert(tile->get_id());
			}
			
			assert(_used_tiles.size() == used_tiles_size);
			assert(_used_tile_ids.size() == used_tiles_size);
		}
		
		// deserialize the trace
		if (true)
		{
			_trace.clear();
			Serializer::deserialize(i_stream, _trace);
			_trace.precompute_data();
		}
		
		// deserialize the path's hard copy
		if (true)
		{
			D_PathHardCopy path_hard_copy;
			Serializer::deserialize(i_stream, path_hard_copy);
			
			// now make a path of it
			// AND
			// build up a fast access vector for looking up the connection pointers
			_path.clear();
			std::vector<PathEntry*> connection_pointers;
			
			int assertion_index = 0; // for assertion only!
			
			D_PathHardCopy::iterator path_hard_copy_iter = path_hard_copy.begin();
			D_PathHardCopy::iterator path_hard_copy_iter_end = path_hard_copy.end();
			PathEntry* previous_entry = 0;
			while (path_hard_copy_iter != path_hard_copy_iter_end)
			{
				PathEntry* entry = new PathEntry(path_hard_copy_iter->second);
				
				if ( (previous_entry != 0) && /*(*previous_entry == *entry)*/(previous_entry->_serializer_id == entry->_serializer_id) )
				{
					_path.insert(std::make_pair(previous_entry->_scan_position,
						previous_entry));
				} else
				{
					_path.insert(std::make_pair(entry->_scan_position, entry));
					connection_pointers.push_back(entry);
					
					assert(connection_pointers[assertion_index] == entry);
					assert(assertion_index == entry->_serializer_id);
					++assertion_index;
				}
				
				previous_entry = entry;
				++path_hard_copy_iter;
			}
			
			
			// what is missing are the connection pointers.
			// set them now.
			D_Path::iterator path_iter = _path.begin();
			D_Path::iterator path_iter_end = _path.end();
			
//			int index = 0;
			
			for (; path_iter != path_iter_end; ++path_iter)
			{
				if (path_iter->second->_serializer_connection_id > -1)
				{
					path_iter->second->_connection = connection_pointers[
						path_iter->second->_serializer_connection_id];
				}
			}
		}
		
		// Now set the _backward_connection.
		if (true)
		{
			D_Path::iterator iter = _path.begin();
			D_Path::iterator iter_end = _path.end();
			while ( (iter != iter_end) && (!iter->second->_is_beginning) )
				++iter;
			
			if (iter != iter_end)
			{
				PathEntry* previous_entry = iter->second;
				PathEntry* entry = previous_entry->_connection;
				while (entry != 0)
				{
					entry->_backward_connection = previous_entry;
					
					entry = entry->_connection;
				}
			}
		}
	}
	
	
	void
	TraceProcessor::Protocol::serialize (std::ostream& o_stream) const
	{
		// it is possible the write a method without using mutable
		// serializer_ids. perhaps later... :-)
		
//		std::cout << _path << std::endl;
//		std::cout << "Protocol::serialize: _path.size() = " << _path.size() << std::endl;
		
		uint64_t used_tiles_size = _used_tile_ids.size();
		Serializer::serialize(o_stream, used_tiles_size);
		
		std::set<Tile::Id>::const_iterator used_tile_ids_iter
			= _used_tile_ids.begin();
		std::set<Tile::Id>::const_iterator used_tile_ids_iter_end
			= _used_tile_ids.end();
		while (used_tile_ids_iter != used_tile_ids_iter_end)
		{
			TileCache::Pointer tile_pointer
				= _tile_cache->get(*used_tile_ids_iter);
			const Tile& tile = *tile_pointer;
			
			Serializer::serialize(o_stream, tile);
			
			++used_tile_ids_iter;
		}
		
		int64_t index = 0;
		D_Path::const_iterator iter = _path.begin();
		D_Path::const_iterator iter_end = _path.end();
		while (iter != iter_end)
		{
			if (iter->second->_serializer_id == -1)
			{
				iter->second->_serializer_id = index;
				++index;
			}
			
			++iter;
		}
		
		iter = _path.begin();
		for (; iter != iter_end; ++iter)
		{
			const PathEntry* path_entry = iter->second;
			if (path_entry->_connection != 0)
			{
				path_entry->_serializer_connection_id
					= path_entry->_connection->_serializer_id;
			} else
			{
				path_entry->_serializer_connection_id = -1;
			}
		}
		
		D_PathHardCopy path_hard_copy;
		
		iter = _path.begin();
		for(; iter != iter_end; ++iter)
		{
			PathEntry* xyz = iter->second;
			PathEntry abc = *(iter->second);
			path_hard_copy.insert(
				std::make_pair(iter->first, *(iter->second)) );
		}
		
		Serializer::serialize(o_stream, _trace);
		Serializer::serialize(o_stream, path_hard_copy);
	}
	
	
	std::ostream&
	operator<<(std::ostream& out, const TraceProcessor::Protocol& protocol)
	{
		out << "Trace: " << protocol._trace << std::endl;
		out << "Path: " << protocol._path << std::endl;
		
		return out;
	}
	
} // namespace mapgeneration
