/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceprocessor.h"

#include <fstream>

#include "tilecache.h"
#include "tilemanager.h"
#include "util/fixedsizequeue.h"
#include "util/pubsub/servicesystem.h"

namespace mapgeneration
{
	
	using mapgeneration_util::FixedSizeQueue;
	using rangereporting::Quadtree;

	#warning rename _MAX_CURVATURE.
	
	const double TraceProcessor::_MIN_STEP_DISTANCE_M = 4.0;
	const double TraceProcessor::_MAX_CURVATURE = 5.0 * d2r;
	const double TraceProcessor::_STEP_DISTANCE_FACTOR = 2.0;
	const int TraceProcessor::_MAX_DEPTH = 15;
	const int TraceProcessor::_MAX_INVALID_ENTRIES_COUNTER = 2;
	const double TraceProcessor::_MAX_INVALID_ENTRIES_DISTANCE = 10.0;
	
	// -----------------------------------------------------------------------//
	// TraceProcessor
	// The implementation.
	// -----------------------------------------------------------------------//
	
/*	const double TraceProcessor::_BEST_SCORE = 10000000.0;
	const double TraceProcessor::_WORST_SCORE = -10000000.0;
	const double TraceProcessor::_INVALID_PATH_SCORE = _WORST_SCORE - 1.0;
	const Node::Id TraceProcessor::_VIRTUAL_NODE_ID_OFFSET = 0xFFFFFFFF00000001ULL;
*/

	TraceProcessor::TraceProcessor(unsigned int id, TileManager* tile_manager,
		pubsub::ServiceList* service_list, FilteredTrace& filtered_trace)
	: _conf(service_list), _filtered_trace(filtered_trace), _id(id),
		_path(), _protocol(), _service_list(service_list),
		_tile_manager(tile_manager), _virtual_entries()
	{
		_tile_cache = _tile_manager->get_tile_cache();
		
		// A BOUNDING BOX AROUND THE TRACE!!!!
		FilteredTrace::const_iterator iter = _filtered_trace.begin();
		FilteredTrace::const_iterator iter_end = _filtered_trace.end();
		
		double min_lat = 0.0;
		double min_lon = 0.0;
		double max_lat = 0.0;
		double max_lon = 0.0;
		
		if (iter != iter_end)
		{
			min_lat = iter->get_latitude();
			min_lon = iter->get_longitude();
			max_lat = iter->get_latitude();
			max_lon = iter->get_longitude();
			
			++iter;
			
			for (; iter != iter_end; ++iter)
			{
				if (iter->get_latitude() < min_lat)
					min_lat = iter->get_latitude();
				
				if (iter->get_latitude() > max_lat)
					max_lat = iter->get_latitude();
				
				if (iter->get_longitude() < min_lon)
					min_lon = iter->get_longitude();
				
				if (iter->get_longitude() > max_lon)
					max_lon = iter->get_longitude();
			}
		}
		
		VirtualNode lower_left_corner;
		lower_left_corner[0] = min_lat - 0.01;
		lower_left_corner[1] = min_lon - 0.01;
		
		VirtualNode upper_right_corner;
		upper_right_corner[0] = max_lat + 0.01;
		upper_right_corner[1] = max_lon + 0.01;
		
		rangereporting::AxisParallelRectangle<VirtualNode> span_rectangle;
		span_rectangle.set_point(0, lower_left_corner);
		span_rectangle.set_point(1, upper_right_corner);
		span_rectangle.validate_corners();
		
		_virtual_entries.set_span_rectangle(span_rectangle);
		_virtual_entries.set_max_depth(15);
		_virtual_entries.init_ready();
		
		mlog(MLog::debug, "TraceProcessor") << "Initialised (" << _id << ").\n";		
	}
	
	
	void
	TraceProcessor::ai_module()
	{
//		#ifdef DEBUG
//			std::cout << "ai_module: [begin]" << std::endl;
//			std::cout << _path << std::endl << std::endl;
//		#endif
				
/*		if (_protocol._path.empty())
		{
			D_Path::iterator iter = _path.begin();
			D_Path::iterator iter_end = _path.end();
			
			double start_position = iter->second->_scan_position;
			D_Path::iterator best_start_iter = iter;
			double best_score = _INVALID_PATH_SCORE;
			
			while ( (iter != iter_end)
				&& (iter->second->_scan_position - start_position
					<= _conf.get(_FAREST_START_POSITION)) )
			{
				PathEntry* entry = iter->second;
				
				if (!entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
	//				D_NodeIds this_recursion_virtual_found_node_ids;
					double score = _INVALID_PATH_SCORE;
					
					score = build_best_connections_recursively(iter, 1,
						iter->second->get_state()/*,
						this_recursion_virtual_found_node_ids******);
					
					score -=
						(iter->second->_scan_position - start_position)
						* _conf.get(_BASE_SCORE_DISTANT_START);
					
					if (!path_is_valid(iter->second))
					{
						score = _INVALID_PATH_SCORE;
					}
					
					iter->second->_score = score;
					
					if (score > best_score)
					{
						best_score = score;
						best_start_iter = iter;
					}
				}
				
				++iter;
			}
			
			best_start_iter->second->_is_beginning = true;
//			std::cout << "11111111111111111";
			
		} else
		{
			D_Path::iterator iter = _path.begin();
			D_Path::iterator iter_end = _path.end();
			while( (iter != iter_end)
				&& (iter->second != _protocol._path.rbegin()->second) )
			{
				++iter;
			}
			
			D_Path::iterator start_iter = iter;
			
			assert(start_iter != _path.end());
			
//			assert(start_iter->second->has_state(PathEntry::_REAL));
			
			PathEntry* start_entry = start_iter->second;
			start_entry->_is_visited = false;
			start_entry->_is_destination = false;
			start_entry->_is_beginning = true;
			start_entry->_score = _INVALID_PATH_SCORE;
			
			if (!_tile_manager->get_invalid_path_score())
			{
				assert(start_entry->_state == PathEntry::_REAL);
			}
			
#warning Little Hack. Test it!
			PathEntry::State saved_state = start_entry->_state;
			if (start_entry->_virtual_node_id > _VIRTUAL_NODE_ID_OFFSET - 1)
			{
				start_entry->_state = PathEntry::_VIRTUAL_CREATED;
			}
			
			// BAD HACK:
			// Change temporary the state of the start_entry.
/*			if (start_entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
			{
				if (start_entry->_position_on_trace == start_entry->_scan_position)
					start_entry->_state = PathEntry::_VIRTUAL_CREATED;
				else if (start_entry->_position_on_trace < start_entry->_scan_position)
					assert(false);
				else
					assert(false);
			}********
			
//			D_NodeIds this_recursion_virtual_found_node_ids;
			start_entry->_score = build_best_connections_recursively(
				start_iter, 1, start_entry->_state/*,
				this_recursion_virtual_found_node_ids****);
			
			if (!path_is_valid(start_entry))
			{
				start_entry->_score = _INVALID_PATH_SCORE;
			}
			
			start_entry->_state = saved_state;
			
			assert(start_iter->second == start_entry);
			assert(start_iter->second->_score == start_entry->_score);
			// BAD HACK cont'd:
			// Restore the state.
//			start_entry->_state = PathEntry::_REAL;
			
//			std::cout << "2222222222222222222222";
		}
		
//		std::cout << "end!" << std::endl;
*/
		assert(_path.size() > 0);
		
		const D_Path::iterator path_iter_begin = _path.begin();
		const D_Path::iterator path_iter_end = _path.end();
		
		D_Path::iterator path_iter = path_iter_end;
		--path_iter;
		
		while ( (!path_iter->second->_is_destination)
			&& (path_iter != path_iter_begin) )
		{
			--path_iter;
		}
		
		assert(path_iter != path_iter_end);
		assert(path_iter->second->_is_destination);
			
		path_iter->second->_score = _BEST_SCORE;
		path_iter->second->_connection = 0;
		
		while ( (path_iter != path_iter_begin)
			&& (!path_iter->second->_is_beginning) )
		{
			--path_iter;
			
			PathEntry* path_entry = path_iter->second;
			
			path_entry->_score = _WORST_SCORE;
			path_entry->_connection = 0;
			
			D_Path::iterator successor_iter = path_iter;
			++successor_iter;
			
			while ( (successor_iter != path_iter_end)
				&& (successor_iter->second->_scan_position
					- path_entry->_scan_position
					<= _conf.get(_FAREST_START_POSITION))
			)
			{
				PathEntry* successor_entry = successor_iter->second;
				
				double connection_score = calculate_connection_score(
					path_entry, successor_entry);
				
				if (connection_score > path_entry->_score)
				{
					path_entry->_score = connection_score;
					path_entry->_connection = successor_entry;
					
					if (path_entry->_state == successor_entry->_state)
					{
						path_entry->_equal_state_successors_count
							= successor_entry->_equal_state_successors_count + 1;
						
/*						if (path_entry->_state == PathEntry::_REAL)
						{
							TileCache::Pointer tile = _tile_cache->get(
								Node::tile_id(path_entry->_node_id));
							const Node& node = tile->node(path_entry->_node_id);
							
							if (!node.has_successor(successor_entry->_node_id))
							{
								path_entry->_equal_state_successors_count = 1;
							}
						}*/
					} else
					{
						path_entry->_equal_state_successors_count = 1;
					}
				}
				
				++successor_iter;
			}
		}
		
		assert(path_iter->second->_is_beginning);
		
		// validate and repair path
		int i = 0;
		const int MAX_LOOPS = 5;
		bool exit_loop = false;
		while (!exit_loop) 
		{
			if (path_is_valid(path_iter->second, true))
				exit_loop = true;
			
			if ( (!exit_loop) && (i >= MAX_LOOPS) )
				exit_loop = true;
			else
				++i;
		}
		
		if (i >= MAX_LOOPS)
			path_iter->second->_score = _INVALID_PATH_SCORE;
		
		
/*		double best_score = _INVALID_PATH_SCORE - 1;
		PathEntry* best_score_entry = 0;
		
		while( (path_iter != path_iter_end)
			&& (path_iter->second->_scan_position 
				<= _conf.get(_FAREST_START_POSITION))
		)
		{
			PathEntry* path_entry = path_iter->second;
			
			if (!path_is_valid(path_entry))
				path_entry->_score = _INVALID_PATH_SCORE;
			
			if (path_entry->_score > best_score)
			{
				best_score = path_entry->_score;
				best_score_entry = path_entry;
			}
			
			if (path_entry->_is_destination)
				path_iter = path_iter_end;
			else
				++path_iter;
		}
		
		assert(best_score_entry != 0);
		best_score_entry->_is_beginning = true;*/
	}
	
	
	bool
	TraceProcessor::apply_module()
	{
		assert(!_path.empty());
		
//		#ifdef DEBUG
//			std::cout << "apply_module:" << std::endl;
//			std::cout << _path << std::endl << std::endl;
//		#endif
				
		
		// The protocol contains all the information we need to merge the trace
		// to the existing map.
		
		// Find the beginning.
		PathEntry* beginning_entry = 0;
		if (true)
		{
			D_Path::iterator iter = _path.begin();
			D_Path::iterator iter_end = _path.end();
			while ( (iter != iter_end) && (!iter->second->_is_beginning) )
				++iter;
			
			assert(iter != iter_end);
			assert(iter->second->_is_beginning);
			
			beginning_entry = iter->second;
		}
		
//		if (iter == iter_end)
//		{
//			mlog(MLog::error, "TraceProcessor") << "protocol ends too early [1]\n";
//			return;
//		}
		
		if (beginning_entry->_score == _INVALID_PATH_SCORE)
		{
			mlog(MLog::warning, "TraceProcessor") << "Path has invalid path score. Discard this path!\n";
			
			_tile_manager->set_invalid_path_score();
			
//			std::cout << _path << std::endl << std::endl;
			
/*			for (int i = 0; i < _CONFIGURATION_VALUES; ++i)
			{
				std::cout << _conf._identifier_strings[i] << " = "
					<< _conf._values[i] << std::endl;
			}*/
			
			return false;
		}
		
		if (beginning_entry->_connection == 0)
		{
			assert(beginning_entry->_is_beginning);
			assert(beginning_entry->_is_destination);
			
			mlog(MLog::info, "TraceProcessor") << "Beginning == Destination\n";
//			mlog(MLog::debug, "TraceProcessor") << *previous_entry << "\n";
			
			return false;
		}
		
		// A map that maps Node::Ids to PathEntry. Needed when a Node
		// moves to another Tile.
		std::multimap<Node::Id, PathEntry*> node_ids_entries_mapper;
		if (true)
		{
			D_Path::iterator iter = _path.begin();
			D_Path::iterator iter_end = _path.end();
			for (; iter != iter_end; ++iter)
			{
				PathEntry* entry = iter->second;
				if(entry->has_state(PathEntry::_REAL))
				{
					node_ids_entries_mapper.insert(
						std::make_pair(entry->_node_id, entry));
				}
			}
		}
		
		PathEntry* current_entry = beginning_entry;
		
		assert(!current_entry->has_state(PathEntry::_VIRTUAL_FOUND));
		
		// Merge previous entry. Current entry is merged within the loop.
		if (current_entry->has_state(PathEntry::_REAL))
		{
			assert(verify_node_bearings(current_entry->_node_id));
			merge(node_ids_entries_mapper, current_entry);
			assert(verify_node_bearings(current_entry->_node_id));
			
		} else if (current_entry->has_state(PathEntry::_VIRTUAL_CREATED))
		{
			// Only holds in the beginning of the _protocol, if at all.
			assert(current_entry->_is_beginning);
			
			current_entry->_node_id = register_new_node(
				_virtual_entries.point(static_cast<D_IndexType>(
					current_entry->_virtual_node_id
					- _VIRTUAL_NODE_ID_OFFSET))
				);
			
			current_entry->_state = PathEntry::_REAL;
			
			node_ids_entries_mapper.insert(
				std::make_pair(current_entry->_node_id, current_entry));
		}
		
		current_entry = current_entry->_connection;
		while (current_entry != 0)
		{
			assert(current_entry->_backward_connection->has_state(PathEntry::_REAL));
			
			if (current_entry->has_state(PathEntry::_REAL))
			{
				assert(verify_node_bearings(current_entry->_node_id));
				merge(node_ids_entries_mapper, current_entry);
				assert(current_entry->has_state(PathEntry::_REAL));
				assert(verify_node_bearings(current_entry->_node_id));
				
			} else if (current_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				current_entry->_node_id = register_new_node(
					_virtual_entries.point(static_cast<D_IndexType>(
						current_entry->_virtual_node_id
						- _VIRTUAL_NODE_ID_OFFSET))
					);
				current_entry->_state = PathEntry::_REAL;
				
				assert(current_entry->has_state(PathEntry::_REAL));
				
			} else // if (current_entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
//				std::cout << "current_entry: " << *current_entry << std::endl;
				#ifndef NDEBUG
					if (current_entry->_position_on_trace >= current_entry->_scan_position)
					{
						std::cout << "path: " << _path << std::endl;
						std::cout << "HIER! " << *current_entry << std::endl;
						assert(current_entry->_position_on_trace < current_entry->_scan_position);
					}
					if (_path.count(current_entry->_position_on_trace) <= 0)
					{
						assert(_path.count(current_entry->_position_on_trace) > 0); // HERE! see below.
						std::cout << "DA! " << _path << std::endl;
					}
				#endif
				
				std::pair<D_Path::iterator, D_Path::iterator> iter_pair
					= _path.equal_range(current_entry->_position_on_trace);
				
				assert(iter_pair.second->first > current_entry->_position_on_trace); // def of equal_range!
				assert(iter_pair.first != iter_pair.second); //same as above. see HERE!
				
				D_Path::iterator iter = iter_pair.first;
				D_Path::iterator iter_end = iter_pair.second;
				while( (iter != iter_end) && (iter->second->_virtual_node_id
					!= current_entry->_virtual_node_id) )
				{
					++iter;
				}
				
				// the correspond virtual entry HAVE TO BE in the path.
				assert(iter->second->_virtual_node_id == current_entry->_virtual_node_id);
				
				PathEntry* corresponding_virtual_entry = iter->second;
				
				// some tests on the corresponding virtual entry.
				assert(corresponding_virtual_entry->_position_on_trace
					== current_entry->_position_on_trace);
				assert(corresponding_virtual_entry->_scan_position
					< current_entry->_scan_position);
				assert(!corresponding_virtual_entry->has_state(PathEntry::_VIRTUAL_FOUND));
				
				assert(corresponding_virtual_entry->has_state(PathEntry::_REAL));
				
				// merge makes _VIRTUAL_FOUND entries _REAL.
				assert(verify_node_bearings(corresponding_virtual_entry->_node_id));
				merge(node_ids_entries_mapper, current_entry,
					corresponding_virtual_entry);
				assert(corresponding_virtual_entry->_node_id == current_entry->_node_id);
				assert(verify_node_bearings(corresponding_virtual_entry->_node_id));
				
				assert(current_entry->has_state(PathEntry::_REAL));
			}
			
			assert(current_entry->has_state(PathEntry::_REAL));
			
			// Now connect the entries.
			PathEntry* previous_entry = current_entry->_backward_connection;
			if ( (!previous_entry->_do_not_use_connection)
				|| (_tile_manager->get_optimisation_mode()) )
			{
				// Test, if we should insert extra entries.
				bool insert_extra_nodes = false;
				if (true)
				{
					TileCache::Pointer previous_tile = _tile_cache->get(
						Node::tile_id(previous_entry->_node_id));
					const Node& previous_node = previous_tile->node(
						previous_entry->_node_id);
					
					insert_extra_nodes =
						(!previous_entry->is_virtual_predecessor_of(*current_entry))
						&& (!previous_node.has_successor(current_entry->_node_id));
				}
				
				// First connect...
				assert(verify_node_bearings(previous_entry->_node_id));
				assert(verify_node_bearings(current_entry->_node_id));
				connect_nodes(previous_entry->_node_id, current_entry->_node_id);
				assert(verify_node_bearings(previous_entry->_node_id));
				assert(verify_node_bearings(current_entry->_node_id));
				
				// ... then insert extra nodes if necessary.
				if (insert_extra_nodes)
				{
					// Interpolate nodes in between.
					
					// Calculate direction difference. If greater than
					// _MAX_CURVATURE we may need to interpolate new nodes.
/*					double direction_difference = 0.0;
					if ( (previous_entry != 0)
						&& (previous_entry->_backward_connection != 0) )
					{
						direction_difference
							= minimal_direction_difference_between(
								previous_entry->_backward_connection->_node_id,
								previous_entry->_node_id, current_entry->_node_id);
					}
					
					double steps_caused_by_angle
						= floor( direction_difference / (0.7 * _MAX_CURVATURE) );
					
					TileCache::Pointer previous_tile = _tile_cache->get(
						Node::tile_id(previous_entry->_node_id));
					const Node& previous_node = previous_tile->node(
						previous_entry->_node_id);
					
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(entry->_node_id));
					const Node& node = tile->node(entry->_node_id);
					double distance = previous_node.distance(node);
					
					double steps_caused_by_distance = ceil(
						(distance / (0.7 * _MIN_STEP_DISTANCE_M)) - 0.5 );
					
*/					
					
//					if (direction_difference > _MAX_CURVATURE)
//					{
						if (_tile_manager->get_optimisation_mode())
						{
							create_extra_nodes_on_connection(previous_entry,
								current_entry, 0.7 * _MIN_STEP_DISTANCE_M);
						} else
						{
							create_extra_nodes_on_connection(previous_entry,
								current_entry, _MIN_STEP_DISTANCE_M);
						}
						
						assert(verify_node_bearings(previous_entry->_node_id));
						assert(verify_node_bearings(current_entry->_node_id));
//					}
					
/*                                double direction_difference
                                        = minimal_direction_difference_between(previous_entry->_node_id,
                                                current_entry->_node_id, current_entry->_connection->_node_id);
                                double steps_caused_by_angle
                                        = floor( direction_difference / (0.7 * _MAX_CURVATURE) ); /////////////////////////////// USE CONFIGURATION VALUE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //                      std::cout << direction_difference << ", " << steps_caused_by_angle << std::endl;

                                double distance = n1.distance(n2);
                                double steps_caused_by_distance
                                                = ceil( (distance / (0.7 * _MIN_STEP_DISTANCE_M)) - 0.5 ); /////////////////////////////// USE CONFIGURATION VALUE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                                double steps = steps_caused_by_angle;
                                if (steps < steps_caused_by_distance)
                                        steps = steps_caused_by_distance;

                                if (distance / steps < 0.7 * _MIN_STEP_DISTANCE_M)
                                {
                                        steps = floor(distance / (0.7 * _MIN_STEP_DISTANCE_M));
                                }*/
				} // end if (insert_extra_nodes)
				
			} // end if (!previous_entry->_do_not_use_connection)
			
			// Update the *_entry pointers:
			current_entry = current_entry->_connection;
			
		} // end while
		
		return true;
	}
	
	
/*	bool
	TraceProcessor::apply_module()
	{
		assert(!_path.empty());
		
//		#ifdef DEBUG
//			std::cout << "apply_module:" << std::endl;
//			std::cout << _path << std::endl << std::endl;
//		#endif
				
		
		// The protocol contains all the information we need to merge the trace
		// to the existing map.
		
		// Find the beginning.
		D_Path::iterator iter = _path.begin();
		D_Path::iterator iter_end = _path.end();
		while ( (iter != iter_end) && (!iter->second->_is_beginning) )
			++iter;
		
		assert(iter != iter_end);
		assert(iter->second->_is_beginning);
		
//		if (iter == iter_end)
//		{
//			mlog(MLog::error, "TraceProcessor") << "protocol ends too early [1]\n";
//			return;
//		}
		
		if (iter->second->_score == _INVALID_PATH_SCORE)
		{
			mlog(MLog::warning, "TraceProcessor") << "Path has invalid path score. Discard this path!\n";
			
			_tile_manager->set_invalid_path_score();
			
//			std::cout << _path << std::endl << std::endl;
			
/*			for (int i = 0; i < _CONFIGURATION_VALUES; ++i)
			{
				std::cout << _conf._identifier_strings[i] << " = "
					<< _conf._values[i] << std::endl;
			}*************
			
			return false;
		}
		
		PathEntry* previous_entry = iter->second;
		PathEntry* current_entry = previous_entry->_connection;
		
		if (current_entry == 0)
		{
			assert(previous_entry->_is_beginning);
			assert(previous_entry->_is_destination);
			
			mlog(MLog::info, "TraceProcessor") << "Beginning == Destination\n";
//			mlog(MLog::debug, "TraceProcessor") << *previous_entry << "\n";
			
			return false;
		}
		
		assert(!previous_entry->has_state(PathEntry::_VIRTUAL_FOUND));
		
		// A map that maps Node::Ids to PathEntry. Needed when a Node
		// moves to another Tile.
		std::multimap<Node::Id, PathEntry*> node_ids_entries_mapper;
		iter = _path.begin();
		iter_end = _path.end();
		for (; iter != iter_end; ++iter)
		{
			PathEntry* entry = iter->second;
			if(entry->has_state(PathEntry::_REAL))
			{
				node_ids_entries_mapper.insert(
					std::make_pair(entry->_node_id, entry));
			}
		}
		
		// Merge previous entry. Current entry is merged within the loop.
		if (previous_entry->has_state(PathEntry::_REAL))
			merge(node_ids_entries_mapper, previous_entry);
		
		while (current_entry != 0)
		{
			// We need two real nodes when calling handle_connection (see
			// below). So, if previous_entry resp. current_entry are virtual we
			// register new nodes (here: take the _node_copys and insert them
			// into the tiles) .
			
			if (previous_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				// Only holds in the beginning of the _protocol, if at all.
				assert(previous_entry->_is_beginning);
				
				previous_entry->_node_id = register_new_node(
					_virtual_entries.point(static_cast<D_IndexType>(
						previous_entry->_virtual_node_id
						- _VIRTUAL_NODE_ID_OFFSET))
					);
				
				previous_entry->_state = PathEntry::_REAL;
				
				node_ids_entries_mapper.insert(
					std::make_pair(previous_entry->_node_id, previous_entry));
			}
			
			assert(previous_entry->has_state(PathEntry::_REAL));
			
			// Search the next real node.
			while (current_entry->has_state(PathEntry::_VIRTUAL_CREATED)
				&& (current_entry->_connection != 0))
			{
				current_entry = current_entry->_connection;
			}
			
			if (current_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				// Only holds if (current_entry->_connection == 0).
				// We are at the end of the whole _path. 
				assert(current_entry->_is_destination);
				
				current_entry->_node_id = register_new_node(
					_virtual_entries.point(static_cast<D_IndexType>(
						current_entry->_virtual_node_id
						- _VIRTUAL_NODE_ID_OFFSET))
					);
				
				current_entry->_state = PathEntry::_REAL;
				
				node_ids_entries_mapper.insert(
					std::make_pair(current_entry->_node_id, current_entry));
				
			} else if (current_entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
//				std::cout << "current_entry: " << *current_entry << std::endl;
				#ifndef NDEBUG
					if (current_entry->_position_on_trace >= current_entry->_scan_position)
					{
						std::cout << "path: " << _path << std::endl;
						std::cout << "HIER! " << *current_entry << std::endl;
						assert(current_entry->_position_on_trace < current_entry->_scan_position);
					}
					if (_path.count(current_entry->_position_on_trace) <= 0)
					{
						assert(_path.count(current_entry->_position_on_trace) > 0); // HERE! see below.
						std::cout << "DA! " << _path << std::endl;
					}
				#endif
				
				std::pair<D_Path::iterator, D_Path::iterator> iter_pair
					= _path.equal_range(current_entry->_position_on_trace);
				
				assert(iter_pair.second->first > current_entry->_position_on_trace); // def of equal_range!
				assert(iter_pair.first != iter_pair.second); //same as above. see HERE!
				
				D_Path::iterator iter = iter_pair.first;
				D_Path::iterator iter_end = iter_pair.second;
				while( (iter != iter_end) && (iter->second->_virtual_node_id
					!= current_entry->_virtual_node_id) )
				{
					++iter;
				}
				
				// the correspond virtual entry HAVE TO BE in the path.
				assert(iter->second->_virtual_node_id == current_entry->_virtual_node_id);
				
				PathEntry* corresponding_virtual_entry = iter->second;
				
				// some tests on the corresponding virtual entry.
				assert(corresponding_virtual_entry->_position_on_trace
					== current_entry->_position_on_trace);
				assert(corresponding_virtual_entry->_scan_position
					< current_entry->_scan_position);
				assert(!corresponding_virtual_entry->has_state(PathEntry::_VIRTUAL_FOUND));
				
				if (corresponding_virtual_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					// we encountered corresponding_virtual_entry while
					// searching the next not _VIRTUAL_CREATED entry.
					// So corresponding_virtual_entry is situated
					// between previous_entry and current_entry.
					// Tests for current_entry has already been done.
					assert(previous_entry->_position_on_trace
						< corresponding_virtual_entry->_position_on_trace);
					assert(previous_entry->_scan_position
						< corresponding_virtual_entry->_scan_position);
					
					corresponding_virtual_entry->_node_id = register_new_node(
						_virtual_entries.point(static_cast<D_IndexType>(
							corresponding_virtual_entry->_virtual_node_id
							- _VIRTUAL_NODE_ID_OFFSET))
						);
					
					corresponding_virtual_entry->_state = PathEntry::_REAL;
					
					node_ids_entries_mapper.insert(std::make_pair(
						corresponding_virtual_entry->_node_id,
						corresponding_virtual_entry));
					
					handle_connection(node_ids_entries_mapper,
						previous_entry, corresponding_virtual_entry);
					
					previous_entry = corresponding_virtual_entry;
				}
				
				assert(corresponding_virtual_entry->has_state(PathEntry::_REAL));
				
				// merge makes _VIRTUAL_FOUND entries _REAL.
				merge(node_ids_entries_mapper, current_entry,
					corresponding_virtual_entry);
				assert(current_entry->has_state(PathEntry::_REAL));
				
			} else // if (current_entry.has_state(PathEntry::_REAL))
			{
				merge(node_ids_entries_mapper, current_entry);
			}
			
			assert(previous_entry->has_state(PathEntry::_REAL));
			assert(current_entry->has_state(PathEntry::_REAL));
			
			handle_connection(node_ids_entries_mapper,
				previous_entry, current_entry);
			
			// Update the *_entry pointers:
			previous_entry = current_entry;
			current_entry = previous_entry->_connection;
			
		} // end while
		
		assert(previous_entry->_is_destination);
		
		return true;
	}*/
	
	
	double
	TraceProcessor::build_best_connections_recursively(
		D_Path::iterator start_iter,
		int similar_nodes_in_row, PathEntry::State similar_nodes_state/*,
		D_NodeIds& this_recursion_virtual_found_node_ids*/)
	{
		/*
		 * This is the recursive method used to scan the path for the
		 * optimal node connections.
		 */
		PathEntry* start_entry = start_iter->second;
		/*
		 * If the time_stamp equals time we already visited this entry and
		 * can immediatly return the stored points.
		 */
//		if (start_entry->_is_visited)
//			return start_entry->_score;
		
		/*
		 * Update the time_stamp.
		 */
//		start_entry->_is_visited = true;
		start_entry->_connection = 0;
		
		/*
		 * The destination is always the last path entry. If we have reached it
		 * we can return a great number of points for this path.
		 */
		if (start_entry->_is_destination)
		{
//			start_entry->_is_visited = true;
			start_entry->_score = _BEST_SCORE;
			
			return start_entry->_score;
		}
		
		/*
		 * Before further recursion we initialize the connection with zero and
		 * points with -100000.0, because we have not yet found anything
		 * usable.
		 */
		start_entry->_score = _WORST_SCORE;
		
		TileCache::Pointer start_entry_tile;
		const GeoCoordinate* start_entry_coordinate;
		if (start_entry->has_state(PathEntry::_REAL))
		{
			start_entry_tile = _tile_cache->get(
				Node::tile_id(start_entry->_node_id) );
			
			start_entry_coordinate
				= &(start_entry_tile->node(start_entry->_node_id));
		} else
		{
			start_entry_coordinate = &(_virtual_entries.point(
				static_cast<D_IndexType>(start_entry->_virtual_node_id 
					- _VIRTUAL_NODE_ID_OFFSET)));
		}
		
		/*
		 * Initialize the path iterators
		 */
		D_Path::iterator path_iter = start_iter;
		D_Path::iterator path_iter_end = _path.end();
		++path_iter;
		
		assert(path_iter != path_iter_end);
		
		bool destination_processed = false;
		while ((!destination_processed) && (path_iter->second->_scan_position
			- start_entry->_scan_position < _conf.get(_FAREST_START_POSITION)) )
		{
			PathEntry* path_entry = path_iter->second;
			
			TileCache::Pointer path_entry_tile;
			const GeoCoordinate* path_entry_coordinate;
			if (path_entry->has_state(PathEntry::_REAL))
			{
				path_entry_tile = _tile_cache->get(
					Node::tile_id(path_entry->_node_id) );
				
				path_entry_coordinate
					= &path_entry_tile->node(path_entry->_node_id);
			} else
			{
				path_entry_coordinate = &_virtual_entries.point(
					static_cast<D_IndexType>(path_entry->_virtual_node_id
						- _VIRTUAL_NODE_ID_OFFSET) );
			}
			
//			if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
//			{
//				this_recursion_virtual_found_node_ids.insert(
//					path_entry->_virtual_node_id);
//			}
			
			// Check if the conditions to enter this section are fulfilled.
			bool valid_virtual_found_entry = true; // That is just dummy. Remove it if you like!!!
			if (valid_virtual_found_entry &&
				(
					(!path_entry->has_state(PathEntry::_VIRTUAL_CREATED))
					|| (!start_entry->has_state(PathEntry::_VIRTUAL_CREATED))
					|| (path_entry->is_virtual_successor_of(*start_entry))
				)
			)
			{
				double score;
				if (path_entry->get_state() == similar_nodes_state)
				{
					score = build_best_connections_recursively( path_iter,
						similar_nodes_in_row + 1, path_entry->get_state()/*,
						this_recursion_virtual_found_node_ids*/ );
				} else
				{
					score = build_best_connections_recursively( path_iter,
						1, path_entry->get_state()/*,
						this_recursion_virtual_found_node_ids*/ );
				}
				
				// Let's calculate some values:
				double step_distance = start_entry_coordinate->distance(
					*path_entry_coordinate);
				
				double start_entry_relevant_position;
				if (start_entry->has_state(PathEntry::_REAL))
				{
					start_entry_relevant_position = start_entry->_scan_position;
				} else
				{
					start_entry_relevant_position = start_entry->_position_on_trace;
				}
				double start_point_distance_to_trace = start_entry_coordinate->
					distance(_filtered_trace.gps_point_at(
						start_entry_relevant_position));
				double squared_start_point_distance_to_trace
					= start_point_distance_to_trace * start_point_distance_to_trace;
				
				double path_entry_relevant_position;
				if (path_entry->has_state(PathEntry::_REAL))
				{
					path_entry_relevant_position = path_entry->_scan_position;
				} else
				{
					path_entry_relevant_position = path_entry->_position_on_trace;
				}
				double path_point_distance_to_trace = path_entry_coordinate->
					distance(_filtered_trace.gps_point_at(
						path_entry_relevant_position));
				double squared_path_point_distance_to_trace
					= path_point_distance_to_trace * path_point_distance_to_trace;
				
				Direction connection_direction(start_entry_coordinate->
					bearing(*path_entry_coordinate));
					
				double connection_direction_difference;
				if (start_entry->has_state(PathEntry::_REAL))
				{
					connection_direction_difference
						= dynamic_cast<const Node*>(start_entry_coordinate)->
							minimal_direction_difference_to(connection_direction);
				} else
				{
					connection_direction_difference = start_entry->_direction.
						angle_difference(connection_direction);
				}
				
				double connection_direction_next_difference;
				if (path_entry->has_state(PathEntry::_REAL))
				{
					connection_direction_next_difference
						= dynamic_cast<const Node*>(path_entry_coordinate)->
							minimal_direction_difference_to(connection_direction);
				} else
				{
					connection_direction_next_difference = path_entry->
						_direction.angle_difference(connection_direction);
				}
				
				// Check the states of the involved entry pair.
				if (start_entry->has_state(PathEntry::_REAL))
				{
					if (path_entry->has_state(PathEntry::_REAL))
					{
						if (dynamic_cast<const Node*>(start_entry_coordinate)
							->is_reachable(path_entry->_node_id))
						{
							score -= _conf.get(_BASE_SCORE_R2R) / similar_nodes_in_row;
						} else
						{
							score -= _conf.get(_BASE_SCORE_R2R);
							score -= _conf.get(_BASE_SCORE_R2R_NOT_REACHABLE);
//							score -= _conf.get(_BASE_SCORE_R2R_STEP_DISTANCE) * pow(step_distance, 1.5);
						}
						
						score -= _conf.get(_BASE_SCORE_R2R_STEP_DISTANCE) * pow(step_distance, 1.5);
						
					} else if (path_entry->has_state(PathEntry::_VIRTUAL_CREATED))
					{
						score -= _conf.get(_BASE_SCORE_R2VC);
						score -= _conf.get(_BASE_SCORE_R2VC_STEP_DISTANCE) * pow(step_distance, 1.5);
					} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
					{
						score -= _conf.get(_BASE_SCORE_R2VF);
						score -= _conf.get(_BASE_SCORE_R2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
					}
					
				} else if (start_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					if (path_entry->has_state(PathEntry::_REAL))
					{
						score -= _conf.get(_BASE_SCORE_VC2R);
						score -= _conf.get(_BASE_SCORE_VC2R_STEP_DISTANCE) * pow(step_distance, 1.5);
					} else if (path_entry->has_state(PathEntry::_VIRTUAL_CREATED))
					{
						if (path_entry->is_virtual_successor_of(*start_entry))
						{
							score -= _conf.get(_BASE_SCORE_VC2VC) / sqrt(similar_nodes_in_row);
						} else
						{
							score -= _conf.get(_BASE_SCORE_VC2VC);
							score -= _conf.get(_BASE_SCORE_VC2VC_NO_SUCCESSOR);
						}
						
						score -= _conf.get(_BASE_SCORE_VC2VC_STEP_DISTANCE) * pow(step_distance, 1.5);
						
					} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
					{
						score -= _conf.get(_BASE_SCORE_VC2VF);
						score -= _conf.get(_BASE_SCORE_VC2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
					}
					
				} else // if (start_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					if (path_entry->has_state(PathEntry::_REAL))
					{
						score -= _conf.get(_BASE_SCORE_VF2R);
						score -= _conf.get(_BASE_SCORE_VF2R_STEP_DISTANCE) * pow(step_distance, 1.5);
					} else if (path_entry->has_state(PathEntry::_VIRTUAL_CREATED))
					{
						score -= _conf.get(_BASE_SCORE_VF2VC);
						score -= _conf.get(_BASE_SCORE_VF2VC_STEP_DISTANCE) * pow(step_distance, 1.5);
					} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
					{
						if (path_entry->is_virtual_successor_of(*start_entry))
						{
							score -= _conf.get(_BASE_SCORE_VF2VF) / similar_nodes_in_row;
						} else
						{
							score -= _conf.get(_BASE_SCORE_VF2VF);
							score -= _conf.get(_BASE_SCORE_VF2VF_NO_SUCCESSOR);
						}
						
						score -= _conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
					}
				}
				
//				score -= _conf.get(_BASE_SCORE_PERPENDULAR_DISTANCE) * squared_path_point_distance_to_trace;
				score -= _conf.get(_BASE_SCORE_DIRECTION_DIFFERENCE) * connection_direction_difference;
				score -= _conf.get(_BASE_SCORE_NEXT_DIRECTION_DIFFERENCE) * connection_direction_next_difference;
	
	/*			
				// Negative points for:				
				// Jump from virtual node to existing node.
				if ((start_entry->is_virtual()) && (!path_entry->is_virtual()))
				{
					points -= distance_to_trace * 5.0;
					points -= pow(step_distance, 1.5);
					points -= 50.0;
					points -= connection_direction_difference * 100.0;
					points -= connection_direction_next_difference * 100.0;
				}
					
				// Jump from existing node to virtual node.
				if ((!start_entry->is_virtual()) && (path_entry->is_virtual()))
				{
					points -= step_distance * 5.0;
					points -= 100.0;
					points -= connection_direction_difference * 100.0;
					points -= connection_direction_next_difference * 100.0;
				}
				
				// Jump from existing node to existing node, 
				if ((!start_entry->is_virtual()) && (!path_entry->is_virtual()))
				{
					// without connection
					if (!start_entry->_node_copy.is_reachable(path_entry->_node_id))
					{
						points -= step_distance * 5.0;
						points -= 50.0;
						points -= connection_direction_difference * 100.0;
						points -= connection_direction_next_difference * 100.0;
					} else //with connection
					{
						points -= distance_to_trace * 0.5;
						points -= step_distance * 0.5;
						points -= connection_direction_difference * 50.0;
						points -= connection_direction_next_difference * 50.0;
					}
				}
					
				// Jump from virtual node to virtual node.
				if ((start_entry->is_virtual()) && (path_entry->is_virtual()))
				{
					points -= step_distance * 1.5; // / similar_nodes_in_row;
				}
	*/
				
				if (score > start_entry->_score)
				{
					start_entry->_score = score;
					start_entry->_connection = path_entry;
				}
				
				if (path_entry->_is_destination)
					destination_processed = true;
			
			} // end the create if-clause that checks the "great" condition.
			
			++path_iter;
		}
		
		/*
		 * Return the optimal number of points we could reach in this path.
		 */
		return start_entry->_score;
	}
	
	
	void
	TraceProcessor::build_crossings_mapper(
		std::multimap<Node::Id, CrossingItem>& crossings_mapper,
		const std::map<Node::Id, CrossingItem>& excluded_crossings,
		bool do_not_verify) const
	{
		crossings_mapper.clear();
		
		D_Path::const_iterator iter = _path.begin();
		D_Path::const_iterator iter_end = _path.end();
		while ( (iter != iter_end) && (!iter->second->_is_beginning) )
			++iter;
		
		if (iter == iter_end)
			return;
		
//		PathEntry* previous_entry = iter->second;
		PathEntry* entry = iter->second;
		while (entry != 0)
		{
			if ( (entry->has_state(PathEntry::_REAL))
				&& (!entry->_do_not_use_connection) )
			{
				TileCache::Pointer tile = _tile_cache->get(
					Node::tile_id(entry->_node_id));
				const Node& node = tile->node(entry->_node_id);
				
				int predecessors = node.predecessors().size();
				int successors = node.successors().size();
				if ( (predecessors > 1) || (successors > 1) )
				{
					std::map<Node::Id, CrossingItem>::const_iterator find_iter
						= excluded_crossings.find(entry->_node_id);
					
					if (find_iter != excluded_crossings.end())
					{
						assert(predecessors >= find_iter->second._predecessors);
						assert(successors >= find_iter->second._successors);
						
						if ( (predecessors > find_iter->second._predecessors)
							|| (successors > find_iter->second._successors) )
						{
							CrossingItem crossing_item;
							crossing_item._predecessors = predecessors;
							crossing_item._successors = successors;
							crossing_item._has_new_predecessors
								= (predecessors > find_iter->second._predecessors);
							crossing_item._has_new_successors
								= (successors > find_iter->second._successors);
//							crossing_item._previous_entry = previous_entry;
							crossing_item._entry = entry;
							
							crossings_mapper.insert( std::make_pair(
								entry->_node_id, crossing_item) );
						}
						
					} else
					{
						CrossingItem crossing_item;
						crossing_item._predecessors = predecessors;
						crossing_item._successors = successors;
						crossing_item._has_new_predecessors = (predecessors > 1);
						crossing_item._has_new_successors = (successors > 1);
//						crossing_item._previous_entry = previous_entry;
						crossing_item._entry = entry;
						
						crossings_mapper.insert( std::make_pair(
							entry->_node_id, crossing_item) );
					}
				}
			}
			
//			previous_entry = entry;
			entry = entry->_connection;
		} // end while (entry != 0)
		
		#ifndef NDEBUG
			if (!do_not_verify)
			{
				// Verify it.
				std::multimap<Node::Id, CrossingItem>::const_iterator test_iter
					= crossings_mapper.begin();
				std::multimap<Node::Id, CrossingItem>::const_iterator test_iter_end
					= crossings_mapper.end();
				for (; test_iter != test_iter_end; ++test_iter)
				{
					assert(test_iter->first == test_iter->second._entry->_node_id);
					
					PathEntry* test_entry = test_iter->second._entry;
					assert(test_entry != 0);
					assert(!test_entry->_do_not_use_connection);
					
					TileCache::Pointer test_tile = _tile_cache->get(
						Node::tile_id(test_entry->_node_id));
					const Node& test_node = test_tile->node(test_entry->_node_id);
					
					if (!test_entry->_is_beginning)
					{
						assert(test_entry->_backward_connection != 0);
						if (!test_entry->_backward_connection->_do_not_use_connection)
						{
							assert(test_node.has_predecessor(
								test_entry->_backward_connection->_node_id) );
						}
					}
					
					if (!test_entry->_is_destination)
					{
						assert(test_entry->_connection != 0);
						assert(test_node.has_successor(
							test_entry->_connection->_node_id));
					}
				}
			}
		#endif
	}
	
	
	void
	TraceProcessor::calculate_cluster_nodes(const GeoCoordinate& start_gc,
		const GeoCoordinate& end_gc, std::list<Node::Id>& result_list) const
	{
		result_list.clear();
		
		// Search the real nodes in the various tiles.
		rangereporting::Segment<GeoCoordinate> segment;
		segment.set_point(0, start_gc);
		segment.set_point(1, end_gc);
		
		const double search_max_distance_m = _conf.get(_SEARCH_MAX_DISTANCE_M);
		const double search_max_angle_difference_pi
			= _conf.get(_SEARCH_MAX_ANGLE_DIFFERENCE_PI);
		
		std::vector<Tile::Id> needed_tile_ids = Tile::get_needed_tile_ids(
			start_gc, end_gc, search_max_distance_m * 1.1);
		std::vector<Tile::Id>::iterator tile_iter = needed_tile_ids.begin();
		std::vector<Tile::Id>::iterator tile_iter_end = needed_tile_ids.end();
		
		for (; tile_iter != tile_iter_end; ++tile_iter)
		{
			// Load the tiles or create a new one if none could be loaded. *
			TileCache::Pointer tile = _tile_cache->get(*tile_iter);
			if (tile == 0)
			{
				_tile_cache->insert(*tile_iter, new Tile(*tile_iter));
				tile = _tile_cache->get(*tile_iter);
			}
			_protocol._used_tile_ids.insert(tile->get_id());
			
			std::vector<Node::Id> cluster_nodes;
			tile->fast_cluster_nodes_search(segment, search_max_distance_m,
				search_max_angle_difference_pi * PI, cluster_nodes);
			
			result_list.insert(result_list.end(), cluster_nodes.begin(),
				cluster_nodes.end());
		}
		
		// Now search the _virtual_entries.
		Segment<VirtualNode> quadrangle_segment;
		
		VirtualNode left_point(start_gc);
		quadrangle_segment.set_point(0, left_point);
		VirtualNode right_point(end_gc);
		quadrangle_segment.set_point(1, right_point);
		
		rangereporting::Quadrangle<VirtualNode> quadrangle;
		quadrangle.build_quadrangle(quadrangle_segment, search_max_distance_m,
			0.01, search_max_distance_m);
		
		std::vector<D_IndexType> cluster_nodes;
		_virtual_entries.range_query(quadrangle, cluster_nodes);

		std::vector<D_IndexType>::iterator ids_iter = cluster_nodes.begin();
		std::vector<D_IndexType>::iterator ids_iter_end = cluster_nodes.end();
		for (; ids_iter != ids_iter_end; ++ids_iter)
		{
			// Calculate the direction difference of the input segment and
			// the found virtual node.
			mapgeneration_util::Direction segment_direction(
				quadrangle_segment.get_point(0).bearing(
				quadrangle_segment.get_point(1)));
			double min_direction_difference = _virtual_entries.point(*ids_iter).
				angle_difference(segment_direction);
			
			if (min_direction_difference
				<= _conf.get(_SEARCH_MAX_ANGLE_DIFFERENCE_PI) * PI)
			{
				result_list.push_back(static_cast<Node::Id>(*ids_iter)
					+ _VIRTUAL_NODE_ID_OFFSET);
/*			} else
			{
				std::cout << "Discard this point! (" << *ids_iter
					<< ", dir = " << _virtual_entries.point(*ids_iter).get_direction()*(180.0/PI)
					<< ", seg_dir = " << segment_direction.get_direction()*(180.0/PI)
					<< ", min_dir_diff = " << min_direction_difference*(180.0/PI)
					<< ")" << std::endl;
*/			}
		}
	}
	
	
	double
	TraceProcessor::calculate_connection_score(PathEntry* start_entry,
		PathEntry* end_entry)
	{
		if (end_entry->_score < start_entry->_score)
		{
			return _INVALID_PATH_SCORE;
		}
		
		if ( (start_entry->_state == PathEntry::_REAL)
			&& (end_entry->_state == PathEntry::_REAL) )
		{
			if (start_entry->_node_id == end_entry->_node_id)
				return _INVALID_PATH_SCORE;
		}
		
		if ( (start_entry->_state != PathEntry::_REAL)
			&& (end_entry->_state != PathEntry::_REAL) )
		{
			if (start_entry->_virtual_node_id == end_entry->_virtual_node_id)
				return _INVALID_PATH_SCORE;
		}
		
		TileCache::Pointer start_entry_tile;
		const GeoCoordinate* start_entry_coordinate;
		if (start_entry->has_state(PathEntry::_REAL))
		{
			start_entry_tile = _tile_cache->get(
				Node::tile_id(start_entry->_node_id) );
			
			start_entry_coordinate
				= &(start_entry_tile->node(start_entry->_node_id));
		} else
		{
			start_entry_coordinate = &(_virtual_entries.point(
				static_cast<D_IndexType>(start_entry->_virtual_node_id 
					- _VIRTUAL_NODE_ID_OFFSET)));
		}
		
		TileCache::Pointer end_entry_tile;
		const GeoCoordinate* end_entry_coordinate;
		if (end_entry->has_state(PathEntry::_REAL))
		{
			end_entry_tile = _tile_cache->get(
				Node::tile_id(end_entry->_node_id) );
			
			end_entry_coordinate
				= &(end_entry_tile->node(end_entry->_node_id));
		} else
		{
			end_entry_coordinate = &(_virtual_entries.point(
				static_cast<D_IndexType>(end_entry->_virtual_node_id 
					- _VIRTUAL_NODE_ID_OFFSET)));
		}
		
		// Let's calculate some values:
		double step_distance = start_entry_coordinate->distance(
			*end_entry_coordinate);
		
		if (step_distance > _conf.get(_FAREST_START_POSITION))
		{
			return _INVALID_PATH_SCORE;
		}
		
		if ( (!start_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			|| (!end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			|| (start_entry->is_virtual_predecessor_of(*end_entry))
		)
		{
			
			
			
			double start_entry_relevant_position;
			if (start_entry->has_state(PathEntry::_REAL))
			{
				start_entry_relevant_position = start_entry->_scan_position;
			} else
			{
				start_entry_relevant_position = start_entry->_position_on_trace;
			}
			double start_point_distance_to_trace = start_entry_coordinate->
				distance(_filtered_trace.gps_point_at(
					start_entry_relevant_position));
/*			double squared_start_point_distance_to_trace
				= start_point_distance_to_trace * start_point_distance_to_trace;
			
			double end_entry_relevant_position;
			if (end_entry->has_state(PathEntry::_REAL))
			{
				end_entry_relevant_position = end_entry->_scan_position;
			} else
			{
				end_entry_relevant_position = end_entry->_position_on_trace;
			}
			double end_point_distance_to_trace = end_entry_coordinate->
				distance(_filtered_trace.gps_point_at(
					end_entry_relevant_position));
			double squared_end_point_distance_to_trace
				= end_point_distance_to_trace * end_point_distance_to_trace;
*/			
			Direction connection_direction(start_entry_coordinate->
				bearing(*end_entry_coordinate));
				
			double connection_direction_difference = 0.0;
			if (start_entry->has_state(PathEntry::_REAL))
			{
				connection_direction_difference
					= dynamic_cast<const Node*>(start_entry_coordinate)->
						minimal_direction_difference_to(connection_direction);
			} else
			{
				connection_direction_difference = start_entry->_direction.
					angle_difference(connection_direction);
			}
			
/*			double connection_direction_next_difference;
			if (end_entry->has_state(PathEntry::_REAL))
			{
				connection_direction_next_difference
					= dynamic_cast<const Node*>(end_entry_coordinate)->
						minimal_direction_difference_to(connection_direction);
			} else
			{
				connection_direction_next_difference = end_entry->
					_direction.angle_difference(connection_direction);
			}*/
			
			// Calculate the score.
			double score = end_entry->_score;
			
			// Check the states of the involved entry pair.
/*			if (start_entry->has_state(PathEntry::_REAL))
			{
				if (end_entry->has_state(PathEntry::_REAL))
				{
					if (dynamic_cast<const Node*>(start_entry_coordinate)
						->is_predecessor_of(end_entry->_node_id))
					{
						double similar_nodes_in_row
							= end_entry->_equal_state_successors_count;
						if (similar_nodes_in_row > 10)
							similar_nodes_in_row = 10;
						
						score -= _conf.get(_BASE_SCORE_R2R)
							/ similar_nodes_in_row;
					} else
					{
						score -= _conf.get(_BASE_SCORE_R2R);
						score -= _conf.get(_BASE_SCORE_R2R_NOT_REACHABLE);
					}
					
					score -= _conf.get(_BASE_SCORE_R2R_STEP_DISTANCE) * pow(step_distance, 1.5);
					
				} else if (end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					score -= _conf.get(_BASE_SCORE_R2VC);
					score -= _conf.get(_BASE_SCORE_R2VC_STEP_DISTANCE) * pow(step_distance, 1.5);
				} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					score -= _conf.get(_BASE_SCORE_R2VF);
					score -= _conf.get(_BASE_SCORE_R2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
				}
				
			} else if (start_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				if (end_entry->has_state(PathEntry::_REAL))
				{
					score -= _conf.get(_BASE_SCORE_VC2R);
					score -= _conf.get(_BASE_SCORE_VC2R_STEP_DISTANCE) * pow(step_distance, 1.5);
				} else if (end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					if (start_entry->is_virtual_predecessor_of(*end_entry))
					{
						double similar_nodes_in_row
							= end_entry->_equal_state_successors_count;
						if (similar_nodes_in_row > 10)
							similar_nodes_in_row = 10;
						
						score -= _conf.get(_BASE_SCORE_VC2VC)
							/ sqrt(similar_nodes_in_row);
					} else
					{
						score -= _conf.get(_BASE_SCORE_VC2VC);
						score -= _conf.get(_BASE_SCORE_VC2VC_NO_SUCCESSOR);
					}
					
					score -= _conf.get(_BASE_SCORE_VC2VC_STEP_DISTANCE) * pow(step_distance, 1.5);
					
				} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					score -= _conf.get(_BASE_SCORE_VC2VF);
					score -= _conf.get(_BASE_SCORE_VC2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
				}
				
			} else // if (start_entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
				if (end_entry->has_state(PathEntry::_REAL))
				{
					score -= _conf.get(_BASE_SCORE_VF2R);
					score -= _conf.get(_BASE_SCORE_VF2R_STEP_DISTANCE) * pow(step_distance, 1.5);
				} else if (end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					score -= _conf.get(_BASE_SCORE_VF2VC);
					score -= _conf.get(_BASE_SCORE_VF2VC_STEP_DISTANCE) * pow(step_distance, 1.5);
				} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					if (start_entry->is_virtual_predecessor_of(*end_entry))
					{
						double similar_nodes_in_row
							= end_entry->_equal_state_successors_count;
						if (similar_nodes_in_row > 10)
							similar_nodes_in_row = 10;
						
						score -= _conf.get(_BASE_SCORE_VF2VF)
							/ similar_nodes_in_row;
					} else if (start_entry->_virtual_node_id < end_entry->_virtual_node_id)
					{
						if ( (end_entry->_virtual_node_id - start_entry->_virtual_node_id)
							* _conf.get(_STEP_DISTANCE_M) > _conf.get(_FAREST_START_POSITION) )
						{
							#warning use a variable here?!
//							score += _conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
						}
					} else
					{
						score -= _conf.get(_BASE_SCORE_VF2VF);
						score -= _conf.get(_BASE_SCORE_VF2VF_NO_SUCCESSOR);
					}
					
					score -= _conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) * pow(step_distance, 1.5);
				}
			}*/
			
//			score -= _conf.get(_BASE_SCORE_PERPENDULAR_DISTANCE) * squared_path_point_distance_to_trace;
//			score -= _conf.get(_BASE_SCORE_DIRECTION_DIFFERENCE) * connection_direction_difference;
//			score -= _conf.get(_BASE_SCORE_NEXT_DIRECTION_DIFFERENCE) * connection_direction_next_difference;
			
//			score -= pow( (_conf.get(_BASE_SCORE_PERPENDULAR_DISTANCE) * start_point_distance_to_trace), 1.6);
//			score -= pow( (_conf.get(_BASE_SCORE_NEXT_DIRECTION_DIFFERENCE) * start_point_distance_to_trace), 1.0);
			#warning Rename that variable!!!
			
			//Abstand
			if (step_distance > 7.0)
			{
//				score -= pow( (_conf.get(_BASE_SCORE_DIRECTION_DIFFERENCE) * start_point_distance_to_trace),
//					_conf.get(_BASE_SCORE_NEXT_DIRECTION_DIFFERENCE) / 30.0);
			} else
			{
//				score -= pow( (_conf.get(_BASE_SCORE_DISTANT_START) * start_point_distance_to_trace),
//					_conf.get(_BASE_SCORE_PERPENDULAR_DISTANCE) / 30.0);
			}
			
			//Winkel
			if (step_distance > 7.0)
			{
//				score -= pow( (_conf.get(_BASE_SCORE_R2R) * connection_direction_difference),
//					_conf.get(_BASE_SCORE_R2VC) / 30.0);
			} else
			{
//				score -= pow( (_conf.get(_BASE_SCORE_R2VF) * connection_direction_difference),
//					_conf.get(_BASE_SCORE_VC2R) / 30.0);
			}
			
			double similar_nodes_in_row
				= end_entry->_equal_state_successors_count;
			if (similar_nodes_in_row > 1.0)
			{
				if (step_distance < 10.0)
				{
//					similar_nodes_in_row = 1.0;
					if (similar_nodes_in_row > 2.0)
						similar_nodes_in_row = 2.0;
				} else
				{
					if (similar_nodes_in_row > 3.0)
						similar_nodes_in_row = 3.0;
				}
			}
			
			if (start_entry->has_state(PathEntry::_REAL))
			{
				if (end_entry->has_state(PathEntry::_REAL))
				{
					if (dynamic_cast<const Node*>(start_entry_coordinate)
						->has_successor(end_entry->_node_id))
					{
						score -= pow(_conf.get(_BASE_SCORE_R2R_STEP_DISTANCE) * step_distance, 1.5)
							/ similar_nodes_in_row;
					} else
					{
						score -= pow( (_conf.get(_BASE_SCORE_R2R_STEP_DISTANCE) + _conf.get(_BASE_SCORE_R2R_NOT_REACHABLE)) * step_distance, 1.5);
					}
					
				} else if (end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					score -= pow(_conf.get(_BASE_SCORE_R2VC_STEP_DISTANCE) * step_distance, 1.5);
				} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					score -= pow(_conf.get(_BASE_SCORE_R2VF_STEP_DISTANCE) * step_distance, 1.5);
				}
				
			} else if (start_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				if (end_entry->has_state(PathEntry::_REAL))
				{
					score -= pow(_conf.get(_BASE_SCORE_VC2R_STEP_DISTANCE) * step_distance, 1.5);
				} else if (end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					assert(start_entry->is_virtual_predecessor_of(*end_entry));
					
//					if (start_entry->is_virtual_predecessor_of(*end_entry))
//					{
						score -= pow(_conf.get(_BASE_SCORE_VC2VC_STEP_DISTANCE) * step_distance, 1.5)
							/ /*sqrt*/(similar_nodes_in_row);
//					} else
//					{
//						score -= pow( (_conf.get(_BASE_SCORE_VC2VC_STEP_DISTANCE) + _conf.get(_BASE_SCORE_VC2VC_NO_SUCCESSOR)) * step_distance, 1.5);
//					}
					
				} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					score -= pow(_conf.get(_BASE_SCORE_VC2VF_STEP_DISTANCE) * step_distance, 1.5);
				}
				
			} else // if (start_entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
				if (end_entry->has_state(PathEntry::_REAL))
				{
					score -= pow(_conf.get(_BASE_SCORE_VF2R_STEP_DISTANCE) * step_distance, 1.5);
				} else if (end_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					score -= pow(_conf.get(_BASE_SCORE_VF2VC_STEP_DISTANCE) * step_distance, 1.5);
				} else // if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					if (start_entry->is_virtual_predecessor_of(*end_entry))
					{
						score -= pow(_conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) * step_distance, 1.5)
							/ similar_nodes_in_row;
					} else
					{
						#warning Change something here!
						// Hier: Kreisverkehrraudierkennung (TM) :-)
						// Mehrfach im Kreis, d.h. start-end >_FAR, aber nicht viel!
						// -> hohe extra score
						// für start-end >> _FAR, nicht so hoch, um evtl. REAL Knoten nicht auszubremsen!
						//
						// Für die KVRE kann ich die step_distance einschränken auf vllt 15m.
						if ( (start_entry->_position_on_trace - end_entry->_position_on_trace)
							> _conf.get(_FAREST_START_POSITION) )
						{
							score -= pow(_conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) * step_distance, 1.5);
//							#warning use another variable here?!
//							score += (start_entry->_position_on_trace - end_entry->_position_on_trace) * _conf.get(_BASE_SCORE_VF2VF);
//							score += pow( (_conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) + _conf.get(_BASE_SCORE_VF2VF_NO_SUCCESSOR) /*+ _conf.get(_BASE_SCORE_VF2VF)*/) * step_distance, 1.5);
//							score -= pow( (_conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) + _conf.get(_BASE_SCORE_VF2VF_NO_SUCCESSOR)) * step_distance, 1.5);
						} else
						{
							score -= pow( (_conf.get(_BASE_SCORE_VF2VF_STEP_DISTANCE) + _conf.get(_BASE_SCORE_VF2VF_NO_SUCCESSOR)) * step_distance, 1.5);
						}
					}
					score += (1.0 - end_entry->_position_on_trace/_filtered_trace.length_m()) * 20.0 * _conf.get(_BASE_SCORE_VF2VF);
//					score += (_filtered_trace.length_m() - end_entry->_position_on_trace) * 
				}
			}
			return score;
			
		} else
		{
			return _INVALID_PATH_SCORE;
		}
	}
	
	
	void
	TraceProcessor::connect_nodes(Node::Id from_node_id, Node::Id to_node_id)
	{
		assert(from_node_id != to_node_id);
		
		// Calculate bearing.
		double direction = PI;
		if (true)
		{
			TileCache::Pointer from_node_tile
				= _tile_cache->get(Node::tile_id(from_node_id));
			const Node& from_node = from_node_tile->node(from_node_id);
			
			TileCache::Pointer to_node_tile
				= _tile_cache->get(Node::tile_id(to_node_id));
			const Node& to_node = to_node_tile->node(to_node_id);
			
			direction = from_node.bearing(to_node);
		}
		
		// Add successor.
		if (true)
		{
			TileCache::Pointer from_node_tile
				= _tile_cache->get(Node::tile_id(from_node_id));
			Node& from_node = from_node_tile.write().node(from_node_id);
			
			from_node.add_successor(to_node_id, direction);
		}
		
		// Add predecessor.
		if (true)
		{
			TileCache::Pointer to_node_tile
				= _tile_cache->get(Node::tile_id(to_node_id));
			Node& to_node = to_node_tile.write().node(to_node_id);
		
			to_node.add_predecessor(from_node_id, direction);
		}
	}
	
	
	bool
	TraceProcessor::consistency_check(TileCache::Pointer pointer) const
	{
		assert(false);
		#ifndef NDEBUG
			// Check existence for various nodes. Randomly.
			for (Node::LocalId i = 0; i < 1000; ++i)
				pointer->exists_node(Node::LocalId(rand()));
			
			// Test for every node if the successors resp predecessors exist.
			for (Node::LocalId i = 0; i < pointer->nodes().size_including_holes(); ++i)
			{
				assert(pointer->exists_node(i) == pointer->nodes()[i].first);
				if(pointer->exists_node(i))
				{
					Node::D_Edges::const_iterator pred_iter
						= pointer->node(i).predecessors().begin();
					Node::D_Edges::const_iterator pred_iter_end
						= pointer->node(i).predecessors().end();
					for (; pred_iter != pred_iter_end; ++pred_iter)
					{
						TileCache::Pointer test_tile = _tile_cache->get(
							Node::tile_id(pred_iter->get_next_node_id()) );
						assert(test_tile->exists_node(pred_iter->get_next_node_id()));
						
						const Node& test_node = test_tile->node(pred_iter->get_next_node_id());
						assert(test_node[0] > 30.0);
						assert(test_node[1] > 3.0);
						assert(test_node.predecessors().size() < 10);
						assert(test_node.successors().size() < 10);
					}
					
					Node::D_Edges::const_iterator succ_iter
						= pointer->node(i).successors().begin();
					Node::D_Edges::const_iterator succ_iter_end
						= pointer->node(i).successors().end();
					for (; succ_iter != succ_iter_end; ++succ_iter)
					{
						TileCache::Pointer test_tile = _tile_cache->get(
							Node::tile_id(succ_iter->get_next_node_id()) );
						assert(test_tile->exists_node(succ_iter->get_next_node_id()));
						
						const Node& test_node = test_tile->node(succ_iter->get_next_node_id());
						assert(test_node[0] > 30.0);
						assert(test_node[1] > 3.0);
						assert(test_node.predecessors().size() < 10);
						assert(test_node.successors().size() < 10);
					}
				}
			}
		#endif
		
		return true;
	}
	
	Node::Id
	TraceProcessor::create_extra_nodes_on_connection(
		const Node::Id from_node_id, const Node::Id to_node_id,
		double max_step_distance_m)
	{
		assert(verify_node_bearings(from_node_id));
		assert(verify_node_bearings(to_node_id));
		
		assert(max_step_distance_m > 0.0);
		
		// Init the return value.
		Node::Id first_successor = to_node_id;
		
		// Get the nodes.
		TileCache::Pointer from_tile = _tile_cache->get(
			Node::tile_id(from_node_id));
		const Node from_node = from_tile->node(from_node_id); // NO reference here!!!
		
		TileCache::Pointer to_tile = _tile_cache->get(
			Node::tile_id(to_node_id));
		const Node to_node = to_tile->node(to_node_id); // NO reference here!!!
		
		assert(from_node.has_successor(to_node_id));
		assert(to_node.has_predecessor(from_node_id));
		
		assert(from_node.successor(to_node_id).get_direction()
			== Direction(from_node.bearing(to_node)));
		
		// Calculate distance and interpolate if necessary.
		double connection_distance = from_node.distance(to_node);
		if (connection_distance > max_step_distance_m)
		{
			// connection_distance too large.
			// Delete connection...
			assert(verify_node_bearings(from_node_id));
			assert(verify_node_bearings(to_node_id));
			disconnect_nodes(from_node_id, to_node_id);
			assert(verify_node_bearings(from_node_id));
			assert(verify_node_bearings(to_node_id));
			
			// ... and insert interpolated nodes.
			int new_nodes = static_cast<int>(
				floor(connection_distance / max_step_distance_m)) - 1;
			
			Node::Id previous_node_id = from_node_id;
			for (int i = 0; i < new_nodes; ++i)
			{
				// Interpolate node.
				double weight = 1.0 -
					(static_cast<double>(i+1) / static_cast<double>(new_nodes+1));
				GeoCoordinate interpolated_gc
					= GeoCoordinate::interpolate(from_node, to_node, weight);
				
				Node::Id current_node_id = register_new_node(interpolated_gc);
				
				assert(verify_node_bearings(previous_node_id));
				assert(verify_node_bearings(current_node_id));
				connect_nodes(previous_node_id, current_node_id);
				assert(verify_node_bearings(previous_node_id));
				assert(verify_node_bearings(current_node_id));

				// Increment previous_node_id
				previous_node_id = current_node_id;
				
				if (i == 0)
					first_successor = current_node_id;
			}
			
			// connect previous_node to to_node
			assert(verify_node_bearings(previous_node_id));
			assert(verify_node_bearings(to_node_id));
			connect_nodes(previous_node_id, to_node_id);
			assert(verify_node_bearings(previous_node_id));
			assert(verify_node_bearings(to_node_id));
		}
		
		return first_successor;
	}
	
	
	void
	TraceProcessor::create_extra_nodes_on_connection(
		PathEntry* const from_entry, PathEntry* const to_entry,
		double max_step_distance_m)
	{
		assert(from_entry->_connection == to_entry);
		assert(from_entry->has_state(PathEntry::_REAL));
		assert(to_entry->has_state(PathEntry::_REAL));
		
		assert(max_step_distance_m > 0.0);
		
		const Node::Id first_successor = create_extra_nodes_on_connection(
			from_entry->_node_id, to_entry->_node_id, max_step_distance_m);
		
		std::vector<PathEntry*> new_entries;
		new_entries.push_back(from_entry);
		
		Node::Id current_node_id = first_successor;
		while (current_node_id != to_entry->_node_id)
		{
			// Create new PathEntry.
			PathEntry* new_entry = new PathEntry;
			new_entry->_is_interpolated = true;
			new_entry->_node_id = current_node_id;
			new_entry->_state = PathEntry::_REAL;
			
			_path.insert(std::make_pair(
				new_entry->_scan_position, new_entry));
			new_entries.push_back(new_entry);
			
			// Increment current_node_id
			TileCache::Pointer current_tile = _tile_cache->get(
				Node::tile_id(current_node_id));
			const Node& current_node = current_tile->node(current_node_id);
			
			assert(current_node.successors().size() == 1);
			
			current_node_id
				= current_node.successors().front().get_next_node_id();
		}
		
		new_entries.push_back(to_entry);
			
		// Connect the new entries.
		for (int i = 0; i < new_entries.size(); ++i)
		{
			if (i > 0)
				new_entries[i]->_backward_connection = new_entries[i-1];
			
			if (i < new_entries.size() - 1)
				new_entries[i]->_connection = new_entries[i+1];
			
			assert(verify_node_bearings(new_entries[i]->_node_id));
		}
	}
	
	
	void
	TraceProcessor::create_extra_nodes_on_connections(
		const std::multimap<Node::Id, CrossingItem>& crossings_mapper)
	{
		std::set<Node::Id> created_extra_nodes_here;
		
		std::multimap<Node::Id, CrossingItem>::const_iterator iter
			= crossings_mapper.begin();
		std::multimap<Node::Id, CrossingItem>::const_iterator iter_end
			= crossings_mapper.end();
		for (; iter != iter_end; ++iter)
		{
			assert(iter->first == iter->second._entry->_node_id);
			assert(iter->second._entry->has_state(PathEntry::_REAL));
			
			PathEntry* const entry = iter->second._entry;
			
			std::set<Node::Id>::const_iterator find_iter
				= created_extra_nodes_here.find(iter->first);
			if (find_iter == created_extra_nodes_here.end())
			{
				// calculate the minimal distance.
				double min_distance = _conf.get(_STEP_DISTANCE_M);
				if (true)
				{
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(entry->_node_id));
					const Node& node = tile->node(entry->_node_id);
					
					if (true)
					{
						Node::D_Edges::const_iterator edges_iter
							= node.predecessors().begin();
						Node::D_Edges::const_iterator edges_iter_end
							= node.predecessors().end();
						for (; edges_iter != edges_iter_end; ++edges_iter)
						{
							TileCache::Pointer test_tile = _tile_cache->get(
								Node::tile_id(edges_iter->get_next_node_id()));
							const Node& test_node = test_tile->node(
								edges_iter->get_next_node_id());
							
							double distance = test_node.distance(node);
							if (distance < min_distance)
								min_distance = distance;
						}
					}
					if (true)
					{
						Node::D_Edges::const_iterator edges_iter
							= node.successors().begin();
						Node::D_Edges::const_iterator edges_iter_end
							= node.successors().end();
						for (; edges_iter != edges_iter_end; ++edges_iter)
						{
							TileCache::Pointer test_tile = _tile_cache->get(
								Node::tile_id(edges_iter->get_next_node_id()));
							const Node& test_node = test_tile->node(
								edges_iter->get_next_node_id());
							
							double distance = test_node.distance(node);
							if (distance < min_distance)
								min_distance = distance;
						}
					}
					
					if (min_distance < _MIN_STEP_DISTANCE_M)
						min_distance = _MIN_STEP_DISTANCE_M;
					
					if (_tile_manager->get_optimisation_mode())
						min_distance = _MIN_STEP_DISTANCE_M;
				}
				
				// Interpolate nodes into the connections.
				if (true)
				{
					assert(verify_node_bearings(entry->_node_id));
					
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(entry->_node_id));
					const Node node = tile->node(entry->_node_id); // NO reference here!!!
					
					if (true)
					{
						Node::D_Edges::const_iterator edges_iter
							= node.predecessors().begin();
						Node::D_Edges::const_iterator edges_iter_end
							= node.predecessors().end();
						for (; edges_iter != edges_iter_end; ++edges_iter)
						{
							assert(verify_node_bearings(edges_iter->get_next_node_id()));
							assert(verify_node_bearings(entry->_node_id));
							create_extra_nodes_on_connection(
								edges_iter->get_next_node_id(), entry->_node_id,
								min_distance);
							assert(verify_node_bearings(edges_iter->get_next_node_id()));
							assert(verify_node_bearings(entry->_node_id));
						}
					}
					if (true)
					{
						Node::D_Edges::const_iterator edges_iter
							= node.successors().begin();
						Node::D_Edges::const_iterator edges_iter_end
							= node.successors().end();
						for (; edges_iter != edges_iter_end; ++edges_iter)
						{
							assert(verify_node_bearings(entry->_node_id));
							assert(verify_node_bearings(edges_iter->get_next_node_id()));
							create_extra_nodes_on_connection(
								entry->_node_id, edges_iter->get_next_node_id(),
								min_distance);
							assert(verify_node_bearings(entry->_node_id));
							assert(verify_node_bearings(edges_iter->get_next_node_id()));
						}
					}
				}
			} // end if (find_iter != created_extra_nodes_here.end()
			
			// Test if previous_entry has entry as successor,
			// resp. entry has next_entry.
			// Set the flags.
			if (true)
			{
				TileCache::Pointer tile = _tile_cache->get(
					Node::tile_id(entry->_node_id));
				const Node& node = tile->node(entry->_node_id);
				
				if ( (entry->_backward_connection != 0)
					&&( !node.has_predecessor(entry->_backward_connection->_node_id)) )
				{
					// we have interpolated some nodes in between.
					entry->_backward_connection->_do_not_use_connection = true;
					entry->_backward_connection->
						_interpolated_nodes_in_between = true;
				}
				
				if ( (entry->_connection != 0)
					&& (!node.has_successor(entry->_connection->_node_id)) )
				{
					// dito
					entry->_do_not_use_connection = true;
					entry->_interpolated_nodes_in_between = true;
				}
			}
		} // end for (; iter != iter_end; ++iter)
	}
	
	
/*	void
	TraceProcessor::create_extra_nodes_at_connection(Node::Id from_node_id,
		Node::Id to_node_id, double step_distance_m)
	{
		if (true)
		{
			TileCache::Pointer from_tile
				= _tile_cache->get(Node::tile_id(from_node_id));
			// Intentionally copied.
			// See comment at create_extra_nodes_at_connections!!
			const Node from_node = from_tile->node(from_node_id);
			
			TileCache::Pointer to_tile
				= _tile_cache->get(Node::tile_id(to_node_id));
			// Intentionally copied.
			// See comment at create_extra_nodes_at_connections!!
			const Node to_node = to_tile->node(to_node_id);
			
			assert(from_node.has_successor(to_node_id));
			assert(to_node.has_predecessor(from_node_id));
			
			double connection_distance = from_node.distance(to_node);
			double tolerance = 1.0 + (1.0 / _STEP_DISTANCE_FACTOR);
			if ( (connection_distance) - (step_distance_m * tolerance) > 0.0 )
			{
				// connection_distance too large.
				// Delete connection...
				disconnect_nodes(from_node_id, to_node_id);
				
				// ... and insert interpolated nodes.
				double new_nodes = floor(
					(connection_distance / step_distance_m)
						+ (1.0 / tolerance) ) - 1.0;
				
				Node::Id previous_node_id = from_node_id;
				for (double i = 1.0; i < new_nodes + 1.0; i += 1.0)
				{
					GeoCoordinate interpolated_gc = GeoCoordinate::interpolate(
						from_node, to_node, 1.0 - i/(new_nodes + 1.0) );
					
					Node::Id current_node_id
						= register_new_node(interpolated_gc);
					
					connect_nodes(previous_node_id, current_node_id);
					
					previous_node_id = current_node_id;
				}
				
				// connect previous_node to to_node
				connect_nodes(previous_node_id, to_node_id);
			}
		}
	}
	
	
	void
	TraceProcessor::create_extra_nodes_at_connections()
	{
		// search beginning.
		const PathEntry* beginning_entry = 0;
		if (true)
		{
			D_Path::const_iterator iter = _path.begin();
			D_Path::const_iterator iter_end = _path.end();
			
			while ( (iter != iter_end) && (!iter->second->_is_beginning) )
				++iter;
			
			assert(iter != iter_end);
			
			beginning_entry = iter->second;
		}
		
		assert(beginning_entry != 0);
		assert(beginning_entry->_is_beginning);
		assert(!beginning_entry->_is_destination);
		
		// Search nodes with more than one predecessor resp. successor,
		// aka a crossing.
		// We will save the entry BEFORE the crossing. We need it to calculate
		// the distance. See below.
		std::vector<const PathEntry*> entering_crossing_path_entries;
		std::vector<const PathEntry*> leaving_crossing_path_entries;
		if (true)
		{
			assert(beginning_entry->_connection != 0);
			
			const PathEntry* previous_entry = beginning_entry;
			const PathEntry* current_entry = beginning_entry->_connection;
			while (current_entry != 0)
			{
				assert(previous_entry->has_state(PathEntry::_REAL));
				assert(current_entry->has_state(PathEntry::_REAL));
				
				if ( (!previous_entry->_was_deleted)
					&& (!current_entry->_was_deleted) )
				{
					// previous_entry.
					TileCache::Pointer previous_tile
						= _tile_cache->get(Node::tile_id(previous_entry->_node_id));
					const Node& previous_node
						= previous_tile->node(previous_entry->_node_id);
				
					if (previous_node.successors().size() > 1)
						leaving_crossing_path_entries.push_back(previous_entry);
					
					// current_entry.
					TileCache::Pointer current_tile
						= _tile_cache->get(Node::tile_id(current_entry->_node_id));
					const Node& current_node
						= current_tile->node(current_entry->_node_id);
					
					if (current_node.predecessors().size() > 1)
						entering_crossing_path_entries.push_back(previous_entry);
				}
				
				previous_entry = current_entry;
				current_entry = current_entry->_connection;
			}
			
			assert(previous_entry->_is_destination);
		}
		
		// Create extra nodes at the crossing.
		// First for the entering nodes
		if (true)
		{
			std::vector<const PathEntry*>::const_iterator iter
				= entering_crossing_path_entries.begin();
			std::vector<const PathEntry*>::const_iterator iter_end
				= entering_crossing_path_entries.end();
			for (; iter != iter_end; ++iter)
			{
				const PathEntry* pre_entry = *iter;
				assert(pre_entry != 0);
				assert(pre_entry->has_state(PathEntry::_REAL));
				assert(!pre_entry->_was_deleted);
				assert(pre_entry->_connection != 0);
				
				TileCache::Pointer pre_tile
					= _tile_cache->get(Node::tile_id(pre_entry->_node_id));
				const Node& pre_node = pre_tile->node(pre_entry->_node_id);
				
				const PathEntry* entry = pre_entry->_connection;
				assert(entry != 0);
				assert(entry->has_state(PathEntry::_REAL));
				assert(!entry->_was_deleted);
				
				TileCache::Pointer tile
					= _tile_cache->get(Node::tile_id(entry->_node_id));
				// Intentionally copied here!!!
				// Reference points to constant address in memory.
				// But the place where the Node is located, can change during
				// this method. When extra nodes are inserted, it may become
				// necessary, that the underlying FixpointVector is reallocated,
				// i.g. copied to another place, enlarged and deleted at the
				// origin place!
				#warning Think twice when using a reference to a Node!
				const Node node = tile->node(entry->_node_id);
				
				double distance = pre_node.distance(node);
				if (distance > _conf.get(_STEP_DISTANCE_M))
					distance = _conf.get(_STEP_DISTANCE_M);
				else if (distance < _MIN_STEP_DISTANCE_M)
					distance = _MIN_STEP_DISTANCE_M;
				
				// predecessors.
				if (true)
				{
					Node::D_Edges::const_iterator edges_iter
						= node.predecessors().begin();
					Node::D_Edges::const_iterator edges_iter_end
						= node.predecessors().end();
					
					std::vector<Node::Id> predecessors;
					for (; edges_iter != edges_iter_end; ++edges_iter)
						predecessors.push_back(edges_iter->get_next_node_id());
					
					for (int i = 0; i < predecessors.size(); ++i)
					{
						create_extra_nodes_at_connection(predecessors[i],
							entry->_node_id, distance);
					}
				}
				
				// successors.
				if (true)
				{
					Node::D_Edges::const_iterator edges_iter
						= node.successors().begin();
					Node::D_Edges::const_iterator edges_iter_end
						= node.successors().end();
					
					std::vector<Node::Id> successors;
					for (; edges_iter != edges_iter_end; ++edges_iter)
						successors.push_back(edges_iter->get_next_node_id());
					
					for (int i = 0; i < successors.size(); ++i)
					{
						create_extra_nodes_at_connection(entry->_node_id,
							successors[i], distance);
					}
				}
			}
		}
		
		// Now for the leaving nodes
		if (true)
		{
			std::vector<const PathEntry*>::const_iterator iter
				= leaving_crossing_path_entries.begin();
			std::vector<const PathEntry*>::const_iterator iter_end
				= leaving_crossing_path_entries.end();
			for (; iter != iter_end; ++iter)
			{
				const PathEntry* entry = *iter;
				assert(entry != 0);
				assert(entry->has_state(PathEntry::_REAL));
				assert(!entry->_was_deleted);
				assert(entry->_connection != 0);
				
				TileCache::Pointer tile
					= _tile_cache->get(Node::tile_id(entry->_node_id));
				// Intentionally copied. See comment above!
				const Node node = tile->node(entry->_node_id);
				
				const PathEntry* post_entry = entry->_connection;
				assert(post_entry != 0);
				assert(post_entry->has_state(PathEntry::_REAL));
				assert(!post_entry->_was_deleted);
				
				TileCache::Pointer post_tile
					= _tile_cache->get(Node::tile_id(post_entry->_node_id));
				const Node& post_node = post_tile->node(post_entry->_node_id);
				
				double distance = node.distance(post_node);
				if (distance > _conf.get(_STEP_DISTANCE_M))
					distance = _conf.get(_STEP_DISTANCE_M);
				else if (distance < _MIN_STEP_DISTANCE_M)
					distance = _MIN_STEP_DISTANCE_M;
				
				// predecessors.
				if (true)
				{
					Node::D_Edges::const_iterator edges_iter
						= node.predecessors().begin();
					Node::D_Edges::const_iterator edges_iter_end
						= node.predecessors().end();
					
					std::vector<Node::Id> predecessors;
					for (; edges_iter != edges_iter_end; ++edges_iter)
						predecessors.push_back(edges_iter->get_next_node_id());
					
					for (int i = 0; i < predecessors.size(); ++i)
					{
						create_extra_nodes_at_connection(predecessors[i],
							entry->_node_id, distance);
					}
				}
				
				// successors.
				if (true)
				{
					Node::D_Edges::const_iterator edges_iter
						= node.successors().begin();
					Node::D_Edges::const_iterator edges_iter_end
						= node.successors().end();
					
					std::vector<Node::Id> successors;
					for (; edges_iter != edges_iter_end; ++edges_iter)
						successors.push_back(edges_iter->get_next_node_id());
					
					for (int i = 0; i < successors.size(); ++i)
					{
						create_extra_nodes_at_connection(entry->_node_id,
							successors[i], distance);
					}
				}
			}
		}
	}*/
	
	
	void
	TraceProcessor::delete_ways(
		const std::multimap<int, PathDeletion>& ways_for_deletion,
		std::set<Node::Id>& deleted_node_ids, bool delete_all)
	{
		#warning Do not delete the longer ways but the newer ones.
		
//		std::cout << "Delete paths. " << paths_for_deletion.size()
//			<< " of them." << std::endl;
		if (ways_for_deletion.size() == 0)
			return;
		
		int left_ways = ways_for_deletion.size();
		std::multimap<int, PathDeletion>::const_reverse_iterator iter
			= ways_for_deletion.rbegin();
		
		while (true)
		{
			if ( (!delete_all) && (left_ways == 1) )
			{
				if (!exists_multiply_ways_in_map(iter->second._start_id,
					iter->second._destination_id, _MAX_DEPTH))
				{
					return;
				}
			}
			
//			std::cout << "Now delete from " << iter->_start_id
//				<< " to " << iter->_destination_id
//				<< " via " << iter->_first_successor << std::endl;
			
//			assert(iter->second._start_id != iter->second._destination_id);
			
			#ifndef NDEBUG
				TileCache::Pointer test_tile
					= _tile_cache->get(Node::tile_id(iter->second._start_id));
				const Node& test_node = test_tile->node(iter->second._start_id);
				
				assert(test_node.has_successor(iter->second._first_successor));
			#endif
			
			// Delete nodes in between.
			Node::Id previous_node_id = iter->second._start_id;
			Node::Id current_node_id = iter->second._first_successor;
			disconnect_nodes(previous_node_id, current_node_id);
			
			while (current_node_id != iter->second._destination_id)
			{
				TileCache::Pointer tile
					= _tile_cache->get(Node::tile_id(current_node_id));
				const Node& node = tile->node(current_node_id);
				
				assert(node.predecessors().size() == 0);
				assert(node.successors().size() == 1);
				
				previous_node_id = current_node_id;
				current_node_id = node.successors().back().get_next_node_id();
				
//				std::cout << "Remove node " << previous_node_id << std::endl;
				disconnect_nodes(previous_node_id, current_node_id);
				
				assert(node.predecessors().empty());
				assert(node.successors().empty());
				
				tile.write().remove_node(previous_node_id);
				deleted_node_ids.insert(previous_node_id);
			}
			
			#warning DO NOT USE NODE REFERENCES TWICE! IS LIKE ASSIGNMENT!!!
			
/*			// Delete two edges.
			if (true)
			{
				// Delete outgoing edge of node with start_id.
				TileCache::Pointer start_tile
					= _tile_cache->get(Node::tile_id(iter->second._start_id));
				Node& start_node
					= start_tile.write().node(iter->second._start_id);
				start_node.remove_successor(iter->second._first_successor);
				
				// Delete incoming edge of node with destination_id.
				TileCache::Pointer destination_tile = _tile_cache->get(
					Node::tile_id(iter->second._destination_id));
				Node& destination_node = destination_tile.write().node(
					iter->second._destination_id);
				destination_node.remove_predecessor(previous_node_id);
			}*/
			
			#ifndef NDEBUG
				std::multimap<int, PathDeletion>::const_reverse_iterator
					test_iter = iter;
				++test_iter;
				assert( (test_iter == ways_for_deletion.rend())
					|| (test_iter->first <= iter->first) );
			#endif
			
			--left_ways;
			++iter;
			
			if (left_ways == 0)
				return;
		} // end while (!exit_loop)
	}

	void
	TraceProcessor::disconnect_nodes(Node::Id from_node_id, Node::Id to_node_id)
	{
		assert(from_node_id != to_node_id);
		
		TileCache::Pointer from_node_tile
			= _tile_cache->get(Node::tile_id(from_node_id));
		Node& from_node = from_node_tile.write().node(from_node_id);
		
		bool successfully_removed_successor
			= from_node.remove_successor(to_node_id);
		assert(successfully_removed_successor);
		
		TileCache::Pointer to_node_tile
			= _tile_cache->get(Node::tile_id(to_node_id));
		Node& to_node = to_node_tile.write().node(to_node_id);
		
		bool successfully_removed_predecessor
			= to_node.remove_predecessor(from_node_id);
		assert(successfully_removed_predecessor);
	}
	
	
	bool
	TraceProcessor::exists_multiply_ways_in_map(Node::Id start_id,
		Node::Id destination_id, int max_depth) const
	{
		assert(max_depth >= 0);
		
		if (max_depth < 2)
			return false;
		
		#warning Mmh, _tile_cache->get(...) is NOT const...
		TileCache::Pointer tile = _tile_cache->get(Node::tile_id(start_id));
		const Node& node = tile->node(start_id);
		
		int path_counter = 0;
		
		Node::D_Edges::const_iterator iter = node.successors().begin();
		Node::D_Edges::const_iterator iter_end = node.successors().end();
		for (; iter != iter_end; ++iter)
		{
			if (exists_way_in_map(start_id, destination_id,
				iter->get_next_node_id(), max_depth - 1) )
			{
				++path_counter;
			}
		}
		
		return path_counter > 1;
	}
	
	
	bool
	TraceProcessor::exists_way_in_map(const Node::Id start_id,
	const Node::Id destination_id, Node::Id recursion_start_id,
		int max_depth) const
	{
		// end of recursion.
		if (recursion_start_id == destination_id)
			return true;
		
		if (recursion_start_id == start_id)
			return false;
		
		if (max_depth == 0)
			return false;
		
		// recursion step.
		TileCache::Pointer tile = _tile_cache->get(
			Node::tile_id(recursion_start_id));
		const Node& node = tile->node(recursion_start_id);
		
		Node::D_Edges::const_iterator iter = node.successors().begin();
		Node::D_Edges::const_iterator iter_end = node.successors().end();
		for (; iter != iter_end; ++iter)
		{
			if (exists_way_in_map(start_id, destination_id,
				iter->get_next_node_id(), max_depth - 1) )
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	bool
	TraceProcessor::exists_way_in_path(const PathEntry* entry,
		Node::Id destination_id, Node::Id first_successor, int max_depth,
		int* steps) const
	{
		assert(entry != 0);
		
		if (entry->_is_destination)
			return false;
		
		// (!entry->_interpolated_nodes_in_between) does not make sense.
		// this if clause won't return false, but definitly the next one!
		if ( (entry->_do_not_use_connection)
			&& (!entry->_interpolated_nodes_in_between) )
		{
			return false;
		}
		
		entry = entry->_connection;
		assert(entry != 0);
		
		if (entry->_node_id != first_successor)
			return false;
		
		int current_depth = 1;
		while (entry != 0)
		{
			if ( (entry->_do_not_use_connection)
				&& (!entry->_interpolated_nodes_in_between) )
			{
				return false;
			}
			
			if (current_depth >= max_depth)
				return false;
			
			if (entry->_node_id == destination_id)
			{
				if (steps != 0)
					*steps = current_depth;
				
				return true;
			}
			
			TileCache::Pointer tile
				= _tile_cache->get(Node::tile_id(entry->_node_id));
			const Node& node = tile->node(entry->_node_id);
			
			if (node.successors().size() > 1)
				return false;
			
			if (node.predecessors().size() > 1)
				return false;
			
			entry = entry->_connection;
			++current_depth;
		}
		
		return false;
	}
	
	
//	std::pair<bool, double>
//	TraceProcessor::calculate_perpendicular_on_trace(PathEntry* path_entry)
	bool
	TraceProcessor::find_and_set_most_fitting_position_on_trace_for(
		PathEntry* path_entry)
	{
		/* Optimization possiblity:
		 * Do not calculate the perpendicular. This might leed to wrong
		 * associations. Search for the most fitting arc. And search in both
		 * directions independently. Perhaps build a product (or whatelse) of
		 * direction (difference) and distance.
		 */
		
/*		TileCache::Pointer tile;
		const GeoCoordinate* entry_coordinate;
		if (path_entry->has_state(PathEntry::_REAL))
		{
			tile = _tile_cache->get(Node::tile_id(path_entry->_node_id));
			entry_coordinate = &(tile->node(path_entry->_node_id));
		} else if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
		{
			entry_coordinate = &(_virtual_entries.point(
				static_cast<D_IndexType>(path_entry->_virtual_node_id
				- _VIRTUAL_NODE_ID_OFFSET)) );
		} else // if (path_entry->_has_state(PathEntry::_VIRTUAL_CREATED))
		{
			return path_entry->_scan_position;
		}*/
		
//		Calculates approximately the perpendicular:
/*		double position = path_entry->_scan_position;
		double previous_distance = 1000000.0;
		double distance = previous_distance - 1.0;
		GeoCoordinate coordinate_on_trace;
		
		double best_position = path_entry->_scan_position;
		
		while ((distance < previous_distance)
			&& (position < _filtered_trace.length_m()))
		{
			previous_distance = distance;
			position += 1.0;
			coordinate_on_trace = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate->distance_approximated(coordinate_on_trace);
			if (distance < previous_distance)
				best_position = position;
		}
		
		previous_distance = distance + 1.0;
		while ((distance < previous_distance) && (position > 0.0))
		{
			previous_distance = distance;
			position -= 1.0;
			coordinate_on_trace = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate->distance_approximated(coordinate_on_trace);
			if (distance < previous_distance)
				best_position = position;
		}
		
		return best_position;*/
		
		
		// Calculates the most fitting arc:
		// Init some values
		double start_position_m = path_entry->_scan_position
			- _conf.get(_STEP_DISTANCE_M)
			- (2.0 * _conf.get(_SEARCH_MAX_DISTANCE_M));
		if (start_position_m < 0.0)
			start_position_m = 0.0;
		
		double end_position_m = path_entry->_scan_position
			+ (2.0 * _conf.get(_SEARCH_MAX_DISTANCE_M));
		if (end_position_m > _filtered_trace.length_m())
			end_position_m = _filtered_trace.length_m() - 0.1;
		#warning some kind of hack here. WHY?
		
		assert(start_position_m < end_position_m);
		
		if (path_entry->has_state(PathEntry::_VIRTUAL_FOUND))
		{
			if ( (start_position_m <= path_entry->_position_on_trace)
				&& (path_entry->_position_on_trace <= end_position_m) )
			{
				path_entry->_scan_position = path_entry->_position_on_trace;
				
				return true;
			}
		}
		
		// Get entry_coordinate
		TileCache::Pointer tile;
		const GeoCoordinate* entry_coordinate;
		if (path_entry->_state == PathEntry::_REAL)
		{
			tile = _tile_cache->get(Node::tile_id(path_entry->_node_id));
			entry_coordinate = &(tile->node(path_entry->_node_id));
		} else if (path_entry->_state == PathEntry::_VIRTUAL_FOUND)
		{
			entry_coordinate = &(_virtual_entries.point(
				static_cast<D_IndexType>(path_entry->_virtual_node_id
				- _VIRTUAL_NODE_ID_OFFSET)) );
		} else // if (path_entry->_state == PathEntry::_VIRTUAL_CREATED)
		{
			return true;
		}
		
		assert(path_entry->_position_on_trace < start_position_m);
		
		// Get involved (real existant, not interpolated) GPSPoints.
		double before_start_point_position_m = 0.0;
		FilteredTrace::const_iterator before_start_point_iter;
		_filtered_trace.gps_points_before_and_after(start_position_m,
			&before_start_point_iter, 0, &before_start_point_position_m);
		
		FilteredTrace::const_iterator after_end_point_iter;
		_filtered_trace.gps_points_before_and_after(end_position_m, 0,
			&after_end_point_iter);
		
		double current_position_on_trace = before_start_point_position_m;
		
		assert(before_start_point_position_m <= start_position_m);
		assert(before_start_point_iter != after_end_point_iter);
		
		// Init used variables.
		Direction smallest_arc_difference_pi = PI;
		double smallest_distance_m = 10.0 * _conf.get(_SEARCH_MAX_DISTANCE_M);
		double best_position_m = -1.0;
		
//		double current_position_on_trace = before_start_point_position_m;
		
		// Search the iters.
//		bool found_perpendicular = false;
		FilteredTrace::const_iterator previous_iter = before_start_point_iter;
		FilteredTrace::const_iterator current_iter = previous_iter;
		while (previous_iter != after_end_point_iter)
		{
			++current_iter;
			
			double previous_position_on_trace = current_position_on_trace;
			current_position_on_trace += previous_iter->distance(*current_iter);
			
			if (entry_coordinate->has_perpendicular_on_segment(
				*previous_iter, *current_iter))
			{
//				found_perpendicular = true;
				
				// search the position on the trace with build together with
				// entry_coordinate the perpendicular.
				double position = search_perpendicular(*entry_coordinate,
					previous_position_on_trace, current_position_on_trace, 1.0);
				
				assert(previous_position_on_trace <= position);
				assert(position <= current_position_on_trace);
				
				GPSPoint test_point = _filtered_trace.gps_point_at(position);
				double distance = entry_coordinate->distance(test_point);
				
				if (distance < smallest_distance_m)
				{
					smallest_distance_m = distance;
					best_position_m = position;
					
					// Calculate arc difference
					if (path_entry->_state == PathEntry::_REAL)
					{
						const Node* entry_node
							= dynamic_cast<const Node*>(entry_coordinate);
						if (entry_node == 0)
						{
							std::cout << "TraceProcessor::calculate_perpendicular_on_trace: first cast failed!" << std::endl;
							throw "Oops.";
						}
						
						smallest_arc_difference_pi = entry_node
							->minimal_direction_difference_to(test_point);
					} else // if (path_entry->_state == PathEntry::_VIRTUAL_FOUND)
					{
						const VirtualNode* entry_virtual_node
							= dynamic_cast<const VirtualNode*>(entry_coordinate);
						if (entry_virtual_node == 0)
						{
							std::cout << "TraceProcessor::calculate_perpendicular_on_trace: second cast failed!" << std::endl;
							throw "Oops.";
						}
						
						smallest_arc_difference_pi = entry_virtual_node
							->angle_difference(test_point);
					}
				} // end if (distance < smallest_distance_m)
			} // end if (entry_coordinate->has_perpendicular_on_segment(...)
			
			++previous_iter;
		} // end while (previous_iter != after_end_point_iter)
		
		// compute distance to real GPSPoints and compare.
		previous_iter = before_start_point_iter;
		current_iter = previous_iter;
		++current_iter;
		
//		assert(current_iter != after_end_point_iter);
		
		current_position_on_trace = before_start_point_position_m;
		while (current_iter != after_end_point_iter)
		{
			current_position_on_trace
				+= previous_iter->distance(*current_iter);
		
			double distance = entry_coordinate->distance(*current_iter);
			if (distance < smallest_distance_m)
			{
				smallest_distance_m = distance;
				best_position_m = current_position_on_trace;
				
				// Calculate arc difference
				if (path_entry->_state == PathEntry::_REAL)
				{
					const Node* entry_node
						= dynamic_cast<const Node*>(entry_coordinate);
					if (entry_node == 0)
					{
						std::cout << "TraceProcessor::calculate_perpendicular_on_trace: first cast failed!" << std::endl;
						throw "Oops.";
					}
					
					smallest_arc_difference_pi = entry_node
						->minimal_direction_difference_to(*current_iter);
				} else // if (path_entry->_state == PathEntry::_VIRTUAL_FOUND)
				{
					const VirtualNode* entry_virtual_node
						= dynamic_cast<const VirtualNode*>(entry_coordinate);
					if (entry_virtual_node == 0)
					{
						std::cout << "TraceProcessor::calculate_perpendicular_on_trace: second cast failed!" << std::endl;
						throw "Oops.";
					}
					
					smallest_arc_difference_pi = entry_virtual_node
						->angle_difference(*current_iter);
				}
			} // end if (distance < smallest_distance_m)
			
			++previous_iter;
			++current_iter;
		} // end while (current_iter != after_end_point_iter)
		
		// The assertion and the if clause should never fulfill, actually.
		// Don't know why that does not work. But a "return false" won't do
		// too much damage.
//		assert(best_position_m != -1.0);
		if (best_position_m == -1.0)
			return false;
		
		if (best_position_m < start_position_m)
			best_position_m = start_position_m;
		
		if (best_position_m > end_position_m)
			best_position_m = end_position_m;
		
/*		// Main loop
		double current_position_m = start_position_m;
		while (current_position_m < end_position_m)
		{
			GPSPoint test_point
				= _filtered_trace.gps_point_at(current_position_m);
			Direction current_arc_difference_pi;
			double current_distance_m;
			
			// Calculate arc difference
			if (path_entry->_state == PathEntry::_REAL)
			{
				const Node* entry_node
					= dynamic_cast<const Node*>(entry_coordinate);
				if (entry_node == 0)
				{
					std::cout << "TraceProcessor::calculate_perpendicular_on_trace: first cast failed!" << std::endl;
					throw "Oops.";
				}
				
				current_arc_difference_pi
					= entry_node->minimal_direction_difference_to(test_point);
			} else // if (path_entry->_state == PathEntry::_VIRTUAL_FOUND)
			{
				const VirtualNode* entry_virtual_node
					= dynamic_cast<const VirtualNode*>(entry_coordinate);
				if (entry_virtual_node == 0)
				{
					std::cout << "TraceProcessor::calculate_perpendicular_on_trace: second cast failed!" << std::endl;
					throw "Oops.";
				}
				
				current_arc_difference_pi
					= entry_virtual_node->angle_difference(test_point);
			}
			
			// Calculate distance
			current_distance_m = entry_coordinate->distance(test_point);
			
/*			if (current_arc_difference_pi < smallest_arc_difference_pi)
			{
				smallest_arc_difference_pi = current_arc_difference_pi;
				best_position_m = current_position_m;
			}*******
			
			if (current_distance_m < smallest_distance_m)
			{
				smallest_distance_m = current_distance_m;
				smallest_arc_difference_pi = current_arc_difference_pi;
				best_position_m = current_position_m;
			}

/*			// Decide which position is the best one
			if (current_arc_difference_pi <= smallest_arc_difference_pi)
			{
				Direction smallest2current_arc_difference
					= smallest_arc_difference_pi.angle_difference(
						current_arc_difference_pi);
				if (smallest2current_arc_difference < PI/18.0)
				{
					double current_arc_difference_distance
						= entry_coordinate->distance_approximated(test_point);
					if (current_arc_difference_distance <= smallest_arc_difference_distance)
					{
						smallest_arc_difference_pi = current_arc_difference_pi;
						smallest_arc_difference_distance = current_arc_difference_distance;
						best_position_m = start_position_m;
					} else
					{
						smallest_arc_difference_pi = current_arc_difference_pi;
					}
					
				} else
				{
					double current_arc_difference_distance
						= entry_coordinate->distance_approximated(test_point);
					
					smallest_arc_difference_pi = current_arc_difference_pi;
					smallest_arc_difference_distance = current_arc_difference_distance;
					best_position_m = start_position_m;
				}
				
			} else //if (current_arc_difference_pi > smallest_arc_difference_pi)
			{
				Direction smallest2current_arc_difference
					= smallest_arc_difference_pi.angle_difference(
						current_arc_difference_pi);
				if (smallest2current_arc_difference < PI/18.0)
				{
					double current_arc_difference_distance
						= entry_coordinate->distance_approximated(test_point);
					if (current_arc_difference_distance <= smallest_arc_difference_distance)
					{
						smallest_arc_difference_distance = current_arc_difference_distance;
						best_position_m = start_position_m;
					}
				}
			}
*******			
			current_position_m += 1.0;
		} // end while
*/		
		if (smallest_distance_m >= 2.0 * _conf.get(_SEARCH_MAX_DISTANCE_M))
		{
			mlog(MLog::warning, "TraceProcessor")
				<< "smallest_distance_m >= 2.0 * _conf.get(_SEARCH_MAX_DISTANCE_M). But WHY?!\n";
		}
		
		path_entry->_scan_position = best_position_m;
		
//		return (smallest_arc_difference_pi < 0.15 * PI);
		return (smallest_distance_m < _conf.get(_SEARCH_MAX_DISTANCE_M))
			&& (smallest_arc_difference_pi < 0.25*PI);
//		return std::make_pair(smallest_arc_difference_pi < 0.15 * PI,
//			best_position_m);
	}
		
	
/*	void
	TraceProcessor::cut_encountered_nodes(T_EncounteredNodeIds& eni,
		T_EncounteredNodesPositions& enp, double position_m) const
	{
		T_EncounteredNodesPositions::iterator iter = enp.begin();
		T_EncounteredNodesPositions::iterator upper_bound_iter
			= enp.upper_bound(position_m);
		
		for(; iter != upper_bound_iter; ++iter)
			eni.erase(iter->second);
		
		enp.erase(enp.begin(), upper_bound_iter);
	}*/
	
	
/*	void
	TraceProcessor::handle_connection(
		std::multimap<Node::Id, PathEntry*>& node_ids_entries_mapper,
		PathEntry* const start_entry, PathEntry* const end_entry)
	{
//		std::cout << _path << std::endl;
		
		assert(start_entry != end_entry);
		assert(start_entry->has_state(PathEntry::_REAL));
		assert(end_entry->has_state(PathEntry::_REAL));
		
		// Insert nodes in between (if there are any).
		PathEntry* current_entry = start_entry;
		while (current_entry->_connection != end_entry)
		{
			current_entry->_connection->_node_id = register_new_node(
				_virtual_entries.point(static_cast<D_IndexType>(
					current_entry->_connection->_virtual_node_id
					- _VIRTUAL_NODE_ID_OFFSET))
				);
			
			node_ids_entries_mapper.insert(std::make_pair(
				current_entry->_connection->_node_id, current_entry->_connection));
			
			current_entry->_connection->_state = PathEntry::_REAL;
			
			current_entry = current_entry->_connection;
		}
		
		// Interpolate entries.
		PathEntry* previous_entry = start_entry;
		current_entry = start_entry;
		while (current_entry != end_entry)
		{
			if (!current_entry->_do_not_use_connection)
			{
				assert(current_entry->_scan_position > -2.0);
				assert(current_entry->_scan_position > -2.0);
				
				assert(current_entry != 0);
				assert(current_entry->_connection != 0);
				
				assert(current_entry->has_state(PathEntry::_REAL));
				assert(current_entry->_connection->has_state(PathEntry::_REAL));
				
				#ifndef NDEBUG
					PathEntry* saved_entry = current_entry;
				#endif
				
				// Both intentionally copied!
				// Same problem as in create_extra_nodes_at_connection!
				TileCache::Pointer tp_1 = _tile_cache->get(
					Node::tile_id(current_entry->_node_id));
				const Node n1 = (tp_1->node(current_entry->_node_id));
				
				TileCache::Pointer tp_2 = _tile_cache->get(
					Node::tile_id(current_entry->_connection->_node_id));
				const Node n2 = (tp_2->node(current_entry->_connection->_node_id));
				
				if (current_entry->is_virtual_predecessor_of(*current_entry->_connection))
				{
					// do nothing
				} else if (n1.has_successor(current_entry->_connection->_node_id))
				{
					// do nothing
				} else
				{
					
					double direction_difference
						= minimal_direction_difference_between(previous_entry->_node_id,
							current_entry->_node_id, current_entry->_connection->_node_id);
					double steps_caused_by_angle
						= floor( direction_difference / (0.7 * _MAX_CURVATURE) ); /////////////////////////////// USE CONFIGURATION VALUE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//			std::cout << direction_difference << ", " << steps_caused_by_angle << std::endl;
					
					double distance = n1.distance(n2);
					double steps_caused_by_distance 
							= ceil( (distance / (0.7 * _MIN_STEP_DISTANCE_M)) - 0.5 ); /////////////////////////////// USE CONFIGURATION VALUE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					
					double steps = steps_caused_by_angle;
					if (steps < steps_caused_by_distance)
						steps = steps_caused_by_distance;
					
					if (distance / steps < 0.7 * _MIN_STEP_DISTANCE_M)
					{
						steps = floor(distance / (0.7 * _MIN_STEP_DISTANCE_M));
					}
		//			steps = 1.0;
		//			std::cout << steps << std::endl;
					
					PathEntry* saved_connection = current_entry->_connection;
					for (double i = 1.0; i < steps; i += 1.0)
					{
						PathEntry* interpolated_entry = new PathEntry;
						interpolated_entry->_position_on_trace = -2.0;
						interpolated_entry->_scan_position = -2.0;
						interpolated_entry->_node_id = register_new_node(
							GeoCoordinate::interpolate( n1, n2, 1.0 - (i / steps) ));
						
						// Uses _path.insert(...) directly. At this point we can
						// ignore predecessor-successor-relationships.
						_path.insert(std::make_pair(
							interpolated_entry->_scan_position,interpolated_entry));
						
						node_ids_entries_mapper.insert(std::make_pair(
								interpolated_entry->_node_id, interpolated_entry));
						
						current_entry->_connection = interpolated_entry;
						current_entry = current_entry->_connection;
					}
					
					current_entry->_connection = saved_connection;
					
				} // end if
			} // end if
			
			previous_entry = current_entry;
			current_entry = current_entry->_connection;
		}
		
		
		// Connect nodes.
		current_entry = start_entry;
		while (current_entry != end_entry)
		{
			assert(current_entry != 0);
			assert(current_entry->_connection != 0);
			
			if (!current_entry->_do_not_use_connection)
			{
				connect_nodes(current_entry->_node_id,
					current_entry->_connection->_node_id);
			}
			
			// Go to next entry.
			current_entry = current_entry->_connection;
		}
	}*/
	
	
	void
	TraceProcessor::handle_double_ways(
		const std::multimap<Node::Id, CrossingItem>& crossings_mapper)
	{
//		std::cout << _path << std::endl;
		
		std::map<PathEntry*, Node::Id> potential_deleted_path_ways;
		std::multimap<int, PathDeletion> ways_for_deletion;
		std::set<Node::Id> deleted_node_ids;
			
		std::multimap<Node::Id, CrossingItem>::const_iterator iter
			= crossings_mapper.begin();
		std::multimap<Node::Id, CrossingItem>::const_iterator iter_end
			= crossings_mapper.end();
		for (; iter != iter_end; ++iter)
		{
			assert(iter->first == iter->second._entry->_node_id);
			assert(iter->second._entry->has_state(PathEntry::_REAL));
			
			Node::Id node_id = iter->first;
			TileCache::Pointer tile = _tile_cache->get(
				Node::tile_id(node_id));
			const Node node = tile->node(node_id);
			
			Node::D_Edges::const_iterator edges_iter
				= node.successors().begin();
			Node::D_Edges::const_iterator edges_iter_end
				= node.successors().end();
			for (; edges_iter != edges_iter_end; ++edges_iter)
			{
				// search a destination node id.
				Node::Id destination_id;
				bool found_destination_id = false;
				
				Node::Id current_node_id = edges_iter->get_next_node_id();
				int current_steps = 0;
				
				bool exit_loop = false;
				while (!exit_loop)
				{
					TileCache::Pointer current_tile
						= _tile_cache->get(Node::tile_id(current_node_id));
					const Node& current_node
						= current_tile->node(current_node_id);
					
					assert(current_node.predecessors().size() >= 1);
					
					if ( (current_node.predecessors().size() > 1))
					{
						// More than one predecessor. This is our
						// destination node.
						destination_id = current_node_id;
						found_destination_id = true;
						exit_loop = true;
					} else // if (current_node.predecessors().size() == 1)
					{
						// One predecessor. The one we came from.
						// Take a look at the successors to decide what to do.
						
						if (current_node.successors().size() == 1)
						{
							// One successor. Follow it if a step are left.
							current_node_id = current_node.successors()
								.front().get_next_node_id();
						} else // if (current_node.successors().size() < resp. > 1)
						{
							// No successor or next crossing is found.
							exit_loop = true;
						}
					}
					
					if (current_steps < _MAX_DEPTH)
						++current_steps;
					else
						exit_loop = true;
				} // end while (!exit_loop)
				
				// if we found a destination node id, we can test if there
				// exists an alternative way.
				if (found_destination_id)
				{
					bool use_this_way = true;
					
					if (use_this_way)
					{
						use_this_way = exists_multiply_ways_in_map(
							node_id, destination_id, _MAX_DEPTH);
					}
					
					int steps = _MAX_DEPTH;
					if (use_this_way)
					{
						use_this_way = exists_way_in_path(iter->second._entry,
							destination_id, edges_iter->get_next_node_id(),
							_MAX_DEPTH, &steps);
					}
					
					if (use_this_way)
					{
						PathDeletion pd;
						pd._start_id = node_id;
						pd._destination_id = destination_id;
						pd._first_successor = edges_iter->get_next_node_id();
						
						potential_deleted_path_ways.insert(std::make_pair(
							iter->second._entry, destination_id));
						ways_for_deletion.insert(std::make_pair(steps, pd));
					}
				}
			} // end for (; edges_iter != edges_iter_end; ++edges_iter)
		} // end for (; iter != iter_end; ++iter)
		
//		assert(deleted_node_ids.empty());
		
		// Are there double entries in ways_for_deletion?!
		if (true)
		{
			std::multimap<int, PathDeletion>::iterator iter
				= ways_for_deletion.begin();
			while (iter != ways_for_deletion.end())
			{
				bool delete_iter = false;
				
				std::multimap<int, PathDeletion>::iterator test_iter
					= ways_for_deletion.begin();
				while ( (!delete_iter) && (test_iter != iter) )
				{
					PathDeletion iter_pd = iter->second;
					PathDeletion test_iter_pd = test_iter->second;
					if ( (test_iter_pd._start_id == iter_pd._start_id)
						&& (test_iter_pd._destination_id == iter_pd._destination_id)
						&& (test_iter_pd._first_successor == iter_pd._first_successor) )
					{
						delete_iter = true;
					}
					
					++test_iter;
				}
				
				if (delete_iter)
					ways_for_deletion.erase(iter);
				
				++iter;
			}
		}
		
		delete_ways(ways_for_deletion, deleted_node_ids);
		
		std::map<PathEntry*, Node::Id>::const_iterator
			deleted_path_ways_iter = potential_deleted_path_ways.begin();
		std::map<PathEntry*, Node::Id>::const_iterator
			deleted_path_ways_iter_end = potential_deleted_path_ways.end();
		while (deleted_path_ways_iter != deleted_path_ways_iter_end)
		{
			PathEntry* const entry = deleted_path_ways_iter->first;
			Node::Id destination_id = deleted_path_ways_iter->second;
			
			if (entry->_connection->_node_id == destination_id)
			{
				TileCache::Pointer tile = _tile_cache->get(
					Node::tile_id(entry->_node_id));
				const Node& node = tile->node(entry->_node_id);
				
				if (!node.has_successor(destination_id))
					entry->_do_not_use_connection = true;
				
			} else if (deleted_node_ids.count(entry->_connection->_node_id) > 0)
			{
				PathEntry* current_entry = entry->_connection;
				while (current_entry->_node_id != destination_id)
					current_entry = current_entry->_connection;
				
				assert(current_entry != 0);
				
				entry->_connection = current_entry;
				current_entry->_backward_connection = entry;
				
				entry->_do_not_use_connection = true;
			}
			
			++deleted_path_ways_iter;
		}
			
			// Mark path entries with deleted nodes.
//			if (true)
//			{
//				D_Path::iterator iter = _path.begin();
//				D_Path::iterator iter_end = _path.end();
//				for (; iter != iter_end; ++iter)
//				{
//					PathEntry* entry = iter->second;
//					if (deleted_node_ids.count(entry->_node_id) > 0)
//						entry->_was_deleted = true;
//				}
//			}
	}
	
	
	void
	TraceProcessor::handle_parallel_autobahn_ways(
		const std::multimap<Node::Id, CrossingItem>& crossings_mapper)
	{
		std::map<Node::Id, PathDeletion> deleted_path_ways;
		
		std::map<Node::Id, CrossingItem>::const_iterator iter
			= crossings_mapper.begin();
		std::map<Node::Id, CrossingItem>::const_iterator iter_end
			= crossings_mapper.end();
		for (; iter != iter_end; ++iter)
		{
			assert(iter->first == iter->second._entry->_node_id);
			assert(iter->second._entry->has_state(PathEntry::_REAL));
			
			Node::Id node_id = iter->first;
			TileCache::Pointer tile = _tile_cache->get(
				Node::tile_id(node_id));
			const Node node = tile->node(node_id);
			
			Node::Id destination_id;
			Node::Id first_successor;
			bool result = search_parallel_ways(node_id,
				destination_id, first_successor);
				
			if (result)
			{
				PathDeletion pd;
				pd._start_id = node_id;
				pd._destination_id = destination_id;
				pd._first_successor = first_successor;
				
				deleted_path_ways.insert(std::make_pair(node_id, pd));
				
				std::multimap<int, PathDeletion> ways_for_deletion;
				ways_for_deletion.insert(std::make_pair(0, pd));
				
				std::set<Node::Id> deleted_node_ids;
				delete_ways(ways_for_deletion, deleted_node_ids, true);
				
				// Mark path entries with deleted nodes.
//				D_Path::iterator path_iter = _path.begin();
//				D_Path::iterator path_iter_end = _path.end();
//				for (; path_iter != path_iter_end; ++path_iter)
//				{
//					PathEntry* entry = path_iter->second;
//					if (deleted_node_ids.count(entry->_node_id) > 0)
//						entry->_was_deleted = true;
//				}
			} // end if (result)
		} // for (; iter != iter_end; ++iter)
		
		// Find the beginning.
		D_Path::iterator path_iter = _path.begin();
		D_Path::iterator path_iter_end = _path.end();
		while ( (path_iter != path_iter_end)
			&& (!path_iter->second->_is_beginning) )
		{
			++path_iter;
		}
		
		assert(path_iter != path_iter_end);
		assert(path_iter->second->_is_beginning);
		
		const std::map<Node::Id, PathDeletion>::const_iterator
			deleted_path_ways_iter_end = deleted_path_ways.end();
		
		PathEntry* entry = path_iter->second;
		while (entry != 0)
		{
			std::map<Node::Id, PathDeletion>::const_iterator find_iter
				= deleted_path_ways.find(entry->_node_id);
			if (find_iter != deleted_path_ways_iter_end)
			{
				assert(entry->_connection != 0);
				
				const PathDeletion& pd = find_iter->second;
				if (entry->_connection->_node_id == pd._first_successor)
				{
					// Set new connection.
					PathEntry* current_entry = entry->_connection;
					while (current_entry->_node_id != pd._destination_id)
						current_entry = current_entry->_connection;
					
					entry->_do_not_use_connection = true;
					
					entry->_connection = current_entry;
					current_entry->_backward_connection = entry;
				}
			}
			
			entry = entry->_connection;
		} // end while (entry != 0)
	}
	
	
	void
	TraceProcessor::handle_loops(
		const std::multimap<Node::Id, CrossingItem>& crossings_mapper)
	{
//		std::cout << _path << std::endl;
		
		std::multimap<Node::Id, CrossingItem>::const_iterator iter
			= crossings_mapper.begin();
		std::multimap<Node::Id, CrossingItem>::const_iterator iter_end
			= crossings_mapper.end();
		for (; iter != iter_end; ++iter)
		{
			assert(iter->first == iter->second._entry->_node_id);
			assert(iter->second._entry->has_state(PathEntry::_REAL));
			
			Node::Id node_id = iter->first;
			TileCache::Pointer tile = _tile_cache->get(
				Node::tile_id(node_id));
			const Node node = tile->node(node_id);
			
			Node::D_Edges::const_iterator edges_iter
				= node.successors().begin();
			Node::D_Edges::const_iterator edges_iter_end
				= node.successors().end();
			for (; edges_iter != edges_iter_end; ++edges_iter)
			{
				if (exists_way_in_path(iter->second._entry, node_id,
					edges_iter->get_next_node_id(), _MAX_DEPTH))
				{
					PathDeletion pd;
					pd._start_id = node_id;
					pd._destination_id = node_id;
					pd._first_successor = edges_iter->get_next_node_id();
					
					std::multimap<int, PathDeletion> ways_for_deletion;
					ways_for_deletion.insert(std::make_pair(0, pd));
					
					std::set<Node::Id> deleted_node_ids;
					delete_ways(ways_for_deletion, deleted_node_ids, true);
					
					// Change pointers.
					PathEntry* entry = iter->second._entry->_connection;
					while (entry->_node_id != node_id)
						entry = entry->_connection;
					
					assert(entry->_connection != 0);
					
					iter->second._entry->_connection = entry->_connection;
					entry->_connection->_backward_connection = iter->second._entry;
					
					// Mark path entries with deleted nodes.
//					D_Path::iterator path_iter = _path.begin();
//					D_Path::iterator path_iter_end = _path.end();
//					for (; path_iter != path_iter_end; ++path_iter)
//					{
//						PathEntry* entry = path_iter->second;
//						if (deleted_node_ids.count(entry->_node_id) > 0)
//							entry->_was_deleted = true;
//					}
				}
			}
		}
	}
	
	
/*	void
	TraceProcessor::handle_double_connections()
	{
//		std::cout << _path << std::endl;
		const int MAX_STEPS = 15;
		
		// search beginning.
		const PathEntry* beginning_entry = 0;
		if (true)
		{
			D_Path::const_iterator iter = _path.begin();
			D_Path::const_iterator iter_end = _path.end();
			
			while ( (iter != iter_end) && (!iter->second->_is_beginning) )
				++iter;
			
			assert(iter != iter_end);
			
			beginning_entry = iter->second;
		}
		
		assert(beginning_entry != 0);
		assert(beginning_entry->_is_beginning);
		assert(!beginning_entry->_is_destination);
		
		// Search nodes with more than one successor, aka a crossing.
		std::vector<const PathEntry*> outgoing_crossing_node_ids;
		if (true)
		{
			const PathEntry* entry = beginning_entry;
			while (entry != 0)
			{
				TileCache::Pointer tile
					= _tile_cache->get(Node::tile_id(entry->_node_id));
				const Node& node = tile->node(entry->_node_id);
				
				if (node.successors().size() > 1)
				{
//					std::cout << *entry << std::endl;
//					std::cout << node << std::endl << std::endl;
					
					outgoing_crossing_node_ids.push_back(entry);
				}
				
				entry = entry->_connection;
			}
		}
		
		// Set with deleted node ids.
		std::set<Node::Id> deleted_node_ids;
		
		// Test the found node ids for loops.
		if (true)
		{
			std::vector<const PathEntry*>::const_iterator iter
				= outgoing_crossing_node_ids.begin();
			std::vector<const PathEntry*>::const_iterator iter_end
				= outgoing_crossing_node_ids.end();
			for (; iter != iter_end; ++iter)
			{
				TileCache::Pointer tile
					= _tile_cache->get(Node::tile_id((*iter)->_node_id));
				const Node node = tile->node((*iter)->_node_id);
				
				Node::D_Edges::const_iterator edges_iter
					= node.successors().begin();
				Node::D_Edges::const_iterator edges_iter_end
					= node.successors().end();
				
				std::vector<Node::Id> successors;
				for (; edges_iter != edges_iter_end; ++edges_iter)
					successors.push_back(edges_iter->get_next_node_id());
				
				for (int i = 0; i < successors.size(); ++i)
				{
					bool use_this_way = true;
					if (use_this_way)
					{
						#warning rename it to exists_unique_...
						use_this_way = exists_way_in_path(*iter, 
							(*iter)->_node_id, successors[i], MAX_STEPS);
						
//						bool test = exists_way_in_map(
//							edges_iter->get_next_node_id(), (*iter)->_node_id, 10 );
					}
					
					if (use_this_way)
					{
						PathDeletion pd;
						pd._start_id = (*iter)->_node_id;
						pd._destination_id = (*iter)->_node_id;
						pd._first_successor = successors[i];
						
						std::multimap<int, PathDeletion> ways_for_deletion;
						ways_for_deletion.insert(std::make_pair(0, pd));
						
						assert(deleted_node_ids.empty());
						delete_ways(ways_for_deletion, deleted_node_ids, true);
						
						// Mark path entries with deleted nodes.
						if (true)
						{
							D_Path::iterator iter = _path.begin();
							D_Path::iterator iter_end = _path.end();
							for (; iter != iter_end; ++iter)
							{
								PathEntry* entry = iter->second;
								if (deleted_node_ids.count(entry->_node_id) > 0)
									entry->_was_deleted = true;
							}
						}
						
						deleted_node_ids.clear();
					}
				}
			}
		}
		
		// Test the found node ids and make the paths for deletion.
		if (true)
		{
			std::vector<const PathEntry*>::const_iterator iter
				= outgoing_crossing_node_ids.begin();
			std::vector<const PathEntry*>::const_iterator iter_end
				= outgoing_crossing_node_ids.end();
			for (; iter != iter_end; ++iter)
			{
//				assert((*iter)->_was_deleted == false);
				if ((*iter)->_was_deleted == false)
				{
					TileCache::Pointer tile
						= _tile_cache->get(Node::tile_id((*iter)->_node_id));
					const Node node = tile->node((*iter)->_node_id);
					
					Node::D_Edges::const_iterator edges_iter
						= node.successors().begin();
					Node::D_Edges::const_iterator edges_iter_end
						= node.successors().end();
					
					std::multimap<int, PathDeletion> ways_for_deletion;
					
					for (; edges_iter != edges_iter_end; ++edges_iter)
					{
						// search a destination node id.
						Node::Id destination_id;
						bool found_destination_id = false;
						
						Node::Id current_node_id = edges_iter->get_next_node_id();
						int current_steps = 0;
						
						bool exit_loop = false;
						while (!exit_loop)
						{
							TileCache::Pointer current_tile
								= _tile_cache->get(Node::tile_id(current_node_id));
							const Node current_node
								= current_tile->node(current_node_id);
							
							assert(current_node.predecessors().size() >= 1);
							
							if ( (current_node.predecessors().size() > 1))
							{
								// More than one predecessor. This is our
								// destination node.
								destination_id = current_node_id;
								found_destination_id = true;
								exit_loop = true;
							} else // if (current_node.predecessors().size() == 1)
							{
								// One predecessor. The one we came from.
								// Take a look at the successors to decide what to do.
								
								if (current_node.successors().size() == 1)
								{
									// One successor. Follow it if a step are left.
									current_node_id = current_node.successors()
										.back().get_next_node_id();
								} else // if (current_node.successors().size() < resp. > 1)
								{
									// No successor or next crossing is found.
									exit_loop = true;
								}
							}
							
							if (current_steps < MAX_STEPS)
								++current_steps;
							else
								exit_loop = true;
						} // end while (!exit_loop)
						
						// if we found a destination node id, we can test if there
						// exists an alternative path.
						if (found_destination_id)
						{
							bool use_this_way = true;
							
							if (use_this_way)
							{
								use_this_way = exists_multiply_way_in_map(
									(*iter)->_node_id, destination_id, MAX_STEPS);
							}
							
							int steps = MAX_STEPS;
							if (use_this_way)
							{
								use_this_way = exists_way_in_path(*iter,
									destination_id, edges_iter->get_next_node_id(),
									MAX_STEPS, &steps);
							}
							
							if (use_this_way)
							{
								PathDeletion pd;
								pd._start_id = (*iter)->_node_id;
								pd._destination_id = destination_id;
								pd._first_successor = edges_iter->get_next_node_id();
								
								ways_for_deletion.insert(std::make_pair(steps, pd));
							}
						}
					} // end for (; edges_iter != edges_iter_end; ++edges_iter)
					
					assert(deleted_node_ids.empty());
					delete_ways(ways_for_deletion, deleted_node_ids);
					
					// Mark path entries with deleted nodes.
					if (true)
					{
						D_Path::iterator iter = _path.begin();
						D_Path::iterator iter_end = _path.end();
						for (; iter != iter_end; ++iter)
						{
							PathEntry* entry = iter->second;
							if (deleted_node_ids.count(entry->_node_id) > 0)
								entry->_was_deleted = true;
						}
					}
					
					deleted_node_ids.clear();
				} // end if ((*iter)->_was_deleted == false)
			} // end for (; iter != iter_end; ++iter)
		} // end if(true)
		
		// Test for parallel Autobahn lanes.
		if (true)
		{
			std::multimap<int, PathDeletion> ways_for_deletion;
						
			std::vector<const PathEntry*>::const_iterator iter
				= outgoing_crossing_node_ids.begin();
			std::vector<const PathEntry*>::const_iterator iter_end
				= outgoing_crossing_node_ids.end();
			for (; iter != iter_end; ++iter)
			{
//				assert((*iter)->_was_deleted == false);
				if ((*iter)->_was_deleted == false)
				{
					Node::Id destination_id;
					Node::Id first_successor;
					bool result = search_parallel_ways(
						(*iter)->_node_id,destination_id, first_successor );
					
					if (result)
					{
						PathDeletion pd;
						pd._start_id = (*iter)->_node_id;
						pd._destination_id = destination_id;
						pd._first_successor = first_successor;
						
						ways_for_deletion.insert(std::make_pair(0, pd));
					}
				}
			}
			
			assert(deleted_node_ids.empty());
			delete_ways(ways_for_deletion, deleted_node_ids, true);
			
			// Mark path entries with deleted nodes.
			if (true)
			{
				D_Path::iterator iter = _path.begin();
				D_Path::iterator iter_end = _path.end();
				for (; iter != iter_end; ++iter)
				{
					PathEntry* entry = iter->second;
					if (deleted_node_ids.count(entry->_node_id) > 0)
						entry->_was_deleted = true;
				}
			}
			
			deleted_node_ids.clear();
		}
	}*/
	
	
	void
	TraceProcessor::insert_entry_into_path(PathEntry* entry)
	{
		std::pair<D_Path::iterator, D_Path::iterator> found_iters
			= _path.equal_range(entry->_scan_position);
		
		// iters equal? Then just insert!
		if (found_iters.first == found_iters.second)
		{
			_path.insert(std::make_pair(entry->_scan_position, entry));
			return;
		}
		
		// else: search insertion position.
		bool insertion_position_found = false;
		
		if (entry->has_state(PathEntry::_REAL))
		{
			TileCache::Pointer tile
				= _tile_cache->get(Node::tile_id(entry->_node_id));
			const Node& node = tile->node(entry->_node_id);
			
			bool node_has_predecessor = false;
			while ( (found_iters.first != found_iters.second) 
				&& (!insertion_position_found) )
			{
				PathEntry* found_entry = found_iters.first->second;
				
				if (found_entry->has_state(PathEntry::_REAL))
				{
					if (node.has_successor(found_entry->_node_id))
					{
						insertion_position_found = true;
					} else if (node.has_predecessor(found_entry->_node_id))
					{
						node_has_predecessor = true;
					} else if (node_has_predecessor)
					{
						assert(!node.has_predecessor(found_entry->_node_id));
						assert(!node.has_successor(found_entry->_node_id));
						insertion_position_found = true;
					}
				}
				
				if (!insertion_position_found)
					++(found_iters.first);
				
				// if no predecessor or successor is found, append it to the
				// end. Might be dangerous if there is this situation:
				// A-B-C. C in path. Now inspecting A. And B follows later.
				// This results is C-A-B in the path. Bad luck.
			}
			
		} else if (entry->has_state(PathEntry::_VIRTUAL_FOUND))
		{
			while ( (found_iters.first != found_iters.second) 
				&& (!insertion_position_found) )
			{
				PathEntry* found_entry = found_iters.first->second;
				
				if ( (found_entry->has_state(PathEntry::_VIRTUAL_FOUND))
					&& (entry->_position_on_trace < found_entry->_position_on_trace) )
				{
					insertion_position_found = true;
				} else
				{
					++(found_iters.first);
				}
			}
		}
		
		if (!insertion_position_found)
		{
			// redundant. And only needed for _VIRTUAL_CREATED entries.
			// garantees that these entries are at the end.
			_path.insert(found_iters.second,
				std::make_pair(entry->_scan_position, entry));
		} else
		{
			#warning Uses std::multimap::insert(iterator, item) here. At my pc \
			that works. But can we garantee it?! What is the exact specification \
			when using the iterator?
			
			_path.insert(found_iters.first,
				std::make_pair(entry->_scan_position, entry));
		}
	}
	
	
	void
	TraceProcessor::insert_extra_entries_into_path()
	{
		D_Path::iterator iter = _path.begin();
		for (; iter != _path.end(); ++iter)
		{
			PathEntry* entry = iter->second;
			if ( (!entry->has_state(PathEntry::_VIRTUAL_CREATED)) && (!entry->_is_extra_entry) )
			{
				double lower_bound = entry->_scan_position 
					- 2.0 * _conf.get(_STEP_DISTANCE_M);
				double upper_bound = entry->_scan_position
					+ 2.0 * _conf.get(_STEP_DISTANCE_M);
				
				if (entry->_state == PathEntry::_REAL)
				{
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(entry->_node_id) );
					const Node& node = tile->node(entry->_node_id);
					
/*					const Node::D_Edges& node_edges = node.edges();
					Node::D_Edges::const_iterator node_edges_iter = node_edges.begin();
					Node::D_Edges::const_iterator node_edges_iter_end = node_edges.end();
					
					std::vector<Node::Id> node_predecessors;
					std::vector<Node::Id> node_successors;
					for (; node_edges_iter != node_edges_iter_end; ++node_edges_iter)
					{
						if (_found_real_node_ids.count(node_edges_iter->get_next_node_id()) > 0)
						{
							if (node_edges_iter->is_outgoing_edge())
							{
								node_successors.push_back(
									node_edges_iter->get_next_node_id() );
							} else
							{
								node_predecessors.push_back(
									node_edges_iter->get_next_node_id() );
							}
						}
					}*/
					
					// Handle predecessors.
					Node::D_Edges::const_iterator id_iter
						= node.predecessors().begin();
					Node::D_Edges::const_iterator id_iter_end
						= node.predecessors().end();
					for (; id_iter != id_iter_end; ++id_iter)
					{
						if (!path_contains_id_in_interval(id_iter->get_next_node_id(), lower_bound, entry->_scan_position))
						{
							PathEntry* extra_entry = new PathEntry;
							extra_entry->_scan_position = entry->_scan_position - 0.5;
							extra_entry->_position_on_trace = -1.0;
							extra_entry->_node_id = id_iter->get_next_node_id();
							extra_entry->_state = PathEntry::_REAL;
							extra_entry->_is_extra_entry = true;
							
							insert_entry_into_path(extra_entry);
						}
					}
					
					// Handle successors.
					id_iter = node.successors().begin();
					id_iter_end = node.successors().end();
					for (; id_iter != id_iter_end; ++id_iter)
					{
						if (!path_contains_id_in_interval(id_iter->get_next_node_id(), entry->_scan_position, upper_bound))
						{
							PathEntry* extra_entry = new PathEntry;
							extra_entry->_scan_position = entry->_scan_position + 0.5;
							extra_entry->_position_on_trace = -1.0;
							extra_entry->_node_id = id_iter->get_next_node_id();
							extra_entry->_state = PathEntry::_REAL;
							extra_entry->_is_extra_entry = true;
							
							insert_entry_into_path(extra_entry);
						}
					}
					
				} else if (entry->_state == PathEntry::_VIRTUAL_FOUND)
				{
					if (entry->_virtual_node_id > _VIRTUAL_NODE_ID_OFFSET)
					{
						if (!path_contains_id_in_interval(entry->_virtual_node_id - 1,
							lower_bound, entry->_scan_position))
						{
							VirtualNode& virtual_node = _virtual_entries.point(
								static_cast<D_IndexType>(entry->_virtual_node_id - 1 - _VIRTUAL_NODE_ID_OFFSET));
							
							PathEntry* extra_entry = new PathEntry;
							extra_entry->_scan_position = entry->_scan_position - 0.5;
							extra_entry->_position_on_trace = virtual_node._position_on_trace;
							extra_entry->_direction = virtual_node.get_direction();
							extra_entry->_virtual_node_id = entry->_virtual_node_id - 1;
							extra_entry->_state = PathEntry::_VIRTUAL_FOUND;
							extra_entry->_is_extra_entry = true;
						
							insert_entry_into_path(extra_entry);
						}
					}
					
					if ( _virtual_entries.exists_point(
						entry->_virtual_node_id + 1 - _VIRTUAL_NODE_ID_OFFSET) )
					{
						if (!path_contains_id_in_interval(entry->_virtual_node_id + 1,
							entry->_scan_position, upper_bound))
						{
							VirtualNode& virtual_node = _virtual_entries.point(
								static_cast<D_IndexType>(entry->_virtual_node_id + 1 - _VIRTUAL_NODE_ID_OFFSET));
							
							PathEntry* extra_entry = new PathEntry;
							extra_entry->_scan_position = entry->_scan_position + 0.5;
							extra_entry->_position_on_trace = virtual_node._position_on_trace;
							extra_entry->_direction = virtual_node.get_direction();
							extra_entry->_virtual_node_id = entry->_virtual_node_id + 1;
							extra_entry->_state = PathEntry::_VIRTUAL_FOUND;
							extra_entry->_is_extra_entry = true;
						
							insert_entry_into_path(extra_entry);
						}
					}
				}
			}
		}
	}
	
	
	void
	TraceProcessor::merge(
		std::multimap<Node::Id, PathEntry*>& node_ids_entries_mapper,
		PathEntry* entry, PathEntry* corresponding_ex_virtual_entry)
	{
		#ifndef NDEBUG
			assert(!entry->has_state(PathEntry::_VIRTUAL_CREATED));
			
			if (entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
				assert(corresponding_ex_virtual_entry != 0);
				assert(corresponding_ex_virtual_entry->has_state(PathEntry::_REAL));
				assert(corresponding_ex_virtual_entry->_virtual_node_id == entry->_virtual_node_id);
			}
		#endif
		
		#warning this about that extra nodes.
		if ( true || !entry->_is_extra_entry )
		{
			// Get the relevant node id.
			Node::Id entry_node_id = _VIRTUAL_NODE_ID_OFFSET;
			if (entry->has_state(PathEntry::_REAL))
				entry_node_id = entry->_node_id;
			else
				entry_node_id = corresponding_ex_virtual_entry->_node_id;
			
			assert(entry_node_id < _VIRTUAL_NODE_ID_OFFSET);
			assert(verify_node_bearings(entry_node_id));
			
			TileCache::Pointer tile = _tile_cache->get(Node::tile_id(entry_node_id));
			const Node copied_original_node = tile->node(entry_node_id); // Intentionally copied!
			
			// Copying the node is very important here! In the following we
			// alter the original node, which will destroy information we need.
			// By copying the node we can access that data.
			
			// First, we delete the predecessors resp. successors from
			// original node. I use the copied_original_node for
			// initalising the iterators.
			// We will restore them at the end using the information from
			// the copied_original_node.
			if (true)
			{
				Node::D_Edges::const_iterator pred_iter
					= copied_original_node.predecessors().begin();
				Node::D_Edges::const_iterator pred_iter_end
					= copied_original_node.predecessors().end();
				for (; pred_iter != pred_iter_end; ++pred_iter)
				{
					disconnect_nodes(pred_iter->get_next_node_id(), entry_node_id);
				}
				
				Node::D_Edges::const_iterator succ_iter
					= copied_original_node.successors().begin();
				Node::D_Edges::const_iterator succ_iter_end
					= copied_original_node.successors().end();
				for (; succ_iter != succ_iter_end; ++succ_iter)
				{
					disconnect_nodes(entry_node_id, succ_iter->get_next_node_id());
				}
			}
			
			// Now, the original node has NO predecessor resp. successor. We can
			// move that node and add the predecessors resp. successors
			// afterwards. And we used the disconnect_nodes resp. connect_nodes
			// methods here which more convenient than doing that manually.
			
			// Now we will copy the original node again. But this time, we will
			// work with the new node. First we merge it with the point from
			// the trace, then we use the position information to move the
			// original node in the quadtree. Finally, we COPY this node into
			// the quadtree. So, to make it very clear: The information of THIS
			// node are relevant from now on!
			Node merged_node = tile->node(entry_node_id); // Intentionally copied!
			
			// Merge the node with the point from the trace.
			GeoCoordinate point_on_trace
				= _filtered_trace.gps_point_at(entry->_scan_position);
			merged_node.merge(point_on_trace);
			
			// Move the node to its new position. Afterwards we can use
			// new_node_id to access it.
			Node::Id old_node_id = entry_node_id;
			Node::Id new_node_id = entry_node_id;
			if (tile.write().move_node(old_node_id, merged_node))
			{
				// Successfully. The move_node above only actualise the position.
				// But some other members have changed too.
				// For example, we deleted the predecessors (see above). In the
				// node which is in the quadtree the predecessors are still
				// available. So we use the assignment operator which copys
				// "the rest" into that node.
				tile.write().node(old_node_id) = merged_node;
				new_node_id = old_node_id;
				
			} else
			{
				// merged_node does not belong to the tile. So remove it...
				tile.write().remove_node(old_node_id);
				
				// ... and insert it in the new tile.
				Tile::Id new_tile_id = Tile::get_tile_id_of(merged_node);
				TileCache::Pointer new_tile = _tile_cache->get(new_tile_id);
				if (new_tile == 0)
				{
					_tile_cache->insert(new_tile_id, new Tile(new_tile_id));
					new_tile = _tile_cache->get(new_tile_id);
				}
				
				new_node_id = new_tile.write().add_node(merged_node);
				
				// Change the node_ids of the entries using old_node_id
				std::vector<PathEntry*> entries_that_changed_node_id;
				
				typedef std::multimap<Node::Id, PathEntry*>::const_iterator
					D_FindIterator;
				
				std::pair<D_FindIterator, D_FindIterator> find_result
					= node_ids_entries_mapper.equal_range(old_node_id);
				
				D_FindIterator iter = find_result.first;
				D_FindIterator iter_end = find_result.second;
				for (; iter != iter_end; ++iter)
				{
					PathEntry* entry = iter->second;
					entry->_node_id = new_node_id;
					entries_that_changed_node_id.push_back(entry);
				}
				
				node_ids_entries_mapper.erase(old_node_id);
				
				for (int i = 0 ; i < entries_that_changed_node_id.size(); ++i)
				{
					node_ids_entries_mapper.insert(std::make_pair(
						new_node_id, entries_that_changed_node_id[i] ));
				}
			} // end if (tile.write().move_node(old_node_id, merged_node))
			
			// Now the node has moved. We have to set the predecessors resp.
			// successors. We can do that quite easily because we have copied
			// (and not referenced) the orginal node.
			if (true)
			{
				Node::D_Edges::const_iterator pred_iter
					= copied_original_node.predecessors().begin();
				Node::D_Edges::const_iterator pred_iter_end
					= copied_original_node.predecessors().end();
				for (; pred_iter != pred_iter_end; ++pred_iter)
				{
					connect_nodes(pred_iter->get_next_node_id(), new_node_id);
				}
				
				Node::D_Edges::const_iterator succ_iter
					= copied_original_node.successors().begin();
				Node::D_Edges::const_iterator succ_iter_end
					= copied_original_node.successors().end();
				for (; succ_iter != succ_iter_end; ++succ_iter)
				{
					connect_nodes(new_node_id, succ_iter->get_next_node_id());
				}
			}
			
			// The entries needs to know the new_node_id. Insert it into
			// the mapper. We only need to do that when the entris is
			// _VIRTUAL_FOUND. Otherwise, it is done at apply_module.
			if (entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
				node_ids_entries_mapper.insert(
					std::make_pair(new_node_id, entry));
			}
			
			// Set the new node id.
			entry->_node_id = new_node_id;
			entry->_state = PathEntry::_REAL;
			
			if (corresponding_ex_virtual_entry != 0)
				assert(corresponding_ex_virtual_entry->_node_id == new_node_id);
			
			assert(verify_node_bearings(new_node_id));
			
		} else
		{
			// Get the relevant node id.
			Node::Id entry_node_id = _VIRTUAL_NODE_ID_OFFSET;
			if (entry->has_state(PathEntry::_REAL))
				entry_node_id = entry->_node_id;
			else
				entry_node_id = corresponding_ex_virtual_entry->_node_id;
			
			// The entries needs to know the new_node_id. Insert it into
			// the mapper. We only need to do that when the entris is
			// _VIRTUAL_FOUND. Otherwise, it is done at apply_module.
			if (entry->has_state(PathEntry::_VIRTUAL_FOUND))
			{
				node_ids_entries_mapper.insert(
					std::make_pair(entry_node_id, entry));
			}
			
			// Set the new node id.
			entry->_node_id = entry_node_id;
			entry->_state = PathEntry::_REAL;
			
		} // end if (!entry->_is_extra_entry)
	}
	
	
	double
	TraceProcessor::minimal_direction_difference_between(Node::Id from_node_id,
		Node::Id over_node_id, Node::Id to_node_id) const
	{
		if (from_node_id == over_node_id)
			return 0.0;
		
		if (over_node_id == to_node_id)
			return 0.0;
		
		TileCache::Pointer from_node_tile = _tile_cache->get(
			Node::tile_id(from_node_id) );
		const Node& from_node = from_node_tile->node(from_node_id);
		
		TileCache::Pointer over_node_tile = _tile_cache->get(
			Node::tile_id(over_node_id) );
		const Node& over_node = over_node_tile->node(over_node_id);
		
		TileCache::Pointer to_node_tile = _tile_cache->get(
			Node::tile_id(to_node_id) );
		const Node& to_node = to_node_tile->node(to_node_id);
		
		Direction from2over_direction;
		if (from_node.has_successor(over_node_id))
		{
			from2over_direction
				= from_node.successor(over_node_id).get_direction();
			
			double bearing = from_node.bearing(over_node);
			assert(from2over_direction == Direction(from_node.bearing(over_node)));
		} else
		{
			from2over_direction = from_node.bearing(over_node);
		}
		
		Direction over2to_direction;
		if (over_node.has_successor(to_node_id))
		{
			over2to_direction = over_node.successor(to_node_id).get_direction();
			
			double bearing = over_node.bearing(to_node);
			assert(over2to_direction == Direction(over_node.bearing(to_node)));
		} else
		{
			over2to_direction = over_node.bearing(to_node);
		}
		
//		std::cout << "dirs: " << from2over_direction << ", " << over2to_direction << std::endl;
		
		return from2over_direction.angle_difference(over2to_direction);
	}
	
	
/*	double
	TraceProcessor::length_m_of(const T_Path& path) const
	{
		return
			(path.rbegin()->second->_scan_position - path.begin()->second->_scan_position);
	}*/
	
	
/*	bool
	TraceProcessor::path_contains_entry(const PathEntry* entry) const
	{
		bool already_in_path = false;
		std::pair<D_Path::const_iterator, D_Path::const_iterator> found_iters
			= _path.equal_range(entry->_scan_position);
		while ( (!already_in_path)
			&& (found_iters.first != found_iters.second) )
		{
			if (*entry == *(found_iters.first->second)) // Do not rely on operator==!!!!!!!!!!
				already_in_path = true;
			
			++(found_iters.first);
		}
		
		return already_in_path;
	}*/
	
	
	bool
	TraceProcessor::path_contains_id_in_interval(Node::Id id,
		double interval_start, double interval_end) const
	{
		assert(interval_start <= interval_end);
		
		D_Path::const_iterator iter = _path.lower_bound(interval_start);
		D_Path::const_iterator iter_end = _path.upper_bound(interval_end);
		for (; iter != iter_end; ++iter)
		{
			PathEntry* entry = iter->second;
			if ( (entry->_node_id == id) || (entry->_virtual_node_id == id) )
				return true;
		}
		
		return false;
	}
	
	
	bool
	TraceProcessor::path_is_valid(PathEntry* const start_entry,
		bool should_repair)
	{
		// Mark used entries.
//		if (true)
//		{
//			PathEntry* entry = start_entry;
//			while (entry != 0)
//			{
//				entry->_is_used = true;
//				entry = entry->_connection;
//			}
//		}
		
		D_NodeIds used_virtual_found_node_ids;
		
		// Fill used_virtual_found_node_ids.
		if (true)
		{
			const PathEntry* test_entry = start_entry;
			while (test_entry != 0)
			{
				if (test_entry->has_state(PathEntry::_VIRTUAL_FOUND))
				{
					used_virtual_found_node_ids.insert(
						test_entry->_virtual_node_id);
				}
				
				test_entry = test_entry->_connection;
			}
		}
		
		// Erase available virtual created nodes from
		// used_virtual_found_node_ids.
		if (true)
		{
			const PathEntry* test_entry = start_entry;
			while ( (test_entry != 0)
				&& (used_virtual_found_node_ids.size() > 0) )
			{
				if (test_entry->has_state(PathEntry::_VIRTUAL_CREATED))
				{
					used_virtual_found_node_ids.erase(
						test_entry->_virtual_node_id);
				}
				
				test_entry = test_entry->_connection;
			}
		}
		
		if (used_virtual_found_node_ids.size() > 0)
		{
			if (should_repair)
			{
				bool successfully_repaired = true;
				
				PathEntry* previous_entry = start_entry;
				PathEntry* entry = previous_entry->_connection;
				
				assert(used_virtual_found_node_ids.count(
					previous_entry->_virtual_node_id) == 0);
				
				while (entry != 0)
				{
					assert(used_virtual_found_node_ids.count(
						previous_entry->_virtual_node_id) == 0);
					
					bool found_invalid_entry = false;
					
					while (used_virtual_found_node_ids.count(
						entry->_virtual_node_id) > 0)
					{
						found_invalid_entry = true;
						
						assert(entry->_connection != 0);
						entry = entry->_connection;
					}
					
					if (found_invalid_entry)
					{
						if ( !search_and_use_alterative_path_entries(
							previous_entry, entry) )
						{
							successfully_repaired = false;
						}
					}
					
					previous_entry = entry;
					entry = entry->_connection;
				}
				
				if (successfully_repaired)
					return true;
			}
			
			if (should_repair)
			{
				int counter = 0;
				
				PathEntry* previous_entry = start_entry;
				PathEntry* entry = previous_entry->_connection;
				while(entry != 0)
				{
					if (used_virtual_found_node_ids.count(entry->_virtual_node_id) > 0)
					{
						previous_entry->_connection = entry->_connection;
						
						++counter;
						double distance = entry->_scan_position
							- previous_entry->_scan_position;
						
						if ( (!_tile_manager->get_optimisation_mode())
							&&
							((counter > _MAX_INVALID_ENTRIES_COUNTER)
							|| (distance > _MAX_INVALID_ENTRIES_DISTANCE)) )
						{
							previous_entry->_do_not_use_connection = true;
						}
						
					} else
					{
						previous_entry = entry;
						
						counter = 0;
					}
					
					entry = entry->_connection;
				}
			}
		}
		
/*		if (used_virtual_found_node_ids.size() > 0)
		{
			std::cout << "Left used_virtual_found_node_ids: ";
			D_NodeIds::iterator iter = used_virtual_found_node_ids.begin();
			D_NodeIds::iterator iter_end = used_virtual_found_node_ids.end();
			for (; iter != iter_end; ++iter)
			{
				std::cout << *iter << ", ";
			}
			std::cout << std::endl;
		} else
		{
			std::cout << "NO left used_virtual_found_node_ids." << std::endl;
		}*/
		
		return (used_virtual_found_node_ids.size() == 0);
	}
	
	
//	Node::Id
//	TraceProcessor::register_new_node(const Node& node)
	Node::Id
	TraceProcessor::register_new_node(const GeoCoordinate& gc)
	{
		if (gc[0] < 30.0 || gc[1] < 5.0)
		{
			std::cout << gc << std::endl;
			assert(false);
		}
		
		Tile::Id tile_id = Tile::get_tile_id_of(gc);
		TileCache::Pointer tile = _tile_cache->get(tile_id);
		if (tile == 0)
		{
			_tile_cache->insert(tile_id, new Tile(tile_id));
			tile = _tile_cache->get(tile_id);
		}
		_protocol._used_tile_ids.insert(tile->get_id());
		
		Node node(gc);
		Node::Id node_id = tile.write().add_node(node);
		
		return node_id;
	}
	
	
	void
	TraceProcessor::run()
	{
		mlog(MLog::info, "TraceProcessor") << "Started (" << _id << ").\n";
	
		_filtered_trace.precompute_data();
		_filtered_trace.calculate_directions();
		
		double filtered_trace_duration = _filtered_trace.back().get_time()
			- _filtered_trace.front().get_time();
			
		mlog(MLog::debug, "TraceProcessor") << "Processes a "
			<< _filtered_trace.length_m() << "m (and "
			<< filtered_trace_duration << "s) long trace.\n";
		
		/* Add trace meters to statistic... */
		pubsub::Service<double>* meters_service
			= _service_list->find_service<double>("statistics.received_filtered_trace_meters");
		
		if (meters_service)
			meters_service->receive(_filtered_trace.length_m());
		/* done. */
		
		/* Add trace time to statistic... */
		pubsub::Service<double>* times_service
			= _service_list->find_service<double>("statistics.received_filtered_trace_times");
		
		if (times_service)
			times_service->receive(filtered_trace_duration);
		/* done. */
		
		// Prepare the virtual nodes.
/*		double scan_position_m = 0.0;
		while (scan_position_m < _filtered_trace.length_m())
		{
			VirtualNode virtual_node(
				_filtered_trace.gps_point_at(scan_position_m) );
			virtual_node._position_on_trace = scan_position_m;
			
			D_IndexType index = _virtual_entries.add_point(virtual_node);
			assert(index == static_cast<D_IndexType>(
				scan_position_m / _conf.get(_STEP_DISTANCE_M)) );
			
			scan_position_m += _conf.get(_STEP_DISTANCE_M);
		}*/
		
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		// THE WHOLE ALGORITHM FOLLOWS HERE. DOCUMENTATE IT!!!!!
		scan_module();
		
//		std::cout << _path << std::endl;
		if (search_and_set_beginning() && search_and_set_ending())
		{
			bool beginning_found = false;
			bool ending_found = false;
			if (true)
			{
				D_Path::iterator iter = _path.begin();
				for (; !ending_found && iter != _path.end(); ++iter)
				{
					if (iter->second->_is_beginning)
						beginning_found = true;
					
					if (iter->second->_is_destination)
						ending_found = true;
				}
			}
			
			if (beginning_found)
			{
				insert_extra_entries_into_path();
				ai_module();
				
				// Set _path's backward pointers.
				if (true)
				{
					// Find the beginning.
					D_Path::iterator iter = _path.begin();
					D_Path::iterator iter_end = _path.end();
					while ( (iter != iter_end) && (!iter->second->_is_beginning) )
						++iter;
					
					assert(iter != iter_end);
					assert(iter->second->_is_beginning);
					
					// Set them.
					PathEntry* previous_entry = iter->second;
					PathEntry* entry = previous_entry->_connection;
					while (entry != 0)
					{
						entry->_backward_connection = previous_entry;
						
						previous_entry = entry;
						entry = previous_entry->_connection;
					}
				}
				
				std::map<Node::Id, CrossingItem> excluded_crossings;
				std::multimap<Node::Id, CrossingItem> crossings_mapper;
				
				// Get the crossings that are already in the map.
				if (true)
				{
					build_crossings_mapper(crossings_mapper,
						excluded_crossings, true);
//					std::cout << "# = " << crossings_mapper.size() << "! ";
					
					std::multimap<Node::Id, CrossingItem>::const_iterator iter
						= crossings_mapper.begin();
					std::multimap<Node::Id, CrossingItem>::const_iterator iter_end
						= crossings_mapper.end();
					for (; iter != iter_end; ++iter)
					{
						CrossingItem crossing_item;
						crossing_item._predecessors = iter->second._predecessors;
						crossing_item._successors = iter->second._successors;
						crossing_item._has_new_predecessors = false;
						crossing_item._has_new_successors = false;
//						crossing_item._previous_entry = 0;
						crossing_item._entry = 0;
						
						std::pair<std::map<Node::Id, CrossingItem>::iterator, bool>
							insert_result;
						
						insert_result = excluded_crossings.insert(
							std::make_pair(iter->first, crossing_item));
						
						#ifndef NDEBUG
							if (insert_result.second == false)
							{
								assert(insert_result.first->second._predecessors
									== crossing_item._predecessors);
								assert(insert_result.first->second._successors
									== crossing_item._successors);
							}
						#endif
					}
					
					crossings_mapper.clear();
				}
				
				
				bool applied_successfully = apply_module();
//				_protocol.path_after_apply(_path);
				
				if (applied_successfully)
				{
					if (_tile_manager->get_optimisation_mode() == false)
					{
						build_crossings_mapper(crossings_mapper, excluded_crossings);
//						std::cout << crossings_mapper.size() << " -> ";
						handle_loops(crossings_mapper);
						
						build_crossings_mapper(crossings_mapper, excluded_crossings);
//						std::cout << crossings_mapper.size() << " -> ";
						handle_double_ways(crossings_mapper);
						
						build_crossings_mapper(crossings_mapper, excluded_crossings);
//						std::cout << crossings_mapper.size() << " -> ";
						handle_parallel_autobahn_ways(crossings_mapper);
						
						build_crossings_mapper(crossings_mapper, excluded_crossings);
//						std::cout << crossings_mapper.size() << " -> ";
						smooth_connections(crossings_mapper);
					}
					
					build_crossings_mapper(crossings_mapper, excluded_crossings);
//					std::cout << crossings_mapper.size() << " -> ";
					create_extra_nodes_on_connections(crossings_mapper);
					
//					build_crossings_mapper(crossings_mapper, excluded_crossings);
//					std::cout << crossings_mapper.size() << std::endl;
				}
				
			} else
			{
				mlog(MLog::info, "TraceProcessor") << "ending found before beginning. "
					<< "will not use this trace.\n";
			}
		}
		
		serialize_protocol();
		
		// Okay, everything is ready. Now, we have to delete the PathEntrys.
/*		bool found_first_start = false;
		D_Path::iterator iter = _protocol._path.begin();
		D_Path::iterator iter_end = _protocol._path.end();
		for (; iter != iter_end; ++iter)
		{
			PathEntry* a = iter->second;
			if (!found_first_start)
			{
				found_first_start = iter->second->_is_beginning;
				delete iter->second;
			} else
			{
				if (!iter->second->_is_beginning)
				{
					delete iter->second;
				}
			}
		}
*/		
		
		// Write a log and finish.
		mlog(MLog::info, "TraceProcessor") << "Finished (" << _id << ").\n";
		_tile_manager->trace_processor_finished(_id);
		
//		if (this->_id == 49)
//			std::cout << "TP READY!";
	}
	
	
	void
	TraceProcessor::scan_module()
	{
		// Build list with step distances.
		std::list<double> step_distances;
		if (true)
		{
			double position_m = 0.0;
			
			while (position_m + _conf.get(_STEP_DISTANCE_M)
				< _filtered_trace.length_m())
			{
				double step_distance_m = _conf.get(_STEP_DISTANCE_M);
				
				bool exit_loop = false;
				while (!exit_loop)
				{
					double max_curvature = 0.0;
					double after_iter_meters = position_m;
					while (after_iter_meters <= position_m + step_distance_m)
					{
						// Quite tricky. gps_points... return the needed
						// iterator AND "increments" the after_iter_meters!
						FilteredTrace::const_iterator before_iter;
						_filtered_trace.gps_points_before_and_after(
							after_iter_meters, &before_iter, 0, 0,
							&after_iter_meters);
						
						// I don't like -O3 optimisation with g++ 4.0.3
						// Seems to use faster but more inaccurate fp ops.
						// after_iter_meters does not point to the next real
						// GPSPoint in _filtered_trace. Probably some
						// micrometers vanished...
						// So, we need a hack. I just add 10 cm. Should work!
						#warning opt hack!
						after_iter_meters += 0.1;
						
						double curvature
							= fabs( _filtered_trace.curvature_at(before_iter) );
						if (curvature > max_curvature)
							max_curvature = curvature;
					}
					
					if (max_curvature * step_distance_m <= _MAX_CURVATURE)
						exit_loop = true;
					else
						step_distance_m /= _STEP_DISTANCE_FACTOR;
					
					if (step_distance_m <= _MIN_STEP_DISTANCE_M)
						exit_loop = true;
				}
				
				assert(step_distance_m >= _MIN_STEP_DISTANCE_M);
				assert(step_distance_m <= _conf.get(_STEP_DISTANCE_M));
				
				step_distances.push_back(step_distance_m);
				position_m += step_distance_m;
			}
			
			step_distances.push_back(_conf.get(_STEP_DISTANCE_M));
			
//			std::list<double>::const_iterator iter = step_distances.begin();
//			for (; iter != step_distances.end(); ++iter)
//				std::cout << *iter << ", ";
//			std::cout << std::endl;
		}
		
		// step distances should not differ too much.
		if (true)
		{
			std::list<double>::iterator iter = step_distances.begin();
			assert(iter != step_distances.end());
			++iter;
			
			for (; iter != step_distances.end(); ++iter)
			{
				std::list<double>::iterator prev_iter = iter;
				--prev_iter;
				
				bool exit_loop = false;
				while (!exit_loop)
				{
					if (*prev_iter < *iter / _STEP_DISTANCE_FACTOR)
					{
						*iter = *iter / _STEP_DISTANCE_FACTOR;
						iter = step_distances.insert(iter, *iter);
						
					} else if (*prev_iter > *iter * _STEP_DISTANCE_FACTOR)
					{
						*prev_iter = *prev_iter / _STEP_DISTANCE_FACTOR;
						step_distances.insert(prev_iter, *prev_iter);
					} else
					{
						exit_loop = true;
					}
				}
			}
		}
		
		// Insert the first virtual node into _virtual_entries.
		// I use an if-clause here to avoid name conflicts.
		if (true)
		{
			VirtualNode virtual_created_node(_filtered_trace.gps_point_at(0.0));
			virtual_created_node._position_on_trace = 0.0;
			
			D_IndexType virtual_created_node_id = _virtual_entries.add_point(virtual_created_node);
			assert(virtual_created_node_id == static_cast<D_IndexType>(0));
			
			PathEntry* virtual_created_entry = new PathEntry;
			virtual_created_entry->_scan_position = 0.0;
			virtual_created_entry->_position_on_trace = virtual_created_node._position_on_trace;
			virtual_created_entry->_direction = virtual_created_node.get_direction();
			virtual_created_entry->_virtual_node_id = virtual_created_node_id + _VIRTUAL_NODE_ID_OFFSET;
			virtual_created_entry->_state = PathEntry::_VIRTUAL_CREATED;
			
			insert_entry_into_path(virtual_created_entry);
		}
		
		// Define some useful variables.
		std::list<double>::const_iterator current_step_distance_iter
			= step_distances.begin();
		double scan_position_m = *current_step_distance_iter;
		GPSPoint previous_gps_point_on_trace;
		GPSPoint current_gps_point_on_trace = _filtered_trace.gps_point_at(0.0);
		
//		D_NodeIds encountered_node_ids;
//		std::multimap<double, Node::Id> encountered_node_ids_positions;
		
//		FixedSizeQueue<double> last_virtual_node_directions(
//			static_cast<unsigned int>( ceil(1.0 +
//				(_conf.get(_SEARCH_MAX_DISTANCE_M)
//				/ _conf.get(_STEP_DISTANCE_M))) ),
//			false, false);
		
		// The main loop. The trace is processed as long as the end is not
		// reached.
		while (scan_position_m < _filtered_trace.length_m())
		{
			// Generate a virtual node.
			VirtualNode virtual_created_node(
				_filtered_trace.gps_point_at(scan_position_m) );
			virtual_created_node._position_on_trace = scan_position_m;
			
			// Insert it into the _virtual_entries.
			D_IndexType virtual_created_node_id = _virtual_entries.add_point(virtual_created_node);
//			assert(virtual_created_node_id == static_cast<D_IndexType>(scan_position_m / _conf.get(_STEP_DISTANCE_M)));
			
			// And insert it into the _path.
			PathEntry* virtual_created_entry = new PathEntry;
			virtual_created_entry->_scan_position = scan_position_m;
			virtual_created_entry->_position_on_trace = virtual_created_node._position_on_trace;
			virtual_created_entry->_direction = virtual_created_node.get_direction();
			virtual_created_entry->_virtual_node_id = virtual_created_node_id + _VIRTUAL_NODE_ID_OFFSET;
			virtual_created_entry->_state = PathEntry::_VIRTUAL_CREATED;
			
			insert_entry_into_path(virtual_created_entry);
			
			// Set the GPSPoints for the search segment.
			previous_gps_point_on_trace
				= current_gps_point_on_trace;
			current_gps_point_on_trace
				= _filtered_trace.gps_point_at(scan_position_m);
			
			
			#warning Can use something like this to shorten insertion time. \
				A data structure containing Node::Id, interval [start, end]...
			// Cut down the encountered_node_ids.
/*			double cut_position = scan_position_m - _conf.get(_STEP_DISTANCE_M)
				- _conf.get(_SEARCH_MAX_DISTANCE_M);
			
			std::multimap<double, Node::Id>::iterator cut_iter
				= encountered_node_ids_positions.begin();
			std::multimap<double, Node::Id>::iterator lower_bound_iter
				= encountered_node_ids_positions.lower_bound(cut_position);
			for (;cut_iter != lower_bound_iter; ++cut_iter)
				encountered_node_ids.erase(cut_iter->second);
			
			encountered_node_ids_positions.erase(
				encountered_node_ids_positions.begin(), lower_bound_iter);
*/			
			// Perform cluster nodes search.
			std::list<Node::Id> cluster_nodes;
			calculate_cluster_nodes(previous_gps_point_on_trace,
				current_gps_point_on_trace, cluster_nodes);
//			cluster_nodes.clear();
//			int cluster_nodes_size = cluster_nodes.size();
//			std::cout << scan_position_m << ": #cluster_nodes = " << cluster_nodes.size() << std::endl;
			
//			bool found_the_one_and_only_virtual_node = false;
			// Insert the cluster nodes into the path.
			std::list<Node::Id>::iterator iter = cluster_nodes.begin();
			std::list<Node::Id>::iterator iter_end = cluster_nodes.end();
			for (; iter != iter_end; ++iter)
			{
				PathEntry* new_entry = 0;
				
				if (PathEntry::is_virtual_node_id(*iter))
				{
					VirtualNode& virtual_node = _virtual_entries.point(
						static_cast<D_IndexType>(*iter - _VIRTUAL_NODE_ID_OFFSET));
					
					assert(virtual_node._position_on_trace <= scan_position_m);
					
					if (virtual_node._position_on_trace < scan_position_m)
					{
						new_entry = new PathEntry;
						new_entry->_scan_position = scan_position_m;
						new_entry->_position_on_trace = virtual_node._position_on_trace;
						new_entry->_direction = virtual_node.get_direction();
						new_entry->_virtual_node_id = *iter;
						new_entry->_state = PathEntry::_VIRTUAL_FOUND;
					}
					
/*					
//					if (encountered_node_ids.count(*iter) == 0)
//					{
						new_entry = new PathEntry;
						new_entry->_scan_position = scan_position_m;
						new_entry->_position_on_trace = virtual_node._position_on_trace;
#warning new_entry->_direction: wofuer?????
						new_entry->_direction = virtual_node.get_direction();
						new_entry->_virtual_node_id = *iter;
						
						if (virtual_node._position_on_trace == scan_position_m)
						{
							// Already inserted at the beginning of the main
							// loop. Do not insert it twice!
							delete new_entry;
							new_entry = 0;
//							new_entry->_state = PathEntry::_VIRTUAL_CREATED;
//							found_the_one_and_only_virtual_node = true;
						} else if (virtual_node._position_on_trace < scan_position_m)
						{
								new_entry->_state = PathEntry::_VIRTUAL_FOUND;
						} else
						{
							delete new_entry;
							new_entry = 0;
						}
						
//					} else // if (encountered_node_ids.count(*iter) > 0)
//					{
//						// last_virtual_node_directions ...
//					}
*/					
					
				} else // if (!PathEntry::is_virtual_node_id(*iter))
				{
//					if (encountered_node_ids.count(*iter) == 0)
//					{
						new_entry = new PathEntry;
						new_entry->_scan_position = scan_position_m;
						new_entry->_position_on_trace = -1.0;
						new_entry->_node_id = *iter;
						new_entry->_state = PathEntry::_REAL;
						
						_found_real_node_ids.insert(*iter);
//					}
				}
				
				if (new_entry != 0)
				{
					// calculate the (exact) perpendicular on the trace and
					// associaNode&te it to new_entry._position
//					std::pair<bool, double> result
//						= calculate_perpendicular_on_trace(new_entry);
//					if (result.first)
//					{
//						new_entry->_scan_position = result.second;
//					}
					
//					if (new_entry->_scan_position < scan_position_m
//						- _conf.get(_STEP_DISTANCE_M)
//						/*- _conf.get(_SEARCH_MAX_DISTANCE_M)*/ )
//					{
//						new_entry->_scan_position = scan_position_m
//							- _conf.get(_STEP_DISTANCE_M)
//							/*- _conf.get(_SEARCH_MAX_DISTANCE_M)*/;
//					}
//					
//					if (new_entry->_scan_position > scan_position_m)
//					{
//						new_entry->_scan_position = scan_position_m;
//					}
					
//					encountered_node_ids.insert(*iter);
//					encountered_node_ids_positions.insert(std::make_pair(
//						new_entry->_scan_position, *iter));
					
					bool should_be_inserted
						= find_and_set_most_fitting_position_on_trace_for(new_entry);
					
					Node::Id new_entry_id;
					if (new_entry->has_state(PathEntry::_REAL))
						new_entry_id = new_entry->_node_id;
					else
						new_entry_id = new_entry->_virtual_node_id;
					
					should_be_inserted = should_be_inserted
						&& !path_contains_id_in_interval(new_entry_id,
							new_entry->_scan_position - 2.0,
							new_entry->_scan_position + 2.0);
						
					if (should_be_inserted)
					{
						insert_entry_into_path(new_entry);
					} else
					{
						delete new_entry;
						new_entry = 0;
					}
				}
				
/*				if (encountered_node_ids.count(*iter) == 0)
				{
					PathEntry* new_entry = 0;
					
					if (PathEntry::is_virtual_node_id(*iter))
					{
						VirtualNode& virtual_node
							= _virtual_entries.point(static_cast<D_IndexType>(
								*iter - _VIRTUAL_NODE_ID_OFFSET));
						
						if (virtual_node._position_on_trace == scan_position_m)
						{
							found_the_one_and_only_virtual_node = true;
							
//							last_virtual_node_directions.push(
//								virtual_node.get_direction());
							
							new_entry = new PathEntry;
							new_entry->_scan_position = scan_position_m;
							new_entry->_position_on_trace = virtual_node._position_on_trace;
							new_entry->_direction = virtual_node.get_direction();
							new_entry->_virtual_node_id = *iter;
							new_entry->_state = PathEntry::_VIRTUAL_CREATED;
						} else
						{
							if ( virtual_node._position_on_trace < scan_position_m
								- _conf.get(_STEP_DISTANCE_M)
								- _conf.get(_SEARCH_MAX_DISTANCE_M) )
							{
								if (encountered_node_ids.count(*iter) == 0)
								{
									new_entry = new PathEntry;
									new_entry->_scan_position = scan_position_m;
									new_entry->_position_on_trace = virtual_node._position_on_trace;
									new_entry->_direction = virtual_node.get_direction();
									new_entry->_virtual_node_id = *iter;
									new_entry->_state = PathEntry::_VIRTUAL_FOUND;
//								std::cout << "scanning: vf1: " << *new_entry << std::endl;
								}
								
							} else if (virtual_node._position_on_trace <= scan_position_m)
							{
								// Calculate the latest direction difference
								// of the encountered virtual (created) nodes.
								int lvnd_size = last_virtual_node_directions.size();
								double direction_difference = 0.0;
								for (int i = 0; i < lvnd_size - 1; ++i)
								{
									direction_difference += 
										last_virtual_node_directions[i]
										- last_virtual_node_directions[i + 1];
								}
								
								if (direction_difference > 1.5 * PI)
								{
									new_entry = new PathEntry;
									new_entry->_scan_position = scan_position_m;
									new_entry->_position_on_trace = virtual_node._position_on_trace;
									new_entry->_direction = virtual_node.get_direction();
									new_entry->_virtual_node_id = *iter;
									new_entry->_state = PathEntry::_VIRTUAL_FOUND;
//								std::cout << "scanning: vf2: " << *new_entry << std::endl;
								}
							}
						}
					} else
					{
						new_entry = new PathEntry;
						new_entry->_scan_position = scan_position_m;
						new_entry->_position_on_trace = -1.0;
						new_entry->_node_id = *iter;
						new_entry->_state = PathEntry::_REAL;
					}
					
					if (new_entry != 0)
					{
						// calculate the (exact) perpendicular on the trace and
						// associate it to new_entry._position
						new_entry->_scan_position
							= calculate_perpendicular_on_trace(new_entry);
						
						if (new_entry->_scan_position < scan_position_m
							- _conf.get(_STEP_DISTANCE_M)
							/*- _conf.get(_SEARCH_MAX_DISTANCE_M)**** )
						{
							new_entry->_scan_position = scan_position_m
								- _conf.get(_STEP_DISTANCE_M)
								/*- _conf.get(_SEARCH_MAX_DISTANCE_M)****;
						}
						
						encountered_node_ids.insert(*iter);
						encountered_node_ids_positions.insert(std::make_pair(
							new_entry->_scan_position, *iter));
						
						_path.insert(
							std::make_pair(new_entry->_scan_position, new_entry));
					}
				} // end if (encountered_node_ids.count(*iter) == 0) */
			}
			
/*			if (!found_the_one_and_only_virtual_node)
			{
				std::cout << scan_position_m << std::endl;
				D_NodeIds::iterator a = encountered_node_ids.begin();
				D_NodeIds::iterator a_end = encountered_node_ids.end();
				for (; a != a_end; ++a)
					std::cout << *a << " ";
				std::cout << std::endl;
				
				std::multimap<double, Node::Id>::iterator b = encountered_node_ids_positions.begin();
				std::multimap<double, Node::Id>::iterator b_end = encountered_node_ids_positions.end();
				for (; b != b_end; ++b)
					std::cout << "(" << b->first << "," << b->second << ") ";
				std::cout << std::endl;
				
	//			assert(false);
			}*/
			
/*			if (search_unambigious_ending_sequence())
			{			assert(*current_step_distance_iter >= _MIN_STEP_DISTANCE_M);
			assert(*current_step_distance_iter <= _conf.get(_STEP_DISTANCE_M));

				ai_module();
				apply_module();
			}*/
			
			assert(*current_step_distance_iter >= _MIN_STEP_DISTANCE_M);
			assert(*current_step_distance_iter <= _conf.get(_STEP_DISTANCE_M));
			
			++current_step_distance_iter;
			
			assert(*current_step_distance_iter >= _MIN_STEP_DISTANCE_M);
			assert(*current_step_distance_iter <= _conf.get(_STEP_DISTANCE_M));
			
			scan_position_m += *current_step_distance_iter;
		}
	}
	
	
/*	bool
	TraceProcessor::search_unambigious_ending_sequence()
	{
		// Returns true, iff the path ends with an unambigious sequence.
		// So either a sequence of virtual nodes occur or a row of connected
		// real nodes is encountered.
		// If this method returns true, a PathEntry is marked to be the
		// destination. That is needed for the following ai_module. That
		// PathEntry is located at most _conf.get(_SUFFICIENT_PATH_LENGTH)/2
		// before the end of the unambigious sequence.
		
		PathEntry* first_real_entry = 0;
		PathEntry* last_real_entry = 0;
		PathEntry* first_virtual_entry = 0; // This has to be _VIRTUAL_CREATED!
		PathEntry* last_virtual_entry = 0;  // This has to be _VIRTUAL_CREATED!
		
		// CAUTION: The iters are travered in backward manner!
		D_Path::iterator iter = _path.end();
		D_Path::iterator iter_end = _path.begin();
		
		bool exit_loop = false;
		while (!exit_loop && iter != iter_end)
		{
			--iter;
			PathEntry* iter_entry = iter->second;
			
			// These if-clauses are not as compact as they could be. But in
			// this form it is much easier to understand what is going on.
			// It is a bit tricky: We search either for a sequence of virtual
			// nodes. This is done in the first part.
			// Or we need a sequence of real nodes. But keep in mind that there
			// is a virtual node every now and then. If we search for real
			// nodes we can omit the virtual ones (This is the path:
			// iter_entry->is_virtual() but !sequence_of_virtual_nodes).
			
			if (iter_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				if (first_virtual_entry == 0)
				{
					assert(last_virtual_entry == 0);
					last_virtual_entry = iter_entry;
				} else
				{
					if (!first_virtual_entry->is_virtual_successor_of(*iter_entry))
					{
						std::cout << "Search unambigious ending sequence" << std::endl;
						std::cout << "The crashing entries: first = " << first_virtual_entry << ", iter = " << iter_entry << std::endl;
						std::cout << "The _path:" << std::endl;
						std::cout << _path << std::endl;
					}
					assert(first_virtual_entry->is_virtual_successor_of(*iter_entry));
				}
				
				first_virtual_entry = iter_entry;
				
			} else if (iter_entry->has_state(PathEntry::_REAL))
			{
				first_virtual_entry = 0;
				last_virtual_entry = 0;
				
				if (first_real_entry == 0)
				{
					assert(last_real_entry == 0);
					last_real_entry = iter_entry;
				} else
				{
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(iter_entry->_node_id) );
					const Node* node = &( tile->node(iter_entry->_node_id) );
					if( !node->is_reachable(first_real_entry->_node_id) )
					{
						last_real_entry = iter_entry;
					}
				}
				
				first_real_entry = iter_entry;
				
			} else // if (iter_entry->has_state(PathEntry::_VIRTUAL_FOUND)
			{
				
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				if (first_virtual_entry != 0)
				{
					assert(last_virtual_entry != 0);
					
/*					Node::Id virtual_node_id = iter_entry->_virtual_node_id;
					
					Node::Id lower_bound = first_virtual_entry->_virtual_node_id
						- static_cast<Node::Id>( ceil(
							(_conf.get(_STEP_DISTANCE_M)
								+ 1.1 * _conf.get(_SEARCH_MAX_DISTANCE_M))
							/ _conf.get(_STEP_DISTANCE_M)) );
					if ((virtual_node_id < lower_bound) ||
						(iter_entry->_scan_position != first_virtual_entry->_scan_position - _conf.get(_STEP_DISTANCE_M)) )
					{*****
						first_virtual_entry = 0;
						last_virtual_entry = 0;
					/*}****
				}
			}
			
			if (first_virtual_entry != 0)
			{
				assert(last_virtual_entry != 0);
				
				exit_loop = sequence_found(iter, first_virtual_entry,
					last_virtual_entry, PathEntry::_VIRTUAL_CREATED);
			} else
			{
				if (first_real_entry != 0)
				{
					assert(last_real_entry != 0);
					
					exit_loop = sequence_found(iter, first_real_entry,
						last_real_entry, PathEntry::_REAL);
				}
			}
		} // end while (!exit_loop && iter != iter_end)
		
		return exit_loop;
	}*/
	
	
	bool
	TraceProcessor::search_and_set_beginning()
	{
		// Returns true, iff the path ends with an unambigious sequence.
		// So either a sequence of virtual nodes occur or a row of connected
		// real nodes is encountered.
		// If this method returns true, a PathEntry is marked to be the
		// destination. That is needed for the following ai_module. That
		// PathEntry is located at most _conf.get(_SUFFICIENT_PATH_LENGTH)/2
		// before the end of the unambigious sequence.
		
		// Don't be confused be the names of the variables. It's just copied and
		// pasted!
		
		PathEntry* first_real_entry = 0;
		PathEntry* last_real_entry = 0;
		D_Path::iterator last_real_entry_iter;
		
		PathEntry* first_virtual_entry = 0; // This has to be _VIRTUAL_CREATED!
		PathEntry* last_virtual_entry = 0;  // This has to be _VIRTUAL_CREATED!
		D_Path::iterator last_virtual_entry_iter;
		
		// CAUTION: The iters are travered in FORWARD=NORMAL manner!
		D_Path::iterator iter = _path.begin();
		D_Path::iterator iter_end = _path.end();
		
		bool exit_loop = false;
		for (;!exit_loop && iter != iter_end; ++iter)
		{
			PathEntry* iter_entry = iter->second;
			
			// These if-clauses are not as compact as they could be. But in
			// this form it is much easier to understand what is going on.
			// It is a bit tricky: We search either for a sequence of virtual
			// nodes. This is done in the first part.
			// Or we need a sequence of real nodes. But keep in mind that there
			// is a virtual node every now and then. If we search for real
			// nodes we can omit the virtual ones (This is the path:
			// iter_entry->is_virtual() but !sequence_of_virtual_nodes).
			
			if (iter_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				if (first_virtual_entry == 0)
				{
					assert(last_virtual_entry == 0);
					last_virtual_entry = iter_entry;
					last_virtual_entry_iter = iter;
				} else
				{
					if (!iter_entry->is_virtual_successor_of(*first_virtual_entry))
					{
						std::cout << "search_and_set_beginning" << std::endl;
						std::cout << "The crashing entries: first = " << first_virtual_entry << ", iter = " << iter_entry << std::endl;
						std::cout << "The _path:" << std::endl;
						std::cout << _path << std::endl;
					}
					assert(iter_entry->is_virtual_successor_of(*first_virtual_entry));
				}
				
				first_virtual_entry = iter_entry;
				
			} else if (iter_entry->has_state(PathEntry::_REAL))
			{
				first_virtual_entry = 0;
				last_virtual_entry = 0;
				
				if (first_real_entry == 0)
				{
					assert(last_real_entry == 0);
					last_real_entry = iter_entry;
					last_real_entry_iter = iter;
				} else
				{
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(iter_entry->_node_id) );
					const Node* node = &( tile->node(iter_entry->_node_id) );
					if( !node->has_predecessor(first_real_entry->_node_id) )
					{
						last_real_entry = iter_entry;
						last_real_entry_iter = iter;
					}
				}
				
				first_real_entry = iter_entry;
				
			} else // if (iter_entry->has_state(PathEntry::_VIRTUAL_FOUND)
			{
				
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				if (first_virtual_entry != 0)
				{
					assert(last_virtual_entry != 0);
					
/*					Node::Id virtual_node_id = iter_entry->_virtual_node_id;
					
					Node::Id lower_bound = first_virtual_entry->_virtual_node_id
						- static_cast<Node::Id>( ceil(
							(_conf.get(_STEP_DISTANCE_M)
								+ 1.1 * _conf.get(_SEARCH_MAX_DISTANCE_M))
							/ _conf.get(_STEP_DISTANCE_M)) );
					if ((virtual_node_id < lower_bound) ||
						(iter_entry->_scan_position != first_virtual_entry->_scan_position - _conf.get(_STEP_DISTANCE_M)) )
					{*/
						first_virtual_entry = 0;
						last_virtual_entry = 0;
					/*}*/
				}
				
				if (first_real_entry != 0)
				{
					assert(last_real_entry != 0);
					
					first_real_entry = 0;
					last_real_entry = 0;
				}
			}
			
			std::pair<bool, PathEntry*> result_pair
				= std::pair<bool, PathEntry*>(false, 0);
			if (first_virtual_entry != 0)
			{
				assert(last_virtual_entry != 0);
				
				result_pair = sequence_found(last_virtual_entry_iter, last_virtual_entry,
					first_virtual_entry, PathEntry::_VIRTUAL_CREATED);
			} else
			{
				if (first_real_entry != 0)
				{
					assert(last_real_entry != 0);
					
					result_pair = sequence_found(last_real_entry_iter, last_real_entry,
						first_real_entry, PathEntry::_REAL);
				}
			}
		
			if (result_pair.first)
			{
				result_pair.second->_is_beginning = true;
				exit_loop = true;
			}
		} // end while (!exit_loop && iter != iter_end)
		
		return exit_loop;
	}
	
	
	bool
	TraceProcessor::search_and_set_ending()
	{
		// Returns true, iff the path ends with an unambigious sequence.
		// So either a sequence of virtual nodes occur or a row of connected
		// real nodes is encountered.
		// If this method returns true, a PathEntry is marked to be the
		// destination. That is needed for the following ai_module. That
		// PathEntry is located at most _conf.get(_SUFFICIENT_PATH_LENGTH)/2
		// before the end of the unambigious sequence.
		
		PathEntry* first_real_entry = 0;
		PathEntry* last_real_entry = 0;
		PathEntry* first_virtual_entry = 0; // This has to be _VIRTUAL_CREATED!
		PathEntry* last_virtual_entry = 0;  // This has to be _VIRTUAL_CREATED!
		
		// CAUTION: The iters are travered in backward manner!
		D_Path::iterator iter = _path.end();
		D_Path::iterator iter_end = _path.begin();
		
		bool exit_loop = false;
		while (!exit_loop && iter != iter_end)
		{
			--iter;
			PathEntry* iter_entry = iter->second;
			
			// These if-clauses are not as compact as they could be. But in
			// this form it is much easier to understand what is going on.
			// It is a bit tricky: We search either for a sequence of virtual
			// nodes. This is done in the first part.
			// Or we need a sequence of real nodes. But keep in mind that there
			// is a virtual node every now and then. If we search for real
			// nodes we can omit the virtual ones (This is the path:
			// iter_entry->is_virtual() but !sequence_of_virtual_nodes).
			
			if (iter_entry->has_state(PathEntry::_VIRTUAL_CREATED))
			{
				if (first_virtual_entry == 0)
				{
					assert(last_virtual_entry == 0);
					last_virtual_entry = iter_entry;
				} else
				{
					if (!iter_entry->is_virtual_predecessor_of(*first_virtual_entry))
					{
						std::cout << "search_and_set_ending" << std::endl;
						std::cout << "The crashing entries: first = " << first_virtual_entry << ", iter = " << iter_entry << std::endl;
						std::cout << "The _path:" << std::endl;
						std::cout << _path << std::endl;
					}
					assert(iter_entry->is_virtual_predecessor_of(*first_virtual_entry));
				}
				
				first_virtual_entry = iter_entry;
				
			} else if (iter_entry->has_state(PathEntry::_REAL))
			{
				first_virtual_entry = 0;
				last_virtual_entry = 0;
				
				if (first_real_entry == 0)
				{
					assert(last_real_entry == 0);
					last_real_entry = iter_entry;
				} else
				{
					TileCache::Pointer tile = _tile_cache->get(
						Node::tile_id(iter_entry->_node_id) );
					const Node* node = &( tile->node(iter_entry->_node_id) );
					if( !node->has_successor(first_real_entry->_node_id) )
					{
						last_real_entry = iter_entry;
					}
				}
				
				first_real_entry = iter_entry;
				
			} else // if (iter_entry->has_state(PathEntry::_VIRTUAL_FOUND)
			{
				
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				// DOCUMENTATE THIS SECTION!!!!!!!!!!!!!!!
				if (first_virtual_entry != 0)
				{
					assert(last_virtual_entry != 0);
					
/*					Node::Id virtual_node_id = iter_entry->_virtual_node_id;
					
					Node::Id lower_bound = first_virtual_entry->_virtual_node_id
						- static_cast<Node::Id>( ceil(
							(_conf.get(_STEP_DISTANCE_M)
								+ 1.1 * _conf.get(_SEARCH_MAX_DISTANCE_M))
							/ _conf.get(_STEP_DISTANCE_M)) );
					if ((virtual_node_id < lower_bound) ||
						(iter_entry->_scan_position != first_virtual_entry->_scan_position - _conf.get(_STEP_DISTANCE_M)) )
					{*/
						first_virtual_entry = 0;
						last_virtual_entry = 0;
					/*}*/
				}
				
				if (first_real_entry != 0)
				{
					assert(last_real_entry != 0);
					
					first_real_entry = 0;
					last_real_entry = 0;
				}
			}
			
			std::pair<bool, PathEntry*> result_pair
				= std::pair<bool, PathEntry*>(false, 0);
			if (first_virtual_entry != 0)
			{
				assert(last_virtual_entry != 0);
				
				result_pair = sequence_found(iter, first_virtual_entry,
					last_virtual_entry, PathEntry::_VIRTUAL_CREATED);
			} else
			{
				if (first_real_entry != 0)
				{
					assert(last_real_entry != 0);
					
					result_pair = sequence_found(iter, first_real_entry,
						last_real_entry, PathEntry::_REAL);
				}
			}
		
			if (result_pair.first)
			{
				result_pair.second->_is_destination = true;
				exit_loop = true;
			}
		} // end while (!exit_loop && iter != iter_end)
		
		return exit_loop;
	}
	
	
	bool
	TraceProcessor::search_and_use_alterative_path_entries(
		PathEntry* const valid_start_entry, PathEntry* const valid_end_entry)
	{
		// preconditions
		assert(valid_start_entry != valid_end_entry);
		assert(valid_start_entry->_connection != valid_end_entry);
//		assert(valid_start_entry->has_state(PathEntry::_VIRTUAL_FOUND));
//		assert(valid_end_entry->has_state(PathEntry::_VIRTUAL_FOUND));
		
		if (valid_start_entry == valid_end_entry)
			return false;
		
		if (valid_start_entry->_connection == valid_end_entry)
			return true;
		
		if (!valid_start_entry->has_state(PathEntry::_VIRTUAL_FOUND))
			return false;
		
		if (!valid_end_entry->has_state(PathEntry::_VIRTUAL_FOUND))
			return false;
		
//		std::cout << _path << std::endl;
		
		// Get the corresponding virtual created entry to valid_start_entry.
		PathEntry* virtual_created_start_entry;
		if (true)
		{
			bool exit_loop = false;
			std::pair<D_Path::iterator, D_Path::iterator> result_iters
				= _path.equal_range(valid_start_entry->_position_on_trace);
			while ( (!exit_loop) && (result_iters.first != result_iters.second) )
			{
				PathEntry* entry = result_iters.first->second;
				if (entry->_virtual_node_id == valid_start_entry->_virtual_node_id)
				{
					assert(entry->has_state(PathEntry::_VIRTUAL_CREATED));
					
					exit_loop = true;
				} else
				{
					++(result_iters.first);
				}
			}
			
			if (result_iters.first != result_iters.second)
				virtual_created_start_entry = result_iters.first->second;
			else
				return false;
		}
		
		assert(virtual_created_start_entry != 0);
		
		// Get the corresponding virtual created entry to valid_end_entry.
		PathEntry* virtual_created_end_entry;
		if (true)
		{
			bool exit_loop = false;
			std::pair<D_Path::iterator, D_Path::iterator> result_iters
				= _path.equal_range(valid_end_entry->_position_on_trace);
			while ( (!exit_loop) && (result_iters.first != result_iters.second) )
			{
				PathEntry* entry = result_iters.first->second;
				if (entry->_virtual_node_id == valid_end_entry->_virtual_node_id)
				{
					assert(entry->has_state(PathEntry::_VIRTUAL_CREATED));
					
					exit_loop = true;
				} else
				{
					++(result_iters.first);
				}
			}
			
			if (result_iters.first != result_iters.second)
				virtual_created_end_entry = result_iters.first->second;
			else
				return false;
		}
		
		assert(virtual_created_end_entry != 0);
		if (virtual_created_start_entry == virtual_created_end_entry)
			return false;
		
		if (virtual_created_start_entry->_scan_position
			> virtual_created_end_entry->_scan_position)
		{
			return false;
		}
		
//		if (!virtual_created_start_entry->_is_used)
//			return false;
		
//		if (!virtual_created_end_entry->_is_used)
//			return false;
		
		// Get the node ids (real resp. virtual) for the entries between
		// virtual_created_start_entry and virtual_created_end_entry.
		std::vector<Node::Id> needed_node_ids;
		if (true)
		{
			PathEntry* entry = virtual_created_start_entry->_connection;
			
//			assert(entry->_is_used);
			
			while (entry != virtual_created_end_entry)
			{
				if (entry->has_state(PathEntry::_REAL))
				{
					assert(entry->_node_id != 0);
					
					needed_node_ids.push_back(entry->_node_id);
				} else
					needed_node_ids.push_back(entry->_virtual_node_id);
				
				entry = entry->_connection;
			}
		}
		
		// Get the path iterator to valid_start_entry.
		D_Path::iterator iter;
		if (true)
		{
			bool exit_loop = false;
			std::pair<D_Path::iterator, D_Path::iterator> result_iters
				= _path.equal_range(valid_start_entry->_scan_position);
			while ( (!exit_loop) && (result_iters.first != result_iters.second) )
			{
				PathEntry* entry = result_iters.first->second;
				if (entry->_virtual_node_id == valid_start_entry->_virtual_node_id)
					exit_loop = true;
				else
					++(result_iters.first);
			}
			
			assert(result_iters.first != result_iters.second);
			assert(result_iters.first->second == valid_start_entry);
			
			iter = result_iters.first;
		}
		
		// Get the path iterator to valid_end_entry.
		D_Path::iterator iter_end;
		if (true)
		{
			bool exit_loop = false;
			std::pair<D_Path::iterator, D_Path::iterator> result_iters
				= _path.equal_range(valid_end_entry->_scan_position);
			while ( (!exit_loop) && (result_iters.first != result_iters.second) )
			{
				PathEntry* entry = result_iters.first->second;
				if (entry->_virtual_node_id == valid_end_entry->_virtual_node_id)
					exit_loop = true;
				else
					++(result_iters.first);
			}
			
			assert(result_iters.first != result_iters.second);
			assert(result_iters.first->second == valid_end_entry);
			
			iter_end = result_iters.first;
		}
		
		// Get the path entries which correspond to the needed_node_ids
		// found above.
		std::vector<PathEntry*> used_path_entries;
		if (needed_node_ids.size() > 0)
		{
			int index = 0;
			while ( (index < needed_node_ids.size()) && (iter != iter_end) )
			{
				PathEntry* entry = iter->second;
				Node::Id nni = needed_node_ids[index];
				
				if (PathEntry::is_virtual_node_id(nni))
				{
					if (entry->_virtual_node_id == nni)
					{
						assert(entry->has_state(PathEntry::_VIRTUAL_FOUND));
						
						used_path_entries.push_back(entry);
						++index;
					}
				} else
				{
					if (entry->_node_id == nni)
					{
						assert(entry->has_state(PathEntry::_REAL));
						
						used_path_entries.push_back(entry);
						++index;
					}
				}
				
				++iter;
			}
			
			if (needed_node_ids.size() != used_path_entries.size())
				return false;
		}
		
		// Unset _is_used flags.
//		if (true)
//		{
//			PathEntry* entry = valid_start_entry->_connection;
//			while (entry != valid_end_entry)
//			{
//				entry->_is_used = false;
//				entry = entry->_connection;
//			}
//		}
		
		// Build the new connections.
		if (true)
		{
			for  (int i = 1; i < used_path_entries.size(); ++i)
			{
				used_path_entries[i-1]->_connection = used_path_entries[i];
			}
			
			if (used_path_entries.size() == 0)
			{
				valid_start_entry->_connection = valid_end_entry;
			} else
			{
				valid_start_entry->_connection = used_path_entries[0];
				used_path_entries.back()->_connection = valid_end_entry;
			}
		}
		
		return true;
	}
	
	
	bool
	TraceProcessor::search_parallel_ways(Node::Id node_id,
		Node::Id& destination_id, Node::Id& first_successor) const
	{
		if (true)
		{
			TileCache::Pointer tile = _tile_cache->get(Node::tile_id(node_id));
			const Node& node = tile->node(node_id);
			
			if (node.successors().size() != 2)
				return false;
		}
		
		Node::Id first_way_current_node_id;
		Node::Id first_way_next_node_id;
		Node::Id second_way_current_node_id;
		Node::Id second_way_next_node_id;
		
		// Set Node Ids.
		if (true)
		{
			TileCache::Pointer tile = _tile_cache->get(Node::tile_id(node_id));
			const Node& node = tile->node(node_id);
			
			first_way_current_node_id = node_id;
			first_way_next_node_id = node.successors().front().get_next_node_id();
			second_way_current_node_id = node_id;
			second_way_next_node_id = node.successors().back().get_next_node_id();
		}
		
		const int FIRST = 1;
		const int SECOND = 2;
		int current_state = FIRST;
		bool last_not_found = false;
		bool potential_destination_found = false;
		
		while (first_way_next_node_id != second_way_next_node_id)
		{
			// Get first way nodes.
			TileCache::Pointer first_way_current_tile = _tile_cache->get(
				Node::tile_id(first_way_current_node_id));
			const Node& first_way_current_node = first_way_current_tile
				->node(first_way_current_node_id);
			
			TileCache::Pointer first_way_next_tile = _tile_cache->get(
				Node::tile_id(first_way_next_node_id));
			const Node& first_way_next_node = first_way_next_tile
				->node(first_way_next_node_id);
			
			// Get second way nodes.
			TileCache::Pointer second_way_current_tile = _tile_cache->get(
				Node::tile_id(second_way_current_node_id));
			const Node& second_way_current_node = second_way_current_tile
				->node(second_way_current_node_id);
			
			TileCache::Pointer second_way_next_tile = _tile_cache->get(
				Node::tile_id(second_way_next_node_id));
			const Node& second_way_next_node = second_way_next_tile
				->node(second_way_next_node_id);
			
			// if more than 1 successor then return false!
			if (first_way_next_node.successors().size() != 1)
				return false;
			if (second_way_next_node.successors().size() != 1)
				return false;
			
			if (!potential_destination_found)
			{
				// if more than 1 predecessor then set potential_destination_found.
				if (first_way_next_node.predecessors().size() > 1)
				{
					assert(second_way_next_node.predecessors().size() == 1);
					
					if (first_way_current_node_id == node_id)
					{
						return false;
					} else
					{
						potential_destination_found = true;
						
						assert(current_state == SECOND);
					}
				}
				
				if (second_way_next_node.predecessors().size() > 1)
				{
					assert(first_way_next_node.predecessors().size() == 1);
					
					if (second_way_current_node_id == node_id)
					{
						return false;
					} else
					{
						potential_destination_found = true;
						
						assert(current_state == FIRST);
					}
				}
				
			} else // if (potential_destination_found)
			{
				// if more than 1 predecessor then return false!
				if ( (current_state == FIRST)
					&& (first_way_next_node.predecessors().size() > 1) )
				{
					return false;
				}
				
				if ( (current_state == SECOND)
					&& (second_way_next_node.predecessors().size() > 1) )
				{
					return false;
				}
			} // end if (!potential_destination_found)
			
			if (!potential_destination_found)
			{
				// calculate_cluster_nodes
				if (current_state == FIRST)
				{
					/** @todo wont work in every situation: two nodes on one line
						 with the arcs +- PI/10 (arbitrary!) will NOT find each other!!! */
					
					std::list<Node::Id> result;
					calculate_cluster_nodes(
						first_way_current_node, first_way_next_node, result);
					
					std::list<Node::Id>::const_iterator find_iter = std::find(
						result.begin(), result.end(), second_way_next_node_id); // next way!
					if (find_iter == result.end())
					{
						if (last_not_found)
						{
							return false;
						} else
						{
							last_not_found = true;
							current_state = SECOND;
						}
					} else
					{
						last_not_found = false;
						
						assert(second_way_next_node.successors().size() == 1);
						
						second_way_current_node_id = second_way_next_node_id; // second!
						second_way_next_node_id = second_way_next_node
							.successors().front().get_next_node_id();
					}
					
				} else if (current_state == SECOND)
				{
					std::list<Node::Id> result;
					calculate_cluster_nodes(
						second_way_current_node, second_way_next_node, result);
					
					std::list<Node::Id>::const_iterator find_iter = std::find(
						result.begin(), result.end(), first_way_next_node_id); // next way!
					if (find_iter == result.end())
					{
						if (last_not_found)
						{
							return false;
						} else
						{
							last_not_found = true;
							current_state = FIRST;
						}
					} else
					{
						last_not_found = false;
						
						assert(first_way_next_node.successors().size() == 1);
						
						first_way_current_node_id = first_way_next_node_id; // first!
						first_way_next_node_id = first_way_next_node
							.successors().front().get_next_node_id();
					}
				}
				
			} else // if (potential_destination_found)
			{
				if (current_state == FIRST)
				{
					std::list<Node::Id> result;
					calculate_cluster_nodes(
						first_way_current_node, first_way_next_node, result);
					
					std::list<Node::Id>::const_iterator find_iter = std::find(
						result.begin(), result.end(), second_way_current_node_id); // current way!
					if (find_iter == result.end())
					{
						return false;
					} else
					{
						assert(first_way_next_node.successors().size() == 1);
						
						first_way_current_node_id = first_way_next_node_id; //first!
						first_way_next_node_id = first_way_next_node
							.successors().front().get_next_node_id();
					}
					
				} else if (current_state == SECOND)
				{
					std::list<Node::Id> result;
					calculate_cluster_nodes(
						second_way_current_node, second_way_next_node, result);
					
					std::list<Node::Id>::const_iterator find_iter = std::find(
						result.begin(), result.end(), first_way_current_node_id); // current way!
					if (find_iter == result.end())
					{
						return false;
					} else
					{
						assert(second_way_next_node.successors().size() == 1);
						
						second_way_current_node_id = second_way_next_node_id; //second!
						second_way_next_node_id = second_way_next_node
							.successors().front().get_next_node_id();
					}
				}
			} // end if (!potential_destination_found)
		} // end while (first_way_next_node_id != second_way_next_node_id)
		
		assert(first_way_next_node_id == second_way_next_node_id);
		
		TileCache::Pointer tile = _tile_cache->get(Node::tile_id(node_id));
		const Node& node = tile->node(node_id);
		
		destination_id = first_way_next_node_id;
		first_successor = node.successors().back().get_next_node_id();
		
		return true;
	}
	
	
	double
	TraceProcessor::search_perpendicular(const GeoCoordinate& test_gc,
		double start_position_m, double end_position_m,
		double min_position_diff) const
	{
		assert(start_position_m >= 0.0);
		assert(end_position_m <= _filtered_trace.length_m());
		assert(start_position_m < end_position_m);
		assert(min_position_diff > 0.0);
		
		double current_start_position_m = start_position_m;
		double current_end_position_m = end_position_m;
		double current_position_m;
		
		GeoCoordinate current_start_point
			= _filtered_trace.gps_point_at(current_start_position_m);
		GeoCoordinate current_end_point
			= _filtered_trace.gps_point_at(current_end_position_m);
		
		while ( current_end_position_m  - current_start_position_m
			>= min_position_diff )
		{
			assert(current_start_position_m < current_end_position_m);
			
			current_position_m =
				(current_start_position_m + current_end_position_m) / 2.0;
			
			GeoCoordinate current_point
				= _filtered_trace.gps_point_at(current_position_m);
			
			if (test_gc.has_perpendicular_on_segment(
				current_start_point, current_point))
			{
				current_end_position_m = current_position_m;
				current_end_point = current_point;
			} else
			{
				current_start_position_m = current_position_m;
				current_start_point = current_point;
			}
		}
		
		if (current_position_m == current_start_position_m)
			return current_end_position_m;
		else
			return current_start_position_m;
		
/*		double distance_to_start_point = test_gc.distance(current_start_point);
		double distance_to_end_point = test_gc.distance(current_end_point);
		
		if (distance_to_start_point < distance_to_end_point)
			return current_start_position_m;
		else
			return current_end_position_m;*/
	}
	
	
	std::pair<bool, TraceProcessor::PathEntry*>
	TraceProcessor::sequence_found(D_Path::iterator iter,
		PathEntry* first_entry, PathEntry* last_entry, PathEntry::State state)
	{
		PathEntry* a = iter->second;
		assert(iter->second == first_entry);
		assert(state != PathEntry::_VIRTUAL_FOUND);
		
		double scan_position_difference
			= last_entry->_scan_position - first_entry->_scan_position;
		
		if (scan_position_difference >= _conf.get(_SUFFICIENT_PATH_LENGTH))
		{
			a = iter->second;
			assert(iter->second == first_entry);
			while(iter->second->_scan_position < first_entry->_scan_position
				+ (scan_position_difference / 2.0) )
			{
				++iter;
				a = iter->second;
				
				assert(iter->second != last_entry);
				if (state == PathEntry::_VIRTUAL_CREATED)
				{
					assert(iter->second->_state != PathEntry::_REAL);
					assert(iter->second->_state != PathEntry::_VIRTUAL_FOUND);
				}
			}
			
			while (!iter->second->has_state(state))
			{
				++iter;
				a = iter->second;
				
				if (iter->second == last_entry)
					return std::pair<bool, PathEntry*>(false, 0);

				
				if (state == PathEntry::_VIRTUAL_CREATED)
				{
					assert(iter->second->_state != PathEntry::_REAL);
					assert(iter->second->_state != PathEntry::_VIRTUAL_FOUND);
				}
			}
			
			return std::make_pair(true, iter->second);
		}
		
		return std::pair<bool, PathEntry*>(false, 0);
	}
	
	
	void
	TraceProcessor::serialize_protocol()
	{
		_protocol._tile_cache = _tile_cache;
		
		
		_protocol._path = _path;
		
		// add trace to the protocol
		_protocol._trace = _filtered_trace;
		
		// build filename
		time_t current_time = _tile_manager->get_start_time();
		tm* time_struct = gmtime(&current_time);
		
		std::stringstream filename_stream;
		filename_stream << "traceprocessor_logs/("
			<< time_struct->tm_year + 1900 << "-"
			<< time_struct->tm_mon + 1 << "-"
			<< time_struct->tm_mday << "_"
			<< time_struct->tm_hour << "-"
			<< time_struct->tm_min
			<< ")_id_" << _id;
		std::string filename = filename_stream.str();
		
//		std::cout << filename << std::endl;
		
		std::ofstream protocol_stream(filename.c_str());
		Serializer::serialize(protocol_stream, _protocol);
		protocol_stream.close();
	}
	
	
	void
	TraceProcessor::smooth_connections(
		const std::multimap<Node::Id, CrossingItem>& crossings_mapper)
	{
		std::multimap<Node::Id, PathEntry*> node_ids_entries_mapper;
		if (true)
		{
			D_Path::const_iterator iter = _path.begin();
			D_Path::const_iterator iter_end = _path.end();
			while ( (iter != iter_end) && (!iter->second->_is_beginning) )
				++iter;
			
			if (iter == iter_end)
				assert(crossings_mapper.empty());
			
			PathEntry* entry = iter->second;
			while (entry != 0)
			{
				node_ids_entries_mapper.insert(
					std::make_pair(entry->_node_id, entry) );
				
				entry = entry->_connection;
			}
		}
		
		std::set<Node::Id> nodes_predecessor_moved;
		std::set<Node::Id> nodes_successors_moved;
		
		std::multimap<Node::Id, CrossingItem>::const_iterator iter
			= crossings_mapper.begin();
		std::multimap<Node::Id, CrossingItem>::const_iterator iter_end
			= crossings_mapper.end();
		for (; iter != iter_end; ++iter)
		{
			assert(iter->first == iter->second._entry->_node_id);
			assert(iter->second._entry->has_state(PathEntry::_REAL));
			
			if (iter->second._has_new_predecessors)
			{
				std::set<Node::Id>::const_iterator find_iter
					= nodes_predecessor_moved.find(iter->first);
				if (find_iter == nodes_predecessor_moved.end())
				{
					const PathEntry* entry = iter->second._entry;
					
					assert(entry->_node_id == iter->first);
					assert(entry->_backward_connection != 0);
					
					PathEntry* previous_entry = entry->_backward_connection;
					PathEntry* next_entry = entry->_connection;
					
					if ( (next_entry != 0)
						&& (!previous_entry->_do_not_use_connection)
						&& (!entry->_do_not_use_connection) )
					{
						TileCache::Pointer tile = _tile_cache->get(
							Node::tile_id(entry->_node_id));
						const Node& node = tile->node(entry->_node_id);
						
						if (node.successors().size() == 1)
						{
							#ifndef NDEBUG
								TileCache::Pointer previous_tile = _tile_cache->get(
									Node::tile_id(previous_entry->_node_id));
								const Node& previous_node = previous_tile->node(
									previous_entry->_node_id);
								
								assert(previous_node.has_successor(entry->_node_id));
								assert(node.has_successor(next_entry->_node_id));
							#endif
							
							if (previous_entry->_node_id != next_entry->_node_id)
							{
								bool connection_used_multiply_times = false;
								
								typedef std::multimap<Node::Id, PathEntry*>::iterator FindIter;
								std::pair<FindIter, FindIter> find_iters
									= node_ids_entries_mapper.equal_range(previous_entry->_node_id);
								while (find_iters.first != find_iters.second)
								{
									PathEntry* test_entry = find_iters.first->second;
									if ( (test_entry != previous_entry)
										&& (test_entry->_connection != 0)
										&& (test_entry->_connection->_node_id == entry->_node_id) )
									{
										connection_used_multiply_times = true;
										find_iters.first = find_iters.second;
									} else
									{
										++find_iters.first;
									}
								}
								
								if (!connection_used_multiply_times)
								{
									disconnect_nodes(previous_entry->_node_id,
										entry->_node_id);
								}
								
								connect_nodes(previous_entry->_node_id,
									next_entry->_node_id);
								
								previous_entry->_connection = next_entry;
								next_entry->_backward_connection = previous_entry;
								
		//						create_extra_nodes_at_connection(
		//							previous_entry->_node_id,
		//							node.successors().back().get_next_node_id(),
		//							_MIN_STEP_DISTANCE_M);
								
								nodes_predecessor_moved.insert(entry->_node_id);
							}
							
						}
					} // end if ( ... && ... && ... && ... && ... )
				} // end if (find_iter == nodes_predecessor_moved.end())
			} // end if (iter->second._has_new_predecessors)
			
			if (iter->second._has_new_successors)
			{
				std::set<Node::Id>::const_iterator find_iter
					= nodes_successors_moved.find(iter->first);
				if (find_iter == nodes_successors_moved.end())
				{
					const PathEntry* entry = iter->second._entry;
					
					PathEntry* previous_entry = entry->_backward_connection;
					PathEntry* next_entry = entry->_connection;
					
					assert(entry->_node_id == iter->first);
					assert(entry->_connection != 0);
					
					if ( (previous_entry != 0)
						&& (!previous_entry->_do_not_use_connection)
						&& (!entry->_do_not_use_connection) )
					{
						TileCache::Pointer tile = _tile_cache->get(
							Node::tile_id(entry->_node_id));
						const Node& node = tile->node(entry->_node_id);
						
						if (node.predecessors().size() == 1)
						{
							#ifndef NDEBUG
								TileCache::Pointer previous_tile = _tile_cache->get(
									Node::tile_id(previous_entry->_node_id));
								const Node& previous_node = previous_tile->node(
									previous_entry->_node_id);
								
								assert(previous_node.has_successor(entry->_node_id));
								assert(node.has_successor(next_entry->_node_id));
							#endif
							
							if (previous_entry->_node_id != next_entry->_node_id)
							{
								bool connection_used_multiply_times = false;
								
								typedef std::multimap<Node::Id, PathEntry*>::iterator FindIter;
								std::pair<FindIter, FindIter> find_iters
									= node_ids_entries_mapper.equal_range(entry->_node_id);
								while (find_iters.first != find_iters.second)
								{
									PathEntry* test_entry = find_iters.first->second;
									if ( (test_entry != previous_entry)
										&& (test_entry->_connection != 0)
										&& (test_entry->_connection->_node_id == next_entry->_node_id) )
									{
										connection_used_multiply_times = true;
										find_iters.first = find_iters.second;
									} else
									{
										++find_iters.first;
									}
								}
								
								if (!connection_used_multiply_times)
								{
									disconnect_nodes(entry->_node_id,
										next_entry->_node_id);
								}
								
								connect_nodes(previous_entry->_node_id,
									next_entry->_node_id);
								
								previous_entry->_connection = next_entry;
								next_entry->_backward_connection = previous_entry;
								
		//						create_extra_nodes_at_connection(
		//							previous_entry->_node_id,
		//							node.successors().back().get_next_node_id(),
		//							_MIN_STEP_DISTANCE_M);
								
								nodes_successors_moved.insert(entry->_node_id);
							}
							
						}
					} // end if ( ... && ... && ... && ... && ... )
				} // end if (find_iter == nodes_successors_moved.end())
			} // end if (iter->second._has_new_successors)
		} // end for (; iter != iter_end; ++iter)
	}
	
	
	bool
	TraceProcessor::verify_node_bearings(Node::Id node_id) const
	{
		TileCache::Pointer tile = _tile_cache->get(Node::tile_id(node_id));
		const Node& node = tile->node(node_id);
		
		if (true)
		{
			Node::D_Edges::const_iterator iter = node.predecessors().begin();
			Node::D_Edges::const_iterator iter_end = node.predecessors().end();
			for (; iter != iter_end; ++iter)
			{
				TileCache::Pointer test_tile = _tile_cache->get(
					Node::tile_id(iter->get_next_node_id()));
				const Node& test_node = test_tile->node(iter->get_next_node_id());
				
				double bearing = test_node.bearing(node);
				if (test_node.successor(node_id).get_direction()
					!= Direction(bearing))
				{
					return false;
				}
			}
		}
		
		if (true)
		{
			Node::D_Edges::const_iterator iter = node.successors().begin();
			Node::D_Edges::const_iterator iter_end = node.successors().end();
			for (; iter != iter_end; ++iter)
			{
				TileCache::Pointer test_tile = _tile_cache->get(
					Node::tile_id(iter->get_next_node_id()));
				const Node& test_node = test_tile->node(iter->get_next_node_id());
				
				double bearing = node.bearing(test_node);
				if (node.successor(iter->get_next_node_id()).get_direction()
					!= Direction(bearing))
				{
					return false;
				}
			}
		}
		
		return true;
	}
			
		
/*	void
	TraceProcessor::smooth_connections()
	{
		// Find the beginning.
		D_Path::iterator iter = _path.begin();
		D_Path::iterator iter_end = _path.end();
		while ( (iter != iter_end) && (!iter->second->_is_beginning) )
			++iter;
		
		assert(iter != iter_end);
		assert(iter->second->_is_beginning);
		
		PathEntry* previous_entry = iter->second;
		PathEntry* entry = previous_entry->_connection;
		
		if (entry == 0)
			return;
		
		while (entry->_connection != 0)
		{
			if ( (!previous_entry->_was_deleted) && (!entry->_was_deleted)
				&& (!entry->_connection->_was_deleted) )
			{
				// new incoming connections
				if (true)
				{
					// There exists several combinations when a new incoming
					// connection is created. I list them now.
					
					// previous_entry was virtual created,
					// entry was virtual found and has a predecessor.
					bool condition_one
						= (previous_entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (previous_entry->_scan_position == previous_entry->_position_on_trace)
						&& (entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (entry->_scan_position > entry->_position_on_trace)
						&& (entry->_position_on_trace > iter->second->_position_on_trace);
					
					// previous_entry was virtual created,
					// entry was real and has at least 2 predecessors.
					bool condition_two
						= (previous_entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (previous_entry->_scan_position == previous_entry->_position_on_trace)
						&& (entry->_virtual_node_id < _VIRTUAL_NODE_ID_OFFSET);
					
					// previous_entry was virtual found,
					// entry was real and has at least 2 predecessors.
					bool condition_three
						= (previous_entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (previous_entry->_scan_position > previous_entry->_position_on_trace)
						&& (entry->_virtual_node_id < _VIRTUAL_NODE_ID_OFFSET);
					
					// It is also possible that two virtual found entries or
					// two real entries builds a new incoming connection. But the
					// probability is very small and the detection is not that
					// easy. So I omit these cases here.
					
					if (condition_one || condition_two || condition_three)
					{
						// May not hold, when _do_not_use_connection was set
						// for the involved entries!
						// assert(node.has_predecessor(previous_entry->_node_id));
						
						TileCache::Pointer tile = _tile_cache->get(
							Node::tile_id(entry->_node_id));
						const Node node = tile->node(entry->_node_id);
						
						if ( (node.has_predecessor(previous_entry->_node_id))
							&& (node.predecessors().size() >= 2)
							&& (node.successors().size() == 1)
							/*&& (node.has_successor(entry->_connection->_node_id))************ )
						{
							TileCache::Pointer previous_tile = _tile_cache->get(
								Node::tile_id(previous_entry->_node_id) );
							const Node previous_node = previous_tile->node(
								previous_entry->_node_id);
							
							TileCache::Pointer next_tile = _tile_cache->get(
								Node::tile_id(node.successors().back().get_next_node_id()) );
							const Node next_node = next_tile->node(
								node.successors().back().get_next_node_id());
							
							if ( (node.distance(next_node) < 5.0)
								|| (previous_node.distance(node) > 2.0) )
							{
								disconnect_nodes(previous_entry->_node_id,
									entry->_node_id);
								connect_nodes(previous_entry->_node_id,
									node.successors().back().get_next_node_id());
								
								create_extra_nodes_at_connection(
									previous_entry->_node_id,
									node.successors().back().get_next_node_id(),
									_MIN_STEP_DISTANCE_M);
							}
						}
					}
				}
				
				// new outgoing connections
				if (true)
				{
					// There exists several combinations when a  new outgoing
					// connection is created. I list them now.
					
					// entry was virtual found,
					// entry->_connection was virtual created.
					bool condition_one
						= (entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (entry->_scan_position > entry->_position_on_trace)
						&& (entry->_connection->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (entry->_connection->_scan_position == entry->_connection->_position_on_trace);
					
					// entry was virtual found,
					// entry->_connection was real.
					bool condition_two
						= (entry->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET)
						&& (entry->_scan_position > entry->_position_on_trace)
						&& (entry->_connection->_virtual_node_id < _VIRTUAL_NODE_ID_OFFSET);
					
					// entry was real and has at least 2 successors,
					// entry->_connection was virtual created/found
					bool condition_three
						= (entry->_virtual_node_id < _VIRTUAL_NODE_ID_OFFSET)
						&& (entry->_connection->_virtual_node_id >= _VIRTUAL_NODE_ID_OFFSET);
					
					// It is also possible that two virtual found entries or
					// two real entries builds a new outgoing connection. But the
					// probability is very small and the detection is not that
					// easy. So I omit these cases here.
					
					if (condition_one || condition_two || condition_three)
					{
						// May not hold, when _do_not_use_connection was set
						// for the involved entries!
						//assert(node.has_successor(entry->_connection->_node_id));
						
						TileCache::Pointer tile = _tile_cache->get(
							Node::tile_id(entry->_node_id));
						const Node node = tile->node(entry->_node_id);
						
						if ( (node.has_successor(entry->_connection->_node_id))
							&& (node.successors().size() >= 2)
							&& (node.predecessors().size() == 1)
							/*&& (node.has_predecessor(previous_entry->_node_id))************ )
						{
							TileCache::Pointer previous_tile = _tile_cache->get(
								Node::tile_id(node.predecessors().back().get_next_node_id()) );
							const Node previous_node = previous_tile->node(
								node.predecessors().back().get_next_node_id());
							
							TileCache::Pointer next_tile = _tile_cache->get(
								Node::tile_id(entry->_connection->_node_id) );
							const Node next_node = next_tile->node(
								entry->_connection->_node_id);
							
							if ( (previous_node.distance(node) < 5.0)
								|| (node.distance(next_node) > 2.0) )
							{
								disconnect_nodes(entry->_node_id,
									entry->_connection->_node_id);
								connect_nodes(node.predecessors().back().get_next_node_id(),
									entry->_connection->_node_id);
								
								create_extra_nodes_at_connection(
									node.predecessors().back().get_next_node_id(),
									entry->_connection->_node_id,
									_MIN_STEP_DISTANCE_M);
							}
						}
					}
				}
			} // end if ( not _was_deleted )
			
			previous_entry = entry;
			entry = entry->_connection;
		} // end while(entry->_connection != 0)
	}*/
	

// -------------------------------------------------------------------------- //
// OLD METHODS OLD METHODS OLD METHODS OLD METHODS OLD METHODS OLD METHODS OL //
// -------------------------------------------------------------------------- //
	
/*				bool only_virtual_nodes = true;
				bool connected_real_nodes = true;
				double first_real_node_position_m = 0;
				double last_real_node_position_m = 0;
				PathEntry* middle_virtual_node = 0;
				double middle_position_m = path.back()._position - 40.0;
				
				std::list<PathEntry>::reverse_iterator 
					path_iter_end = path.rend();
				std::list<PathEntry>::reverse_iterator 
					path_iter = path.rbegin();
				
				PathEntry* next_real_path_entry = 0;
				
				while ((connected_real_nodes || only_virtual_nodes) &&
					(path.back()._position - path_iter->_position <= 100.0))
				{
					if (path_iter->is_virtual())
					{
						if ((middle_virtual_node == 0) &&
							(path_iter->_position < middle_position_m))
						{
							middle_virtual_node = &*path_iter;
						}
					} else 
					{
						only_virtual_nodes = false;
						
						if (next_real_path_entry == 0)
						{
							next_real_path_entry = &*path_iter;
//							first_real_node_position_m = path_iter->_position;
//							last_real_node_position_m = path_iter->_position;
						} else if ((path_iter->_node_copy.
							is_reachable(next_real_path_entry->_node_id)))
						{
							previous_real_node = &*path_iter;
//							last_real_node_position_m = path_iter->_position;
						} else
						{
							connected_real_nodes = false;
						}
					}
					
					++path_iter;
				}
				
				
				bool found_destination = false;
				
				if (only_virtual_nodes)
				{
					assert(middle_virtual_node != 0);
					middle_virtual_node->_is_destination = true;
					found_destination = true;
				} else if (/*connected_real_nodes && *
					((first_real_node_position_m - last_real_node_position_m) > 80.0)/* &&
					(first_real_node_position_m > (path.back()._position - 20.0))*)
				{										
					double middle_position_m = 
						(first_real_node_position_m + last_real_node_position_m) / 2.0;
					
					path_iter_end = path.rend();					
					path_iter = path.rbegin();
					
					while ((path_iter != path_iter_end) && 
						((path_iter->_position > middle_position_m) || (path_iter->_node_id == 0)))
						++path_iter;
					
					assert(path_iter != path_iter_end);

					path_iter->_is_destination = true;
					found_destination = true;
				}
				
				if (found_destination)
				{
					std::list<PathEntry> ready_path;
					std::list<PathEntry>::iterator f_path_iter = path.begin();
					bool copied = false;
					while (!copied && (f_path_iter != path.end()))
					{
						ready_path.push_back(*f_path_iter);
						if (f_path_iter->_is_destination)
							copied = true;
						f_path_iter = path.erase(f_path_iter);
					}
					
					/*
					 * This list will contain the segments calculated by
					 * simplify path.
					 *
					std::list<PathEntry> finished_segment;
					
					distinct_position_m = ready_path.back()._position;
	
					/*
					 * We call the simplify algorithm to calculate the optimal
					 * nodes to use.
					 *
					simplify_path(previous_node_id, ready_path, finished_segment);
						
					/*
					 * This method merges the trace to the map as
					 * described by the segments. previous_node_id and
					 * completed_position_m are updates as needed.
					 *				
					use_segment(finished_segment, completed_position_m, 
						previous_node_id);
				}
			}

			/*
			 * If the path is empty we let the create_nodes method create nodes
			 * up to the distinct_position_m.
			 *
			create_nodes(completed_position_m, distinct_position_m, false,
				previous_node_id);

			scan_position_m += 10.0;
		}*/
			
//		delete _trace_log;

	
	
/*	TraceProcessor::PathEntry::PathEntry(const PathEntry& path_entry)
	: _node_id(path_entry._node_id),
		_node_copy(path_entry._node_copy),
		_position(path_entry._position),
		_virtual_node_id(path_entry._virtual_node_id),
		_score(path_entry._score),
		_time_stamp(path_entry._time_stamp),
		_connection(path_entry._connection),
		_is_destination(path_entry._is_destination)
	{
	}*/
	
	
/*	TraceProcessor::PathEntry&
	TraceProcessor::PathEntry::operator=(const PathEntry& path_entry)
	{
		/** @todo needed when copy constructor is available?! *
		_node_id = path_entry._node_id;
		_node_copy = path_entry._node_copy;
		_position = path_entry._position;
		_virtual_node_id = path_entry._virtual_node_id;
		_score = path_entry._score;
		_time_stamp = path_entry._time_stamp;
		_connection = path_entry._connection;
		
		return *this;
	}*/
	
	
/*	void
	TraceProcessor::build_finished_segment(std::list<PathEntry>& path,
		std::list<PathEntry>& finished_segment,	PathEntry* start_entry)
	{
		finished_segment.clear();
		
		PathEntry* position = start_entry;
		PathEntry* previous_position = 0;
		bool previous_is_unused = false;
		while (position != 0)
		{
			if ((previous_position != 0) && (previous_position->_node_id == 0)
				&& (position->_node_id == 0))
			{
				previous_is_unused = true;
			} else 
			{
				if (previous_is_unused)
				{
					finished_segment.push_back(*previous_position);
					previous_is_unused = false;
				}
				
				finished_segment.push_back(*position);
			}
			
			previous_position = position;			
			position = position->_connection;
		}
	}*/
	

/*	void
	TraceProcessor::connect_nodes(Node::Id first_node_id, 
		Node::Id second_node_id)
	{
		if (first_node_id == second_node_id)
			return;
		
		/** @todo think about uncritical write access to nodes! *
		TileCache::Pointer tile_pointer = _tile_cache->get(Node::tile_id(first_node_id));
		Node* node = &(tile_pointer.write().node(first_node_id));
		
		double direction = node->bearing_default(
			_tile_cache->get(Node::tile_id(second_node_id))->node(second_node_id) );
			
		node->add_next_node(second_node_id, direction);
	}*/
	
	
/*	bool
	TraceProcessor::connection_from_to(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(Node::tile_id(node_id_1));
		if (tile_1_pointer->exists_node(node_id_1) &&
			tile_1_pointer->node(node_id_1).is_reachable(node_id_2))
			return true;
		else
			return false;
	}*/
	
	
/*	void
	TraceProcessor::create_connection(
		double& completed_position_m, Node::Id& previous_node_id, 
		double start_position_m, Node::Id destination_id)
	{
		GPSPoint start_point(_filtered_trace.gps_point_at(start_position_m));
		GPSPoint destination_point( 
			_tile_cache->get(Node::tile_id(destination_id))->node(destination_id)
		);		
		
		double distance = start_point.distance_default(destination_point);
		int new_nodes = (int)(distance / 10.0) - 1;
		double weight_step = 1.0 / ((double)new_nodes + 1.0);
		
		double weight = weight_step;
		for (int i=0; i<new_nodes; ++i)
		{
			GPSPoint new_node_position = GeoCoordinate::interpolate_default(
				start_point, destination_point, 1.0 - weight
			);
			Node::Id new_node_id = create_new_node(new_node_position);
			if (previous_node_id != 0)
				connect_nodes(previous_node_id, new_node_id);
			insert_into_processed_nodes(new_node_id, completed_position_m);
			previous_node_id = new_node_id;
			weight += weight_step;
		}		
	}*/
	
	
/*	void
	TraceProcessor::create_disconnection(
		double& completed_position_m, Node::Id& previous_node_id, 
		Node::Id start_id, double destination_position_m)
	{
		GPSPoint destination_point(_filtered_trace.gps_point_at(destination_position_m));
		GPSPoint start_point( 
			_tile_cache->get(Node::tile_id(start_id))->node(start_id)
		);
		
		double distance = start_point.distance_default(destination_point);
		int new_nodes = (int)(distance / 10.0) - 1;
		double weight_step = 1.0 / ((double)new_nodes + 1.0);
		
		double weight = weight_step;
		for (int i=0; i<new_nodes; ++i)
		{
			GPSPoint new_node_position = GeoCoordinate::interpolate_default(
				start_point, destination_point, 1.0 - weight
			);
			Node::Id new_node_id = create_new_node(new_node_position);
			if (previous_node_id != 0)
				connect_nodes(previous_node_id, new_node_id);
			insert_into_processed_nodes(new_node_id, completed_position_m);
			previous_node_id = new_node_id;
			weight += weight_step;
		}
	}*/


/*	void
	TraceProcessor::create_nodes(double& completed_position_m, double end_position_m,
		bool use_end_position, Node::Id& previous_node_id)
	{		
		/** 
		 * @todo The node distance should be dynamic.
		 *
		/**
		 * The while loop creates nodes up to the last step before end_position.
		 *
		while (completed_position_m < (end_position_m - 15.0))
		{
			completed_position_m += 10.0;
			GPSPoint new_node_position = _filtered_trace.
				gps_point_at(completed_position_m);
			Node::Id new_node_id = create_new_node(new_node_position);
			if (previous_node_id != 0)
				connect_nodes(previous_node_id, new_node_id);
			insert_into_processed_nodes(new_node_id, completed_position_m);
			previous_node_id = new_node_id;
		}
		
		/**
		 * If the end position should be used a node is created at the 
		 * end position.
		 *
		if (use_end_position)
		{
			completed_position_m = end_position_m;
			GPSPoint new_node_position = _filtered_trace.
				gps_point_at(completed_position_m);
			Node::Id new_node_id = create_new_node(new_node_position);
			if (previous_node_id != 0)
				connect_nodes(previous_node_id, new_node_id);
			insert_into_processed_nodes(new_node_id, completed_position_m);
			previous_node_id = new_node_id;
		}
		
	}*/
	
	
/*	double
	TraceProcessor::optimal_node_position(PathEntry path_entry)
	{		
		GeoCoordinate entry_coordinate = _tile_cache->
			get(Node::tile_id(path_entry._node_id))->node(path_entry._node_id);
		double position = path_entry._position;
		double previous_distance = 1000000.0;
		double distance = previous_distance - 1.0;
		GeoCoordinate path_coordinate;
		
		double best_position = position;
		
		while ((distance < previous_distance) && (position <= _filtered_trace.length_m()))
		{
			previous_distance = distance;
			position += 1.0;
			path_coordinate = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate.distance_approximated(path_coordinate);
			if (distance < previous_distance)
				best_position = position;
		}
		
		previous_distance = distance + 1.0;
		while ((distance < previous_distance) && (position >= 1.0))
		{
			previous_distance = distance;
			position -= 1.0;
			path_coordinate = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate.distance_approximated(path_coordinate);
			if (distance < previous_distance)
				best_position = position;
		}
		
		return best_position;
	}*/
		
	
/*	void
	TraceProcessor::simplify_path(Node::Id start_node_id, 
		std::list<PathEntry>& path, 
		std::list<PathEntry>& finished_segment)
	{
		/*
		 * simplify path does a depth first search evaluating each possible
		 * path to find the optimal row of nodes to connect.
		 * The input consists of the id of the already existing start node,
		 * the path and the keep_last_entries option. The path and 
		 * finished_segments will contain the result.
		 */
		
		/*
		 * This is probably not needed anymore.
		 * @todo Check if this is really not needed.
		 *
		if (path.size() < 2)
		{
			path.clear();
			return;
		}
		
		/*
		 * We initialize some values. best_points is really bad in the 
		 * beginning, best_start_entry is simply not found and the method
		 * is 0 -> we are just searching for connected paths, these are
		 * always better than paths with "jumps".
		 *
		double best_points = -100000.0;
		PathEntry* best_start_entry = 0;

		/*
		 * We just have to visit each path entry once, we use time stamps
		 * to control this and increase the time before each search.
		 *
		++_time;
				
		/*
		 * If we have a start_node_id we insert it as a new PathEntry into
		 * the path and start the recursion with this entry.
		 * The best_start_entry is not the start node (as this is already
		 * processed), but the first entry after the start node entry.
		 * Else we start with each PathEntry that is not more than x
		 * meters away from the first entry and choose the best entry from
		 * the recursion results.
		 *
		if (start_node_id != 0)
		{
			PathEntry start_entry;
			
			// The position is just a rough approximation, but that's 
			// enough here.
			start_entry._position = path.front()._position - 5.0;
			start_entry._node_id = start_node_id;
			start_entry._node_copy = _tile_cache->
				get(Node::tile_id(start_node_id))->node(start_node_id);
			path.insert(path.begin(), start_entry);
						
			best_points = build_best_connections_recursively(path,
				path.begin(), 1, false/*, 1001.0*);
			best_start_entry = path.front()._connection;
		} else
		{
			double start_position = path.front()._position;
			std::list<PathEntry>::iterator path_iter = path.begin();
			std::list<PathEntry>::iterator path_iter_end = path.end();
			while ((path_iter != path_iter_end) && 
				(path_iter->_position < start_position + 50.0))
			{
				double points = build_best_connections_recursively(path,
					path_iter, 1, path_iter->is_virtual()/*, 1001.0*);
				points -= path_iter->_position - start_position;
				
				if (points > best_points)
				{
					best_points = points;
					best_start_entry = &(*path_iter);
				}
				
				++path_iter;
				
			}
		}
			
		build_finished_segment(path, finished_segment, best_start_entry);				
	}*/


/*	TileCache::Pointer
	TraceProcessor::tile(Node::Id node_id)
	{
		return _tile_cache->get(Node::tile_id(node_id));
	}*/
	
	
/*	void
	TraceProcessor::use_segment(std::list<PathEntry>& finished_segment, 
		double& completed_position_m, Node::Id& previous_node_id)
	{
		std::list<PathEntry>::iterator segment_iter =
			finished_segment.begin();
		std::list<PathEntry>::iterator previous_segment_iter = 0;
		std::list<PathEntry>::iterator segment_iter_end =
			finished_segment.end();
		
		for (; segment_iter != segment_iter_end; ++segment_iter)
		{
			if (previous_segment_iter != 0)
			{
				if ((previous_segment_iter->_node_id == 0) &&
					(segment_iter->_node_id != 0))
				{
					completed_position_m = segment_iter->_position;
					create_connection(completed_position_m,
						previous_node_id, previous_segment_iter->_position, 
						segment_iter->_node_id);
				} else if ((previous_segment_iter->_node_id != 0) &&
					(segment_iter->_node_id == 0))
				{
					completed_position_m = segment_iter->_position;
					create_disconnection(completed_position_m,
						previous_node_id, previous_segment_iter->_node_id,
						segment_iter->_position);
				} else if ((previous_segment_iter->_node_id == 0) &&
					(segment_iter->_node_id == 0))
				{
					create_nodes(completed_position_m, 
						segment_iter->_position, false, previous_node_id);
				} else if ((previous_segment_iter->_node_id != 0) &&
					(segment_iter->_node_id != 0))
				{
					if (!previous_segment_iter->_node_copy.is_reachable(
						segment_iter->_node_id))
					{
						create_nodes(completed_position_m, 
							segment_iter->_position, false, previous_node_id);
					}
				}
			} else if ((previous_node_id != 0) && 
				(! _tile_cache->get(Node::tile_id(previous_node_id))->
				node(previous_node_id).is_reachable(segment_iter->_node_id)))
			{
				create_nodes(completed_position_m, 
					segment_iter->_position, false, previous_node_id);
			}
			
			completed_position_m = segment_iter->_position;
			Node::Id used_node_id = segment_iter->_node_id;
			
			bool merge = true;
			if (used_node_id == 0)
			{
				merge = false;
				GPSPoint new_node_position = _filtered_trace.
					gps_point_at(segment_iter->_position);
				used_node_id = create_new_node(new_node_position);
				insert_into_processed_nodes(used_node_id, completed_position_m);
			}
			
			if (merge)
			{				
				GPSPoint merge_node_position = _filtered_trace.
					gps_point_at(segment_iter->_position);
				double weight_on_first = (
					double(segment_iter->_node_copy.get_weight()) /
					(double(segment_iter->_node_copy.get_weight()) + 1.0));				
				GPSPoint merged_position = GeoCoordinate::
					interpolate_default(
						segment_iter->_node_copy, 
						merge_node_position,
						weight_on_first
					);
				Node merged_node(merged_position);
				
				TileCache::Pointer tile
					= _tile_cache->get(Node::tile_id(segment_iter->_node_id));
				if (tile != 0)
				{
					bool result = tile.write().move_node(
						segment_iter->_node_id, merged_node);
					
					if (!result)
					{
						mlog(MLog::notice, "TraceProcessor::use_segment")
							<< "Could not move node!\n";
						
						// Test implementation.
						// Well, in 99.9% this will work. But when there are
						// connections from other tiles than "tile", it will
						// (and have to) crash.
						// First solution idea: Double-linked nodes.
						
						
/*						// save moving_node and its ids:
						Node moving_node = tile->node(segment_iter->_range_id);
						Node::Id moving_node_old_id = segment_iter->_range_id;
						// done.
						
						// remove moving_node from old tile:
						tile.write().remove_node(segment_iter->_range_id);
						// done.
						
						// get new tile and save moving_point there:
						Tile::Id moving_node_tile_id = moving_node.get_tile_id();
						TileCache::Pointer new_tile
							= _tile_cache->get(moving_node_tile_id);
						Node::Id moving_node_new_id
							= new_tile.write().add_node(moving_node);
						// done.
						
						// change next_node_ids of moving_node's predecessors:
						FixpointVector<Node>& old_tile_nodes = tile.write().nodes();
						FixpointVector<Node>::iterator old_tile_nodes_iter
							= old_tile_nodes.begin();
						FixpointVector<Node>::iterator old_tile_nodes_iter_end
							= old_tile_nodes.end();
						for(; old_tile_nodes_iter != old_tile_nodes_iter_end;
							++old_tile_nodes_iter)
						{
							std::pair<bool, Node> the_pair = *old_tile_nodes_iter;
							if(the_pair.first &&
								the_pair.second.is_reachable(moving_node_old_id))
							{
								std::vector<Node::Id>::iterator iter
									= the_pair.second.next_node_ids().begin();
								std::vector<Node::Id>::iterator iter_end
									= the_pair.second.next_node_ids().end();
								
								while((iter != iter_end) && (*iter != moving_node_old_id))
									++iter;
								*iter = moving_node_new_id;
							}
						}
						// done.
						*
					} // end if(!result)
				} // end if(tile != 0)
			} // end if(merge)
			
			if (previous_node_id != 0)
				connect_nodes(previous_node_id, used_node_id);
			previous_node_id = used_node_id;
			
			previous_segment_iter = segment_iter;
		}

	}*/
	
	
} // namespace mapgeneration
