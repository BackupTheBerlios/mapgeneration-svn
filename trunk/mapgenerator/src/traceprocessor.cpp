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
	
	
	double
	TraceProcessor::angle_difference(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(Node::tile_id(node_id_1));
		TileCache::Pointer tile_2_pointer = _tile_cache->get(Node::tile_id(node_id_2));		
		
		return (tile_1_pointer->node(node_id_1).
			minimal_direction_difference_to(tile_2_pointer->node(node_id_2))
			);
	}


	void
	TraceProcessor::calculate_cluster_nodes(GPSPoint gps_point,
		std::list<D_RangeReporting::Id>& result_list)
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
				mlog(MLog::error, "TraceProcessor::calculate_best_cluster_node") 
					<< "Created new tile " << (*needed_tile_ids_iter) 
					<< " (SHOULD NOT HAPPEN HERE!)\n";
			}
			
			std::vector<D_RangeReporting::Id> new_cluster_nodes_fast;
			tile->fast_cluster_nodes_search(gps_point, _search_radius_m,
				_search_max_angle_difference_pi * PI, new_cluster_nodes_fast);

/*			std::vector<D_RangeReporting::Id> new_cluster_nodes;
			tile->cluster_nodes_search(gps_point, _search_radius_m,
				_search_max_angle_difference_pi * PI, new_cluster_nodes);
				
			for (int i = 0; i < new_cluster_nodes.size(); ++i)
			{
				bool found = false;
				for (int j = 0; j < new_cluster_nodes_fast.size(); ++j)
				{
					if (*new_cluster_nodes[i] == *new_cluster_nodes_fast[j])
						found = true;
				}
				
				if ( !found )
				{
					Tile::Id tile_id;
					Node::LocalId node_local_id;
					Node::split_id(*new_cluster_nodes[i], tile_id, node_local_id);
					const Node& node = tile->node(*new_cluster_nodes[i]);
					double distance = gps_point.distance(node);
					
					std::cout << "ATTITION!!! Node: " << tile_id << ", " << node_local_id << " = "
						<< tile->node(*new_cluster_nodes[i]) << " not found!" << std::endl;
					std::cout << "ATTITION!!! GPSPoint: " << gps_point << ", distance = " << distance << std::endl;
				}
			}

			for (int i = 0; i < new_cluster_nodes_fast.size(); ++i)
			{
				bool found = false;
				for (int j = 0; j < new_cluster_nodes.size(); ++j)
				{
					if (*new_cluster_nodes[i] == *new_cluster_nodes_fast[j])
						found = true;
				}
				
				if ( !found )
				{
					Tile::Id tile_id;
					Node::LocalId node_local_id;
					Node::split_id(*new_cluster_nodes[i], tile_id, node_local_id);
					const Node& node = tile->node(*new_cluster_nodes[i]);
					double distance = gps_point.distance(node);
					
					std::cout << "ATTITION!!! Node: " << tile_id << ", " << node_local_id << " = "
						<< tile->node(*new_cluster_nodes[i]) << " too much!" << std::endl;
					std::cout << "ATTITION!!! GPSPoint: " << gps_point << ", distance = " << distance << std::endl;
				}
			}*/
			
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
				//mlog(MLog::debug, "TraceProcessor::create_needed_tiles") 
				//	<< "Created new tile " << (*tile_ids_iter) << " \n";
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
		
		std::pair<bool, Node::Id> new_node_id = tile.write().add_node(new_node);
//		_trace_log->new_node(new_node_id, new_node);
		
		return new_node_id.second;
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
			previous_node_id = new_node_id;
		}
		
	}
	
	
	double
	TraceProcessor::distance_from_to(Node::Id node_id_1, Node::Id node_id_2)
	{
		TileCache::Pointer tile_1_pointer = _tile_cache->get(Node::tile_id(node_id_1));
		TileCache::Pointer tile_2_pointer = _tile_cache->get(Node::tile_id(node_id_2));		
		
		return (tile_1_pointer->node(node_id_1).
			distance_default(tile_2_pointer->node(node_id_2))
			);
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
	
	
	void
	TraceProcessor::build_path_and_segments(std::list<PathEntry>& path,
		bool disconnect,
		std::list< std::list<PathEntry> >& finished_segments,
		PathEntry* start_entry)
	{
		/*
		 * This method expect a simplified path starting at start_entry 
		 * and splits up the contents of this path into segments and a new path.
		 */
		/*
		 * At first we have to clear the finished_segments list and create a 
		 * first segment. We may not clear the path here, as this contains
		 * the PathEntries we will walk through!
		 */
		finished_segments.clear();
		std::list<PathEntry> empty_segment;
		finished_segments.push_back(empty_segment);
		
		/*
		 * Then we walk through the optimal path, creating new segments, 
		 * whenever two nodes are not connected.
		 */
		std::list< std::list<PathEntry> >::iterator segment_iter =
			finished_segments.begin();				
		PathEntry* position = start_entry;
		PathEntry* previous_position = 0;
		while (position != 0)
		{			
			if ((previous_position != 0) && !connection_from_to(
				previous_position->_node_id, position->_node_id))
			{
				finished_segments.push_back(empty_segment);
				++segment_iter;
			}
			
			segment_iter->push_back(*position);
			
			previous_position = position;
			position = position->_connection;
		}
		
		/*
		 * Normally we check all segments for their length.
		 * After we have copied the PathEntries into the segments we may
		 * clear the path.
		 */
		/* @todo True?? */
		bool do_not_check_last_segment = false;
		/* @todo ????????????? */
		//if (disconnect) do_not_check_last_segment = true;
		path.clear();
		
		/*
		 * Normally the last PathEntries are copied back into the
		 * path and used in further optimizations. If we are optimizing, because
		 * the path ended several meters in front of the current position
		 * we can use all entries.
		 */
		if (!disconnect)
		{
			/*
			 * If the last segment contains less than x entries we copy the
			 * whole segment into the empty path and remove the segment.
			 * Else we copy the last x entries of the segment into the path and
			 * remove them from the last segment. We set 
			 * do_not_check_last_segment, because we already now that the last
			 * segment was long enough (and it may be too short now.
			 */
			if (segment_iter->size() < 6)
			{
				path.insert(path.end(), segment_iter->begin(), segment_iter->end());
				finished_segments.erase(segment_iter);
				do_not_check_last_segment = false;
			} else
			{
				std::list<PathEntry>::iterator move_start_iter = segment_iter->end();
				--move_start_iter;
				--move_start_iter;
				--move_start_iter;
				--move_start_iter;
				--move_start_iter;
				std::list<PathEntry>::iterator move_end_iter = segment_iter->end();
				path.insert(path.end(), move_start_iter, move_end_iter);
				segment_iter->erase(move_start_iter, move_end_iter);
				do_not_check_last_segment = true;
			}
		}
		
		/*
		 * This loop removes all segments that are shorter than x entries,
		 * ommiting the last segment if do_not_check_last_segment is true.
		 */
		segment_iter = finished_segments.begin();
		std::list< std::list<PathEntry> >::iterator segment_iter_end =
			finished_segments.end();			
		//Do not consider last segment, that was already checked above!
		if (do_not_check_last_segment && (segment_iter != segment_iter_end))
			--segment_iter_end;
		while (segment_iter != segment_iter_end)
		{
			if (segment_iter->size() < 5)
				segment_iter = finished_segments.erase(segment_iter);
			else
				++segment_iter;
		}
		
	}
	
	
	double
	TraceProcessor::build_connections(std::list<PathEntry>& path,
		std::list<PathEntry>::iterator path_iter, double previous_direction,
		bool disconnect, bool only_connected)
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
		
		/*
		 * The next loop searches all nodes that follow the current node
		 * in the path, if it is found (path_iter != path_iter_end) we recurse
		 * and calculate the points for the connection.
		 */
		TileCache::Pointer tile_pointer = _tile_cache->get(Node::tile_id(current_entry->_node_id));
		std::vector<Node::Id>::const_iterator next_nodes_iter = 
			tile_pointer->node(current_entry->_node_id).next_node_ids().begin();
		std::vector<Node::Id>::const_iterator next_nodes_iter_end =
			tile_pointer->node(current_entry->_node_id).next_node_ids().end();
		for (; next_nodes_iter != next_nodes_iter_end; ++next_nodes_iter)
		{
			std::list<PathEntry>::iterator path_iter = path.begin();
			std::list<PathEntry>::iterator path_iter_end = path.end();
			for (; (path_iter != path_iter_end) && 
				(path_iter->_node_id != *next_nodes_iter); ++path_iter);
			
			if (path_iter != path_iter_end)
			{				
				Node current_entry_node = _tile_cache->get(
					Node::tile_id(current_entry->_node_id))->node(current_entry->_node_id);
				Node next_entry_node = _tile_cache->get(
					Node::tile_id(path_iter->_node_id))->node(path_iter->_node_id);
				double direction_to_next = current_entry_node.bearing_default(next_entry_node);
				
				double points = build_connections(path, path_iter, 
					direction_to_next, disconnect, only_connected);
				
				// Negative points for:								
				// distance we want to walk from node to node
				points -= distance_from_to(
					current_entry->_node_id, path_iter->_node_id) * 0.85;
								
				//points -= angle_difference(current_entry->_node_id, path_iter->_node_id);
								
				if (points > current_entry->_points)
				{
					current_entry->_points = points;
					current_entry->_connection = &(*path_iter);
				}
			}
		}
		
		/*
		 * If we have to search not connected ways too, we do that. The point
		 * calculation is seperate from the above calculations because it will
		 * probably be very different.
		 */
		if (!only_connected)
		{
			++path_iter;
			std::list<PathEntry>::iterator path_iter_end = path.end();
			while ((path_iter != path_iter_end) && 
				(path_iter->_position < current_entry->_position + 200.0))	//WAS 50
			{
				bool connected =
					connection_from_to(current_entry->_node_id, path_iter->_node_id);
					
				if (!connected)
				{
					Node current_entry_node = _tile_cache->get(
						Node::tile_id(current_entry->_node_id))->node(current_entry->_node_id);
					Node next_entry_node = _tile_cache->get(
						Node::tile_id(path_iter->_node_id))->node(path_iter->_node_id);
					double direction_to_next = current_entry_node.bearing_default(next_entry_node);
					
					double points = build_connections(path, path_iter, 
						direction_to_next, disconnect, only_connected);
						
					// Negative points for:				
					// no connection
					points -= 10.0;
					
					// distance between point on trace and node in path
					GPSPoint point_on_trace = _filtered_trace.gps_point_at(path_iter->_position);
					Node node = _tile_cache->get(Node::tile_id(path_iter->_node_id))->
						node(path_iter->_node_id);
					points -= point_on_trace.distance_default(node) * 2;
					
					// distance between point on trace and current node
					point_on_trace = _filtered_trace.gps_point_at(current_entry->_position);
					node = _tile_cache->get(Node::tile_id(current_entry->_node_id))->
						node(current_entry->_node_id);
					points -= point_on_trace.distance_default(node) * 2;
					
					// direction difference between point and node
					points -= node.minimal_direction_difference_to(
						point_on_trace) * 5.0;
					
					// distance between current nodes position and connection
					//	position
					points -= (path_iter->_position - current_entry->_position);
					
					// complicated
					if (current_entry->_position < path_iter->_position-20.0)
					{
						Node current_entry_node = _tile_cache->get(
							Node::tile_id(current_entry->_node_id))->node(current_entry->_node_id);
						point_on_trace = _filtered_trace.gps_point_at(current_entry->_position + 10.0);
						double direction = current_entry_node.bearing_default(point_on_trace);
						double diff = current_entry_node.minimal_direction_difference_to(Direction(direction));
						points -= diff * 5.0;
					}
					
					if (previous_direction < 1000.0)
					{
						Node current_entry_node = _tile_cache->get(
							Node::tile_id(current_entry->_node_id))->node(current_entry->_node_id);
						Node next_entry_node = _tile_cache->get(
							Node::tile_id(path_iter->_node_id))->node(path_iter->_node_id);
						double dir_diff = current_entry_node.bearing_default(next_entry_node) - previous_direction;
						double edge_length = current_entry_node.distance_default(next_entry_node);
						double curvature_on_road = 	dir_diff / edge_length;
						double curvature_on_trace = _filtered_trace.curvature_at(current_entry->_position);
						double diff = curvature_on_road - curvature_on_trace;
						diff = (diff > 0 ? diff : -diff);
						points -= diff * 50.0;
					}
					
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
		}

		/*
		 * Return the optimal number of points we could reach in this path.
		 */
		return current_entry->_points;
	}
	
	
	void
	TraceProcessor::simplify_path(Node::Id start_node_id, 
		bool disconnect,
		std::list<PathEntry>& path, 
		std::list< std::list<PathEntry> >& finished_segments)
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
		int method = 0;

		/*
		 * If method 0 (only connected) does not work we need a second
		 * run searching all possible paths.
		 */
		while (best_start_entry == 0 && method < 2)
		{
			/*
			 * We just have to visit each path entry once, we use time stamps
			 * control this and increase the time before each search.
			 */
			++_time;
			
			/*
			 * Make the meaning of method more clear.
			 */
			bool only_connected = (method == 0 ? true : false);
			
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
				path.insert(path.begin(), start_entry);
							
				best_points = build_connections(path, path.begin(), 
					1001.0, disconnect, only_connected);
				best_start_entry = path.front()._connection;
			} else
			{
				double start_position = path.front()._position;
				std::list<PathEntry>::iterator path_iter = path.begin();
				std::list<PathEntry>::iterator path_iter_end = path.end();
				while ((path_iter != path_iter_end) && 
					(path_iter->_position < start_position + 50.0))
				{
					double points = build_connections(path, path_iter, 
						1001.0, disconnect, only_connected);
					points -= path_iter->_position - start_position;
					
					if (points > best_points)
					{
						best_points = points;
						best_start_entry = &(*path_iter);
					}
					
					++path_iter;
					
				}
			}
			
			++method;
			
		}
		
		/*
		 * We build_path_and_segments if we found a result, else we just clear
		 * the path.
		 * @todo Is this still current?
		 */
		if ((best_start_entry != 0) && (best_points > -10000.0))
		{
			build_path_and_segments(path, disconnect,
				finished_segments, best_start_entry);
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
			tracelog_filename.str(), _filtered_trace, _service_list);

		_filtered_trace.calculate_directions();
		_filtered_trace.precompute_data();
		
		mlog(MLog::debug, "TraceProcessor") << "Processes a " << 
			_filtered_trace.length_m() << "m long trace.\n";
		
		pubsub::Service<double>* service
			= _service_list->find_service<double>("statistics.received_meters");
		
		if (service)
			service->receive(_filtered_trace.length_m());
		
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
		
		/*
		 * The main loop. The trace is processed as long as the end is not
		 * reached.
		 */
		while (scan_position_m < _filtered_trace.length_m())
		{
			/*
			 * At first all nodes in a certain radius around the current 
			 * position are searched.
			 */
			std::list<D_RangeReporting::Id> cluster_nodes;
			calculate_cluster_nodes(
				_filtered_trace.gps_point_at(scan_position_m),
				cluster_nodes
			);
			
			/*
			 * Each node from the search result is searched in the pass 
			 * to avoid double entries.
			 */
			new_path_entries.clear();
			std::list<D_RangeReporting::Id>::iterator new_node_iter
				= cluster_nodes.begin();
			while (new_node_iter != cluster_nodes.end())
			{
				/*
				 * This loop checks the last x nodes from the path.
				 */
				PathEntry new_entry(scan_position_m, **new_node_iter);
				std::list<PathEntry>::iterator path_iter = path.end();
				std::list<PathEntry>::iterator path_iter_begin = path.begin();

				bool insert = true;
				/*
				 * @todo: This would be more accurate if we would use meters!
				 */
				int checked_nodes = 0;
				for (; (insert == true) && (checked_nodes < 8) && 
					(path_iter != path_iter_begin); --path_iter, ++checked_nodes)
				{
					if (*path_iter == new_entry)
					{
						insert = false;
						new_node_iter = cluster_nodes.erase(new_node_iter);
					}
				}
				
				/*
				 * If we hit the beginning of the path we have to check this
				 * entry too.
				 */
				if ((insert == true) && (checked_nodes < 8) && (*path_iter == new_entry))
				{
					insert = false;
					new_node_iter = cluster_nodes.erase(new_node_iter);
				}

				/**
				 * If insert is still true we can insert the entry into the 
				 * path.
				 */
				if (insert)
				{
					double optimal_position = optimal_node_position(new_entry);
					new_entry._position = optimal_position;
					
					path_iter = path.begin();
					for (; path_iter!=path.end() && 
						path_iter->_position<new_entry._position; ++path_iter);

					new_path_entries.push_back(
						path.insert(path_iter, new_entry)
					);

					++new_node_iter;
				}
				
				/*
				 *  @todo Implement a more robust system to capture already
				 * processed nodes.
				 */

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
			bool disconnect = false;
			if (path.size() && (path.back()._position < scan_position_m-30.0))
				disconnect = true;
			if (path.size() > 5 || disconnect)
			{	
				int connected_nodes = 0;
				
				/*
				 * If the path contains at least x nodes, we count the nodes 
				 * that are connected in a row at the end of the path.
				 */
				if (path.size() > 5)
				{
					std::list<PathEntry>::iterator path_iter = path.end();
					--path_iter;
															
					Node::Id next_node_id = path_iter->_node_id;
					--path_iter;
					while ((path_iter != path.begin()) && (connected_nodes < 21))
					{
						if (connection_from_to(path_iter->_node_id, next_node_id))
							++connected_nodes;
						
						next_node_id = path_iter->_node_id;
						--path_iter;
					}
					
					/* 
					 * If we found the number of nodes needed to assume that
					 * we are surely on this road, we set connected_nodes_row
					 * to indicate this fact.
					 */
					if (connected_nodes > 5)
						connected_nodes_row = true;
				}
				
				/*
				 * Now we decide if we want to process the path. We process the
				 * path if
				 * - we have found a row of more than x nodes and can be really
				 * sure to be on that road
				 * - we have found a connected_nodes_row (see above), but did
				 * not find it in the current iteration, meaning that there was
				 * a node from another road in the last search
				 * - we have left the path for more than x meters.
				 */
				if ((connected_nodes > 20) ||
					(connected_nodes_row && (connected_nodes < 6)) ||
					disconnect)
				{
					/*
					 * If the second case occured we take back the last search
					 * step. This way we have a connected_nodes_row at the end
					 * of the path.
					 * In the third case we create a new node and insert it into
					 * the path. This node will be used as the destination point
					 * for the path search algorithm.
					 */
					Node::Id new_node_id;
					if (connected_nodes < 6)
					{
						// Take back the last scan step.
						std::vector< std::list<PathEntry>::iterator >::iterator new_entries_iter = new_path_entries.begin();
						for (; new_entries_iter != new_path_entries.end(); ++new_entries_iter)
							path.erase(*new_entries_iter);
						walk_on = false;
					} else if (disconnect)
					{
						// Insert a destination point.
						GPSPoint new_node_position = _filtered_trace.
							gps_point_at(scan_position_m);
						new_node_id = create_new_node(new_node_position);
						path.push_back(PathEntry(scan_position_m, new_node_id));
					//	std::cout << "Inserted destination point.\n";
					}

					/*
					 * This list will contain the segments calculated by
					 * simplify path.
					 */
					std::list< std::list<PathEntry> > finished_segments;
										
					/*
					 * We call the simplify algorithm to calculate the optimal
					 * nodes to use.
					 */
					simplify_path(previous_node_id, disconnect, path, finished_segments);
					
					/*
					 * If the algorithm has returned any segments we can set the
					 * distinct_position_m to the position of the last node in
					 * this results, else we use the current scan_position_m.
					 */
					if (finished_segments.size())
						distinct_position_m = finished_segments.back().back()._position;
					else
						distinct_position_m = scan_position_m;
						
					if (disconnect)
					{						
						/* @todo Delete node with new_node_id.*/
					}

					/*
					 * This method merges the trace to the map as
					 * described by the segments. previous_node_id and
					 * completed_position_m are updates as needed.
					 */					
					use_segments(finished_segments, /*last_used_node_ids,*/ completed_position_m, previous_node_id);
					
					/*
					 * After processing there is no row of connected nodes in
					 * the path.
					 */
					connected_nodes_row = false;
				}
			}

			/*
			 * If the path is empty we let the create_nodes method create nodes
			 * up to the distinct_position_m.
			 */
			if (!path.size())
				create_nodes(completed_position_m, distinct_position_m, false,
					previous_node_id);

			/*
			 * Without walking on we would stay right where we are. ;-)
			 */
			if (walk_on)
				scan_position_m += 10.0;
			else
				walk_on = true;
		}
			
		delete _trace_log;

		mlog(MLog::info, "TraceProcessor") << "Finished (" << _id << ").\n";
		_tile_manager->trace_processor_finished(_id);
	}
	
	
	TileCache::Pointer
	TraceProcessor::tile(Node::Id node_id)
	{
		return _tile_cache->get(Node::tile_id(node_id));
	}
	
	
	void
	TraceProcessor::use_segments(std::list< std::list<PathEntry> >& finished_segments, 
		double& completed_position_m, Node::Id& previous_node_id)
	{
		/*
		 * This method gets a list of segments. Each segment contains a list of
		 * PathEntries that reference nodes. The nodes referenced by the 
		 * PathEntries in one segment are connected and should be used in the
		 * order of their appearance. The segments should also be used
		 * in order, but they (exactly: the last node of 
		 * one segment and the first node of the next segment) are not
		 * already connected and the distance between them might require new
		 * nodes to be created.
		 * After this method finished:
		 * - all nodes inside the segments are merged with the trace.
		 * - new nodes are created to fill the gaps before the segments.
		 * - the completed_position_m is at the end of the last segment.
		 */
		/*
		 * We start with two layers of iterators, the first for the segments,
		 * the second for the path entries in the segments.
		 */
		std::list< std::list<PathEntry> >::iterator finished_segments_iter =
			finished_segments.begin();
		std::list< std::list<PathEntry> >::iterator finished_segments_iter_end =
			finished_segments.end();		

		for (; finished_segments_iter != finished_segments_iter_end;
			++finished_segments_iter)
		{
			std::list<PathEntry>::iterator segment_iter =
				finished_segments_iter->begin();
			std::list<PathEntry>::iterator segment_iter_end =
				finished_segments_iter->end();
					
			/*
			 * The method create_nodes creates nodes from the 
			 * completed_position_m up to the first node of the current segment
			 * (segment_iter->_position), not actually creating the node at
			 * this position (false). The previous_node_id is updated.
			 * After this step the step from the previous_node_id to the
			 * first node of the segment has a correct length.
			 */		
			create_nodes(completed_position_m, segment_iter->_position, false,
				previous_node_id);
			
			/*
			 * Now we can process the segment, connecting nodes as needed (see
			 * below) and merging the trace and nodes (in the near future).
			 */
			for (; segment_iter != segment_iter_end; ++segment_iter)
			{
				completed_position_m = segment_iter->_position;
				Node::Id used_node_id = segment_iter->_node_id;
				/*
				 * We could and should actually just connect the last node
				 * before the segment and the first node of the segment, but the
				 * double connection of the nodes inside the segment is captured
				 * inside connect_nodes.
				 */
				if (previous_node_id != 0)
					connect_nodes(previous_node_id, used_node_id);
				previous_node_id = used_node_id;
			}
		
		}
	}


} // namespace mapgeneration

