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

#include "edge.h"
#include "edgecache.h"
#include "filteredtrace.h"
#include "tile.h"
#include "tilecache.h"
#include "traceprocessor.h"
#include "util/mlog.h"
#include "util/pubsub/servicelist.h"


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
			TileManager(pubsub::ServiceList* service_list, 
				EdgeCache* edge_cache, TileCache* tile_cache);


			/**
			 * @brief Destructor.
			 */
			~TileManager();
			
			
			/**
			 * @brief Creates a new edge.
			 * 
			 * This edge only contains the specified nodes and returns
			 * the id of the new edge.
			 * 
			 * @param first_node_id id of first node
			 * @param second_node_id id of second node
			 * @param time time between the given nodes
			 * @return id of the newly created edge
			 */
			unsigned int
			create_new_edge(std::pair<unsigned int, unsigned int> first_node_id, 
				std::pair<unsigned int, unsigned int> second_node_id, double time);
			
			
			/**
			 * @return pointer to the edge cache
			 */
			inline EdgeCache*
			get_edge_cache();
			
			
			/**
			 * @brief Returns the nodes of the specified edge.
			 * 
			 * @param edge_id the edge id
			 * @return vector of nodes
			 */
			std::vector<Node>
			get_edge_nodes(unsigned int edge_id);
			
			
			/**
			 * @return pointer to the tile cache
			 */
			inline TileCache*
			get_tile_cache();
			
			
			/**
			 * @brief Calculate every node from every edge used in specified tile.
			 * 
			 * @param  tile_id the tile id
			 * @return vector of nodes
			 */
			std::vector< std::vector<Node> >
			get_tile_edges_nodes(const unsigned int tile_id);


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
			 * @brief When an "edge connect" is requested by a Traceprocessor, this
			 * method will be invoked.
			 * 
			 * Connects the last node of from_last_of_this_edge_id to the first
			 * node of to_first_of_that_edge_id. It has to guarantee a convenient,
			 * correct working.
			 * 
			 * @param from_last_of_this_edge_id id of first edge
			 * @param to_first_of_that_edge_id id of second edge
			 * @return id of newly created edge
			 */
			unsigned int
			request_edge_connect(unsigned int from_last_of_this_edge_id, unsigned int to_first_of_that_edge_id);


			/**
			 * @brief When an "edge extend" is requested by a Traceprocessor, this
			 * method will be invoked.
			 * 
			 * Extends the edge "edge_id" by the node "new_node_id". This method
			 * searches for the node "next_to_node_id" and then performs there 
			 * the extension. It has to guarantee a convenient, correct working.
			 * 
			 * @param edge_id the edge id
			 * @param next_to_node_id the node which marks the right end of the edge
			 * @param new_node_id the id of the new node
			 * @param time time between next_to_node_id and new_node_id
			 */
			void
			request_edge_extend(unsigned int edge_id, std::pair<unsigned int, unsigned int> next_to_node_id, 
				std::pair<unsigned int, unsigned int> new_node_id, double time);
			
			
			/**
			 * @brief When an "edge split" is requested by a Traceprocessor, this
			 * method will be invoked.
			 * 
			 * Splits the edge "edge_id" at the node "to_node_id" and creates
			 * a new edge from one part. It has to guarantee a convenient,
			 * correct working.
			 * 
			 * @param edge_id the edge id
			 * @param to_node_id the node where the split takes place
			 * @param new_edge_1_id contains the first new edge ID after a successful
			 * invokation
			 * @param new_edge_2_id contains the second new edge ID after a successful
			 * invokation
			 * 
			 * @return true, if split was successful
			 */
			bool
			request_edge_split(unsigned int edge_id,
				std::pair<unsigned int, unsigned int> to_node_id,
				unsigned int* new_edge_1_id = 0, unsigned int* new_edge_2_id = 0);
			
			
			/**
			 * @brief The TraceProcessors call this method to notify the TileManager
			 * that they have finished and can be deleted.
			 * 
			 * @param trace_processor_id id of the TraceProcessor ready for deletion
			 */
			void
			trace_processor_finished(unsigned int trace_processor_id);
			
			
		protected:
		
			/**
			 * @brief "The thread itself" and also the main method of the TileManager.
			 */
			void
			thread_run();
			
	
		private:

			/**
			 * @brief Pointer to the edge cache.
			 */		
			EdgeCache* _edge_cache;
			
			
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
			 * @brief Returns a vector of tileIDs which are traversed by the edge.
			 * 
			 * @param edge_id the edge id
			 * @return vector of tileIDs
			 */
			std::vector<unsigned int>
			get_edge_tiles(unsigned int edge_id);
			

			/**
			 * @brief Locks tiles which are used when modifying the edge.
			 * 
			 * It has to notify the concerned TraceProcessors
			 * 
			 * @param edge_id the edge id
			 * 
			 * @see unlock_edge_tiles
			 */
			void
			lock_edge_tiles(unsigned int edge_id);
			
			
			/**
			 * @brief This method is called when the TileManager decides to process
			 * the next FilteredTrace.
			 * 
			 * @param filteredTrace the filtered trace
			 */
			unsigned int
			process_trace(FilteredTrace& filteredTrace);


			/**
			 * @brief Unlocks tiles which were previously locked by the corresponding
			 * method.
			 * 
			 * It unlocks the tiles which were used when modifying the edge. It
			 * has to notify the concerned Traceprocessors.
			 * 
			 * @param edge_id the edge id
			 * 
			 * @see lock_edge_tiles
			 */
			void
			unlock_edge_tiles(unsigned int edge_id);

	};


	inline EdgeCache*
	TileManager::get_edge_cache()
	{
		return _edge_cache;
	}
	
	
	inline TileCache*
	TileManager::get_tile_cache()
	{
		return _tile_cache;
	}

} // namespace mapgeneration

#endif //TILEMANAGER_H
