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

	TraceProcessor::TraceProcessor(unsigned int id, TileManager* tile_manager, FilteredTrace& filtered_trace)
	: _filtered_trace(filtered_trace), _id(id), _tile_manager(tile_manager),
		_trace_log(0)
	{
		_edge_cache = _tile_manager->get_edge_cache();
		_tile_cache = _tile_manager->get_tile_cache();
		
		/* Some variables we will need. */
		_best_cluster_node = 0;
		_current_edge = 0;
		_current_edge_node_ids_iter = 0;
		_current_edge_times_iter = 0;
		_current_node = 0;
		_current_node_id;
		_current_node_before_merge;
		_do_not_change_current_edge = false;
		_end_main_loop = false;
		_last_clustering_was_at_crossing = false;
		_previous_node = 0;
		_previous_node_id;
		_trace_processing_is_at_a_beginning_state = true;
		_unused_gps_point = 0;
		
		mlog(MLog::debug, "TraceProcessor") << "Initialised (" << _id << ").\n";		
	}


	void
	TraceProcessor::calculate_best_cluster_node()
	{
		_best_cluster_node = 0;
		
		std::vector< std::pair<unsigned int, unsigned int> > cluster_nodes;	// < All nodes that will be clustered
		
		/* Search best clusterable node. */
		std::vector<unsigned int> needed_tile_ids  = _filtered_trace.front().get_needed_tile_ids(_SEARCH_RADIUS * 2);
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
				tile->cluster_nodes_search(_filtered_trace.front(), _SEARCH_RADIUS, _ANGLE_DIFFERENCE);
			cluster_nodes.insert(cluster_nodes.end(), new_cluster_nodes.begin(), new_cluster_nodes.end());

			#ifdef TRACELOGS
				/* Draw the tile to the log. */
				//_old_trace_log.draw_tile(*tile);
				/* NUR FUER DIE PRAESENTATION !!! */
				_old_trace_log.draw_tile_with_edges(*tile, _tile_manager->get_tile_edges_nodes(tile->get_id()));
			#endif
		}
		
		/* Calculate best cluster point. */
		/** @todo Should we also consider the direction for calculate the
		 * best cluster node? */
		if (cluster_nodes.size()) {
			bool found_node = false;
			_best_cluster_node_id.first = 0;
			_best_cluster_node_id.second = 0;
			double best_distance = 1000000;
			std::vector< std::pair<unsigned int, unsigned int> >::iterator cluster_nodes_iter = cluster_nodes.begin();
			std::vector< std::pair<unsigned int, unsigned int> >::iterator cluster_nodes_iter_end = cluster_nodes.end();
			for (; cluster_nodes_iter != cluster_nodes_iter_end; ++cluster_nodes_iter)
			{
				const Node& node= _tile_cache->get(cluster_nodes_iter->first)->nodes()[cluster_nodes_iter->second].second;
//				if ((_current_edge == 0) || !node.is_on_edge_id(_current_edge->get_id()))
//				{
					double distance = node.distance(_filtered_trace.front());
					if (distance < best_distance)
					{
						best_distance = distance;
						_best_cluster_node_id = *cluster_nodes_iter;
						found_node = true;
					}
//				}
			}
			
			/** @todo: Do we really need a write access? */
			if (found_node)
				_best_cluster_node = &_tile_cache->get(_best_cluster_node_id.first)
					.write().nodes()[_best_cluster_node_id.second].second;
			else
				_best_cluster_node = 0;

		}
	}
	
	
	void
	TraceProcessor::connect_edges()
	{
		assert(_current_edge != 0);
		assert(_previous_node != 0);
		
		unsigned int edge_1_id = _previous_node->edge_ids().front();
		unsigned int edge_2_id = _current_edge->get_id();
		
		unsigned int new_edge_id = _tile_manager->request_edge_connect(edge_1_id, edge_2_id);
		EdgeCache::Pointer new_edge_pointer = _edge_cache->get(new_edge_id);
		set_current_edge(new_edge_pointer);
		
		_trace_log->connect_edges(edge_1_id, edge_2_id, *new_edge_pointer);		
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


	void
	TraceProcessor::create_new_edge(bool start_at_previous, bool end_at_current)
	{
		std::pair<unsigned int, unsigned int> first_node_id;
		std::pair<unsigned int, unsigned int> second_node_id;
		
		if (start_at_previous)
		{
			first_node_id = _previous_node_id;
		} else
		{
			assert(_unused_gps_point != 0);

			first_node_id = create_new_node(*_unused_gps_point);
			
			/* that calculation is ALMOST equivalent but the variant below the
			 * following is more precise */
			// time = _filtered_trace.front().get_time() - _unused_gps_point->get_time();
		}
		
		if (end_at_current)
		{
			second_node_id = _current_node_id;
		} else
		{
			second_node_id = create_new_node(_filtered_trace.front());
		}
		
		double time = _filtered_trace.get_time_from_previous_start();
		unsigned int new_edge_id = _tile_manager->create_new_edge(first_node_id, second_node_id, time);

		/** @todo: Do we really need a write access? */
		_current_node = &_tile_cache->get(second_node_id.first)
			.write().nodes()[second_node_id.second].second;
		_current_node_id = second_node_id;
		delete _unused_gps_point;
		_unused_gps_point = 0;
		
		EdgeCache::Pointer new_edge_pointer = 
			_edge_cache->get(new_edge_id);
		
		set_current_edge(new_edge_pointer);
		
		_trace_log->new_edge(*new_edge_pointer);
		
		#ifdef TRACELOGS
			Node& first_node = _tile_cache->get(first_node_id.first)->nodes()[first_node_id.second].second;
			Node& second_node = _tile_cache->get(second_node_id.first)->nodes()[second_node_id.second].second;
			_old_trace_log.draw_circle(first_node, 8.0);
			_old_trace_log.draw_circle(second_node, 8.0);
			_old_trace_log.next_step();
		#endif
		
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
		
//		_trace_log->add_tile(
		_trace_log->new_node(new_node_id, new_node);
		
		return new_node_id;
	}
	
	
	void
	TraceProcessor::extend_edge(
		unsigned int edge_id,
		std::pair<unsigned int, unsigned int>& next_to_node_id,
		std::pair<unsigned int, unsigned int>& new_node_id
	)
	{
		double time = _filtered_trace.get_time_from_previous_start();
		_tile_manager->request_edge_extend(edge_id, next_to_node_id, new_node_id, time);
		
		_trace_log->extend_edge(edge_id, next_to_node_id, new_node_id, time);

/*		_current_edge_node_ids_iter = _current_edge.write().node_ids().end();
		--_current_edge_node_ids_iter;
		_current_edge_times_iter = _current_edge.write().times().end();
		--_current_edge_times_iter;*/
	}
	
	
	void
	TraceProcessor::merge_node_and_gps_point(bool first_point)
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
		std::vector<unsigned int>::iterator current_node_edge_ids_iter = _current_node->edge_ids().begin();
		std::vector<unsigned int>::iterator current_node_edge_ids_iter_end = _current_node->edge_ids().end();
		for (; current_node_edge_ids_iter != current_node_edge_ids_iter_end; ++current_node_edge_ids_iter)
		{
			set_current_edge(_edge_cache->get(*current_node_edge_ids_iter));
			
			if (!_current_edge->node_is_at_start(_current_node_id))
			{
				/* time correction for edge before _current_node */
				--_current_edge_node_ids_iter;
				Node& previous_node = _tile_cache->get(_current_edge_node_ids_iter->first)
					.write().nodes()[_current_edge_node_ids_iter->second].second;
				++_current_edge_node_ids_iter;

				--_current_edge_times_iter;
				double time_before_merge = *_current_edge_times_iter;
				double old_distance = previous_node.distance(_current_node_before_merge);
				double new_distance = previous_node.distance(*_current_node);
				double time_after_merge = time_before_merge * (new_distance / old_distance);
				double weight = _current_node->get_weight() - 1; /** first approximation! */
				
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
				/* time correction for edge after _current_node */
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

		if (_current_node->edge_ids().size() > 1)
		{
			_last_clustering_was_at_crossing = true;
			set_current_edge(0);
		}
		
		_trace_log->merge_node(_best_cluster_node_id, saved_gps_point_for_logging,
			*_best_cluster_node);
	}
	
	
	inline bool
	TraceProcessor::mergeable()
	{
		/** @todo Only called once. So do we need a method?! */
		
		Node* work_node = 0;
		
		/* if no _current_node exists
		 * => last step was extend_edge (or this method won't be called!!!)
		 * => calculate_best_cluster_node! */
		if (_current_node == 0)
		{
			calculate_best_cluster_node();
			if (_best_cluster_node == 0)
				return false;
			else
				work_node = &(*_best_cluster_node);
		} else
		{
			work_node = &(*_current_node);
			
			_best_cluster_node = _current_node;
			_best_cluster_node_id = _current_node_id;
		}
		
		double distance = work_node->distance(_filtered_trace.front());
		double angle_difference = work_node->angle_difference(_filtered_trace.front());
		return ((distance <= _SEARCH_RADIUS) && (angle_difference <= _ANGLE_DIFFERENCE));
	}
	
	
	bool
	TraceProcessor::on_the_same_edge()
	{
		/* This method calculates if _previous_node and _current_node are on the
		 * same edge. It was outsourced because it is used twice. */
		std::vector<unsigned int>::iterator previous_node_edge_ids_iter
			= _previous_node->edge_ids().begin();
		std::vector<unsigned int>::iterator previous_node_edge_ids_iter_end
			= _previous_node->edge_ids().end();
		for (; previous_node_edge_ids_iter != previous_node_edge_ids_iter_end; ++previous_node_edge_ids_iter)
		{
			if (_current_node->is_on_edge_id(*previous_node_edge_ids_iter))
			{
				return true;
			}
		}
		
		return false;
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
	
	
	void
	TraceProcessor::recalculate_current_edge_iters()
	{
		/** @todo Think about a better system for the status variables.
		 * Especially these iters. */
		assert(_current_edge != 0);
		assert(_current_node != 0);
//		std::cout << "recalc...start; ";
		
		_current_edge_node_ids_iter = _current_edge.write().node_ids().begin();
		_current_edge_times_iter = _current_edge.write().times().begin();
		while ((_current_edge_node_ids_iter->first != _current_node_id.first) ||
			(_current_edge_node_ids_iter->second != _current_node_id.second))
		{
			++_current_edge_node_ids_iter;
			++_current_edge_times_iter;
		}
//		std::cout << "recalc...end; ";
	}


	void
	TraceProcessor::run()
	{
		mlog(MLog::info, "TraceProcessor") << "Started (" << _id << ").\n";
		
		#ifdef TRACELOGS
			/* Ids of the last draw_edge and draw_trace orders. */
			int draw_edge_id = 0;
			int draw_trace_id = 0;
			int draw_tile_id = 0;
		#endif
		
		create_needed_tiles();
		
		std::ostringstream tracelog_filename;
		tracelog_filename << "tracelog" << _id;
		tracelog_filename.flush();
		_trace_log = new TraceLogWriter(_tile_manager, 
			tracelog_filename.str(), _filtered_trace);

		#ifdef TRACELOGS
			/* Draw the initial trace. */
			draw_trace_id = _old_trace_log.draw_filtered_trace(_filtered_trace);
			_old_trace_log.next_step();
		#endif

		/* Filter the trace. */
		_filtered_trace.filter();
		_filtered_trace.calculate_directions();
		//_filtered_trace.calculate_times();
		
		#ifdef TRACELOGS
			/* Draw the new trace. */
			_old_trace_log.remove_order(draw_trace_id);
			draw_trace_id = _old_trace_log.draw_filtered_trace(_filtered_trace);
			_old_trace_log.next_step();
		#endif

		while (!_end_main_loop)
		{
			/*
			 * The cases handled below:
			 * ========================
			 * 
			 * Legend:
			 * -------
			 * C = Crossing (may be drawn uncompletely!)
			 * U = Unused gps point is located here
			 * 
			 * ----> = Existing edges
			 * ....> = actual situation of filtered trace
			 * |   |
			 * |   actual position
			 * last position
			 * 
			 * (1)            /\
			 *                |
			 *                |
			 *    ----------->C
			 *                ........>
			 *    
			 *    Starts at a crossing and no best cluster node.
			 * 
			 * 
			 * (2)            /\  |
			 *                |   |
			 *                |   \/
			 *    ----------->C   C-------->
			 *                ....>
			 *    
			 *    Starts at one crossing and ends at another.
			 * 
			 * 
			 * (3)            /\
			 *                |
			 *                |
			 *    ----------->C   -------->
			 *                ....>
			 *    
			 *    Starts at one crossing and ends at beginning of an edge.
			 * 
			 * 
			 * (4)            /\  |
			 *                |   |
			 *                |   \
			 *    ----------->C    \-------->
			 *                ......>
			 *    
			 *    Starts at one crossing and ends somewhere in the middle of an edge.
			 * 
			 * 
			 * (5)
			 *    .>
			 *    
			 *    Beginning state. Will set unused gps point.
			 * 
			 * 
			 * (6)
			 *    U
			 *    ....>
			 *    
			 *    Beginning state with unused gps point set. Creates new edge.
			 * 
			 * 
			 * (7)
			 *  a)                           b)
			 *    -------------->              U   ---------->
			 *    .>                           .....>
			 *    
			 *    Beginning state with best cluster node. Just merge. (See comment!)
			 * 
			 * 
			 * (8)               |
			 *                   |
			 *                   \/
			 *    -------------->C-------->
			 *               ....>
			 *    
			 *    Start somewhere (exactly: last node before end) of an edge and
			 *    ends at a crossing where that edge also ends. Nothing to do.
			 * 
			 * 
			 * (9)               |
			 *                   |
			 *                   \/
			 *    ----------->   C-------->
			 *               ....>
			 *    
			 *    Start at the end of an edge and ends at a crossing. Extend edge.
			 * 
			 * 
			 * (10)           /\ |
			 *                |  |
			 *                /  \/
			 *    -----------/   C-------->
			 *               ....>
			 *    
			 *    Start somewhere of an edge and ends at a crossing. Split and create.
			 * 
			 * 
			 * (11)
			 *    -------------->
			 *        ....>
			 *    
			 *    Starts and end on same edge. Nothing to do.
			 * 
			 * 
			 * (12)
			 *    -------------->
			 *        ..>.
			 *            .
			 *        <...
			 *    <--------------
			 *    
			 *    Situation enlarged! Edges on same street.
			 *    U-turn. Nothing to do.
			 * 
			 * 
			 * (13)
			 *    -------------->   ------------>
			 *                  ....>
			 *    
			 *    Starts at end of an edge and ends at beginning of another edge.
			 *    Connect.
			 * 
			 * 
			 * (14)              |
			 *                   |
			 *                   \
			 *    --------------> \------->
			 *                 ...>
			 *    
			 *    Starts at end of an edge and ends at somewhere of another edge.
			 *    Extend and split.
			 * 
			 * 
			 * (15)           /\
			 *                |
			 *                /
			 *    -----------/   -------->
			 *               ....>
			 *    
			 *    Start somewhere of an edge and ends at beginning of another edge.
			 *    Split and extend.
			 * 
			 * 
			 * (16)           /\ |
			 *                |  |
			 *                /  \
			 *    -----------/    \-------->
			 *               .....>
			 *    
			 *    Start somewhere of an edge and ends somewhere of another edge.
			 *    2x split and create.
			 * 
			 * 
			 * (17)
			 *    -------------->
			 *        ..>.
			 *            .
			 *        <...
			 *    
			 *    Situation enlarged!
			 *    U-turn. Will set unused gps point. Now at beginning state.
			 * 
			 * 
			 * (18)
			 *    -------------->
			 *                  ....>
			 *    
			 *    Starts at end of an edge and ends in the middle of nowhere. Extend.
			 * 
			 * 
			 * (19)           /\
			 *                |
			 *                /
			 *    -----------/
			 *               ....>
			 *    
			 *    Start somewhere of an edge and ends in the middle of nowhere.
			 *    Split and create.
			 */

//			std::cout << "Status: ";
			if (_last_clustering_was_at_crossing)
			{
				/* Last clustering took place at a crossing, so no _current_edge exits.
				 * We will calculate the best cluster node and then we will see...
				 * See the TODOs concerning set_current_edge(0) or look below. */
//				std::cout << "_last_clustering_was_at_crossing; ";
				calculate_best_cluster_node();
	
				if (_best_cluster_node == 0)
				{
					/* CASE 1:
					 * We did not find a best cluster node, so we have to create a new
					 * edge which will start at the _previous_node (the crossing!). 
					 * 
					 * And we know that there was no clustering at a crossing, so we set
					 * the corresponding attribute to false. */
//					std::cout << "_best_cluster_node = 0; ";
					create_new_edge(true, false);
					_last_clustering_was_at_crossing = false;
				} else // explicit: if (_best_cluster_node != 0)
				{
					/* We found a best cluster node, so cluster them! */
//					std::cout << "_best_cluster_node != 0; ";
					merge_node_and_gps_point(false);
					
					/* Now we have to calculate if _previous_node and _current_node
					 * are on a same edge. Consequences? See below! */
					bool on_same_edge = on_the_same_edge();
					
					if (on_same_edge)
					{
						/* We know that _previous_node and _current_node are on a same
						 * edge so no extra work is needed. We merged the points (see
						 * above) and that's it! */
//						std::cout << "on_same_edge; ";
					} else //explicit: if (!on_same_edge)
					{
						/* We know that _previous_node and _current_node have no edge in
						 * common. We have to test if the very last merge took place at a
						 * crossing. */
//						std::cout << "!on_same_edge; ";
						if (_last_clustering_was_at_crossing)
						{
							/* CASE 2:
							 *  The very last clustering was at a crossing. Because no edge
							 * connects _previous_node and _current_node (we tested that
							 * above) we will create a new edge. And we will not change
							 * _last_clustering_was_at_crossing because now we are again at
							 * a crossing!
							 * The encapsulation of create_new_edge with the 
							 * _do_not_change_current_edge attributes is done only (!) for 
							 * speed reasons. */
//							std::cout << "_last_clustering_was_at_crossing; ";
							_do_not_change_current_edge = true;
							create_new_edge(true, true);
							_do_not_change_current_edge = false;
						} else // explicit: if (!_last_clustering_was_at_crossing)
						{
							/* The very last clustering was NOT at a crossing. But we ran
							 * on an existing edge (now named _current_edge). So we will now
							 * test, where we entered that edge. */
//							std::cout << "!_last_clustering_was_at_crossing; ";
							if (_current_edge->node_is_at_start(_current_node_id))
							{
								/* CASE 3:
								 *  We entered _current_edge at the beginning, so we will extend
								 * that edge using the corresponding method of the TileManager. */
//								std::cout << "_current_edge->node_at_start(_current_node_id); ";
								extend_edge(_current_edge->get_id(), _current_node_id,
									_previous_node_id);
/*								_tile_manager->request_edge_extend(
									_current_edge->get_id(),
									_current_node_id,
									_previous_node_id,
									_filtered_trace.get_time_from_previous_start()
								);*/
							} else // explicit: if (!_current_edge->node_is_at_start(_current_node))
							{
								/* CASE 4:
								 *  We entered _current_edge somewhere after the beginning.
								 * A split of _current_edge is needed as well as to create a
								 * new edge between _previoud_node and _current_node.
								 * 
								 * We do not need to test if we are at the end of _current_edge
								 * (we will not need a split there!) because the method of the
								 * TileManager is (hopefully :-), and well it is!) smart enough
								 * to recognize that situation.
								 * 
								 * As said above the encapsulation of create_new_edge with the 
								 * _do_not_change_current_edge attributes is done only (!) for 
								 * speed reasons.
								 * 
								 * But then we have to set _last_clustering_was_at_a_crossing
								 * to true because we are at a kind of t-crossing.
								 * 
								 * set_current_edge(0) is needed for the decision if we have to
								 * walk on egde AND trace or on trace only. */
								/** @todo Think about a better system to decide when to walk
								 *  where. Do not rely on on _current_edge! */
//								std::cout << "!_current_edge->node_at_start(_current_node_id); ";
								#ifdef TRACELOGS
									Node& second_node = _tile_cache->get(_current_node_id.first)
										->nodes()[_current_node_id.second].second;
									_old_trace_log.draw_circle(second_node, 10.0);
									_old_trace_log.next_step();
								#endif
								
								split_edge(_current_edge->get_id(), _current_node_id);
/*								_tile_manager->request_edge_split(_current_edge->get_id(), _current_node_id);*/
								
								_do_not_change_current_edge = true;
								create_new_edge(true, true);
								_do_not_change_current_edge = false;

								_last_clustering_was_at_crossing = true;
								set_current_edge(0);
								
							} // end: if (_current_edge->node_is_at_start(_current_node))
						} // end: if (on_same_edge)
					} // end: if (_last_clustering_was_at_crossing)
				} // end: if (_best_cluster_node == 0)
					
			} else // explicit: if (!_last_clustering_was_at_crossing)
			{
				/* At the end of last loop _last_clustering_was_at_crossing was false.
				 * We have to destinguish if the trace processing is at a beginning
				 * state or not.
				 * A beginning state occurs
				 * 1st: at the very first beginning of the FilteredTrace.
				 * 2nd: after an u-turn
				 *		(Following conditions matches either:
				 * 		 - _previous_node->angle_difference(*_current_node)
				 * 		   > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES) )
				 * 		 - _previous_node->angle_difference(_filtered_trace.front())
				 * 		   > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES) ) */
//				std::cout << "!_last_clustering_was_at_crossing; ";
				if (_trace_processing_is_at_a_beginning_state)
				{
					/* So we are at a beginning state.
					 * We calculate the best cluster node and then we will see... */
//					std::cout << "_trace_processing_is_at_a_beginning_state; ";

					#ifdef TRACELOGS
//						_old_trace_log.remove_order(draw_trace_id);
						draw_trace_id = _old_trace_log.draw_circle(_filtered_trace.front(), 10.0);
						_old_trace_log.next_step();
					#endif
					
					calculate_best_cluster_node();
					if (_best_cluster_node == 0)
					{
						/* A best cluster node was not found. Now the behaviour depends
						 * on the state of _unused_gps_point. */
//						std::cout << "_best_cluster_node = 0; ";
						if (_unused_gps_point == 0)
						{
							/* CASE 5:
							 *  _unused_gps_point unset. So we will save the front of
							 * _filtered_trace in _unused_gps_point. And will see next loop
							 * if we need the _unused_gps_point. */
//							std::cout << "_unused_gps_point = 0; ";
							_unused_gps_point = new GPSPoint(_filtered_trace.front());
						} else // explicit: if (_unused_gps_point != 0)
						{
							/* CASE 6:
							 *  _unused_gps_point set. We will create a new edge from the
							 * _unused_gps_point to the front of _filtered_trace and states
							 * that we are no more at a beginning state. */
//							std::cout << "_unused_gps_point != 0; ";
							create_new_edge(false, false);
							delete _unused_gps_point;
							_unused_gps_point = 0;
							_trace_processing_is_at_a_beginning_state = false;
						} // end: if (_unused_gps_point == 0)
						
					} else // explicit: if (_best_cluster_node != 0)
					{
						/* CASE 7:
						 *  We found a best cluster node. So we will merge points without
						 * considering _unused_points_point at all. We will simple
						 * forget it. And trace processing is not at a beginning state
						 * any longer. */
//						std::cout << "_best_cluster_node != 0; ";
						merge_node_and_gps_point(true);
						_trace_processing_is_at_a_beginning_state = false;
					} // end: if (_best_cluster_node == 0)
					
				} else // explicit: if (!_trace_processing_is_at_beginning)
				{
					/* Trace processing is not at a beginning state, so we have to check
					 * if there is a mergeable node. */
//					std::cout << "!_trace_processing_is_at_a_beginning_state; ";
					if (mergeable())
					{
						/* We found a mergeable node (which is saved to
						 * _best_cluster_node!). We will merge points and then we have to
						 * consider the place of merge: Crossing or not?
						 * Besides we save the ID of _current_edge. We will need it. */
//						std::cout << "mergeable(); ";
						unsigned int edge_id_before_merge = _current_edge->get_id();
						merge_node_and_gps_point(false);
	
						/* Now the following statement are valid:
						 * _current_node = _best_cluster_node;
						 * _current_node_id = _best_cluster_node_id;
						 * if (_current_node->edge_ids().size() > 1)
						 *   _last_clustering_was_at_crossing = true;
						 * else
						 * {
						 *   _last_clustering_was_at_crossing = false;
						 * 	 set_current_edge(_edge_cache->get(_current_node->edge_ids().front());
						 * }
						 */
						 
						if (_last_clustering_was_at_crossing)
						{
							/* We are at a crossing. */
//							std::cout << "_last_clustering_was_at_crossing; ";

							/* Now we have to calculate if _previous_node and _current_node
							 * are on a same edge. Consequences? See below! */
							bool on_same_edge = on_the_same_edge();
							
							if (on_same_edge)
							{
								/* CASE 8:
								 * We know that _previous_node and _current_node are on a same
								 * edge so no extra work is needed. We merged the points (see
								 * above) and that's it! */
//								std::cout << "on_same_edge; ";
							} else //explicit: if (!on_same_edge)
							{
								/* We know that _previous_node and _current_node have no edge in
								 * common. But we know that the merge before the very last merge
								 * took NOT place at a crossing. We have to check where we
								 * left the edge _previous_node is located on (named
								 * previous_edge in the following). */
//								std::cout << "!on_same_edge; ";
								EdgeCache::Pointer previous_edge = _edge_cache->get(edge_id_before_merge);
								
								if (previous_edge->node_is_at_end(_previous_node_id))
								{
									/* CASE 9:
									 * We left previous_edge over the end. Perfect. Extend that
									 * edge by _current_node.
									 * Remember: _last_clustering_was_at_crossing is true, so
									 * we do not need the set it here. */
//									std::cout << "previous_edge->node_is_at_end(_previous_node_id); ";
									extend_edge(previous_edge->get_id(), _previous_node_id,
										_current_node_id);
/*									_tile_manager->request_edge_extend(
											previous_edge->get_id(),
											_previous_node_id,
											_current_node_id,
											_filtered_trace.get_time_from_previous_start()
									);*/
								} else // explicit: if (!previous_edge->node_is_at_end(_previous_node_id))
								{
									/* CASE 10:
									 * We left previous_edge somewhere before the end. So we need
									 * the split previous_edge there und create a new edge from
									 * _previous_node to _current_node.
									 * As said above the encapsulation of create_new_edge with the 
									 * _do_not_change_current_edge attributes is done only (!) for 
									 * speed reasons. */
//									std::cout << "!previous_edge->node_is_at_end(_previous_node_id); ";
										split_edge(previous_edge->get_id(), _previous_node_id);
/*									_tile_manager->request_edge_split(previous_edge->get_id(), _previous_node_id);*/
									
									_do_not_change_current_edge = true;
									create_new_edge(true, true);
									_do_not_change_current_edge = false;
								} // end: if (previous_edge->node_is_at_end(_previous_node_id))
							} // end: if(on_same_edge)

						} else // exclicit: if (!_last_clustering_was_at_crossing)
						{
							/* We are not a crossing (respectively merge_node_and_gps_point
							 * did not get it).
							 * So we have to check manually what happened exactly. First we
							 * check if _current_edge changed (remember we saved the ID of
							 * _current_edge before merging). */
//							std::cout << "!_last_clustering_was_at_crossing; ";
							if (_current_edge->get_id() == edge_id_before_merge)
							{
								/* CASE 11:
								 * ID of _current_edge did not change (and consequently
								 * _current_edge itself did not either). We merged points
								 * and that's it. Next loop please :-) */
//								std::cout << "_current_eged->get_id() == edge_id_before_merge; ";
							} else // explicit: if (_current_edge->get_id() != edge_id_before_merge)
							{
								/* ID of _current_edge changed.
								 * Normally this happens when a new crossing is built or two
								 * edges are connected together but we have to consider a
								 * special case: */
//								std::cout << "_current_edge->get_id() != edge_id_before_merge; ";
								if (_previous_node->angle_difference(*_current_node) > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES)
								{
									/* CASE 12:
									 * The diffence in angle between _previous_node and
									 * _current_node is too big. Possibly a u-turn. We do not want
									 * to create a crossing from the one direction of a edge
									 * directly to the other direction. So we do nothing here. */
//									std::cout << "_previous_node->angle_difference(*_current_node) > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES; ";
								} else // explicit: if (_previous_node->angle_difference(_current_node) <= _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES)
								{
									/* Okay, the difference in angle is small enough.
									 * We have to check where _previous_node respectively
									 * _current_node are located in their edges. So first we
									 * fetch the edge where _previous_node is located in 
									 * (named previous_edge in the following.). */
//									std::cout << "_previous_node->angle_difference(*_current_node) <= _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES; ";
									EdgeCache::Pointer previous_edge = _edge_cache->get(edge_id_before_merge);
									if (previous_edge->node_is_at_end(_previous_node_id))
									{
										/* _previous_node is an ending point of an edge. */
//										std::cout << "previous_edge->node_is_at_end(_previous_node_id); ";
										if (_current_edge->node_is_at_start(_current_node_id))
										{
											/* CASE 13:
											 * And _current_node is a starting point. Perfect. We
											 * will just connect the two edges. */
//											std::cout << "_current_edge->node_is_at_start(_current_node_id); ";
											connect_edges();
										} else // explicit: if (!_current_edge->node_is_at_start(_current_node);
										{
											/* CASE 14:
											 * _current_node is NOT a starting point. A t-crossing.
											 * We will extend previous_edge by _current_node and then
											 * split the _current_edge.
											 * Because we are at a crossing now we have to set
											 * _last_clustering_ws_at_crossing.
											 * set_current_edge(0) is needed for the decision if we
											 * have to walk on egde AND trace or on trace only.
											 * See the TODOs concerning set_current_edge(0) or look
											 * above. */
//											std::cout << "!_current_edge->node_is_at_start(_current_node_id); ";
											extend_edge(previous_edge->get_id(), _previous_node_id, 
												_current_node_id);
/*											_tile_manager->request_edge_extend(previous_edge->get_id(),
													_previous_node_id,
													_current_node_id,
													_filtered_trace.get_time_from_previous_start());*/
											
											split_edge(_current_edge->get_id(), _current_node_id);
/*											_tile_manager->request_edge_split(_current_edge->get_id(), _current_node_id);*/
											
											#ifdef TRACELOGS
												Node& second_node = _tile_cache->get(_current_node_id.first)->nodes()[_current_node_id.second].second;
												_old_trace_log.draw_circle(second_node, 10.0);
												_old_trace_log.next_step();
											#endif
											
											_last_clustering_was_at_crossing = true;
											set_current_edge(0);
										} // end if(_current_edge->node_is_at_start(_current_node)

									} else // explicit: if (!previous_edge->node_is _at_end(_previous_node)
									{
										/* _previous_node is NOT an ending point.
										 * We left previous_edge before its end. So we need a split
										 * there wherever _current_node is located.*/
//										std::cout << "!previous_edge->node_is_at_end(_previous_node_id); ";
										split_edge(previous_edge->get_id(), _previous_node_id);
/*										_tile_manager->request_edge_split(previous_edge->get_id(), _previous_node_id);*/
										#ifdef TRACELOGS
											Node& second_node = _tile_cache->get(_previous_node_id.first)->nodes()[_previous_node_id.second].second;
											_old_trace_log.draw_circle(second_node, 10.0);
											_old_trace_log.next_step();
										#endif

										if (_current_edge->node_is_at_start(_current_node_id))
										{
											/* CASE 15:
											 * _current_node is a starting point of an edge. Well,
											 * we have to connect that edge to the crossing we just
											 * made by extending that edge by _previous_node. */
//											std::cout << "_current_edge->node_is_at_start(_current_node_id); ";
											extend_edge(_current_edge->get_id(), _current_node_id,
												_previous_node_id);
/*											_tile_manager->request_edge_extend(_current_edge->get_id(),
												_current_node_id,
												_previous_node_id,
												_filtered_trace.get_time_from_previous_start());*/
										} else // explicit: if (!_current_edge->node_is_at_start(_current_node_id);
										{
											/* CASE 16:
											 * _current_node is NOT a starting point. So we have to
											 * split _current_edge as well. Afterwards we create a
											 * connection between _previous_node and _current_node
											 * using create_new_edge.
											 * 
											 * As said above the encapsulation of create_new_edge with
											 * the _do_not_change_current_edge attributes is done
											 * only (!) for speed reasons.
											 * 
											 * Then we have to set _last_clustering_was_at_a_crossing.
											 * 
											 * set_current_edge(0) is needed for the decision if we
											 * have to walk on egde AND trace or on trace only.
											 * See the TODOs concerning set_current_edge(0) or look
											 * above. */
//											std::cout << "!_current_edge->node_is_at_start(_current_node_id); ";
											split_edge(_current_edge->get_id(), _current_node_id);
/*											_tile_manager->request_edge_split(_current_edge->get_id(), _current_node_id);*/
											#ifdef TRACELOGS
												second_node = _tile_cache->get(_current_node_id.first)->nodes()[_current_node_id.second].second;
												_old_trace_log.draw_circle(second_node, 10.0);
												_old_trace_log.next_step();
											#endif
				
											_do_not_change_current_edge = true;
											create_new_edge(true, true);
											_do_not_change_current_edge = false;
											
											_last_clustering_was_at_crossing = true;
											set_current_edge(0);
										
										} // end: if(_current_edge->node_is_at_start(_current_node_id)
									} // end: if (previous_edge->node_is_at_end(_previous_node_id)
								} // end: if (_previous_node->angle_difference(_current_node) > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES)
							} // end: if (_current_edge->get_id() == edge_id_before_merge)				
						} // end: if (_last_clustering_was_at_crossing)
						
					} else // explicit: if (!mergeable())
					{
						/* Remember: We are not at a crossing, not at a beginning state and
						 * now we did not find a mergeable point. */
//						std::cout << "!mergeable(); ";
						if (_previous_node->angle_difference(_filtered_trace.front()) > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES)
						{
							/* CASE 17:
							 * First we test the difference in angle between _previous_node
							 * and the front of _filtered_trace. If this difference exceeds
							 * a defined threshold, we assume that a u-turn took place. 
							 * And u-turn are bad, very bad :-)) So we ends the _current_edge
							 * and prepares the creation of a new one which goes to the
							 * opposite direction by setting _unused_gps_point to the
							 * front of _filtered_trace and
							 * _trace_processing_is_at_a_beginning_state to true. */
//							std::cout << "_previous_node->angle_difference(_filtered_trace.front()) > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES; ";
							_unused_gps_point = new GPSPoint(_filtered_trace.front());
							_trace_processing_is_at_a_beginning_state = true;
							set_current_edge(0);
						} else // explicit: if (_previous_node->angle_difference(_filtered_trace.front()) <= _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES)
						{
							/* Difference of angle within the defined threshold.
							 * We check where we left _current_edge. */
//							std::cout << "_previous_node->angle_difference(_filtered_trace.front()) <= _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES; ";
							if (_current_edge->node_is_at_end(_previous_node_id))
							{
								/* CASE 18:
								 * We left _current_edge over the ending point. Perfect.
								 * Just extend that edge by the front of _filtered_trace. */
//								std::cout << "_current_edge_->node_is_at_end(_previous_node_id); ";
								std::pair<unsigned int, unsigned int> new_node_id
									= create_new_node(_filtered_trace.front());
								_current_node = &_tile_cache->get(new_node_id.first).write()
									.nodes()[new_node_id.second].second;
								_current_node_id = new_node_id;
								
								extend_edge(_current_edge->get_id(), _previous_node_id,
									_current_node_id);
									
								_current_edge_node_ids_iter = _current_edge.write().node_ids().end();
								--_current_edge_node_ids_iter;
								_current_edge_times_iter = _current_edge.write().times().end();
								--_current_edge_times_iter;
									
							} else // explicit: if (!_current_edge->node_is_at_end(_previous_id)
							{
								/* CASE 19:
								 * We left _current_edge somewhere before the ending point. So
								 * we have to split that edge and create a new edge from
								 * _previous_node to the front of _filtered_trace.
								 * Note: We do not have to check if we left _current_edge at the
								 * starting point (then no split is needed). The method of
								 * TileManager ist smart enough to cover this case. */
//								std::cout << "_current_edge_->node_is_at_end(_previous_node_id); ";
								split_edge(_current_edge->get_id(), _previous_node_id);
/*								_tile_manager->request_edge_split(_current_edge->get_id(), _previous_node_id);*/
								#ifdef TRACELOGS
									Node& second_node = _tile_cache->get(_previous_node_id.first)->nodes()[_previous_node_id.second].second;
									_old_trace_log.draw_circle(second_node, 10.0);
									_old_trace_log.next_step();
								#endif
								create_new_edge(true, false);
							} // end: if (_current_edge->node_is_at_end(_previous_id)
						} // end: if (_previous_node->angle_difference(_filtered_trace.front()) > _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES)
					} // end: if (mergeable())
				} // end: if (_current_edge == 0)
			} // end: if (_last_clustering_was_at_crossing)

			#ifdef TRACELOGS				
			/* NUR F??R DIE PRAESENTATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
			{
				std::vector<unsigned int> needed_tile_ids  = _filtered_trace.front().get_needed_tile_ids(30);
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
						mlog(MLog::debug, "TraceProcessor::run") 
							<< "Presentation Created new tile (should disappear soon!)" 
							<< (*needed_tile_ids_iter) << " \n";
					}
				
					/* Draw the tile to the log. */
					//_old_trace_log.draw_tile(*tile);
					if (draw_tile_id != 0) _old_trace_log.remove_order(draw_tile_id);
					draw_tile_id = _old_trace_log.draw_tile_with_edges(*tile, _tile_manager->get_tile_edges_nodes(tile->get_id()));
				}		
			}
			#endif
			
			#ifdef TRACELOGS
				/* ENDE NUR F??R DIE PRAESENTATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
				
				if (_current_edge != 0)
				{
					_old_trace_log.remove_order(draw_edge_id);
					draw_edge_id = _old_trace_log.draw_edge(*_current_edge, _tile_manager->get_edge_nodes(_current_edge->get_id()));
				}			
				_old_trace_log.remove_order(draw_trace_id);
				draw_trace_id = _old_trace_log.draw_filtered_trace(_filtered_trace);
				_old_trace_log.next_step();
			#endif
				
			/* Now we have to walk a little bit. If there is a _current_edge we walk
			 * on edge AND trace. Else we only walk on trace.
			 * See the TODOs concerning set_current_edge(0) or look above. */
			if (_current_edge == 0)
			{
				walk_on_trace();
//				std::cout << "walk_on_trace(); ";
			}	else
			{
				walk_on_edge_and_trace();
//				std::cout << "walk_on_edge_and_trace(); ";
			}
			
			_trace_log->next_step();
				
//			std::cout << std::endl << std::endl;
		} //end of main for loop
		
		/* Hey, you (AND me) made it! Be happy and enjoy the results! */
		
		#ifdef TRACELOGS
			std::ostringstream stringstream;
			stringstream << "oldtracelog" << _id;
			std::ofstream file_stream(stringstream.rdbuf()->str().c_str()); 
			if (!file_stream) mlog(MLog::error, "TraceProcessor") << "Could not write log!";
			file_stream << _old_trace_log.get_log_string();
		#endif
		
		delete _trace_log;
		_trace_log = 0;

		mlog(MLog::info, "TraceProcessor") << "Finished (" << _id << ").\n";
		_tile_manager->trace_processor_finished(_id);
	}
	
	
	void
	TraceProcessor::set_current_edge(EdgeCache::Pointer new_current_edge)
	{
		if (_do_not_change_current_edge)
			return;
			
		/** @todo: Changed from 
		 * (*_current_edge == *new_current_edge)
		 * to:
		 * (_current_edge == new_current_edge)
		 * does this work?
		 */
		if ((_current_edge != 0) && (new_current_edge != 0) && (_current_edge == new_current_edge))
			return;
			
		if (new_current_edge != 0)
		{
			_current_edge = new_current_edge;
			recalculate_current_edge_iters();
		} else
		{
			_current_edge = 0;
		}
	}


	void
	TraceProcessor::split_edge(unsigned int edge_id, std::pair<unsigned int, unsigned int>& node_id)
	{
		unsigned int new_edge_1_id;
		unsigned int new_edge_2_id;
		
		if (_tile_manager->request_edge_split(edge_id, node_id, &new_edge_1_id, &new_edge_2_id))
		{
			
			_trace_log->split_edge(edge_id, *(_edge_cache->get(new_edge_1_id)),
				*(_edge_cache->get(new_edge_2_id)) );
		}
	}
	
	
	void
	TraceProcessor::walk_on_edge_and_trace()
	{
		assert(_current_edge_node_ids_iter != 0);
		assert(_current_edge_times_iter != 0);
		assert(_current_edge != 0);
//		assert(_current_node != 0);
		
		++_current_edge_node_ids_iter;
		++_current_edge_times_iter;
		
		if (_current_edge_node_ids_iter == _current_edge->node_ids().end())
		{
			walk_on_trace();
		} else
		{
			Node* next_node = &_tile_cache->get(_current_edge_node_ids_iter->first)
				.write().nodes()[_current_edge_node_ids_iter->second].second;
			walk_on_trace(_current_node->distance(*next_node));
			_current_node = next_node;
			_current_node_id = *_current_edge_node_ids_iter;
		}
	}
	
	
	void
	TraceProcessor::walk_on_trace(double meters)
	{
//		std::cout << "walking [=" << meters << "m]; ";
		std::list<GPSPoint>::iterator new_start_iter = _filtered_trace.new_gps_point_at(meters);
		if (new_start_iter == 0)
		{
			_end_main_loop = true;
		} else
		{
			int removed_gps_points;
			_filtered_trace.new_start(new_start_iter, &removed_gps_points);
			_trace_log->changed_trace(*new_start_iter, removed_gps_points);
		}
		move_current_to_previous();
	}

} // namespace mapgeneration
