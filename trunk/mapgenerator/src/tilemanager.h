/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TILEMANAGER_H
#define TILEMANAGER_H

namespace mapgeneration
{
	class TileManager;
}

#include <cc++/thread.h>
#include <list>
#include <map>

#include "filteredtrace.h"
#include "tile.h"
#include "tilecache.h"
#include "traceprocessor.h"
#include "util/mlog.h"
#include "util/pubsub/servicesystem.h"


using namespace mapgeneration_util;


namespace mapgeneration
{

	/**
	 * @brief TileManager manages the incoming FilteredTraces, the chache
	 * connections and starts TraceProcessors.
	 * 
	 * This class is one of the main classes. It will perform various tasks:
	 * <ul>
	 * <li>receive new traces</li>
	 * <li>start new TraceProcessors</li>
	 * <li>manage the reservation of tiles</li>
	 * <li>... (extensable)</li>
	 * </ul>
	 */
	class TileManager : public ControlledThread
	{

		public:

			/**
			 * @brief Constructor that introduces the caches.
			 * 
			 * @param edge_cache pointer to the edge cache
			 * @param tile_cache pointer to the tile cache
			 */
			TileManager(pubsub::ServiceList* service_list, TileCache* tile_cache);


			/**
			 * @brief Destructor.
			 */
			~TileManager();
						
			
			/**
			 * @return pointer to the tile cache
			 */
			inline TileCache*
			get_tile_cache();
			

			/**
			 * @brief This methods has to be called when a new trace is inserted
			 * into the routines of the TileManager, ready for the Traceprocessor.
			 * 
			 * It calculates the needed tile IDs for the filtered_trace and puts it
			 * into a queue.
			 * 
			 * @param filtered_trace the filtered trace
			 */
			void
			new_trace(FilteredTrace& filtered_trace);
			
			
			/**
			 * @brief The TraceProcessors call this method to notify the TileManager
			 * that they have finished and can be deleted.
			 * 
			 * @param trace_processor_id id of the TraceProcessor ready for deletion
			 */
			void
			trace_processor_finished(unsigned int trace_processor_id);
			
			
		protected:
			
			void
			thread_deinit();
			
			
			/**
			 * @brief "The thread itself" and also the main method of the TileManager.
			 */
			void
			thread_run();
			
	
		private:

			/**
			 * @brief Pointer to the edge cache.
			 */		
//			EdgeCache* _edge_cache;
			
			
			/**
			 * @brief List of the TraceProcessorIDs of the TraceProcessors 
			 * that are finished and can be deleted.
			 */
			std::vector<unsigned int> _finished_trace_processor_ids;
			
			
			/**
			 * @brief Map of 2-tuples (TileID, TraceProcessorID) indicating locked
			 * tiles.
			 * 
			 * Locked tiles are necessery to avoid deadlocks.
			 */
			std::map<unsigned int, unsigned int> _locked_tiles;
			
			
			/**
			 * @brief Counter for the next traceprocessor id.
			 */
			unsigned int _next_trace_processor_id;
			
			
			/**
			 * @brief A pointer to the central ServiceList.
			 */
			pubsub::ServiceList* _service_list;


			/**
			 * @brief Queue of FilteredTraces.
			 */
			std::list<FilteredTrace> _trace_queue;

			/**
			 * @brief Pointer to the tile cache.
			 */
			TileCache* _tile_cache;

			
			/**
			 * @brief Map of 2-tuples (TraceporcessorID, Pointer) for identifying the
			 * TraceProcessors by IDs.
			 */
			std::map<unsigned int, TraceProcessor*> _trace_processors;
			
			
			/**
			 * @brief This method is run by thread_run to delete a specific
			 * TraceProcessor.
			 * 
			 * @param id id of the TraceProcessor to delete
			 */
			void
			delete_trace_processor(unsigned int id);
			
			
			/**
			 * @brief This method is called when the TileManager decides to process
			 * the next FilteredTrace.
			 * 
			 * @param filteredTrace the filtered trace
			 */
			unsigned int
			process_trace(FilteredTrace& filteredTrace);

	};

	
	
	inline TileCache*
	TileManager::get_tile_cache()
	{
		return _tile_cache;
	}

} // namespace mapgeneration

#endif //TILEMANAGER_H
