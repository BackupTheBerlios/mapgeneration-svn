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
		_tile_manager(tile_manager), _trace_log(0), _time(0)
	{
		_tile_cache = _tile_manager->get_tile_cache();

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
		
		_search_max_angle_difference_pi = 0.4;
		
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
			
			std::vector<Node::Id> new_cluster_nodes = 
				tile->cluster_nodes_search(gps_point, _search_radius_m, _search_max_angle_difference_pi * PI);
						
			result_vector.insert(result_vector.end(), 
				new_cluster_nodes.begin(), new_cluster_nodes.end());
		}
	}
	
	
	void
	TraceProcessor::connect_nodes(Node::Id first_node_id, 
		Node::Id second_node_id)
	{
		if (first_node_id == second_node_id)
			return;

		TileCache::Pointer tile_pointer = _tile_cache->get(Node::tile_id(first_node_id));
		Node* node = &(tile_pointer.write().nodes()[Node::local_id(first_node_id)].second);
		
		double direction = node->calculate_direction(_tile_cache->
			get(Node::tile_id(second_node_id))->nodes()[Node::local_id(second_node_id)].second);
			
		node->add_next_node_id(second_node_id);
		node->set_direction(direction);

		/*std::cout << "Connected nodes " << Node::tile_id(first_node_id) << ", "
			<< Node::local_id(first_node_id) << " and " << Node::tile_id(second_node_id) << ", "
			<< Node::local_id(second_node_id) << "  Direction is " << direction << "\n";*/
	}
	
	
	bool
	TraceProcessor::connection_from_to(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(Node::tile_id(node_id_1));
		if (tile_1_pointer->nodes()[Node::local_id(node_id_1)].first &&
			tile_1_pointer->nodes()[Node::local_id(node_id_1)].second.
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


	Node::Id
	TraceProcessor::create_new_node(GPSPoint& gps_point)
	{
		Node new_node(gps_point);
		unsigned int new_tile_id = new_node.get_tile_id();
		
		TileCache::Pointer tile = _tile_cache->get(new_tile_id);
		if (tile == 0)
		{
			_tile_cache->insert(new_tile_id, new Tile(new_tile_id));
			tile = _tile_cache->get(new_tile_id);
		}
		
		unsigned int new_node_id_part = (unsigned int)tile.write().nodes().insert(new_node);
		
		Node::Id new_node_id = Node::merge_id_parts(new_tile_id, new_node_id_part);

//		_trace_log->new_node(new_node_id, new_node);
		
		return new_node_id;
	}
	
	
	double
	TraceProcessor::distance_from_to(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(Node::tile_id(node_id_1));
		TileCache::Pointer tile_2_pointer = _tile_cache->get(Node::tile_id(node_id_2));		
		
		return (tile_1_pointer->nodes()[Node::local_id(node_id_1)].second.
			distance(tile_2_pointer->nodes()[Node::local_id(node_id_2)].second));
	}
	
	
	void
	TraceProcessor::merge_node_and_gps_point(/*bool first_point*/)
	{
/*		assert(_best_cluster_node != 0);
		
		_last_clustering_was_at_crossing = false;
		
		_current_node_before_merge = *_best_cluster_node;
		_best_cluster_node->merge(_filtered_trace.front());
		_current_node = _best_cluster_node;
		_current_node_id = _best_cluster_node_id;

		GPSPoint saved_gps_point_for_logging = _filtered_trace.front();
		_filtered_trace.move_start_point(*_current_node);
		
		/* time correction *
		/** @todo Fix time corrections. *
		std::vector<unsigned int>::iterator current_node_edge_ids_iter = _current_node->edge_ids().begin();
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
		}

		if (_current_node->next_node_ids().size() > 1)
			_last_clustering_was_at_crossing = true;
		
		_trace_log->merge_node(_best_cluster_node_id, saved_gps_point_for_logging,
			*_best_cluster_node);*/
	}
	
	
	double
	TraceProcessor::optimal_node_position(PathEntry path_entry)
	{
		/* That does not work really. I have to think about that. But probably
		 * the implementation below is faster (and it's easier!).
		 * 
		 * Next Problem: We do not a cartesian coordinate system...
		 */
		 
/*		FilteredTrace::const_iterator point_before;
		FilteredTrace::const_iterator point_after;
		double point_before_meters;
		double point_after_meters;

		FilteredTrace::const_iterator dummy;
		double dummy_meters;
		
		bool successful
			= _filtered_trace.gps_points_before_and_after(
				path_entry._position - _search_radius_m,
				&point_before,
				&dummy,
				&point_before_meters
			);
		if (!successful)
			return path_entry._position;
			
		successful
			= _filtered_trace.gps_points_before_and_after(
				path_entry._position + _search_radius_m,
				&dummy,
				&point_after,
				&dummy_meters,
				&point_after_meters
			);
		if (!successful)
			return path_entry._position;
		
		GeoCoordinate entry_coordinate =
			_tile_cache->get(path_entry._node_id.first)->
			nodes()[path_entry._node_id.second].second;

		FilteredTrace::const_iterator iter = point_before;
		FilteredTrace::const_iterator previous_iter = point_before;
		++iter;
		
		FilteredTrace::const_iterator best_iter = point_before;
		double best_distance = 1000000.0;
		
		while (previous_iter != point_after)
		{
			const GeoCoordinate p = entry_coordinate;
			const GeoCoordinate o = *previous_iter;
			const GeoCoordinate v = *iter - *previous_iter;
			
			const GeoCoordinate p_minus_o = p - o;

			double t_0 = (v * p_minus_o) / (v * v);
			GeoCoordinate o_plus_t_0_v = o + (t_0 * v);
			double distance = (p - (o_plus_t_0_v)).abs();
			if (distance < 0.0)
				distance = p_minus_o.abs();
			else if (distance > 1.0)
				distance = (p - (o + v)).abs();
			
			if (distance < best_distance)
			{
				best_distance = t_0;
				best_iter = previous_iter;
			}
			
			++previous_iter;
			++iter;
		}
		
		if (best_distance <= 1.0)
		{
			FilteredTrace::const_iterator next_iter = best_iter;
			++next_iter;
			
			double position = _filtered_trace.length_m(point_before, best_iter)
				+ t_0 * _filtered_trace.length_m(best_iter, next_iter);
			
			return position;
			
		} else // explicit: if (best_distance > 1.0)
		{
			
		}
		
		
			
			
			
			/* calculate ...
			double t_0 = [v * (p - o)] / [ v * v]
			point p = o + t_0 * v
			
			o = point_before
			v = point_after - point_before
			p = entry_coordinate *
			
			const GeoCoordinate p = entry_coordinate;
			const GeoCoordinate o = *point_before;
			const GeoCoordinate v = *point_after - *point_before;
			
			return ( ((p - o) * v) / (v * v) );
			
			
		} else
		{
			return path_entry._position;
		}*/
		
		
		
		
		GeoCoordinate entry_coordinate = _tile_cache->
			get(Node::tile_id(path_entry._node_id))->
			nodes()[Node::local_id(path_entry._node_id)].second;
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
			distance = entry_coordinate.approximated_distance(path_coordinate);
			if (distance < previous_distance)
			{
//				distance = entry_coordinate.distance(path_coordinate);
				best_position = position;
			}
		}
		
		previous_distance = distance + 1.0;
		while ((distance < previous_distance) && (position >= 1.0))
		{
			previous_distance = distance;
			position -= 1.0;
			path_coordinate = _filtered_trace.gps_point_at(position);
			distance = entry_coordinate.approximated_distance(path_coordinate);
			if (distance < previous_distance)
			{
//				distance = entry_coordinate.distance(path_coordinate);
				best_position = position;
			}
		}
		
		return best_position;
	}
	
	
	void
	TraceProcessor::rebuild_path(std::list<PathEntry>& path,
		PathEntry* start_entry)
	{
		std::list<PathEntry> new_path;
		
		std::cout << "Rebuilding path:\n";
		
		PathEntry* position = start_entry;
		PathEntry* next_position = 0;
		while (position != 0)
		{
			std::cout << Node::tile_id(position->_node_id) << ", " << 
				Node::local_id(position->_node_id) << "\n";
				
			next_position = position->_connection;
			position->_connection = 0;
			new_path.push_back(*position);
			
			position = next_position;
		}
		
		std::cout << "New path ready!\n";
		
		path.clear();
		path.insert(path.end(), new_path.begin(), new_path.end());
	}
	
	
	double
	TraceProcessor::build_connections(std::list<PathEntry>& path,
		std::list<PathEntry>::iterator path_iter, bool only_connected)
	{
		std::cout << Node::tile_id(path_iter->_node_id) << ", " << 
							Node::local_id(path_iter->_node_id) << "\n";
		if (path_iter->_time_stamp == _time)
			return path_iter->_points;
		
		std::list<PathEntry>::iterator current_entry = path_iter;
		current_entry->_time_stamp = _time;
		
		std::list<PathEntry>::iterator destination_iter = path.end();
		--destination_iter;
		if (path_iter == destination_iter)
		{
			//std::cout << "Reached destination point!";
			current_entry->_connection = 0;
			current_entry->_points = 100000.0;
			
			return current_entry->_points;
		}
		
		current_entry->_connection = 0;
		current_entry->_points = -100000.0;
		
		TileCache::Pointer tile_pointer = _tile_cache->get(Node::tile_id(current_entry->_node_id));
		std::vector<Node::Id>::const_iterator next_nodes_iter = 
			tile_pointer->nodes()[Node::local_id(current_entry->_node_id)].second.next_node_ids().begin();
		std::vector<Node::Id>::const_iterator next_nodes_iter_end =
			tile_pointer->nodes()[Node::local_id(current_entry->_node_id)].second.next_node_ids().end();
		for (; next_nodes_iter != next_nodes_iter_end; ++next_nodes_iter)
		{
			std::list<PathEntry>::iterator path_iter = path.begin();
			std::list<PathEntry>::iterator path_iter_end = path.end();
			for (; (path_iter != path_iter_end) && 
				(path_iter->_node_id != *next_nodes_iter); ++path_iter);
			
			if (path_iter != path_iter_end)
			{
				std::cout << "C-->\n";
				double points = build_connections(path, path_iter, only_connected);
				std::cout << "C<--\n";
			
				std::cout << "Points: " << points;
				
				points -= distance_from_to(current_entry->_node_id, path_iter->_node_id) / 10;
				
				std::cout << "  Modified to " << points;
					
				std::cout << "\n";
				
				if (points > current_entry->_points)
				{
					current_entry->_points = points;
					current_entry->_connection = &(*path_iter);
				}
			}
		}
		
		if (!only_connected)
		{
			++path_iter;
			std::list<PathEntry>::iterator path_iter_end = path.end();
			while ((path_iter != path_iter_end) && 
				(path_iter->_position < current_entry->_position + 50.0))
			{
				bool connected =
					connection_from_to(current_entry->_node_id, path_iter->_node_id);
					
	//			if ((only_connected && connected) || !only_connected)
				if (!connected)
				{
					std::cout << "-->\n";
					double points = build_connections(path, path_iter, only_connected);
					std::cout << "<--\n";
				
					std::cout << "Points: " << points;
										
					points -= 1000;
					
					points -= distance_from_to(current_entry->_node_id, path_iter->_node_id);
					
					std::cout << "  Modified to " << points;
	
					std::cout << "\n";
					
					if (points > current_entry->_points)
					{
						current_entry->_points = points;
						current_entry->_connection = &(*path_iter);
					}
				}
				
				++path_iter;
			}
		}

		return current_entry->_points;
	}
	
	
	void
	TraceProcessor::simplify_path(Node::Id start_node_id, 
		std::list<PathEntry>& path)
	{
		
		if (path.size() < 2)
		{
			std::cout << "PATH IS TOO SHORT!!!";
			return;
		}
		
		double best_points = -100000.0;
		PathEntry* best_start_entry = 0;

		std::cout << "Trying to find connected way.\n";
		
		++_time;
		
		if (start_node_id != 0)
		{
			PathEntry start_entry;
			
			// The position is just a rough approximation, but that's 
			// enough here.
			start_entry._position = path.front()._position - 5.0;
			start_entry._node_id = start_node_id;			
			path.insert(path.begin(), start_entry);
						
			best_points = build_connections(path, path.begin(), true);
			best_start_entry = path.front()._connection;
		} else
		{
			double start_position = path.front()._position;
			std::list<PathEntry>::iterator path_iter = path.begin();
			std::list<PathEntry>::iterator path_iter_end = path.end();
			while ((path_iter != path_iter_end) && 
				(path_iter->_position < start_position + 50.0))
			{
				double points = build_connections(path, path_iter, true);
				
				std::cout << points << " points.";
				
				if (points > best_points)
				{
					std::cout << " NEW BEST!";
					best_points = points;
					best_start_entry = &(*path_iter);
				}
				
				std::cout << "\n";
				
				++path_iter;
			}
		}
		
		if (best_start_entry == 0)
		{
			++_time;
			
			std::cout << "Could not find connection, searching for unconnected best way.\n";
			if (start_node_id != 0)
			{
				PathEntry start_entry;
				
				// The position is just a rough approximation, but that's 
				// enough here.
				start_entry._position = path.front()._position - 5.0;
				start_entry._node_id = start_node_id;			
				path.insert(path.begin(), start_entry);
							
				best_points = build_connections(path, path.begin(), false);
				best_start_entry = path.front()._connection;
			} else
			{
				double start_position = path.front()._position;
				std::list<PathEntry>::iterator path_iter = path.begin();
				std::list<PathEntry>::iterator path_iter_end = path.end();
				while ((path_iter != path_iter_end) && 
					(path_iter->_position < start_position + 50.0))
				{
					double points = build_connections(path, path_iter, false);
					
					std::cout << points << " points.";
					
					if (points > best_points)
					{
						std::cout << " NEW BEST!";						
						best_points = points;
						best_start_entry = &(*path_iter);
					}
					
					std::cout << "\n";
					
					++path_iter;
				}
			}	
		}
		
		if ((best_start_entry != 0) && (best_points > -10000.0))
		{
			std::cout << "Found way with " << best_points << " points.\n";
			rebuild_path(path, best_start_entry);
		} else
		{
			std::cout << "Could not find any way, clearing path.\n";
			path.clear();
		}
				
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

		_filtered_trace.calculate_directions();
		_filtered_trace.precompute_data();
				
		double trace_length_m = _filtered_trace.length_m();
		double position_on_trace_m = 0;
		double distinct_position_m = 0;
		double complete_position_m = 0;
		double finished_position_m = 0;
		int previous_path_id = 0;
		int next_path_id = 1;
		Node::Id previous_node_id = 0;
		std::list<PathEntry> path;
		while (position_on_trace_m < trace_length_m)
		{
			std::list<Node::Id> cluster_nodes;
			calculate_cluster_nodes(
				_filtered_trace.gps_point_at(position_on_trace_m),
				cluster_nodes
			);
						
			//std::cout << "Found " << cluster_nodes.size() << " cluster nodes\n";
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
						insert = false;
						new_node_iter = cluster_nodes.erase(new_node_iter);
						
					}
				}
				
				if (insert)
				{										
					std::cout << "Inserting new node into path: " <<
						Node::tile_id(new_entry._node_id) << ", " << 
						Node::local_id(new_entry._node_id) << "\n.";
					
					//std::cout << "  Optimizing position starting with " << position_on_trace_m << "\n";
					double optimal_position = optimal_node_position(new_entry);
					//std::cout << "  Optimal position is " << optimal_position << "\n";
					new_entry._position = optimal_position;
					
					/*std::list<PathEntry>::iterator*/ path_iter = path.begin();
					for (; path_iter!=path.end() && 
						path_iter->_position<new_entry._position; ++path_iter);
					path.insert(path_iter, new_entry);
				}

				++new_node_iter;
			}


			if (!cluster_nodes.size() && !path.size())
			{
				distinct_position_m = position_on_trace_m;
				//std::cout << "Set distinct_position_m to " << distinct_position_m << "\n";
			}


/*			if ((path.size() && used_different_path_ids) ||
				(path.size() && (path.back()._position < position_on_trace_m-20.0)))*/
			if (path.size() > 5 ||
				(path.size() && (path.back()._position < position_on_trace_m-20.0)))
			{				
				int connected_nodes = 0;
				
				if (path.size() > 5)
				{
					std::list<PathEntry>::iterator path_iter = path.end();
					--path_iter;
															
					Node::Id next_node_id = path_iter->_node_id;
					--path_iter;
					while ((path_iter != path.begin()) && (connected_nodes < 3))
					{
						if (connection_from_to(path_iter->_node_id, next_node_id))
							++connected_nodes;
						
						next_node_id = path_iter->_node_id;
						--path_iter;
					}
				}
				
				if ((connected_nodes > 2) || (path.back()._position < position_on_trace_m-20.0))
				{
					std::cout << "Last " << connected_nodes << " nodes are connected.\n";
					simplify_path(previous_node_id, path);
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
					if (previous_node_id != 0)
						connect_nodes(previous_node_id, new_node_id);
					previous_node_id = new_node_id;
				} else if (path.size()/* && complete_position_m>=(path.back()._position+20.0)*/)
				{					
					complete_position_m = path.front()._position;
					Node::Id used_node_id = path.front()._node_id;
					
					std::cout << "Using node " << Node::tile_id(used_node_id) << ", " << 
						Node::local_id(used_node_id) << " at path position " << complete_position_m << "\n";
					if (previous_node_id != 0)
						connect_nodes(previous_node_id, path.front()._node_id);
					previous_node_id = path.front()._node_id;
					path.pop_front();
				}
			}

			position_on_trace_m += 10.0;
			//std::cout << "Walked to " << position_on_trace_m << "m ********************************************\n";
			
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
		return _tile_cache->get(Node::tile_id(node_id));
	}


} // namespace mapgeneration

