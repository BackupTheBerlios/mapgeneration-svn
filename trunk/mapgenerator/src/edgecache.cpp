/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "edgecache.h"

using namespace mapgeneration_util;

namespace mapgeneration
{

	EdgeCache::EdgeCache(DBConnection* db_connection, Strategy strategy, 
		unsigned int options, int hard_max_cached_size, 
		int soft_max_cached_size)
	: Cache<unsigned int, Edge >::Cache(strategy, options, hard_max_cached_size,
		soft_max_cached_size), _db_connection(db_connection)
	{
	}


	EdgeCache::~EdgeCache()
	{
	}


	bool
	EdgeCache::persistent_erase(unsigned int id)
	{
		_db_connection->delete_tile(id);
	}
	
	
	std::vector<unsigned int>
	EdgeCache::persistent_get_free_ids()
	{
		return _db_connection->get_free_edge_ids();
	}
	
	
	std::vector<unsigned int>
	EdgeCache::persistent_get_used_ids()
	{
		return _db_connection->get_all_used_edge_ids();
	}
	

	Edge*
	EdgeCache::persistent_load(unsigned int id)
	{
		std::string* edge_string = _db_connection->load_edge(id);
		if (!edge_string) return 0;
		
		Edge* edge = new Edge;
		Serializer::deserialize(*edge_string, *edge);

		return edge;
	}


	void
	EdgeCache::persistent_save(unsigned int id, Edge* edge)
	{
		std::string edge_string = Serializer::serialize(*edge);
		_db_connection->save_edge(id, edge_string);
	}


	int
	EdgeCache::persistent_size_of(Edge* edge)
	{
		if (edge == 0)
			return sizeof(Pointer);
		else
			return edge->size_of();
	}


} // namespace mapgeneration_util
