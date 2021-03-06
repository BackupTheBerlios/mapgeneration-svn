/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tilecache.h"

using namespace mapgeneration_util;

namespace mapgeneration
{

	TileCache::TileCache(DBConnection* db_connection, 
		size_t table_id,
		Strategy strategy,
		unsigned int options, int hard_max_cached_size, 
		int soft_max_cached_size)
	: Cache<unsigned int, Tile>::Cache(strategy, options, hard_max_cached_size, 
		soft_max_cached_size), _db_connection(db_connection), 
		_table_id(table_id)
	{
	}


	TileCache::~TileCache()
	{
	}
		
	
	bool
	TileCache::persistent_erase(unsigned int id)
	{
		_db_connection->remove(_table_id, id);
	}
	
	
	std::vector<unsigned int>
	TileCache::persistent_get_free_ids()
	{
		return _db_connection->get_free_ids(_table_id);
	}
	
	
	std::vector<unsigned int>
	TileCache::persistent_get_used_ids()
	{
		return _db_connection->get_all_used_ids(_table_id);
	}
	
	
	Tile*
	TileCache::persistent_load(unsigned int id)
	{
		std::string* tile_string = _db_connection->load(_table_id, id);
		if (!tile_string) return 0;
		
		Tile* tile = new Tile;
		Serializer::deserialize(*tile_string, *tile);
		delete tile_string;

		return tile;
	}
	

	void
	TileCache::persistent_save(unsigned int id, Tile* tile)
	{
		std::string tile_string = Serializer::serialize(*tile);
		_db_connection->save(_table_id, id, tile_string);
	}


	int
	TileCache::persistent_size_of(Tile* tile)
	{
		/** @todo Perhaps this should be implemented more exact. */
		if (tile == 0)
			return sizeof(Pointer);
		else
			return tile->size_of();
	}


} // namespace mapgeneration_util
