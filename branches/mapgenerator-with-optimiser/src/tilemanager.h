/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <list>
#include <map>
#include <vector>

#include "filteredtracereceiver.h"
#include "util/controlledthread.h"

/* Forward declarations... */
namespace pubsub
{
	
	class ServiceList;
	
} // namespace pubsub

namespace mapgeneration
{
	
	class TileCache;
	class TraceProcessor;
	
} // namespace mapgeneration
/* Forward declarations done! */


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
	class TileManager : public FilteredTraceReceiver,
		public mapgeneration_util::ControlledThread
	{

		public:

			/**
			 * @brief Constructor that introduces the caches.
			 * 
			 * @param tile_cache pointer to the tile cache
			 */
			TileManager(pubsub::ServiceList* service_list, TileCache* tile_cache);


			/**
			 * @brief Destructor.
			 */
			virtual
			~TileManager();
			
			
			inline void
			continue_processing();
			
			
			inline bool
			get_invalid_path_score() const;
			
			
			int
			get_number_of_processed_traces();
			
			
			inline bool
			get_optimisation_mode() const;
			
			
			inline bool
			get_single_trace_mode() const;
			
			
			time_t
			get_start_time() const;
			
			
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
			virtual void
			new_trace(FilteredTrace& filtered_trace);
			
			
			/**
			 * @brief The TraceProcessors call this method to notify the TileManager
			 * that they have finished and can be deleted.
			 * 
			 * @param trace_processor_id id of the TraceProcessor ready for deletion
			 */
			void
			trace_processor_finished(unsigned int trace_processor_id);
			
			
			inline void
			set_invalid_path_score();
			
			
			inline void
			set_optimisation_mode(bool flag);
			
			
			inline void
			set_single_trace_mode(bool flag);
			
			
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
			
			
			bool _invalid_path_score;
		
			
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
			
			
			int _number_of_processed_traces;
			
			
			ost::Mutex _number_of_processed_traces_mutex;
			
			
			bool _optimisation_mode;
			
			
			/**
			 * @brief A pointer to the central ServiceList.
			 */
			pubsub::ServiceList* _service_list;
			
			
			bool _single_trace_mode;
			ost::Event _single_trace_mode_event;
			
			
			time_t _start_time;


			/**
			 * @brief Queue of FilteredTraces.
			 */
			std::list<FilteredTrace> _trace_queue;


			/**
			 * @brief The mutex that protect the _trace_queue.
			 */
			ost::Mutex _trace_queue_mutex;
			
			
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

	
	inline void
	TileManager::continue_processing()
	{
		_single_trace_mode_event.signal();
	}
	
	
	inline bool
	TileManager::get_invalid_path_score() const
	{
		return _invalid_path_score;
	}
	
	
	inline bool
	TileManager::get_optimisation_mode() const
	{
		return _optimisation_mode;
	}
	
	
	inline bool
	TileManager::get_single_trace_mode() const
	{
		return _single_trace_mode;
	}
	
	
	inline TileCache*
	TileManager::get_tile_cache()
	{
		return _tile_cache;
	}
	
	
	inline void
	TileManager::set_invalid_path_score()
	{
		_invalid_path_score = true;
		
		if (_optimisation_mode)
			this->controlled_stop(false);
	}
	
	
	inline void
	TileManager::set_optimisation_mode(bool flag)
	{
		_optimisation_mode = flag;
	}
	
	
	inline void
	TileManager::set_single_trace_mode(bool flag)
	{
		_single_trace_mode = flag;
	}

} // namespace mapgeneration

#endif //TILEMANAGER_H
