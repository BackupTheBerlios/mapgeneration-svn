/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceprocessor.h"

#include <cassert>
#include <fstream>


namespace mapgeneration
{

	TraceProcessor::TraceProcessor(unsigned int id, TileManager* tile_manager,
		pubsub::ServiceList* service_list, FilteredTrace& filtered_trace)
	: _filtered_trace(filtered_trace), _id(id), _service_list(service_list),
		_tile_manager(tile_manager), _trace_log(0)
	{
		_tile_cache = _tile_manager->get_tile_cache();
		
		/* Some variables we will need. */
		_best_cluster_node = 0;
		_current_node = 0;
		_current_node_id;
		_current_node_before_merge;
		_end_main_loop = false;
		_last_clustering_was_at_crossing = false;
		_previous_node = 0;
		_previous_node_id;

		_service_list->get_service_value(
			"traceprocessor.search_step_size_m", _search_step_size_meters
		);
		
		_search_step_size_meters = 10.0;
		
		_service_list->get_service_value(
			"traceprocessor.search_radius_m", _search_radius_m
		);
		
		_search_radius_m = 15.0;
	
		_service_list->get_service_value(
			"traceprocessor.search_max_angle_difference_pi", _search_max_angle_difference_pi
		);
		
		_search_max_angle_difference_pi = 0.3;
		
		mlog(MLog::debug, "TraceProcessor") << "Initialised (" << _id << ").\n";		
	}


	void
	TraceProcessor::calculate_cluster_nodes(GPSPoint gps_point,
		std::list<Node::Id>& result_vector)
	{
		result_vector.clear();
		
		std::vector<unsigned int> needed_tile_ids  = gps_point.get_needed_tile_ids(_search_radius_m * 2.0);
		std::vector<unsigned int>::iterator needed_tile_ids_iter = needed_tile_ids.begin();
		std::vector<unsigned int>::iterator needed_tile_ids_iter_end = needed_tile_ids.end();
		for (; needed_tile_ids_iter != needed_tile_ids_iter_end; ++needed_tile_ids_iter)
		{
			/* Load the tiles or create a new one if none could be loaded. */
			TileCache::Pointer tile = _tile_cache->get(*needed_tile_ids_iter);
			if (tile == 0)
			{
				_tile_cache->insert(*needed_tile_ids_iter, new Tile(*needed_tile_ids_iter));
				tile = _tile_cache->get(*needed_tile_ids_iter);
				mlog(MLog::error, "TraceProcessor::calculate_best_cluster_node") 
					<< "Created new tile " << (*needed_tile_ids_iter) 
					<< " (SHOULD NOT HAPPEN HERE!)\n";
			}
			
			std::vector< std::pair<unsigned int, unsigned int> > new_cluster_nodes = 
				tile->cluster_nodes_search(gps_point, _search_radius_m, _search_max_angle_difference_pi * PI);
						
			result_vector.insert(result_vector.end(), 
				new_cluster_nodes.begin(), new_cluster_nodes.end());
		}
	}
	
	
	void
	TraceProcessor::connect_nodes(Node::Id first_node_id, 
		Node::Id second_node_id)
	{
		if (first_node_id.first == second_node_id.first &&
			first_node_id.second == second_node_id.second)
			return;

		TileCache::Pointer tile_pointer = _tile_cache->get(first_node_id.first);
		Node* node = &(tile_pointer.write().nodes()[first_node_id.second].second);
		
		double direction = node->calculate_direction(_tile_cache->
			get(second_node_id.first)->nodes()[second_node_id.second].second);
			
		node->add_next_node_id(second_node_id);
		node->set_direction(direction);

		std::cout << "Connected nodes " << first_node_id.first << ", "
			<< first_node_id.second << " and " << second_node_id.first << ", "
			<< second_node_id.second << "  Direction is " << direction << "\n";
	}
	
	
	bool
	TraceProcessor::connection_from_to(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(node_id_1.first);
		if (tile_1_pointer->nodes()[node_id_1.second].first &&
			tile_1_pointer->nodes()[node_id_1.second].second.
			is_reachable(node_id_2))
			return true;
		else
			return false;
	}
	
	
	void
	TraceProcessor::create_needed_tiles()
	{
		std::vector<unsigned int>::const_iterator tile_ids_iter = 
			_filtered_trace.needed_tile_ids().begin();
		std::vector<unsigned int>::const_iterator tile_ids_iter_end = 
			_filtered_trace.needed_tile_ids().end();
		for (; tile_ids_iter != tile_ids_iter_end; ++tile_ids_iter)
		{
			TileCache::Pointer tile_pointer = _tile_cache->get(*tile_ids_iter);
			if (tile_pointer == 0)
			{
				_tile_cache->insert(*tile_ids_iter, new Tile(*tile_ids_iter));
				mlog(MLog::debug, "TraceProcessor::create_needed_tiles") 
					<< "Created new tile " << (*tile_ids_iter) << " \n";
			}
		}		
	}


	std::pair<unsigned int, unsigned int>
	TraceProcessor::create_new_node(GPSPoint& gps_point)
	{
		Node new_node(gps_point);
		int new_tile_id = new_node.get_tile_id();
		
		TileCache::Pointer tile = _tile_cache->get(new_tile_id);
		if (tile == 0)
		{
			_tile_cache->insert(new_tile_id, new Tile(new_tile_id));
			tile = _tile_cache->get(new_tile_id);
		}
		
		
		std::pair<unsigned int, unsigned int> new_node_id =
			std::make_pair(
				new_tile_id, (unsigned int)tile.write().nodes().insert(new_node)
			);

		_trace_log->new_node(new_node_id, new_node);
		
		return new_node_id;
	}
	
	
	void
	TraceProcessor::merge_node_and_gps_point(/*bool first_point*/)
	{
		assert(_best_cluster_node != 0);
		
		_last_clustering_was_at_crossing = false;
		
		_current_node_before_merge = *_best_cluster_node;
		_best_cluster_node->merge(_filtered_trace.front());
		_current_node = _best_cluster_node;
		_current_node_id = _best_cluster_node_id;

		GPSPoint saved_gps_point_for_logging = _filtered_trace.front();
		_filtered_trace.move_start_point(*_current_node);
		
		/* time correction */
		/** @todo Fix time corrections. */
/*		std::vector<unsigned int>::iterator current_node_edge_ids_iter = _current_node->edge_ids().begin();
		std::vector<unsigned int>::iterator current_node_edge_ids_iter_end = _current_node->edge_ids().end();
		for (; current_node_edge_ids_iter != current_node_edge_ids_iter_end; ++current_node_edge_ids_iter)
		{
			set_current_edge(_edge_cache->get(*current_node_edge_ids_iter));
			
			if (!_current_edge->node_is_at_start(_current_node_id))
			{
				// time correction for edge before _current_node
				--_current_edge_node_ids_iter;
				Node& previous_node = _tile_cache->get(_current_edge_node_ids_iter->first)
					.write().nodes()[_current_edge_node_ids_iter->second].second;
				++_current_edge_node_ids_iter;

				--_current_edge_times_iter;
				double time_before_merge = *_current_edge_times_iter;
				double old_distance = previous_node.distance(_current_node_before_merge);
				double new_distance = previous_node.distance(*_current_node);
				double time_after_merge = time_before_merge * (new_distance / old_distance);
				double weight = _current_node->get_weight() - 1; // first approximation!
				
				if (first_point)
					*_current_edge_times_iter = time_after_merge;
				else
					*_current_edge_times_iter =
							(time_after_merge * weight + _filtered_trace.get_time_from_previous_start())
							/ (weight + 1);

				++_current_edge_times_iter;
			}
			
			if (!_current_edge->node_is_at_end(_current_node_id))
			{
				// time correction for edge after _current_node 
				++_current_edge_node_ids_iter;
				Node& next_node = _tile_cache->get(_current_edge_node_ids_iter->first)
					.write().nodes()[_current_edge_node_ids_iter->second].second;
				--_current_edge_node_ids_iter;

				double time_before_merge = *_current_edge_times_iter;
				double old_distance = _current_node_before_merge.distance(next_node);
				double new_distance = _current_node->distance(next_node);
				*_current_edge_times_iter = time_before_merge * (new_distance / old_distance);
			}
		}*/

		if (_current_node->next_node_ids().size() > 1)
			_last_clustering_was_at_crossing = true;
		
		_trace_log->merge_node(_best_cluster_node_id, saved_gps_point_for_logging,
			*_best_cluster_node);
	}

	
	void
	TraceProcessor::move_current_to_previous()
	{
		_previous_node = _current_node;
		_previous_node_id = _current_node_id;
		
		/* Don't  rely on these! May be removed for speed reasons. */
		/* But now we relay on them!!! Possible reasons? Perhaps mergeable()? */
		_current_node = 0;
		_current_node_id.first = 0;
		_current_node_id.second = 0;
		
		_best_cluster_node = 0;
	}
	
	
	double
	TraceProcessor::optimal_node_position(PathEntry path_entry)
	{
		GeoCoordinate entry_coordinate =
			_tile_cache->get(path_entry._node_id.first)->
			nodes()[path_entry._node_id.second].second;
		double position = path_entry._position;
		double previous_distance = 1000000.0;
		double distance = previous_distance - 1.0;
		GeoCoordinate path_coordinate;
		
		double best_position = position;
		
		while (distance < previous_distance)
		{
			previous_distance = distance;
			position += 1.0;
			path_coordinate = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate.distance(path_coordinate);
			if (distance < previous_distance)
			{
				distance = entry_coordinate.distance(path_coordinate);
				best_position = position;
			}
		}
		
		previous_distance = distance + 1.0;
		while (distance < previous_distance)
		{
			previous_distance = distance;
			position -= 1.0;
			path_coordinate = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate.distance(path_coordinate);
			if (distance < previous_distance)
			{
				distance = entry_coordinate.distance(path_coordinate);
				best_position = position;
			}
		}
		
		return best_position;
	}
	
	
	void
	TraceProcessor::simplify_path(std::list<PathEntry>& path)
	{
		/*
		 * Very short pathes are simplified by clearing:
		 * we don't want that short connections.
		 */
		if ((path.back()._position - path.front()._position) < 50.0)
		{
			std::cout << "Simplifying by clearing, path has a total length of just "
				<< path.front()._position - path.back()._position << "\n";				
			path.clear();
			return;
		}
		
		std::list< std::list< std::list<PathEntry>::iterator > > possible_paths;
		
		std::list<PathEntry>::iterator init_iter = path.begin();
		std::list<PathEntry>::iterator path_end_iter = path.end();
		int first_path_id = init_iter->_path_id;
		std::list< std::list<PathEntry>::iterator > new_path;
		new_path.push_back(init_iter);
		possible_paths.push_back(new_path);
		std::cout << "Added path " << possible_paths.back().back()->_path_id << " to possible pathes.\n";
		++init_iter;
		
		while (init_iter != path_end_iter && first_path_id != init_iter->_path_id)
		{
			new_path.clear();
			new_path.push_back(init_iter);
			possible_paths.push_back(new_path);
			std::cout << "Added path " << possible_paths.back().back()->_path_id << " to possible pathes.\n";
			++init_iter;
		}
		
		std::cout << "Created " << possible_paths.size() << " initial possible pathes.\n";
		
		std::list< std::list< std::list<PathEntry>::iterator > > new_possible_paths;
		bool end = false;
		while (!end)
		{
			end = true;
			
			std::list< std::list< std::list<PathEntry>::iterator > >::iterator ppath_iter = possible_paths.begin();
			for (; ppath_iter != possible_paths.end(); ++ppath_iter)
			{
				bool created_longer_path = false;
				
				std::vector<int> used_path_ids;
				std::list<PathEntry>::iterator pos_iter = ppath_iter->back();
				int last_path_id = ppath_iter->back()->_path_id;
				++pos_iter;
				while (pos_iter!=path.end() && pos_iter->_path_id!=last_path_id &&
					std::find(used_path_ids.begin(), used_path_ids.end(), pos_iter->_path_id)==used_path_ids.end())
				{
					created_longer_path = true;
					used_path_ids.push_back(pos_iter->_path_id);
					std::list< std::list<PathEntry>::iterator > new_possible_path(*ppath_iter);
					new_possible_path.push_back(pos_iter);
					new_possible_paths.push_back(new_possible_path);
					
					std::cout << "Added new_possible_path with length " << new_possible_path.size() << "\n";
					
					end = false;
					++pos_iter;
				}
				
				if (pos_iter != path.end())
				{
					created_longer_path = true;					
					std::list< std::list<PathEntry>::iterator > new_possible_path(*ppath_iter);
					new_possible_path.push_back(pos_iter);
					new_possible_paths.push_back(new_possible_path);
					
					std::cout << "Added new_possible_path with length " << new_possible_path.size() << "\n";
					
					end = false;
				}
				
				if (created_longer_path == false)
				{
					std::list< std::list<PathEntry>::iterator > new_possible_path(*ppath_iter);
					new_possible_paths.push_back(new_possible_path);
					
					std::cout << "Copied new_possible_path with length " << new_possible_path.size() << "\n";
				}
			}
			
			possible_paths = new_possible_paths;
			new_possible_paths.clear();
		}
		
		std::cout << "Created " << possible_paths.size() << " final possible pathes.\n";
		
		std::list< std::list< std::list<PathEntry>::iterator > >::iterator ppath_iter = possible_paths.begin();
		std::list< std::list< std::list<PathEntry>::iterator > >::iterator best_path_iter = possible_paths.begin();
		double best_points = -1000000;
		for (; ppath_iter != possible_paths.end(); ++ppath_iter)
		{
			double points = 0;
			std::list< std::list<PathEntry>::iterator >::iterator x_iter = ppath_iter->begin();
			std::list< std::list<PathEntry>::iterator >::iterator old_x_iter = x_iter;
			if (x_iter != ppath_iter->end())
				++x_iter;				
			std::cout << "**************************\n";
			for (; x_iter != ppath_iter->end(); ++x_iter)
			{				
				Node::Id old_node_id = std::make_pair((*old_x_iter)->_node_id.first, (*old_x_iter)->_node_id.second);				
				Node::Id node_id = std::make_pair((*x_iter)->_node_id.first, (*x_iter)->_node_id.second);
				std::cout << old_node_id.first << ", " << old_node_id.second << " -> " << node_id.first << ", " << node_id.second << ": ";
				
				TileCache::Pointer p1 = _tile_cache->get(old_node_id.first);
				TileCache::Pointer p2 = _tile_cache->get(node_id.first);
				
				/*
				 * Negative points for distance between nodes that are not
				 * connected.
				 */
				if (! p1->nodes()[old_node_id.second].second.is_reachable(node_id))
					points -= p1->nodes()[old_node_id.second].second.distance(p2->nodes()[node_id.second].second);
				
				/*
				 * Negative points for the direction difference.
				 */
				// Not yet implemented.
				
				std::cout << points << "\n";
				
				if (points < best_points) break;
				
				old_x_iter = x_iter;
			}
			
			if (points > best_points)
			{
				best_points = points;
				best_path_iter = ppath_iter;
				std::cout << "Found new best path with " << points << " value points.\n";
			}
		}


		std::cout << "Creating new path.\n";
		std::list< std::list<PathEntry>::iterator >::iterator bp_iter = best_path_iter->begin();
		std::list<PathEntry> n_path;
		for (; bp_iter != best_path_iter->end(); ++bp_iter)
		{
			n_path.push_back(**bp_iter);
		}
		
		std::cout << "Checking new path for too short connections.\n";
		std::list<PathEntry>::iterator n_path_iter = n_path.begin();
		std::list<PathEntry>::iterator previous_n_path_iter = n_path.begin();
		std::list<PathEntry>::iterator switched_at_iter = n_path.begin();
		int connection_length = 0;
		while (n_path_iter != n_path.end())
		{
			if ((previous_n_path_iter != n_path_iter) &&
				(!(_tile_cache->get(previous_n_path_iter->_node_id.first)->
				nodes()[previous_n_path_iter->_node_id.second].second.
				is_reachable(n_path_iter->_node_id))))
			{
				if (connection_length < 5)
				{										
					while (switched_at_iter != n_path_iter)
						switched_at_iter = n_path.erase(switched_at_iter);
					std::cout << "Kicked to short connection of length " << connection_length << "\n";
				}
				
				switched_at_iter = n_path_iter; // See while loop above!
				connection_length = 0;
			}
			
			++connection_length;
			previous_n_path_iter = n_path_iter;
			++n_path_iter;
		}
		
		path.clear();
		
		if (n_path.size() > 4)
			path.insert(path.end(), n_path.begin(), n_path.end());
		
		std::cout << "New path has " << path.size() << " path entries\n";
		
	}


	void
	TraceProcessor::run()
	{
		mlog(MLog::info, "TraceProcessor") << "Started (" << _id << ").\n";
		
		create_needed_tiles();
	
		std::ostringstream tracelog_filename;
		tracelog_filename << "tracelog" << _id;
		tracelog_filename.flush();
		_trace_log = new TraceLogWriter(_tile_manager, 
			tracelog_filename.str(), _filtered_trace);

		/* Filter the trace. */
//		_filtered_trace.filter(); this is done in the TraceFilter!!!
		_filtered_trace.calculate_directions();
		//_filtered_trace.calculate_times();
				
		double trace_length_m = _filtered_trace.length_meters();
		double position_on_trace_m = 0;
		double distinct_position_m = 0;
		double complete_position_m = 0;
		double finished_position_m = 0;
		int previous_path_id = 0;
		bool used_different_path_ids = false;
		int next_path_id = 1;
		Node::Id previous_node_id = std::make_pair(0, 0);
		std::list<PathEntry> path;				
		while (position_on_trace_m < trace_length_m)
		{
			std::list<Node::Id> cluster_nodes;
			calculate_cluster_nodes(
				_filtered_trace.gps_point_at(position_on_trace_m),
				cluster_nodes
			);
			
			
			std::cout << "Found " << cluster_nodes.size() << " cluster nodes\n";


			std::list<Node::Id>::iterator new_node_iter = cluster_nodes.begin();
			while (new_node_iter != cluster_nodes.end())
			{
				PathEntry new_entry(position_on_trace_m, next_path_id, *new_node_iter);
				++next_path_id;
				std::list<PathEntry>::iterator path_iter = path.begin();
				
				bool insert = true;
				for (; path_iter != path.end() && insert == true; ++path_iter)
				{
					if (*path_iter == new_entry)
					{
						std::cout << "Catched double entry: " << new_entry._node_id.first << ", " << 
						new_entry._node_id.second << "\n";
					
						insert = false;
						new_node_iter = cluster_nodes.erase(new_node_iter);
						
					} else if (_tile_cache->get(path_iter->_node_id.first)->nodes()[path_iter->_node_id.second].second.connected_nodes()==1 &&
						_tile_cache->get(path_iter->_node_id.first)->nodes()[path_iter->_node_id.second].second.is_reachable(new_entry._node_id))
					{
						new_entry._path_id = path_iter->_path_id;
					}
				}
				
				if (insert)
				{										
					std::cout << "Inserting new node into path: " <<
						new_entry._node_id.first << ", " << new_entry._node_id.second << "  ID: " << new_entry._path_id << "\n";
						
					if (previous_path_id!=0 && new_entry._path_id!=previous_path_id)
						used_different_path_ids = true;						
					
					previous_path_id = new_entry._path_id;
					
					std::cout << "  Optimizing position starting with " << position_on_trace_m << "\n";				
					double optimal_position = optimal_node_position(new_entry);
					std::cout << "  Optimal position is " << optimal_position << "\n";
					new_entry._position = optimal_position;
					
					/*std::list<PathEntry>::iterator*/ path_iter = path.begin();
					for (; path_iter!=path.end() && 
						path_iter->_position<new_entry._position; ++path_iter);
					path.insert(path_iter, new_entry);
				}

				++new_node_iter;
			}


			if (!used_different_path_ids || (!cluster_nodes.size() && !path.size()))
			{
				distinct_position_m = position_on_trace_m;
				std::cout << "Set distinct_position_m to " << distinct_position_m << "\n";
			}


			if ((path.size() && used_different_path_ids) ||
				(path.size() && (path.back()._position < position_on_trace_m-20.0)))
			{
				used_different_path_ids = false;
				
				std::list<PathEntry>::iterator path_iter = path.end();
				--path_iter;
				int path_id = path_iter->_path_id;
				int ident_path_ids = 0;
				while (path_iter != path.begin())
				{
					if (path_iter->_path_id == path_id)
						++ident_path_ids;
					--path_iter;
				}
				if (path_iter->_path_id == path_id)
					++ident_path_ids;
				if ((ident_path_ids > 5) || (path.back()._position < position_on_trace_m-20.0))
				{
					std::cout << "Last " << ident_path_ids << " path ids are identically " << path_id << " simplifying path.\n";
					simplify_path(path);
					distinct_position_m = position_on_trace_m;
					std::cout << "Done, set distinct position_m to " << distinct_position_m << "\n";
				}
			}


//			std::list<PathEntry>::iterator path_iter = path.begin();
			bool end = false;
			while(complete_position_m < distinct_position_m-20 && !end)
			{
				if ((path.size() && complete_position_m<(path.front()._position-20.0))
					|| (!path.size() && complete_position_m<(distinct_position_m-20.0)))
				{
					complete_position_m += 10.0;
					std::cout << "Creating new node at position " << complete_position_m << "\n";
					GPSPoint new_node_position = _filtered_trace.
						gps_point_at(complete_position_m);
					Node::Id new_node_id = create_new_node(new_node_position);
					if (previous_node_id.first != 0)
						connect_nodes(previous_node_id, new_node_id);
					previous_node_id.first = new_node_id.first;
					previous_node_id.second = new_node_id.second;
				} else if (path.size() /*&& complete_position_m>=(path.front()._position-20.0)*/)
				{
					bool connect = true;
					if (previous_node_id.first!=0 && 
						!connection_from_to(previous_node_id, path.front()._node_id))
					{
						int connection_length = 0;
						std::list<PathEntry>::iterator path_iter = path.begin();
						std::list<PathEntry>::iterator 
							previous_path_iter = path_iter;
						++path_iter;
						while (path_iter!=path.end() && 
							connection_from_to(previous_path_iter->_node_id,
							path_iter->_node_id))
						{
							++connection_length;
							previous_path_iter = path_iter;
							++path_iter;
						}	
						std::cout << "Connection length is " << connection_length << "\n";
						if (connection_length < 3)
						{							
							if (path_iter!=path.end() || path.back()._position<position_on_trace_m-20.0)
							{								
								std::cout << "Not using node at path position " << path.front()._position << "\n";								
								path.pop_front();
							}
							end = true;
							connect = false;
						} 
					}
					if (connect)
					{
						std::cout << "Using node at path position " << complete_position_m << "\n";
						complete_position_m = path.front()._position;
						if (previous_node_id.first != 0)
							connect_nodes(previous_node_id, path.front()._node_id);
						previous_node_id.first = path.front()._node_id.first;
						previous_node_id.second = path.front()._node_id.second;
						path.pop_front();
					}
			}
			}

			position_on_trace_m += 10.0;
			std::cout << "Walked to " << position_on_trace_m << "********************************************\n";
			
		}
			
		delete _trace_log;

		mlog(MLog::info, "TraceProcessor") << "Processed a " << 
			trace_length_m << "m long trace.\n";
		mlog(MLog::info, "TraceProcessor") << "Finished (" << _id << ").\n";
		_tile_manager->trace_processor_finished(_id);
	}
	
	
	TileCache::Pointer
	TraceProcessor::tile(Node::Id node_id)
	{
		return _tile_cache->get(node_id.first);
	}


} // namespace mapgeneration

