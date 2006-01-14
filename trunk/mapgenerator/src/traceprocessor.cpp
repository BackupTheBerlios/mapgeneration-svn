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
	: _filtered_trace(filtered_trace), _id(id), 
		_processed_nodes(), _service_list(service_list),
//		_tile_manager(tile_manager), _trace_log(0), _time(0)
		_tile_manager(tile_manager), _time(0)
	{
		_tile_cache = _tile_manager->get_tile_cache();

		_service_list->get_service_value(
			"traceprocessor.search_step_size_m", _search_step_size_meters
		);
		
		_search_step_size_meters = 8.0;
		
		_service_list->get_service_value(
			"traceprocessor.search_radius_m", _search_radius_m
		);
		
		_search_radius_m = 18.0;
	
		_service_list->get_service_value(
			"traceprocessor.search_max_angle_difference_pi", _search_max_angle_difference_pi
		);
		
		_search_max_angle_difference_pi = 0.4;
		
		mlog(MLog::debug, "TraceProcessor") << "Initialised (" << _id << ").\n";		
	}
	
	
	void
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
	}
	


	void
	TraceProcessor::calculate_cluster_nodes(GPSPoint gps_point,
		std::list<Node::Id>& result_list)
	{
		result_list.clear();
		
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
			}
			
			std::vector<Node::Id> new_cluster_nodes_fast;
			tile->fast_cluster_nodes_search(gps_point, _search_radius_m,
				_search_max_angle_difference_pi * PI, new_cluster_nodes_fast);
			
			result_list.insert(result_list.end(), 
				new_cluster_nodes_fast.begin(), new_cluster_nodes_fast.end());
		}
	}
	
	
	void
	TraceProcessor::connect_nodes(Node::Id first_node_id, 
		Node::Id second_node_id)
	{
		if (first_node_id == second_node_id)
			return;
		
		/** @todo think about uncritical write access to nodes! */
		TileCache::Pointer tile_pointer = _tile_cache->get(Node::tile_id(first_node_id));
		Node* node = &(tile_pointer.write().node(first_node_id));
		
		double direction = node->bearing_default(
			_tile_cache->get(Node::tile_id(second_node_id))->node(second_node_id) );
			
		node->add_next_node(second_node_id, direction);
	}
	
	
	bool
	TraceProcessor::connection_from_to(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(Node::tile_id(node_id_1));
		if (tile_1_pointer->exists_node(node_id_1) &&
			tile_1_pointer->node(node_id_1).is_reachable(node_id_2))
			return true;
		else
			return false;
	}
	
	
	void
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
	}
	
	
	void
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
		
		Node::Id new_node_id = tile.write().add_node(new_node);
//		_trace_log->new_node(new_node_id, new_node);
		
		return new_node_id;
	}
	
	
	void
	TraceProcessor::create_nodes(double& completed_position_m, double end_position_m,
		bool use_end_position, Node::Id& previous_node_id)
	{		
		/** 
		 * @todo The node distance should be dynamic.
		 */
		/**
		 * The while loop creates nodes up to the last step before end_position.
		 */
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
		 */
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
		
	}
	
	
	void
	TraceProcessor::cut_processed_nodes(double position_m)
	{
		std::list< std::pair<Node::Id, double> >::iterator
			proc_nodes_iter = _processed_nodes.begin();
		for (; (proc_nodes_iter != _processed_nodes.end()) &&
			(proc_nodes_iter->second < position_m);
			++proc_nodes_iter);
		if (proc_nodes_iter != _processed_nodes.begin())
		{
			proc_nodes_iter--;
			_processed_nodes.erase(_processed_nodes.begin(), proc_nodes_iter);
		}
	}
	
	
	void
	TraceProcessor::insert_into_processed_nodes(Node::Id node_id, 
		double position_m)
	{
		std::list< std::pair<Node::Id, double> >::iterator
			proc_nodes_iter = _processed_nodes.begin();
		for (; (proc_nodes_iter != _processed_nodes.end()) &&
			(proc_nodes_iter->second <= position_m) &&
			(proc_nodes_iter->first != node_id);
			++proc_nodes_iter);

		if ((proc_nodes_iter == _processed_nodes.end()) ||
			(proc_nodes_iter->first != node_id))
			_processed_nodes.insert(
				proc_nodes_iter,
				std::make_pair(node_id, position_m)
			);
	}
	
	
	double
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
	}
		
	
	double
	TraceProcessor::build_connections(std::list<PathEntry>& path,
		std::list<PathEntry>::iterator path_iter, double previous_direction)
	{
		/*
		 * This is the recursive method used to scan the path for the
		 * optimal node connections.
		 */
		/*
		 * If the time_stamp equals time we already visited this entry and
		 * can immidiatly return the stored points.
		 */
		if (path_iter->_time_stamp == _time)
			return path_iter->_points;
		
		/*
		 * Copy the path_iter to current_entry and update the time_stamp.
		 */
		std::list<PathEntry>::iterator current_entry = path_iter;
		current_entry->_time_stamp = _time;
		
		/*
		 * The destination is always the last path entry. If we have reached it
		 * we can return a great number of points for this path.
		 */
		std::list<PathEntry>::iterator destination_iter = path.end();
		--destination_iter;
		if (path_iter == destination_iter)
		{
			// connection is zero at the end of the path
			current_entry->_connection = 0;
			current_entry->_points = 100000.0;
			
			return current_entry->_points;
		}
		
		/*
		 * Before further recursion we initialize the connection with zero and
		 * points with -100000.0, because we have not yet found anything
		 * usable.
		 */
		current_entry->_connection = 0;
		current_entry->_points = -100000.0;
		
		++path_iter;
		std::list<PathEntry>::iterator path_iter_end = path.end();
		while ((path_iter != path_iter_end) && 
			(path_iter->_position < current_entry->_position + 50.0))
		{
			if ((path_iter->_node_id != 0) || (current_entry->_node_id != 0) ||
				(path_iter->_virtual_node_id == (current_entry->_virtual_node_id+1)))
			{
				double direction_to_next = current_entry->_node_copy.
					bearing_default(path_iter->_node_copy);
				
				double points = build_connections(path, path_iter, 
					direction_to_next);
					
				// Let's calculate some values:
				double step_distance = current_entry->_node_copy.
					distance_default(path_iter->_node_copy);
				
				GPSPoint point_on_trace = _filtered_trace.gps_point_at(current_entry->_position);
				
				double distance_to_trace = current_entry->_node_copy.
					distance_default(point_on_trace);
					
				double connection_direction = current_entry->_node_copy.
					bearing_default(path_iter->_node_copy);
				
				double connection_direction_difference =
					current_entry->_node_copy.minimal_direction_difference_to(
						Direction(connection_direction)
					);
				
				double connection_direction_next_difference =
					path_iter->_node_copy.minimal_direction_difference_to(
						Direction(connection_direction)
					);
					
				// Negative points for:				
				// Jump from virtual node to existing node.
				if ((current_entry->_node_id == 0) && (path_iter->_node_id != 0))
				{
					points -= step_distance * 5.0;
					points -= 20.0;
					points -= connection_direction_difference * 100.0;
					points -= connection_direction_next_difference * 100.0;
				}
					
				// Jump from existing node to virtual node.
				if ((current_entry->_node_id != 0) && (path_iter->_node_id == 0))
				{
					points -= step_distance * 5.0;
					points -= 20.0;
					points -= connection_direction_difference * 100.0;
					points -= connection_direction_next_difference * 100.0;
				}
				
				// Jump from existing node to existing node, 
				// without connection
				if (!current_entry->_node_copy.is_reachable(path_iter->_node_id))
				{
					points -= step_distance * 5.0;
					points -= 20.0;
					points -= connection_direction_difference * 100.0;
					points -= connection_direction_next_difference * 100.0;
				} else //with connection
				{
					points -= step_distance * 0.5;
				}
					
				// Jump from virtual node to virtual node.
				if ((current_entry->_node_id == 0) && (path_iter->_node_id == 0))
				{
					points -= step_distance * 1.5;				
				}
								
				// distance between point on trace and node in path
			/*	GPSPoint point_on_trace = _filtered_trace.gps_point_at(path_iter->_position);
				points -= point_on_trace.
					distance_default(path_iter->_node_copy) * 2;*/
				
				// distance between point on trace and current node
				//GPSPoint point_on_trace = _filtered_trace.gps_point_at(current_entry->_position);
				//points -= point_on_trace.
				//	distance_default(current_entry->_node_copy);
				//points -= entry_trace_distance;
				
				// direction difference between point and node
				/*points -= current_entry->_node_copy.
					minimal_direction_difference_to(point_on_trace) * 20.0;*/
				
				// distance between current nodes position and connection
				//	position
				//points -= (path_iter->_position - current_entry->_position);
				
				// complicated
				/*if (current_entry->_position < path_iter->_position-20.0)
				{
					point_on_trace = _filtered_trace.gps_point_at(current_entry->_position + 10.0);
					double direction = current_entry->_node_copy.
						bearing_default(point_on_trace);
					double diff = current_entry->_node_copy.
						minimal_direction_difference_to(Direction(direction));
					points -= diff * 5.0;
				}*/
				
	/*			if (previous_direction < 1000.0)
				{
					double dir_diff = current_entry->_node_copy.
						bearing_default(path_iter->_node_copy) - 
						previous_direction;
					double edge_length = current_entry->_node_copy.
						distance_default(path_iter->_node_copy);
					
					double curvature_on_road = 	dir_diff / edge_length;
					double curvature_on_trace = _filtered_trace.curvature_at(current_entry->_position);
					double diff = curvature_on_road - curvature_on_trace;
					diff = (diff > 0 ? diff : -diff);
					points -= diff * 100.0;
				}*/
				
				// The two blocks below could be used for smooth connects and disconnects.
				/*if (current_entry->_position < path_iter->_position-20.0)
				{
					Node next_entry_node = _tile_cache->get(
						Node::tile_id(path_iter->_node_id))->node(path_iter->_node_id);
					point_on_trace = _filtered_trace.gps_point_at(path_iter->_position - 10.0);
					double direction = point_on_trace.bearing_default(next_entry_node);
					double diff = next_entry_node.minimal_direction_difference_to(Direction(direction));
					points -= diff * 100.0;
				}
				
				if (disconnect && 
					path_iter->_node_id == destination_iter->_node_id)
				{
					Node current_entry_node = _tile_cache->get(
						Node::tile_id(current_entry->_node_id))->node(current_entry->_node_id);
					point_on_trace = _filtered_trace.gps_point_at(current_entry->_position + 10.0);
					double direction = current_entry_node.bearing_default(point_on_trace);
						double diff = current_entry_node.minimal_direction_difference_to(Direction(direction));
						points -= diff * 20.0;
					}*/
	
				if (points > current_entry->_points)
				{
					current_entry->_points = points;
					current_entry->_connection = &(*path_iter);
				}
			}
			
			++path_iter;
		}

		/*
		 * Return the optimal number of points we could reach in this path.
		 */
		return current_entry->_points;
	}
	
	
	bool	
	TraceProcessor::search_in_processed_nodes(Node::Id node_id)
	{
		std::list< std::pair<Node::Id, double> >::iterator
			proc_nodes_iter = _processed_nodes.begin();
		for (; (proc_nodes_iter != _processed_nodes.end()) &&
			(proc_nodes_iter->first != node_id);
			++proc_nodes_iter);
					
		bool found = (proc_nodes_iter != _processed_nodes.end());
		
		return found;
	}
	
	
	void
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
		 */
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
		 */
		double best_points = -100000.0;
		PathEntry* best_start_entry = 0;

		/*
		 * We just have to visit each path entry once, we use time stamps
		 * control this and increase the time before each search.
		 */
		++_time;
				
		/*
		 * If we have a start_node_id we insert it as a new PathEntry into
		 * the path and start the recursion with this entry.
		 * The best_start_entry is not the start node (as this is already
		 * processed), but the first entry after the start node entry.
		 * Else we start with each PathEntry that is not more than x
		 * meters away from the first entry and choose the best entry from
		 * the recursion results.
		 */
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
						
			best_points = build_connections(path, path.begin(), 1001.0);
			best_start_entry = path.front()._connection;
		} else
		{
			double start_position = path.front()._position;
			std::list<PathEntry>::iterator path_iter = path.begin();
			std::list<PathEntry>::iterator path_iter_end = path.end();
			while ((path_iter != path_iter_end) && 
				(path_iter->_position < start_position + 50.0))
			{
				double points = build_connections(path, path_iter, 1001.0);
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
	}


	void
	TraceProcessor::run()
	{
		mlog(MLog::info, "TraceProcessor") << "Started (" << _id << ").\n";
	
//		std::ostringstream tracelog_filename;
//		tracelog_filename << "tracelog" << _id;
//		tracelog_filename.flush();
//		_trace_log = new TraceLogWriter(_tile_manager, 
//			tracelog_filename.str(), _filtered_trace, _service_list);

		_filtered_trace.calculate_directions();
		_filtered_trace.precompute_data();
		
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
		
		// All position are relative to the start of the trace and in meters.
		
		/* 
		 * The current position of the scanner that look for existing nodes. The
		 * scanner will almost always be some steps ahead of the distinct and
		 * completed positions.
		 */		
		double scan_position_m = 0.0;
		
		/* 
		 * The position up to which the usage of the trace information is 
		 * distinct, when creating new nodes the trace information may and
		 * should be used roughly up to this position. We start with -100.0,
		 * because the position 0.0 might be used if we initialized this with
		 * 0.0.
		 */
		double distinct_position_m = -100.0;
		
		/*
		 * The position up to which the trace is already merged to the map.
		 */
		double completed_position_m = 0.0;
		
		/*
		 * All path entries that where added to the path in the current
		 * loop. This is stored to revert the last search if needed.
		 */
		std::vector< std::list<PathEntry>::iterator > new_path_entries;
		
		/*
		 * The id of the last connected node. If the next node in the path
		 * is choosen or created, a connection from the node with this id to
		 * the newly choosen or created node has to be created.
		 */
		Node::Id previous_node_id = 0;
		
		/*
		 * The list of PathEntries in the order in which the corresponding 
		 * nodes occur along the path. Each path entry contains the nodes
		 * id and the position on the trace that is associated to this node.
		 */
		std::list<PathEntry> path;
		
		/*
		 * This is set to true, if a row of connected nodes is found, that
		 * is long enough to process a part of the trace.
		 */
		bool connected_nodes_row = false;
		
		/*
		 * This is only false, if we reverted the previous search and want
		 * to search for nodes at the same position again.
		 */
		bool walk_on = true;
		
		
		int next_virtual_node_id = 0;
		
		/*
		 * The main loop. The trace is processed as long as the end is not
		 * reached.
		 */
		while (scan_position_m < _filtered_trace.length_m())
		{
			PathEntry virtual_entry(scan_position_m, 0);
			Node virtual_node(_filtered_trace.gps_point_at(scan_position_m));
			virtual_entry._node_copy = virtual_node;
			virtual_entry._virtual_node_id = next_virtual_node_id;
			++next_virtual_node_id;
			path.push_back(virtual_entry);
			
			/*
			 * At first all nodes in a certain radius around the current 
			 * position are searched.
			 */
			std::list<Node::Id> cluster_nodes;
			calculate_cluster_nodes(
				_filtered_trace.gps_point_at(scan_position_m),
				cluster_nodes
			);
			
			/*
			 * Each node from the search result is searched in the processed
			 * nodes list to prevent insertion of already processed nodes into
			 * the path.
			 */
			cut_processed_nodes(scan_position_m - (_search_radius_m * 1.5));
			
			new_path_entries.clear();
			std::list<Node::Id>::iterator new_node_iter
				= cluster_nodes.begin();
			while (new_node_iter != cluster_nodes.end())
			{
				PathEntry new_entry(scan_position_m, *new_node_iter);
				
				bool insert = !(search_in_processed_nodes(new_entry._node_id));
								
				/**
				 * If insert is true we can insert the entry into the 
				 * path and processed_nodes.
				 */
				if (insert)
				{
					new_entry._node_copy = _tile_cache->get(
						Node::tile_id(new_entry._node_id))->
						node(new_entry._node_id);
					
					double optimal_position = optimal_node_position(new_entry);
					new_entry._position = optimal_position;
					
					new_entry._range_id = *new_node_iter;
					
					/*
					 * @todo: Start search at the end of the path, should be
					 * faster!
					 */
					std::list<PathEntry>::iterator path_iter = path.begin();					 
					std::list<PathEntry>::iterator path_iter_end = path.end();
					
					for (; path_iter!=path_iter_end && 
						path_iter->_position<new_entry._position; ++path_iter);

					new_path_entries.push_back(
						path.insert(path_iter, new_entry)
					);

					insert_into_processed_nodes(new_entry._node_id,
						new_entry._position);

					++new_node_iter;
				} else
				{
					new_node_iter = cluster_nodes.erase(new_node_iter);
				}

			}

			/*
			 * If we did not find any nodes and the path is empty we set the
			 * distinct_position_m to some meters behind the current 
			 * scan_position_m. Otherwise we would never be able to create new
			 * roads.
			 * @todo This should be scan_position_m - x !!!
			 */
			if (!cluster_nodes.size() && !path.size())
			{
				distinct_position_m = scan_position_m;
			}


			/*
			 * If the path contains at least x nodes or the last node in the
			 * path is already more than x meters away we do some further check
			 * to decide what to do.
			 */						 
			if ((path.back()._position - path.front()._position) > 100.0)
			{				
				bool only_virtual_nodes = true;
				bool connected_real_nodes = true;
				double first_real_node_position_m = 0;
				double last_real_node_position_m = 0;
				PathEntry* middle_virtual_node = 0;
				double middle_position_m = path.back()._position - 40.0;
				
				std::list<PathEntry>::reverse_iterator 
					path_iter_end = path.rend();
				std::list<PathEntry>::reverse_iterator 
					path_iter = path.rbegin();
				
				PathEntry* previous_real_node = 0;
				while ((path_iter != path_iter_end) && 
					(path_iter->_position > (path.back()._position - 100.0) &&
					(only_virtual_nodes || connected_real_nodes)))
				{
					if (path_iter->_node_id == 0)
					{
						if ((path_iter->_position < middle_position_m) &&
							middle_virtual_node == 0)
							middle_virtual_node = &*path_iter;
						
					} else 
					{
						only_virtual_nodes = false;
						
						if (previous_real_node == 0)
						{
							previous_real_node = &*path_iter;
							first_real_node_position_m = path_iter->_position;
							last_real_node_position_m = path_iter->_position;
						} else if (connected_real_nodes && 
							(path_iter->_node_copy.
							is_reachable(previous_real_node->_node_id)))
						{
							previous_real_node = &*path_iter;
							last_real_node_position_m = path_iter->_position;
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
				} else if (/*connected_real_nodes && */
					((first_real_node_position_m - last_real_node_position_m) > 80.0)/* &&
					(first_real_node_position_m > (path.back()._position - 20.0))*/)
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
					 */
					std::list<PathEntry> finished_segment;
					
					distinct_position_m = ready_path.back()._position;
	
					/*
					 * We call the simplify algorithm to calculate the optimal
					 * nodes to use.
					 */
					simplify_path(previous_node_id, ready_path, finished_segment);
						
					/*
					 * This method merges the trace to the map as
					 * described by the segments. previous_node_id and
					 * completed_position_m are updates as needed.
					 */					
					use_segment(finished_segment, completed_position_m, 
						previous_node_id);
				}
			}

			/*
			 * If the path is empty we let the create_nodes method create nodes
			 * up to the distinct_position_m.
			 */
			create_nodes(completed_position_m, distinct_position_m, false,
				previous_node_id);

			scan_position_m += 10.0;
		}
			
//		delete _trace_log;

		mlog(MLog::info, "TraceProcessor") << "Finished (" << _id << ").\n";
		_tile_manager->trace_processor_finished(_id);
	}
	
	
	TileCache::Pointer
	TraceProcessor::tile(Node::Id node_id)
	{
		return _tile_cache->get(Node::tile_id(node_id));
	}
	
	
	void
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
				
				TileCache::Pointer tile = _tile_cache->get(Node::tile_id(segment_iter->_node_id));
				if (tile != 0)
				{
					tile.write().node(segment_iter->_node_id).set(
						merged_position.get_latitude(),
						merged_position.get_longitude(),
						merged_position.get_altitude()
					);
					bool result = 
						tile.write().move_node(segment_iter->_range_id, merged_node);
					if (!result)
						mlog(MLog::warning, "TraceProcessor::use_segment")
							<< "Could not move node!\n";
				}
			}
			
			if (previous_node_id != 0)
				connect_nodes(previous_node_id, used_node_id);
			previous_node_id = used_node_id;
			
			previous_segment_iter = segment_iter;
		}

	}


} // namespace mapgeneration
