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

#include <deque>
#include <cc++/thread.h>
#include "filteredtrace.h"
#include "node.h"
#include "tilemanager.h"
#include "tracelogwriter.h"
#include "util/constants.h"
#include "util/mlog.h"
#include "util/pubsub/servicesystem.h"


namespace mapgeneration
{

	/**
	 * @brief TraceProcessor process a new trace and include it into the
	 * existing map.
	 */
	class TraceProcessor : public ost::Thread
	{
		
		
		private:
		
			class PathConnection;
		
		
			class PathEntry
			{
				public:
					double _position;
					Node::Id _node_id;
					int _time_stamp;
					
					double _points;
										
					PathEntry* _connection;
					
					
					bool
					operator==(PathEntry path_entry)
					{
						if (_node_id == path_entry._node_id)
							return true;
						else
							return false;
					}
					
					
					PathEntry()
					: _position(0), _node_id(0), _connection(0), _time_stamp(0)
					{
					}
					
					PathEntry(const double position, const int path_id, const Node::Id node_id)
					: _position(position), _node_id(node_id), _connection(0), _time_stamp(0)
					{
					}
			};

		
		
		public:	

			/**
			 * @brief Constructor that transfers the necessary inforamtion to run
			 * the TraceProcessor.
			 */
			TraceProcessor (unsigned int id, TileManager* tile_manager, 
				pubsub::ServiceList* service_list, 
				FilteredTrace& filtered_trace);


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
//			static const double _ANGLE_DIFFERENCE = PI / 4.0; // was 30??
			
			
			/**
			 * @brief a constant for the maximum angle difference between two
			 * following nodes
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
//			static const double _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES = PI * 0.9;
			
			
			/**
			 * @brief a constant for the search radius (in meters)
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
//			static const double _SEARCH_RADIUS = 24.0; // was 16m
		
		
		private:
		
		
			/*
			 * We start with an extra section for all the parameter:
			 * *****************************************************
			 */

			double _search_step_size_meters;
			
			
			double _search_radius_m;
			
			
			double _search_max_angle_difference_pi;
			
			

			/*
			 * Now the ordinary members:
			 * *************************
			 */
			
			/**
			 * @brief the FilteredTrace the TraceProcessor is processing
			 */
			FilteredTrace _filtered_trace;
			
			
			/**
			 * @brief the ID
			 */
			unsigned int _id;
			
			
			/**
			 * @brief The service list.
			 */
			pubsub::ServiceList* _service_list;
			
			
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
			
			
			double
			build_connections(std::list<PathEntry>& path,
				std::list<PathEntry>::iterator path_iter, bool only_connected);


			/**
			 * @brief Calculates all clusterable nodes and adds them to the
			 * given vector.
			 * 
			 * @param geo_coordinate The position to start from.
			 * @param result_vector The vector to store the information into.
			 */
			void
			calculate_cluster_nodes(GPSPoint gps_point,
				std::list<Node::Id>& result_vector);
			

			void
			connect_nodes(Node::Id first_node_id, Node::Id second_node_id);
			
			
			bool
			connection_from_to(Node::Id node_id_1, Node::Id node_id_2);
			
			
			/**
			 * @brief Creates all needed tiles.
			 * 
			 * This method creates all tiles from the list of needed tiles in
			 * the filtered trace. This way all tiles already exist when the
			 * tracelog is started.
			 */
			void
			create_needed_tiles();
		

			/**
			 * @brief Creates a new Node.
			 * 
			 * @param gps_point the GPSPoint the Node is created from.
			 * 
			 * @return Id of the new created node.
			 */
			Node::Id
			create_new_node(GPSPoint& gps_point);
			
			
			double
			TraceProcessor::distance_from_to(Node::Id node_id_1, 
				Node::Id node_id_2);

			
			/**
			 * @brief Merges a Node and a GPSPoint.
			 * 
			 * @param first_point if set false, the time is merged too, else not
			 */
			void
			merge_node_and_gps_point(/*bool first_point*/);
			
			
			/**
			 * @brief Return true, if at least one mergeable point is found.
			 * 
			 * @return true, if at least one mergeable point is found, else false
			 */
/*			bool
			mergeable();*/
			
			
			double
			optimal_node_position(PathEntry path_entry);
			
			
			void
			TraceProcessor::rebuild_path(std::list<PathEntry>& path,
				PathEntry* start_entry);
			
			
			void
			simplify_path(Node::Id previous_node_id, std::list<PathEntry>& path);
			
			
			/**
			 * @brief Returns a TileCache::Pointer to the tile the node
			 * with the given id is on.
			 * 
			 * @return TileCache::Pointer to the tile the node is on.
			 */
			TileCache::Pointer
			tile(Node::Id node_id);
			
			
			int _time;
	};

	
} // namespace mapgeneration

#endif //TRACEPROCESSOR_H
