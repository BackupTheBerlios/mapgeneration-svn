/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EDGECACHE_H
#define EDGECACHE_H

#include "dbconnection.h"
#include "edge.h"
#include "util/cache.h"


using namespace mapgeneration_util;


namespace mapgeneration
{

	/**
	 * \brief EdgeCache is an implementation of a Cache that uses the 
	 * database table for edges as storage.
	 * 
	 * EdgeCache just defines the four virtual functions from Cache. For
	 * further information look at the documentation for Cache.
	 */
	class EdgeCache : public Cache<unsigned int, Edge >{
		
		public:

			/**
			 * \brief The default constructor.
			 */
			EdgeCache::EdgeCache(DBConnection* db_connection,
				Strategy strategy = _FIFO, unsigned int options = 0, 
				int hard_max_cached_size = 0, int soft_max_cached_size=0);


			/**
			 * \brief The destructor.
			 */
			~EdgeCache();
					
		
		protected:
		
			bool
			persistent_erase(unsigned int id);
			
			
			std::vector<unsigned int>
			persistent_get_free_ids();
			
			
			std::vector<unsigned int>
			persistent_get_used_ids();
									
		
			Edge*
			persistent_load(unsigned int id);
			
			
			void
			persistent_save(unsigned int id, Edge* edge);
			

			int
			persistent_size_of(Edge* edge);

			
		private:
		
			/** 
			 * \brief The used DB-Connection.
			 */
			DBConnection* _db_connection;

	};


} // namespace mapgeneration_util

#endif // EDGECACHE_H
