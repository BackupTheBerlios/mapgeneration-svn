/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <vector>
#include "dbconnection/odbcdbconnection.h"
#include "tile.h"
#include "tilecache.h"
#include "util/mlog.h"

using namespace mapgeneration;
using namespace mapgeneration_util;


void show_state(TileCache& tile_cache, std::vector<int>& used_tiles)
{
	mlog(MLog::debug, "test_tilecache") << "Cache state (only our tiles shown):\n";
	mlog(MLog::debug, "test_tilecache") << "Elements in Cache: " << tile_cache.cached_objects() << "\n";
	mlog(MLog::debug, "test_tilecache") << "Accumulated size in Cache: " << tile_cache.cached_size() << "\n";
	std::vector<int>::iterator iter = used_tiles.begin();
	std::vector<int>::iterator iter_end = used_tiles.end();
	for (; iter != iter_end; ++iter)
	{
		TileCache::Entry* entry;
		entry = tile_cache.get_entry(*iter);
		if (entry)
			mlog(MLog::error, "test_tilecache") << "Tile " << (*iter) << "  Users: " << entry->users << "  Dirty: " << entry->dirty <<"\n";
		else 
			mlog(MLog::error, "test_tilecache") << "Tile " << (*iter) << " not in cache.\n";
	}
}


int main()
{
	
	/*
	 * Initialization
	 */
	std::vector<int> used_tiles;
	
	mlog(MLog::info, "test_tilecache") << "Starting!\n";
	
	mlog(MLog::info, "test_tilecache") << "This test will try not to destroy an existing db."
		<< " It will just add some empty tiles.\n";
		
	mlog(MLog::debug, "test_tilecache") << "Creating DBConnection\n";
	ODBCDBConnection* db_connection = new ODBCDBConnection();
	size_t test_table_id = db_connection->register_table("test");
	db_connection->set_parameters("MapGeneration", "mapgeneration", "mg", true);
	db_connection->connect();
	
	mlog(MLog::debug, "test_tilecache") << "Generating a cache with some content...\n";	
	TileCache* tile_cache = new TileCache(db_connection, test_table_id, 
		TileCache::_FIFO, TileCache::_STANDARD_CACHE, 1000000, 800000);


	/*
	 * Search for unused slots
	 */	
	mlog(MLog::debug, "test_tilecache") << "Will try to use these tiles:";
	for (int i=-1000; i<0 && used_tiles.size()<5; ++i)
	{
		if (tile_cache->get(i) == 0)
		{
			used_tiles.push_back(i);
			mlog << " " << i;
		}		
	}
	mlog << "\n";


	/*
	 * Insertion of elements.
	 */	
	mlog(MLog::debug, "test_tilecache") << "Inserting tiles for use:";
	std::vector<int>::iterator iter = used_tiles.begin();
	std::vector<int>::iterator iter_end = used_tiles.end();
	for (; iter != iter_end; ++iter)
	{
		if (tile_cache->insert(*iter, new Tile()))
			mlog << " " << (*iter);
		else 
			mlog(MLog::error, "test_tilecache") << "\n" << "Could not insert tile "
				<< (*iter);
	}
	mlog << "\n";
	show_state(*tile_cache, used_tiles);

	mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";
	show_state(*tile_cache, used_tiles);


	/*
	 * Const Pointer check.
	 */
	mlog(MLog::debug, "test_tilecache") << "We will now use tile " << used_tiles[1] << " as const and see what happens: \n";
	{
		const TileCache::Pointer const_pointer = tile_cache->get(used_tiles[1]);
		mlog(MLog::debug, "test_tilecache") << "Id of the tile: " << const_pointer->get_id() << "\n";
		show_state(*tile_cache, used_tiles);
		mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";
	}
	mlog(MLog::debug, "test_tilecache") << "After leaving the scope of the pointer:\n";
	show_state(*tile_cache, used_tiles);
	mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";


	/*
	 * Pointer check.
	 */
	mlog(MLog::debug, "test_tilecache") << "Same procedure again with tile " << used_tiles[1] << " using a writeable pointer: \n";
	{
		TileCache::Pointer writeable_pointer = tile_cache->get(used_tiles[1]);
		mlog(MLog::debug, "test_tilecache") << "Size of the tile: " << writeable_pointer->size_of() << "\n";
		mlog(MLog::debug, "test_tilecache") << "Also adding a node to the tile.\n";
		Node node;
		writeable_pointer.write().add_node(node);
		show_state(*tile_cache, used_tiles);
		mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";
	}
	mlog(MLog::debug, "test_tilecache") << "After leaving the scope of the pointer:\n";
	show_state(*tile_cache, used_tiles);
	mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";
	
	
	/*
	 * Flushing the cache.
	 */
	mlog(MLog::debug, "test_tilecache") << "Flush flushed " << tile_cache->flush() << " elements.\n";
	tile_cache->flush();
	show_state(*tile_cache, used_tiles);
	
	
	/*
	 * Accessing tile.
	 */
	mlog(MLog::debug, "test_tilecache") << "Accessing tile " << used_tiles[1] << "\n";
	{
		TileCache::Pointer writeable_pointer = tile_cache->get(used_tiles[1]);
		mlog(MLog::debug, "test_tilecache") << "Size of the tile: " << writeable_pointer->size_of() << "\n";
		mlog(MLog::debug, "test_tilecache") << "Also adding a node to the tile.\n";
		Node node;
		writeable_pointer.write().add_node(node);
		show_state(*tile_cache, used_tiles);
		mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";
	}
	mlog(MLog::debug, "test_tilecache") << "After leaving the scope of the pointer:\n";
	show_state(*tile_cache, used_tiles);
	mlog(MLog::debug, "test_tilecache") << "Writeback wrote " << tile_cache->write_back() << " elements.\n";
	
	/*
	 * Removing tiles.
	 */
	mlog(MLog::debug, "test_tilecache") << "And finaly as promised: Removing used tiles:";
	iter = used_tiles.begin();
	iter_end = used_tiles.end();
	for (; iter != iter_end; ++iter)
	{
		if (tile_cache->remove(*iter))
			mlog << " " << (*iter);
	}
	mlog << "\n";
	
	mlog(MLog::debug, "test_tilecache") << "And deleting the test table: ";
	db_connection->drop_tables();
	mlog << "Ok\n";

	mlog(MLog::debug, "test_tilecache") << "Deleting TileCache!\n";
	delete tile_cache;
	
	mlog(MLog::debug, "test_tilecache") << "Deleting DBConnection!\n";
	delete db_connection;
	
	mlog(MLog::info, "test_tilecache") << "Finished.\n";

	return 0;
}
