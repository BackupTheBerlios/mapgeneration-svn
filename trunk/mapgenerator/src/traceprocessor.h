/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACEPROCESSOR_H
#define TRACEPROCESSOR_H

namespace mapgeneration
{
	class TraceProcessor;
}

#include <cc++/thread.h>
#include "filteredtrace.h"
#include "tilemanager.h"
#include "tracelogwriter.h"
#include "util/constants.h"
#include "util/mlog.h"


namespace mapgeneration
{

	/**
	 * @brief TraceProcessor process a new trace and include it into the
	 * existing map.
	 */
	class TraceProcessor : public ost::Thread {

		public:	

			/**
			 * @brief Constructor that transfers the necessary inforamtion to run
			 * the TraceProcessor.
			 */
			TraceProcessor (unsigned int id, TileManager* tile_manager, FilteredTrace& filtered_trace);


			/**
			 * @brief The run method. Mainly the cluster algorithm.
			 */
			void
			run();

		
			/**
			 * @brief a constant for the angle difference
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
			static const double _ANGLE_DIFFERENCE = PI / 18.0; // was 30??
			
			
			/**
			 * @brief a constant for the maximum angle difference between two
			 * following nodes
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
			static const double _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES = PI * 0.9;
			
			
			/**
			 * @brief a constant for the search radius (in meters)
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
			static const double _SEARCH_RADIUS = 24.0; // was 16m


			/**
			 * @brief a constant for the walk meters on a trace (in meters)
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
			static const double _WALK_METERS_ON_TRACE = 30.0; // was 20m
		
		
		private:

			/**
			 * @brief Pointer to the EdgeCache
			 */
			EdgeCache* _edge_cache;
			
			
			/**
			 * @brief the FilteredTrace the TraceProcessor is processing
			 */
			FilteredTrace _filtered_trace;
			
			
			/**
			 * @brief the ID
			 */
			unsigned int _id;
			
			
			/**
			 * @brief Pointer to the TileCache
			 */
			TileCache* _tile_cache;
			
			
			/**
			 * @brief Pointer to the TileManager
			 */
			TileManager* _tile_manager;
			
			
			/**
			 * @brief The TraceProcessorLogger for this TraceProcessor
			 */
			TraceLogWriter* _trace_log;


			#ifdef TRACELOGS
			/**
			 * @brief the TraceProcessorLogger for this TraceProcessor.
			 */
			OldTraceProcessorLog _old_trace_log;
			#endif


			/**
			 * @brief Pointer to the best clusterable node.
			 * 
			 * Only contains a reasonable value after invocation of
			 * calculate_best_cluster_node.
			 * 
			 * @see calculate_best_cluster_node
			 * @see _best_cluster_node_id
			 */
			Node* _best_cluster_node;			


			/**
			 * @brief the ID of the best clusterable node, always corresponds 
			 * to the _best_cluster_node.
			 * 
			 * Only contains a reasonable value after invocation of
			 * calculate_best_cluster_node.
			 * 
			 * @see calculate_best_cluster_node
			 * @see _best_cluster_node
			 */
			std::pair<unsigned int, unsigned int> _best_cluster_node_id;


			/** 
			 * @brief Pointer to the current edge, if known.
			 */
			EdgeCache::Pointer _current_edge;


			/**
			 * @brief An iterator that points to the _current_node_id in the
			 * _current_edge, if possible.
			 */
			std::list< std::pair<unsigned int, unsigned int> >::iterator _current_edge_node_ids_iter;


			/**
			 * @brief An iterator that points to the time that corresponds to
			 * the next edge part after the current node.
			 */
			std::list<double>::iterator _current_edge_times_iter;
			
			
			/**
			 * @brief Pointer to the current node.
			 * 
			 * Only defined after current node is known.
			 * 
			 * @see _current_node_id
			 */
			Node* _current_node;
			
			
			/**
			 * @brief The ID of the current node, always corresponds to the
			 * _current_node.
			 * 
			 * @see _current_node
			 */
			std::pair<unsigned int, unsigned int> _current_node_id;
			
			
			/**
			 * @brief A copy of the current node before it was merged.
			 */
			Node _current_node_before_merge;


			/**
			 * @brief Flag indicating that the _current_edge is not to be changes.
			 */
			bool _do_not_change_current_edge;
			
			
			/**
			 * @brief Flag indicating that the main loop ends.
			 */
			bool _end_main_loop;


			/**
			 * @brief Flag indicating that the last clustering was at a crossing.
			 */
			bool _last_clustering_was_at_crossing;
			
			
			/**
			 * @brief Pointer to the previous node.
			 * 
			 * Is 0 in the first loop.
			 * 
			 * @see _previous_node_id
			 */
			Node* _previous_node;


			/**
			 * @brief The ID of the previous node, always corresponds to the 
			 * _previous_node.
			 * 
			 * @see _previous_node
			 */
			std::pair<unsigned int, unsigned int> _previous_node_id;
			
			
			/**
			 * @brief Flag indicating that the trace processing is just at
			 * its beginning.
			 */
			bool _trace_processing_is_at_a_beginning_state;


			/**
			 * @brief Pointer to an older unused GPSPoint if it was not used.
			 */
			GPSPoint* _unused_gps_point;


			/**
			 * @brief Calculates the best cluster node.
			 * 
			 * After invokation _best_cluster_node and _best_cluster_node_id
			 * contains reasonable values.
			 */
			void
			calculate_best_cluster_node();
			
			
			/**
			 * @brief Connects edges.
			 * 
			 * This method connects the edges _previous_node resp. _current_node
			 * are located on.
			 */
			void
			connect_edges();
			
			
			/**
			 * @brief Creates all needed tiles.
			 * 
			 * This method creates all tiles from the list of needed tiles in
			 * the filtered trace. This way all tiles already exist when the
			 * tracelog is started.
			 */
			void
			TraceProcessor::create_needed_tiles();
		
			
			/**
			 * @brief Creates a new edge.
			 * 
			 * The flag controls the start resp. the end of the new edge.
			 * 
			 * @param start_at_previous if set true, the new edge starts at
			 * _previous_node, else at _unused_gps_point
			 * @param end_at_current if set true, the new edge ends at _current_node,
			 * else at a new Node creates from the starting point of the
			 * _filtered_trace.
			 */
			void
			create_new_edge(bool start_at_previous, bool end_at_current);


			/**
			 * @brief Creates a new Node.
			 * 
			 * @param gps_point the GPSPoint the Node is created from
			 */
			std::pair<unsigned int, unsigned int>
			create_new_node(GPSPoint& gps_point);			


			/**
			 * @brief Extends an edge.
			 * 
			 * This method extends an edge.
			 *
			 * @param edge_id the edge 
			 * @param next_to_node_id the to this node the edge will be extended
			 * @param new_node_id the new node
			 */
			void
			extend_edge(unsigned int edge_id,
				std::pair<unsigned int, unsigned int>& next_to_node_id,
				std::pair<unsigned int, unsigned int>& new_node_id);

			
			/**
			 * @brief Merges a Node and a GPSPoint.
			 * 
			 * @param first_point if set false, the time is merged too, else not
			 */
			void
			merge_node_and_gps_point(bool first_point);
			
			
			/**
			 * @brief Return true, if at least one mergeable point is found.
			 * 
			 * @return true, if at least one mergeable point is found, else false
			 */
			bool
			mergeable();
			
			
			/**
			 * @brief Copies every _current_* attribute to the corresponding
			 * _previous_* attribute.
			 */
			void
			move_current_to_previous();
			
			
			/**
			 * @brief Calculates if we are on the same edge
			 */
			bool
			on_the_same_edge();
			
			
			/**
			 * @brief Recalculates the "current edge iters".
			 * 
			 * Needed when _current_edge has changed.
			 */
			void
			recalculate_current_edge_iters();
			
			
			/**
			 * @brief Sets a new _current_edge.
			 * 
			 * @param new_current_edge the new _current_edge
			 * 
			 * @see recalculate_current_edge_iters
			 */
			void
			set_current_edge(EdgeCache::Pointer new_current_edge);
			
			
			/**
			 * @brief Splits an edge.
			 * 
			 * This method splits the edge with edge_id at the node with node_id.
			 * 
			 * @param edge_id the edge ID
			 * @param node_id the node ID
			 */
			void
			split_edge(unsigned int edge_id, std::pair<unsigned int, unsigned int>& node_id);
			

			/**
			 * @brief Walks on the edge and the trace for _WALK_METERS_ON_TRACE.
			 * 
			 * @todo Does _WALK_METERS_ON_TRACE still exist?
			 */
			void
			walk_on_edge_and_trace();
			
			
			/**
			 * @brief Walks on the trace for _WALK_METERS_ON_TRACE.
			 * 
			 * @todo Does _WALK_METERS_ON_TRACE still exist?
			 */
			inline void
			walk_on_trace();


			/**
			 * @brief Walks on the trace for the specified distance.
			 * 
			 * @param meters the distance to walk
			 */
			void
			walk_on_trace(double meters);
	};
	
	
	inline void
	TraceProcessor::walk_on_trace()
	{
		walk_on_trace(_WALK_METERS_ON_TRACE);
	}
	
} // namespace mapgeneration

#endif //TRACEPROCESSOR_H
