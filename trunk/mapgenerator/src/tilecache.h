/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TILECACHE_H
#define TILECACHE_H

#include "tile.h"
#include "dbconnection/dbconnection.h"
#include "util/cache.h"

using namespace mapgeneration_util;

namespace mapgeneration
{
	
	/**
	 * \brief TileCache is an implementation of a Cache that uses the 
	 * database table for tiles as storage.
	 * 
	 * TileCache just defines the four virtual functions from Cache. For
	 * further information look at the documentation for Cache.
	 */
	 class TileCache : public Cache<unsigned int, Tile>{
		
		public:

			/**
			 * The default constructor.
			 */
			TileCache::TileCache(DBConnection* db_connection, 
				size_t table_id,
				Strategy strategy = _FIFO, unsigned int options = 0, 
				int hard_max_cached_size=0, int soft_max_cached_size=0);


			/**
			 * The destructor.
			 */
			~TileCache();
		

		protected:
		
			bool
			persistent_erase(unsigned int id);
			
			
			std::vector<unsigned int>
			persistent_get_free_ids();
			
			
			std::vector<unsigned int>
			persistent_get_used_ids();			
			
		
			Tile*
			persistent_load(unsigned int id);
			
			
			void
			persistent_save(unsigned int id, Tile* tile);
			
			
			int
			persistent_size_of(Tile* tile);

			
		private:
		
			/** 
			 * \brief The used DB-Connection.
			 */			
			DBConnection* _db_connection;
			
			
			/**
			 * @brief The id of the tiles table in the DBConnection.
			 */
			size_t _table_id;

	};


} // namespace mapgeneration_util

#endif // TILECACHE_H
