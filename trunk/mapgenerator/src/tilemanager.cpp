/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tilemanager.h"

#include <vector>

#include "util/arithmeticoperations.h"
#include "util/mlog.h"
#include "util/pubsub/arithmeticservice.h"

using namespace mapgeneration_util;

namespace mapgeneration
{

	TileManager::TileManager(pubsub::ServiceList* service_list, 
		TileCache* tile_cache)
	: _tile_cache(tile_cache), _service_list(service_list), _trace_queue(),
		_trace_queue_mutex()
	{
		_finished_trace_processor_ids;
		_locked_tiles;
		_next_trace_processor_id = 1;
		_trace_queue;
		_trace_processors;
		
		/* init service for processed filtered traces... */
		pubsub::GenericService* meters_service
			= new pubsub::ArithmeticService<double>(
				"statistics.received_filtered_trace_meters",
				0.0, mapgeneration_util::add<double>
			);
		
		_service_list->add(meters_service);
		
		pubsub::GenericService* times_service
			= new pubsub::ArithmeticService<double>(
				"statistics.received_filtered_trace_times",
				0.0, mapgeneration_util::add<double>
			);
		
		_service_list->add(times_service);
		/* done. */
	}
	
	
	TileManager::~TileManager()
	{
	}
	
	
/*	unsigned int
	TileManager::create_new_edge(std::pair<unsigned int, unsigned int> first_node_id, 
		std::pair<unsigned int, unsigned int> second_node_id, double time)
	{
		Edge* new_edge = new Edge();		
		unsigned int this_edge_id;
		_edge_cache->insert(&this_edge_id, new_edge);
		_edge_cache->get(this_edge_id).write().set_id(this_edge_id);
		_edge_cache->get(this_edge_id).write().init(first_node_id, second_node_id, time);
		EdgeCache::Pointer ex = _edge_cache->get(this_edge_id);
		
		_tile_cache->get(first_node_id.first)
			.write().nodes()[first_node_id.second].second.edge_ids().push_back(this_edge_id);
		_tile_cache->get(second_node_id.first)
			.write().nodes()[second_node_id.second].second.edge_ids().push_back(this_edge_id);			
		
		return this_edge_id;
	}*/
	
	
/*	std::vector<Node>
	TileManager::get_edge_nodes(unsigned int edge_id)
	{
		std::vector<Node> result;
		
		EdgeCache::Pointer edge = _edge_cache->get(edge_id);
		if (edge == 0)
		{
			mlog(MLog::error, "TileManager::get_nodes") << "Could not load edge! Returning empty result.\n";
			return result;
		}		
		
		std::list< std::pair<unsigned int, unsigned int> >::const_iterator iter = edge->node_ids().begin();
		std::list< std::pair<unsigned int, unsigned int> >::const_iterator iter_end = edge->node_ids().end();
		
		// With an invalid value for tile_id we could skip this extra init..
		TileCache::Pointer tile;
		unsigned int tile_id;
		if (iter != iter_end)
		{
			tile_id = iter->first;
			tile = _tile_cache->get(tile_id);
			if (tile == 0)
			{
				mlog(MLog::error, "Edge::get_nodes") << "Could not load tile referenced by edge! Returning empty result.\n";
				return result;
			}
		}

		for (; iter != iter_end; ++iter)
		{
			if (iter->first != tile_id)
			{
				tile_id = iter->first;
				tile = _tile_cache->get(tile_id);
				if (tile == 0)
				{
					mlog(MLog::error, "Edge::get_nodes") << "Could not load tile referenced by edge! Returning partial result.\n";
					return result;
				}					
			}
			
			result.push_back(tile->nodes()[iter->second].second);
		}
		
		return result;
	}*/
	
	
/*	std::vector< std::vector<Node> >
	TileManager::get_tile_edges_nodes(const unsigned int tile_id)
	{
		std::vector< std::vector<Node> > edges_nodes;
	
		const TileCache::Pointer tile = _tile_cache->get(tile_id);
		
		const std::vector<unsigned int> edge_ids = tile->get_edge_ids();
		std::vector<unsigned int>::const_iterator iter = edge_ids.begin();
		std::vector<unsigned int>::const_iterator iter_end = edge_ids.end();
		for (; iter != iter_end; ++iter)
		{
			edges_nodes.push_back(get_edge_nodes(*iter));
		}

		return edges_nodes;
	}*/


	void
	TileManager::new_trace(FilteredTrace& filtered_trace)
	{
		double search_radius_m;
		_service_list->get_service_value(
			"traceprocessor.search_radius_m", search_radius_m
		);
		filtered_trace.calculate_needed_tile_ids(search_radius_m * 2.5);
		
		_trace_queue_mutex.enterMutex();
		_trace_queue.push_back(filtered_trace);
		_trace_queue_mutex.leaveMutex();
	}
	
	
/*	unsigned int
	TileManager::request_edge_connect(unsigned int from_last_of_this_edge_id, unsigned int to_first_of_that_edge_id)
	{
//		std::cout << "TileManager::request_edge_connect; ";
		EdgeCache::Pointer first_edge = _edge_cache->get(from_last_of_this_edge_id);
		EdgeCache::Pointer second_edge = _edge_cache->get(to_first_of_that_edge_id);
		
		std::list< std::pair<unsigned int, unsigned int> >::iterator iter =
			second_edge.write().node_ids().begin();
		std::list< std::pair<unsigned int, unsigned int> >::iterator iter_end = 
			second_edge.write().node_ids().end();
		
		// With an invalid value for tile_id we could skip this extra init.. 
		TileCache::Pointer tile;
		unsigned int tile_id;
		if (iter != iter_end)
		{
			tile_id = iter->first;
			tile = _tile_cache->get(tile_id);
			if (tile == 0)
			{
				mlog(MLog::error, "Edge::get_nodes") << "Could not load tile referenced by edge!\n";
			}
		}

		for (; iter != iter_end; ++iter)
		{
			if (iter->first != tile_id)
			{
				tile_id = iter->first;
				tile = _tile_cache->get(tile_id);
				if (tile == 0)
				{
					mlog(MLog::error, "Edge::get_nodes") << "Could not load tile referenced by edge!\n";
				}
			}
			
			tile.write().nodes()[iter->second].second.renumber(to_first_of_that_edge_id, from_last_of_this_edge_id);
		}
		
		first_edge.write().node_ids().insert(first_edge.write().node_ids().end(), second_edge.write().node_ids().begin(), second_edge.write().node_ids().end());
		// @todo Perhaps we need a possibility to delete an edge!
		// Here: second_edge is obsolent!
		
		return (first_edge->get_id());
	}*/
	
	
/*	void
	TileManager::request_edge_extend(unsigned int edge_id, std::pair<unsigned int, unsigned int> next_to_node_id, 
		std::pair<unsigned int, unsigned int> new_node_id, double time)
	{
//		std::cout << "TileManager::request_edge_extend; ";
		EdgeCache::Pointer edge = _edge_cache->get(edge_id);
		
		if (edge == 0)
		{
			mlog(MLog::error, "TileManager::request_edge_extend") << "You should really not try to extend a non existent edge!\n";
			return;
		}
		
		edge.write().extend(next_to_node_id, new_node_id, time);
		
		_tile_cache->get(new_node_id.first)
			.write().nodes()[new_node_id.second].second.edge_ids().push_back(edge->get_id());
	}


	bool
	TileManager::request_edge_split(
		unsigned int edge_id, std::pair<unsigned int, unsigned int> to_node_id,
		unsigned int* new_edge_1_id, unsigned int* new_edge_2_id
	)
	{
//		std::cout << "TileManager:request_edge_split; ";
//		lock_edge_tiles(edge_id); // @todo think about that! 
		
		EdgeCache::Pointer edge = _edge_cache->get(edge_id);
		if (edge == 0)
		{
			mlog(MLog::error, "TileManager") << "You should really not try to split "
				<< "a non existent edge!\n";
			return false;
		}
		
		// no split needed when condition becomes true!
		if (edge->node_is_at_start(to_node_id) || edge->node_is_at_end(to_node_id))
			return false;

		std::list< std::pair<unsigned int, unsigned int> >::iterator find_iter =
				std::find(edge.write().node_ids().begin(), edge.write().node_ids().end(), to_node_id);
		if (find_iter != edge.write().node_ids().end())
		{
			// create new edge (but do not use so-called method!) 
			unsigned int new_edge_id;
			_edge_cache->insert(&new_edge_id, new Edge());
			EdgeCache::Pointer new_edge = _edge_cache->get(new_edge_id);
			new_edge.write().set_id(new_edge_id);
			
			// move all nodes until to_node_id (exclusively) to the new edge
			//
			// a variante tres chic:
			// std::transform(edge->node_ids().begin(), find_iter, new_edge->node_ids().end(), FUNCTION); 
			std::list< std::pair<unsigned int, unsigned int> >::iterator loop_iter = edge.write().node_ids().begin();
			for (; loop_iter != find_iter; ++loop_iter)
			{
				Node& node = _tile_cache->get(loop_iter->first).write().nodes()[loop_iter->second].second;
				node.renumber(edge_id, new_edge->get_id());
			}
			new_edge.write().node_ids().splice(new_edge.write().node_ids().begin(), edge.write().node_ids(),
					edge.write().node_ids().begin(), find_iter);
			
			// copy to_node_id into new_edge 
			Node& node = _tile_cache->get(find_iter->first).write().nodes()[find_iter->second].second;
			node.edge_ids().push_back(new_edge->get_id());
			new_edge.write().node_ids().push_back(*find_iter);
			
			// move times
			//
			 // would be nice, but list::iterator does not support operator+()!
			// new_edge->times().splice(new_edge->times().begin(), edge->times(),
			//		edge->times().begin(), edge->times().begin() + new_edge->node_ids().size() - 1); 
			std::list<double>::iterator edge_times_iter = edge.write().times().begin();
			int size = static_cast<int>(new_edge->node_ids().size());
			for (int count = 0; count < size - 2; ++count)
				++edge_times_iter;

			new_edge.write().times().splice(new_edge.write().times().end(), edge.write().times(),
					edge.write().times().begin(), edge_times_iter);
			
			if (new_edge_1_id != 0)
				*new_edge_1_id = edge_id;
			if (new_edge_2_id != 0)
				*new_edge_2_id = new_edge_id;
			
			return true;
		} else
		{
			mlog(MLog::warning, "TileManager") << "Did not find given node id!\n";
			return false;
		}
		
//		unlock_edge_tiles(edge_id);
	}*/


	void
	TileManager::trace_processor_finished(unsigned int trace_processor_id)
	{
		_finished_trace_processor_ids.push_back(trace_processor_id);		
	}
	
	
	void
	TileManager::thread_deinit()
	{
		double meters;
		_service_list->get_service_value("statistics.received_filtered_trace_meters", meters);
		
		double times;
		_service_list->get_service_value("statistics.received_filtered_trace_times", times);
		
		mlog(MLog::info, "TileManager") << "Received filtered trace meters: "
			<< meters << "m\n";
		mlog(MLog::info, "TileManager") << "Received filtered trace times: "
			<< times << "s (= " << (times / 3600.0) << "h)\n";
	}
	
	
	void
	TileManager::thread_run()
	{
		mlog(MLog::info, "TileManager") << "Running.\n";
		while (!should_stop())
		{
			/* Delete finished TraceProcessors. */
			while(_finished_trace_processor_ids.size() > 0)
			{
				delete_trace_processor(_finished_trace_processor_ids.back());
				_finished_trace_processor_ids.pop_back();
			}
			
			int new_trace_processors = 2 - _trace_processors.size();

			_trace_queue_mutex.enterMutex();
			std::list<FilteredTrace>::iterator iter = _trace_queue.begin();
			while (iter != _trace_queue.end() && new_trace_processors > 0)
			{
				/** @todo: This loop might be expensive, because it will try
				 * to start the same blocked traces again and again.*/
				//mlog(MLog::debug, "TileManager") << "Trying to create new TraceProcessor.\n";
				/* Copy and remove the next FilteredTrace from the queue */
				FilteredTrace new_filtered_trace(*iter);

				/* Try to create the new TraceProcessor. */
				unsigned int new_trace_processor_id = process_trace(new_filtered_trace);

				if (new_trace_processor_id != 0)
				{
					mlog(MLog::debug, "TileManager") << "Created new TraceProcessor "
						<< new_trace_processor_id << ".\n";
					iter = _trace_queue.erase(iter);
					--new_trace_processors;
				} else
					++iter;
			}
			_trace_queue_mutex.leaveMutex();
			
			_should_stop_event.wait(100);
		}

		mlog(MLog::info, "TileManager") << "Stopped.\n";
	}
	
	
	void
	TileManager::delete_trace_processor(unsigned int trace_processor_id)
	{
		std::vector< std::map<unsigned int, unsigned int>::iterator > iterators;
		std::map<unsigned int, unsigned int>::iterator iter = _locked_tiles.begin();
		for (; iter != _locked_tiles.end(); ++iter)
		{
			if (iter->second == trace_processor_id) 
				iterators.push_back(iter);
		}
		
		std::vector< std::map<unsigned int, unsigned int>::iterator >::iterator iter2 =
			iterators.begin();
		std::vector< std::map<unsigned int, unsigned int>::iterator >::iterator iter2_end =
			iterators.end();
		for (; iter2 != iter2_end; iter2++)
		{
			_locked_tiles.erase(*iter2);
		}
		
		
		std::map<unsigned int, TraceProcessor*>::iterator search_result = _trace_processors.find(trace_processor_id);
		if (search_result != _trace_processors.end())
		{
			delete search_result->second;
			_trace_processors.erase(search_result);
			mlog(MLog::debug, "TileManager") << "Deleted TraceProcessor " 
				<< trace_processor_id << "\n";
		} else
		{
			mlog(MLog::error, "TileManager") << "Could not find TraceProcessor "
				<< trace_processor_id << " to delete it!!!";
		}
	}
	
	
/*	void
	TileManager::lock_edge_tiles(unsigned int edge_id)
	{
	}*/


	unsigned int
	TileManager::process_trace(FilteredTrace& filtered_trace)
	{
		std::vector<unsigned int> needed_tile_ids = filtered_trace.get_needed_tile_ids();
		
		/* Check if all needed tiles are available. */
		std::vector<unsigned int>::iterator iter = needed_tile_ids.begin();
		std::vector<unsigned int>::iterator iter_end = needed_tile_ids.end();
		std::map<unsigned int, unsigned int>::iterator search_result;
		std::map<unsigned int, unsigned int>::iterator not_found_position = _locked_tiles.end();
		bool all_available = true;
		for (; iter != iter_end && all_available; ++iter)
		{
			search_result = _locked_tiles.find(*iter);
			if (search_result != not_found_position)
				all_available = false;
		}
		
		/* If a tile is not available the processor cannot be started. */
		if (!all_available) return 0;
		
		/* Get the _next_trace_processor_id and increase it for
		 * the next TraceProcessor */
		unsigned int this_trace_processor_id = _next_trace_processor_id;
		do {
			_next_trace_processor_id++;
		} while (_next_trace_processor_id < 1);
		
		/* Lock the needed tiles. */
		iter = needed_tile_ids.begin();
		iter_end = needed_tile_ids.end();
		for (; iter != iter_end && all_available; ++iter)
		{
			std::pair<unsigned int, unsigned int> locked_tiles_entry;
			locked_tiles_entry.first = *iter;
			locked_tiles_entry.second = this_trace_processor_id;
			_locked_tiles.insert(locked_tiles_entry);
		}
		
		/** @todo A mutex is needed here (EdgeSplit between push_back and run).*/
		/* Create a new TraceProcessor */
		TraceProcessor* new_trace_processor = new TraceProcessor(
			this_trace_processor_id,
			this, _service_list, filtered_trace
		);
				
		/* Create the entry for the map of TraceProcessors and insert it */
		std::pair<unsigned int, TraceProcessor*> trace_entry;
		trace_entry.first = this_trace_processor_id;
		trace_entry.second = new_trace_processor;
		_trace_processors.insert(trace_entry);
		
		/* Start the new TraceProcessor */
		new_trace_processor->start();
		
		return this_trace_processor_id;
	}


/*	void
	TileManager::unlock_edge_tiles(unsigned int edge_id)
	{
	}*/

} // namespace mapgeneration

